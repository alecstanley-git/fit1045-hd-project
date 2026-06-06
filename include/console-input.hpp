#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

/*
This is the same kind of thing used throughout the semester, typically in a file called utilities.h/utilities.cpp.
I rewrite it without SplashKit.
It is specifically for handling CONSOLE input
*/

// Read in a string from user
// @param std::string message - the user prompt
// @return std::string - the user input
std::string read_string(std::string message);

// Read in an integer from user
// @param std::string message - the user prompt
// @return int - the user input
int read_integer(std::string message);

// Read in a bounded integer from user
// @param std::string message - the user prompt
// @param int minimum - min value
// @param int maximum - max value
// @return int - the user input
int read_integer_range(std::string message, int minimum, int maximum);

// Read in a double from user
// @param std::string message - the user prompt
// @return double - the user input
double read_double(std::string message);

// Read in a bounded double from user
// @param std::string message - the user prompt
// @param double minimum - min value
// @param double maximum - max value
// @return double - the user input
double read_double_range(std::string message, double minimum, double maximum, bool allow_zero = true);

// Read in a true/false value from user (prompted as y/n)
// @param std::string message - the user prompt
// @return bool - the user input (0/1)
bool read_boolean(std::string message);

#endif