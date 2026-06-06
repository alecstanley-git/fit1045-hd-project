#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include <string>
#include <array>
#include <filesystem> // for listing config files at runtime
#include <cmath>      // for infinity and sqrt
#include "body.hpp"
#include "simulator.hpp"
#include "console-input.hpp"
#include "parameters.hpp"
#include "constants.hpp"
#include "dynamic-array.hpp"
#include "parsejson.hpp"

using namespace Parameters;
using namespace Constants;

// Selects which numerical integration scheme advances the simulation
// Currently only the leapfrog method is implemented
enum Integrator
{
    LEAPFROG
};

// Tracks whether the simulation is currently running or paused/stopped
enum SimState
{
    INACTIVE,
    ACTIVE
};

class Simulator
{
    // Advances every body one TIME_STEP forward using the leapfrog (kick-drift-kick) scheme
    void leapfrog();
    // Advances every body one TIME_STEP backward, exploiting leapfrog's time-reversibility
    void leapfrog_reverse();

public:
    int n_bodies = 0;           // Total number of bodies currently in the simulation
    int current_step = 0;       // Index of the current integration step (0 at simulation start)
    SimState state = INACTIVE;  // Whether the simulation is running or paused
    dynamic_array<Body> bodies; // Buffer of every body being simulated

    // Default constructor; bodies are added later via the fill_* methods
    Simulator();

    // Interactively prompts the user at the console to build the body list
    void fill_console();

    // Loads the body list from a JSON configuration file
    // @param const std::string &path - path to the JSON config file
    void fill_from_file(const std::string &path);

    // Lists the available JSON configs and asks the user to pick one
    // @return std::string - full path of the chosen file, or "" if none available
    static std::string choose_config_file();

    // Reads a {x, y, z} sub-object from a JSON value into a Vec3
    // @param const JsonValue &obj - the parent JSON object to read from
    // @param const std::string &key - key of the vector sub-object
    // @return Vec3 - the parsed vector, or a zero vector if the key is missing
    Vec3 read_vec3(const JsonValue &obj, const std::string &key);

    // Prompts the user at the console for a single body's initial state
    // @return BodyState - the mass, angle, rings, position and velocity entered
    BodyState fetch_user_config_console();

    // Prints the state of every body in the simulation to the console
    void print_all();

    // Generates a set of massless test-particle rings around a primary body
    // Follows the format from Toomre and Toomre (1972)
    // @param const BodyState &params - the primary body's state and ring count
    void build_rings(const BodyState &params);

    // Recomputes the gravitational acceleration acting on every body
    void calculate_acceleration();

    // Advances the simulation one step forward in time
    // @param Integrator integrator - the integration scheme to use
    // @param const bool &override_time - if true, ignores the SIM_TIME upper bound
    void step(Integrator integrator, const bool &override_time);

    // Advances the simulation one step backward in time
    // @param Integrator integrator - the integration scheme to use
    // @param const bool &override_time - if true, allows stepping before step 0
    void step_backward(Integrator integrator, const bool &override_time);
};

Simulator::Simulator() {}

void Simulator::fill_console()
{
    // Prompt user in console for all the values
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
    // Get the lookup point for the key-value pair
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
    // Pull all the values straight from the file
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

    FreeJson(config); // release the parsed tree
}

// Lists CONFIG_DIR/*.json, prints a numbered menu to the terminal, and returns the
// chosen file's full path. Returns "" if the directory is missing/empty. I made it static so it needs no instance state and runs
// before the new Simulator exists.
inline std::string Simulator::choose_config_file()
{
    dynamic_array<std::string> files;
    std::error_code e;

    for (const auto &entry : std::filesystem::directory_iterator(CONFIG_DIR, e))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.add(entry.path().string());
    }

    if (e || files.length() == 0)
    {
        std::cout << "No configuration files found in '" << CONFIG_DIR << "'." << std::endl;
        return ""; // caller treats empty as "no selection"
    }

    // Selection sort for a stable, alphabetical menu.
    // operator[] returns a reference, so std::swap works on entries.
    for (int i = 0; i < files.length() - 1; i++)
    {
        int min_idx = i;
        for (int j = i + 1; j < files.length(); j++)
            if (files[j] < files[min_idx])
                min_idx = j;

        if (min_idx != i)
            std::swap(files[i], files[min_idx]);
    }

    // Get the user's selection
    std::cout << "\nAvailable configurations:\n";
    for (int i = 0; i < files.length(); i++)
        std::cout << "  " << (i + 1) << ") "
                  << std::filesystem::path(files[i]).filename().string() << "\n";

    int choice = read_integer_range("Select a configuration: ", 1, files.length());
    return files[choice - 1];
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
            inv_distance = 1.0 / std::sqrt(distance_sq);
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

// Opposite of leapfrog forward
inline void Simulator::leapfrog_reverse()
{
    for (int i = 0; i < n_bodies; i++)
    {
        bodies[i].data.velocity = bodies[i].data.velocity + bodies[i].data.acceleration * (-0.5 * TIME_STEP);
        bodies[i].data.position = bodies[i].data.position + bodies[i].data.velocity * (-TIME_STEP);
    }
    calculate_acceleration();
    for (int i = 0; i < n_bodies; i++)
    {
        bodies[i].data.velocity = bodies[i].data.velocity + bodies[i].data.acceleration * (-0.5 * TIME_STEP);
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

// Mirror of step() that calls leapfrog_reverse().
inline void Simulator::step_backward(Integrator integrator, const bool &override_time = false)
{
    if (current_step > 0 || override_time)
    {
        switch (integrator)
        {
        case LEAPFROG:
            leapfrog_reverse();
            break;
        default:
            break;
        }
        current_step -= 1;
    }
}

#endif