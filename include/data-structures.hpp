#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

struct Point2D
{
    int x;
    int y;  
};

struct Vec3
{
    // Three coordinate axes initialised to zero - no constructor needed; public variables
    double x{0.0}, y{0.0}, z{0.0};

    // Vec3 + Vec3 - should add element-by-element
    Vec3 operator+(const Vec3 &other) const;

    // Vec3 += Vec 3 - should just call the adder
    Vec3 &operator+=(const Vec3 &other);

    // Vec3 - Vec3 - element-by-element
    Vec3 operator-(const Vec3 &other) const;

    // Scalar multiply
    Vec3 operator*(const double &coefficient) const;

    // Element-by-element multiply two vectors
    Vec3 operator*(const Vec3 &other) const;

    // Dot product of two vectors;
    double dot(const Vec3 &other) const;

    // Cross product of two vectors
    Vec3 cross(const Vec3 &other) const;

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
    double m[4][4] = {};

    static Mat4 identity()
    {
        Mat4 mat;
        mat.m[0][0] = 1.0;
        mat.m[1][1] = 1.0;
        mat.m[2][2] = 1.0;
        mat.m[3][3] = 1.0;
        return mat;
    }
};

inline Vec4 operator*(const Mat4 &mat, const Vec4 &vec)
{
    Vec4 result;
    result.x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3] * vec.w;
    result.y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3] * vec.w;
    result.z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3] * vec.w;
    result.w = mat.m[3][0] * vec.x + mat.m[3][1] * vec.y + mat.m[3][2] * vec.z + mat.m[3][3] * vec.w;
    return result;
}

inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
{
    Mat4 result;
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            result.m[row][col] = a.m[row][0] * b.m[0][col] +
                                 a.m[row][1] * b.m[1][col] +
                                 a.m[row][2] * b.m[2][col] +
                                 a.m[row][3] * b.m[3][col];
        }
    }
    return result;
}

#endif