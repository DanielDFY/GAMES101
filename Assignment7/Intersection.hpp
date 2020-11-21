#pragma once

#include <memory>

#include "Object.hpp"
#include "Material.hpp"
#include "Math.hpp"

class Object;

struct Intersection {
    float time = FLOAT_INFINITY;
    Vector3f pos = { 0.0f };
    Vector3f normal = { 0.0f };
    Vector2f uv = { 0.0f };
    std::shared_ptr<Object> obj_ptr = nullptr;
    std::shared_ptr<Material> mat_ptr = nullptr;
};