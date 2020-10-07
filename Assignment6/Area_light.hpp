#pragma once

#include "Light.hpp"
#include "Vector.hpp"
#include "Utility.hpp"

class Area_light : public Light {
public:
    Area_light(const Vector3f& pos, const Vector3f& inten);

    [[nodiscard]] Vector3f random_sample_point() const;

public:
    float length;
    Vector3f normal;
    Vector3f u;
    Vector3f v;
};
