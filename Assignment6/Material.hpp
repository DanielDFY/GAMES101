#pragma once

#include "Vector.hpp"

enum class Material_type { DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };

class Material {
public:
    Material() : type(Material_type::DIFFUSE_AND_GLOSSY), color(1.0f, 1.0f, 1.0f), emission(0.0f, 0.0f, 0.0f),
                ior(0.0f), kd(0.0f), ks(0.0f), specular_exponent(0.0f) {}
                
    Material(Material_type t,const Vector3f& c,const Vector3f& e, float ior_value, float kd_value, float ks_value, float specular_exp)
             : type(t), color(c), emission(e), ior(ior_value), kd(kd_value), ks(ks_value), specular_exponent(specular_exp) {}

public:
    Material_type type;
    Vector3f color;
    Vector3f emission;
    float ior;
    float kd, ks;
    float specular_exponent;
};

Vector3f reflect(const Vector3f& ray_dir, const Vector3f& normal);

Vector3f refract(const Vector3f& ray_dir, const Vector3f& normal, float ior);

float fresnel(const Vector3f& ray_dir, const Vector3f& normal, float ior);