#include "figure.hpp"

#include <iostream>

void Figure::plot(const dynamic_array<double> &x, const dynamic_array<double> &y, const Color &_color, const int _markersize)
{
    if (x.length() != y.length())
    {
        std::cerr << "Invalid array sizes for plot" << std::endl;
        return;
    }

    // Defining the screen space from the data space
    dynamic_array<int> x_data_screen;
    dynamic_array<int> y_data_screen;

    double x_range;
    double y_range;

    x_range = (xlim.x == xlim.y) ? 1.0 : (xlim.y - xlim.x);

    y_range = (ylim.x == ylim.y) ? 1.0 : (ylim.y - ylim.x);

    double x_padding = dimensions.x * padding_multiplier;
    double y_padding = dimensions.y * padding_multiplier;

    double drawable_width = dimensions.x - (2 * x_padding);
    double drawable_height = dimensions.y - (2 * y_padding);

    // Combined loop
    for (int i = 0; i < x.length(); i++)
    {
        Point pt;
        pt.color = _color;
        pt.markersize = _markersize;

        // Calculate and store screen coordinates (with rounding)
        pt.pos.x = static_cast<int>(std::round(pos.x + x_padding + (x[i] - xlim.x) / x_range * drawable_width));
        pt.pos.y = static_cast<int>(std::round(pos.y + dimensions.y - y_padding - (y[i] - ylim.x) / y_range * drawable_height));
        pt.pos.z = 0.0;

        buffer.add(pt);
    }

}

void Figure::plot3d(const dynamic_array<double> &x, const dynamic_array<double> &y, const dynamic_array<double> &z)
{

}

void Figure::show()
{
    for (int i = 0; i < buffer.length(); i++)
    {
        window.fill_circle(buffer[i].pos.x, buffer[i].pos.y, buffer[i].markersize, buffer[i].color);
    }
}