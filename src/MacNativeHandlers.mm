#import <Cocoa/Cocoa.h>

extern "C" void MakeWindowKey(void* windowPtr) {
  NSWindow* window = (__bridge NSWindow*)windowPtr;

    [NSApp activateIgnoringOtherApps:YES];
    // Optional but useful: keep window above others
    [window setLevel:NSFloatingWindowLevel];
    [window makeKeyAndOrderFront:nil]; // Accept input}
}

extern "C" void DeactivateApp() {
    [NSApp hide:nil]; // This returns focus to the previously active app
}

