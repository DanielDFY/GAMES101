#pragma once

#include "Object.hpp"

class Sphere : public Object {
public:
    Sphere(const Eigen::Vector3f& c, float r)
        : center(c)
        , radius(r)
        , radius_squared(r * r)
    {}

    bool intersect(const Eigen::Vector3f& ori, const Eigen::Vector3f& dir, float& t_near, uint32_t&, Eigen::Vector2f&) const override;

    void get_surface_properties(const Eigen::Vector3f& p, const Eigen::Vector3f&, uint32_t, const Eigen::Vector2f&,
                            Eigen::Vector3f& normal, Eigen::Vector2f&) const override {
        normal = (p - center).normalized();
    }

    Eigen::Vector3f center;
    float radius, radius_squared;
};
