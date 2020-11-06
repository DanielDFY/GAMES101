#pragma once

#include <optional>
#include <memory>
#include <vector>

#include "Bounding_box.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

struct Sample {
    Intersection intersection;
    float pdf = 0.0f;
};

class Object {
public:
    Object() = default;
    virtual ~Object() = default;
	
    virtual float area() const = 0;
    virtual Bounding_box bound() const = 0;
    virtual bool emitting() const = 0;
	
    [[nodiscard]] virtual std::optional<Intersection> intersect(const Ray& ray) = 0;
    [[nodiscard]] virtual std::optional<Sample> sample() = 0;
    
};

// Deal with polymorphism
template <typename T, typename = typename std::enable_if<std::is_base_of<Object, T>::value>::type>
[[nodiscard]] std::vector<std::shared_ptr<Object>> transform_to_object_vector(const std::vector<std::shared_ptr<T>>& obj_ptr_list) {
    std::vector<std::shared_ptr<Object>> obj_ptrs(obj_ptr_list.size());
    std::transform(obj_ptr_list.begin(), obj_ptr_list.end(), obj_ptrs.begin(), [](const auto& obj_ptr) { return std::static_pointer_cast<Object>(obj_ptr);});
    return obj_ptrs;
}