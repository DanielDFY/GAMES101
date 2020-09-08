#pragma once

#include <eigen3/Eigen/Eigen>

#include "Texture.hpp"

struct Fragment_shader_payload {
    Fragment_shader_payload() = default;

    Fragment_shader_payload(const Eigen::Vector3f& col, const Eigen::Vector3f& nor,const Eigen::Vector2f& tc, std::shared_ptr<Texture> tp) :
         color(col), normal(nor), tex_coords(tc), tex_ptr(tp) {}

    Eigen::Vector3f view_pos;
    Eigen::Vector3f color;
    Eigen::Vector3f normal;
    Eigen::Vector2f tex_coords;
    std::shared_ptr<Texture> tex_ptr = nullptr;
};

struct Vertex_shader_payload {
    Eigen::Vector3f position;
};