#include "raytracer_renderer.h"

#include "linalg.h"
#include "renderer/raytracer/raytracer.h"
#include "resource.h"
#include "settings.h"
#include "utils/resource_utils.h"
#include "utils/timer.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace cg::renderer {

namespace {

constexpr auto miss_shader = [](const ray& ray) {
    payload payload{};
    payload.color = {0, 0, (ray.direction.y + 1) / static_cast<float>(2)};
    return payload;
};

constexpr auto shadow_miss_shader = [](const ray& /*ray*/) {
    payload payload{};
    payload.t = -1;
    return payload;
};

constexpr auto shadow_any_hit_shader = [](const ray& /*ray*/, payload& payload, const triangle<vertex>& /*triangle*/) {
    return payload;
};

} // namespace

ray_tracing_renderer::ray_tracing_renderer(std::shared_ptr<cg::settings> settings)
    : renderer{std::move(settings)}, render_target{std::make_shared<cg::resource<cg::unsigned_color>>(
                                         renderer::settings->width, renderer::settings->height)} {}

void ray_tracing_renderer::init() {
    renderer::load_model();
    renderer::load_camera();

    auto closest_hit_shader = [&](const ray& ray,
                                  payload& payload,
                                  const triangle<vertex>& triangle,
                                  std::size_t /*depth*/) {
        float3 position = ray.position + payload.t * ray.direction;
        float3 normal = linalg::normalize(payload.bary.x * triangle.na + payload.bary.y * triangle.nb +
                                          payload.bary.z * triangle.nc);

        float3 result_color = triangle.emissive;

        for (const light& light : lights) {
            cg::renderer::ray to_light{position, light.position - position};
            auto shadow_payload = shadow_raytracer->trace_ray(to_light, 1, linalg::length(light.position - position));
            if (shadow_payload.t < 0) {
                result_color += triangle.diffuse * light.color * std::max(linalg::dot(normal, to_light.direction), 0.F);
            }
        }

        payload.color = color::from_float3(result_color);
        return payload;
    };

    static constexpr float3 light_pos = float3{0, 1.58F, -0.03F};
    static constexpr float3 light_dir = float3{0.78F, 0.78F, 0.78F};
    lights.push_back({light_pos, light_dir});

    using RayTracer = cg::renderer::raytracer<cg::vertex, cg::unsigned_color>;
    raytracer = std::make_shared<RayTracer>(RayTracer{settings->width,
                                                      settings->height,
                                                      miss_shader,
                                                      closest_hit_shader,
                                                      nullptr,
                                                      render_target,
                                                      model->get_vertex_buffers(),
                                                      model->get_index_buffers(),
                                                      {}});
    shadow_raytracer = std::make_shared<RayTracer>(RayTracer{settings->width,
                                                             settings->height,
                                                             shadow_miss_shader,
                                                             nullptr,
                                                             shadow_any_hit_shader,
                                                             render_target,
                                                             model->get_vertex_buffers(),
                                                             model->get_index_buffers(),
                                                             {}});
}

void ray_tracing_renderer::destroy() {}

void ray_tracing_renderer::update() {}

void ray_tracing_renderer::render() {
    raytracer->clear_render_target({0, 0, 0});
    raytracer->build_acceleration_structure();
    shadow_raytracer->build_acceleration_structure();

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
    // TODO Lab: 2.04 Define `any_hit_shader` and `miss_shader` for `shadow_raytracer`
    // TODO Lab: 2.04 Adjust `closest_hit_shader` of `raytracer` to cast shadows rays and to ignore occluded lights
    // TODO Lab: 2.05 Adjust `ray_tracing_renderer` class to build the acceleration structure
    // TODO Lab: 2.06 (Bonus) Adjust `closest_hit_shader` for Monte-Carlo light tracing
}

} // namespace cg::renderer
