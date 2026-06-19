#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
Parameters adjusting how the program runs
*/

#include "colors.hpp"
#include "constants.hpp"
#include "data-structures.hpp"
#include "unitsystem.hpp"
#include <string>

namespace Parameters {
// Simulation params
inline constexpr double TIME_STEP =
    0.005; // Simulation time between steps (unitless)
inline constexpr double SIM_TIME = 50; // Total simulation runtime (unitless)
inline constexpr double SOFTENING =
    0.1; // A softening factor to handle close collisions, avoiding dividing by
         // zero (unphysical)
inline Scale SCALE =
    SOLAR_SYSTEM; // initial scale - SOLAR_SYSTEM or INTERGALACTIC
inline const std::string CONFIG_DIR =
    "configurations"; // directory scanned for .json configs at runtime

// Window params
inline constexpr int WINDOW_WIDTH = 1220;
inline constexpr int WINDOW_HEIGHT = 760;
inline constexpr int FPS = 60;
inline constexpr int TITLE_TEXTSIZE = 65;
inline constexpr int BUTTON_TEXTSIZE = 22;
inline const std::string WINDOW_NAME = "Simulator";
inline const std::string GLOBAL_FONT = "Aboreto";

// Tick params
inline constexpr int TICK_COUNT = 5;     // target ticks per axis
inline constexpr int TICK_LENGTH_PX = 5; // tick mark length in pixels
inline constexpr int TICK_TEXTSIZE = 16;
inline const Color TICK_COLOR = StarWhite;

// Colours
inline const Color TITLE_COLOR = StarWhite;
inline const Color BACKGROUND_COLOR = SpaceVoid;
inline const Color BUTTON_BACKGROUND = MidnightNavy;
inline const Color BUTTON_TEXT = StarWhite;
inline const Color BUTTON_BG_HELD = StellarCyan;
inline const Color BUTTON_BG_HOVER = NebulaPurple;
inline const Color BUTTON_TEXT_HOVER = MidnightNavy;

// Camera params
inline Vec3 CAM_POSITION = {
    0.6, -2.5, 1.2}; // initial position (can change by clicking+dragging)
inline constexpr float CAM_FOV =
    50.0f * Constants::pi / 180.0f; // Field of view in radians
inline constexpr float CAM_ASPECT =
    1.3333f; // Keep to 4/3 unless absolutely necessary
inline constexpr float CAM_ZNEAR =
    0.1f; // Objects must be at least this far in front of camera to render
inline constexpr float CAM_ZFAR =
    50.0f; // Objects further than this number in front of camera won't render
inline constexpr float SCROLL_ZOOM_FACTOR =
    0.1f; // Increase for faster scrolling on 3D plot
inline constexpr float MIN_ZOOM_LEVEL = 10.0f;
inline constexpr float MAX_ZOOM_LEVEL = 750.0f;
inline constexpr double DRAG_SENSITIVITY = 0.005; // tune 0.004–0.008
inline constexpr double AXIS_3D_LOWER =
    -5; // currently no way to adjust 3D axis limits during runtime, but objects
        // will render beyond axes in 3D (unlike 2D)
inline constexpr double AXIS_3D_UPPER = 5;
inline double AXIS_2D_LOWER =
    -1.5; // initial 2D axis bounds, can be zoomed in/out during runtime
inline double AXIS_2D_UPPER = 1.5;
}; // namespace Parameters

#endif
