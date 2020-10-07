#pragma once

#include "Scene.hpp"

struct Hit_payload {
    float t_near;
    uint32_t index;
    Eigen::Vector2f uv;
    std::shared_ptr<Object> obj_ptr;
};

Eigen::Vector3f reflect(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal);

Eigen::Vector3f refract(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal, float ior);

float fresnel(const Eigen::Vector3f& ray_in, const Eigen::Vector3f& normal, float ior);

std::optional<Hit_payload> trace(const Eigen::Vector3f& ori,
                                const Eigen::Vector3f& dir,
                                const std::vector<std::shared_ptr<Object>>& obj_ptr_list);

Eigen::Vector3f cast_ray( const Eigen::Vector3f& ori, const Eigen::Vector3f& dir, const Scene& scene, int depth);


class Renderer {
public:
    void render(const Scene& scene);
};