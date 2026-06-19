#ifndef FIGURE_H
#define FIGURE_H

#include "camera.hpp"
#include "data-structures.hpp"
#include "dynamic-array.hpp"
#include "window.hpp"

// Tells the figure what coordinate system it is working with (2D or 3D)
// Crucial for rendering axis lines and labels correctly
enum CoordSys { DIM2, DIM3 };

class Figure {
  Window &window; // Reference to the parent window object
  Camera *camera =
      nullptr;         // Pointer to the camera object initialised on startup
  Point2D pos;         // x, y position in pixels of the top-left of the figure
  Point2D dimensions;  // Pixel dimensions (width, height) of the figure
  CoordSys sys = DIM2; // The coordinate system

  // Optional figure parameters
  std::string title = "";
  std::string xlabel = "";
  std::string ylabel = "";
  std::string zlabel = "";

  // The range is pretty much the same as Point2D but it stores named doubles
  struct Range {
    double min = 0.0; // Lower bound of the axis
    double max = 0.0; // Upper bound of the axis
  };

  // Fig params: the data-space range drawn on each axis
  Range xlim; // Range of the x-axis
  Range ylim; // Range of the y-axis
  Range zlim; // Range of the z-axis (3D only)

  // Used in 2D mode, this determines how far in from the plot walls we want to
  // draw our data
  double padding_multiplier = 0.1; // 0.1 corresponds to 10% margins

  // The point struct allows us to associate a color and radius to each point in
  // space
  struct Point {
    Vec3 pos;       // World-space position of the point
    Color color;    // Fill color used when drawing the marker
    int markersize; // px radius
  };

  // Buffer of points that will get drawn each frame
  // Allows us to call plot() repeatedly and then show() only once.
  dynamic_array<Point> buffer;

  // Describes the axis tick layout chosen by compute_ticks()
  struct TickSpec {
    double low, high, step; // First tick, last tick, and spacing between ticks
  };

  // Private show() extensions that properly render axes and labels
  void show2d(); // Draws the 2D plot, axes and labels
  void show3d(); // Draws the 3D plot, axes and labels

  // Maps a clip-space point (post viewProj, pre perspective-divide) to screen
  // pixels. Assumes clip.w > 0.
  // @param const Vec4 &clip - the clip-space point to map
  // @return Point2D - the equivalent point in screen pixels
  Point2D clip_to_screen(const Vec4 &clip);

  // World Vec3 to screen pixels. Returns false if the point is behind the near
  // plane.
  // @param const Mat4 &viewProj - combined view-projection matrix
  // @param const Vec3 &world - the world-space point to project
  // @param Point2D &screen - output parameter that receives the screen position
  // @return bool - false if the point is behind the near plane, true otherwise
  bool project(const Mat4 &viewProj, const Vec3 &world, Point2D &screen);

  // projects a line segment, clipping it against the near plane in clip space
  // first. returns false if the whole segment is behind the camera.
  // @param const Mat4 &viewProj - combined view-projection matrix
  // @param const Vec3 &w1 - first endpoint in world space
  // @param const Vec3 &w2 - second endpoint in world space
  // @param Point2D &s1 - output parameter that receives the first screen
  // endpoint
  // @param Point2D &s2 - output parameter that receives the second screen
  // endpoint
  // @return bool - false if the whole segment is behind the camera, true
  // otherwise
  bool project_segment(const Mat4 &viewProj, const Vec3 &w1, const Vec3 &w2,
                       Point2D &s1, Point2D &s2);

  // Liang-Barsky algorithm: trims segment (x1,y1)-(x2,y2) to the figure
  // rectangle in place. Returns false if the segment lies entirely outside the
  // box.
  // @param int &x1, &y1 - first endpoint, trimmed in place to the figure
  // rectangle
  // @param int &x2, &y2 - second endpoint, trimmed in place to the figure
  // rectangle
  // @return bool - false if the segment lies entirely outside the box, true
  // otherwise
  bool clip_to_box(int &x1, int &y1, int &x2, int &y2);

