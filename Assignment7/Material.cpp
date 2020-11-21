#include "Material.hpp"

Vector3f reflect(const Vector3f& ray_in_dir, const Vector3f& normal) {
    return ray_in_dir - 2 * ray_in_dir.dot(normal) * normal;
}

// We need to handle with care the two possible situations:
// 1. When the ray is inside the object
// 2. When the ray is outside.
//
// If the ray is outside, you need to make cos_i positive
// If the ray is inside, you need to invert the refractive indices and negate the normal
Vector3f refract(const Vector3f& ray_in_dir, const Vector3f& normal, float ior) {
    auto cos_i = clamp(-1.0f, 1.0f, ray_in_dir.dot(normal));
    auto eta_i = 1.0f;
    auto eta_t = ior;
    Vector3f correct_normal = normal;
    if (cos_i < 0.0f) {
        cos_i = -cos_i;
    } else {
        std::swap(eta_i, eta_t);
        correct_normal = -normal;
    }

    const auto eta = eta_i / eta_t;
    const auto k = 1 - eta * eta * (1 - cos_i * cos_i);
    return k < 0 ? Vector3f{0.0f, 0.0f, 0.0f} : (eta * ray_in_dir + (eta * cos_i - std::sqrtf(k)) * correct_normal);
}

// observation_dir is the incident observing direction
// normal is the normal at the intersection point
// ior is the material refractive index
float fresnel(const Vector3f& observation_dir, const Vector3f& normal, float ior) {
    auto cos_i = clamp(-1.0f, 1.0f, observation_dir.dot(normal));
    auto eta_i = 1.0f;
    auto eta_t = ior;
    if (cos_i > 0.0f) {
        std::swap(eta_i, eta_t);
    }
    // Compute sin_i using Snell's law
    const auto sin_t = eta_i / eta_t * sqrtf(std::max(0.0f, 1.0f - cos_i * cos_i));
    // Total internal reflection
    if (sin_t >= 1.0f) {
        return 1.0f;
    } else {
        const auto cost = sqrtf(std::max(0.0f, 1.0f - sin_t * sin_t));
        cos_i = fabsf(cos_i);
        const auto Rs = ((eta_t * cos_i) - (eta_i * cost)) / ((eta_t * cos_i) + (eta_i * cost));
        const auto Rp = ((eta_i * cos_i) - (eta_t * cost)) / ((eta_i * cos_i) + (eta_t * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
}

float GGX::distribution(float normal_dot_half_vec, float roughness_sq) {
    const auto normal_dot_half_vec_sq = normal_dot_half_vec * normal_dot_half_vec;
    auto denominator = normal_dot_half_vec_sq * (roughness_sq - 1.0f) + 1.0f;
    denominator = PI * denominator * denominator;
    return roughness_sq / denominator;
}

Vector3f GGX::fresnel_schlick(float half_vec_dot_observer_dir, const Vector3f& f0) {
    return f0 + (Vector3f(1.0f) - f0) * pow(1.0f - half_vec_dot_observer_dir, 5);
}

float GGX::geometry(float normal_dot_observer_dir, float normal_dot_light_source_dir, float roughness) {
    return 0.5f / lerp(2 * normal_dot_light_source_dir * normal_dot_observer_dir, normal_dot_light_source_dir + normal_dot_observer_dir, roughness);
}

Vector3f GGX::sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal, float roughness_sq) {
    const auto r0 = get_random_float();
    const auto r1 = get_random_float();
    const auto theta = std::acosf(std::sqrtf((1.0f - r0) / ((roughness_sq - 1.0f) * r0 + 1.0f)));
    const auto phi = 2 * PI * r1;
	
    const auto local_micro_surface_normal = polar_to_cartesian(theta, phi);
    const auto micro_surface_normal = local_to_world(local_micro_surface_normal, normal);
    return reflect(ray_in_dir, micro_surface_normal);
}

float GGX::pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal, float roughness_sq) {
    const auto half_vec = (ray_source_dir + ray_out_dir).normalized();
    const auto normal_dot_half_vec = normal.dot(half_vec);
    const auto half_vec_dot_observer_dir = ray_out_dir.dot(half_vec);
	// importance sampling for NDF
    return (distribution(normal_dot_half_vec, roughness_sq) * normal_dot_half_vec) / (4 * half_vec_dot_observer_dir);
}

bool Diffuse::emitting() const {
    return _emission.magnitude_squared() > 0.0f;
}

Vector3f Diffuse::emission(float u, float v) const {
    return _emission;
}

Vector3f Diffuse::sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) const {
    // uniformly sample the hemisphere
    const auto x1 = get_random_float();
    const auto x2 = get_random_float();
    const auto z = std::fabs(1.0f - 2.0f * x1);
    const auto r = std::sqrt(1.0f - z * z);
    const auto phi = 2 * PI * x2;

    // get local direction of the ray out
    const Vector3f local_ray_out_dir = { r * std::cos(phi), r * std::sin(phi), z };

    // transform to the world space
    return local_to_world(local_ray_out_dir, normal);
}

float Diffuse::pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    // uniformly sampling from hemisphere results in probability 1 / (2 * PI)
    return ray_out_dir.dot(normal) > 0.0f ? 0.5f * INV_PI : 0.0f;
}

Vector3f Diffuse::contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    return ray_out_dir.dot(normal) > 0.0f ? _albedo * INV_PI : Vector3f(0.0f);
}

bool MetalRough::emitting() const {
    return _emission.magnitude_squared() > 0.0f;
}

Vector3f MetalRough::emission(float u, float v) const {
    return _emission;
}

Vector3f MetalRough::sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) const {
    return GGX::sample_ray_out_dir(ray_in_dir, normal, _roughness_sq);
}

float MetalRough::pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    return GGX::pdf(ray_source_dir, ray_out_dir, normal, _roughness_sq);
}

Vector3f MetalRough::contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto normal_dot_light_source_dir = normal.dot(ray_source_dir);
    const auto normal_dot_observer_dir = normal.dot(ray_out_dir);
    if (normal_dot_light_source_dir == 0.0f || normal_dot_observer_dir == 0.0f)
        return { 0.0f };

    const auto half_vec = (ray_source_dir + ray_out_dir).normalized();
    const auto normal_dot_half_vec = normal.dot(half_vec);
	
    const auto D = GGX::distribution(normal_dot_half_vec, _roughness_sq);
    const auto G = GGX::geometry(half_vec.dot(ray_out_dir), half_vec.dot(ray_source_dir), _roughness);

    // Metal-roughness workflow
    const Vector3f f0_base(0.04f);
    const auto f0 = lerp(f0_base, _albedo, _metallic);
    const auto half_vec_dot_observer_dir = half_vec.dot(ray_out_dir);
    const auto F = GGX::fresnel_schlick(half_vec_dot_observer_dir, f0);
    const auto kd = (Vector3f(1.0f) - F) * (1.0f - _metallic);

    // Lambert diffuse
    const auto diffuse = kd * _albedo * INV_PI;
    // Cook–Torrance Specular (original denominator is merged into G because of Smith-Joint approximation)
    const auto specular =  D * F * G;

    // BSDF
    return diffuse + specular;
}
