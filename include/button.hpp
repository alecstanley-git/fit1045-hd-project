#ifndef BUTTON_H
#define BUTTON_H

#include <iostream>
#include <string>
#include "point2d.hpp"

enum ButtonState
{
    IDLE,
    HOVERING,
    PRESSED
};

struct Button
{
    int x;
    int y;
    int width;
    int height;
    std::string text;
    ButtonState state;

    Button(int x_, int y_, int width_, int height_, std::string text_)
        : x(x_), y(y_), width(width_), height(height_), text(text_), state(IDLE) {}

    bool is_hovering(Point2D &mouse_position) const;
    bool is_pressed() const;

    void update_state(Point2D &mouse_position, bool mouse_pressed);
};

inline bool Button::is_hovering(Point2D &mouse_position) const
{
    return (mouse_position.x >= x && mouse_position.x <= x + width &&
            mouse_position.y >= y && mouse_position.y <= y + height);
}

inline bool Button::is_pressed() const
{
    if (state == PRESSED)
    {
        return true;
    }
    return false;
}

// State modifier function
inline void Button::update_state(Point2D &mouse_position, bool mouse_pressed)
{
    if (is_hovering(mouse_position))
    {
        if (mouse_pressed)
        {
            state = PRESSED;
        }
        else
        {
            state = HOVERING;
        }
    }
    else
    {
        state = IDLE;
    }
}

#endif