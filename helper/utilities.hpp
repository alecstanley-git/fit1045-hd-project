#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
using namespace std;

string read_string(string message);

int read_integer(string message);

int read_integer_range(string message, int minimum, int maximum);

double read_double(string message);

double read_double_range(string message, double minimum, double maximum, bool allow_zero = true);

bool read_boolean(string message);

#endif