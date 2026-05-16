#ifdef __APPLE__
#include "window.hpp"
#include <iostream>
#import <AppKit/AppKit.h> // import is preferred for Obj-C
#import <objc/runtime.h> // required for associating the private delegate
#import <QuartzCore/QuartzCore.h>
#import <CoreText/CoreText.h>

/*
Managing a graphical interface is heavily OS-dependent. To manage this, I have written my code to be compatible with MacOS, as well as Windows 64-bit architecture, as these are the devices I will be running this application on.
*/

/*
Another note:
This file is a .mm file, which allows us to mix standard C++ code and Objective-C code in the same file, which is a requirement for coding with Apple's proprietary Cocoa library.
*/

/*
Private delegate window class
*/
@interface MacWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation MacWindowDelegate - (void)windowWillClose:(NSNotification *)notification {
    // Properly close the app when X is pressed
    [NSApp terminate:nil];
}

@end

/*
The colour conversion to the NSColor object used by AppKit
Uses bitwise shifting and comparing and then normalise between 0 and 1.
*/
static NSColor* convertColor(Color hexColor)
{
    CGFloat r = ((hexColor >> 24) & 0xFF) / 255.0;
    CGFloat g = ((hexColor >> 16) & 0xFF) / 255.0;
    CGFloat b = ((hexColor >> 8) & 0xFF) / 255.0;
    CGFloat a = ((hexColor & 0xFF) & 0xFF) / 255.0;
    return [NSColor colorWithDeviceRed:r green:g blue:b alpha:a];
}

/*
Default constructor
Sourced mostly from https://www.electronjs.org/docs/latest/tutorial/native-code-and-electron-objc-macos
*/
Window::Window(int _width, int _height, std::string _title)  : width(_width), height(_height), title(_title)
{
    // Encase everything in @autoreleasepool to manage memory for Obj-C blocks
    // It means any objects created in this block will be deallocated afterwards, except returns
    @autoreleasepool {
        // initialise global app instance
        [NSApplication sharedApplication];

        // Set policy to regular so it appears in dock and has a UI
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Define window position and size, starting from bottom left on Mac
        NSRect frame = NSMakeRect(0, 0, width, height);

        // Set some styles...
        NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;

        // Create the blank window
        NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];

        [window setTitle:[NSString stringWithUTF8String:title.c_str()]];

        // Set up the private window delegate
        MacWindowDelegate* delegate = [[MacWindowDelegate alloc] init];
        [window setDelegate:delegate];

        // Need to associate the delegate to the window object to stop clearing it from memory once this function is complete
        objc_setAssociatedObject(window, (__bridge const void *)(@"MacWindowDelegate"), delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

        // Enable core animation layers for the window
        [[window contentView] setWantsLayer:YES];

        // Focus the window
        [window makeKeyAndOrderFront:nil];

        // Bring to front
        [NSApp activateIgnoringOtherApps:YES];

        // Tell the Cocoa framework the window is done launching.
        // We avoid using [NSApp run] because this is a blocking method
        [NSApp finishLaunching];

        is_open = true;
        _window = (__bridge_retained void *)window; // Assign the pointer to the window class
    }
}

bool Window::load_font(const std::string& file_path)
{
    @autoreleasepool
    {
        NSString *path = [NSString stringWithUTF8String:file_path.c_str()];
        NSURL *fontURL = [NSURL fileURLWithPath:path];

        CFErrorRef error = NULL;

        bool success = CTFontManagerRegisterFontsForURL((__bridge CFURLRef)fontURL, kCTFontManagerScopeProcess, &error);

        if (!success)
        {
            if (error)
            {
                CFStringRef errorDescription = CFErrorCopyDescription(error);
                std::cerr << "Failed to load font" << std::endl;
                CFRelease(errorDescription);
            }
            return false;
        }
        return true;
    }
}

/*
This basically picks up all operating system events and sends them to the NSApp object, including allowing the window to close properly
I added mouse tracking to this method since the nature of my program requires it every frame anyway.
*/
void Window::process_events()
{
    @autoreleasepool {
        NSEvent* event;
        // Dequeue events (like clicks, drags, typing) in a non-blocking fashion
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }

        NSWindow* window = (__bridge NSWindow *)_window;
        if (window)
        {
            NSPoint mouseLoc = [window mouseLocationOutsideOfEventStream];

            mouse_position.x = static_cast<int>(mouseLoc.x);
            mouse_position.y = height - static_cast<int>(mouseLoc.y);
        }
        
        // Flush any UI updates
        [NSApp updateWindows];
    }
}

/*
The layering functionality in AppKit allows us to build virtual 'layers' on top of the window.
Clear screen has us delete all the layers and set a background color.
*/
void Window::clear_screen(Color color)
{
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow *)_window;

        if (window) {
            CALayer *layer = [[window contentView] layer];
            layer.sublayers = nil;
            layer.backgroundColor = [convertColor(color) CGColor];
        }
    }
}

/*
This method needs to create a new layer and put a rectangle on it, then push the layer to the screen
*/
void Window::fill_rectangle(int x, int y, int w, int h, Color color)
{
    @autoreleasepool
    {
        NSWindow* window = (__bridge NSWindow *)_window;
        
        if (window)
        {
            CALayer *rectLayer = [CALayer layer];
            rectLayer.frame = CGRectMake(x, height - y - h, w, h);
            rectLayer.backgroundColor = [convertColor(color) CGColor];
            
            // This part is important because by default the system will try and smooth animation changes, but we want to show it straight away
            [CATransaction begin];
            [CATransaction setDisableActions:YES];

            [[[window contentView] layer] addSublayer:rectLayer];

            [CATransaction commit];
        }
    }
}

/*
This does a similar thing to fill_rectangle but with text
*/
void Window::draw_text(const std::string& text, int x, int y, double size, Color color)
{
    @autoreleasepool
    {
        NSWindow* window = (__bridge NSWindow *)_window;
        
        if (window)
        {
            CATextLayer *textLayer = [CATextLayer layer];

            textLayer.string = [NSString stringWithUTF8String:text.c_str()];
            textLayer.fontSize = size;
            textLayer.foregroundColor = [convertColor(color) CGColor];
            textLayer.contentsScale = [window backingScaleFactor];

            std::string font_name = "Inter-VariableFont_opsz,wght.ttf";
            textLayer.font = (__bridge CFTypeRef)[NSString stringWithUTF8String:font_name.c_str()];

            int box_width = 300;
            int box_height = 50;

            y = height - y - box_height;

            textLayer.frame = CGRectMake(x, y, box_width, box_height);

            [CATransaction begin];
            [CATransaction setDisableActions:YES];
            [[[window contentView] layer] addSublayer:textLayer];
            [CATransaction commit];
        }

    }
}

bool Window::is_left_mouse_down() const
{
    @autoreleasepool
    {
        return ([NSEvent pressedMouseButtons] & 1) != 0;
    }
}

#endif