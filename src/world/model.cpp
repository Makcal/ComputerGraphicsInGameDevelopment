#define TINYOBJLOADER_IMPLEMENTATION
#include "model.h"
#include "resource.h"

#include "utils/error_handler.h"

#include <linalg.h>

#include <cstddef>
#include <memory>
#include <unordered_map>

using namespace linalg::aliases;
using namespace cg::world;

void cg::world::model::load_obj(const std::filesystem::path& model_path) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = model_path.parent_path().string();
    reader_config.triangulate = true;

    tinyobj::ObjReader reader{};
    if (!reader.ParseFromFile(model_path.string(), reader_config)) {
        if (!reader.Error().empty())
            THROW_ERROR(reader.Error());
    }

    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const tinyobj::attrib_t& attributes = reader.GetAttrib();
    const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

    allocate_buffers(shapes);
    fill_buffers(shapes, attributes, materials, model_path.parent_path());
}

void model::allocate_buffers(const std::vector<tinyobj::shape_t>& shapes) {
    for (const tinyobj::shape_t& shape : shapes) {
        std::size_t index_offset = 0;
        std::size_t vertex_buffer_size = 0;
        std::size_t index_buffer_size = 0;
        std::unordered_map<int3, std::size_t> index_map;
        const tinyobj::mesh_t& mesh = shape.mesh;

        for (const unsigned char& fv : mesh.num_face_vertices) {
            for (std::size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = mesh.indices[index_offset + v]; // indices is flattened
                int3 idx_tuple = {idx.vertex_index, idx.normal_index, idx.texcoord_index};
                if (auto it = index_map.find(idx_tuple); it == index_map.end()) {
                    it = index_map.emplace(idx_tuple, vertex_buffer_size).first;
                    ++vertex_buffer_size;
                }
                ++index_buffer_size;
            }
            index_offset += fv;
        }
        vertex_buffers.push_back(std::make_shared<resource<vertex>>(vertex_buffer_size));
        index_buffers.push_back(std::make_shared<resource<std::size_t>>(index_buffer_size));
    }
    textures.resize(shapes.size());
}

float3
cg::world::model::compute_normal(const tinyobj::attrib_t& attrib, const tinyobj::mesh_t& mesh, size_t index_offset) {
    const tinyobj::index_t& a_id = mesh.indices[index_offset]; // get face
    const tinyobj::index_t& b_id = mesh.indices[index_offset + 1];
    const tinyobj::index_t& c_id = mesh.indices[index_offset + 2];

    float3 a{attrib.vertices[3L * a_id.vertex_index],
             attrib.vertices[(3L * a_id.vertex_index) + 1],
             attrib.vertices[(3L * a_id.vertex_index) + 2]};
    float3 b{attrib.vertices[3L * b_id.vertex_index],
             attrib.vertices[(3L * b_id.vertex_index) + 1],
             attrib.vertices[(3L * b_id.vertex_index) + 2]};
    float3 c{attrib.vertices[3L * c_id.vertex_index],
             attrib.vertices[(3L * c_id.vertex_index) + 1],
             attrib.vertices[(3L * c_id.vertex_index) + 2]};

    return linalg::normalize(linalg::cross(b - a, c - a));
}

void model::fill_vertex_data(cg::vertex& vertex,              // vector object
                             const tinyobj::attrib_t& attrib, // data arrays
                             const tinyobj::index_t idx,      // where to find data about this vertex
                             const float3& computed_normal,
                             const tinyobj::material_t& material) {
    vertex.v = {attrib.vertices[3L * idx.vertex_index],
                attrib.vertices[(3L * idx.vertex_index) + 1],
                attrib.vertices[(3L * idx.vertex_index) + 2]};

    if (idx.normal_index < 0)
        vertex.n = computed_normal;
    else {
        vertex.n = {attrib.normals[3L * idx.normal_index],
                    attrib.normals[(3L * idx.normal_index) + 1],
                    attrib.normals[(3L * idx.normal_index) + 2]};
    }

    if (idx.texcoord_index >= 0) {
        vertex.tex = {
            attrib.texcoords[2L * idx.texcoord_index],
            attrib.texcoords[(2L * idx.texcoord_index) + 1],
        };
    }

    vertex.ambient = {material.ambient[0], material.ambient[1], material.ambient[2]};
    vertex.diffuse = {material.diffuse[0], material.diffuse[1], material.diffuse[2]};
    vertex.emissive = {material.emission[0], material.emission[1], material.emission[2]};
}

void model::fill_buffers(const std::vector<tinyobj::shape_t>& shapes,
                         const tinyobj::attrib_t& attrib,
                         const std::vector<tinyobj::material_t>& materials,
                         const std::filesystem::path& base_folder) {
    for (std::size_t shape_i = 0; shape_i < shapes.size(); ++shape_i) {
        const tinyobj::shape_t& shape = shapes[shape_i];
        std::shared_ptr<cg::resource<vertex>>& vertex_buffer = vertex_buffers[shape_i];
        std::shared_ptr<cg::resource<std::size_t>>& index_buffer = index_buffers[shape_i];
        const tinyobj::mesh_t& mesh = shape.mesh;

        std::size_t index_offset = 0;
        std::unordered_map<int3, std::size_t> index_map;
        std::size_t index_buffer_id = 0;
        std::size_t vertex_buffer_id = 0;

        for (std::size_t face_i = 0; face_i < mesh.num_face_vertices.size(); ++face_i) {
            unsigned char fv = mesh.num_face_vertices[face_i];

            float3 normal;
            if (mesh.indices[index_offset].normal_index < 0)
                normal = compute_normal(attrib, mesh, index_offset);

            for (std::size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = mesh.indices[index_offset + v]; // indices is flattened
                int3 idx_tuple = {idx.vertex_index, idx.normal_index, idx.texcoord_index};

                if (auto it = index_map.find(idx_tuple); it == index_map.end()) {
                    it = index_map.emplace(idx_tuple, vertex_buffer_id).first;

                    vertex& vertex = vertex_buffer->item(vertex_buffer_id);
                    const tinyobj::material_t& material = materials[mesh.material_ids[face_i]];
                    fill_vertex_data(vertex, attrib, idx, normal, material);

                    ++vertex_buffer_id;
                }
                index_buffer->item(index_buffer_id) = index_map[idx_tuple];
                ++index_buffer_id;
            }
            index_offset += fv;
        }
        if (!materials[mesh.material_ids[0]].diffuse_texname.empty()) {
            textures[shape_i] = base_folder / materials[mesh.material_ids[0]].diffuse_texname;
        }
    }
    textures.resize(shapes.size());
}

const std::vector<std::shared_ptr<cg::resource<cg::vertex>>>& cg::world::model::get_vertex_buffers() const {
    return vertex_buffers;
}

const std::vector<std::shared_ptr<cg::resource<std::size_t>>>& cg::world::model::get_index_buffers() const {
    return index_buffers;
}

const std::vector<std::filesystem::path>& cg::world::model::get_per_shape_texture_files() const {
    return textures;
}

const float4x4 cg::world::model::get_world_matrix() const {
    return float4x4{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
}
