#include "body.hpp"
#include "parameters.hpp"
#include "data-structures.hpp"
#include "console-input.hpp"
#include <iostream>
#include <vector>

using Parameters::TIME_STEP;

Body::Body(const BodyState &state) : data(state) {}

Body::Body() : data() {}

void Body::print(int id = 0)
{
    std::cout << std::endl
              << "----- Body " << id << " -----" << std::endl;
    if (data.mass != 0.0)
    {
        std::cout << "Mass: " << data.mass << std::endl;
        std::cout << "Angle: " << data.angle << std::endl;
        std::cout << "Rings: " << data.rings << std::endl;
    }
    std::cout << "Position: (" << data.position.x << ", " << data.position.y << ", " << data.position.z << ")" << std::endl;
    std::cout << "Velocity: (" << data.velocity.x << ", " << data.velocity.y << ", " << data.velocity.z << ")" << std::endl;
    std::cout << "Acceleration: (" << data.acceleration.x << ", " << data.acceleration.y << ", " << data.acceleration.z << ")" << std::endl;
    std::cout << "---------------------" << std::endl;
}
