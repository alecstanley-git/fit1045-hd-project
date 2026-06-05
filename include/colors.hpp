#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

/*
This enum stores colour information in hexadecimal format.
Both Windows (GDI+) and Mac (NSColor) store colour differently. This format is easy to translate in the OS-specific methods by bitshifting.

It lives in its own header (with no dependencies) so that both window.hpp and
parameters.hpp can use it without including each other.
*/
enum Color : std::uint64_t
{
    Red = 0xFF0000FF,
    Green = 0x00FF00FF,
    Blue = 0x0000FFFF,
    Black = 0x000000FF,
    Grey = 0x707070FF,
    LightGrey = 0xC2C2C2FF,
    White = 0xFFFFFFFF,

    // I used AI to generate a nice colour palette for me
    SpaceVoid = 0x0B0E1AFF,      // near-black deep-space backdrop with a faint blue tint
    MidnightNavy = 0x1B2A4AFF,   // deep navy for panels and gradient skies
    NebulaPurple = 0x6C5CE7FF,   // vivid nebula violet for accents
    NebulaPink = 0xC44FE0FF,     // magenta nebula glow
    CosmicTeal = 0x12B5B0FF,     // cyan-teal for highlights and rings
    StellarCyan = 0x4FD1E0FF,    // bright cyan for UI glints and trails
    StarWhite = 0xF5F7FFFF,      // cool white for stars and text
    SolarGold = 0xFFC857FF,      // warm gold for stars and the sun
    SolarOrange = 0xFF8C42FF,    // orange for stellar flares and warnings
    MarsRust = 0xC1440EFF,       // rusty red-orange for rocky planets
    PlasmaPink = 0xFF4D9DFF,     // hot pink for energy / thrust effects
    AsteroidGrey = 0x4A4E69FF    // muted blue-grey for asteroids and rock
};

#endif
