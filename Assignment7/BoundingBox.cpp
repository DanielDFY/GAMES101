#include "BoundingBox.hpp"

BoundingBox::Axis BoundingBox::max_extent() const {
    const auto d = diagonal();
    if (d.x > d.y && d.x > d.z)
        return Axis::AXIS_X;
    else if (d.y > d.z)
        return Axis::AXIS_Y;
    else
        return Axis::AXIS_Z;
}

bool BoundingBox::intersect(const Ray& ray) const {
    // Test if ray bound intersects
    const auto t_pmax = (p_max - ray.ori) * ray.inv_dir;
    const auto t_pmin = (p_min - ray.ori) * ray.inv_dir;

    // t_min & t_max along x/y/z-axis
    const auto t_min = Vector3f::min_elems(t_pmin, t_pmax);
    const auto t_max = Vector3f::max_elems(t_pmin, t_pmax);

    const auto t_enter = std::max(std::max(t_min.x, t_min.y), t_min.z);
    const auto t_exit = std::min(std::min(t_max.x, t_max.y), t_max.z);

    return (t_exit >= t_enter) && (t_exit > 0.0f);
}

Vector3f BoundingBox::offset_ratio(const Vector3f& p) const {
    Vector3f o = p - p_min;
    if (p_max.x > p_min.x)
        o.x /= p_max.x - p_min.x;
    if (p_max.y > p_min.y)
        o.y /= p_max.y - p_min.y;
    if (p_max.z > p_min.z)
        o.z /= p_max.z - p_min.z;
    return o;
}