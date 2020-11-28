#pragma once

#include "Math.hpp"

// Compute reflection direction
Vector3f reflect(const Vector3f& ray_in_dir, const Vector3f& normal);

// Compute refraction direction using Snell's law
Vector3f refract(const Vector3f& ray_in_dir, const Vector3f& normal, float ior);

// Compute Fresnel equation
static float fresnel(const Vector3f& ray_in_dir, const Vector3f& normal, float ior);

class Material {
public:
    virtual ~Material() = default;
	
public:
    virtual bool emitting() const = 0;
    virtual Vector3f emission(float u, float v) const = 0;

    // Given the direction of the observer, calculate a random ray source direction.
    [[nodiscard]] virtual Vector3f sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const = 0;

    // Given the sampled ray source direction, the direction of ray out and a normal vector,
    // calculate its value of PDF (probability distribution function).
    [[nodiscard]] virtual float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const = 0;

    // Given the directions of the ray source and ray out and a normal vector,
    // calculate its contribution from BSDF (bidirectional scattering distribution function).
    [[nodiscard]] virtual Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const = 0;
};

class Microfacet {
public:
    // Generalized-Trowbridge-Reitz Normal Distribution Function (GTR-NDF) when γ = 2.
    static float distribution(float normal_dot_micro_surface_normal, float roughness_sq);
    // Fresnel-Schlick approximation
    static Vector3f fresnel_schlick(float micro_surface_normal_dot_ray_out_dir, const Vector3f& f0);
    // Smith-Joint Approximation from Respawn Entertainment.
    static float geometry(float normal_dot_light_source_dir, float normal_dot_observer_dir, float roughness_sq);

	// Sample a micro-surface under the distribution function and calculate its surface normal.
    static Vector3f sample_micro_surface(const Vector3f& normal, float roughness_sq);
    // probability distribution function for importance sampling on GTR-NDF
    static float pdf_micro_surface(float normal_dot_micro_surface_normal, float roughness_sq);

    // The absolute value of the determinant of the Jacobian matrix for the transformation
    // between micro-surface normal and reflected ray.
    static float reflect_jacobian(float micro_surface_normal_dot_ray_out_dir);
    // The absolute value of the determinant of the Jacobian matrix for the transformation
    // between micro-surface normal and refracted ray.
    static float refract_jacobian(float normal_dot_ray_source_dir, float normal_dot_ray_out_dir, float micro_surface_normal_dot_ray_source_dir, float micro_surface_normal_dot_ray_out_dir, float ior);
};

// Diffuse
class Diffuse : public Material {
public:
    Diffuse(const Vector3f& albedo, const Vector3f& emission = {0.0f, 0.0f, 0.0f}) : _albedo(albedo), _emission(emission) {}

public:
    bool emitting() const override;
	Vector3f emission(float u, float v) const override;
	
    [[nodiscard]] Vector3f sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const override;
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
	
    [[nodiscard]] Vector3f sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;

private:
    Vector3f _albedo;
    float _roughness;
    float _roughness_sq;
    float _metallic;
    Vector3f _emission;
};

// Transparent
class Transparent : public Material {
public:
    Transparent(float roughness, float ior, const Vector3f& emission = { 0.0f, 0.0f, 0.0f })
        : _roughness(roughness), _roughness_sq(roughness * roughness), _ior(ior), _emission(emission) { }

public:
    bool emitting() const override;
    Vector3f emission(float u, float v) const override;

    [[nodiscard]] Vector3f sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] float pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;
    [[nodiscard]] Vector3f contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const override;

private:
    float _roughness;
    float _roughness_sq;
    float _ior;
    Vector3f _emission;
};