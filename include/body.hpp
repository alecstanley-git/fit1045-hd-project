#ifndef BODY_H
#define BODY_H

#include "data-structures.hpp"
#include "parameters.hpp"

using Parameters::SIM_TIME;
using Parameters::TIME_STEP;

// Stores configuration for a galaxy/star - passed into the Body class
// Default values must be used here to avoid errors if the body is not
// initialised properly by the user
struct BodyState {
  // Intrinsic parameters
  double mass = 1.0;
  double angle = 0.0;
  int rings = 0;

  // State parameters
  Vec3 position{};
  Vec3 velocity{};
  Vec3 acceleration{};

  /*
  // These fields could be used in future versions of the simulator
  double kinetic_energy = 0;
  double potential_energy = 0;
  Vec3 angular_momentum{};
  */
};

struct Body {
  // State parameters - passed in from the constructor or initialised by default
  // in GalaxyState
  BodyState data;

  // Main constructor
  // @param const BodyState &state - the struct of parameters to build the Body
  // from
  Body(const BodyState &state);

  // Empty constructor called on program startup
  Body();

  // Print galaxy parameters
  // @param id - allow a unique galaxy ID to be passed in and displayed
  void print(int id);
};

#endif
