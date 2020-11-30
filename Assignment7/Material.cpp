#include "Material.hpp"

Vector3f reflect(const Vector3f& ray_in_dir, const Vector3f& normal) {
    return ray_in_dir - 2 * ray_in_dir.dot(normal) * normal;
}

// If the ray is outside, make cos_i positive.
// If the ray is inside, invert the refractive indices and negate the normal.
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
    const auto k = 1.0f - eta * eta * (1.0f - cos_i * cos_i);
    return k < 0.0f ? Vector3f{0.0f, 0.0f, 0.0f} : (eta * ray_in_dir + (eta * cos_i - std::sqrtf(k)) * correct_normal).normalized();
}

float fresnel(const Vector3f& observation_dir, const Vector3f& normal, float ior) {
    auto cos_i = clamp(-1.0f, 1.0f, observation_dir.dot(normal));
    auto eta_i = 1.0f;
    auto eta_t = ior;
    if (cos_i > 0.0f) {
        std::swap(eta_i, eta_t);
    }
    // Compute sin_t using Snell's law
    const auto sin_t = eta_i / eta_t * std::sqrt(std::max(0.0f, 1.0f - cos_i * cos_i));
    // Total internal reflection
    if (sin_t >= 1.0f) {
        return 1.0f;
    } else {
        const auto cos_t = std::sqrt(std::max(0.0f, 1.0f - sin_t * sin_t));
        cos_i = fabsf(cos_i);
        const auto rs = ((eta_t * cos_i) - (eta_i * cos_t)) / ((eta_t * cos_i) + (eta_i * cos_t));
        const auto rp = ((eta_i * cos_i) - (eta_t * cos_t)) / ((eta_i * cos_i) + (eta_t * cos_t));
        return (rs * rs + rp * rp) / 2;
    }
}



float Microfacet::distribution(float normal_dot_micro_surface_normal, float roughness_sq) {
    const auto normal_dot_micro_surface_normal_sq = normal_dot_micro_surface_normal * normal_dot_micro_surface_normal;
    auto denominator = normal_dot_micro_surface_normal_sq * (roughness_sq - 1.0f) + 1.0f;
    denominator = PI * denominator * denominator;
    return roughness_sq / denominator;
}

Vector3f Microfacet::fresnel_schlick(float micro_surface_normal_dot_ray_out_dir, const Vector3f& f0) {
    return f0 + (Vector3f(1.0f) - f0) * pow(1.0f - micro_surface_normal_dot_ray_out_dir, 5);
}

float Microfacet::geometry(float normal_dot_light_source_dir, float normal_dot_observer_dir, float roughness) {
    return 2.0f / lerp(abs(2 * normal_dot_light_source_dir * normal_dot_observer_dir), abs(normal_dot_light_source_dir + normal_dot_observer_dir), roughness);
}

Vector3f Microfacet::sample_micro_surface(const Vector3f& normal, float roughness_sq) {
    const auto r0 = get_random_float();
    const auto r1 = get_random_float();
    const auto theta = std::acosf(std::sqrtf((1.0f - r0) / ((roughness_sq - 1.0f) * r0 + 1.0f)));
    const auto phi = 2 * PI * r1;
	
    const auto local_micro_surface_normal = polar_to_cartesian(theta, phi);
    return local_to_world(local_micro_surface_normal, normal);
}

float Microfacet::pdf_micro_surface(float normal_dot_micro_surface_normal, float roughness_sq) {
	// importance sampling on NDF
	const auto normal_dot_micro_surface_normal_abs = abs(normal_dot_micro_surface_normal);
    return (distribution(normal_dot_micro_surface_normal_abs, roughness_sq) * normal_dot_micro_surface_normal_abs);
}

Vector3f Microfacet::outward_micro_surface_normal(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, bool is_same_side, bool is_surface_outward, float ior) {
    if (is_same_side) {
        // reflection
        if (is_surface_outward) {
            return (ray_source_dir + ray_out_dir).normalized();
        } else {
            return -(ray_source_dir + ray_out_dir).normalized();
        }
    } else {
        // refraction
        if (is_surface_outward) {
            return -(ray_out_dir + ray_source_dir * ior).normalized();
        } else {
            return -(ior * ray_out_dir + ray_source_dir).normalized();
        }
    }
}

