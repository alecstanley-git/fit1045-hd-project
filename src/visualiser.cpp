#include "visualiser.hpp"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int open_glfw_window()
{
    if (~glfwInit())
    {
        std::cout << "Failed to initialise glfw" << std::endl;
        return -1;
    }

    // Set OpenGL version for version 4.6
    glfwWindowHint(GLFW_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "N-Body Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to open window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    std::cout << "Window opened" << std::endl;

    return 0;
}