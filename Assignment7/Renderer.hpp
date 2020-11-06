#pragma once

#include "Scene.hpp"

class Renderer {
public:
    // The main render function. This where we iterate over all pixels in the image,
    // generate primary rays and cast these rays into the scene. The content of the
    // frame buffer is saved to a png image file with tools from stb library.
    //
    // *spp* is for sampling amount per pixel.
    // *total_thread_count* indicates how many threads are used to do multi-thread rendering.
    void render(const Scene& scene, unsigned int spp, unsigned int total_thread_count) const;

private:
    // Implementation of the path tracing algorithm
    //
    // This function cast the given ray in the given scene and does shading at the intersection point.
    //
    // If hit nothing, return the background color.
    //
    // If hit light source, return its emission.
    //
    // If hit other objects, the return illumination is composed of two parts:
    //     1. the direct illumination of the light sources with multiple
    //        importance sampling (sampling light source, sampling BRDF)
    //     2. the indirect illumination of the other objects that reflect
    //        the emission of the light sources.
    //
    // Russian Roulette method is applied to limit the depth of recursion.
    [[nodiscard]] Vector3f cast_ray(const Scene& scene, const Ray& ray) const;

    // Rendering task function for one thread
    void render_thread(unsigned int total_thread_count, unsigned int thread_id, const Scene& scene, unsigned int spp, std::vector<Vector3f>& frame_buffer) const;
};
