#include <iostream>
#include "simulator.hpp"
#include "console-input.hpp"
#include "constants.hpp"
#include "parameters.hpp"
#include "window.hpp"
#include "figure.hpp"
#include "button.hpp"
#include "camera.hpp"
#include "data-structures.hpp"

/*
IMPLEMENTATION PLAN:

[x] Create the body class and allow it to be filled with the required parameters
[x] Initialise all the stars automatically by filling rings with stars around each body
[x] Helper functions for acceleration, energy, and momentum
[x] Design a basic numerical integrator (leapfrog)
[/] Develop a graphical library to display plots and 3D visualisations of the data.
  - Want to adjust dot size based on distance from camera
  - Display 3D grid lines
  - Possible extension: fully fledged plotting library
[ ] Tell the makefile to copy the assets next to the executable (i.e fonts, images)
[ ] Develop a more complex hybrid integrator (look into mercurius)
  - Solver should use fast symplectic solver on a large scale (i.e. leapfrog/wisdom-holman) and switch to RK45 (or similar) for close encounters (non-symplectic)
  - Look into time regularisation (i.e. Mikkola's or Logarithmic Hamiltonian) using a new time variable ds, where dt = r*ds.
  - Best option might be the one used by REBOUND project - IAS15 and mercurius.
[ ] Develop a UnitSystem class for taking in and outputting realistic units - the solver will still only interact with normalised units
[x] Clean up main.cpp
[ ] Allow bodies to be initialised by uploading a JSON configuration file, to help make configurations modular/repeatable (will help when recording H+1 video).

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

using namespace Constants;
using namespace Parameters;
using namespace std; // Only using std namespace in main file

/*
This enum defines what 'menu' the user is currently looking at. They start on the main menu. This enum is called whenever the program draws the screen.
*/
enum Menu
{
    MAIN,
    PLOT2D,
    PLOT3D
};

/*
These commands represent the 'instructions' the user can send to the application by pressing buttons.
Some UI buttons will execute code directly in the same menu, but instructions that need to refer to other menus can be passed as one of these instructions (i.e. menu navigation)
*/
enum MenuCommand
{
    NONE,
    GO_BACK,
    GO_2D,
    GO_3D,
    START,
    QUIT
};

/*
A helper function to call the fill() method and return the pointer to the simulation.
Handles memory using new and delete
*/
Simulator *initialise_simulation(Simulator *sim)
{
    delete sim;
    sim = nullptr;

    Simulator *new_sim = new Simulator();
    new_sim->fill(); // Request user input to initialise simulation

    return new_sim;
}

