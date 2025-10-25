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

auto make_closest_hit_shader(const std::vector<light>& lights) {
    return [&lights](const ray& ray, payload& payload, const triangle<vertex>& triangle, std::size_t /*depth*/) {
        float3 position = ray.position + payload.t * ray.direction;
        float3 normal = linalg::normalize(payload.bary.x * triangle.na + payload.bary.y * triangle.nb +
                                          payload.bary.z * triangle.nc);

        float3 result_color = triangle.emissive;

        for (const light& light : lights) {
            cg::renderer::ray to_light{position, light.position - position};
            result_color += triangle.diffuse * light.color * std::max(linalg::dot(normal, to_light.direction), 0.F);
        }

        payload.color = color::from_float3(result_color);
        return payload;
    };
}

// constexpr auto any_hit_shader = [](const ray& /*ray*/, payload& payload, const triangle<vertex>& triangle) {};
constexpr auto any_hit_shader = nullptr;

} // namespace

struct RayTracerInitializer {
    using RayTracer = cg::renderer::raytracer<cg::vertex, cg::unsigned_color>;

    ray_tracing_renderer& renderer; // NOLINT(*ref-data*)

    operator RayTracer() { // NOLINT(*explicit*)
        return {renderer.settings->width,
                renderer.settings->height,
                miss_shader,
                make_closest_hit_shader(renderer.lights),
                any_hit_shader,
                renderer.render_target,
                renderer.model->get_vertex_buffers(),
                renderer.model->get_index_buffers(),
                {}};
    }
};

ray_tracing_renderer::ray_tracing_renderer(std::shared_ptr<cg::settings> settings)
    : renderer{std::move(settings)}, render_target{std::make_shared<cg::resource<cg::unsigned_color>>(
                                         renderer::settings->width, renderer::settings->height)} {}

void ray_tracing_renderer::init() {
    renderer::load_model();
    renderer::load_camera();

    raytracer = std::make_shared<RayTracerInitializer::RayTracer>(RayTracerInitializer{*this});

    static constexpr float3 light_pos = float3{0, 1.58F, -0.03F};
    static constexpr float3 light_dir = float3{0.78F, 0.78F, 0.78F};
    lights.push_back({light_pos, light_dir});
    // TODO Lab: 2.04 Initialize `shadow_raytracer` in `ray_tracing_renderer`
}

void ray_tracing_renderer::destroy() {}

void ray_tracing_renderer::update() {}

void ray_tracing_renderer::render() {
    raytracer->clear_render_target({0, 0, 0});
    raytracer->build_acceleration_structure();

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
