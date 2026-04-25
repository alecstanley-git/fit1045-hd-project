#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

std::string read_string(std::string message);

int read_integer(std::string message);

int read_integer_range(std::string message, int minimum, int maximum);

double read_double(std::string message);

double read_double_range(std::string message, double minimum, double maximum, bool allow_zero = true);

bool read_boolean(std::string message);

#endif