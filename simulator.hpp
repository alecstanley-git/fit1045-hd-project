#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <string>
#include <array>
#include "galaxy.hpp"
#include "simulator.hpp"

template <int galaxy_count>
struct Simulator
{
    array<Galaxy, galaxy_count> galaxies;

    Simulator()
    {
    }

    void fill_galaxies()
    {
        for (int i = 0; i < galaxy_count; i++)
        {
            cout << endl << "[-] Initialising galaxy " << i+1 << "..." << endl << endl;
            Galaxy::State params = fetch_user_config_console();
            Galaxy new_galaxy(params);
            galaxies[i] = new_galaxy;
        }
        cout << endl << "[-] All galaxies initialised, ready to commence." << endl;
    }

    Galaxy::State fetch_user_config_console()
    {
        Galaxy::State config;
        config.mass = read_double_range("   Enter mass (solar masses): ", 0, INFINITY, false);
        config.angle = read_double_range("   Enter angle (degrees): ", -180, 180);
        config.rings = read_integer_range("   Enter number of rings: ", 0, INFINITY);
        bool custom_position = read_boolean("   Want to enter a custom initial position (defaults to origin)?");
        if (!custom_position)
        {
            config.initial_position = {0.0, 0.0, 0.0};
        }
        else
        {
            config.initial_position.x = read_double("     X: ");
            config.initial_position.y = read_double("     Y: ");
            config.initial_position.z = read_double("     Z: ");
        }

        bool custom_velocity = read_boolean("   Want to enter a custom initial velocity (defaults to [1, 0, 0])?");
        if (!custom_velocity)
        {
            config.initial_velocity = {1.0, 0.0, 0.0};
        }
        else
        {
            config.initial_velocity.x = read_double("     Vx: ");
            config.initial_velocity.y = read_double("     Vy: ");
            config.initial_velocity.z = read_double("     Vz: ");
        }

        return config;
    }
};

#endif