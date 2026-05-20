#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include "button.hpp"
#include "dynamic-array.hpp"
#include "point2d.hpp"

/*
This enum stores colour information in hexadecimal format.
Both Windows and Mac store colour differently. This format is easy to translate in the OS-specific methods.
*/
enum Color : uint64_t
{
    Red = 0xFF0000FF,
    Green = 0x00FF00FF,
    Blue = 0x0000FFFF,
    Black = 0x000000FF,
    Grey = 0x707070FF,
    LightGrey = 0xC2C2C2FF,
    White = 0xFFFFFFFF
};

/*
Main window class.
Stores state information about the window object.
*/
class Window
{
    int width;
    int height;
    std::string title;
    bool is_open = false;
    Point2D mouse_position;
    void *_window; // This points to the os-specific window object - must be a pointer*.
    bool is_mouse_down = false;
    bool was_mouse_down = false;

    void setup_mouse_listeners();

public:
    // The buttons should be a publicly accessible field - they are just pointers to the structs
    dynamic_array<Button *> buttons;

    /*
    Default constructor
    TODO - @params
    */
    Window(int _width, int _height, std::string _title);

    ~Window();

    // Basic window management methods
    void process_events();
    void clear_screen(uint64_t color);
    bool is_running();

    // Rendering methods
    void fill_rectangle(int x, int y, int width, int height, Color color, bool is_button = false);
    void fill_circle(int x, int y, int r, Color color);
    void draw_text(const std::string &text, int x, int y, double size, Color color, int box_width, int box_height);
    bool load_font(const std::string &file_path);

    // High-level methods
    Button *add_button(int x, int y, int width, int height, std::string text);
    void process_buttons(dynamic_array<int> &indices);
    void plot(const dynamic_array<double> &x_data, const dynamic_array<double> &y_data, int x, int y, double scale, const std::string x_label, const std::string y_label, const std::string title, double x_min, double x_max, double y_min, double y_max);
};

inline Window::~Window()
{
    for (int i = 0; i < buttons.length(); i++)
    {
        delete buttons[i];
        buttons.remove(i);
    }
}

inline bool Window::is_running()
{
    return is_open;
}

inline Button *Window::add_button(int x, int y, int w, int h, std::string text)
{
    Button *ptr = new Button(x, y, w, h, text);
    buttons.add(ptr);
    return ptr;
}

inline void Window::process_buttons(dynamic_array<int> &indices)
{
    Color box_color;
    Color text_color;
    int idx;
    for (int i = 0; i < indices.length(); i++)
    {
        idx = indices[i];
        if (buttons[idx]->is_hovering(mouse_position))
        {
            if (is_mouse_down)
            {
                if (was_mouse_down)
                {
                    buttons[idx]->state = HELD;
                }
                else if (!was_mouse_down)
                {
                    buttons[idx]->state = CLICKED;
                    was_mouse_down = true;
                }
                box_color = Blue;
                text_color = White;
            }
            else
            {
                box_color = LightGrey;
                text_color = Black;
                buttons[idx]->state = HOVERING;
                was_mouse_down = false;
            }
        }
        else
        {
            box_color = Grey;
            text_color = White;
            buttons[idx]->state = IDLE;
        }
        fill_rectangle(buttons[idx]->x, buttons[idx]->y, buttons[idx]->width, buttons[idx]->height, box_color, true);
        draw_text(buttons[idx]->text, buttons[idx]->x, buttons[idx]->y + buttons[idx]->height / 4, 18, text_color, buttons[idx]->width, buttons[idx]->height);
    }
}

inline void Window::plot(
    const dynamic_array<double> &x_data,
    const dynamic_array<double> &y_data,
    int x, int y,
    double scale,
    const std::string x_label,
    const std::string y_label,
    const std::string title,
    double x_min, double x_max,
    double y_min, double y_max)
{
    if (x_data.length() != y_data.length())
        return;

    // PARAMETERS
    int thickness = 1;
    int point_size = 2;
    Color point_color = Blue;
    double padding_multiplier = 0.1; // 0.1 corresponds to 10% margins

    int scaled_width = (int)(scale * 400);
    int scaled_height = (int)(scale * 400);
    fill_rectangle(x, y + scaled_height - thickness, scaled_width, thickness, Black);
    fill_rectangle(x, y, thickness, scaled_height, Black);

    draw_text(title, x, y - 20, 18, Black, scaled_width, scaled_height);
    draw_text(x_label, x, y + scaled_height + 10, 18, Black, scaled_width, scaled_height);
    draw_text(y_label, x - 20, y, 18, Black, 20, scaled_height);

    // Defining the screen space from the data space
    dynamic_array<int> x_data_screen;
    dynamic_array<int> y_data_screen;

    double x_range;
    double y_range;

    x_range = (x_min == x_max) ? 1.0 : (x_max - x_min);

    y_range = (y_min == y_max) ? 1.0 : (y_max - y_min);

    double x_padding = scaled_width * padding_multiplier;
    double y_padding = scaled_height * padding_multiplier;

    double drawable_width = scaled_width - (2 * x_padding);
    double drawable_height = scaled_height - (2 * y_padding);

    // Combined loop
    for (int i = 0; i < x_data.length(); i++)
    {
        // Calculate and store screen coordinates (with rounding)
        int x_point = static_cast<int>(std::round(x_padding + (x_data[i] - x_min) / x_range * drawable_width));
        int y_point = static_cast<int>(std::round(scaled_height - y_padding - (y_data[i] - y_min) / y_range * drawable_height));

        x_data_screen.add(x_point);
        y_data_screen.add(y_point);

        fill_circle(x + x_point, y + y_point, point_size, point_color);
    }
}

#endif