#include "Sphere.hpp"

std::optional<Intersection> Sphere::intersect(const Ray& ray) {
    const Vector3f l = ray.ori - _center;
    const auto a = ray.dir.magnitude_squared();
    const auto b = 2 * ray.dir.dot(l);
    const auto c = l.magnitude_squared() - _radius_sq;

    float t0, t1;
    if (!solve_quadratic(a, b, c, t0, t1))
        return std::nullopt;

    if (t0 < 0)
        t0 = t1;
    if (t0 < 0)
        return std::nullopt;
    
    Intersection intersection;
    intersection.time = t0;
    intersection.pos = ray.at_time(t0);
    intersection.normal = (intersection.pos - _center).normalized();
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    return intersection;
}

Bounding_box Sphere::bound() const {
    return {
        {_center.x-_radius, _center.y-_radius, _center.z-_radius},
        {_center.x+_radius, _center.y+_radius, _center.z+_radius}
    };
}