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
#include "unitsystem.hpp"

/*
IMPLEMENTATION PLAN:

[x] Create the body class and allow it to be filled with the required parameters
[x] Initialise all the stars automatically by filling rings with stars around each body
[x] Helper functions for acceleration, energy, and momentum
[x] Design a basic numerical integrator (leapfrog)
[x] Develop a graphical library to display plots and 3D visualisations of the data.
  - Want to adjust dot size based on distance from camera
  - Display 3D grid lines
  - Possible extension: fully fledged plotting library
[/] Develop a more complex hybrid integrator (look into mercurius)
  - Solver should use fast symplectic solver on a large scale (i.e. leapfrog/wisdom-holman) and switch to RK45 (or similar) for close encounters (non-symplectic)
  - Look into time regularisation (i.e. Mikkola's or Logarithmic Hamiltonian) using a new time variable ds, where dt = r*ds.
  - Best option might be the one used by REBOUND project - IAS15 and mercurius.
[x] Develop a UnitSystem class for taking in and outputting realistic units - the solver will still only interact with normalised units
[x] Clean up main.cpp
[x] Allow bodies to be initialised by uploading a JSON configuration file, to help make configurations modular/repeatable (will help when recording H+1 video).

Future areas:
- Tracking kinetic/potential energy and angular momentum to demonstrate conservation laws. This is not too hard to implement but I didn't go down this route since it doesn't demonstrate much more coding ability for the unit (calculating a value each frame and plotting it isn't much more interesting).
- The leapfrog integrator is very good at resolving Newtonian mechanics, especially with the softening factor I included. The time-to-benefit ratio was poor in terms of implementing a more advanced hybrid integrator. Such an endeavour would be more mathematics than physics focused, not ideal for a coding HD project, and would give me marginally better true-to-life results.
- Another feature I'd like to add as an extension is the ability to dynamically add bodies to the simulation while it is running.
*/

