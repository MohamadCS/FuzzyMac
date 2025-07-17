#import <Cocoa/Cocoa.h>

#include <QWidget>
#include <algorithm>
#include <objc/objc-runtime.h>
#include <string>
#include <vector>

extern "C" void deactivateApp() {

  [NSApp hide:nil]; // This returns focus to the previously active app
}


extern "C" void centerWindow(QWidget *widget) {
  @autoreleasepool {
    NSView *native_view = reinterpret_cast<NSView *>(widget->winId());
    NSWindow *window = [native_view window];
    if (!window)
      return;
    [window center];
  }
}
extern "C" void makeWindowFloating(QWidget *widget) {
  // Get the native NSWindow handle
  @autoreleasepool {
    NSView *native_view = reinterpret_cast<NSView *>(widget->winId());
    NSWindow *window = [native_view window];

    // Set the floating window level
    [window setLevel:NSFloatingWindowLevel];
    [window setHidesOnDeactivate:YES];

    [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                   NSWindowCollectionBehaviorTransient |
                                   NSWindowCollectionBehaviorStationary)];

    [window setStyleMask:(NSWindowStyleMaskBorderless)];
    [window center];
  }
}

extern "C" void disableCmdQ() {
  @autoreleasepool {

    NSApplication *app = [NSApplication sharedApplication];

    // Get the class of the NSApplication instance
    Class app_class = [app class];
    SEL terminate_selector = @selector(terminate:);

    // Get the original terminate: method
    Method original_method =
        class_getInstanceMethod(app_class, terminate_selector);
    if (!original_method) {
      return;
    }

    // Create a new implementation block that does nothing on terminate:
    IMP new_imp = imp_implementationWithBlock(^void(id _self, id sender){
        // No call to original terminate:, so quitting is disabled
    });

    // Replace the original implementation with our new one
    method_setImplementation(original_method, new_imp);
  }
}
