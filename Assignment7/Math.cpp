#include "Math.hpp"

#include <random>
#include <iostream>

std::optional<std::tuple<float, float>> solve_quadratic(float a, float b, float c) {
    float x0, x1;

    const auto delta = b * b - 4 * a * c;
    if (delta < 0) {
        return std::nullopt;
    } else if (delta == 0) {
        x0 = x1 = -0.5f * b / a;
    } else {
        const auto delta_sqrt = sqrtf(delta);
        const auto q = (b > 0) ? -0.5f * (b + delta_sqrt) : -0.5f * (b - delta_sqrt);
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        std::swap(x0, x1);

    return { {x0, x1} };
}

float get_random_float() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    return dist(rng);
}

Vector3f Vector3f::normalized() const {
    const auto mag_sq = magnitude_squared();
    if (mag_sq > 0) {
        const auto inv_mag = 1 / sqrtf(mag_sq);
        return { x * inv_mag, y * inv_mag, z * inv_mag };
    } else {
        return { 0.0f, 0.0f, 0.0f };
    }
}

Vector3f local_to_world(const Vector3f& local_dir, const Vector3f& normal) {
    Vector3f t;
    if (std::fabs(normal.x) > std::fabs(normal.y)) {
        const auto inv_len = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
        t = { normal.z * inv_len, 0.0f, -normal.x * inv_len };
    } else {
        const auto inv_len = 1.0f / std::sqrt(normal.y * normal.y + normal.z * normal.z);
        t = { 0.0f, normal.z * inv_len, -normal.y * inv_len };
    }
    const auto b = t.cross(normal);
    return local_dir.x * b + local_dir.y * t + local_dir.z * normal;
}

void update_progress(float progress) {
    constexpr int barWidth = 70;

    std::cout << "\r[";
    const auto pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0f) << " %";
    std::cout.flush();
}