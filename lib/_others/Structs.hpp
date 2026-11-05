#pragma once

template <typename T>
struct Vector2
{
    T x;
    T y;

    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}

    Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); }
    Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); }
    Vector2<T> operator*(const T& scalar) const { return Vector2<T>(x * scalar, y * scalar); }
    Vector2<T> operator/(const T& scalar) const { return Vector2<T>(x / scalar, y / scalar); }
};

template <typename T>
struct Vector3
{
    T x;
    T y;
    T z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

    Vector3<T> operator+(const Vector3<T>& other) const { return Vector3<T>(x + other.x, y + other.y, z + other.z); }
    Vector3<T> operator-(const Vector3<T>& other) const { return Vector3<T>(x - other.x, y - other.y, z - other.z); }
    Vector3<T> operator*(const T& scalar) const { return Vector3<T>(x * scalar, y * scalar, z * scalar); }
    Vector3<T> operator/(const T& scalar) const { return Vector3<T>(x / scalar, y / scalar, z / scalar); }
};

// 姿勢を表すクォータニオン。初期値は回転なし。
template <typename T>
struct Quaternion
{
    T w;
    T x;
    T y;
    T z;

    Quaternion() : w(1), x(0), y(0), z(0)
    {
    }

    Quaternion(T w, T x, T y, T z) : w(w), x(x), y(y), z(z)
    {
    }
};