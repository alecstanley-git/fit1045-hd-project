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

    Window window(800, 600, "Simulator");

    window.load_font("Aboreto-Regular.ttf");

    const int BUTTON_WIDTH = 140;
    const int BUTTON_HEIGHT = 40;

    Button* initButton = window.add_button(100, 100, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    Button* printButton = window.add_button(100, 200, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    Button* runButton = window.add_button(100, 300, BUTTON_WIDTH, BUTTON_HEIGHT, "Run");
    Button* quitButton = window.add_button(100, 400, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");

    while (window.is_running())
    {
        window.process_events();
        window.clear_screen(Color::White);

        window.process_buttons();
        
        if (initButton->is_pressed())
        {
            simulation.fill_galaxies();
        }

        if (printButton->is_pressed())
        {
            for (int i = 0; i < (int)simulation.galaxies.size(); i++)
            {
                simulation.galaxies[i].print(i + 1);
            }
        }

        if (runButton->is_pressed())
        {
            simulation.integrate();
        }

        if (quitButton->is_pressed())
        {
            return EXIT_SUCCESS;
        }

        // ~16 milliseconds should hit roughly 60fps, and avoid maxing out the CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return EXIT_SUCCESS;
}
