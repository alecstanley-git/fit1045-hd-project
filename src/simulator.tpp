#include <iostream>
#include <string>
#include <array>
#include <cmath> // for infinity
#include "galaxy.hpp"
#include "simulator.hpp"
#include "console-input.hpp"

template <int galaxy_count>
Simulator<galaxy_count>::Simulator() {}

template <int galaxy_count>
void Simulator<galaxy_count>::fill_galaxies()
{
    for (int i = 0; i < galaxy_count; i++)
    {
        std::cout << std::endl << "[-] Initialising galaxy " << i+1 << "..." << std::endl << std::endl;
        GalaxyState params = fetch_user_config_console();
        Galaxy new_galaxy(params);
        galaxies[i] = new_galaxy;
    }
    std::cout << std::endl << "[-] All galaxies initialised, ready to commence." << std::endl;
}

template <int galaxy_count>
GalaxyState Simulator<galaxy_count>::fetch_user_config_console()
{
    GalaxyState config;
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