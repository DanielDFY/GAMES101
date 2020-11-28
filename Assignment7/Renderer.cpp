#include "Renderer.hpp"

#include <iostream>
#include <future>
#include <functional>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void Renderer::render(const Scene& scene, unsigned int spp, unsigned int total_thread_count) const {
    const auto scene_size = static_cast<size_t>(scene.width()) * static_cast<size_t>(scene.height());
    std::vector<Vector3f> frame_buffer(scene_size);

    std::cout << "SPP: " << spp << std::endl;

    std::vector<std::future<void>> thread_handles(total_thread_count);
    for (unsigned int thread_id = 0; thread_id < total_thread_count; ++thread_id) {
        thread_handles[thread_id] = std::async(std::launch::async, &Renderer::render_thread, this, total_thread_count, thread_id, std::cref(scene), spp, std::ref(frame_buffer));
    }

    for (const auto& handle : thread_handles) {
        handle.wait();
    }

    update_progress(1.0f);
    std::cout << std::endl;

    // save frame buffer to file with tools from stb library
    const std::string output_file_name("output.png");
    constexpr unsigned int channel_num = 3;
    const size_t image_size = static_cast<size_t>(scene_size) * static_cast<size_t>(channel_num);
    const auto stride_in_bytes = static_cast<size_t>(scene.width()) * channel_num * sizeof(unsigned char);
    const std::unique_ptr<unsigned char[]> pixel_data_ptr(new unsigned char[image_size]);

    for (auto i = 0, idx = 0; i < scene_size; ++i) {
        // pow(color, exponent) for gamma correction
		pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * std::pow(clamp(0.0f, 1.0f, frame_buffer[i].x), 0.6f));
        pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * std::pow(clamp(0.0f, 1.0f, frame_buffer[i].y), 0.6f));
        pixel_data_ptr[idx++] = static_cast<unsigned char>(255.0f * std::pow(clamp(0.0f, 1.0f, frame_buffer[i].z), 0.6f));
    }

    stbi_write_png(output_file_name.c_str(), static_cast<int>(scene.width()), static_cast<int>(scene.height()), channel_num, pixel_data_ptr.get(), stride_in_bytes);
}

Vector3f Renderer::cast_ray(const Scene& scene, const Ray& ray, Culling culling) const {
    const auto intersection = scene.intersect(ray, culling);
    if (!intersection)
        return scene.background_color();

    const auto pos = intersection->pos;         // position of shading point
    const auto normal = intersection->normal;   // normal at shading point
    const auto observer_dir = -ray.dir;         // observer direction
    const auto mat_ptr = intersection->mat_ptr;             // material at shading point

    // Direct illumination
    Vector3f i_direct = { 0.0f, 0.0f, 0.0f };

    // sample light sources
    const auto light_sample = scene.sample_light_sources();
    if (light_sample) {
        const auto light_sample_pos = light_sample->intersection.pos;               // position of sample point
        const auto intersection_to_light_sample = light_sample_pos - pos;           // shading point to light sample point
        const auto light_sample_dir = intersection_to_light_sample.normalized();    // light sample point direction
        const auto light_dir = -light_sample_dir;                                   // light direction
        const auto light_sample_normal = light_sample->intersection.normal;         // normal at sample point

        // check block between shading point and light sample point
        const auto check_intersection = scene.intersect({ pos, light_sample_dir }, Culling::BACK);
        if (check_intersection && (check_intersection->pos - light_sample_pos).magnitude_squared() < EPSILON) {
            const auto emission = light_sample->intersection.mat_ptr->emission(light_sample->intersection.uv.x, light_sample->intersection.uv.y);

            // light source importance sampling
            const auto light_dir_dot_light_sample_normal = light_dir.dot(light_sample_normal);
            const auto pdf_light_sample = (light_dir_dot_light_sample_normal == 0.0f) ? 
                0.0f : (intersection_to_light_sample.magnitude_squared() * light_sample->pdf) / abs(light_dir_dot_light_sample_normal);

            // bsdf importance sampling
            const auto pdf_bsdf = mat_ptr->pdf(light_sample_dir, observer_dir, normal);

            // balanced heuristic multiple importance sampling
            const auto pdf_sum = pdf_light_sample + pdf_bsdf;
            if (pdf_sum > 0.0f) {
                i_direct += emission * mat_ptr->contribution(light_sample_dir, observer_dir, normal) * abs(light_sample_dir.dot(normal)) / pdf_sum;
            }
        }
    }

    // Indirect illumination
    Vector3f i_indirect = { 0.0f, 0.0f, 0.0f };

    // Use Russian Roulette to limit the recursion depth
    if (get_random_float() < scene.russian_roulette()) {
        // sample a direction for indirect illumination
        const auto indirect_light_source_dir = mat_ptr->sample_ray_source_dir(observer_dir, normal);

        // bsdf importance sampling
        const auto pdf_bsdf = mat_ptr->pdf(indirect_light_source_dir, observer_dir, normal);

        if (pdf_bsdf > 0.0f) {
            const auto next_culling = indirect_light_source_dir.dot(normal) > 0.0f ? Culling::BACK : Culling::FRONT;
            i_indirect = cast_ray(scene, { pos, indirect_light_source_dir }, next_culling)
                * mat_ptr->contribution(indirect_light_source_dir, observer_dir, normal)
                * abs(indirect_light_source_dir.dot(normal))
                / (pdf_bsdf * scene.russian_roulette());
        }
    }

    return  i_direct + i_indirect;
}

void Renderer::render_thread(unsigned int total_thread_count, unsigned int thread_id, const Scene& scene, unsigned int spp, std::vector<Vector3f>& frame_buffer) const {
    const auto scale = std::tan(degree_to_rad(scene.fov() * 0.5f));
    const auto image_aspect_ratio = static_cast<float>(scene.width()) / static_cast<float>(scene.height());

    const auto pixel_count = scene.width() * scene.height();

    // Use the amount of threads as interval to avoid mutex locking.
    for (auto pixel_idx = thread_id; pixel_idx < pixel_count; pixel_idx += total_thread_count) {
        const auto pixel_row = pixel_idx / scene.width();
        const auto pixel_col = pixel_idx % scene.width();

        Vector3f color(0.0f);
        for (unsigned int k = 0; k < spp; k++) {
            // generate primary ray direction for each sample
            const auto x = (2 * (static_cast<float>(pixel_col) + get_random_float()) / static_cast<float>(scene.width()) - 1.0f) * scale * image_aspect_ratio;
            const auto y = (1.0f - 2 * (static_cast<float>(pixel_row) + get_random_float()) / static_cast<float>(scene.height())) * scale;

            const auto dir = Vector3f(-x, y, 1.0f).normalized();
            const Ray ray(scene.eye_pos(), dir);

            const auto intersection = scene.intersect(ray, Culling::BACK);
            if (intersection && intersection->mat_ptr->emitting()) {
                // hit light source directly
                color += intersection->mat_ptr->emission(intersection->uv.x, intersection->uv.y);
            }
            // do path tracing
            color += cast_ray(scene, ray, Culling::BACK);
        }
        frame_buffer[pixel_idx] = color / static_cast<float>(spp);

        if (thread_id == 0) {
            update_progress(static_cast<float>(pixel_row) / static_cast<float>(scene.height()));
        }
    }
}