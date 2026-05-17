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
[x] Design the numerical integrator
  - Need to make important decisions on how the integrator will store data at each step
[ ] Develop a graphical library to display plots and 3D visualisations of the data.
[ ] Implement a dynamic time-step that changes based on how quickly the acceleration is changing, and also a damping parameter to ease extreme motion.
[ ] Tell the makefile to copy the assets next to the executable

*/

// Should separate these out only to wherever they're used, but for now this is fine.
using namespace Constants;
using namespace Parameters;
using namespace std;

int main()
{
    Simulator<num_galaxies> simulation;
    bool sim_running = false;

    Window window(800, 600, "Simulator");

    window.load_font("Aboreto-Regular.ttf");

    const int BUTTON_WIDTH = 140;
    const int BUTTON_HEIGHT = 40;

    Button* initButton = window.add_button(100, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    Button* printButton = window.add_button(100, 200, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    Button* stepButton = window.add_button(100, 300, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    Button* runButton = window.add_button(100, 400, BUTTON_WIDTH, BUTTON_HEIGHT, "Run all");
    Button* quitButton = window.add_button(100, 500, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");

    while (window.is_running())
        {
        dynamic_array<double> *x = new dynamic_array<double>;
        dynamic_array<double> *y = new dynamic_array<double>;
        window.process_events();
        window.clear_screen(Color::White);

        window.process_buttons();
        
        if (initButton->is_clicked())
        {
            simulation.fill_galaxies();
        }

        if (printButton->is_clicked())
        {
            simulation.print_all_galaxies();
        }

        if (stepButton->is_clicked())
        {
            simulation.leapfrog();
            simulation.print_all_galaxies();
            for (int i = 0; i < num_galaxies; i++)
            {
                x->add(simulation.galaxies[i].data.position.x);
                y->add(simulation.galaxies[i].data.position.y);
            }
        }

        if (runButton->is_clicked())
        {
            if (!sim_running)
            {
                sim_running = true;
            }
            else
            {
                sim_running = false;
            }
        }

        if (quitButton->state == CLICKED)
        {
            return EXIT_SUCCESS;
        }

        if (sim_running)
        {
            if (simulation.step < (int)(sim_time/time_step))
            {
                simulation.leapfrog();
                simulation.print_all_galaxies();
                for (int i = 0; i < num_galaxies; i++)
                {
                    x->add(simulation.galaxies[i].data.position.x);
                    y->add(simulation.galaxies[i].data.position.y);
                }
            }
        }

        window.draw_text(std::to_string((int)simulation.step) + "/" + std::to_string((int)(sim_time/time_step)), 10, 10, 12, Black, 100, 30);
        window.plot(*x, *y, 300, 100, 1, "x", "y", "X-Y Simulation Projection", -1.5, 1.5, -1.5, 1.5);

        delete x;
        delete y;
        // 60 fps ~ 16 ms
        // 20 fps ~ 50 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return EXIT_SUCCESS;
}