  // Clips a line to the figure rectangle, then draws the visible portion (if
  // any).
  // @param int x1, y1 - first endpoint of the line in screen pixels
  // @param int x2, y2 - second endpoint of the line in screen pixels
  // @param Color color - the color to draw the line with
  void draw_clipped_line(int x1, int y1, int x2, int y2, Color color);

  // Maps a data-space (dx,dy) point to screen pixels (same mapping as plot()).
  // @param double dx - the x coordinate in data space
  // @param double dy - the y coordinate in data space
  // @return Point2D - the equivalent point in screen pixels
  Point2D data_to_screen(double dx, double dy);

  // Heckbert's nice number algorithm, similar to MatPlotLib (Python)
  // @param const double &x - the value to round to a "nice" number
  // @param const bool &round - if true round to nearest, otherwise round up
  // @return double - the nearest aesthetically "nice" number
  static double nice_num(const double &x, const bool &round);

  // Returns nice numbers and step for a target tick count
  // @param const double &min - lower bound of the axis range
  // @param const double &max - upper bound of the axis range
  // @param const int target - desired number of ticks (defaults to TICK_COUNT)
  // @return TickSpec - the chosen first tick, last tick and step
  static TickSpec compute_ticks(const double &min, const double &max,
                                const int target = TICK_COUNT);

public:
  // Main constructur calls the parameters absolutely necessary for the figure
  // @param Window &_window - the parent window the figure draws into
  // @param const Point2D &_pos - pixel position of the figure's top-left corner
  // @param const Point2D &_dimensions - pixel width and height of the figure
  Figure(Window &_window, const Point2D &_pos, const Point2D &_dimensions)
      : window(_window), pos(_pos), dimensions(_dimensions) {}

  // Configurable settings
  // Methods to allow user to edit figure properties on the fly
  void set_xlim(const double min, const double max) {
    xlim = {min, max};
  } // Set the x-axis range
  void set_ylim(const double min, const double max) {
    ylim = {min, max};
  } // Set the y-axis range
  void set_zlim(const double min, const double max) {
    zlim = {min, max};
  } // Set the z-axis range (3D only)
  void set_padding(const double _padding) {
    padding_multiplier = _padding;
  } // Set the margin between data and plot walls
  void set_title(const std::string &_title) {
    title = _title;
  } // Set the figure title
  void set_xlabel(const std::string &_label) {
    xlabel = _label;
  } // Set the x-axis label
  void set_ylabel(const std::string &_label) {
    ylabel = _label;
  } // Set the y-axis label
  void set_zlabel(const std::string &_label) {
    zlabel = _label;
  } // Set the z-axis label (3D only)

  // Displaying methods
  // Must be called after the settings methods
  // Queues a 2D series of points into the draw buffer
  // @param const dynamic_array<double> &x - x coordinates of the points
  // @param const dynamic_array<double> &y - y coordinates of the points
  // @param const Color &_color - the color to draw the points with
  // @param const int _markersize - px radius of each point marker
  void plot(const dynamic_array<double> &x, const dynamic_array<double> &y,
            const Color &_color, const int _markersize);

  // Queues a 3D series of points into the draw buffer
  // @param Camera &camera - the camera used to view the 3D scene
  // @param const dynamic_array<double> &x - x coordinates of the points
  // @param const dynamic_array<double> &y - y coordinates of the points
  // @param const dynamic_array<double> &z - z coordinates of the points
  // @param const Color &_color - the color to draw the points with
  // @param const int _markersize - px radius of each point marker
  void plot3d(Camera &camera, const dynamic_array<double> &x,
              const dynamic_array<double> &y, const dynamic_array<double> &z,
              const Color &_color, const int _markersize);

  // Actually draws objects to screen. Must be the very last method called in
  // the sequence
  void show();
};

#endif
