#include <chrono>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv) {
    Scene scene(500, 500, {278.0f, 273.0f, -800.0f}, 40.0f);
    constexpr unsigned int spp = 16;
    constexpr unsigned int total_thread_count = 16;

    const auto red_diffuse_mat_ptr = std::make_shared<Diffuse>(Vector3f(0.63f, 0.065f, 0.05f));
    const auto green_diffuse_mat_ptr = std::make_shared<Diffuse>(Vector3f(0.14f, 0.45f, 0.091f));
    const auto white_diffuse_mat_ptr = std::make_shared<Diffuse>(Vector3f(0.725f, 0.71f, 0.68f));

    const auto light_emission = Vector3f(8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f)
                                     + 15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f)
                                     + 18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f));
    const auto light_mat_ptr = std::make_shared<Diffuse>(Vector3f(0.65f), light_emission);

    const auto white_marble_mat_ptr = std::make_shared<MetalRough>(Vector3f(0.825f, 0.81f, 0.78f), 0.01f, 0.1f);
    const auto white_silver_mat_ptr = std::make_shared<MetalRough>(Vector3f(0.95f, 0.93f, 0.88f), 0.01f, 1.0f);

    const auto ceil_floor_back_wall_triangles = load_triangles_from_model_file("../models/cornellbox/floor.obj", white_marble_mat_ptr);
    const auto left_wall_triangles = load_triangles_from_model_file("../models/cornellbox/left.obj", red_diffuse_mat_ptr);
    const auto right_wall_triangles = load_triangles_from_model_file("../models/cornellbox/right.obj", green_diffuse_mat_ptr);
    const auto short_box_triangles = load_triangles_from_model_file("../models/cornellbox/shortbox.obj", white_diffuse_mat_ptr);
    const auto tall_box_triangles = load_triangles_from_model_file("../models/cornellbox/tallbox.obj", white_silver_mat_ptr);
    const auto ceiling_lamp_triangles = load_triangles_from_model_file("../models/cornellbox/light.obj", light_mat_ptr);

    std::cout << " - Generating triangle mesh for ceiling, floor and back wall..." << std::endl;
    const auto ceil_floor_back_wall_ptr = std::make_shared<TriangleMesh>(ceil_floor_back_wall_triangles, BVH_tree::SplitMethod::NAIVE);
    std::cout << " - Generating triangle mesh for left wall..." << std::endl;
    const auto left_wall_ptr = std::make_shared<TriangleMesh>(left_wall_triangles, BVH_tree::SplitMethod::NAIVE);
    std::cout << " - Generating triangle mesh for right wall..." << std::endl;
    const auto right_wall_ptr = std::make_shared<TriangleMesh>(right_wall_triangles, BVH_tree::SplitMethod::NAIVE);
    std::cout << " - Generating triangle mesh for short box..." << std::endl;
    const auto short_box_ptr = std::make_shared<TriangleMesh>(short_box_triangles, BVH_tree::SplitMethod::NAIVE);
    std::cout << " - Generating triangle mesh for tall box..." << std::endl;
    const auto tall_box_ptr = std::make_shared<TriangleMesh>(tall_box_triangles, BVH_tree::SplitMethod::NAIVE);
    std::cout << " - Generating triangle mesh for ceiling lamp..." << std::endl;
    const auto ceiling_lamp_ptr = std::make_shared<TriangleMesh>(ceiling_lamp_triangles, BVH_tree::SplitMethod::NAIVE);

    scene.add_object(ceil_floor_back_wall_ptr);
    scene.add_object(left_wall_ptr);
    scene.add_object(right_wall_ptr);
    scene.add_object(short_box_ptr);
    scene.add_object(tall_box_ptr);
    scene.add_object(ceiling_lamp_ptr);

    scene.build_BVH();

    Renderer r;

    std::cout << " - Rendering Scene..." << std::endl;
    const auto start = std::chrono::system_clock::now();
    r.render(scene, spp, total_thread_count);
    const auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}