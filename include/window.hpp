#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <thread>
#include "button.hpp"
#include "dynamic-array.hpp"
#include "data-structures.hpp"
#include "camera.hpp"
#include "colors.hpp"
#include "parameters.hpp"

using namespace Parameters;

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
    */
    Window(int _width, int _height, std::string _title);

    ~Window();

    // Basic window management methods, os-specific
    void process_events();
    void clear_screen(std::uint64_t color);
    bool is_running();

    // Rendering methods, os-specific
    void fill_rectangle(int x, int y, int width, int height, Color color, bool is_button = false);
    void fill_circle(int x, int y, int radius, Color color);
    void draw_line(int x1, int y1, int x2, int y2, Color color, int linewidth = 1);
    void draw_text(const std::string &text, int x, int y, double size, Color color, int box_width, int box_height);
    bool load_font(const std::string &file_path);

    // High-level os-agnostic methods defined within header itself
    Button *add_button(int x, int y, int width, int height, std::string text);
    void process_buttons(dynamic_array<int> &indices);
    void update_window(double fps);
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
                box_color = BUTTON_BG_HELD;
                text_color = BUTTON_TEXT;
            }
            else
            {
                box_color = BUTTON_BG_HOVER;
                text_color = BUTTON_TEXT_HOVER;
                buttons[idx]->state = HOVERING;
                was_mouse_down = false;
            }
        }
        else
        {
            box_color = BUTTON_BACKGROUND;
            text_color = BUTTON_TEXT;
            buttons[idx]->state = IDLE;
        }
        fill_rectangle(buttons[idx]->x, buttons[idx]->y, buttons[idx]->width, buttons[idx]->height, box_color, true);
        draw_text(buttons[idx]->text, buttons[idx]->x, buttons[idx]->y, BUTTON_TEXTSIZE, text_color, buttons[idx]->width, buttons[idx]->height);
    }
}

/*
This is a thin helper procedure that calls the process_events() method and sleeps the frame for a set time to achieve a target FPS
*/
inline void Window::update_window(double fps)
{
    process_events();

    int ms = (int)std::floor(1000.0 / fps);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif