#ifdef __APPLE__
#include "window.hpp"
#include <iostream>
#import <AppKit/AppKit.h> // import is preferred for Obj-C
#import <objc/runtime.h> // required for associating the private delegate

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

Window::Window(int _width, int _height, std::string _title)  : width(_width), height(_height), title(_title)
{
    // Encase everything in @autoreleasepool to manage memory for Obj-C blocks
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
        
        // Flush any UI updates
        [NSApp updateWindows];
    }
}

void Window::clear_screen()
{
    @autoreleasepool {
        NSWindow* window = (__bridge NSWindow *)_window;

        if (window) {
            [window setBackgroundColor:[NSColor redColor]];
            std::cout << "hello?" << std::endl;
            [window displayIfNeeded];
        }
    }
}

#endif