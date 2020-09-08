#pragma once

#include <memory>

#include <eigen3/Eigen/Eigen>

#include "Texture.hpp"

class Triangle {
public:
    Eigen::Vector4f v[3]; // the original coordinates of the triangle, v0, v1, v2 in counter clockwise order
    
    // Per vertex values
    Eigen::Vector3f normal[3];     // normal vector at each vertex
    Eigen::Vector3f color[3];      // color at each vertex;
    Eigen::Vector2f tex_coords[3]; // texture coordinates at for each vertex
    
    std::shared_ptr<Texture> tex_ptr = nullptr;

    Triangle();

    Eigen::Vector4f a() const { return v[0]; }
    Eigen::Vector4f b() const { return v[1]; }
    Eigen::Vector4f c() const { return v[2]; }

    void set_vertex(int ind, Eigen::Vector4f ver); // set coordinates of i-th vertex
    void set_normal(int ind, Eigen::Vector3f n);   // set normal vector of i-th vertex
    void set_color(int ind, float r, float g, float b); // set color of i-th vertex
    void set_tex_coord(int ind, Eigen::Vector2f uv);    // set texture coordinates of i-th vertex

    void set_normals(const std::array<Eigen::Vector3f, 3>& normals);
    void set_colors(const std::array<Eigen::Vector3f, 3>& colors);

    Eigen::Vector3f get_color() const { return color[0] * 255.0f; } // only one color per triangle.

    // Check if the point (x, y) is inside the triangle in 2d space
    bool inside_triangle_2d(float x, float y) const;

    // Compute the barycentric coordinates of the point (x, y) in the triangle in 2d space
    std::tuple<float, float, float> compute_barycentric_2d(float x, float y) const;
};

// Check if the point (x, y) is inside the triangle represented by v[0], v[1], v[2] in 2d space
bool inside_triangle_2d(float x, float y, std::array<Eigen::Vector4f, 3> v);

// Compute the barycentric coordinates of the point (x, y) in the triangle represented by v[0], v[1], v[2] in 2d space
std::tuple<float, float, float> compute_barycentric_2d(float x, float y, std::array<Eigen::Vector4f, 3> v);