#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <string>
#include <array>
#include <cmath> // for infinity
#include "body.hpp"
#include "simulator.hpp"
#include "console-input.hpp"
#include "parameters.hpp"
#include "dynamic-array.hpp"

using namespace Parameters;

struct Simulator
{
    int n_bodies = 0;
    double step = 0;
    dynamic_array<Body> galaxies;

    Simulator();

    void fill_galaxies();

    BodyState fetch_user_config_console();

    void print_all_galaxies();

    void calculate_acceleration();

    void leapfrog();
};

Simulator::Simulator() {}

void Simulator::fill_galaxies()
{
    std::cout << std::endl;
    n_bodies = read_integer_range("How many bodies in the simulation? ", 1, INT_MAX);
    std::cout << std::endl;

    for (int i = 0; i < n_bodies; i++)
    {
        std::cout << std::endl
                  << "[-] Initialising galaxy " << i + 1 << "..." << std::endl
                  << std::endl;
        BodyState params = fetch_user_config_console();
        Body new_galaxy(params);
        galaxies.add(new_galaxy);
    }
    std::cout << std::endl
              << "[-] All galaxies initialised, ready to commence." << std::endl;
}

BodyState Simulator::fetch_user_config_console()
{
    BodyState config;
    config.mass = read_double_range("   Enter mass (solar masses): ", 0, INFINITY, false);
    config.angle = read_double_range("   Enter angle (degrees): ", -180, 180);
    config.rings = read_integer_range("   Enter number of rings: ", 0, INT_MAX);
    bool custom_position = read_boolean("   Want to enter a custom initial position (defaults to origin)?");
    if (!custom_position)
    {
        config.position = {0.0, 0.0, 0.0};
    }
    else
    {
        config.position.x = read_double("     X: ");
        config.position.y = read_double("     Y: ");
        config.position.z = read_double("     Z: ");
    }

    bool custom_velocity = read_boolean("   Want to enter a custom initial velocity (defaults to [1, 0, 0])?");
    if (!custom_velocity)
    {
        config.velocity = {1.0, 0.0, 0.0};
    }
    else
    {
        config.velocity.x = read_double("     Vx: ");
        config.velocity.y = read_double("     Vy: ");
        config.velocity.z = read_double("     Vz: ");
    }

    return config;
}

inline void Simulator::print_all_galaxies()
{
    for (int i = 0; i < (int)galaxies.length(); i++)
    {
        galaxies[i].print(i + 1);
    }
}

inline void Simulator::calculate_acceleration()
{
    Vec3 dx{};
    double distance_sq{};
    double distance{};

    for (int i = 0; i < n_bodies; i++)
    {
        galaxies[i].data.acceleration = Vec3{0.0, 0.0, 0.0};
        for (int j = 0; j < n_bodies; j++)
        {
            if (i != j)
            {
                dx = galaxies[j].data.position - galaxies[i].data.position;

                // Softened distance
                distance_sq = dx.x * dx.x + dx.y * dx.y + dx.z * dx.z + softening * softening;
                distance = std::sqrt(distance_sq);

                galaxies[i].data.acceleration += dx * (galaxies[j].data.mass / (distance * distance_sq));
            }
        }
    }
}

inline void Simulator::leapfrog()
{
    for (int i = 0; i < n_bodies; i++)
    {
        galaxies[i].data.velocity = galaxies[i].data.velocity + galaxies[i].data.acceleration * (0.5 * time_step);
        galaxies[i].data.position = galaxies[i].data.position + galaxies[i].data.velocity * time_step;
    }
    calculate_acceleration();
    for (int i = 0; i < n_bodies; i++)
    {
        galaxies[i].data.velocity = galaxies[i].data.velocity + galaxies[i].data.acceleration * (0.5 * time_step);

        galaxies[i].save_state();
    }

    step += 1;
}

#endif