#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <array>
#include "body.hpp"

template <int galaxy_count>
struct Simulator
{
    double step = 0;
    std::array<Body, galaxy_count> galaxies;

    Simulator();

    void fill_galaxies();

    BodyState fetch_user_config_console();

    void calculate_acceleration();

    void leapfrog();

    void integrate();
};

#include "../src/simulator.tpp"

#endif