#ifndef UNITSYSTEM_H
#define UNITSYSTEM_H

#include <cmath>

enum Scale
{
    SOLAR_SYSTEM,
    INTERGALACTIC
};

class UnitSystem
{
private:
    double mass_unit_g;
    double length_unit_cm;
    double time_unit_s;

    // First define common CGS units of measurement
    const double G_cgs = 6.674e-8;  // cm^3 g^-1 s^-2
    const double Msun_g = 1.989e33; // g
    const double AU_cm = 1.496e13;  // cm
    const double MPC_cm = 3.086e24; // cm

    const double SECONDS_IN_YEAR = 31557600.0; // equal to 365.25 days

public:
    UnitSystem(Scale scale)
    {
        if (scale == SOLAR_SYSTEM)
        {
            mass_unit_g = Msun_g;
            length_unit_cm = AU_cm;
        }
        if (scale == INTERGALACTIC)
        {
            mass_unit_g = 1e12 * Msun_g; // 1 trillion solar masses
            length_unit_cm = MPC_cm;
        }

        // Derive time unit using the normalisation that G in the simulation is 1
        time_unit_s = std::sqrt(std::pow(length_unit_cm, 3) / (G_cgs * mass_unit_g));
    }

    // Core conversions from CGS
    double mass_to_g(const double &sim_mass) const
    {
        return sim_mass * mass_unit_g;
    }

    double length_to_cm(const double &sim_length) const
    {
        return sim_length * length_unit_cm;
    }

    double time_to_s(const double &sim_time) const
    {
        return sim_time * time_unit_s;
    }

    double velocity_to_cms(const double &sim_vel) const
    {
        return sim_vel * (length_unit_cm / time_unit_s);
    }

    // Readable conversions
    double mass_to_solar(const double &sim_mass) const
    {
        return mass_to_g(sim_mass) / Msun_g;
    }

    double length_to_au(const double &sim_length) const
    {
        return length_to_cm(sim_length) / AU_cm;
    }

    double length_to_mpc(const double &sim_length) const
    {
        return length_to_cm(sim_length) / MPC_cm;
    }

    double time_to_years(const double &sim_time) const
    {
        return time_to_s(sim_time) / SECONDS_IN_YEAR;
    }

    double velocity_to_km_per_s(const double &sim_vel) const
    {
        return velocity_to_cms(sim_vel) / 1e5;
    }
};

#endif