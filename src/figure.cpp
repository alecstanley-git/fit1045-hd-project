#include "figure.hpp"

#include <iostream>

void Figure::plot(const dynamic_array<double> &x, const dynamic_array<double> &y, const Color &_color, const int _markersize)
{
    if (x.length() != y.length())
    {
        std::cerr << "Invalid array sizes for plot" << std::endl;
        return;
    }

    double x_range = (xlim.min == xlim.max) ? 1.0 : (xlim.max - xlim.min);
    double y_range = (ylim.min == ylim.max) ? 1.0 : (ylim.max - ylim.min);

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
        pt.pos.x = static_cast<int>(std::round(pos.x + x_padding + (x[i] - xlim.min) / x_range * drawable_width));
        pt.pos.y = static_cast<int>(std::round(pos.y + dimensions.y - y_padding - (y[i] - ylim.min) / y_range * drawable_height));
        pt.pos.z = 0.0;

        buffer.add(pt);
    }
}

void Figure::plot3d(Camera &_camera, const dynamic_array<double> &x, const dynamic_array<double> &y, const dynamic_array<double> &z, const Color &_color, const int _markersize)
{
    if (x.length() != y.length() || x.length() != z.length())
    {
        std::cerr << "Invalid array sizes for plot3d" << std::endl;
        return;
    }

    sys = DIM3; // so show() routes to show3d()
    camera = &_camera;

    Mat4 viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();

    for (int i = 0; i < x.length(); i++)
    {
        Point2D screen;
        if (!project(viewProj, {x[i], y[i], z[i]}, screen))
            continue; // skip points behind the camera

        Point pt;
        pt.color = _color;
        pt.markersize = _markersize;
        pt.pos.x = screen.x;
        pt.pos.y = screen.y;
        pt.pos.z = 0.0;
        buffer.add(pt);
    }
}

bool Figure::project(const Mat4 &viewProj, const Vec3 &world, Point2D &screen)
{
    Vec4 p = {world.x, world.y, world.z, 1.0};
    p = viewProj * p; // perform transformation

    if (p.w <= 0.0)
        return false; // behind the camera

    // Normalised device coordinates from -1 to 1
    double ndc_x = p.x / p.w; // map to NDC
    double ndc_y = p.y / p.w;

    double x_padding = dimensions.x * padding_multiplier;
    double y_padding = dimensions.y * padding_multiplier;
    double drawable_width = dimensions.x - (2 * x_padding);
    double drawable_height = dimensions.y - (2 * y_padding);

    // Screen mapping
    // ndc * 0.5 + 0.5 maps [-1,1] to [0,1]
    // y needs to be flipped because in NDC, +1 is up, but on screen, +1 is down.
    screen.x = static_cast<int>(std::round(pos.x + x_padding + (ndc_x * 0.5 + 0.5) * drawable_width));
    screen.y = static_cast<int>(std::round(pos.y + y_padding + (1.0 - (ndc_y * 0.5 + 0.5)) * drawable_height));
    return true;
}

/*
Liang-Barsky line clipping against the figure rectangle.
The segment is parameterised as P(t) = P1 + t*(P2 - P1), t in [0,1]. We progressively
shrink the visible interval [t0, t1] against each of the four box edges, then rebuild
the trimmed endpoints. Returns false if nothing of the segment is inside the box.

I got this from the wikipedia page, it has very good documentation on this
https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
*/
bool Figure::clip_to_box(int &x1, int &y1, int &x2, int &y2)
{
    double xmin = pos.x;
    double xmax = pos.x + dimensions.x;
    double ymin = pos.y;
    double ymax = pos.y + dimensions.y;

    double dx = x2 - x1;
    double dy = y2 - y1;

    // p[i]/q[i] encode each edge: left, right, top, bottom
    double p[4] = {-dx, dx, -dy, dy};
    double q[4] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};

    double t0 = 0.0;
    double t1 = 1.0;

    for (int i = 0; i < 4; i++)
    {
        if (p[i] == 0.0)
        {
            // Segment is parallel to this edge; reject if it starts outside it
            if (q[i] < 0.0)
                return false;
        }
        else
        {
            double t = q[i] / p[i];
            if (p[i] < 0.0)
            {
                // Line enters the box here -> tighten the start
                if (t > t1)
                    return false;
                if (t > t0)
                    t0 = t;
            }
            else
            {
                // Line leaves the box here -> tighten the end
                if (t < t0)
                    return false;
                if (t < t1)
                    t1 = t;
            }
        }
    }

    // Rebuild endpoints from the original P1 using the trimmed interval
    x2 = static_cast<int>(std::round(x1 + t1 * dx));
    y2 = static_cast<int>(std::round(y1 + t1 * dy));
    x1 = static_cast<int>(std::round(x1 + t0 * dx));
    y1 = static_cast<int>(std::round(y1 + t0 * dy));
    return true;
}

void Figure::draw_clipped_line(int x1, int y1, int x2, int y2, Color color)
{
    if (clip_to_box(x1, y1, x2, y2))
        window.draw_line(x1, y1, x2, y2, color);
}

void Figure::show()
{
    if (buffer.length() > 0)
    {
        for (int i = 0; i < buffer.length(); i++)
        {
            // A point is a zero-length segment: clip_to_box returns true only if it lies inside the box
            int cx = static_cast<int>(buffer[i].pos.x);
            int cy = static_cast<int>(buffer[i].pos.y);
            if (clip_to_box(cx, cy, cx, cy))
                window.fill_circle(cx, cy, buffer[i].markersize, buffer[i].color);
        }

        if (title != "")
        {
            window.draw_text(title, pos.x, pos.y - 28, BUTTON_TEXTSIZE, TITLE_COLOR, dimensions.x, 22);
        }

        window.draw_line(pos.x, pos.y, pos.x + dimensions.x, pos.y, Grey);
        window.draw_line(pos.x + dimensions.x, pos.y, pos.x + dimensions.x, pos.y + dimensions.y, Grey);
        window.draw_line(pos.x, pos.y, pos.x, pos.y + dimensions.y, Grey);
        window.draw_line(pos.x, pos.y + dimensions.y, pos.x + dimensions.x, pos.y + dimensions.y, Grey);

        if (sys == DIM2)
        {
            show2d();
        }
        else if (sys == DIM3)
        {
            show3d();
        }
    }
    else
    {
        return;
    }
}

void Figure::show2d()
{
    if (xlabel != "")
    {
        window.draw_text(xlabel, pos.x, pos.y + dimensions.y + 8, BUTTON_TEXTSIZE, TITLE_COLOR, dimensions.x, 22);
    }

    if (ylabel != "")
    {
        window.draw_text(ylabel, pos.x - 20, pos.y, BUTTON_TEXTSIZE, TITLE_COLOR, 20, dimensions.y);
    }

    draw_clipped_line(pos.x, pos.y + dimensions.y, pos.x + dimensions.x, pos.y + dimensions.y, TITLE_COLOR);
    draw_clipped_line(pos.x, pos.y, pos.x, pos.y + dimensions.y, TITLE_COLOR);
}

void Figure::show3d()
{
    if (!camera)
        return;

    Mat4 viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();

    Point2D a, b;

    if (project(viewProj, {xlim.min, 0.0, 0.0}, a) && project(viewProj, {xlim.max, 0.0, 0.0}, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // X axis

    if (project(viewProj, {0.0, ylim.min, 0.0}, a) && project(viewProj, {0.0, ylim.max, 0.0}, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // Y axis

    if (project(viewProj, {0.0, 0.0, zlim.min}, a) && project(viewProj, {0.0, 0.0, zlim.max}, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // Z axis
}