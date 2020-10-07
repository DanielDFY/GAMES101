#pragma once

#include <iostream>
#include <cmath>
#include <algorithm>

class Vector3f {
public:
    Vector3f() : x(0), y(0), z(0) {}
    explicit Vector3f(float k) : x(k), y(k), z(k) {}
    Vector3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

    Vector3f operator -() const { return {-x, -y, -z}; }

    Vector3f operator +(const Vector3f& v) const { return {x + v.x, y + v.y, z + v.z}; }

    Vector3f operator -(const Vector3f& v) const { return {x - v.x, y - v.y, z - v.z}; }

    Vector3f operator *(const Vector3f& v) const { return {x * v.x, y * v.y, z * v.z}; }
    Vector3f operator *(float k) const { return {x * k, y * k, z * k}; }

    Vector3f operator /(float k) const { const auto inv_k = 1 / k; return {x * inv_k, y * inv_k, z * inv_k};}

    Vector3f& operator +=(const Vector3f& v) { x += v.x, y += v.y, z += v.z; return *this; }

    float operator [](int index) const { return (&x)[index]; }
    float& operator [](int index) { return (&x)[index]; }

    [[nodiscard]] Vector3f normalized() const {
        const auto mag_sq = magnitude_squared();
        if (mag_sq > 0) {
            const auto inv_mag = 1 / sqrtf(mag_sq);
            return {x * inv_mag, y * inv_mag, z * inv_mag};
        } else {
            return {0.0f, 0.0f, 0.0f};
        }
    }

    [[nodiscard]] float magnitude_squared() const { return x * x + y * y + z * z;}

    [[nodiscard]] float magnitude() const { return sqrtf(x * x + y * y + z * z); }

    [[nodiscard]] float dot(const Vector3f rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

    [[nodiscard]] Vector3f cross(const Vector3f rhs) const { return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x}; }

public:
    float x, y, z;
};

inline Vector3f operator *(float k, const Vector3f& v) { return {v.x * k, v.y * k, v.z * k}; }

inline std::ostream& operator <<(std::ostream& os, const Vector3f& v) { return os << v.x << ", " << v.y << ", " << v.z; }

inline Vector3f point_with_min_coords(const Vector3f& p1, const Vector3f& p2) {
    return {std::fmin(p1.x, p2.x), std::fmin(p1.y, p2.y), std::fmin(p1.z, p2.z)};
}

inline Vector3f point_with_max_coords(const Vector3f& p1, const Vector3f& p2) {
    return {std::fmax(p1.x, p2.x), std::fmax(p1.y, p2.y), std::fmax(p1.z, p2.z)};
}

class Vector2f {
public:
    Vector2f() : x(0), y(0) {}
    explicit Vector2f(float k) : x(k), y(k) {}
    Vector2f(float xx, float yy) : x(xx), y(yy) {}
    Vector2f operator *(float k) const { return Vector2f(x * k, y * k); }
    Vector2f operator +(const Vector2f& v) const { return Vector2f(x + v.x, y + v.y); }
    float x, y;
};

inline Vector2f operator *(float k, const Vector2f& v) { return {v.x * k, v.y * k}; }

inline std::ostream& operator <<(std::ostream& os, const Vector2f& v) { return os << v.x << ", " << v.y; }