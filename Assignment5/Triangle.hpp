#pragma once

#include <memory>
#include <cstring>

#include "Object.hpp"

bool ray_triangle_intersect(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, 
                            const Eigen::Vector3f& orig, const Eigen::Vector3f& dir, float& t_near,
                            float& u, float& v);

class Triangle_mesh : public Object {
public:
    Triangle_mesh(const std::vector<Eigen::Vector3f>& vertex_list, const std::vector<uint32_t>& index_list,
                const std::vector<Eigen::Vector2f>& st_list);

    bool intersect(const Eigen::Vector3f& ori, const Eigen::Vector3f& dir,
                float& t_near, uint32_t& index, Eigen::Vector2f& uv) const override;

    void get_surface_properties(const Eigen::Vector3f&, const Eigen::Vector3f&, uint32_t index, const Eigen::Vector2f& uv,
                                Eigen::Vector3f& normal, Eigen::Vector2f& st) const override;

    Eigen::Vector3f eval_diffuse_color(const Eigen::Vector2f& st) const override;

    uint32_t tri_num;
    std::unique_ptr<Eigen::Vector3f[]> vertices;
    std::unique_ptr<uint32_t[]> indices;
    std::unique_ptr<Eigen::Vector2f[]> st_coords;
};