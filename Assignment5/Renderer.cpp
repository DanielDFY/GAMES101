#include "Renderer.hpp"

#include <iostream>
#include <optional>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Scene.hpp"
#include "Utility.hpp"

// Compute reflection direction
Eigen::Vector3f reflect(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal) {
    return ray_in - 2 * ray_in.dot(normal) * normal;
}

// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
// 1. When the ray is inside the object
// 2. When the ray is outside.
//
// If the ray is outside, you need to make cosi positive
// If the ray is inside, you need to invert the refractive indices and negate the normal

Eigen::Vector3f refract(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal, float ior) {
    auto cosi = clamp(-1.0f, 1.0f, ray_in.dot(normal));
    auto etai = 1.0f;
    auto etat = ior;
    Eigen::Vector3f correct_normal = normal;
    if (cosi < 0) {
         cosi = -cosi; 
    } else {
        std::swap(etai, etat);
        correct_normal = -normal;
    }

    const auto eta = etai / etat;
    const auto k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Eigen::Vector3f(0.0f, 0.0f, 0.0f) : (eta * ray_in + (eta * cosi - sqrtf(k)) * correct_normal);
}

// Compute Fresnel equation
//
// ray_in is the incident view direction
// normal is the normal at the intersection point
// ior is the material refractive index
float fresnel(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal, float ior) {
    auto cosi = clamp(-1.0f, 1.0f, ray_in.dot(normal));
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

// Returns true if the ray intersects an object, false otherwise.
//
// ori is the ray origin
// dir is the ray direction
// objects is the list of objects the scene contains
// t_near contains the distance to the cloesest intersected object.
// index stores the index of the intersect triangle if the interesected object is a mesh.
// uv stores the u and v barycentric coordinates of the intersected point
// obj_ptr stores the unique_ptr to the intersected object (used to retrieve material information, etc.)
// 
// We can return from the function sooner as soon as we have found a hit.
std::optional<Hit_payload> trace(
        const Eigen::Vector3f& ori, const Eigen::Vector3f& dir,
        const std::vector<std::shared_ptr<Object>>& obj_ptr_list) {

    // final record
    float t_near = FLOAT_INFINITY;
    std::optional<Hit_payload> payload;

    for (const auto& obj_ptr : obj_ptr_list) {
        // temporary record
        float t_near_temp = FLOAT_INFINITY;
        uint32_t index_temp;
        Eigen::Vector2f uv_temp;
        if (obj_ptr->intersect(ori, dir, t_near_temp, index_temp, uv_temp) && t_near_temp < t_near) {
            payload.emplace();
            payload->obj_ptr = obj_ptr;
            payload->t_near = t_near_temp;
            payload->index = index_temp;
            payload->uv = uv_temp;
            t_near = t_near_temp;
        }
    }

    return payload;
}

// Implementation of the Whitted-style light transport algorithm
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refraction direction and cast two new rays into the scene
// by calling the cast_ray() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refraction depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is diffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
Eigen::Vector3f cast_ray( const Eigen::Vector3f& ori, const Eigen::Vector3f& dir, const Scene& scene, int depth) {
    // exceed max depth
    if (depth > scene.max_depth) {
        return {0.0f, 0.0f, 0.0f};
    }

    auto hit_color = scene.background_color;
    const auto payload = trace(ori, dir, scene.get_objects());
    if (payload) {
        const Eigen::Vector3f hit_point = ori + dir * payload->t_near;

        Eigen::Vector3f normal; // normal
        Eigen::Vector2f st; // st coordinates
        payload->obj_ptr->get_surface_properties(hit_point, dir, payload->index, payload->uv, normal, st);

        switch (payload->obj_ptr->material_type) {
            case Material_type::REFLECTION_AND_REFRACTION: {
                const auto reflection_dir = reflect(dir, normal).normalized();
                const auto reflection_ray_ori = (reflection_dir.dot(normal) < 0) ?
                                                (hit_point + normal * -scene.epsilon) :
                                                (hit_point + normal * scene.epsilon);
                const auto reflection_color = cast_ray(reflection_ray_ori, reflection_dir, scene, depth + 1);

                const auto refraction_dir = refract(dir, normal, payload->obj_ptr->ior).normalized();
                const auto refraction_ray_ori = (refraction_dir.dot(normal) < 0) ?
                                                (hit_point + normal * -scene.epsilon) :
                                                (hit_point + normal * scene.epsilon);
                
                const auto refraction_color = cast_ray(refraction_ray_ori, refraction_dir, scene, depth + 1);
                
                const auto kr = fresnel(dir, normal, payload->obj_ptr->ior);
                hit_color = reflection_color * kr + refraction_color * (1 - kr);
                break;
            } case Material_type::REFLECTION: {
                const auto kr = fresnel(dir, normal, payload->obj_ptr->ior);
                const auto reflection_dir = reflect(dir, normal).normalized();
                const auto reflection_ray_ori = (reflection_dir.dot(normal) < 0) ?
                                                (hit_point + normal * -scene.epsilon) :
                                                (hit_point + normal * scene.epsilon);
                hit_color = kr * cast_ray(reflection_ray_ori, reflection_dir, scene, depth + 1);
                break;
            } default: {
                // We use the Phong illumation model for the default case. The phong model
                // is composed of a diffuse and a specular reflection component.
                Eigen::Vector3f light_ambient = {0.0f, 0.0f, 0.0f};
                Eigen::Vector3f specular_color = {0.0f, 0.0f, 0.0f};

                Eigen::Vector3f shadow_point_ori = (dir.dot(normal) < 0) ?
                                           hit_point + normal * scene.epsilon :
                                           hit_point + normal * -scene.epsilon;

                // Loop over all lights in the scene and sum their contribution up
                // We also apply the lambert cosine law
                for (const auto& light : scene.get_lights()) {
                    Eigen::Vector3f light_dir = light->position - hit_point;
                    // square of the distance between hitPoint and the light
                    float light_dist_squared = light_dir.squaredNorm();
                    light_dir = light_dir.normalized();
                    float l_dot_n = std::max(0.0f, light_dir.dot(normal));
                    // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                    auto shadow_res = trace(shadow_point_ori, light_dir, scene.get_objects());
                    bool in_shadow = shadow_res && (shadow_res->t_near * shadow_res->t_near < light_dist_squared);

                    if (!in_shadow)
                        light_ambient += light->intensity * l_dot_n;

                    const auto reflection_dir = reflect(-light_dir, normal);

                    specular_color += powf(std::max(0.0f, -reflection_dir.dot(dir)),
                        payload->obj_ptr->specular_exponent) * light->intensity;
                }

                // note: color production is different from vector dot production
                hit_color = light_ambient.cwiseProduct(payload->obj_ptr->eval_diffuse_color(st)) * payload->obj_ptr->kd + specular_color * payload->obj_ptr->ks;
                break;
            }
        }
    }

    return hit_color;
}

// The main render function. This where we iterate over all pixels in the image, generate
// primary rays and cast these rays into the scene. The content of the framebuffer is
// saved to a file.
void Renderer::render(const Scene& scene) {
    const auto scene_size = scene.width * scene.height;
    std::vector<Eigen::Vector3f> framebuffer(scene_size);

    float scale = std::tan(deg_to_rad(scene.fov * 0.5f));
    float image_aspect_ratio = static_cast<float>(scene.width) / static_cast<float>(scene.height);

    // Use this variable as the eye position to start your rays.
    Eigen::Vector3f eye_pos(0.0f, 0.0f, 0.0f);
    int pixel_idx = 0;
    for (int j = 0; j < scene.height; ++j) {
        for (int i = 0; i < scene.width; ++i) {
            // Find the x and y positions of the current pixel to get the direction
            // vector that passes through it. Also, multiply both of them with the
            // variable *scale*, and x (horizontal) variable with the *image_aspect_ratio*    
            const auto x = (static_cast<float>(i) / static_cast<float>(scene.width) - 0.5f) * 2 * scale * image_aspect_ratio; 
            const auto y = (1.0f - static_cast<float>(j) / static_cast<float>(scene.height) - 0.5f) * 2 * scale;

            const auto dir = Eigen::Vector3f(x, y, -1).normalized(); // Don't forget to normalize this direction!
            framebuffer[pixel_idx++] = cast_ray(eye_pos, dir, scene, 0);
        }
        update_progress(j / static_cast<float>(scene.height));
    }

    update_progress(1.0f);
    std::cout << std::endl;

    // save framebuffer to file with tools from stb library
    const std::string output_file_name("output.png");
    constexpr int channel_num = 3;
    const int image_size = scene_size * channel_num;
	const size_t stride_in_bytes = scene.width * channel_num * sizeof(unsigned char);
    const std::unique_ptr<unsigned char[]> pixel_data_ptr(new unsigned char[image_size]);

    for (auto i = 0, idx = 0; i < scene_size; ++i) {
        pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].x()));
		pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].y()));
		pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].z()));
    }

    stbi_write_png(output_file_name.c_str(), scene.width, scene.height, channel_num, pixel_data_ptr.get(), stride_in_bytes);
}
