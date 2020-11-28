#include "Object.hpp"

std::optional<Intersection> Sphere::intersect(const Ray& ray, Culling culling) {
    const auto l = ray.ori - _center;
    const auto a = ray.dir.magnitude_squared();
    const auto b = 2 * ray.dir.dot(l);
    const auto c = l.magnitude_squared() - _radius_sq;

    const auto result = solve_quadratic(a, b, c);
    if (!result) return std::nullopt;

    const auto& [t0, t1] = result.value();

    float t;
    if (culling == Culling::BACK) {
        t = t0;
    } else if (culling == Culling::FRONT) {
        t = t1;
    } else {
        t = (t0 > 0.0f) ? t0 : t1;
    }

    if (t <= 0.0f)
        return std::nullopt;

    Intersection intersection;
    intersection.time = t0;
    intersection.pos = ray.at_time(t0);
    const auto r = intersection.pos - _center;
    intersection.normal = r.normalized();
    intersection.uv = {
        std::atan(intersection.pos.y / intersection.pos.x) * 0.5f * INV_PI,
        std::asin(intersection.pos.z / r.magnitude()) * INV_PI + 0.5f
    };
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    return intersection;
}


BoundingBox Sphere::bound() const {
    return {
        {_center.x - _radius, _center.y - _radius, _center.z - _radius},
        {_center.x + _radius, _center.y + _radius, _center.z + _radius}
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
    intersection.uv = { phi * 0.5f * INV_PI, 1.0f - theta * INV_PI };
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    const auto pdf = 1.0f / _area;

    return { {intersection, pdf} };
}

Triangle::Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2,
                std::shared_ptr<Material> material_ptr)
 : _v0(v0), _v1(v1), _v2(v2), _mat_ptr(std::move(material_ptr)) {
    _e1 = _v1 - _v0;
    _e2 = _v2 - _v0;

    // default
    _t0 = _t1 = _t2 = {0.0f, 0.0f};

    const auto n = _e1.cross(_e2);
    _normal = n.normalized();
    _area = 0.5f * n.magnitude() ;
}

std::optional<Intersection> Triangle::intersect(const Ray& ray, Culling culling) {
    // Check surface direction
    const auto dir_dot_normal = ray.dir.dot(_normal);
    if ((culling == Culling::BACK && dir_dot_normal > 0.0f) || (culling == Culling::FRONT && dir_dot_normal < 0.0f))
        return std::nullopt;

    // Moller-Trumbore algorithm
    const auto S = ray.ori - _v0;
    const auto S1 = ray.dir.cross(_e2);
    const auto S2 = S.cross(_e1);
    const auto denominator = S1.dot(_e1);
    if (denominator == 0.0f)
        return std::nullopt;

    const auto inv_denominator = 1.0f / denominator;
    const auto t = S2.dot(_e2) * inv_denominator;
    const auto b1 = S1.dot(S) * inv_denominator;
    const auto b2 = S2.dot(ray.dir) * inv_denominator;

    const bool check_intersect = (t > 0.0f) && (b1 > 0.0f) && (b2 > 0.0f) && (1.0f - b1 - b2 > 0.0f);
    if (!check_intersect)
        return std::nullopt;

    Intersection intersection;

    intersection.time = t;
    intersection.pos = ray.at_time(t);
    intersection.normal = _normal;
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    return intersection;
}

BoundingBox Triangle::bound() const {
    return union_box({_v0, _v1}, _v2);
}

std::optional<Sample> Triangle::sample() {
    const auto x = std::sqrt(get_random_float());
    const auto y = get_random_float();

    const auto c0 = 1.0f - x;
    const auto c1 = x * (1.0f - y);
    const auto c2 = x * y;

    Intersection intersection;
    intersection.pos = _v0 * c0 + _v1 * c1 + _v2 * c2;
    intersection.normal = _normal;
    intersection.uv = _t0 * c0 + _t1 * c1 + _t2 * c2;
    intersection.obj_ptr = shared_from_this();
    intersection.mat_ptr = _mat_ptr;

    const auto pdf = 1.0f / _area;

    return {{intersection, pdf}};
}
