#pragma once

#include <eigen3/Eigen/Eigen>

using namespace Eigen;

class Triangle {
  public:
    Vector3f v[3]; // the original coordinates of the triangle, v0, v1, v2 in counter clockwise order
    
    // Per vertex values
    Vector3f color[3];      // color at each vertex;
    Vector3f normal[3];     // normal vector for each vertex

    Triangle();

    Eigen::Vector3f a() const { return v[0]; }
    Eigen::Vector3f b() const { return v[1]; }
    Eigen::Vector3f c() const { return v[2]; }

    void set_vertex(int ind, Vector3f ver); // set coordinates of i-th vertex
    void set_normal(int ind, Vector3f n);   // set normal vector of i-th vertex
    void set_color(int ind, float r, float g, float b); // set color of i-th vertex

    std::array<Vector4f, 3> to_vector4() const;
};