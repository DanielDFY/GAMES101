#pragma once

#include <vector>

#include "Object.hpp"
#include "Light.hpp"
#include "BVH.hpp"
#include "Ray.hpp"

class Scene {
public:
    Scene() = default;
    Scene(int w, int h) : _width(w), _height(h) {}

    int width() const { return _width; }
    int height() const { return _height; }
    float fov() const { return _fov; }
    Vector3f background_color() const { return _background_color; }
    int max_depth() const { return _max_depth; }

    [[nodiscard]] const std::vector<std::shared_ptr<Object>> objects() const { return _obj_ptrs; }
    [[nodiscard]] const std::vector<std::shared_ptr<Light>> lights() const { return _light_ptrs; }

    void add_object(std::shared_ptr<Object> obj_ptr) { _obj_ptrs.push_back(obj_ptr); }
    void add_light(std::shared_ptr<Light> light_ptr) { _light_ptrs.push_back(light_ptr); }

    void build_BVH();
    void build_SVH();

    std::optional<Intersection> intersect(const Ray& ray) const;

private:
    int _width = 1280;
    int _height = 960;
    float _fov = 90.0f;
    Vector3f _background_color = { 0.235294f, 0.67451f, 0.843137f };
    int _max_depth = 5;

    std::vector<std::shared_ptr<Object>> _obj_ptrs;
    std::vector<std::shared_ptr<Light>> _light_ptrs;

    BVH_tree _bvh_tree;
};