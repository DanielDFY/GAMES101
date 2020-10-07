#include "Area_light.hpp"

Area_light::Area_light(const Vector3f &pos, const Vector3f &inten)
    : Light(pos, inten),
      length(100.0f),
      normal({0.0f, -1.0f, 0.0f}),
      u({1.0f, 0.0f, 0.0f}),
      v({1.0f, 0.0f, 0.0f}) {}

Vector3f Area_light::random_sample_point() const {
    const auto random_u = get_random_float();
    const auto random_v = get_random_float();
    return position + random_u * u + random_v * v;
}