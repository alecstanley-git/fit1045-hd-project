#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
Parameters adjusting how the program runs
*/

#include <iostream>
#include <string>
#include "constants.hpp"
#include "window.hpp"
#include "data-structures.hpp"

namespace Parameters
{
    inline constexpr double TIME_STEP = 0.01;
    inline constexpr double SIM_TIME = 25;
    inline constexpr double SOFTENING = 0.1; // A softening factor to handle close collisions, avoiding dividing by zero (unphysical)

    // Window params
    inline constexpr int WINDOW_WIDTH = 800;
    inline constexpr int WINDOW_HEIGHT = 600;
    inline constexpr int FPS = 60;
    inline const std::string WINDOW_NAME = "Simulator";
    inline const Color BACKGROUND_COLOR = LightGrey;
    inline const std::string GLOBAL_FONT = "Aboreto";

    // Camera params
    inline const Vec3 CAM_POSITION = {0.5, -2.0, 1.0};
    inline constexpr float CAM_FOV = 45.0f * Constants::pi / 180.0f;
    inline constexpr float CAM_ASPECT = 1.3333f;
    inline constexpr float CAM_ZNEAR = 0.1f;
    inline constexpr float CAM_ZFAR = 50.0f;
};

#endif