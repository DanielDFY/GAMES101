#include "Renderer.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void Renderer::render(const Scene& scene) {
    const auto scene_size = scene.width() * scene.height();
    std::vector<Vector3f> framebuffer(scene_size);

    float scale = std::tan(degree_to_rad(scene.fov() * 0.5f));
    float image_aspect_ratio = static_cast<float>(scene.width()) / static_cast<float>(scene.height());

    // Use this variable as the eye position to start your rays.
    Vector3f eye_pos(-1.0f, 5.0f, 10.0f);
    int pixel_idx = 0;
    for (uint32_t j = 0; j < scene.height(); ++j) {
        for (uint32_t i = 0; i < scene.width(); ++i) {
            // generate primary ray direction
            const auto x = (2 * (static_cast<float>(i) + 0.5f) / static_cast<float>(scene.width()) - 1.0f) * scale * image_aspect_ratio;
            const auto y = (1.0f - 2 * (static_cast<float>(j) + 0.5f) / static_cast<float>(scene.height())) * scale;
            
            const auto dir = Vector3f(x, y, -1.0f).normalized();
            const Ray ray(eye_pos, dir);
            // Ray stops transport after its first hit in Whitted-syle light transport algorithm, so input max depth should be 0.
            framebuffer[pixel_idx++] = cast_ray(scene, ray, 0);
        }
        update_progress(static_cast<float>(j) / static_cast<float>(scene.height()));
    }

    update_progress(1.0f);
    std::cout << std::endl;

    // save framebuffer to file with tools from stb library
    const std::string output_file_name("output.png");
    constexpr int channel_num = 3;
    const int image_size = scene_size * channel_num;
	const size_t stride_in_bytes = scene.width() * channel_num * sizeof(unsigned char);
    const std::unique_ptr<unsigned char[]> pixel_data_ptr(new unsigned char[image_size]);

    for (auto i = 0, idx = 0; i < scene_size; ++i) {
        pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].x));
		pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].y));
		pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * clamp(0, 1, framebuffer[i].z));
    }

    stbi_write_png(output_file_name.c_str(), scene.width(), scene.height(), channel_num, pixel_data_ptr.get(), stride_in_bytes); 
}

Vector3f Renderer::cast_ray(const Scene& scene, const Ray& ray, int depth) const {
    // exceed max depth
    if (depth > scene.max_depth()) {
        return {0.0f, 0.0f, 0.0f};
    }

    auto color = scene.background_color();
    const auto intersection = scene.intersect(ray);
    if(intersection) {
        const auto pos = intersection->pos;
        const auto normal = intersection->normal;
        const auto mat_ptr = intersection->mat_ptr;

        switch (mat_ptr->type) {
            case Material_type::REFLECTION_AND_REFRACTION: {
                // Reflection
                const auto reflection_dir = reflect(ray.dir, normal).normalized();
                const auto reflection_ori = (reflection_dir.dot(normal) < 0) ? (pos + normal * -EPSILON) : (pos + normal * EPSILON);
                const Ray reflection_ray(reflection_ori, reflection_dir);
                const auto reflection_color = cast_ray(scene, reflection_ray, depth + 1);

                // Refraction
                const auto ior = mat_ptr->ior;
                const auto refraction_dir = refract(ray.dir, normal, ior).normalized();
                const auto refraction_ori = (refraction_dir.dot(normal) < 0) ? (pos + normal * -EPSILON) : (pos + normal * EPSILON);
                const Ray refraction_ray(refraction_ori, refraction_dir);
                
                const auto refraction_color = cast_ray(scene, refraction_ray, depth + 1);
                
                // Combine two results
                const auto kr = fresnel(ray.dir, normal, ior);
                color = reflection_color * kr + refraction_color * (1.0f - kr);
                break;
            } case Material_type::REFLECTION: {
                const auto ior = mat_ptr->ior;
                const auto kr = fresnel(ray.dir, normal, ior);

                const auto reflection_dir = reflect(ray.dir, normal).normalized();
                const auto reflection_ori = (reflection_dir.dot(normal) < 0) ? (pos + normal * -EPSILON) : (pos + normal * EPSILON);
                const Ray reflection_ray(reflection_ori, reflection_dir);
                color = cast_ray(scene, reflection_ray, depth + 1) * kr;
                break;
            } default: {
                // We use the Phong illumation model for the default case. The phong model
                // is composed of a diffuse and a specular reflection component.
                Vector3f light_ambient = {0.0f, 0.0f, 0.0f};
                Vector3f specular_color = {0.0f, 0.0f, 0.0f};

                const auto specular_exponent = intersection->mat_ptr->specular_exponent;

                Vector3f shadow_point_ori = (ray.dir.dot(normal) < 0) ? (pos + normal * EPSILON) : (pos + normal * -EPSILON);

                // Loop over all lights in the scene and sum their contribution up
                // We also apply the lambert cosine law
                for (const auto& light : scene.lights()) {
                    Vector3f light_dir = light->position - pos;
                    // square of the distance between intersection position and the light
                    float light_dist_sq = light_dir.magnitude_squared();
                    light_dir = light_dir.normalized();
                    float l_dot_n = std::max(0.0f, light_dir.dot(normal));

                    // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                    const auto shadow_res = scene.intersect({shadow_point_ori, light_dir});

                    // the direction vector of a ray has magnitude of 1, so the time value in the intersection record can be used as distance.
                    if (!shadow_res && (shadow_res->time * shadow_res->time < light_dist_sq))
                        light_ambient += light->intensity * l_dot_n;

                    const auto reflection_dir = reflect(-light_dir, normal);

                    specular_color += powf(std::max(0.0f, -reflection_dir.dot(ray.dir)), specular_exponent) * light->intensity;
                }

                // note: color production is different from vector dot production
                const auto ambient_color = light_ambient * mat_ptr->color;

                color = ambient_color * mat_ptr->kd + specular_color * mat_ptr->ks;
                break;
            }
        }
    }

    return color;
}