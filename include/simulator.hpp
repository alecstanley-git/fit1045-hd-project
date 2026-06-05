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
#include "constants.hpp"
#include "dynamic-array.hpp"
#include "parsejson.hpp"

using namespace Parameters;
using namespace Constants;

enum Integrator
{
    LEAPFROG
};

enum SimState
{
    INACTIVE,
    ACTIVE
};

// Fast Inverse Square Root (Double Precision)
// This avoids the expensive std::sqrt division
inline double fast_inv_sqrt_d(double number)
{
    double y = number;
    double x2 = y * 0.5;
    std::int64_t i = *(std::int64_t *)&y;

    i = 0x5fe6eb50c7b537a9 - (i >> 1);
    y = *(double *)&i;

    // One iteration of Newton's method
    y = y * (1.5 - (x2 * y * y));
    return y;
}

class Simulator
{
    void leapfrog();

public:
    int n_bodies = 0;
    int current_step = 0;
    SimState state = INACTIVE;
    dynamic_array<Body> bodies;

    Simulator();
    void fill_console();
    void fill_from_file(const std::string &path);
    Vec3 read_vec3(const JsonValue &obj, const std::string &key);
    BodyState fetch_user_config_console();
    void print_all();
    void build_rings(const BodyState &params);
    void calculate_acceleration();
    void step(Integrator integrator, const bool &override_time);
};

Simulator::Simulator() {}

void Simulator::fill_console()
{
    std::cout << std::endl;
    int to_add = read_integer_range("How many masses in the simulation? ", 1, INT_MAX);
    std::cout << std::endl;

    for (int i = 0; i < to_add; i++)
    {
        std::cout << std::endl
                  << "[-] Initialising body " << i + 1 << "..." << std::endl
                  << std::endl;
        BodyState params = fetch_user_config_console();
        Body new_body(params);
        n_bodies += 1;
        bodies.add(new_body);

        if (params.rings > 0)
            build_rings(params);
    }
    std::cout << std::endl
              << "[-] "
              << n_bodies
              << " bodies initialised, ready to commence." << std::endl;
}

inline Vec3 Simulator::read_vec3(const JsonValue &obj, const std::string &key)
{
    Vec3 v{};
    const JsonValue *sub = Lookup(obj, key);
    if (sub != nullptr)
    {
        v.x = GetDouble(*sub, "x", 0.0);
        v.y = GetDouble(*sub, "y", 0.0);
        v.z = GetDouble(*sub, "z", 0.0);
    }
    return v;
}

inline void Simulator::fill_from_file(const std::string &path)
{
    JsonValue config = ParseJson(path);
    int to_add = GetInt(config, "count", 0);

    for (int i = 1; i <= to_add; i++)
    {
        const JsonValue *body = Lookup(config, std::to_string(i));
        if (body == nullptr)
            continue; // body key missing; skip

        BodyState params;
        params.mass = GetDouble(*body, "mass", 1.0);
        params.angle = GetDouble(*body, "angle", 0.0);
        params.rings = GetInt(*body, "rings", 0);
        params.position = read_vec3(*body, "position");
        params.velocity = read_vec3(*body, "velocity");

        n_bodies += 1;
        bodies.add(Body(params));

        if (params.rings > 0)
            build_rings(params);
    }

    FreeJson(config); // release the parsed tree (matches the main.cpp pattern)
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

inline void Simulator::print_all()
{
    for (int i = 0; i < (int)bodies.length(); i++)
    {
        bodies[i].print(i + 1);
    }
}

inline void Simulator::build_rings(const BodyState &params)
{
    // Pre-calculate constants for the primary body
    double a_rad = params.angle * pi / 180.0;
    double cos_a = std::cos(a_rad);
    double sin_a = std::sin(a_rad);
    double softening_sq = SOFTENING * SOFTENING;

    // Adding rings according to Toomre and Toomre 1972
    for (int i = 1; i < params.rings + 1; i++)
    {
        double ri = i * dr;
        double nphi = 12 + 6 * (i - 1);
        double r_sq = ri * ri;
        double dist_sq = r_sq + softening_sq;
        double vphi = std::sqrt((params.mass * r_sq) / (dist_sq * std::sqrt(dist_sq)));

        double dphi = 2 * pi / nphi;
        double cos_dphi = std::cos(dphi);
        double sin_dphi = std::sin(dphi);

        double cos_phi = 1.0;
        double sin_phi = 0.0;

        for (int j = 1; j < nphi + 1; j++)
        {
            BodyState config;
            config.mass = 0.0;
            config.angle = 0.0;
            config.rings = 0;

            config.position.x = params.position.x + ri * cos_phi * cos_a;
            config.position.y = params.position.y + ri * sin_phi;
            config.position.z = params.position.z - ri * cos_phi * sin_a;

            config.velocity.x = params.velocity.x - vphi * sin_phi * cos_a;
            config.velocity.y = params.velocity.y + vphi * cos_phi;
            config.velocity.z = params.velocity.z + vphi * sin_phi * sin_a;

            n_bodies += 1;
            bodies.add(Body(config));

            // Advance phi using rotation matrix
            double next_cos = cos_phi * cos_dphi - sin_phi * sin_dphi;
            double next_sin = sin_phi * cos_dphi + cos_phi * sin_dphi;
            cos_phi = next_cos;
            sin_phi = next_sin;
        }
    }
}

inline void Simulator::calculate_acceleration()
{
    Vec3 dx{};
    double distance_sq{};
    double inv_distance{};
    double inv_distance3{};

    for (int i = 0; i < n_bodies; i++)
    {
        bodies[i].data.acceleration = Vec3{0.0, 0.0, 0.0};
        for (int j = 0; j < n_bodies; j++)
        {
            if (i == j || bodies[j].data.mass == 0.0)
                continue;

            dx = bodies[j].data.position - bodies[i].data.position;

            // Softened distance
            distance_sq = dx.x * dx.x + dx.y * dx.y + dx.z * dx.z + SOFTENING * SOFTENING;
            inv_distance = fast_inv_sqrt_d(distance_sq);
            inv_distance3 = inv_distance * inv_distance * inv_distance;

            bodies[i].data.acceleration += dx * (bodies[j].data.mass * inv_distance3);
        }
    }
}

inline void Simulator::leapfrog()
{
    for (int i = 0; i < n_bodies; i++)
    {
        bodies[i].data.velocity = bodies[i].data.velocity + bodies[i].data.acceleration * (0.5 * TIME_STEP);
        bodies[i].data.position = bodies[i].data.position + bodies[i].data.velocity * TIME_STEP;
    }
    calculate_acceleration();
    for (int i = 0; i < n_bodies; i++)
    {
        bodies[i].data.velocity = bodies[i].data.velocity + bodies[i].data.acceleration * (0.5 * TIME_STEP);
    }
}

inline void Simulator::step(Integrator integrator, const bool &override_time = false)
{
    if (current_step < (int)(SIM_TIME / TIME_STEP) || override_time)
    {
        switch (integrator)
        {
        case LEAPFROG:
            leapfrog();
            break;
        default:
            break;
        }
        current_step += 1;
    }
}

#endif