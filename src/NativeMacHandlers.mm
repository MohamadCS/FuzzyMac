#include "FuzzyMac/NativeMacHandlers.hpp"

#include <QDebug>
#include <QImage>
#include <QPointer>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <objc/objc-runtime.h>
#include <semaphore>
#include <string>
#include <vector>

#import <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
#import <LocalAuthentication/LocalAuthentication.h>
#include <QuickLook/QuickLook.h>
#import <QuickLookThumbnailing/QuickLookThumbnailing.h>
#include <QuickLookUI/QuickLookUI.h>

extern "C++" int getClipboardCount() {
  @autoreleasepool {
    return [[NSPasteboard generalPasteboard] changeCount];
  }
}
extern "C" void deactivateApp() {
  @autoreleasepool {
    [NSApp hide:nil]; // This returns focus to the previously active app
  }
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
    });

    // Replace the original implementation with our new one
    method_setImplementation(original_method, new_imp);
  }
}

extern "C++" QImage getThumbnailImage(const QString &filePath, int width,
                                      int height) {
  @autoreleasepool {

    NSURL *url = [NSURL
        fileURLWithPath:[NSString stringWithString:filePath.toNSString()]];

    auto *generator = [QLThumbnailGenerator sharedGenerator];

    auto *request = [[[QLThumbnailGenerationRequest alloc]
          initWithFileAtURL:url
                       size:CGSizeMake(width, height)
                      scale:[NSScreen mainScreen].backingScaleFactor
        representationTypes:
            QLThumbnailGenerationRequestRepresentationTypeThumbnail]
        autorelease];

    // allow modification inside the block
    __block QImage result;
    std::binary_semaphore *sem = new std::binary_semaphore{0};

    [generator
        generateBestRepresentationForRequest:request
                           completionHandler:^(
                               QLThumbnailRepresentation *thumbnail,
                               NSError *) {
                             if (thumbnail && thumbnail.CGImage) {
                               size_t w = CGImageGetWidth(thumbnail.CGImage);
                               size_t h = CGImageGetHeight(thumbnail.CGImage);
                               QImage img(w, h,
                                          QImage::Format_ARGB32_Premultiplied);
                               CGContextRef ctx = CGBitmapContextCreate(
                                   img.bits(), w, h, 8, img.bytesPerLine(),
                                   CGImageGetColorSpace(thumbnail.CGImage),
                                   kCGImageAlphaPremultipliedFirst |
                                       kCGBitmapByteOrder32Host);
                               CGContextDrawImage(ctx, CGRectMake(0, 0, w, h),
                                                  thumbnail.CGImage);
                               CGContextRelease(ctx);
                               result = std::move(img);
                             }

                             sem->release();
                           }];

    sem->acquire();

    delete sem;
    return result;
  }
}

#import <Foundation/Foundation.h>
#import <LocalAuthentication/LocalAuthentication.h>

extern "C++" bool authenticateWithTouchID() {
  @autoreleasepool {

    LAContext *context = [[[LAContext alloc] init] autorelease];
    NSError *authError = nil;

    bool canEvaluate = [context
        canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics
                    error:&authError];

    if (!canEvaluate) {
      return false;
    }

    __block BOOL success = NO;
    std::binary_semaphore *sem = new std::binary_semaphore{0};

    [context evaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics
            localizedReason:@"Unlock clipboard history"
                      reply:^(BOOL didSucceed, NSError *_Nullable error) {
                        success = didSucceed;
                        sem->release();
                      }];

    sem->acquire();
    delete sem;
    return success;
  }
}

extern "C++" std::string getFrontmostAppName() {
  @autoreleasepool {

    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSRunningApplication *active_app = [workspace frontmostApplication];
    NSURL *bundle_url = [active_app bundleURL];

    std::string result;
    if (bundle_url != nil) {
      NSString *path = [bundle_url path];
      result = [path UTF8String];
    } else {
      result = "Unknown";
    }

    return result;
  }
}
