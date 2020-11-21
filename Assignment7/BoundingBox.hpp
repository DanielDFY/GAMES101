#pragma once

#include "Ray.hpp"
#include "Math.hpp"

struct BoundingBox {
    enum class Axis { AXIS_X, AXIS_Y, AXIS_Z };
    static int axis_to_dim(Axis axis) {
        switch (axis) {
            case Axis::AXIS_X:
                return 0; 
            case Axis::AXIS_Y:
                return 1; 
            case Axis::AXIS_Z:
                return 2; 
            default:
                throw std::runtime_error("unknown axis");
        }
    }

    // two points to specify the bounding box
    Vector3f p_min = {FLOAT_INFINITY, FLOAT_INFINITY , FLOAT_INFINITY };
    Vector3f p_max = {FLOAT_LOWEST, FLOAT_LOWEST, FLOAT_LOWEST};

    BoundingBox() = default;
    BoundingBox(const Vector3f& p) : p_min(p), p_max(p) {}
    BoundingBox(const Vector3f& p1, const Vector3f& p2)
     : p_min(Vector3f::min_elems(p1, p2)),
       p_max(Vector3f::max_elems(p1, p2)) {}

    Vector3f diagonal() const { return p_max - p_min; }
    Vector3f centroid() const { return 0.5f * p_min + 0.5f * p_max; }
    Axis max_extent() const;
    float surface_area() const {
        const auto d = diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    bool intersect(const Ray& ray) const;

    // offset ratio from *p_min* to *p_max* on each axis
    Vector3f offset_ratio(const Vector3f& p) const;

    bool overlaps(const BoundingBox& box) const {
        const auto x = (p_max.x >= box.p_min.x) && (p_min.x <= box.p_max.x);
        const auto y = (p_max.y >= box.p_min.y) && (p_min.y <= box.p_max.y);
        const auto z = (p_max.z >= box.p_min.z) && (p_min.z <= box.p_max.z);
        return (x && y && z);
    }

    bool inside(const Vector3f& p) const {
        return p.x >= p_min.x && p.x <= p_max.x
            && p.y >= p_min.y && p.y <= p_max.y
            && p.z >= p_min.z && p.z <= p_max.z;
    }
};

inline BoundingBox intersection_box(const BoundingBox& box1, const BoundingBox& box2) {
    return {
        Vector3f::max_elems(box1.p_min, box2.p_min),
        Vector3f::min_elems(box1.p_max, box2.p_max),
    };
}

inline BoundingBox union_box(const BoundingBox& box1, const BoundingBox& box2) {
    BoundingBox ret;
    ret.p_min = Vector3f::min_elems(box1.p_min, box2.p_min);
    ret.p_max = Vector3f::max_elems(box1.p_max, box2.p_max);
    return ret;
}

inline BoundingBox union_box(const BoundingBox& b, const Vector3f& p) {
    BoundingBox ret;
    ret.p_min = Vector3f::min_elems(b.p_min, p);
    ret.p_max = Vector3f::max_elems(b.p_max, p);
    return ret;
}