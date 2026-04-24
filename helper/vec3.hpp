#ifndef VEC3_H
#define VEC3_H

#include <cmath> // for sqrt

/*
-- Motivation for creating my own 3D vector class --

- C++'s own std:vector type has too much overhead - they are dynamic and not ideal for my use case
- The std::array method is good but it's limited. To allow for math operations I'd still have to code them myself. Given this, I made the decision to make my own class so I know exactly how to implement these features, picking and choosing what to include and what not to include.

- When creating this I had to choose between hard-defining x, y, z or using a C++ array... either way should have the same compile/run time, but I went with the former for readability. It especially helps in my main loop where I can call galaxy.position.x rather than galaxy.position[0] which is ambiguous.
- A downside of this is it makes looping through each index of my vector more difficult, but the nature of my problem does not ever require this. My Cartesian directions are independent and uncoupled.

- I used a struct over class because it is public by default. Seems to be more conventional for what I'm trying to achieve. Nothing needs to be private here.
*/

struct Vec3
{
    // Three coordinate axes initialised to zero - no constructor needed; public variables
    double x{0.0};
    double y{0.0};
    double z{0.0};

    // OPERATORS

    // Vec3 + Vec3 - should add element-by-element
    Vec3 operator+(const Vec3& other) const
    {
        return {x + other.x, y + other.y, z + other.z};
    }

    // Vec3 - Vec3 - element-by-element
    Vec3 operator-(const Vec3& other) const
    {
        return {x - other.x, y - other.y, z - other.z};
    }

    // Scalar multiply
    Vec3 operator*(const double& coefficient) const
    {
        return {coefficient * x, coefficient * y, coefficient * z};
    }

    // Element-by-element multiply two vectors
    Vec3 operator*(const Vec3& other) const
    {
        return {x * other.x, y * other.y, z * other.z};
    }

    // Vector magnitude
    double magnitude() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Normal/unit vector (vector / magnitude)
    Vec3 normal() const
    {   
        if (magnitude() != 0.0)
        {
            return {x / magnitude(), y / magnitude(), z / magnitude()};
        }
        else
        {
            return {0.0, 0.0, 0.0}; // Prevent division by zero
        }
    }
};

#endif