#pragma once

#include <optional>
#include <memory>
#include <vector>

#include "BoundingBox.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

enum class Culling { NONE, BACK, FRONT };

struct Sample {
    Intersection intersection;
    float pdf = 0.0f;
};

class Object {
public:
    virtual ~Object() = default;
	
    virtual float area() const = 0;
    virtual BoundingBox bound() const = 0;
    virtual bool emitting() const = 0;
	
    [[nodiscard]] virtual std::optional<Intersection> intersect(const Ray& ray, Culling culling) = 0;
    [[nodiscard]] virtual std::optional<Sample> sample() = 0;
    
};

// Deal with polymorphism
template <typename T, typename = typename std::enable_if<std::is_base_of<Object, T>::value>::type>
[[nodiscard]] std::vector<std::shared_ptr<Object>> transform_to_object_vector(const std::vector<std::shared_ptr<T>>& obj_ptr_list) {
    std::vector<std::shared_ptr<Object>> obj_ptrs(obj_ptr_list.size());
    std::transform(obj_ptr_list.begin(), obj_ptr_list.end(), obj_ptrs.begin(), [](const auto& obj_ptr) { return std::static_pointer_cast<Object>(obj_ptr);});
    return obj_ptrs;
}

class Sphere : public Object, public std::enable_shared_from_this<Sphere> {
public:
    Sphere(const Vector3f& center, float radius, std::shared_ptr<Material> material_ptr = std::make_shared<Material>())
        : _center(center), _radius(radius), _radius_sq(radius* radius), _area(4 * _radius_sq * PI), _mat_ptr(std::move(material_ptr)) {
    }

    float area() const override { return _area; }
    BoundingBox bound() const override;
    bool emitting() const override { return _mat_ptr->emitting(); }

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray, Culling culling) override;
    [[nodiscard]] std::optional<Sample> sample() override;

    Vector3f center() const { return _center; }
    float radius() const { return _radius; }
    float radius_sq() const { return _radius_sq; }

private:
    Vector3f _center;
    float _radius, _radius_sq;
    float _area;
    std::shared_ptr<Material> _mat_ptr;
};

class Triangle : public Object, public std::enable_shared_from_this<Triangle> {
public:
    Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2,
        std::shared_ptr<Material> material_ptr = std::make_shared<Material>());

    float area() const override { return _area; }
    BoundingBox bound() const override;
    bool emitting() const override { return _mat_ptr->emitting(); }

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray, Culling culling) override;
    [[nodiscard]] std::optional<Sample> sample() override;

private:
    Vector3f _v0, _v1, _v2; // vertices in counter-clockwise order
    Vector3f _e1, _e2;      // 2 edges v1-v0, v2-v0;
    Vector2f _t0, _t1, _t2; // texture coords
    Vector3f _normal;
    float _area;
    std::shared_ptr<Material> _mat_ptr;
};