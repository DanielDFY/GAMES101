#pragma once

#include <cmath>
#include <iostream>
#include <random>

#include <eigen3/Eigen/Eigen>

constexpr float FLOAT_INFINITY = std::numeric_limits<float>::max();

inline Eigen::Vector3f lerp(const Eigen::Vector3f& a, const Eigen::Vector3f& b, float t) {
    return a * (1.0f - t) + b * t;
}

inline float clamp(const float& lo, const float& hi, const float& v) {
    return std::max(lo, std::min(hi, v));
}

inline bool solve_quadratic(float a, float b, float c, float& x0, float& x1) {
    const auto delta = b * b - 4 * a * c;
    const auto delta_sqrt = sqrtf(delta);
    if (delta < 0) {
        return false;
    } else if (delta == 0) {
        x0 = x1 = -0.5f * b / a;
    } else {
        const auto q = (b > 0) ? -0.5f * (b + delta_sqrt) : -0.5f * (b - delta_sqrt);
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        std::swap(x0, x1);
    
    return true;
}

inline float get_random_float()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f); // distribution in range [1, 6]

    return dist(rng);
}

inline float deg_to_rad(float deg) { return deg * M_PI / 180.0f; }

inline void update_progress(float progress) {
    constexpr int barWidth = 70;

    std::cout << "[";
    const auto pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0f) << " %\r";
    std::cout.flush();
}
