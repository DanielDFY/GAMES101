#pragma once

constexpr float MY_PI = 3.1415926f;

inline Eigen::Vector2f interpolate(float alpha, float beta, float gamma,
                            const Eigen::Vector2f& vert1, const Eigen::Vector2f& vert2, const Eigen::Vector2f& vert3) {
    const auto u = (alpha * vert1[0] + beta * vert2[0] + gamma * vert3[0]);
    const auto v = (alpha * vert1[1] + beta * vert2[1] + gamma * vert3[1]);

    return {u, v};
}

inline Eigen::Vector3f interpolate(float alpha, float beta, float gamma,
                            const Eigen::Vector3f& vert1, const Eigen::Vector3f& vert2, const Eigen::Vector3f& vert3) {
    return (alpha * vert1 + beta * vert2 + gamma * vert3);
}

inline Eigen::Vector3f interpolate(float alpha, float beta, float gamma,
                            const Eigen::Vector4f& vert1, const Eigen::Vector4f& vert2, const Eigen::Vector4f& vert3) {
    const auto vert1_vector3 = vert1.head<3>() / vert1.w();
    const auto vert2_vector3 = vert2.head<3>() / vert2.w();
    const auto vert3_vector3 = vert3.head<3>() / vert3.w();
    
    return (alpha * vert1_vector3 + beta * vert2_vector3 + gamma * vert3_vector3);
}

inline Eigen::Vector3f reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis) {
    const auto costheta = vec.dot(axis);
    return (2 * costheta * axis - vec).normalized();
}