#include "renderer.h"
#include "settings.h"

#include <memory>
#include <utility>

#ifdef RASTERIZATION
#include "renderer/rasterizer/rasterizer_renderer.h"
#endif

#ifdef RAYTRACING
#include "renderer/raytracer/raytracer_renderer.h"
#endif

#ifdef DX12
#include "renderer/dx12/dx12_renderer.h"
#endif

using namespace cg::renderer;

renderer::renderer(std::shared_ptr<cg::settings> settings) : settings{std::move(settings)} {}

unsigned cg::renderer::renderer::get_height() {
    return settings->height;
}

unsigned cg::renderer::renderer::get_width() {
    return settings->width;
}

std::shared_ptr<renderer> cg::renderer::make_renderer(std::shared_ptr<cg::settings> settings) {
#ifdef RASTERIZATION
    return std::make_shared<cg::renderer::rasterization_renderer>(std::move(settings));
#endif
#ifdef RAYTRACING
    return std::make_shared<cg::renderer::ray_tracing_renderer>(std::move(settings));
#endif
#ifdef DX12
    return std::make_shared<cg::renderer::dx12_renderer>(std::move(settings));
#endif
#if !defined(RASTERIZATION) && !defined(RAYTRACING) && !defined(DX12)
    static_assert(false, "Type of renderer is not selected");
#endif
}

void cg::renderer::renderer::move_forward(float delta) {
    camera->set_position(camera->get_position() + camera->get_direction() * delta * frame_duration);
}

void cg::renderer::renderer::move_backward(float delta) {
    camera->set_position(camera->get_position() - camera->get_direction() * delta * frame_duration);
}

void cg::renderer::renderer::move_left(float delta) {
    camera->set_position(camera->get_position() - camera->get_right() * delta * frame_duration);
}

void cg::renderer::renderer::move_right(float delta) {
    camera->set_position(camera->get_position() + camera->get_right() * delta * frame_duration);
}

void cg::renderer::renderer::move_yaw(float delta) {
    camera->set_theta(camera->get_theta() + delta);
}

void cg::renderer::renderer::move_pitch(float delta) {
    camera->set_phi(camera->get_phi() + delta);
}

void cg::renderer::renderer::load_model() {
    // TODO Lab: 1.03 Adjust `cg::renderer::rasterization_renderer` and `cg::renderer::renderer` classes to consume
    // `cg::world::model`
}

void cg::renderer::renderer::load_camera() {
    // TODO Lab: 1.04 Setup an instance of camera `cg::world::camera` class in `cg::renderer::renderer` and
    // `cg::renderer::rasterization_renderer`
}
