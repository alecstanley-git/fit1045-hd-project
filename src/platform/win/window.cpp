#ifdef _WIN32

#ifndef UNICODE
#define UNICODE
#endif

#include "window.hpp"
#include "parameters.hpp"
#include <windows.h>
#include <tchar.h>
#include <iostream>

// Include GDI+ for high quality antialiasing
#include <gdiplus.h>

using namespace Parameters;

/*
Windows window management with Win32 is a bit more verbose. To achieve the same visual effect as I can with AppKit, I opted to work with GDI+, a Microsoft-recommended addon to the default GDI which allows for easy anti-aliasing and better object handling for objects I want to render.
*/

// GDI+ global initialisations
ULONG_PTR gdiplusToken;
static Gdiplus::PrivateFontCollection* g_fontCollection = nullptr;
static Gdiplus::FontFamily* g_customFontFamily = nullptr;

/*
Struct to hold win-specific rendering context.
We want to try and mimic AppKit's very clean layering system. We can do this using double buffering (which is essentially what AppKit does behind the scenes) by rendering all our objects to a hidden device context, and then pushing it all to the screen in one go.
*/
struct Win32Context
{
    HWND hwnd;
    HDC memDC;
    HBITMAP backBuffer;
    HBITMAP oldBitmap;
};

/*
The window creation and event loop is mostly pulled directly from Microsoft's official Windows documentation
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        if (g_customFontFamily) delete g_customFontFamily;
        if (g_fontCollection) delete g_fontCollection;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
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

/*
Converting the Color enum values (uint64_t) to Windows' COLORREF.
*/
static COLORREF ConvertColor(std::uint64_t hexColor)
{
    BYTE r = (hexColor >> 24) & 0xFF;
    BYTE g = (hexColor >> 16) & 0xFF;
    BYTE b = (hexColor >> 8) & 0xFF;
    return RGB(r, g, b);
}

Window::Window(int _width, int _height, std::string _title) : width(_width), height(_height), title(_title)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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
        0,                                                        // Optional window styles
        CLASS_NAME,                                               // The class
        ToWideString(title).c_str(),                              // Title
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // Style (title bar, border, etc.)
        CW_USEDEFAULT, CW_USEDEFAULT,                             // Position x, y
        width, height,                                            // Size - width, height
        NULL,                                                     // Parent window
        NULL,                                                     // Menu
        hInstance,                                                // Instance handle
        NULL                                                      // Additional data
    );

    // Force the window to match the exact requested dimensions
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

    // Make visible
    ShowWindow(hwnd, SW_SHOW);

    // Initialise double buffering (mimicking AppKit layers)
    Win32Context *ctx = new Win32Context();
    ctx->hwnd = hwnd;
    HDC hdc = GetDC(hwnd);
    ctx->memDC = CreateCompatibleDC(hdc);
    ctx->backBuffer = CreateCompatibleBitmap(hdc, width, height);
    ctx->oldBitmap = (HBITMAP)SelectObject(ctx->memDC, ctx->backBuffer);
    ReleaseDC(hwnd, hdc);

    _window = (void *)ctx;
    is_open = true;

    load_font(GLOBAL_FONT + ".ttf");
}

void Window::setup_mouse_listeners()
{
    // Don't need this. Windows mouse tracking is more lightweight/efficient so can run inside process_events()
}

bool Window::load_font(const std::string &file_path)
{
    // Initialize the collection if it hasn't been created yet
    if (!g_fontCollection) {
        g_fontCollection = new Gdiplus::PrivateFontCollection();
    }
    
    std::wstring w_path = ToWideString(file_path);
    
    // Load the .ttf file directly into the GDI+ collection
    if (g_fontCollection->AddFontFile(w_path.c_str()) != Gdiplus::Ok)
    {
        std::cerr << "Failed to load font: " << file_path << std::endl;
        return false;
    }

    // Cache the Font Family object so drawing text is extremely fast
    std::wstring wfont = ToWideString(GLOBAL_FONT);
    g_customFontFamily = new Gdiplus::FontFamily(wfont.c_str(), g_fontCollection);
    
    return true;
}

void Window::process_events()
{
    Win32Context *ctx = static_cast<Win32Context *>(_window);

    // Emulate NSApp updatewindows by pushing back buffer to screen
    HDC hdc = GetDC(ctx->hwnd);
    BitBlt(hdc, 0, 0, width, height, ctx->memDC, 0, 0, SRCCOPY);
    ReleaseDC(ctx->hwnd, hdc);

    // Non-blocking message pump
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            is_open = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!IsWindow(ctx->hwnd))
    {
        is_open = false;
        return;
    }

    // Mouse tracking
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(ctx->hwnd, &pt);
    mouse_position.x = pt.x;
    mouse_position.y = pt.y;

    is_mouse_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void Window::clear_screen(std::uint64_t color)
{
    Win32Context *ctx = static_cast<Win32Context *>(_window);
    RECT rect = {0, 0, width, height};
    HBRUSH brush = CreateSolidBrush(ConvertColor(color));

    FillRect(ctx->memDC, &rect, brush);
    DeleteObject(brush);
}

