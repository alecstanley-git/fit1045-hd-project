#ifdef _WIN32

#include "window.hpp"
#include <windows.h>
#include <tchar.h>

/*
The code in this file is mostly pulled directly from Microsoft's official Windows documentation
*/

int open_window()
{
    return 0;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    // This instance of the WNDCLASSEX structure contains the window parameters such as icon, colour, title etc.
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    // Register the window with the operating system
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
                   _T("Call to RegisterClassEx failed!"),
                   _T("Windows Desktop Guided Tour"),
                   NULL);

        return 1;
    }

    // Create the window
    static TCHAR szWindowClass[] = _T("DesktopApp");
    static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, height)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 100,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (!hWnd)
    {
        MessageBox(NULL,
                   _T("Call to CreateWindowEx failed!"),
                   _T("Windows Desktop Guided Tour"),
                   NULL);

        return 1;
    }

    // Show the window
    ShowWindow(hWnd,
               nCmdShow);
    UpdateWindow(hWnd);

    // Message loop to handle events passed in from the operating system
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;

    return (int) msg.wParam;
}

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return LRESULT();
}

#endif