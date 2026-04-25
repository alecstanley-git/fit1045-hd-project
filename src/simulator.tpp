#include <iostream>
#include <string>
#include <array>
#include <cmath> // for infinity
#include "body.hpp"
#include "simulator.hpp"
#include "console-input.hpp"

template <int galaxy_count>
Simulator<galaxy_count>::Simulator() {}

template <int galaxy_count>
void Simulator<galaxy_count>::fill_galaxies()
{
    for (int i = 0; i < galaxy_count; i++)
    {
        std::cout << std::endl
                  << "[-] Initialising galaxy " << i + 1 << "..." << std::endl
                  << std::endl;
        BodyState params = fetch_user_config_console();
        Body new_galaxy(params);
        galaxies[i] = new_galaxy;
    }
    std::cout << std::endl
              << "[-] All galaxies initialised, ready to commence." << std::endl;
}

template <int galaxy_count>
BodyState Simulator<galaxy_count>::fetch_user_config_console()
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

template <int galaxy_count>
inline void Simulator<galaxy_count>::calculate_acceleration()
{
    Vec3 dx{};
    double distance{};

    for (int i = 0; i < galaxy_count; i++)
    {
        for (int j = 0; j < galaxy_count; j++)
        {
            if (i != j)
            {
                dx = galaxies[j].data.position - galaxies[i].data.position;
                distance = dx.magnitude();

                galaxies[i].data.acceleration = dx * (galaxies[j].data.mass/(distance * distance * distance));
            }
        }
    }
}

template <int galaxy_count>
inline void Simulator<galaxy_count>::leapfrog()
{
    for (int i = 0; i < galaxy_count; i++)
    {
        galaxies[i].data.velocity = galaxies[i].data.velocity + galaxies[i].data.acceleration * (0.5 * time_step);
        galaxies[i].data.position = galaxies[i].data.position + galaxies[i].data.velocity * time_step;
    }
    calculate_acceleration();
    for (int i = 0; i < galaxy_count; i++)
    {
        galaxies[i].data.velocity = galaxies[i].data.velocity + galaxies[i].data.acceleration * (0.5 * time_step);

        galaxies[i].save_state();
    }
}

template <int galaxy_count>
inline void Simulator<galaxy_count>::integrate()
{
    while (step < (int)(sim_time/time_step))
    {
        leapfrog();
        step += 1;

        for (int i = 0; i < (int)galaxies.size(); i++)
        {
            galaxies[i].print(i + 1);
        }
    }
}