#pragma once

#include "Light.hpp"
#include "Vector.hpp"

class Area_light : public Light {
public:
    Area_light(const Vector3f& pos, const Vector3f& inten);

    [[nodiscard]] Vector3f random_sample_point() const;

public:
    float length = 100.0f;
    Vector3f normal = { 0.0f, -1.0f, 0.0f };
    Vector3f u = { 1.0f, 0.0f, 0.0f };
    Vector3f v = { 0.0f, 0.0f, 1.0f };
};
