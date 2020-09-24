#pragma once
#include "Scene.hpp"

struct Hit_payload {
    float t_near;
    uint32_t index;
    Eigen::Vector2f uv;
    std::shared_ptr<Object> obj_ptr;
};

class Renderer {
public:
    void Render(const Scene& scene);
};