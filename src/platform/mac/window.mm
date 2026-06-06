#ifdef __APPLE__
#include "window.hpp"
#include "parameters.hpp"
#include <iostream>
#include <cstdint>
#import <AppKit/AppKit.h> // import is preferred for Obj-C
#import <objc/runtime.h> // required for associating the private delegate
#import <QuartzCore/QuartzCore.h>
#import <CoreText/CoreText.h>
#import <Carbon/Carbon.h> // required for detecting key presses

using namespace Parameters;

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
static NSColor* convertColor(std::uint64_t hexColor)
{
    // Shift each 8-bit channel down to the low byte, mask it off, then normalise to 0..1
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
        NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

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
        // __bridge_retained hands ownership to the void* so the window survives past the autoreleasepool
        _window = (__bridge_retained void *)window; // Assign the pointer to the window class
    }
    setup_input_listeners();
    load_font(GLOBAL_FONT + ".ttf");
}

// only ran once during window/library initialization
void Window::setup_input_listeners()
{
    @autoreleasepool
    {
        // Listen for Left Mouse Down
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskLeftMouseDown handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
            this->is_mouse_down = true;
            return event;
        }];

        // Listen for Left Mouse Up (The "Lifted" state)
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskLeftMouseUp handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
            this->is_mouse_down = false;
            return event;
        }];

        // Listen for scroll wheel and store a rolling total as a class field
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskScrollWheel handler:^NSEvent * _Nullable(NSEvent * _Nonnull event) {
            if (event.scrollingDeltaY != 0)
            {
                this->zoom_level += event.scrollingDeltaY * SCROLL_ZOOM_FACTOR;
            }
            return event;
        }];

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
            // Flip y: AppKit measures from the bottom-left, the rest of the API uses top-left
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
void Window::clear_screen(std::uint64_t color)
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
void Window::fill_rectangle(int x, int y, int w, int h, Color color, bool is_button)
{
    @autoreleasepool
    {
        NSWindow* window = (__bridge NSWindow *)_window;
        
        if (window)
        {
            CALayer *rectLayer = [CALayer layer];
            rectLayer.frame = CGRectMake(x, height - y - h, w, h);
            rectLayer.backgroundColor = [convertColor(color) CGColor];

            if (is_button)
                {
                rectLayer.cornerRadius = 6.0;
                rectLayer.shadowColor = [convertColor(color) CGColor];
                rectLayer.shadowOpacity = 0.3;  // Subtle idle glow
                rectLayer.shadowRadius = 6.0;
                rectLayer.shadowOffset = CGSizeZero; // Glowes outward evenly
            }
            
            // This part is important because by default the system will try and smooth animation changes, but we want to show it straight away
            [CATransaction begin];
            [CATransaction setDisableActions:YES];

            [[[window contentView] layer] addSublayer:rectLayer];

            [CATransaction commit];
        }
    }
}

/*
Creates a CAShapeLayer holding an ellipse path and pushes it to the screen.
Like the other draw calls, the y is flipped (height - y) to convert from our top-left origin to AppKit's bottom-left.
*/
void Window::fill_circle(int x, int y, int radius, Color color)
{
    @autoreleasepool
    {
        NSWindow* window = (__bridge NSWindow *)_window;

        if (window)
        {
            CAShapeLayer *circleLayer = [CAShapeLayer layer];
            // Build a square bounding box centred on (x, y); the ellipse is inscribed within it
            CGRect bounding_rect = CGRectMake(x - radius, height - y - radius, 2*radius, 2*radius);

            CGPathRef path = CGPathCreateWithEllipseInRect(bounding_rect, NULL);
            circleLayer.path = path;
            CGPathRelease(path);
            circleLayer.fillColor = [convertColor(color) CGColor];

            [CATransaction begin];
            [CATransaction setDisableActions:YES];

            [[[window contentView] layer] addSublayer:circleLayer];

            [CATransaction commit];
        }
    }
}

/*
Draws a straight line between two points at any angle, using a stroked CAShapeLayer.
Note the y-flip (height - y): AppKit's content view uses a bottom-left origin, while the rest of the API uses top-left.
*/
void Window::draw_line(int x1, int y1, int x2, int y2, Color color, int linewidth)
{
    @autoreleasepool
    {
        NSWindow* window = (__bridge NSWindow *)_window;

        if (window)
        {
            CAShapeLayer *lineLayer = [CAShapeLayer layer];

            CGMutablePathRef path = CGPathCreateMutable();
            CGPathMoveToPoint(path, NULL, x1, height - y1);
            CGPathAddLineToPoint(path, NULL, x2, height - y2);
            lineLayer.path = path;
            CGPathRelease(path);

            lineLayer.strokeColor = [convertColor(color) CGColor];
            lineLayer.lineWidth = linewidth;
            lineLayer.fillColor = [[NSColor clearColor] CGColor];

            [CATransaction begin];
            [CATransaction setDisableActions:YES];

            [[[window contentView] layer] addSublayer:lineLayer];

            [CATransaction commit];
        }
    }
}

/*
This does a similar thing to fill_rectangle but with text
*/
void Window::draw_text(const std::string& text, int x, int y, double size, Color color, int box_width, int box_height)
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
            textLayer.alignmentMode = kCAAlignmentCenter;
            textLayer.contentsScale = [window backingScaleFactor];

            textLayer.font = (__bridge CFTypeRef)[NSString stringWithUTF8String:GLOBAL_FONT.c_str()];

            // CATextLayer has no vertical-centre mode, so size the layer to the
            // text's line height and centre that frame within the button box.
            NSFont *nsFont = [NSFont fontWithName:[NSString stringWithUTF8String:GLOBAL_FONT.c_str()] size:size];
            CGFloat lineHeight = nsFont ? (nsFont.ascender - nsFont.descender) : size;

            int box_bottom = height - y - box_height;
            CGFloat textY = box_bottom + (box_height - lineHeight) / 2.0;

            textLayer.frame = CGRectMake(x, textY, box_width, lineHeight);

            [CATransaction begin];
            [CATransaction setDisableActions:YES];
            [[[window contentView] layer] addSublayer:textLayer];
            [CATransaction commit];
        }

    }
}

#endif