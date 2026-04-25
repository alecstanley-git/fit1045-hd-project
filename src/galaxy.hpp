#ifndef GALAXY_H
#define GALAXY_H

#include "vec3.hpp"

// Stores configuration for a galaxy - passed into the Galaxy class
// Default values must be used here to avoid errors if the galaxy is not initialised properly by the user
struct GalaxyState
{
    double mass = 1.0;
    double angle = 0.0;
    int rings = 0;

    Vec3 position{};
    Vec3 velocity{};
    Vec3 acceleration{};
};

class Galaxy
{
private:
    // State parameters - passed in from the constructor or initialised by default in GalaxyState
    GalaxyState data;

public:
    // Main constructor
    Galaxy(const GalaxyState &state);

    // Empty constructor called on program startup
    Galaxy();

    // Print galaxy parameters
    // @param id - allow a unique galaxy ID to be passed in and displayed
    void print(int id);
};

#endif