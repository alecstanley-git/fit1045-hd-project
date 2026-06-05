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
#include "parsejson.hpp"

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
[x] Allow bodies to be initialised by uploading a JSON configuration file, to help make configurations modular/repeatable (will help when recording H+1 video).

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
    PLOT3D,
    INIT
};

/*
These commands represent the 'instructions' the user can send to the application by pressing buttons.
Some UI buttons will execute code directly in the same menu, but instructions that need to refer to other menus can be passed as one of these instructions (i.e. menu navigation)
*/
enum MenuCommand
{
    NONE,
    RESTART,
    GO_2D,
    GO_3D,
    START_INIT,
    START,
    QUIT
};

/*
A helper function to call the fill() method and return the pointer to the simulation.
Handles memory using new and delete
*/
Simulator *initialise_simulation(Simulator *sim, const string &filepath = "")
{
    delete sim;
    sim = nullptr;

    Simulator *new_sim = new Simulator();

    if (filepath == "")
    {
        new_sim->fill_console(); // Request user input to initialise simulation
    }
    else
    {
        new_sim->fill_from_file(filepath);
    }

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
    BUTTON_WIDTH = 240;
    BUTTON_HEIGHT = 80;
    window.add_button(WINDOW_WIDTH / 2 - 280, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Start");
    window.add_button(WINDOW_WIDTH / 2 + 40, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit");

    // Plot menu buttons ------
    BUTTON_WIDTH = 180;
    BUTTON_HEIGHT = 60;
    const double x = WINDOW_WIDTH / 10;
    const double y = WINDOW_HEIGHT / 6;
    window.add_button(x, y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Initialise");
    window.add_button(x, 2 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Print state");
    window.add_button(x, 3 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Step");
    window.add_button(x, 4 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Run/Stop");
    window.add_button(x, 5 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Return");
    window.add_button(WINDOW_WIDTH - 60, 20, 40, 40, "3D");
    window.add_button(WINDOW_WIDTH - 60, 20, 40, 40, "2D");

    // Init menu buttons ------
    BUTTON_WIDTH = 260;
    BUTTON_HEIGHT = 80;
    window.add_button(WINDOW_WIDTH / 2 - 300, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Enter new");
    window.add_button(WINDOW_WIDTH / 2 + 40, WINDOW_HEIGHT / 2 + 50, BUTTON_WIDTH, BUTTON_HEIGHT, "Upload from file");
    window.add_button(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 200, 200, 60, "Cancel");
}

// ---- MENUS ----

/*
The main menu will simply render the title and the start/quit buttons
*/
MenuCommand main_menu(Window &window)
{
    window.draw_text("N-Body Collision", 0, WINDOW_HEIGHT / 4, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);
    window.draw_text("Simulator", 0, WINDOW_HEIGHT / 4 + 65, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);

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
    {
        return START_INIT;
    }
    if (window.buttons[3]->is_clicked())
        if (sim)
            sim->print_all();
    if (window.buttons[4]->is_clicked())
        if (sim)
            sim->step(LEAPFROG, true); // Allows the user to manually step even if the sim time is finished
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
        return RESTART;
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
        window.draw_text(std::to_string((int)sim->current_step) + "/" + std::to_string((int)(SIM_TIME / TIME_STEP)), 20, 20, BUTTON_TEXTSIZE, TITLE_COLOR, 200, 50);

        // Initialise arrays, ones for the central bodies and ones for the rings (different colours/sizes)
        dynamic_array<double> x_c, y_c, z_c;
        dynamic_array<double> x_r, y_r, z_r;

        for (int i = 0; i < sim->bodies.length(); i++)
        {
            if (sim->bodies[i].data.mass > 0.0)
            {
                x_c.add(sim->bodies[i].data.position.x);
                y_c.add(sim->bodies[i].data.position.y);
                z_c.add(sim->bodies[i].data.position.z);
            }
            else if (sim->bodies[i].data.mass == 0.0)
            {
                x_r.add(sim->bodies[i].data.position.x);
                y_r.add(sim->bodies[i].data.position.y);
                z_r.add(sim->bodies[i].data.position.z);
            }
        }

        Figure fig(window, Point2D{WINDOW_WIDTH / 3, 60}, Point2D{WINDOW_HEIGHT - 120, WINDOW_HEIGHT - 120});
        fig.set_xlim(-1.5, 1.5);
        fig.set_ylim(-1.5, 1.5);
        fig.set_xlabel("X");
        fig.set_ylabel("Y");

        if (menu == PLOT2D)
        {
            fig.set_title("X-Y Projection");

            fig.plot(x_c, y_c, SolarGold, 4);
            fig.plot(x_r, y_r, CosmicTeal, 2);
        }
        else if (menu == PLOT3D)
        {
            fig.set_zlim(-1.5, 1.5);
            fig.set_title("3D Projection");
            fig.set_zlabel("Z");

            fig.plot3d(camera, x_c, y_c, z_c, SolarGold, 4);
            fig.plot3d(camera, x_r, y_r, z_r, CosmicTeal, 2);
        }
        fig.show();
    }

    return NONE;
}

MenuCommand init_menu(Window &window, Simulator *&sim)
{
    window.draw_text("Choose method", 0, WINDOW_HEIGHT / 4, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);

    dynamic_array<int> indices;
    indices.add(9);
    indices.add(10);
    indices.add(11);
    window.process_buttons(indices);

    if (window.buttons[9]->is_clicked())
    {
        sim = initialise_simulation(sim);
        return GO_2D;
    }

    if (window.buttons[10]->is_clicked())
    {
        sim = initialise_simulation(sim, CONFIG_FILEPATH);
        return GO_2D;
    }

    if (window.buttons[11]->is_clicked())
    {
        return GO_2D;
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
        case INIT:
            command = init_menu(window, sim);
        default:
            break;
        }

        switch (command)
        {
        case RESTART:
            menu = MAIN;
            break;
        case GO_2D:
            menu = PLOT2D;
            break;
        case GO_3D:
            menu = PLOT3D;
            break;
        case START_INIT:
            menu = INIT;
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
