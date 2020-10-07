// Bonus part
// #define SVH

#include <chrono>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv) {
    Scene scene(1280, 960);

    const auto bunny_triangles = load_triangles_from_model_file("../models/bunny/bunny.obj");

    #ifdef SVH
    std::cout << " - Generating BVH for Bunny with SAH..." << std::endl;
    const auto bunny_ptr = std::make_shared<Triangle_mesh>(bunny_triangles, BVH_tree::Split_method::SAH);
    #else
    std::cout << " - Generating BVH for Bunny..." << std::endl;
    const auto bunny_ptr = std::make_shared<Triangle_mesh>(bunny_triangles, BVH_tree::Split_method::NAIVE);
    #endif

    scene.add_object(bunny_ptr);

    scene.add_light(std::make_shared<Light>(Vector3f{-20.0f, 70.0f, 20.0f}, Vector3f{1.0f, 1.0f, 1.0f}));
    scene.add_light(std::make_shared<Light>(Vector3f{20.0f, 70.0f, 20.0f}, Vector3f{1.0f, 1.0f, 1.0f}));
    #ifdef SVH
    scene.build_SVH();
    #else
    scene.build_BVH();
    #endif

    Renderer r;

    std::cout << " - Rendering Scene..." << std::endl;
    const auto start = std::chrono::system_clock::now();
    r.render(scene);
    const auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}