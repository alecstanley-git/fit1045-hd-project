#ifndef BUTTON_H
#define BUTTON_H

#include "data-structures.hpp"
#include <string>

/*
Button state enum defining whether a button object is being hovered with the
mouse, actively being pressed, or neither
*/
enum ButtonState { IDLE, HOVERING, HELD, CLICKED };

/*
Main button structure.
Only holds state information of a single button, does not contain any drawing
logic
*/
struct Button {
  int x;
  int y;
  int width;
  int height;
  std::string text;
  ButtonState state;

  /*
  Default constructor, casts all initialised parameters into the new object and
  sets the state to idle
  @param int x_ - x-position (top left)
  @param int y_ - y-position (top left)
  @param int width_ - the button width in pixels
  @param int height_ - the button height in pixels
  @param std::string text_ - the text to display
  */
  Button(int x_, int y_, int width_, int height_, std::string text_)
      : x(x_), y(y_), width(width_), height(height_), text(text_), state(IDLE) {
  }

  /*
  A simple check to see if the current mouse position lies over the button
  boundary
  @param Point2D &mouse_position - the x, y position of the mouse, passed as a
  reference
  */
  bool is_hovering(Point2D &mouse_position) const;

  // Checks to see if mouse is clicked or actively being held
  bool is_clicked();
  bool is_held();
};

inline bool Button::is_hovering(Point2D &mouse_position) const {
  return (mouse_position.x >= x && mouse_position.x <= x + width &&
          mouse_position.y >= y && mouse_position.y <= y + height);
}

inline bool Button::is_clicked() {
  if (state == CLICKED) {
    state = IDLE;
    return true;
  }
  return false;
}

inline bool Button::is_held() {
  if (state == HELD) {
    return true;
  } else {
    return false;
  }
}

#endif
