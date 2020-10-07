#pragma once

#include <cmath>
#include <iostream>
#include <random>
#include <utility>

#include <eigen3/Eigen/Eigen>

constexpr float FLOAT_INFINITY = std::numeric_limits<float>::max();

inline Eigen::Vector3f lerp(const Eigen::Vector3f& a, const Eigen::Vector3f& b, float t) {
    return a * (1.0f - t) + b * t;
}

inline float clamp(float lo, float hi, float v) {
    return std::max(lo, std::min(hi, v));
}

bool solve_quadratic(float a, float b, float c, float& x0, float& x1);

float get_random_float();

inline float deg_to_rad(float deg) { return deg * M_PI / 180.0f; }

void update_progress(float progress);