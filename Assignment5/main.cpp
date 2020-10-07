#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Light.hpp"
#include "Renderer.hpp"

// In the main function of the program, we create the scene (create objects and lights)
// as well as set the options for the render (image width and height, maximum recursion
// depth, field-of-view, etc.). We then call the render function().
int main() {
    Scene scene(1280, 960);

    auto sph1 = std::make_unique<Sphere>(Eigen::Vector3f(-1.0f, 0.0f, -12.0f), 2.0f);
    sph1->material_type = Material_type::DIFFUSE_AND_GLOSSY;
    sph1->diffuse_color = Eigen::Vector3f(0.6f, 0.7f, 0.8f);

    auto sph2 = std::make_unique<Sphere>(Eigen::Vector3f(0.5f, -0.5f, -8.0f), 1.5f);
    sph2->ior = 1.5f;
    sph2->material_type = Material_type::REFLECTION_AND_REFRACTION;

    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    const std::vector<Eigen::Vector3f> vertex_list = {
        {-5.0f, -3.0f,  -6.0f},
        {5.0f,  -3.0f,  -6.0f},
        {5.0f,  -3.0f,  -16.0f},
        {-5.0f, -3.0f,  -16.0f}
    };
    const std::vector<uint32_t> index_list = {0, 1, 3, 1, 2, 3};
    const std::vector<Eigen::Vector2f> st_list = {
        {0.0f,  0.0f},
        {1.0f,  0.0f},
        {1.0f,  1.0f},
        {0.0f,  1.0f}
    };
    auto mesh = std::make_unique<Triangle_mesh>(vertex_list, index_list, st_list);
    mesh->material_type = Material_type::DIFFUSE_AND_GLOSSY;

    scene.Add(std::move(mesh));
    scene.Add(std::make_unique<Light>(Eigen::Vector3f(-20.0f, 70.0f, 20.0f), Eigen::Vector3f(0.5f, 0.5f, 0.5f)));
    scene.Add(std::make_unique<Light>(Eigen::Vector3f(30.0f, 50.0f, -12.0f), Eigen::Vector3f(0.5f, 0.5f, 0.5f)));    

    Renderer r;
    r.render(scene);

    return 0;
}