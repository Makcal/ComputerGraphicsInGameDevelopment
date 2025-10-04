#pragma once

#include "resource.h"

#include <linalg.h>

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <utility>

namespace cg::renderer {

using namespace linalg::aliases;

static constexpr float DEFAULT_DEPTH = std::numeric_limits<float>::max();

template <typename VertexBufferElement, typename RenderTargetElement>
class rasterizer {
  public:
    rasterizer(std::size_t width,
               std::size_t height,
               std::shared_ptr<resource<RenderTargetElement>> in_render_target,
               std::shared_ptr<resource<float>> in_depth_buffer);

    void clear_render_target(const RenderTargetElement& in_clear_value, float in_depth = DEFAULT_DEPTH);

    void set_vertex_buffer(std::shared_ptr<resource<VertexBufferElement>> in_vertex_buffer);
    void set_index_buffer(std::shared_ptr<resource<std::size_t>> in_index_buffer);

    void draw(std::size_t num_vertexes, std::size_t vertex_offset);

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
    vertex_buffer = in_vertex_buffer;
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::set_index_buffer(std::shared_ptr<resource<std::size_t>> in_index_buffer) {
    index_buffer = std::move(in_index_buffer);
}

template <typename VB, typename RT>
void rasterizer<VB, RT>::draw(std::size_t num_vertexes, std::size_t vertex_offset) {
    // TODO Lab: 1.04 Implement `cg::world::camera` class
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

} // namespace cg::renderer
