#include "Triangle.hpp"

#include <algorithm>
#include <array>

Triangle::Triangle() {
    v[0] << 0.0f, 0.0f, 0.0f, 1.0f;
    v[1] << 0.0f, 0.0f, 0.0f, 1.0f;
    v[2] << 0.0f, 0.0f, 0.0f, 1.0f;

    normal[0] << 0.0f, 0.0f, 0.0f;
    normal[1] << 0.0f, 0.0f, 0.0f;
    normal[2] << 0.0f, 0.0f, 0.0f;
    
    color[0] << 0.0f, 0.0f, 0.0f;
    color[1] << 0.0f, 0.0f, 0.0f;
    color[2] << 0.0f, 0.0f, 0.0f;

    tex_coords[0] << 0.0f, 0.0f;
    tex_coords[1] << 0.0f, 0.0f;
    tex_coords[2] << 0.0f, 0.0f;
}

void Triangle::set_vertex(int ind, Eigen::Vector4f ver) { v[ind] = ver; }

void Triangle::set_normal(int ind, Eigen::Vector3f n) { normal[ind] = n; }

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

void Triangle::set_tex_coord(int ind, Eigen::Vector2f uv) { tex_coords[ind] = uv; }

void Triangle::set_normals(const std::array<Eigen::Vector3f, 3>& normals) {
    normal[0] = normals[0];
    normal[1] = normals[1];
    normal[2] = normals[2];
}

void Triangle::set_colors(const std::array<Eigen::Vector3f, 3>& colors) {
    set_color(0, colors[0][0], colors[0][1], colors[0][2]);
    set_color(1, colors[1][0], colors[1][1], colors[1][2]);
    set_color(2, colors[2][0], colors[2][1], colors[2][2]);
}

bool Triangle::inside_triangle_2d(float x, float y) const {
    const Eigen::Vector3f point(x, y, 1.0f);
    const std::array<Eigen::Vector3f, 3> v3 = {
        Eigen::Vector3f(v[0].x(), v[0].y(), 1.0f),
        Eigen::Vector3f(v[1].x(), v[1].y(), 1.0f),
        Eigen::Vector3f(v[2].x(), v[2].y(), 1.0f),
    };
    const auto p0p1_cross_p0p = (v3[1] - v3[0]).cross(point - v3[0]);
    const auto p1p2_cross_p1p = (v3[2] - v3[1]).cross(point - v3[1]);
    const auto p2p0_cross_p2p = (v3[0] - v3[2]).cross(point - v3[2]);

    return (p0p1_cross_p0p.dot(p1p2_cross_p1p) > 0) && (p1p2_cross_p1p.dot(p2p0_cross_p2p) > 0);
}

std::tuple<float, float, float> Triangle::compute_barycentric_2d(float x, float y) const {
    const auto c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    const auto c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    const auto c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

bool inside_triangle_2d(float x, float y, std::array<Eigen::Vector4f, 3> v) {
    const Eigen::Vector3f point(x, y, 1.0f);
    const std::array<Eigen::Vector3f, 3> v3 = {
        Eigen::Vector3f(v[0].x(), v[0].y(), 1.0f),
        Eigen::Vector3f(v[1].x(), v[1].y(), 1.0f),
        Eigen::Vector3f(v[2].x(), v[2].y(), 1.0f),
    };
    const auto p0p1_cross_p0p = (v3[1] - v3[0]).cross(point - v3[0]);
    const auto p1p2_cross_p1p = (v3[2] - v3[1]).cross(point - v3[1]);
    const auto p2p0_cross_p2p = (v3[0] - v3[2]).cross(point - v3[2]);

    return (p0p1_cross_p0p.dot(p1p2_cross_p1p) > 0) && (p1p2_cross_p1p.dot(p2p0_cross_p2p) > 0);
}

std::tuple<float, float, float> compute_barycentric_2d(float x, float y, std::array<Eigen::Vector4f, 3> v) {
    const auto c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    const auto c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    const auto c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}