#ifndef FIGURE_H
#define FIGURE_H

#include "window.hpp"
#include "data-structures.hpp"
#include "dynamic-array.hpp"

struct Figure
{
    Window window;
    Point2D pos;
    Point2D dimensions;

    Figure(Window &_window, Point2D &_pos, Point2D &_dimensions) : window(_window), pos(_pos), dimensions(_dimensions) {}

    void plot(const dynamic_array<double> &x, const dynamic_array<double> &y);
};

#endif