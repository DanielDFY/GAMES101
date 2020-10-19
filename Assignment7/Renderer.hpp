#pragma once

#include "Scene.hpp"

class Renderer {
public:
    // The main render function. This where we iterate over all pixels in the image,
    // generate primary rays and cast these rays into the scene. The content of the
    // framebuffer is saved to a png image file with tools from stb library.
    //
    // Change the spp value to change sample amount.
    void render(const Scene& scene, unsigned int spp);

private:
    // Implementation of the path tracing algorithm
    //
    // This function cast the given ray in the given scene and does shading
    // at the intersection point.
    //
    // Russian Roulette method is applied to limit the depth of recursion.
    //
    // If hit nothing, return the background color.
    //
    // If hit any illuminant, return its emission.
    //
    // If hit other objects, the return illumination is composed of two parts:
    //     1. From the direct illumination of the light sources.
    //     2. From the indirect illumination of the other objects that reflect
    //        the emission of the light sources.
    [[nodiscard]] Vector3f cast_ray(const Scene& scene, const Ray& ray) const;
};
