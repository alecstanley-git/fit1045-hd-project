#ifndef CAMERA_H
#define CAMERA_H

#include "data-structures.hpp"

class Camera
{
public:
    Vec3 position; // Where the camera is in space
    Vec3 target;   // Target position
    Vec3 vertical; // Defines which way is up

    // Camera properties
    float fov;
    float aspect;
    float zNear;
    float zFar;

    Camera(Vec3 pos, Vec3 t, Vec3 u, float f, float a, float near, float far) : position(pos), target(t), vertical(u), fov(f), aspect(a), zNear(near), zFar(far) {}

    // The view matrix transforms the entire 'world' of objects relative to the camera
    Mat4 GetViewMatrix() const;

    // This is the perspective effect
    Mat4 GetProjectionMatrix() const;
};

#endif