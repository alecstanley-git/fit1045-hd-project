#include "vec3.hpp"
#include "console-input.hpp"
#include "galaxy.hpp"
#include <iostream>

Galaxy::Galaxy(const GalaxyState &state) : data(state) {}

Galaxy::Galaxy() : data() {}

void Galaxy::print(int id = 0)
{
    std::cout << std::endl
              << "----- Galaxy " << id << " -----" << std::endl;
    std::cout << "Mass: " << data.mass << std::endl;
    std::cout << "Angle: " << data.angle << std::endl;
    std::cout << "Rings: " << data.rings << std::endl;
    std::cout << "Position: (" << data.position.x << ", " << data.position.y << ", " << data.position.z << ")" << std::endl;
    std::cout << "Velocity: (" << data.velocity.x << ", " << data.velocity.y << ", " << data.velocity.z << ")" << std::endl;
    std::cout << "Acceleration: (" << data.acceleration.x << ", " << data.acceleration.y << ", " << data.acceleration.z << ")" << std::endl;
    std::cout << "---------------------" << std::endl;
}