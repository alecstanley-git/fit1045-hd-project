#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
Parameters adjusting how the program runs
*/

#include <iostream>
#include <string>
#include "window.hpp"

namespace Parameters
{
    inline constexpr double TIME_STEP = 0.01;
    inline constexpr double SIM_TIME = 25;
    inline constexpr double SOFTENING = 0.1; // A softening factor to handle close collisions, avoiding dividing by zero (unphysical)

    // Window params
    inline constexpr int WINDOW_WIDTH = 800;
    inline constexpr int WINDOW_HEIGHT = 600;
    inline const std::string WINDOW_NAME = "Simulator";
    inline const Color BACKGROUND_COLOR = LightGrey;
    inline const std::string GLOBAL_FONT = "Aboreto-Regular";
};

#endif