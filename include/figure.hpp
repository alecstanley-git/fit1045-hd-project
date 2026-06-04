#ifndef FIGURE_H
#define FIGURE_H

#include <iostream>
#include "window.hpp"
#include "data-structures.hpp"
#include "dynamic-array.hpp"

class Figure
{
    Window &window;
    Point2D pos;
    Point2D dimensions;
    std::string title = "";
    std::string xlabel = "";
    std::string ylabel = "";
    std::string zlabel = "";
    Point2D xlim;
    Point2D ylim;
    Point2D zlim;
    double padding_multiplier = 0.1; // 0.1 corresponds to 10% margins

    struct Point
    {
        Vec3 pos;
        Color color;
        int markersize; // px radius
    };

    struct Line
    {
        Point2D start;
        Point2D end;
        Color color;
        int linewidth;
    };

    // Buffers
    dynamic_array<Point> buffer;

public:
    Figure(Window &_window, const Point2D &_pos, const Point2D &_dimensions) : window(_window), pos(_pos), dimensions(_dimensions) {}

    void plot(const dynamic_array<double> &x, const dynamic_array<double> &y, const Color &_color, const int _markersize);
    void plot3d(const dynamic_array<double> &x, const dynamic_array<double> &y, const dynamic_array<double> &z);
    void show();
};

/*
TEST SYNTAX:
Figure fig(window, {xpos, ypos}, {width, height});
fig.plot(x, y)

TODO:
fig.title(string);
fig.xlabel(string);
fig.ylabel(string);

fig.plot3d(x, y, z);
fig.title(string);
fig.xlabel(string);
fig.ylabel(string);
fig.zlabel(string);
*/

#endif