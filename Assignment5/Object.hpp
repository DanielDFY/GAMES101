#pragma once

#include "Utility.hpp"

enum class Material_type {
    DIFFUSE_AND_GLOSSY,
    REFLECTION_AND_REFRACTION,
    REFLECTION
};

class Object {
public:
    Object()
        : material_type(Material_type::DIFFUSE_AND_GLOSSY)
        , ior(1.3f)
        , kd(0.8f)
        , ks(0.2f)
        , diffuse_color(0.2f, 0.2f, 0.2f)
        , specular_exponent(25)
    {}

    virtual ~Object() = default;

    virtual bool intersect(const Eigen::Vector3f&, const Eigen::Vector3f&, float&, uint32_t&, Eigen::Vector2f&) const = 0;

    virtual void get_surface_properties(const Eigen::Vector3f&, const Eigen::Vector3f&, uint32_t, const Eigen::Vector2f&,
                                    Eigen::Vector3f&, Eigen::Vector2f&) const = 0;

    virtual Eigen::Vector3f eval_diffuse_color(const Eigen::Vector2f&) const {
        return diffuse_color;
    }

    // material properties
    Material_type material_type;
    float ior;
    float kd, ks;
    Eigen::Vector3f diffuse_color;
    int specular_exponent;
};
