#pragma once

#include <limits>
#include <array>

#include "Ray.hpp"
#include "Utility.hpp"

inline Vector3f point_with_min_coords(const Vector3f& p1, const Vector3f& p2) {
    return {std::fmin(p1.x, p2.x), std::fmin(p1.y, p2.y), std::fmin(p1.z, p2.z)};
}

inline Vector3f point_with_max_coords(const Vector3f& p1, const Vector3f& p2) {
    return {std::fmax(p1.x, p2.x), std::fmax(p1.y, p2.y), std::fmax(p1.z, p2.z)};
}

struct Bounding_box {
    enum class Axis { AXIS_X, AXIS_Y, AXIS_Z };
    static int axis_to_dim(Axis axis) {
        switch (axis) {
            case Axis::AXIS_X:
                return 0; 
                break;
            case Bounding_box::Axis::AXIS_Y:
                return 1; 
                break;
            case Bounding_box::Axis::AXIS_Z:
                return 2; 
                break;
        }
    }

    // two points to specify the bounding box
    Vector3f p_min = {FLOAT_INFINITY, FLOAT_INFINITY, FLOAT_INFINITY};
    Vector3f p_max = {FLOAT_LOWEST, FLOAT_LOWEST, FLOAT_LOWEST};

    Bounding_box() = default;
    Bounding_box(const Vector3f& p) : p_min(p), p_max(p) {}
    Bounding_box(const Vector3f& p1, const Vector3f& p2) {
        p_min = point_with_min_coords(p1, p2);
        p_max = point_with_max_coords(p1, p2);
    }

    [[nodiscard]] Vector3f diagonal() const { return p_max - p_min; }

    [[nodiscard]] Vector3f centroid() const { return 0.5f * p_min + 0.5f * p_max; }
    
    [[nodiscard]] Axis max_extent() const;

    [[nodiscard]] float surface_area() const {
        const auto d = diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    [[nodiscard]] bool intersect(const Ray& ray) const;

    // offset ratio from *p_min* to *p_max* on each axis
    [[nodiscard]] Vector3f offset_ratio(const Vector3f& p) const;

    [[nodiscard]] bool overlaps(const Bounding_box& box) const {
        const bool x = (p_max.x >= box.p_min.x) && (p_min.x <= box.p_max.x);
        const bool y = (p_max.y >= box.p_min.y) && (p_min.y <= box.p_max.y);
        const bool z = (p_max.z >= box.p_min.z) && (p_min.z <= box.p_max.z);
        return (x && y && z);
    }

    [[nodiscard]] bool inside(const Vector3f& p) const {
        return p.x >= p_min.x && p.x <= p_max.x
            && p.y >= p_min.y && p.y <= p_max.y
            && p.z >= p_min.z && p.z <= p_max.z;
    }
};

inline Bounding_box intersection_box(const Bounding_box& box1, const Bounding_box& box2) {
    return {
        {std::fmax(box1.p_min.x, box2.p_min.x), std::fmax(box1.p_min.y, box2.p_min.y), std::fmax(box1.p_min.z, box2.p_min.z)},
        {std::fmin(box1.p_max.x, box2.p_max.x), std::fmin(box1.p_max.y, box2.p_max.y), std::fmin(box1.p_max.z, box2.p_max.z)},
    };
}

inline Bounding_box union_box(const Bounding_box& box1, const Bounding_box& box2) {
    Bounding_box ret;
    ret.p_min = point_with_min_coords(box1.p_min, box2.p_min);
    ret.p_max = point_with_max_coords(box1.p_max, box2.p_max);
    return ret;
}

inline Bounding_box union_box(const Bounding_box& b, const Vector3f& p) {
    Bounding_box ret;
    ret.p_min = point_with_min_coords(b.p_min, p);
    ret.p_max = point_with_max_coords(b.p_max, p);
    return ret;
}