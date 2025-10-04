#pragma once

#include "resource.h"

#include <linalg.h>

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
    void set_pixel_shader(std::function<cg::color(const VertexBufferElement& vertex_data, const float z)> shader);

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
    std::function<cg::color(const VertexBufferElement& vertex_data, const float z)> pixel_shader;
    // NOLINTEND(*-non-private-*)

    int edge_function(int2 a, int2 b, int2 c);
    bool depth_test(float z, std::size_t x, std::size_t y);
};

template <typename VB, typename RT>
rasterizer<VB, RT>::rasterizer(std::size_t width,
                               std::size_t height,
                               std::shared_ptr<resource<RT>> in_render_target,
                               std::shared_ptr<resource<float>> in_depth_buffer)
    : width{width}, height{height}, render_target{std::move(in_render_target)},
      depth_buffer{std::move(in_depth_buffer)} {
    // TODO Lab: 1.06 Adjust `set_render_target`, and `clear_render_target` methods of `cg::renderer::rasterizer` class
    // to consume a depth buffer
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::clear_render_target(const RT& in_clear_value, const float /*in_depth*/) {
    for (std::size_t i = 0; i < render_target->count(); ++i) {
        render_target->item(i) = in_clear_value;
        // depth_buffer->item(i) = in_depth;
    }
    // TODO Lab: 1.06 Adjust `set_render_target`, and `clear_render_target` methods of `cg::renderer::rasterizer` class
    // to consume a depth buffer
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
        vertices.reserve(3);
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));
        vertices.push_back(vertex_buffer->item(index_buffer->item(vertex_i++)));

        for (vertex& vertex : vertices) {
            float4 coords{vertex.v.x, vertex.v.y, vertex.v.z, 1};
            std::pair<float4, VB> transformed = vertex_shader(coords, vertex);

            vertex.v.x = transformed.first.x / transformed.first.w;
            vertex.v.y = transformed.first.y / transformed.first.w;
            vertex.v.z = transformed.first.z / transformed.first.w;

            vertex.v.x = (vertex.v.x + 1) * width / 2;
            vertex.v.y = (-vertex.v.y + 1) * height / 2;
        }
    }
    // TODO Lab: 1.05 Add `Rasterization` and `Pixel shader` stages to `draw` method of `cg::renderer::rasterizer`
    // TODO Lab: 1.06 Add `Depth test` stage to `draw` method of `cg::renderer::rasterizer`
}

template <typename VB, typename RT>
int rasterizer<VB, RT>::edge_function(int2 /*a*/, int2 /*b*/, int2 /*c*/) {
    // TODO Lab: 1.05 Implement `cg::renderer::rasterizer::edge_function` method
    return 0;
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
void rasterizer<VB, RT>::set_pixel_shader(std::function<cg::color(const VB& vertex_data, const float z)> shader) {
    pixel_shader = std::move(shader);
}

} // namespace cg::renderer
