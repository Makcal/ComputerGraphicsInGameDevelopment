#pragma once

#include "resource.h"

#include <iostream>
#include <linalg.h>
#if __has_include(<omp.h>)
#include <omp.h>
#endif

#include <cstddef>
#include <memory>
#include <utility>

namespace cg::renderer {

using namespace linalg::aliases;

struct ray {
    ray(float3 position, float3 direction) : position(position) {
        this->direction = normalize(direction);
    }
    float3 position;
    float3 direction;
};

struct payload { // NOLINT(*member-init)
    float t;
    float3 bary;
    cg::color color;
};

template <typename VB>
struct triangle {
    triangle(const VB& vertex_a, const VB& vertex_b, const VB& vertex_c);

    float3 a;
    float3 b;
    float3 c;

    float3 ba;
    float3 ca;

    float3 na;
    float3 nb;
    float3 nc;

    float3 ambient;
    float3 diffuse;
    float3 emissive;
};

template <typename VB>
inline triangle<VB>::triangle(const VB& vertex_a, const VB& vertex_b, const VB& vertex_c)
    : a(vertex_a.v), b(vertex_b.v), c(vertex_c.v), ba{b - a}, ca{c - a}, na{vertex_a.n}, nb{vertex_b.n}, nc{vertex_c.n},
      ambient{vertex_a.ambient}, diffuse{vertex_a.diffuse}, emissive{vertex_a.emissive} {}

template <typename VB>
class aabb {
  public:
    void add_triangle(const triangle<VB>& triangle);
    const std::vector<triangle<VB>>& get_triangles() const;
    [[nodiscard]] bool aabb_test(const ray& ray) const;

  protected:
    // NOLINTBEGIN(*non-private*)
    std::vector<triangle<VB>> triangles;

    float3 aabb_min;
    float3 aabb_max;
    // NOLINTEND(*non-private*)
};

struct light {
    float3 position;
    float3 color;
};

template <typename VB, typename RT>
class raytracer {
  public:
    raytracer(std::size_t width,
              std::size_t height,
              std::function<payload(const ray& ray)> miss_shader,
              std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle, std::size_t depth)>
                  closest_hit_shader,
              std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle)> any_hit_shader,
              std::shared_ptr<resource<RT>> in_render_target,
              std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers,
              std::vector<std::shared_ptr<cg::resource<std::size_t>>> in_index_buffers,
              std::vector<aabb<VB>> in_acceleration_structures);

    void clear_render_target(const RT& in_clear_value);

    void build_acceleration_structure();

    void ray_generation(
        float3 position, float3 direction, float3 right, float3 up, std::size_t depth, std::size_t accumulation_num);

    [[nodiscard]] payload
    trace_ray(const ray& ray, std::size_t depth, float max_t = 1000.f, float min_t = 0.001f) const; // NOLINT

    payload intersection_shader(const triangle<VB>& triangle, const ray& ray) const;

    float2 get_jitter(std::size_t frame_id);

  protected:
    // NOLINTBEGIN(*non-private*)
    std::shared_ptr<cg::resource<RT>> render_target;
    std::shared_ptr<cg::resource<float3>> history;
    std::vector<std::shared_ptr<cg::resource<std::size_t>>> index_buffers;
    std::vector<std::shared_ptr<cg::resource<VB>>> vertex_buffers;
    std::vector<triangle<VB>> triangles;

    std::size_t width;
    std::size_t height;

    std::vector<aabb<VB>> acceleration_structures;

    std::function<payload(const ray& ray)> miss_shader = nullptr;
    std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle, std::size_t depth)>
        closest_hit_shader = nullptr;
    std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle)> any_hit_shader = nullptr;
    // NOLINTEND(*non-private*)
};

template <typename VB, typename RT>
inline raytracer<VB, RT>::raytracer(
    std::size_t width,
    std::size_t height,
    std::function<payload(const ray& ray)> miss_shader,
    std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle, std::size_t depth)>
        closest_hit_shader,
    std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle)> any_hit_shader,
    std::shared_ptr<resource<RT>> in_render_target,
    std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers,
    std::vector<std::shared_ptr<cg::resource<std::size_t>>> in_index_buffers,
    std::vector<aabb<VB>> in_acceleration_structures)
    : width{width}, height{height}, history{std::make_shared<resource<float3>>(width, height)},
      miss_shader{std::move(miss_shader)}, closest_hit_shader{std::move(closest_hit_shader)},
      any_hit_shader{std::move(any_hit_shader)}, render_target{std::move(in_render_target)},
      vertex_buffers{std::move(in_vertex_buffers)}, index_buffers{std::move(in_index_buffers)},
      acceleration_structures{std::move(in_acceleration_structures)} {};

