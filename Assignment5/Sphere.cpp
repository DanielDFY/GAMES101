#include "Sphere.hpp"

bool Sphere::intersect(const Eigen::Vector3f& ori, const Eigen::Vector3f& dir, float& t_near, uint32_t&, Eigen::Vector2f&) const {
    // analytic solution
    const auto L = ori - center;
    const auto a = dir.squaredNorm();
    const auto b = 2 * dir.dot(L);
    const auto c = L.squaredNorm() - radius_squared;
    
    float t0, t1;

    // no intersection
    if (!solve_quadratic(a, b, c, t0, t1))
        return false;

    if (t0 < 0)
        t0 = t1;
    if (t0 < 0)
        return false;
        
    // record intersection
    t_near = t0;

    return true;
}