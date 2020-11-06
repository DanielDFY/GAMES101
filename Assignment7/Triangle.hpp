#pragma once

#include <cassert>
#include <array>

#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "OBJ_Loader.h"
#include "Object.hpp"

class Triangle : public Object, public std::enable_shared_from_this<Triangle> {
public:
    Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2,
        std::shared_ptr<Material> material_ptr = std::make_shared<Material>());

    Triangle(const Triangle& rhs) = default;

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray) override;
    Bounding_box bound() const override;
    float area() const override { return _area; }
    [[nodiscard]] std::optional<Sample> sample() override;
    bool emitting() const override { return _mat_ptr->emitting(); }

private:
    Vector3f _v0, _v1, _v2; // vertices in counter-clockwise order
    Vector3f _e1, _e2;      // 2 edges v1-v0, v2-v0;
    Vector2f _t0, _t1, _t2; // texture coords
    Vector3f _normal;
    float _area;
    std::shared_ptr<Material> _mat_ptr;
};

[[nodiscard]] std::vector<std::shared_ptr<Triangle>> load_triangles_from_model_file(const std::string& file_name, const std::shared_ptr<Material>& mat_ptr);

class Triangle_mesh : public Object {
public:
    Triangle_mesh(const std::vector<std::shared_ptr<Triangle>>& triangle_ptr_list,
                BVH_tree::Split_method split_method = BVH_tree::Split_method::NAIVE);

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray) override { return _bvh_tree.intersect(ray);}
    Bounding_box bound() const override { return _bvh_tree.bound(); }
    float area() const override { return _bvh_tree.area(); }
    [[nodiscard]] std::optional<Sample> sample() override { return _bvh_tree.sample(); }
    bool emitting() const override { return _emitting; }

    [[nodiscard]] std::vector<std::shared_ptr<Triangle>> triangles() const { return _triangle_ptrs; }

private:
    std::vector<std::shared_ptr<Triangle>> _triangle_ptrs;
    BVH_tree _bvh_tree;
    bool _emitting;
};