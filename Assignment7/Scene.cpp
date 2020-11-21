#include "Scene.hpp"

#include <iostream>

void Scene::build_BVH() {
    std::cout << " - Generating BVH for Scene..." << std::endl;
    _bvh_tree_ptr = std::make_unique<BVH_tree>(_obj_ptrs, BVH_tree::SplitMethod::NAIVE);
}

void Scene::build_SVH() {
    std::cout << " - Generating BVH for Scene with SAH..." << std::endl;
    _bvh_tree_ptr = std::make_unique<BVH_tree>(_obj_ptrs, BVH_tree::SplitMethod::SAH);
}

std::optional<Intersection> Scene::intersect(const Ray& ray, Culling culling) const {
    return _bvh_tree_ptr->intersect(ray, culling);
}

std::optional<Sample> Scene::sample_light_sources() const {
    float total_emitting_area = 0.0f;
    for (const auto& obj_ptr : _obj_ptrs) {
        if (obj_ptr->emitting()) {
            total_emitting_area += obj_ptr->area();
        }
    }

    const auto p = get_random_float();
    auto threshold = p * total_emitting_area;

    float current_emitting_area_sum = 0.0f;
    for (const auto& obj_ptr : _obj_ptrs) {
        if (obj_ptr->emitting()) {
            const auto obj_area = obj_ptr->area();
            current_emitting_area_sum += obj_area;
            if (current_emitting_area_sum >= threshold) {
                auto s = obj_ptr->sample();
                if (s) {
                    return s;
                } else {
                    current_emitting_area_sum -= obj_area;
                    total_emitting_area -= obj_area;
                    threshold = p * total_emitting_area;
                }
            }
        }
    }

    return std::nullopt;
}