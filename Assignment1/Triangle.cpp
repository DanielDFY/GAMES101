#include "Triangle.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

Triangle::Triangle() {
    v[0] << 0, 0, 0;
    v[1] << 0, 0, 0;
    v[2] << 0, 0, 0;

    color[0] << 0.0, 0.0, 0.0;
    color[1] << 0.0, 0.0, 0.0;
    color[2] << 0.0, 0.0, 0.0;
}

void Triangle::set_vertex(int ind, Eigen::Vector3f ver) { v[ind] = ver; }

void Triangle::set_normal(int ind, Vector3f n) { normal[ind] = n; }

void Triangle::set_color(int ind, float r, float g, float b) {
    if ((r < 0.0f) || (r > 255.0f) || (g < 0.0f) || (g > 255.0f) || (b < 0.0f) || (b > 255.0f)) {
        throw std::runtime_error("Invalid color values");
    }

    color[ind] = {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f
    };
}

std::array<Vector4f, 3> Triangle::to_vector4() const {
    std::array<Vector4f, 3> res;
    std::transform(std::begin(v), std::end(v), res.begin(), [](auto& vec) {
        return Vector4f(vec.x(), vec.y(), vec.z(), 1.f);
    });
    return res;
}
