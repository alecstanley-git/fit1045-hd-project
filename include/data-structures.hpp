#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

struct Vec3
{
    // Three coordinate axes initialised to zero - no constructor needed; public variables
    double x{0.0}, y{0.0}, z{0.0};

    // Vec3 + Vec3 - should add element-by-element
    Vec3 operator+(const Vec3& other) const;

    // Vec3 += Vec 3 - should just call the adder
    Vec3& operator+=(const Vec3& other);

    // Vec3 - Vec3 - element-by-element
    Vec3 operator-(const Vec3& other) const;

    // Scalar multiply
    Vec3 operator*(const double& coefficient) const;

    // Dot product of two vectors using * operator
    Vec3 operator*(const Vec3& other) const;

    // Cross product of two vectors
    Vec3 cross(const Vec3& other) const;

    // Vector magnitude
    double magnitude() const;

    // Normal/unit vector (vector / magnitude)
    Vec3 normal() const;
};

struct Vec4
{
    double x{0.0}, y{0.0}, z{0.0}, w{0.0}; // w is a homogeneous coordinate - mostly used for the camera
};

struct Mat4
{
    // 4x4 matrix
    double m[4][4] = {0.0};

    // Matrix multiplication
    Mat4 operator*(const Mat4& other) const;

    // Allow matrix * vector for transformations
    Vec4 operator*(const Vec4& other) const;
};

#endif