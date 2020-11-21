#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "Object.hpp"
#include "Ray.hpp"
#include "BoundingBox.hpp"
#include "Intersection.hpp"

struct BVH_node {
    BoundingBox bound;
    float area;
    std::unique_ptr<BVH_node> left_ptr;
    std::unique_ptr<BVH_node> right_ptr;
    std::shared_ptr<Object> obj_ptr;

    BVH_node() : bound(), area(0.0f), left_ptr(nullptr), right_ptr(nullptr), obj_ptr(nullptr) { }
};

class BVH_tree {
public:
    enum class SplitMethod { 
        NAIVE,  // do bisection based on the axis that brings the max extent.
        SAH     // do partition with Surface Area Heuristic
    };

    BVH_tree();
    BVH_tree(std::vector<std::shared_ptr<Object>>& obj_ptrs,
            SplitMethod split_method = SplitMethod::NAIVE);
    BVH_tree(std::vector<std::shared_ptr<Object>>&& obj_ptrs,
            SplitMethod split_method = SplitMethod::NAIVE);
    BVH_tree(BVH_tree&& rhs) noexcept : _root_ptr(std::move(rhs._root_ptr)), _split_method(rhs._split_method) {};

    float area() const { return _root_ptr == nullptr ? 0.0f : _root_ptr->area; };
    BoundingBox bound() const { return _root_ptr == nullptr ? bound() : _root_ptr->bound; }
    SplitMethod split_method() const { return _split_method; }

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray, Culling culling) const;
    [[nodiscard]] std::optional<Sample> sample() const;

private:
    [[nodiscard]] std::unique_ptr<BVH_node> recursive_build(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end);
    [[nodiscard]] std::unique_ptr<BVH_node> naive_partition(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end, size_t obj_span);
    [[nodiscard]] std::unique_ptr<BVH_node> sah_partition(std::vector<std::shared_ptr<Object>>& obj_ptrs, size_t start, size_t end);

    [[nodiscard]] std::optional<Intersection> intersect(const std::unique_ptr<BVH_node>& node_ptr, const Ray& ray, Culling culling) const;
    [[nodiscard]] std::optional<Sample> sample(const std::unique_ptr<BVH_node>& node_ptr, float threshold) const;

private:
    std::unique_ptr<BVH_node> _root_ptr;
    SplitMethod _split_method;
};