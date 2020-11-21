#pragma once

#include "Math.hpp"

// Compute reflection direction
Vector3f reflect(const Vector3f& ray_in_dir, const Vector3f& normal);

// Compute refraction direction using Snell's law
Vector3f refract(const Vector3f& ray_in_dir, const Vector3f& normal, float ior);

// Compute Fresnel equation
static float fresnel(const Vector3f& observation_dir, const Vector3f& normal, float ior);

class Material {
public:
    virtual ~Material() = default;
	
public:
    virtual bool emitting() const = 0;
    virtual Vector3f emission(float u, float v) const = 0;

    // Given the direction of an incident ray and a normal vector,
    // calculate a random ray-out direction with some kind of sample distribution.
    [[nodiscard]] virtual Vector3f sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) const = 0;

    // Given the directions of the ray source and ray out and a normal vector,
    // calculate its value of PDF (probability distribution function).
    [[nodiscard]] virtual float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const = 0;

    // Given the directions of the ray source and ray out and a normal vector,
    // calculate its contribution from BSDF (bidirectional scattering distribution function).
    [[nodiscard]] virtual Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const = 0;
};

class GGX {
public:
    // Generalized-Trowbridge-Reitz Normal Distribution Function (GTR-NDF) when γ = 2
    static float distribution(float normal_dot_half_vec, float roughness_sq);
    // Fresnel-Schlick approximation
    static Vector3f fresnel_schlick(float half_vec_dot_observer_dir, const Vector3f& f0);
    // Smith-Joint Approximation from Respawn Entertainment
    static float geometry(float normal_dot_observer_dir, float normal_dot_light_source_dir, float roughness_sq);

	// calculate a random ray-out direction under GTR-NDF
    static Vector3f sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal, float roughness_sq);
    // probability distribution function for importance sampling on GTR-NDF
    static float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal, float roughness_sq);
};

// Diffuse
class Diffuse : public Material {
public:
    Diffuse(const Vector3f& albedo, const Vector3f& emission = {0.0f, 0.0f, 0.0f}) : _albedo(albedo), _emission(emission) {}

public:
    bool emitting() const override;
	Vector3f emission(float u, float v) const override;
	
    [[nodiscard]] Vector3f sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) const override;
    [[nodiscard]] float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;
	
private:
    Vector3f _albedo;
    Vector3f _emission;
};

// Metal-roughness workflow
class MetalRough : public Material {
public:
	MetalRough(const Vector3f& albedo, float roughness, float metallic,
        const Vector3f& emission = { 0.0f, 0.0f, 0.0f })
        : _albedo(albedo), _roughness(roughness), _roughness_sq(roughness * roughness), _metallic(metallic), _emission(emission) { }

public:
    bool emitting() const override;
    Vector3f emission(float u, float v) const override;
	
    [[nodiscard]] Vector3f sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) const override;
    [[nodiscard]] float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;

private:
    Vector3f _albedo;
    float _roughness;
    float _roughness_sq;
    float _metallic;
    Vector3f _emission;
};