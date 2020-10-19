#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "Object.hpp"
#include "Bounding_box.hpp"
#include "Material.hpp"

class Sphere : public Object, public std::enable_shared_from_this<Sphere> {
public:
    Sphere(const Vector3f& center, float radius, std::shared_ptr<Material> material_ptr = std::make_shared<Material>())
     : _center(center), _radius(radius), _radius_sq(radius * radius), _area(4 * _radius_sq * PI), _mat_ptr(std::move(material_ptr)) {}

    [[nodiscard]] std::optional<Intersection> intersect(const Ray& ray) override;
    Bounding_box bound() const override;
    float area() const override { return _area; }
    [[nodiscard]] std::optional<Sample> sample() override;
    bool emitting() const override { return _mat_ptr->emitting(); }

    Vector3f center() const { return _center; }
    float radius() const { return _radius; }
    float radius_sq() const { return _radius_sq; }

private:
    Vector3f _center;
    float _radius, _radius_sq;
    float _area;
    std::shared_ptr<Material> _mat_ptr;
};