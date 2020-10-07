#include "Triangle.hpp"

bool ray_triangle_intersect(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, 
                            const Eigen::Vector3f& ori, const Eigen::Vector3f& dir,
                            float& t_near, float& u, float& v) {
    // Test whether the triangle that's specified by v0, v1 and v2
    // intersects with the ray (whose origin is *orig* and direction is *dir*)

    // Moller-Trumbore algorithm
    const auto E1 = v1 - v0;
    const auto E2 = v2 - v0;
    const auto S = ori - v0;
    const auto S1 = dir.cross(E2);
    const auto S2 = S.cross(E1);
    const auto inv_denominator = 1.0f / S1.dot(E1);
    const auto t = S2.dot(E2) * inv_denominator;
    const auto b1 = S1.dot(S) * inv_denominator;
    const auto b2 = S2.dot(dir) * inv_denominator;

    t_near = t;
    u = b1;
    v = b2;

    return (t>0) && (b1>0) && (b2>0) && (1.0f-b1-b2>0);
}

Triangle_mesh::Triangle_mesh(const std::vector<Eigen::Vector3f>& vertex_list, const std::vector<uint32_t>& index_list,
                            const std::vector<Eigen::Vector2f>& st_list) {
    const auto vertex_num = vertex_list.size();
    const auto index_num = index_list.size();

    tri_num = index_num / 3;
        
    vertices = std::unique_ptr<Eigen::Vector3f[]>(new Eigen::Vector3f[vertex_num]);
    memcpy(vertices.get(), vertex_list.data(), sizeof(Eigen::Vector3f) * vertex_num);

    indices = std::unique_ptr<uint32_t[]>(new uint32_t[index_num]);
    memcpy(indices.get(), index_list.data(), sizeof(uint32_t) * index_num);

    st_coords = std::unique_ptr<Eigen::Vector2f[]>(new Eigen::Vector2f[vertex_num]);
    memcpy(st_coords.get(), st_list.data(), sizeof(Eigen::Vector2f) * vertex_num);
}

bool Triangle_mesh::intersect(const Eigen::Vector3f& ori, const Eigen::Vector3f& dir,
                    float& t_near, uint32_t& index, Eigen::Vector2f& uv) const {
    bool intersect = false;
    for (uint32_t k = 0; k < tri_num; ++k) {
        const auto base_idx = k * 3;
        const auto& v0 = vertices[indices[base_idx]];
        const auto& v1 = vertices[indices[base_idx + 1]];
        const auto& v2 = vertices[indices[base_idx + 2]];

        float t, u, v;
        if (ray_triangle_intersect(v0, v1, v2, ori, dir, t, u, v) && t < t_near) {
            t_near = t;
            uv = {u, v};
            index = k;
            intersect |= true;
        }
    }

    return intersect;
}

void Triangle_mesh::get_surface_properties(const Eigen::Vector3f&, const Eigen::Vector3f&, uint32_t index, const Eigen::Vector2f& uv,
                                        Eigen::Vector3f& normal, Eigen::Vector2f& st) const {
    const auto base_idx = index * 3;
    const Eigen::Vector3f &v0 = vertices[indices[base_idx]];
    const Eigen::Vector3f &v1 = vertices[indices[base_idx + 1]];
    const Eigen::Vector3f &v2 = vertices[indices[base_idx + 2]];

    normal = (v1 - v0).cross(v2 - v1).normalized();

    const auto& st0 = st_coords[indices[base_idx]];
    const auto& st1 = st_coords[indices[base_idx + 1]];
    const auto& st2 = st_coords[indices[base_idx + 2]];
    st = st0 * (1.0f - uv.x() - uv.y()) + st1 * uv.x() + st2 * uv.y();
}

Eigen::Vector3f Triangle_mesh::eval_diffuse_color(const Eigen::Vector2f& st) const {
    constexpr float scale = 5.0f;
    const float pattern = (fmodf(st.x() * scale, 1.0f) > 0.5f) ^ (fmodf(st.y() * scale, 1.0f) > 0.5f);
    return lerp({0.815f, 0.235f, 0.031f}, {0.937f, 0.937f, 0.231f}, pattern);
}