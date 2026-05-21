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

[x] Create the body class and allow it to be filled with the required parameters
[ ] Initialise all the stars automatically by filling rings with stars around each body
[x] Helper functions for acceleration, energy, and momentum
[x] Design a basic numerical integrator (leapfrog)
[/] Develop a graphical library to display plots and 3D visualisations of the data.
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
    PLOT2D
};

/*
These commands represent the 'instructions' the user can send to the application by pressing buttons.
Some UI buttons will execute code directly in the same menu, but instructions that need to refer to other menus can be passed as one of these instructions (i.e. menu navigation)
*/
enum MenuCommand
{
    NONE,
    GO_BACK,
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
    new_sim->fill();
    return new_sim;
}

/*
This procedure is run once on startup. It adds all the UI buttons necessary for the program to run and adds them to the window class.
Individual buttons can be called by referencing the buttons dynamic array, a field of the window class.
*/
void create_buttons(Window &window)
{
    int BUTTON_WIDTH = 200;
    int BUTTON_HEIGHT = 60;

    // Main menu buttons
    window.add_button(WINDOW_WIDTH / 2 - 250, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Start");
    window.add_button(WINDOW_WIDTH / 2 + 50, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");

    BUTTON_WIDTH = 140;
    BUTTON_HEIGHT = 40;
    // Program running buttons
    const double width = WINDOW_WIDTH / 10;
    const double height = WINDOW_HEIGHT / 6;
    window.add_button(width, height - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    window.add_button(width, 2 * height - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    window.add_button(width, 3 * height - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    window.add_button(width, 4 * height - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Run/Stop");
    window.add_button(width, 5 * height - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Return");
}

// ---- MENUS ----

/*
The main menu will simply render the title and the start/quit buttons
*/
MenuCommand main_menu(Window &window)
{
    window.draw_text("N-Body Collision", 0, WINDOW_HEIGHT / 4, 50, Black, WINDOW_WIDTH, 100);
    window.draw_text("Simulator", 0, WINDOW_HEIGHT / 4 + 50, 50, Black, WINDOW_WIDTH, 100);

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
The plot2d menu is a bit more complicated. It allows the user to actually initialise and run the simulation, showing a 2D x-y projection of the objects on the right.
*/
MenuCommand plot2d_menu(Window &window, Simulator *&sim)
{
    dynamic_array<int> indices;
    indices.add(2);
    indices.add(3);
    indices.add(4);
    indices.add(5);
    indices.add(6);
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

    if (sim && sim->state == ACTIVE)
    {
        sim->step(LEAPFROG);
    }

    // Once the simulation has been created, this code runs and plots the current position of every body, every frame.
    if (sim)
    {
        // Write the current step in the top-left corner
        window.draw_text(std::to_string((int)sim->current_step) + "/" + std::to_string((int)(SIM_TIME / TIME_STEP)), 10, 10, 12, Black, 100, 30);

        dynamic_array<double> x, y;
        for (int i = 0; i < sim->bodies.length(); i++)
        {
            x.add(sim->bodies[i].data.position.x);
            y.add(sim->bodies[i].data.position.y);
        }
        window.plot(x, y, 300, 100, 1, "x", "y", "Body Positions (x-y projection)", -1.5, 1.5, -1.5, 1.5);
    }

    return NONE;
}

/*
This is a thin helper procedure that calls the process_events() method and sleeps the frame for a set time to achieve a target FPS
*/
void update_window(Window &window, double fps)
{
    window.process_events();

    int ms = std::floor(1000.0 / fps);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    // Initialising the program
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
    Simulator *sim = nullptr; // Initialise an empty pointer
    create_buttons(window);

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
            command = plot2d_menu(window, sim);
            break;
        default:
            break;
        }

        switch (command)
        {
        case GO_BACK:
            menu = MAIN;
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

        update_window(window, FPS);
    };
    return EXIT_SUCCESS;
}
