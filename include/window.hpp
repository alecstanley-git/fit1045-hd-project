#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>

enum Color
{

};

class Window
{
    int width;
    int height;
    std::string title;
    bool is_open = false;
    void* _window; // This points to the os-specific window object - must be a pointer*.

public:
    Window(int _width, int _height, std::string _title);

    void process_events();
    void clear_screen();
};

#endif