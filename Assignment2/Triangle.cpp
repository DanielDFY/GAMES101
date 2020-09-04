#include "Triangle.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

Triangle::Triangle() {
    v[0] << 0,0,0;
    v[1] << 0,0,0;
    v[2] << 0,0,0;

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

bool Triangle::inside_triangle(float x, float y) const {
    const Eigen::Vector3f point(x, y, 1.0f);
    const auto p0p1_cross_p0p = (v[1] - v[0]).cross(point - v[0]);
    const auto p1p2_cross_p1p = (v[2] - v[1]).cross(point - v[1]);
    const auto p2p0_cross_p2p = (v[0] - v[2]).cross(point - v[2]);

    return (p0p1_cross_p0p.dot(p1p2_cross_p1p) > 0) && (p1p2_cross_p1p.dot(p2p0_cross_p2p) > 0);
}

std::tuple<float, float, float> Triangle::compute_barycentric(float x, float y) const {
    const auto c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    const auto c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    const auto c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}