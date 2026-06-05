#ifndef FIGURE_H
#define FIGURE_H

#include <iostream>
#include "window.hpp"
#include "data-structures.hpp"
#include "dynamic-array.hpp"
#include "camera.hpp"

// Tells the figure what coordinate system it is working with (2D or 3D)
// Crucial for rendering axis lines and labels correctly
enum CoordSys
{
    DIM2,
    DIM3
};

class Figure
{
    Window &window;
    Camera *camera = nullptr;
    Point2D pos;
    Point2D dimensions;
    CoordSys sys = DIM2;
    std::string title = "";
    std::string xlabel = "";
    std::string ylabel = "";
    std::string zlabel = "";

    struct Range
    {
        double min = 0.0;
        double max = 0.0;
    };

    Range xlim;
    Range ylim;
    Range zlim;

    double padding_multiplier = 0.1; // 0.1 corresponds to 10% margins

    struct Point
    {
        Vec3 pos;
        Color color;
        int markersize; // px radius
    };

    // Buffers
    dynamic_array<Point> buffer;

    // Private show() extensions that properly render axes and labels
    void show2d();
    void show3d();

    // World Vec3 to screen pixels. Returns false if the point is behind the camera.
    bool project(const Mat4 &viewProj, const Vec3 &world, Point2D &screen);

    // Liang-Barsky: trims segment (x1,y1)-(x2,y2) to the figure rectangle in place.
    // Returns false if the segment lies entirely outside the box.
    bool clip_to_box(int &x1, int &y1, int &x2, int &y2);

    // Clips a line to the figure rectangle, then draws the visible portion (if any).
    void draw_clipped_line(int x1, int y1, int x2, int y2, Color color);

public:
    // Main constructur calls the parameters absolutely necessary for the figure
    Figure(Window &_window, const Point2D &_pos, const Point2D &_dimensions) : window(_window), pos(_pos), dimensions(_dimensions) {}

    // Configurable settings
    // Methods to allow user to edit figure properties on the fly
    void set_xlim(const double min, const double max) { xlim = {min, max}; }
    void set_ylim(const double min, const double max) { ylim = {min, max}; }
    void set_zlim(const double min, const double max) { zlim = {min, max}; }
    void set_padding(const double _padding) { padding_multiplier = _padding; }
    void set_title(const std::string &_title) { title = _title; }
    void set_xlabel(const std::string &_label) { xlabel = _label; }
    void set_ylabel(const std::string &_label) { ylabel = _label; }
    void set_zlabel(const std::string &_label) { zlabel = _label; }

    // Displaying methods
    // Must be called after the settings methods
    void plot(const dynamic_array<double> &x, const dynamic_array<double> &y, const Color &_color, const int _markersize);
    void plot3d(Camera &camera, const dynamic_array<double> &x, const dynamic_array<double> &y, const dynamic_array<double> &z, const Color &_color, const int _markersize);

    // Actually draws objects to screen. Must be the very last method called in the sequence
    void show();
};

#endif