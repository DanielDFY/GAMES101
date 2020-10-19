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
    float russian_roulette() const { return _russian_roulette; }

    [[nodiscard]] std::vector<std::shared_ptr<Object>> objects() const { return _obj_ptrs; }

    void add_object(const std::shared_ptr<Object>& obj_ptr) { if (obj_ptr != nullptr) _obj_ptrs.push_back(obj_ptr); }

    void build_BVH();
    void build_SVH();

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray) const;
    [[nodiscard]] std::optional<Sample> sample_light_sources() const;

private:
    int _width = 1280;
    int _height = 960;
    float _fov = 40.0f;
    Vector3f _background_color = { 0.0f, 0.0f, 0.0f };
    float _russian_roulette = 0.8f;

    std::vector<std::shared_ptr<Object>> _obj_ptrs;

    BVH_tree _bvh_tree;
};