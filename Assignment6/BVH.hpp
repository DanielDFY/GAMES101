#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <ctime>
#include <optional>

#include "Object.hpp"
#include "Ray.hpp"
#include "Bounding_box.hpp"
#include "Intersection.hpp"

struct BVH_node {
    Bounding_box bound;
    std::shared_ptr<BVH_node> left_ptr;
    std::shared_ptr<BVH_node> right_ptr;
    std::shared_ptr<Object> obj_ptr;

    BVH_node() : bound(), left_ptr(nullptr), right_ptr(nullptr), obj_ptr(nullptr) { }
};

class BVH_tree {
public:
    enum class Split_method { 
        NAIVE,  // do bisection based on the axis that brings the max extent.
        SAH     // do partition with Surface Area Heuristic
    };

    BVH_tree();
    BVH_tree(std::vector<std::shared_ptr<Object>>& obj_ptrs,
            Split_method split_method = Split_method::NAIVE);
    BVH_tree(std::vector<std::shared_ptr<Object>>&& obj_ptrs,
            Split_method split_method = Split_method::NAIVE);

    [[nodiscard]] std::optional<Intersection> intersect(const Ray &ray) const;

    [[nodiscard]] Bounding_box bound() const;

    [[nodiscard]] Split_method split_method() const { return _split_method; }

private:
    [[nodiscard]] std::shared_ptr<BVH_node> recursive_build(std::vector<std::shared_ptr<Object>>& obj_ptr_list, size_t start, size_t end);

    [[nodiscard]] std::optional<Intersection> intersect(const std::shared_ptr<BVH_node>& node_ptr, const Ray& ray) const;

private:
    std::shared_ptr<BVH_node> _root_ptr;
    Split_method _split_method;
};