/*
AI disclosure: most of the JSON configurations used in demonstrations were AI generated. I asked it to write some files (in the format I designed) that demonstrate some interesting configurations
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
A helper function to call the simulation fill methods and return the pointer to the simulation.
Handles memory using new and delete
@param Simulator *&sim - the pointer to the simulation object, passed by reference
@param const string &filepath - the file path to pass to the fill method, optional if filling from file
@return Simulator* - pointer to the new simulator object
*/
Simulator *initialise_simulation(Simulator *&sim, const string &filepath = "")
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
@param Window &window - the window object passed by reference
*/
void load_buttons(Window &window)
{
    // Helpful variables for aligning lots of buttons to the same style across menus
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
    window.add_button(x, 3 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH / 2 - 10, BUTTON_HEIGHT, "Step -");
    window.add_button(x + BUTTON_WIDTH / 2 + 10, 3 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH / 2 - 10, BUTTON_HEIGHT, "Step +");
    window.add_button(x, 4 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Run/Stop");
    window.add_button(x, 5 * y - BUTTON_HEIGHT / 2, BUTTON_WIDTH, BUTTON_HEIGHT, "Return");
    window.add_button(WINDOW_WIDTH - 70, 20, 50, 50, "3D");
    window.add_button(WINDOW_WIDTH - 70, 20, 50, 50, "2D");
    window.add_button(WINDOW_WIDTH - 70, 90, 50, 50, "+");
    window.add_button(WINDOW_WIDTH - 70, 160, 50, 50, "-");
    window.add_button(WINDOW_WIDTH - 70, WINDOW_HEIGHT - 70, 50, 50, "S");

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
@param Window &window - the window object passed by reference
@return MenuCommand - an enum type representing the command to execute
*/
MenuCommand main_menu(Window &window)
{
    // Draw main title
    window.draw_text("N-Body Collision", 0, WINDOW_HEIGHT / 4, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);
    window.draw_text("Simulator", 0, WINDOW_HEIGHT / 4 + 65, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);

    // We can just pick exactly what buttons to call by passing in array indices
    // In this case just the main menu buttons
    dynamic_array<int> indices;
    indices.add(0);
    indices.add(1);
    window.process_buttons(indices);

    // Can execute actions for when buttons are clicked
    if (window.buttons[0]->is_clicked())
        return START;
    if (window.buttons[1]->is_clicked())
        return QUIT;

    return NONE;
}

/*
The plot menu is a bit more complicated. It allows the user to actually initialise and run the simulation, showing a 2D or 3D projection of the motion.
@param Window &window - the window object passed by reference
@param Simulator *&sim - the pointer to the simulation object, passed by reference
@param Menu &menu - this screen has two subtypes that share features, one for 2d and one for 3d, so pass that in as well
@param Camera &camera - the camera object (for which there is only one initialised on startup) passed by reference
@return MenuCommand - an enum type representing the command to execute
*/
MenuCommand plot_menu(Window &window, Simulator *&sim, Menu &menu, Camera &camera)
{
    // First, add all our buttons
    dynamic_array<int> indices;
    indices.add(2);
    indices.add(3);
    indices.add(4);
    indices.add(5);
    indices.add(6);
    indices.add(7);
    window.process_buttons(indices);

    // A bunch of if statements for different button actions
    if (window.buttons[2]->is_clicked())
    {
        return START_INIT; // Switch to the initialisation menu
    }
    if (window.buttons[3]->is_clicked())
        if (sim)
            sim->print_all();
    if (window.buttons[4]->is_clicked())
        if (sim)
            sim->step_backward(LEAPFROG, true);
    if (window.buttons[5]->is_clicked())
        if (sim)
            sim->step(LEAPFROG, true);
    
    // Toggle the active state of the simulation (play/pause)
    if (window.buttons[6]->is_clicked())
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
    if (window.buttons[7]->is_clicked())
    {
        // If the user returns to the main menu it should reset the simulation
        delete sim;
        sim = nullptr;
        return RESTART;
    }

    // If the simulation has been initialised and is active, we want to step it every frame
    // We only call this inside this menu so if the user is in another menu, it pauses
    if (sim && sim->state == ACTIVE)
    {
        sim->step(LEAPFROG);
    }

    // Once the simulation has been created, this code runs and plots the current position of every body, every frame.
    if (sim)
    {
        // Extra buttons appear only once sim is generated
        dynamic_array<int> indices;

        // Add the buttons to switch 2d/3d modes and zoom
        if (menu == PLOT2D)
        {
            indices.add(8);  // The 3d button
            indices.add(10); // Zoom out
            indices.add(11); // Zoom in
        }
        else if (menu == PLOT3D)
        {
            indices.add(9); // The 2d button
        }
        indices.add(12); // The timescale switcher button
        window.process_buttons(indices);

        // The 2d/3d button should just toggle the mode
        if (window.buttons[8]->is_clicked())
        {
            return GO_3D;
        }
        if (window.buttons[9]->is_clicked())
        {
            return GO_2D;
        }

        // The zoom buttons handle scaling the axes and are only shown in 2D mode
        if (window.buttons[10]->is_clicked())
        {
            AXIS_2D_LOWER *= 0.9;
            AXIS_2D_UPPER *= 0.9;
        }
        if (window.buttons[11]->is_clicked())
        {
            // Prefer /= 0.9 instead of something like *= 1.1 to ensure the process is reversible
            AXIS_2D_LOWER /= 0.9;
            AXIS_2D_UPPER /= 0.9;
        }

        // Timescale switcher
        if (window.buttons[12]->is_clicked())
        {
            if (SCALE == INTERGALACTIC)
            {
                SCALE = SOLAR_SYSTEM;
            }
            else if (SCALE == SOLAR_SYSTEM)
            {
                SCALE = INTERGALACTIC;
            }
        }

        // Initialise the unit system (for display only, remember sim only works in non-dimensionalised units)
        UnitSystem u(SCALE);

        // Write the current step in the top-left corner
        window.draw_text(std::to_string((int)sim->current_step) + "/" + std::to_string((int)(SIM_TIME / TIME_STEP)), 40, 10, BUTTON_TEXTSIZE, TITLE_COLOR, 200, 50);

        // Get real time and display just below step count
        double current_time = u.time_to_years(sim->current_step * TIME_STEP);
        window.draw_text(std::format("{:.2e}", current_time) + " years", 40, 35, BUTTON_TEXTSIZE, TITLE_COLOR, 200, 50);

        // Initialise arrays, ones for the central bodies and ones for the rings (different colours/sizes)
        dynamic_array<double> x_c, y_c, z_c;
        dynamic_array<double> x_r, y_r, z_r;

        // Get all the body positions and add to the arrays
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

        // Setup a new figure at the desired position
        Figure fig(window, Point2D{WINDOW_WIDTH / 3, 60}, Point2D{WINDOW_HEIGHT - 120, WINDOW_HEIGHT - 120});
        fig.set_xlabel("X");
        fig.set_ylabel("Y");

        if (menu == PLOT2D)
        {
            // 2D plot features
            fig.set_title("X-Y Projection (top down)");

            fig.set_xlim(AXIS_2D_LOWER, AXIS_2D_UPPER);
            fig.set_ylim(AXIS_2D_LOWER, AXIS_2D_UPPER);

            fig.plot(x_c, y_c, SolarGold, 4);
            fig.plot(x_r, y_r, CosmicTeal, 2);
        }
        else if (menu == PLOT3D)
        {
            // Clamp the accumulated scroll so zoom never reaches the hard limit in camera class
            if (window.zoom_level < MIN_ZOOM_LEVEL)
                window.zoom_level = MIN_ZOOM_LEVEL;
            else if (window.zoom_level > MAX_ZOOM_LEVEL)
                window.zoom_level = MAX_ZOOM_LEVEL;

            // Allow the user's zoom to apply each frame (from scrolling)
            camera.zoom((double)(window.zoom_level / 100.0f));

            // Also allow clicking and dragging to move perspective
            camera.drag(window.mouse_velocity());

            // 3D plot features
            fig.set_xlim(AXIS_3D_LOWER, AXIS_3D_UPPER);
            fig.set_ylim(AXIS_3D_LOWER, AXIS_3D_UPPER);
            fig.set_zlim(AXIS_3D_LOWER, AXIS_3D_UPPER);
            fig.set_title("3D Interactive");
            fig.set_zlabel("Z");

            fig.plot3d(camera, x_c, y_c, z_c, SolarGold, 4);
            fig.plot3d(camera, x_r, y_r, z_r, CosmicTeal, 2);
        }
        fig.show(); // Necessary to draw all objects from the buffer
    }

    return NONE;
}

/*
The init menu simply asks the user whether they want to make a custom configuration or load from a file
@param Window &window - the window object passed by reference
@param Simulator *&sim - the pointer to the simulation object, passed by reference
@return MenuCommand - an enum type representing the command to execute
*/
MenuCommand init_menu(Window &window, Simulator *&sim)
{
    // Title text
    window.draw_text("Pick an option", 0, WINDOW_HEIGHT / 4, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);
    window.draw_text("(Prompts in console)", 0, WINDOW_HEIGHT / 4 + 65, TITLE_TEXTSIZE, TITLE_COLOR, WINDOW_WIDTH, 100);

    // Add necessary buttons
    dynamic_array<int> indices;
    indices.add(13);
    indices.add(14);
    indices.add(15);
    window.process_buttons(indices);

    if (window.buttons[13]->is_clicked()) // New configuration option
    {
        sim = initialise_simulation(sim);
        return GO_2D;
    }

    if (window.buttons[14]->is_clicked())
    {
        // Prompt the user to pick a file using the console

        string path = Simulator::choose_config_file();
        if (!path.empty())
        {
            sim = initialise_simulation(sim, path);
            return GO_2D;
        }
        return NONE;
    }

    // Cancel button
    if (window.buttons[15]->is_clicked())
    {
        return GO_2D;
    }

    return NONE;
}

int main()
{
    // Initialising the window
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);

    // Camera is initially normalised to be 2 units from origin, regardless of initial position
    // Camera is always focused on origin (cannot be spun)
    Camera camera(CAM_POSITION.normal() * 2, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, CAM_FOV, CAM_ASPECT, CAM_ZNEAR, CAM_ZFAR);

    Simulator *sim = nullptr; // Initialise an empty pointer

    load_buttons(window);

    // Set initial variable states
    Menu menu = MAIN;
    MenuCommand command = NONE;

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
