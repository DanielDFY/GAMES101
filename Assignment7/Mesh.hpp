#pragma once

#include "OBJ_Loader.h"

#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "Object.hpp"

[[nodiscard]] std::vector<std::shared_ptr<Triangle>> load_triangles_from_model_file(const std::string& file_name, const std::shared_ptr<Material>& mat_ptr);

class TriangleMesh : public Object {
public:
    TriangleMesh(const std::vector<std::shared_ptr<Triangle>>& triangle_ptr_list,
                BVH_tree::SplitMethod split_method = BVH_tree::SplitMethod::NAIVE);

    float area() const override { return _bvh_tree.area(); }
    BoundingBox bound() const override { return _bvh_tree.bound(); }
    bool emitting() const override { return _emitting; }

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray, Culling culling) override { return _bvh_tree.intersect(ray, culling); }
    [[nodiscard]] std::optional<Sample> sample() override { return _bvh_tree.sample(); }
    [[nodiscard]] std::vector<std::shared_ptr<Triangle>> triangles() const { return _triangle_ptrs; }

private:
    std::vector<std::shared_ptr<Triangle>> _triangle_ptrs;
    BVH_tree _bvh_tree;
    bool _emitting;
};