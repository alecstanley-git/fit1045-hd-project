#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
Parameters adjusting how the program runs
*/

#include <iostream>
#include <string>
#include "constants.hpp"
#include "colors.hpp"
#include "data-structures.hpp"

namespace Parameters
{
    // Simulation params
    inline constexpr double TIME_STEP = 0.01;
    inline constexpr double SIM_TIME = 25;
    inline constexpr double SOFTENING = 0.1; // A softening factor to handle close collisions, avoiding dividing by zero (unphysical)
    inline const std::string CONFIG_FILEPATH = "configurations/triple_collision.json";

    // Window params
    inline constexpr int WINDOW_WIDTH = 1220;
    inline constexpr int WINDOW_HEIGHT = 760;
    inline constexpr int FPS = 60;
    inline constexpr int TITLE_TEXTSIZE = 65;
    inline constexpr int BUTTON_TEXTSIZE = 24;
    inline const std::string WINDOW_NAME = "Simulator";
    inline const std::string GLOBAL_FONT = "Aboreto";

    // Colours
    inline const Color TITLE_COLOR = StarWhite;
    inline const Color BACKGROUND_COLOR = SpaceVoid;
    inline const Color BUTTON_BACKGROUND = MidnightNavy;
    inline const Color BUTTON_TEXT = StarWhite;
    inline const Color BUTTON_BG_HELD = StellarCyan;
    inline const Color BUTTON_BG_HOVER = NebulaPurple;
    inline const Color BUTTON_TEXT_HOVER = MidnightNavy;

    // Camera params
    inline const Vec3 CAM_POSITION = {0.6, -2.5, 1.2};
    inline constexpr float CAM_FOV = 50.0f * Constants::pi / 180.0f;
    inline constexpr float CAM_ASPECT = 1.3333f;
    inline constexpr float CAM_ZNEAR = 0.1f;
    inline constexpr float CAM_ZFAR = 50.0f;
};

#endif