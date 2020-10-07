#pragma once

#include "Scene.hpp"

class Renderer {
public:
    // The main render function. This where we iterate over all pixels in the image,
    // generate primary rays and cast these rays into the scene. The content of the
    // framebuffer is saved to a png image file with tools from stb library.
    void render(const Scene& scene);

private:
    // Implementation of the Whitted-syle light transport algorithm
    //
    // This function is the function that compute the color at the intersection point
    // of a ray defined by a position and a direction. Note that thus function is recursive.
    //
    // If the material of the intersected object is either reflective or reflective and refractive,
    // then we compute the reflection/refracton direction and cast two new rays into the scene
    // by calling the cast_ray() function recursively. When the surface is transparent, we mix
    // the reflection and refraction color using the result of the fresnel equations (it computes
    // the amount of reflection and refractin depending on the surface normal, incident view direction
    // and surface refractive index).
    //
    // If the surface is duffuse/glossy we use the Phong illumation model to compute the color
    // at the intersection point.
    Vector3f cast_ray(const Scene& scene, const Ray& ray, int depth) const;
};
