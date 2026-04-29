#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>

class Window
{
    int width;
    int height;
    std::string title;
    bool is_open = false;
    void* handle; // This points to the os-specific window object - must be a pointer*.

public:
    Window(int _width, int _height, std::string _title);

    int open();
};

#endif