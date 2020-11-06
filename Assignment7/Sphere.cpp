#include "Sphere.hpp"

std::optional<Intersection> Sphere::intersect(const Ray& ray) {
    const auto l = ray.ori - _center;
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
    const auto r = intersection.pos - _center;
    intersection.normal = r.normalized();
    intersection.uv = {
            std::atan(intersection.pos.y / intersection.pos.x) * INV_2PI,

            std::asin(intersection.pos.z / r.magnitude()) * INV_PI + 0.5f
    };
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

std::optional<Sample> Sphere::sample() {
    const auto theta = 2.0f * PI * get_random_float();
    const auto phi = PI * get_random_float();

    Intersection intersection;
    intersection.normal = {
        std::cos(phi),
        std::sin(phi) * std::cos(theta),
        std::sin(phi) * std::sin(theta)
    };
    intersection.pos = _center + _radius * intersection.normal;
    intersection.uv = {phi * INV_2PI, 1.0f - theta * INV_PI};
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    const auto pdf = 1.0f / _area;

    return {{intersection, pdf}};
}