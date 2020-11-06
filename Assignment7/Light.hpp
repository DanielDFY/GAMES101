#pragma once

#include "Vector.hpp"

class Light {
public:
    Light(const Vector3f& pos, const Vector3f& inten)
     : position(pos), intensity(inten) {}

    Vector3f position;
    Vector3f intensity;
};