float Microfacet::reflect_jacobian(float micro_surface_normal_dot_ray_out_dir) {
    return micro_surface_normal_dot_ray_out_dir == 0.0f ? 0.0f : 1.0f / (4 * abs(micro_surface_normal_dot_ray_out_dir));
}

float Microfacet::refract_jacobian(float micro_surface_normal_dot_ray_source_dir, float ior_in, float micro_surface_normal_dot_ray_out_dir, float ior_out) {
    auto denominator = ior_in * micro_surface_normal_dot_ray_source_dir + ior_out * micro_surface_normal_dot_ray_out_dir;
    denominator *= denominator;
    return denominator == 0.0f ? 0.0f : (ior_out * ior_out * abs(micro_surface_normal_dot_ray_out_dir)) / denominator;
}



bool Diffuse::emitting() const {
    return _emission.magnitude_squared() > 0.0f;
}

Vector3f Diffuse::emission(float u, float v) const {
    return _emission;
}

Vector3f Diffuse::sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const {
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

Vector3f MetalRough::sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto micro_surface_normal = Microfacet::sample_micro_surface(normal, _roughness_sq);
    const auto observation_dir = -ray_out_dir;
    return reflect(observation_dir, micro_surface_normal);  // trace back
}

float MetalRough::pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto check_ray_dir = normal.dot(ray_source_dir) * normal.dot(ray_out_dir);
    if (check_ray_dir <= 0.0f)
        return 0.0f;    // no refraction

    const auto micro_surface_normal = (ray_source_dir + ray_out_dir).normalized();
    const auto normal_dot_micro_surface_normal = normal.dot(micro_surface_normal);
    const auto micro_surface_normal_dot_ray_out_dir = micro_surface_normal.dot(ray_out_dir);

    const auto pdf_micro_surface = Microfacet::pdf_micro_surface(normal_dot_micro_surface_normal, _roughness_sq);
    const auto jacobian = Microfacet::reflect_jacobian(micro_surface_normal_dot_ray_out_dir);

    return pdf_micro_surface * jacobian;
}

Vector3f MetalRough::contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto check_ray_dir = normal.dot(ray_source_dir) * normal.dot(ray_out_dir);
    if (check_ray_dir <= 0.0f)
        return 0.0f;    // no refraction

    const auto micro_surface_normal = (ray_source_dir + ray_out_dir).normalized();
    const auto normal_dot_micro_surface_normal = normal.dot(micro_surface_normal);
    const auto micro_surface_normal_dot_ray_source_dir = micro_surface_normal.dot(ray_source_dir);
    const auto micro_surface_normal_dot_ray_out_dir = micro_surface_normal.dot(ray_out_dir);
	
    const auto D = Microfacet::distribution(normal_dot_micro_surface_normal, _roughness_sq);
    const auto G = Microfacet::geometry(micro_surface_normal_dot_ray_source_dir, micro_surface_normal_dot_ray_out_dir, _roughness);

    // Metal-roughness workflow
    const Vector3f f0_base(0.04f);
    const auto f0 = lerp(f0_base, _albedo, _metallic);
    const auto F = Microfacet::fresnel_schlick(micro_surface_normal_dot_ray_out_dir, f0);
    const auto kd = (Vector3f(1.0f) - F) * (1.0f - _metallic);

    // Lambert diffuse
    const auto diffuse = kd * _albedo * INV_PI;
    // Cook–Torrance Specular (original denominator is merged into G for Smith-Joint approximation)
    const auto specular =  D * F * G / 4.0f;

    // BSDF
    return diffuse + specular;
}



bool FrostedGlass::emitting() const {
    return _emission.magnitude_squared() > 0.0f;
}

Vector3f FrostedGlass::emission(float u, float v) const {
    return _emission;
}

Vector3f FrostedGlass::sample_ray_source_dir(const Vector3f& ray_out_dir, const Vector3f& normal) const {
    // randomly choose a micro surface
    const auto micro_surface_normal = Microfacet::sample_micro_surface(normal, _roughness_sq);
    const auto observation_dir  = -ray_out_dir;

    const auto f = fresnel(observation_dir, micro_surface_normal, _ior);

    // trace back
    if (get_random_float() < f) {
        // reflection
        return reflect(observation_dir, micro_surface_normal);
    } else {
        // refraction(transmission)
        return refract(observation_dir, micro_surface_normal, _ior);
    }
}

