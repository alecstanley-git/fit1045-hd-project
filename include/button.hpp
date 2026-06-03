#ifndef BUTTON_H
#define BUTTON_H

#include <iostream>
#include <string>
#include "data-structures.hpp"

/*
Button state enum defining whether a button object is being hovered with the mouse, actively being pressed, or neither
*/
enum ButtonState
{
    IDLE,
    HOVERING,
    HELD,
    CLICKED
};

/*
Main button structure.
Only holds state information of a single button, does not contain any drawing logic
*/
struct Button
{
    int x;
    int y;
    int width;
    int height;
    std::string text; // Text to display over the button (currently required)
    ButtonState state;

    /*
    Default constructor, casts all initialised parameters into the new object and sets the state to idle
    @param x
    TODO
    */
    Button(int x_, int y_, int width_, int height_, std::string text_)
        : x(x_), y(y_), width(width_), height(height_), text(text_), state(IDLE) {}

    /*
    TODO
    */
    bool is_hovering(Point2D &mouse_position) const;

    bool is_clicked();
    bool is_held();
};

inline bool Button::is_hovering(Point2D &mouse_position) const
{
    return (mouse_position.x >= x && mouse_position.x <= x + width &&
            mouse_position.y >= y && mouse_position.y <= y + height);
}

inline bool Button::is_clicked()
{
    if (state == CLICKED)
    {
        state = IDLE;
        return true;
    }
    return false;
}

inline bool Button::is_held()
{
    if (state == HELD)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif