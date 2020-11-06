#pragma once

#include "Vector.hpp"
#include "Utility.hpp"

enum class Material_type { DIFFUSE, REFLECTION_AND_REFRACTION, REFLECTION };

class Material {
public:
    Material() : _type(Material_type::DIFFUSE), _emission(0.0f, 0.0f, 0.0f),
                 _ior(0.0f), _kd(0.0f), _ks(0.0f), _specular_exponent(0.0f) {}
                
    Material(Material_type t, const Vector3f& e, float ior_value, Vector3f kd_value, Vector3f ks_value, float specular_exp)
             : _type(t), _emission(e), _ior(ior_value), _kd(kd_value), _ks(ks_value), _specular_exponent(specular_exp) {}

    inline Material_type type() { return _type; }

    inline bool emitting() { return _emission.magnitude() > EPSILON; }

    inline Vector3f emission(float u, float v) { return _emission; }

    // Given the direction of an incident ray and a normal vector,
    // calculate a random ray-out direction with some kind of sample distribution.
    [[nodiscard]] Vector3f sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal);

    // Given the directions of the ray in and ray out and a normal vector,
    // calculate its value of PDF (probability distribution function).
    float pdf(const Vector3f& ray_in_dir, const Vector3f& ray_out_dir, const Vector3f& normal);

    // Given the directions of the ray in and ray out and a normal vector,
    // calculate its contribution from BRDF (bidirectional reflectance distribution function).
    Vector3f contribution(const Vector3f& ray_in_dir, const Vector3f& ray_out_dir, const Vector3f& normal);

public:
    Material_type _type;
    Vector3f _emission;
    float _ior;
    Vector3f _kd, _ks;
    float _specular_exponent;
};

// Compute reflection direction
Vector3f reflect(const Vector3f& ray_dir, const Vector3f& normal);

// Compute refraction direction using Snell's law
Vector3f refract(const Vector3f& ray_dir, const Vector3f& normal, float ior);

// Compute Fresnel equation
float fresnel(const Vector3f& ray_dir, const Vector3f& normal, float ior);