void Window::fill_rectangle(int x, int y, int w, int h, Color color, bool is_button)
{
    Win32Context *ctx = static_cast<Win32Context *>(_window);
    
    // Attach GDI+ graphics context
    Gdiplus::Graphics graphics(ctx->memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    // Isolate color channels
    BYTE r_chan = (color >> 24) & 0xFF;
    BYTE g_chan = (color >> 16) & 0xFF;
    BYTE b_chan = (color >> 8) & 0xFF;

    if (is_button)
    {
        int diameter = 12;
        int radius = diameter / 2;

        // Helper lambda to construct a rounded rectangle path for GDI+
        auto add_round_rect = [](Gdiplus::GraphicsPath& path, int rx, int ry, int rw, int rh, int rr) {
            path.AddArc(rx, ry, rr * 2, rr * 2, 180, 90);
            path.AddArc(rx + rw - rr * 2, ry, rr * 2, rr * 2, 270, 90);
            path.AddArc(rx + rw - rr * 2, ry + rh - rr * 2, rr * 2, rr * 2, 0, 90);
            path.AddArc(rx, ry + rh - rr * 2, rr * 2, rr * 2, 90, 90);
            path.CloseFigure();
        };

        // Soft, fading drop shadow
        // Stacking 3 semi-transparent layers creates a pseudo-Gaussian blur effect
        for (int i = 0; i < 3; ++i)
        {
            Gdiplus::GraphicsPath shadowPath;
            // Shift slightly down and right, expanding outwards each iteration
            add_round_rect(shadowPath, x + 1 - i, y + 2 - i, w + (i * 2), h + (i * 2), radius + i);
            
            // Alpha channel decreases (fades out) as the shadow expands
            Gdiplus::SolidBrush shadowBrush(Gdiplus::Color(40 - (i * 12), 0, 0, 0)); 
            graphics.FillPath(&shadowBrush, &shadowPath);
        }

        // Build the main button path
        Gdiplus::GraphicsPath buttonPath;
        add_round_rect(buttonPath, x, y, w, h, radius);
        
        // Draw the button face
        Gdiplus::SolidBrush buttonBrush(Gdiplus::Color(255, r_chan, g_chan, b_chan));
        graphics.FillPath(&buttonBrush, &buttonPath);

        // Draw the subtle border outline
        Gdiplus::Pen borderPen(Gdiplus::Color(255, 112, 112, 112), 1.0f);
        graphics.DrawPath(&borderPen, &buttonPath);
    }
    else
    {
        // Standard, non-button rectangle rendering
        Gdiplus::SolidBrush brush(Gdiplus::Color(255, r_chan, g_chan, b_chan));
        graphics.FillRectangle(&brush, x, y, w, h);
    }
}

void Window::fill_circle(int x, int y, int radius, Color color)
{
    Win32Context *ctx = static_cast<Win32Context *>(_window);

    // Activate the sub-pixel edge smoothing to mimic MacOS behaviour
    Gdiplus::Graphics graphics(ctx->memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    BYTE r = (color >> 24) & 0xFF;
    BYTE g = (color >> 16) & 0xFF;
    BYTE b = (color >> 8) & 0xFF;
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, r, g, b));

    graphics.FillEllipse(&brush, x - r, y - radius, radius * 2, radius * 2);
}

void Window::draw_text(const std::string &text, int x, int y, double size, Color color, int box_width, int box_height)
{
    Win32Context *ctx = static_cast<Win32Context *>(_window);

    // Attach GDI+ graphics context
    Gdiplus::Graphics graphics(ctx->memDC);
    
    // Set text anti-aliasing to match macOS CoreGraphics smooth font rendering
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // Construct the font directly from the cached GDI+ font family
    Gdiplus::Font gdiplusFont(
        g_customFontFamily, 
        static_cast<Gdiplus::REAL>(size), 
        Gdiplus::FontStyleRegular, 
        Gdiplus::UnitPixel // Ensures size parameter acts as raw logical pixels (mimics Mac)
    );

    // Setup the text color brush
    BYTE r_chan = (color >> 24) & 0xFF;
    BYTE g_chan = (color >> 16) & 0xFF;
    BYTE b_chan = (color >> 8) & 0xFF;
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, r_chan, g_chan, b_chan));

    // Setup the text formatting and alignment
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter); 
    format.SetLineAlignment(Gdiplus::StringAlignmentNear); // Equivalent to DT_TOP
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

    // Define the floating-point bounding box
    Gdiplus::RectF layoutRect(
        static_cast<float>(x), 
        static_cast<float>(y), 
        static_cast<float>(box_width), 
        static_cast<float>(box_height)
    );
                              
    std::wstring wtext = ToWideString(text);

    // Render the text seamlessly to the double-buffer
    graphics.DrawString(wtext.c_str(), -1, &gdiplusFont, layoutRect, &format, &textBrush);
}

#endif