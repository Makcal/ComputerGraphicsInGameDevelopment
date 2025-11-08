#define _USE_MATH_DEFINES
#include "linalg.h"

#include "camera.h"

#include <cmath>

using namespace cg::world;

const float kToRadians = std::acos(-1.F) / 180; // NOLINT(*err58*)

cg::world::camera::camera(float width, float height)
    : theta(0.F), phi(0.F), width(width), height(height), aspect_ratio(width / height), angle_of_view(1.04719F),
      z_near(0.001F), z_far(100), position({0, 0, 0}) {} // NOLINT(*magic-number*)

void cg::world::camera::set_position(float3 in_position) {
    position = in_position;
}

void cg::world::camera::set_theta(float in_theta) {
    theta = in_theta * kToRadians;
}

void cg::world::camera::set_phi(float in_phi) {
    phi = in_phi * kToRadians;
}

void cg::world::camera::set_angle_of_view(float in_aov) {
    angle_of_view = in_aov * kToRadians;
}

void cg::world::camera::set_z_near(float in_z_near) {
    z_near = in_z_near;
}

void cg::world::camera::set_z_far(float in_z_far) {
    z_far = in_z_far;
}

float4x4 cg::world::camera::get_view_matrix() const {
    float3 up{0, 1, 0};
    float3 eye = position + get_direction();

    float3 z_axis = linalg::normalize(position - eye);
    float3 x_axis = linalg::normalize(linalg::cross(up, z_axis));
    float3 y_axis = linalg::normalize(linalg::cross(z_axis, x_axis));

    using linalg::dot;
    return {
        {x_axis.x, y_axis.x, z_axis.x, 0},
        {x_axis.y, y_axis.y, z_axis.y, 0},
        {x_axis.z, y_axis.z, z_axis.z, 0},
        {-dot(x_axis, position), -dot(y_axis, position), -dot(z_axis, position), 1},
    };
}

#ifdef DX12
const DirectX::XMMATRIX cg::world::camera::get_dxm_view_matrix() const {
    DirectX::XMFLOAT3 eye_position{position.x, position.y, position.z};
    DirectX::XMFLOAT3 eye_direction{get_direction().x, get_direction().y, get_direction().z};
    DirectX::XMFLOAT3 up_direction{get_up().x, get_up().y, get_up().z};
    return DirectX::XMMatrixLookToRH(DirectX::XMLoadFloat3(&eye_position),
                                     DirectX::XMLoadFloat3(&eye_direction),
                                     DirectX::XMLoadFloat3(&up_direction));
}

const DirectX::XMMATRIX cg::world::camera::get_dxm_projection_matrix() const {
    return DirectX::XMMatrixPerspectiveFovRH(angle_of_view, aspect_ratio, z_near, z_far);
}

const DirectX::XMMATRIX camera::get_dxm_mvp_matrix() const {
    return get_dxm_view_matrix() * get_dxm_projection_matrix();
}
#endif

float4x4 cg::world::camera::get_projection_matrix() const {
    float f = 1 / tanf(angle_of_view / 2);
    return {
        {f / aspect_ratio, 0, 0, 0},
        {0, f, 0, 0},
        {0, 0, z_far / (z_near - z_far), -1},
        {0, 0, (z_far * z_near) / (z_near - z_far), 0},
    };
}

float3 cg::world::camera::get_position() const {
    return position;
}

float3 cg::world::camera::get_direction() const {
    return {
        // from spherical
        std::sin(theta) * std::cos(phi),
        std::sin(phi),
        -std::cos(theta) * std::cos(phi),
    };
}

float3 cg::world::camera::get_right() const {
    return linalg::cross(get_direction(), {0, 1, 0});
}

float3 cg::world::camera::get_up() const {
    return linalg::cross(get_right(), get_direction());
}

float camera::get_theta() const {
    return theta;
}

float camera::get_phi() const {
    return phi;
}
