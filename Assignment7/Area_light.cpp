#include "Area_light.hpp"

#include "Utility.hpp"

Area_light::Area_light(const Vector3f &pos, const Vector3f &inten)
    : Light(pos, inten) {}

Vector3f Area_light::random_sample_point() const {
    const auto random_u = get_random_float();
    const auto random_v = get_random_float();
    return position + random_u * u + random_v * v;
}