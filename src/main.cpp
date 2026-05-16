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

[ ] Create the galaxy class and allow it to be filled with the required parameters
[ ] Initialise all the stars automatically by filling rings with stars around each galaxy
[ ] Helper functions for acceleration, energy, and momentum
[ ] Design the numerical integrator
  - Need to make important decisions on how the integrator will store data at each step
[ ] Develop a graphical library to display plots and 3D visualisations of the data.
[ ] Implement a dynamic time-step that changes based on how quickly the acceleration is changing, and also a damping parameter to ease extreme motion.

*/

// Should separate these out only to wherever they're used, but for now this is fine.
using namespace Constants;
using namespace Parameters;
using namespace std;

enum MenuOption
{
    INIT,
    PRINT,
    RUN,
    QUIT
};

MenuOption menu()
{
    cout << endl
         << "[-] Main Menu" << endl;
    cout << "1. Initialise" << endl;
    cout << "2. Print state" << endl;
    cout << "3. Run simulation" << endl;
    cout << "4. Quit" << endl;
    cout << endl;

    int option = read_integer_range("  > ", 1, 4);
    return (MenuOption)(option - 1);
}

int main()
{
    Simulator<num_galaxies> simulation;

    Window window(800, 600, "Simulator");


    Button* initButton = window.add_button(100, 100, 80, 30, "Initialise");
    Button* printButton = window.add_button(100, 200, 80, 30, "Print state");
    Button* runButton = window.add_button(100, 300, 80, 30, "Run");
    Button* quitButton = window.add_button(100, 400, 80, 30, "Quit");

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

    
    // MenuOption option;

    // do
    // {
    //     option = menu();

    //     switch (option)
    //     {
    //     case INIT:
    //         simulation.fill_galaxies();
    //         break;
    //     case PRINT:
    //         for (int i = 0; i < (int)simulation.galaxies.size(); i++)
    //         {
    //             simulation.galaxies[i].print(i + 1);
    //         }
    //         break;
    //     case RUN:
    //         simulation.integrate();
    //         break;
    //     default:
    //         break;
    //     }
    // } while (option != QUIT);

    return EXIT_SUCCESS;
}
