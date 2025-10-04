#pragma once

#include <linalg.h>
#ifdef DX12
#include <DirectXMath.h>
#endif

namespace cg::world {

using namespace linalg::aliases;

class camera {
  public:
    camera(float width, float height);

    void set_position(float3 in_position);
    void set_theta(float in_theta);
    void set_phi(float in_phi);

    void set_angle_of_view(float in_aov);
    void set_z_near(float in_z_near);
    void set_z_far(float in_z_far);

    [[nodiscard]] float4x4 get_view_matrix() const;
    [[nodiscard]] float4x4 get_projection_matrix() const;

#ifdef DX12
    const DirectX::XMMATRIX get_dxm_view_matrix() const;
    const DirectX::XMMATRIX get_dxm_projection_matrix() const;
    const DirectX::XMMATRIX get_dxm_mvp_matrix() const;
#endif
    [[nodiscard]] float3 get_position() const;
    [[nodiscard]] float3 get_direction() const;
    [[nodiscard]] float3 get_right() const;
    [[nodiscard]] float3 get_up() const;
    [[nodiscard]] float get_theta() const;
    [[nodiscard]] float get_phi() const;

  protected:
    // NOLINTBEGIN(*-non-private-*)
    float3 position;
    float theta;
    float phi;

    float width;
    float height;
    float aspect_ratio;
    float angle_of_view;
    float z_near;
    float z_far;
    // NOLINTEND(*-non-private-*)
};
} // namespace cg::world
