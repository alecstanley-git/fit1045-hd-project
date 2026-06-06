#include "figure.hpp"

#include <iostream>
#include <format>

void Figure::plot(const dynamic_array<double> &x, const dynamic_array<double> &y, const Color &_color, const int _markersize)
{
    if (x.length() != y.length())
    {
        std::cerr << "Invalid array sizes for plot" << std::endl;
        return;
    }

    for (int i = 0; i < x.length(); i++)
    {
        Point pt;
        pt.color = _color;
        pt.markersize = _markersize;

        // Calculate and store screen coordinates (with rounding)
        Point2D screen = data_to_screen(x[i], y[i]);
        pt.pos.x = screen.x;
        pt.pos.y = screen.y;
        pt.pos.z = 0.0;

        buffer.add(pt);
    }
}

// Maps a data-space (dx,dy) point to screen pixels (same mapping as plot()).
Point2D Figure::data_to_screen(double dx, double dy)
{
    double x_range = (xlim.min == xlim.max) ? 1.0 : (xlim.max - xlim.min);
    double y_range = (ylim.min == ylim.max) ? 1.0 : (ylim.max - ylim.min);

    double x_padding = dimensions.x * padding_multiplier;
    double y_padding = dimensions.y * padding_multiplier;

    double drawable_width = dimensions.x - (2 * x_padding);
    double drawable_height = dimensions.y - (2 * y_padding);

    Point2D p;
    p.x = static_cast<int>(std::round(pos.x + x_padding + (dx - xlim.min) / x_range * drawable_width));
    p.y = static_cast<int>(std::round(pos.y + dimensions.y - y_padding - (dy - ylim.min) / y_range * drawable_height));
    return p;
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

Point2D Figure::clip_to_screen(const Vec4 &clip)
{
    // Normalised device coordinates from -1 to 1
    double ndc_x = clip.x / clip.w; // map to NDC
    double ndc_y = clip.y / clip.w;

    double x_padding = dimensions.x * padding_multiplier;
    double y_padding = dimensions.y * padding_multiplier;
    double drawable_width = dimensions.x - (2 * x_padding);
    double drawable_height = dimensions.y - (2 * y_padding);

    // Screen mapping
    // ndc * 0.5 + 0.5 maps [-1,1] to [0,1]
    // y needs to be flipped because in NDC, +1 is up, but on screen, +1 is down.
    Point2D screen;
    screen.x = static_cast<int>(std::round(pos.x + x_padding + (ndc_x * 0.5 + 0.5) * drawable_width));
    screen.y = static_cast<int>(std::round(pos.y + y_padding + (1.0 - (ndc_y * 0.5 + 0.5)) * drawable_height));
    return screen;
}

bool Figure::project(const Mat4 &viewProj, const Vec3 &world, Point2D &screen)
{
    Vec4 p = viewProj * Vec4{world.x, world.y, world.z, 1.0}; // perform transformation

    // Reject points behind the near plane (d = z + w <= 0), not just behind the camera
    // (w <= 0). A point with a tiny positive w sits between the camera and the near plane;
    // its ndc = x/w blows up and overflows the int cast in clip_to_screen, crashing when
    // zoomed in very close. d > 0 guarantees w >= zNear, keeping screen coordinates finite.
    if (p.z + p.w <= 0.0)
        return false;

    screen = clip_to_screen(p);
    return true;
}

/*
Projects a line segment to screen, clipping it against the near plane in clip space.
A single point with w <= 0 is behind the camera, but for a segment we only want to
trim the behind-camera portion away.

We must clip against the actual near plane, NOT the w = 0 plane. A point clipped to
w ~ 0 has ndc = x/w -> +/-infinity, which produces astronomically large screen
coordinates that overflow the int cast and make the axis crash/jump/vanish.
On the near plane the standard projection maps z_ndc = -1, i.e. z_clip = -w_clip, so
the signed "in front of near plane" distance is d = z_clip + w_clip (d >= 0 is visible).
At the crossing d = 0 the surviving endpoint has w = zNear, keeping ndc finite.
We interpolate the crossing in clip space *before* the perspective divide.
*/
bool Figure::project_segment(const Mat4 &viewProj, const Vec3 &w1, const Vec3 &w2, Point2D &s1, Point2D &s2)
{
    Vec4 a = viewProj * Vec4{w1.x, w1.y, w1.z, 1.0};
    Vec4 b = viewProj * Vec4{w2.x, w2.y, w2.z, 1.0};

    // Signed distance from the near plane; >= 0 is in front of it (visible).
    double da = a.z + a.w;
    double db = b.z + b.w;

    if (da < 0.0 && db < 0.0)
        return false; // whole segment behind the near plane

    // Linear interpolation of all 4 clip-space components at the near-plane crossing.
    auto lerp4 = [](const Vec4 &p, const Vec4 &q, double t)
    {
        return Vec4{p.x + (q.x - p.x) * t, p.y + (q.y - p.y) * t,
                    p.z + (q.z - p.z) * t, p.w + (q.w - p.w) * t};
    };

    // Only one endpoint can be behind here (both-behind already returned). When signs
    // differ, da - db is bounded away from zero, so t is well-defined and in [0, 1].
    if (da < 0.0)
    {
        double t = da / (da - db);
        a = lerp4(a, b, t);
    }
    else if (db < 0.0)
    {
        double t = da / (da - db);
        b = lerp4(a, b, t);
    }

    s1 = clip_to_screen(a);
    s2 = clip_to_screen(b);
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

double Figure::nice_num(const double &x, const bool &round)
{
    double exp = std::floor(std::log10(x));
    double f = x / std::pow(10, exp);
    double nf;
    if (round)
        nf = (f < 1.5) ? 1 : (f < 3) ? 2
                         : (f < 7)   ? 5
                                     : 10;
    else
        nf = (f <= 1) ? 1 : (f <= 2) ? 2
                        : (f <= 5)   ? 5
                                     : 10;
    return nf * std::pow(10.0, exp);
}

Figure::TickSpec Figure::compute_ticks(const double &min, const double &max, const int target)
{
    double range = nice_num(max - min, false);
    double step = nice_num(range / (target - 1), true);
    double low = std::floor(min / step) * step;
    double high = std::ceil(max / step) * step;
    return {low, high, step};
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

        window.draw_line(pos.x, pos.y, pos.x + dimensions.x, pos.y, TICK_COLOR);
        window.draw_line(pos.x + dimensions.x, pos.y, pos.x + dimensions.x, pos.y + dimensions.y, TICK_COLOR);
        window.draw_line(pos.x, pos.y, pos.x, pos.y + dimensions.y, TICK_COLOR);
        window.draw_line(pos.x, pos.y + dimensions.y, pos.x + dimensions.x, pos.y + dimensions.y, TICK_COLOR);

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
        window.draw_text(xlabel, pos.x, pos.y + dimensions.y + 32, BUTTON_TEXTSIZE, TITLE_COLOR, dimensions.x, 22);
    }

    if (ylabel != "")
    {
        window.draw_text(ylabel, pos.x - 55, pos.y, BUTTON_TEXTSIZE, TITLE_COLOR, 20, dimensions.y);
    }

    draw_clipped_line(pos.x, pos.y + dimensions.y, pos.x + dimensions.x, pos.y + dimensions.y, TITLE_COLOR);
    draw_clipped_line(pos.x, pos.y, pos.x, pos.y + dimensions.y, TITLE_COLOR);

    // X ticks along the bottom axis. data_to_screen() gives the data-aligned x (which keeps
    // the padding so the data stays inset), but the tick must hang off the bottom frame line
    // (pos.y + dimensions.y), not the padded data position, otherwise it floats inside the plot.
    int bottom_y = pos.y + dimensions.y;
    TickSpec tx = compute_ticks(xlim.min, xlim.max);
    for (double v = tx.low; v <= tx.high + tx.step * 0.5; v += tx.step)
    {
        if (v < xlim.min || v > xlim.max)
            continue;
        double label = (v == 0.0) ? 0.0 : v; // collapse -0 to 0
        Point2D p = data_to_screen(v, ylim.min);
        window.draw_line(p.x, bottom_y, p.x, bottom_y + TICK_LENGTH_PX, TICK_COLOR);
        window.draw_text(std::format("{:g}", label), p.x - 20, bottom_y + 8, TICK_TEXTSIZE, TITLE_COLOR, 40, 18);
    }

    // Y ticks along the left axis: data-aligned y, anchored to the left frame line (pos.x).
    int left_x = pos.x;
    TickSpec ty = compute_ticks(ylim.min, ylim.max);
    for (double v = ty.low; v <= ty.high + ty.step * 0.5; v += ty.step)
    {
        if (v < ylim.min || v > ylim.max)
            continue;
        double label = (v == 0.0) ? 0.0 : v;
        Point2D p = data_to_screen(xlim.min, v);
        window.draw_line(left_x - TICK_LENGTH_PX, p.y, left_x, p.y, TICK_COLOR);
        window.draw_text(std::format("{:g}", label), left_x - 44, p.y - 9, TICK_TEXTSIZE, TITLE_COLOR, 40, 18);
    }
}

void Figure::show3d()
{
    if (!camera)
        return;

    Mat4 viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();

    // Text can't be Liang-Barsky clipped like a line, so we clip it at the boundary by
    // skipping any label whose anchor projects outside the figure rectangle (same
    // point-in-box test show() uses for markers; clip_to_box leaves a point unchanged).
    auto inside_box = [&](const Point2D &p)
    {
        int cx = p.x, cy = p.y;
        return clip_to_box(cx, cy, cx, cy);
    };

    Point2D a, b;

    if (project_segment(viewProj, {xlim.min, 0.0, 0.0}, {xlim.max, 0.0, 0.0}, a, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // X axis

    if (project_segment(viewProj, {0.0, ylim.min, 0.0}, {0.0, ylim.max, 0.0}, a, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // Y axis

    if (project_segment(viewProj, {0.0, 0.0, zlim.min}, {0.0, 0.0, zlim.max}, a, b))
        draw_clipped_line(a.x, a.y, b.x, b.y, TITLE_COLOR); // Z axis

    // Ticks + numeric labels for one axis. axis: 0=X, 1=Y, 2=Z. Tick positions are
    // points, so project() (skip-if-behind-camera) is enough; the perpendicular nudge
    // is in world space so tick size stays consistent as the camera rotates.
    auto draw_ticks = [&](double lo_lim, double hi_lim, int axis)
    {
        double k = (hi_lim - lo_lim) * 0.005;
        TickSpec ts = compute_ticks(lo_lim, hi_lim, 10);
        for (double v = ts.low; v <= ts.high + ts.step * 0.5; v += ts.step)
        {
            if (v < lo_lim || v > hi_lim)
                continue;

            Vec3 base, nudge;
            if (axis == 0)
            {
                base = {v, -k, 0.0};
                nudge = {v, k, 0.0};
            }
            else if (axis == 1)
            {
                base = {-k, v, 0.0};
                nudge = {k, v, 0.0};
            }
            else
            {
                base = {-k, 0.0, v};
                nudge = {k, 0.0, v};
            }

            Point2D p, q;
            if (project(viewProj, base, p) && project(viewProj, nudge, q))
            {
                draw_clipped_line(p.x, p.y, q.x, q.y, TICK_COLOR);
                double label = (v == 0.0) ? 0.0 : v; // collapse -0 to 0
                if (inside_box(p))
                    window.draw_text(std::format("{:g}", label), p.x - 20, p.y + 6, TICK_TEXTSIZE, TITLE_COLOR, 40, 18);
            }
        }
    };

    draw_ticks(xlim.min, xlim.max, 0);
    draw_ticks(ylim.min, ylim.max, 1);
    draw_ticks(zlim.min, zlim.max, 2);

    // Axis-name labels: drawn just beyond each axis max.
    Point2D lp;
    if (xlabel != "" && project(viewProj, {xlim.max * 1.1, 0.0, 0.0}, lp) && inside_box(lp))
        window.draw_text(xlabel, lp.x - 20, lp.y, TICK_TEXTSIZE, TICK_COLOR, 40, 22);
    if (ylabel != "" && project(viewProj, {0.0, ylim.max * 1.1, 0.0}, lp) && inside_box(lp))
        window.draw_text(ylabel, lp.x - 20, lp.y, TICK_TEXTSIZE, TICK_COLOR, 40, 22);
    if (zlabel != "" && project(viewProj, {0.0, 0.0, zlim.max * 1.1}, lp) && inside_box(lp))
        window.draw_text(zlabel, lp.x - 20, lp.y, TICK_TEXTSIZE, TICK_COLOR, 40, 22);
}