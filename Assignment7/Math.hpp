#pragma once

#include <algorithm>
#include <optional>
#include <ostream>
#include <cmath>

constexpr float PI = 3.141592653589793f;            // pi
constexpr float INV_PI = 1.0f / PI;                 // 1 / pi
constexpr float DEGREE_TO_RAD_RATIO = PI / 180.0f;  // (2 * pi) / 360
constexpr float EPSILON = 0.000001f;

inline float degree_to_rad(float degree) { return degree * DEGREE_TO_RAD_RATIO; }

constexpr auto FLOAT_INFINITY = std::numeric_limits<float>::max();
constexpr auto FLOAT_LOWEST = std::numeric_limits<float>::lowest();

inline float clamp(float lo, float hi, float v) { return std::max(lo, std::min(hi, v)); }

inline float lerp(float x, float y, float t) { return x * (1.0f - t) + y * t; }

std::optional<std::tuple<float, float>> solve_quadratic(float a, float b, float c);

float get_random_float();

struct Vector3f {
    float x, y, z;

    Vector3f() : x(0), y(0), z(0) {}
    Vector3f(float k) : x(k), y(k), z(k) {}
    Vector3f(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}

    static Vector3f min_elems(const Vector3f& v1, const Vector3f& v2) {
        return { std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z) };
    }

    static Vector3f max_elems(const Vector3f& v1, const Vector3f& v2) {
        return { std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z) };
    }

    Vector3f operator -() const { return { -x, -y, -z }; }

    Vector3f operator +(const Vector3f& v) const { return { x + v.x, y + v.y, z + v.z }; }

    Vector3f operator -(const Vector3f& v) const { return { x - v.x, y - v.y, z - v.z }; }

    Vector3f operator *(const Vector3f& v) const { return { x * v.x, y * v.y, z * v.z }; }
    Vector3f operator *(float k) const { return { x * k, y * k, z * k }; }

    Vector3f operator /(float k) const { const auto inv_k = 1 / k; return { x * inv_k, y * inv_k, z * inv_k }; }

    Vector3f& operator +=(const Vector3f& v) { x += v.x, y += v.y, z += v.z; return *this; }

    float operator [](int index) const { return (&x)[index]; }
    float& operator [](int index) { return (&x)[index]; }

    Vector3f normalized() const;

    float magnitude_squared() const { return x * x + y * y + z * z; }

    float magnitude() const { return sqrtf(x * x + y * y + z * z); }

    float dot(const Vector3f rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

    Vector3f cross(const Vector3f rhs) const { return { y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x }; }
};

inline Vector3f operator *(float k, const Vector3f& v) { return { v.x * k, v.y * k, v.z * k }; }

inline std::ostream& operator <<(std::ostream& os, const Vector3f& v) { return os << v.x << ", " << v.y << ", " << v.z; }

inline Vector3f lerp(const Vector3f& a, const Vector3f& b, float t) {  return a * (1.0f - t) + b * t; }

struct Vector2f {
    float x, y;

    Vector2f() : x(0), y(0) {}
    Vector2f(float k) : x(k), y(k) {}
    Vector2f(float xx, float yy) : x(xx), y(yy) {}
    Vector2f operator *(float k) const { return Vector2f(x * k, y * k); }
    Vector2f operator +(const Vector2f& v) const { return Vector2f(x + v.x, y + v.y); }

};

inline Vector2f operator *(float k, const Vector2f& v) { return { v.x * k, v.y * k }; }

inline std::ostream& operator <<(std::ostream& os, const Vector2f& v) { return os << v.x << ", " << v.y; }

// transform direction from local to world space
Vector3f local_to_world(const Vector3f& local_dir, const Vector3f& normal);

// transform polar coordinates to cartesian coordinates
inline Vector3f polar_to_cartesian(float theta, float phi) {
	return {
        std::sin(theta) * std::cos(phi),
        std::sin(theta) * std::sin(phi),
         std::cos(theta)
	};
}

void update_progress(float progress);