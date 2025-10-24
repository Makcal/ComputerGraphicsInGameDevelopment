#include "raytracer_renderer.h"

#include "renderer/raytracer/raytracer.h"
#include "resource.h"
#include "settings.h"
#include "utils/resource_utils.h"
#include "utils/timer.h"

#include <memory>
#include <utility>

cg::renderer::ray_tracing_renderer::ray_tracing_renderer(std::shared_ptr<cg::settings> settings)
    : renderer{std::move(settings)} {}

void cg::renderer::ray_tracing_renderer::init() {
    this->render_target = std::make_shared<resource<unsigned_color>>(settings->width, settings->height);
    raytracer = std::make_shared<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>>(
        settings->width, settings->height, render_target);

    renderer::load_model();
    renderer::load_camera();
    // TODO Lab: 2.03 Add light information to `lights` array of `ray_tracing_renderer`
    // TODO Lab: 2.04 Initialize `shadow_raytracer` in `ray_tracing_renderer`
}

void cg::renderer::ray_tracing_renderer::destroy() {}

void cg::renderer::ray_tracing_renderer::update() {}

void cg::renderer::ray_tracing_renderer::render() {
    raytracer->clear_render_target({0, 0, 0});

    raytracer->set_miss_shader([](const ray& ray) {
        payload payload{};
        payload.color = {0, 0, (ray.direction.y + 1) / static_cast<float>(2)};
        return payload;
    });

    {
        utils::timer timer{"Ray generation"};

        raytracer->ray_generation(camera->get_position(),
                                  camera->get_direction(),
                                  camera->get_right(),
                                  camera->get_up(),
                                  settings->raytracing_depth,
                                  settings->accumulation_num);
    }

    utils::save_resource(*render_target, settings->result_path);
    // TODO Lab: 2.02 Add `closest_hit_shader` to `raytracer` class to return diffuse color
    // TODO Lab: 2.03 Adjust `closest_hit_shader` of `raytracer` to implement Lambertian shading model
    // TODO Lab: 2.04 Define `any_hit_shader` and `miss_shader` for `shadow_raytracer`
    // TODO Lab: 2.04 Adjust `closest_hit_shader` of `raytracer` to cast shadows rays and to ignore occluded lights
    // TODO Lab: 2.05 Adjust `ray_tracing_renderer` class to build the acceleration structure
    // TODO Lab: 2.06 (Bonus) Adjust `closest_hit_shader` for Monte-Carlo light tracing
}
