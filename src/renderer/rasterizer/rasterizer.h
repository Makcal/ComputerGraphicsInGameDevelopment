#pragma once

#include "resource.h"

#include <linalg.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace cg::renderer {

using namespace linalg::aliases;

static constexpr float kDefaultDepth = std::numeric_limits<float>::max();

template <typename VertexBufferElement, typename RenderTargetElement>
class rasterizer {
  public:
    rasterizer(std::size_t width,
               std::size_t height,
               std::shared_ptr<resource<RenderTargetElement>> in_render_target,
               std::shared_ptr<resource<float>> in_depth_buffer);

    void clear_render_target(const RenderTargetElement& in_clear_value, float in_depth = kDefaultDepth);

    void set_vertex_buffer(std::shared_ptr<resource<VertexBufferElement>> in_vertex_buffer);
    void set_index_buffer(std::shared_ptr<resource<std::size_t>> in_index_buffer);

    void set_vertex_shader(
        std::function<std::pair<float4, VertexBufferElement>(float4 vertex, VertexBufferElement vertex_data)> shader);
    void set_pixel_shader(std::function<cg::color(const VertexBufferElement& vertex_data, float z)> shader);

    void draw(std::size_t num_vertices, std::size_t vertex_offset);

  protected:
    // NOLINTBEGIN(*-non-private-*)
    std::shared_ptr<cg::resource<VertexBufferElement>> vertex_buffer;
    std::shared_ptr<cg::resource<std::size_t>> index_buffer;
    std::shared_ptr<cg::resource<RenderTargetElement>> render_target;
    std::shared_ptr<cg::resource<float>> depth_buffer;

    std::size_t width;
    std::size_t height;

    std::function<std::pair<float4, VertexBufferElement>(float4 vertex, VertexBufferElement vertex_data)> vertex_shader;
    std::function<cg::color(const VertexBufferElement& vertex_data, float z)> pixel_shader;
    // NOLINTEND(*-non-private-*)

    int edge_function(int2 a, int2 b, int2 p);
    bool depth_test(float z, std::size_t x, std::size_t y);
};

template <typename VB, typename RT>
rasterizer<VB, RT>::rasterizer(std::size_t width,
                               std::size_t height,
                               std::shared_ptr<resource<RT>> in_render_target,
                               std::shared_ptr<resource<float>> in_depth_buffer)
    : width{width}, height{height}, render_target{std::move(in_render_target)},
      depth_buffer{std::move(in_depth_buffer)} {}

template <typename VB, typename RT>
void rasterizer<VB, RT>::clear_render_target(const RT& in_clear_value, const float in_depth) {
    for (std::size_t i = 0; i < render_target->count(); ++i) {
        render_target->item(i) = in_clear_value;
        depth_buffer->item(i) = in_depth;
    }
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::set_vertex_buffer(std::shared_ptr<resource<VB>> in_vertex_buffer) {
    vertex_buffer = std::move(in_vertex_buffer);
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::set_index_buffer(std::shared_ptr<resource<std::size_t>> in_index_buffer) {
    index_buffer = std::move(in_index_buffer);
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::draw(std::size_t num_vertices, std::size_t vertex_offset) {
    for (std::size_t vertex_i = vertex_offset; vertex_i < num_vertices + vertex_offset;) {
        std::vector<VB> vertices;
        vertices.reserve(3); // take by portions of 3
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));

        for (VB& vertex : vertices) {
            float4 coords{vertex.v.x, vertex.v.y, vertex.v.z, 1};
            std::pair<float4, VB> transformed = vertex_shader(coords, vertex);

            vertex.v.x = transformed.first.x / transformed.first.w;
            vertex.v.y = transformed.first.y / transformed.first.w;
            vertex.v.z = transformed.first.z / transformed.first.w;

            vertex.v.x = (vertex.v.x + 1) * width / 2;
            vertex.v.y = (-vertex.v.y + 1) * height / 2;
        }

        int2 vertex_a{static_cast<int>(vertices[0].v.x), static_cast<int>(vertices[0].v.y)};
        int2 vertex_b{static_cast<int>(vertices[1].v.x), static_cast<int>(vertices[1].v.y)};
        int2 vertex_c{static_cast<int>(vertices[2].v.x), static_cast<int>(vertices[2].v.y)};

        int2 min_border{0, 0};
        int2 max_border{static_cast<int>(width), static_cast<int>(height)};

        int2 min_vertex = linalg::min(vertex_a, linalg::min(vertex_b, vertex_c));
        min_vertex = linalg::clamp(min_vertex, min_border, max_border);
        int2 max_vertex = linalg::max(vertex_a, linalg::max(vertex_b, vertex_c));
        max_vertex = linalg::clamp(max_vertex, min_border, max_border);

        float edge = edge_function(vertex_a, vertex_b, vertex_c);

        for (int x = min_vertex.x; x < max_vertex.x; ++x) {
            for (int y = min_vertex.y; y < max_vertex.y; ++y) {
                int2 point{x, y};
                float u = static_cast<float>(edge_function(vertex_b, vertex_c, point)) / edge;
                float v = static_cast<float>(edge_function(vertex_c, vertex_a, point)) / edge;
                float w = static_cast<float>(edge_function(vertex_a, vertex_b, point)) / edge;
                if (u >= 0 && v >= 0 && w >= 0) {
                    float depth = (u * vertices[0].v.z) + (v * vertices[1].v.z) + (w * vertices[2].v.z);
                    if (depth_test(depth, x, y)) {
                        color result = pixel_shader(vertices[1], depth);
                        render_target->item(x, y) = RT::from_color(result);
                        depth_buffer->item(x, y) = depth;
                    }
                }
            }
        }
    }
}

template <typename VB, typename RT>
int rasterizer<VB, RT>::edge_function(int2 a, int2 b, int2 p) { // point P relative to AB
    int dy = b.y - a.y;
    int dx = b.x - a.x;
    return ((p.x - a.x) * dy) - ((p.y - a.y) * dx);
}

template <typename VB, typename RT>
bool rasterizer<VB, RT>::depth_test(float z, std::size_t x, std::size_t y) {
    if (!depth_buffer)
        return true;
    return depth_buffer->item(x, y) > z;
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::set_vertex_shader(std::function<std::pair<float4, VB>(float4 vertex, VB vertex_data)> shader) {
    vertex_shader = std::move(shader);
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::set_pixel_shader(std::function<cg::color(const VB& vertex_data, float z)> shader) {
    pixel_shader = std::move(shader);
}

} // namespace cg::renderer
