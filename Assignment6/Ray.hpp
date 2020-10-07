#pragma once

#include <ostream>

#include "Vector.hpp"

struct Ray {
    // destination = origin + t * direction
    Vector3f ori;
    Vector3f dir, inv_dir;
    float t;    // transportation time,
    float t_min, t_max;

    Ray(const Vector3f& origin, const Vector3f& direction, float time = 0.0f);

    [[nodiscard]] Vector3f at_time(float time) const {return ori + time * dir;}
};

std::ostream& operator<<(std::ostream& os, const Ray& r);