/*
This procedure is run once on startup. It adds all the UI buttons necessary for the program to run and adds them to the window class.
Individual buttons can be called by referencing the buttons dynamic array, a field of the window class.
*/
void load_buttons(Window &window)
{
    int BUTTON_WIDTH;
    int BUTTON_HEIGHT;

    // Main menu buttons ------
    BUTTON_WIDTH = 200;
    BUTTON_HEIGHT = 60;
    window.add_button(WINDOW_WIDTH / 2 - 250, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Start");
    window.add_button(WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");

    // Plot menu buttons ------
    BUTTON_WIDTH = 140;
    BUTTON_HEIGHT = 40;
    const double x = WINDOW_WIDTH / 10;
    const double y = WINDOW_HEIGHT / 6;
    window.add_button(x, y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    window.add_button(x, 2 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    window.add_button(x, 3 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    window.add_button(x, 4 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Run/Stop");
    window.add_button(x, 5 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Return");
    window.add_button(WINDOW_WIDTH - 60, 20, 40, 40, "3D");
    window.add_button(WINDOW_WIDTH - 60, 20, 40, 40, "2D");
}

// ---- MENUS ----

/*
The main menu will simply render the title and the start/quit buttons
*/
MenuCommand main_menu(Window &window)
{
    window.draw_text("N-Body Collision", 0, WINDOW_HEIGHT / 4, 50, Black, WINDOW_WIDTH, 100);
    window.draw_text("Simulator", 0, WINDOW_HEIGHT / 4 + 50, 50, Black, WINDOW_WIDTH, 100);

    // We can just pick exactly what buttons to call by passing in array indices
    dynamic_array<int> indices;
    indices.add(0);
    indices.add(1);
    window.process_buttons(indices);

    if (window.buttons[0]->is_clicked())
        return START;
    if (window.buttons[1]->is_clicked())
        return QUIT;

    return NONE;
}

/*
The plot menu is a bit more complicated. It allows the user to actually initialise and run the simulation, showing a 2D or 3D projection of the motion.
*/
MenuCommand plot_menu(Window &window, Simulator *&sim, Menu &menu, Camera &camera)
{
    dynamic_array<int> indices;
    indices.add(2);
    indices.add(3);
    indices.add(4);
    indices.add(5);
    indices.add(6);
    if (menu == PLOT2D)
    {
        indices.add(7); // The 3d button
    }
    else if (menu == PLOT3D)
    {
        indices.add(8); // The 2d button
    }
    window.process_buttons(indices);

    if (window.buttons[2]->is_clicked())
        sim = initialise_simulation(sim);
    if (window.buttons[3]->is_clicked())
        if (sim)
            sim->print_all();
    if (window.buttons[4]->is_clicked())
        if (sim)
            sim->step(LEAPFROG);
    if (window.buttons[5]->is_clicked())
    {
        if (sim && sim->state == INACTIVE)
        {
            sim->state = ACTIVE;
        }
        else if (sim)
        {
            sim->state = INACTIVE;
        }
    }
    if (window.buttons[6]->is_clicked())
    {
        delete sim;
        sim = nullptr;
        return GO_BACK;
    }
    if (window.buttons[7]->is_clicked())
    {
        return GO_3D;
    }
    if (window.buttons[8]->is_clicked())
    {
        return GO_2D;
    }

    if (sim && sim->state == ACTIVE)
    {
        sim->step(LEAPFROG);
    }

    // Once the simulation has been created, this code runs and plots the current position of every body, every frame.
    if (sim)
    {
        // Write the current step in the top-left corner
        window.draw_text(std::to_string((int)sim->current_step) + "/" + std::to_string((int)(SIM_TIME / TIME_STEP)), 10, 10, 12, Black, 100, 30);

        dynamic_array<double> x, y, z;
        for (int i = 0; i < sim->bodies.length(); i++)
        {
            x.add(sim->bodies[i].data.position.x);
            y.add(sim->bodies[i].data.position.y);
            z.add(sim->bodies[i].data.position.z);
        }

        if (menu == PLOT2D)
        {
            window.plot(x, y, 300, 100, 1, "x", "y", "Body Positions (x-y projection)", -1.5, 1.5, -1.5, 1.5);
        }
        else if (menu == PLOT3D)
        {
            window.plot3d(camera, x, y, z);
        }
    }

    return NONE;
}

int main()
{
    // Initialising the program
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
    Camera camera(CAM_POSITION, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, CAM_FOV, CAM_ASPECT, CAM_ZNEAR, CAM_ZFAR);
    Simulator *sim = nullptr; // Initialise an empty pointer

    load_buttons(window);

    // Set initial variable states and define empty variables
    Menu menu = MAIN;
    MenuCommand command;

    // Main program loop
    while (window.is_running())
    {
        window.clear_screen(BACKGROUND_COLOR);

        switch (menu)
        {
        case MAIN:
            command = main_menu(window);
            break;
        case PLOT2D:
        case PLOT3D:
            command = plot_menu(window, sim, menu, camera);
            break;
        default:
            break;
        }

        switch (command)
        {
        case GO_BACK:
            menu = MAIN;
            break;
        case GO_2D:
            menu = PLOT2D;
            break;
        case GO_3D:
            menu = PLOT3D;
            break;
        case START:
            menu = PLOT2D;
            break;
        case QUIT:
            return EXIT_SUCCESS;
        default:
            break;
        }
        command = NONE;

        window.update_window(FPS);
    };
    return EXIT_SUCCESS;
}
