#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>

namespace cg {

struct settings {
    static std::shared_ptr<settings> parse_settings(int argc, char** argv);

    std::size_t height;
    std::size_t width;

    std::filesystem::path model_path;

    std::vector<float> camera_position;
    float camera_theta;
    float camera_phi;
    float camera_angle_of_view;
    float camera_z_near;
    float camera_z_far;

    std::filesystem::path result_path;

    std::size_t raytracing_depth;
    std::size_t accumulation_num;

    std::filesystem::path shader_path;
};

} // namespace cg
