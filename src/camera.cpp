#include "camera.hpp"

#include <math.h> // Used for tan()

Mat4 Camera::GetViewMatrix() const
{
    Vec3 F = target - position;
    F = F.normal();

    Vec3 R = F.cross(vertical);
    R = R.normal();

    Vec3 U = R.cross(F);

    Mat4 view;
    // rotation
    view.m[0][0] = R.x;
    view.m[0][1] = R.y;
    view.m[0][2] = R.z;
    view.m[1][0] = U.x;
    view.m[1][1] = U.y;
    view.m[1][2] = U.z;
    view.m[2][0] = -F.x;
    view.m[2][1] = -F.y;
    view.m[2][2] = -F.z;

    // translation
    view.m[0][3] = -R.dot(position);
    view.m[1][3] = -U.dot(position);
    view.m[2][3] = F.dot(position);
    view.m[3][3] = 1.0;

    return view;
}

Mat4 Camera::GetProjectionMatrix() const
{
    Mat4 proj;
    float f = 1.0f / tan(fov / 2.0f);

    proj.m[0][0] = f / aspect;
    proj.m[1][1] = f;
    proj.m[2][2] = (zFar + zNear) / (zNear - zFar);
    proj.m[2][3] = (2.0f * zFar * zNear) / (zNear - zFar);
    proj.m[3][2] = -1.0f;

    return proj;
}

void Camera::zoom(const double &zoom)
{
    if (zoom > 0.01)
        position = Parameters::CAM_POSITION * zoom;
}

void Camera::drag(const Point2D &mouse_velocity)
{
    const int dx = mouse_velocity.x;
    const int dy = mouse_velocity.y;
    if (dx == 0 && dy == 0)
        return;

    Vec3 base = Parameters::CAM_POSITION;
    double radius = base.magnitude();
    if (radius < 1e-9)
        return;

    Vec3 F = (target - base).normal();   // toward origin
    Vec3 R = F.cross(vertical).normal(); // screen-right in world
    Vec3 U = R.cross(F);                 // screen-up in world (already unit)

    const double s = Parameters::DRAG_SENSITIVITY;

    // Package the mouse velocities into a vector (no new operators needed).
    Vec3 mouse = {(double)dx, (double)dy, 0.0};

    // Azimuth (horizontal) — always allowed. "Drag the world" feel => -mouse.x.
    Vec3 cand = (base + (R * (-mouse.x * s))).normal();

    // Elevation (vertical) — only apply if it stays clear of the poles.
    Vec3 candV = ((cand * radius) + (U * (mouse.y * s))).normal();
    const double LIMIT = 0.985; // ~10 deg off each pole
    double cosE = candV.dot(vertical);
    if (cosE <= LIMIT && cosE >= -LIMIT)
        cand = candV;

    Parameters::CAM_POSITION = cand * radius; // re-project to fixed-radius sphere
}