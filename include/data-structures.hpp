#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

/*
Used a lot in the program especially for things like mouse coordinates, screen coordinates
Sometimes used interchangeably with Vec3 and Vec4 if they keep z and/or w to be 0, for pixel points.
*/
struct Point2D
{
    int x;
    int y;
};

/*
Main data type holding three-dimensional state information about a particular object in the simulation
*/
struct Vec3
{
    // Three coordinate axes initialised to zero - no constructor needed; public variables
    double x{0.0}, y{0.0}, z{0.0};

    // Vec3 + Vec3 - should add element-by-element
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param Vec3 - the output vector after the operation
    Vec3 operator+(const Vec3 &other) const;

    // Vec3 += Vec 3 - should just call the adder
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param Vec3 - the output vector after the operation
    Vec3 &operator+=(const Vec3 &other);

    // Vec3 - Vec3 - element-by-element
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param Vec3 - the output vector after the operation
    Vec3 operator-(const Vec3 &other) const;

    // Scalar multiply
    // @param const double &coefficient - the scalar multiplier
    // @param Vec3 - the output vector after the operation
    Vec3 operator*(const double &coefficient) const;

    // Element-by-element multiply two vectors
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param Vec3 - the output vector after the operation
    Vec3 operator*(const Vec3 &other) const;

    // Dot product of two vectors;
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param double - the scalar dot product
    double dot(const Vec3 &other) const;

    // Cross product of two vectors
    // @param const Vec3 &other - the additional vector to operate on, passed by reference
    // @param Vec3 - the output vector cross product
    Vec3 cross(const Vec3 &other) const;

    // Vector magnitude
    // @return double - the scalar vector magnitude
    double magnitude() const;

    // Divides the vector position by its magnitude, a.k.a a unit vector
    // @return Vec3 - the output unit vector
    Vec3 normal() const;
};

// Used only for the intermediate step in transforming world coordinates to screen space
// When in camera-relative space, the fourth coordinate 'w' represents the z-distance from the camera.
// In this space, the camera is at the origin looking out into the negative z axis.
struct Vec4
{
    double x{0.0}, y{0.0}, z{0.0}, w{0.0};
};

// Simple matrix type used to encode matrix transformations between coordinate systems
struct Mat4
{
    // 4x4 matrix
    double m[4][4] = {};
};

// Defines a vector transformation by a matrix: matrix * vector
// @param const Mat4 &mat - the transformation matrix, passed by reference
// @param const Vec4 &vec - the target vector, by reference
// @return Vec4 - the output transformed vector
inline Vec4 operator*(const Mat4 &mat, const Vec4 &vec)
{
    Vec4 result;

    // This is just the physical representation of row by column matrix multiplication
    result.x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3] * vec.w;
    result.y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3] * vec.w;
    result.z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3] * vec.w;
    result.w = mat.m[3][0] * vec.x + mat.m[3][1] * vec.y + mat.m[3][2] * vec.z + mat.m[3][3] * vec.w;
    return result;
}

// Allows for proper matrix multiplication
// @param const Mat4 &a - first transformation matrix, passed by reference
// @param const Mat4 &b - second transformation matrix, passed by reference
// @return Mat4 - the output transformation matrix
inline Mat4 operator*(const Mat4 &a, const Mat4 &b)
{
    Mat4 result;

    // Use nested for loops to iterate across matrix elements and perform the multiplication
    // This is analogous to how we are taught to do it by hand
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