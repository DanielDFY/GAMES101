#include "Ray.hpp"

#include "Utility.hpp"

Ray::Ray(const Vector3f& origin, const Vector3f& direction, float time)
    : ori(origin), dir(direction), t(time), t_min(0.0f), t_max(FLOAT_INFINITY) {
    inv_dir = {1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z};
}

std::ostream& operator<<(std::ostream& os, const Ray& r){
    os << "[origin:=" << r.ori << ", direction=" << r.dir << ", time=" << r.t<<"]" << std::endl;
    return os;
}