template <typename VB, typename RT>
inline void raytracer<VB, RT>::clear_render_target(const RT& in_clear_value) {
    for (std::size_t i = 0; i < render_target->count(); i++) {
        render_target->item(i) = in_clear_value;
        history->item(i) = float3{0.F};
    }
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::build_acceleration_structure() {
    for (std::size_t s = 0; s < index_buffers.size(); s++) {
        auto& index_buffer = index_buffers[s];
        auto& vertex_buffer = vertex_buffers[s];
        aabb<VB> aabb;
        for (std::size_t i = 0; i < index_buffer->count();) {
            triangle<VB> triangle{vertex_buffer->item(index_buffer->item(i++)),
                                  vertex_buffer->item(index_buffer->item(i++)),
                                  vertex_buffer->item(index_buffer->item(i++))};
            aabb.add_triangle(triangle);
        }
        acceleration_structures.push_back(std::move(aabb));
    }
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::ray_generation(const float3 position,
                                              const float3 direction,
                                              const float3 right,
                                              const float3 up,
                                              const std::size_t depth,
                                              const std::size_t accumulation_num) {
    const float frame_weight = 1 / static_cast<float>(accumulation_num);
    for (std::size_t frame_id = 0; frame_id < accumulation_num; frame_id++) {
        std::cout << "Tracing frame #" << frame_id + 1 << '/' << accumulation_num << '\n';
        const float2 jitter = get_jitter(frame_id);

#pragma omp parallel for default(shared) // NOLINT(*use-default-none)
        for (std::size_t x = 0; x < width; x++) {
            for (std::size_t y = 0; y < height; y++) {
                float u = ((2 * static_cast<float>(x) + jitter.x) / static_cast<float>(width - 1)) - 1;
                float v = ((2 * static_cast<float>(y) + jitter.y) / static_cast<float>(height - 1)) - 1;
                u *= static_cast<float>(width) / static_cast<float>(height);
                float3 ray_direction = direction + u * right - v * up;

                ray ray{position, ray_direction};
                payload payload = trace_ray(ray, depth);

                float3& history_pixel = history->item(x, y);
                history_pixel += linalg::sqrt(payload.color.to_float3() * frame_weight);

                if (frame_id + 1 == accumulation_num)
                    render_target->item(x, y) = RT::from_float3(history_pixel);
            }
        }
    }
}

template <typename VB, typename RT>
inline payload raytracer<VB, RT>::trace_ray(const ray& ray, std::size_t depth, float max_t, float min_t) const {
    if (depth == 0)
        return miss_shader(ray);
    --depth;

    payload closest_hit_payload{};
    closest_hit_payload.t = max_t;
    const triangle<VB>* closest_triangle = nullptr;

    for (const aabb<VB>& aabb : acceleration_structures) {
        if (!aabb.aabb_test(ray))
            continue;
        for (const triangle<VB>& triangle : aabb.get_triangles()) {
            payload payload = intersection_shader(triangle, ray);
            if (payload.t > min_t && payload.t < closest_hit_payload.t) {
                if (any_hit_shader)
                    return any_hit_shader(ray, payload, triangle);
                closest_hit_payload = payload;
                closest_triangle = &triangle;
            }
        }
    }

    if (closest_hit_payload.t < max_t)
        return closest_hit_shader(ray, closest_hit_payload, *closest_triangle, depth);

    return miss_shader(ray);
}

template <typename VB, typename RT>
inline payload raytracer<VB, RT>::intersection_shader(const triangle<VB>& triangle, const ray& ray) const {
    using linalg::dot;

    payload payload{};
    payload.t = -1;

    float3 pvec = cross(ray.direction, triangle.ca);
    float det = dot(triangle.ba, pvec);

    static constexpr float kPrecision = 1e-8;
    if (-kPrecision < det && det < kPrecision)
        return payload;

    float inv_det = 1 / det;
    float3 tvec = ray.position - triangle.a;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0 || u > 1)
        return payload;

    float3 qvec = cross(tvec, triangle.ba);
    float v = dot(ray.direction, qvec) * inv_det;
    if (v < 0 || u + v > 1)
        return payload;

    payload.t = dot(triangle.ca, qvec) * inv_det;
    payload.bary = {1 - u - v, u, v};

    return payload;
}

template <typename VB, typename RT>
float2 raytracer<VB, RT>::get_jitter(std::size_t frame_id) {
    float2 result{0, 0};
    const int base_x = 2;
    std::size_t index = frame_id + 1;
    float inv_base = 1.F / base_x;
    float fraction = inv_base;
    while (index > 0) {
        result.x += static_cast<float>(index % base_x) * fraction;
        index /= base_x;
        fraction += inv_base;
    }
    const int base_y = 3;
    index = frame_id + 1;
    inv_base = 1.F / base_y;
    fraction = inv_base;
    while (index > 0) {
        result.y += static_cast<float>(index % base_y) * fraction;
        index /= base_y;
        fraction += inv_base;
    }
    return result - 1.F / 2;
}

template <typename VB>
inline void aabb<VB>::add_triangle(const triangle<VB>& triangle) {
    if (triangles.empty()) {
        aabb_max = triangle.a;
        aabb_min = triangle.a;
    }
    triangles.push_back(triangle);
    aabb_max = linalg::max(aabb_max, triangle.a);
    aabb_max = linalg::max(aabb_max, triangle.b);
    aabb_max = linalg::max(aabb_max, triangle.c);
    aabb_min = linalg::min(aabb_min, triangle.a);
    aabb_min = linalg::min(aabb_min, triangle.b);
    aabb_min = linalg::min(aabb_min, triangle.c);
}

template <typename VB>
inline const std::vector<triangle<VB>>& aabb<VB>::get_triangles() const {
    return triangles;
}

template <typename VB>
inline bool aabb<VB>::aabb_test(const ray& ray) const {
    float3 inv_ray_direction = float3(1) / ray.direction;
    float3 t0 = (aabb_max - ray.position) * inv_ray_direction;
    float3 t1 = (aabb_min - ray.position) * inv_ray_direction;
    float3 tmax = linalg::max(t0, t1);
    float3 tmin = linalg::min(t0, t1);
    return linalg::maxelem(tmin) <= linalg::minelem(tmax);
}

} // namespace cg::renderer
