#pragma once

#include <memory>
#include <optional>

#include "Object.hpp"
#include "Bounding_box.hpp"
#include "Material.hpp"

class Sphere : public Object, public std::enable_shared_from_this<Sphere> {
public:
    Sphere(const Vector3f& center, float radius, std::shared_ptr<Material> material_ptr = std::make_shared<Material>())
     : _center(center), _radius(radius), _radius_sq(radius * radius), _mat_ptr(material_ptr) {}

    std::optional<Intersection> intersect(const Ray& ray) override;

    Bounding_box bound() const override;

    [[nodiscard]] Vector3f center() { return _center; }
    [[nodiscard]] float radius() { return _radius; }
    [[nodiscard]] float radius_sqared() { return _radius_sq; }
    

private:
    Vector3f _center;
    float _radius, _radius_sq;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ;
    std::shared_ptr<Material> _mat_ptr;
};