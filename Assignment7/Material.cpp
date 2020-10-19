#include "Material.hpp"

#include "Utility.hpp"

Vector3f reflect(const Vector3f& ray_dir, const Vector3f& normal) {
    return ray_dir - 2 * ray_dir.dot(normal) * normal;
}

// We need to handle with care the two possible situations:
// 1. When the ray is inside the object
// 2. When the ray is outside.
//
// If the ray is outside, you need to make cos_i positive
// If the ray is inside, you need to invert the refractive indices and negate the normal
Vector3f refract(const Vector3f& ray_dir, const Vector3f& normal, float ior) {
    auto cos_i = clamp(-1.0f, 1.0f, ray_dir.dot(normal));
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
    return k < 0 ? Vector3f{0.0f, 0.0f, 0.0f} : (eta * ray_dir + (eta * cos_i - sqrtf(k)) * correct_normal);
}

// ray_dir is the incident view direction
// normal is the normal at the intersection point
// ior is the material refractive index
float fresnel(const Vector3f& ray_dir, const Vector3f& normal, float ior) {
    auto cos_i = clamp(-1.0f, 1.0f, ray_dir.dot(normal));
    auto eta_i = 1.0f;
    auto eta_t = ior;
    if (cos_i > 0.0f) {
        std::swap(eta_i, eta_t);
    }
    // Compute sini using Snell's law
    const auto sin_t = eta_i / eta_t * sqrtf(std::max(0.0f, 1.0f - cos_i * cos_i));
    // Total internal reflection
    if (sin_t >= 1.0f) {
        return 1.0f;
    } else {
        const auto cost = sqrtf(std::max(0.0f, 1.0f - sin_t * sin_t));
        cos_i = fabsf(cos_i);
        float Rs = ((eta_t * cos_i) - (eta_i * cost)) / ((eta_t * cos_i) + (eta_i * cost));
        float Rp = ((eta_i * cos_i) - (eta_t * cost)) / ((eta_i * cos_i) + (eta_t * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
}

Vector3f Material::sample_ray_out_dir(const Vector3f& ray_in_dir, const Vector3f& normal) {
    switch(_type){
        case Material_type::DIFFUSE: {
            // uniformly sample the hemisphere
            const auto x1 = get_random_float();
            const auto x2 = get_random_float();
            const auto z = std::fabs(1.0f - 2.0f * x1);
            const auto r = std::sqrt(1.0f - z * z);
            const auto phi = 2 * PI * x2;

            // get local direction of the ray out
            const Vector3f local_ray_dir = {r * std::cos(phi), r * std::sin(phi), z};

            // transform to the world space
            Vector3f t, b;
            if (std::fabs(normal.x) > std::fabs(normal.y)){
                const auto inv_len = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
                t = {normal.z * inv_len, 0.0f, -normal.x * inv_len};
            } else {
                const auto inv_len = 1.0f / std::sqrt(normal.y * normal.y + normal.z * normal.z);
                t = {0.0f, normal.z * inv_len, -normal.y * inv_len};
            }
            b = t.cross(normal);
            return local_ray_dir.x * b + local_ray_dir.y * t + local_ray_dir.z * normal;
        } default: {
            throw std::runtime_error("no implemented sampling method for current material");
        }
    }
}

float Material::pdf(const Vector3f& ray_in_dir, const Vector3f& ray_out_dir, const Vector3f& normal) {
    switch(_type){
        case Material_type::DIFFUSE: {
            // uniform sample probability 1 / (2 * PI)
            return ray_out_dir.dot(normal) > 0.0f ? INV_2PI : 0.0f;
        } default: {
            throw std::runtime_error("no implemented pdf for current material");
        }
    }
}

Vector3f Material::contribution(const Vector3f& ray_in_dir, const Vector3f& ray_out_dir, const Vector3f& normal) {
    switch(_type){
        case Material_type::DIFFUSE: {
            return ray_out_dir.dot(normal) > 0.0f ? _kd * INV_PI : Vector3f(0.0f);
        } default: {
            throw std::runtime_error("no implemented BRDF for current material");
        }
    }
}