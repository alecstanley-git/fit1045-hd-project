#include <iostream>
#include <chrono>
#include <thread>
#include "simulator.hpp"
#include "console-input.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "window.hpp"
#include "button.hpp"

/*
IMPLEMENTATION PLAN:

[x] Create the galaxy class and allow it to be filled with the required parameters
[ ] Initialise all the stars automatically by filling rings with stars around each galaxy
[x] Helper functions for acceleration, energy, and momentum
[x] Design the basic numerical integrator
[/] Develop a graphical library to display plots and 3D visualisations of the data.
[ ] Tell the makefile to copy the assets next to the executable (i.e fonts, images)
[ ] Develop a more complex hybrid integrator (look into mercurius)
  - Solver should use fast symplectic solver on a large scale (i.e. leapfrog/wisdom-holman) and switch to RK45 (or similar) for close encounters (non-symplectic)
  - Look into time regularisation (i.e. Mikkola's or Logarithmic Hamiltonian) using a new time variable ds, where dt = r*ds.
  - Best option might be the one used by REBOUND project - IAS15 and mercurius.
[ ] Develop a UnitSystem class for taking in and outputting realistic units - the solver will still only interact with normalised units
[ ] Clean up main.cpp (it's currently spaghetti code)

*/

/*
Interesting configurations:
1) n_bodies = 3
First: m=2, pos=(-0.5, 0, 0), vel=(0, 0.5, 0)
Second: m=1, pos=(1, 0, 0), vel=(0, -1, 0)
Third: m=0.5, pos=(0, 0, 0), vel=(0, 0, 0.2)

2) n_bodies = 2 (simple circular orbit)
First: m=1, pos=(1, 0, 0), vel=(0, 0.5, 0)
Second: m=1, pos=(-1, 0, 0), vel=(0, -0.5, 0)
*/

// Should separate these out only to wherever they're used, but for now this is fine.
using namespace Constants;
using namespace Parameters;
using namespace std;

enum MainMenu
{
    INITIALISE,
    PRINT,
    STEP,
    TOGGLE,
    QUIT,
    NO_CHOICE
};

Simulator *initialise_simulation(Simulator *sim)
{
    delete sim;
    Simulator *new_sim = new Simulator();
    new_sim->fill_galaxies();
    return new_sim;
}

void draw_window(Window &window, const Simulator *sim)
{
    window.clear_screen(BACKGROUND_COLOR);

    if (sim)
    {
        window.draw_text(std::to_string((int)sim->current_step) + "/" + std::to_string((int)(SIM_TIME / TIME_STEP)), 10, 10, 12, Black, 100, 30);

        dynamic_array<double> x, y;
        for (int i = 0; i < sim->galaxies.length(); i++)
        {
            x.add(sim->galaxies[i].data.position.x);
            y.add(sim->galaxies[i].data.position.y);
        }
        window.plot(x, y, 300, 100, 1, "x", "y", "Body Positions (x-y projection)", -1.5, 1.5, -1.5, 1.5);
    }
}

void create_buttons(Window &window)
{
    const int BUTTON_WIDTH = 140;
    const int BUTTON_HEIGHT = 40;

    window.add_button(100, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    window.add_button(100, 200, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    window.add_button(100, 300, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    window.add_button(100, 400, BUTTON_WIDTH, BUTTON_HEIGHT, "Run/Stop");
    window.add_button(100, 500, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");
    window.add_button(100, 200, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");
}

MainMenu main_menu(Window &window, const Simulator *sim)
{
    dynamic_array<int> *indices = new dynamic_array<int>;
    if (sim)
    {
        for (int i = 0; i < 5; i++)
        {
            indices->add(i);
        }
    }
    else
    {
        indices->add(0);
        indices->add(5);
    }
    window.process_buttons(indices);
    delete indices;

    if (window.buttons[0]->is_clicked())
        return INITIALISE;
    if (window.buttons[1]->is_clicked())
        return PRINT;
    if (window.buttons[2]->is_clicked())
        return STEP;
    if (window.buttons[3]->is_clicked())
        return TOGGLE;
    if (window.buttons[4]->is_clicked() || window.buttons[5]->is_clicked())
        return QUIT;

    return NO_CHOICE;
}

void update_window(Window &window, double fps)
{
    window.process_events();
    int ms = std::floor(1000.0 / fps);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
    create_buttons(window);

    Simulator *sim = nullptr; // Initialise an empty pointer

    while (window.is_running())
    {
        draw_window(window, sim);

        MainMenu option = main_menu(window, sim);

        switch (option)
        {
        case INITIALISE:
            sim = initialise_simulation(sim);
            break;
        case PRINT:
            if (sim)
                sim->print_all_galaxies();
            break;
        case STEP:
            if (sim)
                sim->step(LEAPFROG);
            break;
        case TOGGLE:
            if (sim && sim->state == INACTIVE)
                sim->state = ACTIVE;
            else if (sim)
                sim->state = INACTIVE;
            break;
        case QUIT:
            delete sim;
            return EXIT_SUCCESS;
        default:
            break;
        }

        if (sim && sim->state == ACTIVE)
        {
            sim->step(LEAPFROG);
        }

        update_window(window, 30);
    };
    delete sim;
    return EXIT_SUCCESS;
}
