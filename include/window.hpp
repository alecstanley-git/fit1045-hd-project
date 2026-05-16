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

public:
    // The buttons should be a publicly accessible field - they are just pointers to the structs
    dynamic_array<Button*> buttons;

    /*
    Default constructor
    TODO - @params
    */
    Window(int _width, int _height, std::string _title);

    // Basic window management methods
    void process_events();
    void clear_screen(Color color);
    bool is_running();

    // Rendering methods
    void fill_rectangle(int x, int y, int width, int height, Color color);
    void draw_text(const std::string &text, int x, int y, double size, Color color, int box_width, int box_height);
    bool is_left_mouse_down() const;
    bool load_font(const std::string& file_path);

    // High-level methods
    Button* add_button(int x, int y, int width, int height, std::string text);
    void process_buttons();
};

inline bool Window::is_running()
{
    return is_open;
}

inline Button* Window::add_button(int x, int y, int w, int h, std::string text)
{
    Button* ptr = new Button(x, y, w, h, text);
    buttons.add(ptr);
    return ptr;
}

inline void Window::process_buttons()
{
    Color box_color;
    Color text_color;
    for (int i = 0; i < buttons.length(); i++)
    {
        if (buttons[i]->is_hovering(mouse_position))
        {
            if (is_left_mouse_down())
            {
                box_color = Blue;
                text_color = White;
                buttons[i]->update_state(mouse_position, true);
            }
            else
            {
                box_color = LightGrey;
                text_color = Black;
                buttons[i]->update_state(mouse_position, false);
            }
        }
        else
        {
            box_color = Grey;
            text_color = White;
            buttons[i]->update_state(mouse_position, false);
        }
        fill_rectangle(buttons[i]->x, buttons[i]->y, buttons[i]->width, buttons[i]->height, box_color);
        draw_text(buttons[i]->text, buttons[i]->x, buttons[i]->y+buttons[i]->height/4, 18, text_color, buttons[i]->width, buttons[i]->height);
    }
}

#endif