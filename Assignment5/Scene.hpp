#pragma once

#include <vector>
#include <memory>

#include "Object.hpp"
#include "Light.hpp"

class Scene {
public:
    // setting up options
    int width = 1280;
    int height = 960;
    float fov = 90.0f;
    Eigen::Vector3f background_color = Eigen::Vector3f(0.235294f, 0.67451f, 0.843137f);
    int max_depth = 5;
    float epsilon = 0.00001f;

    Scene(int w, int h) : width(w), height(h) {}

    void Add(std::shared_ptr<Object> object_ptr) { objects.push_back(object_ptr); }
    void Add(std::shared_ptr<Light> light_ptr) { lights.push_back(light_ptr); }

    [[nodiscard]] const std::vector<std::shared_ptr<Object>>& get_objects() const { return objects; }
    [[nodiscard]] const std::vector<std::shared_ptr<Light>>&  get_lights() const { return lights; }

private:
    // creating the scene (adding objects and lights)
    std::vector<std::shared_ptr<Object> > objects;
    std::vector<std::shared_ptr<Light> > lights;
};