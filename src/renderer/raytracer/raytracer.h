#pragma once

#include "resource.h"

#include <linalg.h>
#include <omp.h>

#include <cstddef>
#include <memory>

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
inline triangle<VB>::triangle(const VB& vertex_a, const VB& vertex_b, const VB& vertex_c) {
    // TODO Lab: 2.02 Implement a constructor of `triangle` struct
}

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
    raytracer(std::size_t width, std::size_t height);

    void set_render_target(std::shared_ptr<resource<RT>> in_render_target);
    void clear_render_target(const RT& in_clear_value);
    void set_viewport(std::size_t in_width, std::size_t in_height);

    void set_vertex_buffers(std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers);
    void set_index_buffers(std::vector<std::shared_ptr<cg::resource<unsigned int>>> in_index_buffers);
    void set_acceleration_structures(std::vector<aabb<VB>> in_acceleration_structures);
    void build_acceleration_structure();

    void set_miss_shader(std::function<payload(const ray& ray)> miss_shader);
    void set_closest_hit_shader(
        std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle, std::size_t depth)>
            closest_hit_shader);
    void set_any_hit_shader(
        std::function<payload(const ray& ray, payload& payload, const triangle<VB>& triangle)> any_hit_shader);

    void ray_generation(
        float3 position, float3 direction, float3 right, float3 up, std::size_t depth, std::size_t accumulation_num);

    [[nodiscard]] payload
    trace_ray(const ray& ray, std::size_t depth, float max_t = 1000.f, float min_t = 0.001f) const; // NOLINT

    payload intersection_shader(const triangle<VB>& triangle, const ray& ray) const;

    float2 get_jitter(int frame_id);

  protected:
    // NOLINTBEGIN(*non-private*)
    std::shared_ptr<cg::resource<RT>> render_target;
    std::shared_ptr<cg::resource<float3>> history;
    std::vector<std::shared_ptr<cg::resource<unsigned int>>> index_buffers;
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
inline raytracer<VB, RT>::raytracer(std::size_t width, std::size_t height) : width{width}, height{height} {};

template <typename VB, typename RT>
inline void raytracer<VB, RT>::set_render_target(std::shared_ptr<resource<RT>> in_render_target) {
    // TODO Lab: 2.01 Implement `set_render_target`, `set_viewport`, and `clear_render_target` methods of `raytracer`
    // class
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::set_viewport(std::size_t in_width, std::size_t in_height) {
    // TODO Lab: 2.01 Implement `set_render_target`, `set_viewport`, and `clear_render_target` methods of `raytracer`
    // class
    // TODO Lab: 2.06 Add `history` resource in `raytracer` class
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::clear_render_target(const RT& in_clear_value) {
    // TODO Lab: 2.01 Implement `set_render_target`, `set_viewport`, and `clear_render_target` methods of `raytracer`
    // class
    // TODO Lab: 2.06 Add `history` resource in `raytracer` class
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::set_vertex_buffers(std::vector<std::shared_ptr<cg::resource<VB>>> in_vertex_buffers) {
    // TODO Lab: 2.02 Implement `set_vertex_buffers` and `set_index_buffers` of `raytracer` class
}

template <typename VB, typename RT>
void raytracer<VB, RT>::set_index_buffers(std::vector<std::shared_ptr<cg::resource<unsigned int>>> in_index_buffers) {
    // TODO Lab: 2.02 Implement `set_vertex_buffers` and `set_index_buffers` of `raytracer` class
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::build_acceleration_structure() {
    // TODO Lab: 2.02 Fill `triangles` vector in `build_acceleration_structure` of `raytracer` class
    // TODO Lab: 2.05 Implement `build_acceleration_structure` method of `raytracer` class
}

template <typename VB, typename RT>
inline void raytracer<VB, RT>::ray_generation(
    float3 position, float3 direction, float3 right, float3 up, std::size_t depth, std::size_t accumulation_num) {
    // TODO Lab: 2.01 Implement `ray_generation` and `trace_ray` method of `raytracer` class
    // TODO Lab: 2.06 Implement TAA in `ray_generation` method of `raytracer` class
}

template <typename VB, typename RT>
inline payload raytracer<VB, RT>::trace_ray(const ray& ray, std::size_t depth, float max_t, float min_t) const {
    // TODO Lab: 2.01 Implement `ray_generation` and `trace_ray` method of `raytracer` class
    // TODO Lab: 2.02 Adjust `trace_ray` method of `raytracer` class to traverse geometry and call a closest hit shader
    // TODO Lab: 2.04 Adjust `trace_ray` method of `raytracer` to use `any_hit_shader`
    // TODO Lab: 2.05 Adjust `trace_ray` method of `raytracer` class to traverse the acceleration structure
    return payload{};
}

template <typename VB, typename RT>
inline payload raytracer<VB, RT>::intersection_shader(const triangle<VB>& triangle, const ray& ray) const {
    // TODO Lab: 2.02 Implement an `intersection_shader` method of `raytracer` class
    return payload{};
}

template <typename VB, typename RT>
float2 raytracer<VB, RT>::get_jitter(int frame_id) {
    // TODO Lab: 2.06 Implement `get_jitter` method of `raytracer` class
}

template <typename VB>
inline void aabb<VB>::add_triangle(const triangle<VB>& triangle) {
    // TODO Lab: 2.05 Implement `aabb` class
}

template <typename VB>
inline const std::vector<triangle<VB>>& aabb<VB>::get_triangles() const {
    // TODO Lab: 2.05 Implement `aabb` class
}

template <typename VB>
inline bool aabb<VB>::aabb_test(const ray& ray) const {
    // TODO Lab: 2.05 Implement `aabb` class
}

} // namespace cg::renderer
