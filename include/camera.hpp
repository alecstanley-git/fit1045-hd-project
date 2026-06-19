#ifndef CAMERA_H
#define CAMERA_H

#include "data-structures.hpp"

class Camera {
public:
  Vec3 position; // Where the camera is in space
  Vec3 target;   // Target position
  Vec3 vertical; // Defines which way is up

  // Camera properties
  float fov;    // angular field of view
  float aspect; // aspect ratio
  float zNear;  // the z-value in camera space of the closest renderable objects
  float zFar; // the z-value in camera space of the furthest renderable objects

  // Default constructor
  // @param all same as above
  Camera(Vec3 pos, Vec3 t, Vec3 u, float f, float a, float near, float far)
      : position(pos), target(t), vertical(u), fov(f), aspect(a), zNear(near),
        zFar(far) {}

  // The view matrix transforms the entire 'world' of objects relative to the
  // camera
  // @return Mat4 - a 4x4 matrix representing a transformation from real-world
  // points to camera-relative points
  Mat4 GetViewMatrix() const;

  // This is the perspective effect
  // @return Mat4 - a 4x4 matrix representing a transformation from
  // camera-relative points to camera-perspective points Scales points based on
  // distance
  Mat4 GetProjectionMatrix() const;

  // Allow user to zoom (handled externally with scroll wheel)
  // @param const double &zoom - the factor to zoom by
  void zoom(const double &zoom);

  // Allow user to drag by performing an operation on the camera's position
  // based on the mouse velocity
  // @param const Point2D &mouse_velocty - the x and y velocities of the mouse
  // in a given frame
  void drag(const Point2D &mouse_velocity);
};

#endif
