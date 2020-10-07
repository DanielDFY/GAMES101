#include "Material.hpp"

#include "Utility.hpp"

// Compute reflection direction
Vector3f reflect(const Vector3f& ray_dir, const Vector3f& normal) {
    return ray_dir - 2 * ray_dir.dot(normal) * normal;
}

// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
// 1. When the ray is inside the object
// 2. When the ray is outside.
//
// If the ray is outside, you need to make cosi positive
// If the ray is inside, you need to invert the refractive indices and negate the normal

Vector3f refract(const Vector3f& ray_dir, const Vector3f& normal, float ior) {
    auto cosi = clamp(-1.0f, 1.0f, ray_dir.dot(normal));
    auto etai = 1.0f;
    auto etat = ior;
    Vector3f correct_normal = normal;
    if (cosi < 0) {
         cosi = -cosi;
    } else {
        std::swap(etai, etat);
        correct_normal = -normal;
    }

    const auto eta = etai / etat;
    const auto k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Vector3f{0.0f, 0.0f, 0.0f} : (eta * ray_dir + (eta * cosi - sqrtf(k)) * correct_normal);
}

// Compute Fresnel equation
//
// ray_dir is the incident view direction
// normal is the normal at the intersection point
// ior is the material refractive index
float fresnel(const Vector3f& ray_dir, const Vector3f& normal, float ior) {
    auto cosi = clamp(-1.0f, 1.0f, ray_dir.dot(normal));
    auto etai = 1.0f;
    auto etat = ior;
    if (cosi > 0.0f) {
        std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    const auto sint = etai / etat * sqrtf(std::max(0.0f, 1.0f - cosi * cosi));
    // Total internal reflection
    if (sint >= 1.0f) {
        return 1.0f;
    } else {
        const auto cost = sqrtf(std::max(0.0f, 1.0f - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}