#include "rasterizer_renderer.h"
#include "linalg.h"
#include "rasterizer.h"
#include "resource.h"
#include "utils/resource_utils.h"
#include "utils/timer.h"

#include <cstddef>
#include <memory>
#include <utility>

namespace cg {

renderer::rasterization_renderer::rasterization_renderer(std::shared_ptr<cg::settings> settings)
    : renderer{std::move(settings)} {}

void renderer::rasterization_renderer::init() {
    render_target = std::make_shared<resource<unsigned_color>>(settings->width, settings->height);
    // depth_buffer = std::make_shared<resource<float>>(settings->width,
    // settings->height);

    rasterizer = std::make_shared<cg::renderer::rasterizer<vertex, unsigned_color>>(
        settings->width, settings->height, render_target, nullptr);

    renderer::load_model();
    renderer::load_camera();

    float4x4 matrix =
        linalg::mul(camera->get_projection_matrix(), camera->get_view_matrix(), model->get_world_matrix());
    rasterizer->set_vertex_shader([matrix](float4 vertex, cg::vertex vertex_data) {
        float4 transformed = mul(matrix, vertex);
        return std::make_pair(transformed, vertex_data);
    });

    rasterizer->set_pixel_shader([](const vertex& vertex_data, float  /*z*/) {
        return color::from_float3(vertex_data.ambient);
    });
    // TODO Lab: 1.06 Add depth buffer in `cg::renderer::rasterization_renderer`
}

void cg::renderer::rasterization_renderer::render() {
    static constexpr unsigned_color color = {.r = 153, .g = 255, .b = 204};
    {
        utils::timer timer{"render"};
        rasterizer->clear_render_target(color);
    }

    for (std::size_t shape_i = 0; shape_i < model->get_index_buffers().size(); ++shape_i) {
        rasterizer->set_vertex_buffer(model->get_vertex_buffers()[shape_i]);
        rasterizer->set_index_buffer(model->get_index_buffers()[shape_i]);
        rasterizer->draw(model->get_index_buffers()[shape_i]->count(), 0);
    }
}

void cg::renderer::rasterization_renderer::destroy() {
    utils::save_resource(*render_target, settings->result_path);
}

void cg::renderer::rasterization_renderer::update() {}

} // namespace cg
