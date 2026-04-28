#import "visualiser.h" // import is preferred for Obj-C
#import <AppKit/AppKit.h>

/*
Managing a graphical interface is heavily OS-dependent. To manage this, I have written my code to be compatible with MacOS, as well as Windows 64-bit architecture, as these are the devices I will be running this application on.
*/

/*
Another note:
This file is a .mm file, which allows us to mix standard C++ code and Objective-C code in the same file, which is a requirement for coding with Apple's proprietary Cocoa library.
*/

int open_window()
{
    // initialise global app instance
    [NSApplication sharedApplication];

    // Set policy to regular so it appears in dock and has a UI
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Define window position and size, starting from bottom left on Mac
    NSRect frame = NSMakeRect(100, 100, 480, 270);

    // Set some styles...
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;

    // Create the blank window
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:@"Blank Window"];

    // Focus the window
    [window makeKeyAndOrderFront:nil];

    // Bring to front
    [NSApp activateIgnoringOtherApps:YES];

    // Start event loop to keep window open. The event loop also registers keyboard/mouse input each frame.
    [NSApp run];

    return 0;
}