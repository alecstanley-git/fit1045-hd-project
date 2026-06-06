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
    int width;            // Width of the window in pixels
    int height;           // Height of the window in pixels
    std::string title;    // Title shown in the window's title bar
    bool is_open = false; // True while the window is alive; flipped false when it closes
    void *_window;        // This points to the os-specific window object - must be a pointer*.

    // Interaction fields
    Point2D mouse_position = {0, 0};     // Current mouse position in window pixels, updated each frame
    Point2D previous_mouse_pos = {0, 0}; // Mouse position from the previous frame, used to compute velocity
    bool is_mouse_down = false;          // True while the left mouse button is currently pressed
    bool was_mouse_down = false;         // Tracks the previous press state to distinguish a fresh click from a hold

    // Registers the os-specific event handlers (mouse, scroll) needed for interaction.
    // Only ran once during window construction.
    void setup_input_listeners();

public:
    // The buttons should be a publicly accessible field - they are just pointers to the structs
    dynamic_array<Button *> buttons;
    float zoom_level = 100.0f; // Running zoom total driven by the scroll wheel, consumed by the camera

    /*
    Default constructor
    */
    // @param int _width - desired window width in pixels
    // @param int _height - desired window height in pixels
    // @param std::string _title - text shown in the window's title bar
    Window(int _width, int _height, std::string _title);

    // Destructor - frees every heap-allocated button owned by the window
    ~Window();

    // Basic window management methods, os-specific

    // Pumps the os event queue (clicks, drags, close) and refreshes mouse state. Called every frame.
    void process_events();

    // Wipes the window to a single background color, clearing the previous frame.
    // @param std::uint64_t color - the 0xRRGGBBAA fill color
    void clear_screen(std::uint64_t color);

    // @return bool - true while the window is still open
    bool is_running();

    // Rendering methods, os-specific

    // Draws a filled rectangle. Buttons get rounded corners and a soft shadow.
    // @param int x, y - pixel position of the rectangle's top-left corner
    // @param int width, height - pixel dimensions of the rectangle
    // @param Color color - fill color
    // @param bool is_button - when true, renders the button styling (defaults to false)
    void fill_rectangle(int x, int y, int width, int height, Color color, bool is_button = false);

    // Draws a filled, anti-aliased circle.
    // @param int x, y - pixel position of the circle's centre
    // @param int radius - radius of the circle in pixels
    // @param Color color - fill color
    void fill_circle(int x, int y, int radius, Color color);

    // Draws a straight line between two points.
    // @param int x1, y1 - first endpoint in screen pixels
    // @param int x2, y2 - second endpoint in screen pixels
    // @param Color color - line color
    // @param int linewidth - stroke width in pixels (defaults to 1)
    void draw_line(int x1, int y1, int x2, int y2, Color color, int linewidth = 1);

    // Draws a string centred within the given box.
    // @param const std::string &text - the text to draw
    // @param int x, y - pixel position of the text box's top-left corner
    // @param double size - font size in pixels
    // @param Color color - text color
    // @param int box_width, box_height - dimensions of the box the text is centred within
    void draw_text(const std::string &text, int x, int y, double size, Color color, int box_width, int box_height);

    // Registers a .ttf font file with the os so draw_text() can use it.
    // @param const std::string &file_path - path to the font file
    // @return bool - false if the font failed to load, true otherwise
    bool load_font(const std::string &file_path);

    // High-level os-agnostic methods defined within header itself

    // Allocates a new button, stores it in the buttons array, and returns a pointer to it.
    // @param int x, y - pixel position of the button's top-left corner
    // @param int width, height - pixel dimensions of the button
    // @param std::string text - the button's label
    // @return Button* - pointer to the newly created button
    Button *add_button(int x, int y, int width, int height, std::string text);

    // Updates the interaction state of the listed buttons and redraws them with the matching style.
    // @param dynamic_array<int> &indices - indices into the buttons array to process this frame
    void process_buttons(dynamic_array<int> &indices);

    // Thin per-frame helper: processes events then sleeps to hit the target frame rate.
    // @param double fps - the target frames per second
    void update_window(double fps);

    // @return Point2D - mouse movement (dx, dy) since the last frame, or {0,0} when the button is up
    Point2D mouse_velocity();
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
                // The button was already down last frame, so this is a sustained hold
                if (was_mouse_down)
                {
                    buttons[idx]->state = HELD;
                }
                // First frame of the press: register a one-shot CLICKED, then latch was_mouse_down
                // so subsequent frames are treated as HELD rather than repeated clicks
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

inline Point2D Window::mouse_velocity()
{
    // Difference between this frame's and last frame's mouse position
    int dx = mouse_position.x - previous_mouse_pos.x;
    int dy = mouse_position.y - previous_mouse_pos.y;
    previous_mouse_pos = mouse_position; // ALWAYS update, even when up

    // Only report movement while dragging, so a release doesn't register a phantom jump
    if (!is_mouse_down)
        return {0, 0};
    return {dx, dy};
}

#endif