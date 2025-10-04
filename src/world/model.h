#pragma once

#include "resource.h"

#include <linalg.h>
#include <tiny_obj_loader.h>

#include <cstddef>
#include <filesystem>

namespace cg::world {

using namespace linalg::aliases;

class model {
  public:
    model() = default;

    void load_obj(const std::filesystem::path& model_path);

    [[nodiscard]] const std::vector<std::shared_ptr<cg::resource<cg::vertex>>>& get_vertex_buffers() const;
    [[nodiscard]] const std::vector<std::shared_ptr<cg::resource<std::size_t>>>& get_index_buffers() const;
    [[nodiscard]] const std::vector<std::filesystem::path>& get_per_shape_texture_files() const;

    [[nodiscard]] const float4x4 get_world_matrix() const;

  protected:
    // NOLINTBEGIN(*-non-private-*)
    std::vector<std::shared_ptr<cg::resource<cg::vertex>>> vertex_buffers;
    std::vector<std::shared_ptr<cg::resource<std::size_t>>> index_buffers;
    std::vector<std::filesystem::path> textures;
    // NOLINTEND(*-non-private-*)

    void allocate_buffers(const std::vector<tinyobj::shape_t>& shapes);
    static float3 compute_normal(const tinyobj::attrib_t& attrib, const tinyobj::mesh_t& mesh, size_t index_offset);
    static void fill_vertex_data(cg::vertex& vertex,
                                 const tinyobj::attrib_t& attrib,
                                 tinyobj::index_t idx,
                                 const float3& computed_normal,
                                 const tinyobj::material_t& material);
    void fill_buffers(const std::vector<tinyobj::shape_t>& shapes,
                      const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::material_t>& materials,
                      const std::filesystem::path& base_folder);
};
} // namespace cg::world