float FrostedGlass::pdf(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto normal_dot_ray_source_dir = normal.dot(ray_source_dir);
    const auto normal_dot_ray_out_dir = normal.dot(ray_out_dir);
    const auto observation_dir = -ray_out_dir;

    const auto check_ray_dir = normal_dot_ray_source_dir * normal_dot_ray_out_dir;
    if (check_ray_dir == 0.0f)
        return 0.0f;

    const auto is_same_side = check_ray_dir > 0.0f;
    const auto is_surface_outward = normal_dot_ray_out_dir > 0.0f;

    const auto micro_surface_normal = Microfacet::outward_micro_surface_normal(ray_source_dir, ray_out_dir, is_same_side, is_surface_outward, _ior);
    const auto f = fresnel(observation_dir, micro_surface_normal, _ior);

    const auto normal_dot_micro_surface_normal = normal.dot(micro_surface_normal);
    const auto pdf_micro_surface = Microfacet::pdf_micro_surface(normal_dot_micro_surface_normal, _roughness_sq);

    const auto micro_surface_normal_dot_ray_source_dir = micro_surface_normal.dot(ray_source_dir);
    const auto micro_surface_normal_dot_ray_out_dir = micro_surface_normal.dot(ray_out_dir);

    if (is_same_side) {
        // reflection
        const auto jacobian = Microfacet::reflect_jacobian(micro_surface_normal_dot_ray_out_dir);
        return pdf_micro_surface * f * jacobian;
    } else {
        // refraction
        const auto ior_in = normal_dot_ray_source_dir < 0.0f ? _ior : 1.0f;
        const auto ior_out = normal_dot_ray_out_dir < 0.0f ? _ior : 1.0f;
        const auto jacobian = Microfacet::refract_jacobian(micro_surface_normal_dot_ray_source_dir, ior_in, micro_surface_normal_dot_ray_out_dir, ior_out);
        return pdf_micro_surface * (1.0f - f) * jacobian;
    }
}

Vector3f FrostedGlass::contribution(const Vector3f& ray_source_dir, const Vector3f& ray_out_dir, const Vector3f& normal) const {
    const auto normal_dot_ray_source_dir = normal.dot(ray_source_dir);
    const auto normal_dot_ray_out_dir = normal.dot(ray_out_dir);
    const auto check_ray_dir = normal_dot_ray_source_dir * normal_dot_ray_out_dir;
    if (check_ray_dir == 0.0f)
        return 0.0f;

    const auto observation_dir = -ray_out_dir;
    const auto is_same_side = check_ray_dir > 0.0f;
    const auto is_surface_outward = normal_dot_ray_out_dir > 0.0f;

    const auto micro_surface_normal = Microfacet::outward_micro_surface_normal(ray_source_dir, ray_out_dir, is_same_side, is_surface_outward, _ior);

    const auto normal_dot_micro_surface_normal = normal.dot(micro_surface_normal);
    const auto micro_surface_normal_dot_ray_source_dir = micro_surface_normal.dot(ray_source_dir);
    const auto micro_surface_normal_dot_ray_out_dir = micro_surface_normal.dot(ray_out_dir);

    const auto D = Microfacet::distribution(normal_dot_micro_surface_normal, _roughness_sq);
    const auto G = Microfacet::geometry(micro_surface_normal_dot_ray_source_dir, micro_surface_normal_dot_ray_out_dir, _roughness);
    const auto F = fresnel(observation_dir, micro_surface_normal, _ior);

    if (is_same_side) {
        // reflection
        return D * F * G / 4.0f;   // Cook–Torrance Specular (original denominator is merged into G for Smith-Joint approximation)
    } else {
        // refraction
        const auto ior_in = normal_dot_ray_source_dir < 0.0f ? _ior : 1.0f;
        const auto ior_out = normal_dot_ray_out_dir < 0.0f ? _ior : 1.0f;

        // Transmission (original denominator is merged into G for Smith-Joint approximation)
        return Microfacet::refract_jacobian(micro_surface_normal_dot_ray_source_dir, ior_in, micro_surface_normal_dot_ray_out_dir, ior_out)
            * abs(micro_surface_normal_dot_ray_source_dir) * D * (1.0f - F) * G;
    }
}