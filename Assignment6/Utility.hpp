#pragma once

#include <iostream>
#include <cmath>

#undef M_PI
#define M_PI 3.141592653589793f

constexpr float EPSILON = 0.00001f;

inline float degree_to_rad(float degree) { return degree * M_PI / 180.0f; }

constexpr float FLOAT_INFINITY = std::numeric_limits<float>::max();
constexpr float FLOAT_LOWEST = std::numeric_limits<float>::lowest();

inline float clamp(float lo, float hi, float v) { return std::max(lo, std::min(hi, v)); }

bool solve_quadratic(float a, float b, float c, float& x0, float& x1);

float get_random_float();

void update_progress(float progress);
