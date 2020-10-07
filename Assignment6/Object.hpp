#pragma once

#include <optional>

#include "Utility.hpp"
#include "Bounding_box.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Object {
public:
    Object() = default;
    virtual ~Object() = default;

    virtual std::optional<Intersection> intersect(const Ray& ray) = 0;
    virtual Bounding_box bound() const = 0;
};