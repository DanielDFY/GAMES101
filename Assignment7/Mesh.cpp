#include "Mesh.hpp"

#include <cassert>
#include <array>

std::vector<std::shared_ptr<Triangle>> load_triangles_from_model_file(const std::string& file_name, const std::shared_ptr<Material>& mat_ptr) {
    objl::Loader loader;
    loader.LoadFile(file_name);

    assert(loader.LoadedMeshes.size() == 1);
    const objl::Mesh mesh = loader.LoadedMeshes[0];

    std::vector<std::shared_ptr<Triangle>> triangle_ptrs;

    const auto vertex_num = mesh.Vertices.size();
    for (size_t i = 0; i < vertex_num; i += 3) {
        std::array<Vector3f, 3> triangle_vertices;
        for (size_t j = 0; j < 3; ++j) {
            const auto idx = i + j;
            triangle_vertices[j] = {
                mesh.Vertices[idx].Position.X,
                mesh.Vertices[idx].Position.Y,
                mesh.Vertices[idx].Position.Z
            };
        }

        triangle_ptrs.push_back(std::make_shared<Triangle>(triangle_vertices[0],
                                triangle_vertices[1],
                                triangle_vertices[2],
                                mat_ptr));
    }

    return triangle_ptrs;
}

TriangleMesh::TriangleMesh(const std::vector<std::shared_ptr<Triangle>>& triangle_ptr_list,
                            BVH_tree::SplitMethod split_method)
     : _triangle_ptrs(triangle_ptr_list),
       _bvh_tree(transform_to_object_vector<Triangle>(triangle_ptr_list), split_method),
       _emitting(false) {
    for (const auto& tri_ptr : _triangle_ptrs) {
        if (tri_ptr->emitting()) {
            _emitting = true;
            break;
        }
    }
}