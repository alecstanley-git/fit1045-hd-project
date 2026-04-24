#include <iostream>
#include <string>
#include "console-input.hpp"

using namespace std;

string read_string(string message)
{
    cout << message;
    string input;
    getline(cin, input);

    return input;
}

int read_integer(string message)
{
    string input;

    while (true)
    {
        cout << message;
        getline(cin, input);
        try
        {
            std::size_t pos = 0;
            int output = stoi(input, &pos);

            // Check if the whole string was consumed
            if (pos != input.size())
            {
                cerr << "Input must be an integer" << endl;
            }
            else
            {
                return output;
            }
        }
        catch (const exception &e)
        {
            cerr << "Conversion failed: " << e.what() << endl;
        }
    }
}

int read_integer_range(string message, int minimum, int maximum)
{
    int input;
    bool in_range = false;

    do
    {
        input = read_integer(message);
        if (input < minimum || input > maximum)
        {
            cerr << "Out of range - must be between " << minimum << " and " << maximum << endl;
        }
        else
        {
            in_range = true;
        }
    } while (!in_range);

    return input;
}

double read_double(string message)
{
    string input;

    while (true)
    {
        cout << message;
        getline(cin, input);
        try
        {
            std::size_t pos = 0;
            double output = stod(input, &pos);

            // Check if the whole string was consumed
            if (pos != input.size())
            {
                cerr << "Input must be decimal" << endl;
            }
            else
            {
                return output;
            }
        }
        catch (const exception &e)
        {
            cerr << "Conversion failed: " << e.what() << endl;
        }
    }
}

double read_double_range(string message, double minimum, double maximum, bool allow_zero)
{
    double input;
    bool in_range = false;

    do
    {
        input = read_double(message);
        if (input < minimum || input > maximum)
        {
            cerr << "Out of range - must be between " << minimum << " and " << maximum << endl;
        }
        else if (!allow_zero && input == 0)
        {
            cerr << "Zero is not allowed" << endl;
        }
        else
        {
            in_range = true;
        }
    } while (!in_range);

    return input;
}

bool read_boolean(string message)
{
    string input;

    while (true)
    {
        cout << message << " (Y/n): ";
        getline(cin, input);

        if (input == "n" or input == "N")
        {
            return false;
        }
        return true;
    }
}
