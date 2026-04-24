#ifndef VEC3_H
#define VEC3_H

struct Vec3
{
    // Three coordinate axes initialised to zero - no constructor needed; public variables
    double x{0.0};
    double y{0.0};
    double z{0.0};

    // Vec3 + Vec3 - should add element-by-element
    Vec3 operator+(const Vec3& other) const;

    // Vec3 - Vec3 - element-by-element
    Vec3 operator-(const Vec3& other) const;

    // Scalar multiply
    Vec3 operator*(const double& coefficient) const;

    // Element-by-element multiply two vectors
    Vec3 operator*(const Vec3& other) const;

    // Vector magnitude
    double magnitude() const;

    // Normal/unit vector (vector / magnitude)
    Vec3 normal() const;
};

#endif