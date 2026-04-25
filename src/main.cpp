#include <iostream>
#include "simulator.hpp"
#include "console-input.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "visualiser.hpp"

/*
IMPLEMENTATION PLAN:

[ ] Create the galaxy class and allow it to be filled with the required parameters
[ ] Initialise all the stars automatically by filling rings with stars around each galaxy
[ ] Helper functions for acceleration, energy, and momentum
[ ] Design the numerical integrator
  - Need to make important decisions on how the integrator will store data at each step
[ ] Implement graphing logic with Raylib
  - Raylib is an extremely lightweight and great for 3D visuals
[ ] Move configuration from console to GUI using Dear Imgui library
  - Dear Imgui can build interactive windows and sit on top of Raylib with sliders/buttons.

** Creating my own OpenGL or Vulkan renderer is a path I am not willing to go down and is likely beyond the scope of the project requirements, this is where pre-existing libraries would be essential.
*/

// Should separate these out only to wherever they're used, but for now this is fine.
using namespace Constants;
using namespace Parameters;
using namespace std;

enum MenuOption
{
    INIT,
    PRINT,
    QUIT
};

MenuOption menu()
{
    cout << endl
         << "[-] Main Menu" << endl;
    cout << "1. Initialise" << endl;
    cout << "2. Print state" << endl;
    cout << "3. Quit" << endl;
    cout << endl;

    int option = read_integer_range("  > ", 1, 3);
    return (MenuOption)(option - 1);
}

int main()
{
    Simulator<num_galaxies> simulation;
    
    MenuOption option;

    do
    {
        option = menu();

        switch (option)
        {
        case INIT:
            simulation.fill_galaxies();
            break;
        case PRINT:
            for (int i = 0; i < (int)simulation.galaxies.size(); i++)
            {
                simulation.galaxies[i].print(i+1);
            }
            break;
        default:
            break;
        }
    } while (option != QUIT);

    return 0;
}