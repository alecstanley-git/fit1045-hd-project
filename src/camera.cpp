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
    view.m[0][0] = R.x;  view.m[0][1] = R.y;  view.m[0][2] = R.z;
    view.m[1][0] = U.x;  view.m[1][1] = U.y;  view.m[1][2] = U.z;
    view.m[2][0] = -F.x; view.m[2][1] = -F.y; view.m[2][2] = -F.z;
    
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
