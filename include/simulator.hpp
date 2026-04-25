#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <array>
#include "galaxy.hpp"

template <int galaxy_count>
struct Simulator
{
    std::array<Galaxy, galaxy_count> galaxies;

    Simulator();

    void fill_galaxies();

    GalaxyState fetch_user_config_console();
};

#include "../src/simulator.tpp"

#endif