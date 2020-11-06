#pragma once

#include <algorithm>

constexpr float PI = 3.141592653589793f;    // pi
constexpr float INV_PI = 1 / PI;            // 1 / pi
constexpr float INV_2PI = INV_PI / 2;       // 1 / (2 * pi)
constexpr float EPSILON = 0.00001f;

inline float degree_to_rad(float degree) { return degree * PI / 180.0f; }

constexpr auto FLOAT_INFINITY = std::numeric_limits<float>::max();
constexpr auto FLOAT_LOWEST = std::numeric_limits<float>::lowest();

inline float clamp(float lo, float hi, float v) { return std::max(lo, std::min(hi, v)); }

bool solve_quadratic(float a, float b, float c, float& x0, float& x1);

float get_random_float();

void update_progress(float progress);
