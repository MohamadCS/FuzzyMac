#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/MainWindow.hpp"

#include <QDebug>
#include <QImage>
#include <QPointer>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
#include <algorithm>
#include <objc/objc-runtime.h>
#include <semaphore>
#include <string>
#include <vector>

#import <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
#import <LocalAuthentication/LocalAuthentication.h>
#import <QuartzCore/QuartzCore.h>
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

extern "C" void setupWindowDecoration(MainWindow *widget, ConfigManager *cfg) {
  // Get the native NSWindow handle
  @autoreleasepool {
    //
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

    [window setOpaque:NO]; // Set window to be non-opaque
    [window setBackgroundColor:[NSColor clearColor]]; // Set the background
                                                      // transparent

    NSView *content_view = [window contentView];
    CGFloat radius = cfg->get<float>({"corner_radius"});

    [content_view setWantsLayer:YES];
    content_view.layer.cornerRadius = radius;
    content_view.layer.masksToBounds = YES;

    // Border color
    QColor border_color(
        QString::fromStdString(cfg->get<std::string>({"colors","outer_border"})));
    CGFloat r = border_color.redF();
    CGFloat g = border_color.greenF();
    CGFloat b = border_color.blueF();
    CGFloat a = border_color.alphaF();
    NSColor *cg_radius_color = [NSColor colorWithCalibratedRed:r
                                                         green:g
                                                          blue:b
                                                         alpha:a];

    // Remove old border layer if exists
    NSArray<CALayer *> *sublayers = [content_view.layer.sublayers copy];
    for (CALayer *layer in sublayers) {
      if ([layer.name isEqualToString:@"BorderLayer"]) {
        [layer removeFromSuperlayer];
      }
    }

    // Rounded corners mask
    NSBezierPath *mask_path =
        [NSBezierPath bezierPathWithRoundedRect:[content_view bounds]
                                        xRadius:radius
                                        yRadius:radius];
    CAShapeLayer *mask_layer = [CAShapeLayer layer];
    mask_layer.path = [mask_path CGPath];
    content_view.layer.mask = mask_layer;

    // Rounded border
    CGFloat lineWidth = cfg->get<int>({"border_size"});
    NSRect borderRect =
        NSInsetRect([content_view bounds], lineWidth / 2, lineWidth / 2);
    NSBezierPath *border_path =
        [NSBezierPath bezierPathWithRoundedRect:borderRect
                                        xRadius:radius - lineWidth / 2
                                        yRadius:radius - lineWidth / 2];
    CAShapeLayer *border_layer = [CAShapeLayer layer];
    border_layer.name = @"BorderLayer"; // identify for removal
    border_layer.path = [border_path CGPath];
    border_layer.fillColor = [[NSColor clearColor] CGColor];
    border_layer.strokeColor = [cg_radius_color CGColor];
    border_layer.lineWidth = lineWidth;
    border_layer.frame = [content_view bounds];
    [content_view.layer addSublayer:border_layer];
  }
}

extern "C" void addMaterial(QWidget *widget) {

  @autoreleasepool {
    //   NSView *native_view = reinterpret_cast<NSView *>(widget->winId());
    //   NSWindow *window = [native_view window];
    // NSVisualEffectView *effectView =
    //     [[NSVisualEffectView alloc] initWithFrame:window.contentView.frame];
    //
    // // Set the desired vibrancy effect (light or dark blur)
    // [effectView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
    //
    //                                                      // Light or Dark
    //
    // // Add the effect view to the window's content view
    // [window.contentView addSubview:effectView
    //                     positioned:NSWindowBelow
    //                     relativeTo:nil];
    //
    //
    // NSView *native_view = reinterpret_cast<NSView *>(widget->winId());
    // NSWindow *window = [native_view window];
    //
    // NSVisualEffectView *visualEffectView =
    //     [[NSVisualEffectView alloc] initWithFrame:[window frame]];
    // visualEffectView.material = NSVisualEffectMaterialHUDWindow;
    // visualEffectView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    // // visualEffectView.state = NSVisualEffectStateActive;
    //
    // // Set the frosted effect on the native window
    // [window setContentView:visualEffectView];
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

@interface QuickLookDelegate : NSObject <QLPreviewPanelDataSource>
@property(nonatomic, strong) NSArray<NSURL *> *file_urls;
@end

@implementation QuickLookDelegate
- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
  return self.file_urls.count;
}

- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel
               previewItemAtIndex:(NSInteger)index {
  return self.file_urls[index];
}
@end

static QuickLookDelegate *g_delegate = nil;

extern "C++" void showQuickLookPanel(const QString &filePath) {
  if (!g_delegate) {
    g_delegate = [[QuickLookDelegate alloc] init];
  }

  NSURL *url = [NSURL fileURLWithPath:filePath.toNSString()];
  g_delegate.file_urls = @[ url ];

  QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
  panel.dataSource = g_delegate;

  dispatch_async(dispatch_get_main_queue(), ^{
    [panel makeKeyAndOrderFront:nil];
  });
}
