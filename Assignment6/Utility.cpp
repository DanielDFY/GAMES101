#include "Utility.hpp"

#include <random>

bool solve_quadratic(float a, float b, float c, float& x0, float& x1) {
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

float get_random_float() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.1f, 1.0f);

    return dist(rng);
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