#pragma once

#include <memory>

#include "Object.hpp"
#include "Material.hpp"
#include "Utility.hpp"

class Object;
class Sphere;

struct Intersection {
    float time = FLOAT_INFINITY;
    Vector3f pos = {0.0f};
    Vector3f normal = {0.0f};
    Vector2f uv = {0.0f};
    std::shared_ptr<Object> obj_ptr = nullptr;
    std::shared_ptr<Material> mat_ptr = std::make_shared<Material>();
};