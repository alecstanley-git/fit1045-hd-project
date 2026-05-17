#ifndef PARAMETERS_H
#define PARAMETERS_H

/*
Eventually this file will be deprecated in favour of a GUI-based parameter selection panel
*/

namespace Parameters
{
    inline constexpr int num_galaxies = 2; // Number of galaxies to simulate
    inline constexpr double time_step = 0.01;
    inline constexpr double sim_time = 25;
    inline constexpr double softening = 0.1; // A softening factor to handle close collisions, avoiding dividing by zero (unphysical)
};

#endif