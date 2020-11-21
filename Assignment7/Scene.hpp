#pragma once

#include <vector>

#include "Object.hpp"
#include "BVH.hpp"
#include "Ray.hpp"

class Scene {
public:
    Scene() = default;
    Scene(unsigned int w, unsigned int h) : _width(w), _height(h) {}
    Scene(unsigned int w, unsigned int h, const Vector3f& eye_pos, float fov)
        : _width(w), _height(h), _eye_pos(eye_pos), _fov(fov) {
    }

    unsigned int width() const { return _width; }
    unsigned int height() const { return _height; }
    Vector3f eye_pos() const { return _eye_pos; }
    float fov() const { return _fov; }
    Vector3f background_color() const { return _background_color; }
    float russian_roulette() const { return _russian_roulette; }

    [[nodiscard]] std::vector<std::shared_ptr<Object>> objects() const { return _obj_ptrs; }

    void add_object(const std::shared_ptr<Object>& obj_ptr) { if (obj_ptr != nullptr) _obj_ptrs.push_back(obj_ptr); }

    void build_BVH();
    void build_SVH();

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray, Culling culling) const;
    [[nodiscard]] std::optional<Sample> sample_light_sources() const;

private:
    unsigned int  _width = 1280;
    unsigned int  _height = 960;
    Vector3f _eye_pos = { 0.0f, 0.0f, 0.0f };
    float _fov = 90.0f;
    Vector3f _background_color = { 0.0f, 0.0f, 0.0f };
    float _russian_roulette = 0.8f;

    std::vector<std::shared_ptr<Object>> _obj_ptrs;

    std::unique_ptr<BVH_tree> _bvh_tree_ptr;
};