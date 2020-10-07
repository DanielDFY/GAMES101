#include "Scene.hpp"

void Scene::build_BVH() {
    std::cout << " - Generating BVH for Scene..." << std::endl;
    _bvh_tree = BVH_tree(_obj_ptrs, BVH_tree::Split_method::NAIVE);
}

void Scene::build_SVH() {
    std::cout << " - Generating BVH for Scene with SAH..." << std::endl;
    _bvh_tree = BVH_tree(_obj_ptrs, BVH_tree::Split_method::SAH);
}

std::optional<Intersection> Scene::intersect(const Ray& ray) const {
    return _bvh_tree.intersect(ray);
}