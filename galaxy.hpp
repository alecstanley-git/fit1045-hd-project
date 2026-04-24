#ifndef GALAXY_H
#define GALAXY_H

#include "helper/vec3.hpp"
#include "helper/utilities.hpp"

class Galaxy
{
public:
    // Define the struct first in the class
    struct State
    {
        double mass = 1.0;
        double angle = 0.0;
        int rings = 0;

        Vec3 initial_position = {0.0, 0.0, 0.0};
        Vec3 initial_velocity = {0.0, 0.0, 0.0};
    };

private:
    // Constants - must be named in the constructor
    double mass = 1.0;
    double angle = 0.0;
    int rings = 0;

    // Initialised to zero
    Vec3 position = {0.0, 0.0, 0.0};
    Vec3 velocity = {0.0, 0.0, 0.0};
    Vec3 acceleration = {0.0, 0.0, 0.0};

public:
    // Main constructor using a member initialiser list
    Galaxy(const State &state)
        : mass(state.mass),
          angle(state.angle),
          rings(state.rings),
          position(state.initial_position),
          velocity(state.initial_velocity)
    {
        // Empty body for now
    }

    // Empty constructor called on program startup
    Galaxy() {}

    // Print galaxy parameters
    // @param id - allow a unique galaxy ID to be passed in and displayed
    void print(int id);
};

void Galaxy::print(int id = 0)
{
    cout << endl << "----- Galaxy " << id << " -----" << endl;
    cout << "Mass: " << mass << endl;
    cout << "Angle: " << angle << endl;
    cout << "Rings: " << rings << endl;
    cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << endl;
    cout << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << endl;
    cout << "Acceleration: (" << acceleration.x << ", " << acceleration.y << ", " << acceleration.z << ")" << endl;
    cout << "---------------------" << endl;
}

#endif