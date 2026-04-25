#include "helper/vec3.hpp"
#include "helper/console-input.hpp"
#include "body.hpp"
#include "parameters.hpp"
#include <iostream>
#include <vector>

using Parameters::time_step;

Body::Body(const BodyState &state) : data(state) {
    save_state();
}

Body::Body() : data() {}

void Body::print(int id = 0)
{
    std::cout << std::endl
              << "----- Body " << id << " -----" << std::endl;
    std::cout << "Mass: " << data.mass << std::endl;
    std::cout << "Angle: " << data.angle << std::endl;
    std::cout << "Rings: " << data.rings << std::endl;
    std::cout << "Position: (" << data.position.x << ", " << data.position.y << ", " << data.position.z << ")" << std::endl;
    std::cout << "Velocity: (" << data.velocity.x << ", " << data.velocity.y << ", " << data.velocity.z << ")" << std::endl;
    std::cout << "Acceleration: (" << data.acceleration.x << ", " << data.acceleration.y << ", " << data.acceleration.z << ")" << std::endl;
    std::cout << "---------------------" << std::endl;
}

void Body::save_state()
{
    data.position_history.push_back(data.position);
    data.velocity_history.push_back(data.velocity);
    data.acceleration_history.push_back(data.acceleration);
}
