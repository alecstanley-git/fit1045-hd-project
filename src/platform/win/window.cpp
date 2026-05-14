#ifdef _WIN32

#ifndef UNICODE
#define UNICODE
#endif

#include "window.hpp"
#include <windows.h>
#include <tchar.h>

/*
The code in this file is mostly pulled directly from Microsoft's official Windows documentation
*/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Fill with default background colour
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
Helper function to convert std::string type to a wide string literal used by the Win32 API
*/
std::wstring ToWideString(const std::string &narrow)
{
    if (narrow.empty())
        return L"";

    // Get required size
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), NULL, 0);

    // Initialise the new wide string
    std::wstring wide(size_needed, 0);

    // Perform conversion
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), &wide[0], size_needed);

    return wide;
}

Window::Window(int _width, int _height, std::string _title) : width(_width), height(_height), title(_title)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    // Register the window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                            // Optional window styles
        CLASS_NAME,                   // The class
        ToWideString(title).c_str(),  // Title
        WS_OVERLAPPEDWINDOW,          // Style (title bar, border, etc.)
        CW_USEDEFAULT, CW_USEDEFAULT, // Position x, y
        width, height,                // Size - width, height
        NULL,                         // Parent window
        NULL,                         // Menu
        hInstance,                    // Instance handle
        NULL                          // Additional data
    );

    // Make visible
    ShowWindow(hwnd, SW_SHOW);

    // Main loop - keeps it open and responsive
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    is_open = true;
}

void Window::clear_screen()
{
    std::cout << "TODO" << std::endl;
}

void Window::process_events()
{
    std::cout << "TODO" << std::endl;
}

#endif