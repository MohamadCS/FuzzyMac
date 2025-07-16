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

#import <Foundation/Foundation.h>
#import <string>
#import <vector>

extern "C++" std::vector<std::string>
spotlightSearch(const std::vector<std::string> &dirs,
                const std::string &query) {
  @autoreleasepool {
    std::vector<std::string> results;

    NSMetadataQuery *metadata_query = [[NSMetadataQuery alloc] init];
    if (!metadata_query) {
      return results;
    }

    // Convert query string to NSPredicate
    NSPredicate *predicate = nil;
    @try {
      predicate = [NSPredicate
          predicateWithFormat:[NSString stringWithUTF8String:query.c_str()]];
    } @catch (NSException *exception) {
      return results;
    }

    if (!predicate) {
      return results;
    }

    [metadata_query setPredicate:predicate];

    // Add search scopes
    NSMutableArray *scopes = [NSMutableArray array];
    for (const auto &dir : dirs) {
      NSString *path = [NSString stringWithUTF8String:dir.c_str()];
      if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
        [scopes addObject:[NSURL fileURLWithPath:path]];
      } else {
      }
    }

    if (scopes.count == 0) {
      return results;
    }

    [metadata_query setSearchScopes:scopes];

    __block BOOL query_finished = NO;

    // Add observer for query finished notification
    id observer = [[NSNotificationCenter defaultCenter]
        addObserverForName:NSMetadataQueryDidFinishGatheringNotification
                    object:metadata_query
                     queue:nil
                usingBlock:^(NSNotification *note) {
                  query_finished = YES;
                }];

    if (![metadata_query startQuery]) {
      [[NSNotificationCenter defaultCenter] removeObserver:observer];
      return results;
    }

    // Run loop until query finishes or timeout (max 5 seconds)
    NSDate *timeout_date = [NSDate dateWithTimeIntervalSinceNow:5.0];
    while (!query_finished &&
           [[NSDate date] compare:timeout_date] == NSOrderedAscending) {
      [[NSRunLoop currentRunLoop]
             runMode:NSDefaultRunLoopMode
          beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    }

    // Stop query & cleanup
    [metadata_query stopQuery];
    [[NSNotificationCenter defaultCenter] removeObserver:observer];

    // Check if query timed out
    if (!query_finished) {
      return results;
    }

    // Extract results
    const int max = 30;
    for (NSMetadataItem *item in metadata_query.results) {
      if (results.size() > max) {
        return results;
      }
      NSString *path =
          [item valueForAttribute:(__bridge NSString *)kMDItemPath];
      if (path) {
        results.emplace_back([path UTF8String]);
      }
    }

    return results;
  }
}
//
// #import <Cocoa/Cocoa.h>
// #import <QuickLook/QuickLook.h>
// #import <string>
// #import <vector>
//
// #import <Cocoa/Cocoa.h>
// #import <QuickLook/QuickLook.h>
// #import <string>
//
// @interface QLPreviewHelper : NSObject <QLPreviewPanelDataSource, QLPreviewPanelDelegate>
// @property (nonatomic, strong) NSURL *fileURL;
// @end
//
// @implementation QLPreviewHelper
//
// - (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
//     return 1;
// }
//
// - (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index {
//     return self.fileURL;
// }
//
// // Called when the panel is about to close
// - (void)previewPanelWillClose:(QLPreviewPanel *)panel {
//     // Stop the app’s event loop so it can exit
//     [NSApp stop:nil];
// }
//
// @end

extern "C++" void quickLook(const std::string &path) {
    // @autoreleasepool {
    //     // Setup Cocoa app context if needed
    //     if (NSApp == nil) {
    //         [NSApplication sharedApplication];
    //     }
    //
    //     // Create the preview helper
    //     QLPreviewHelper *helper = [[QLPreviewHelper alloc] init];
    //     helper.fileURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]];
    //
    //     // Configure the QLPreviewPanel
    //     QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
    //     panel.delegate = helper;
    //     panel.dataSource = helper;
    //
    //     [panel updateController];
    //     [panel makeKeyAndOrderFront:nil];
    //
    //     // Run the app event loop so user can interact with the panel
    //     [NSApp run];
    //
    //     // Cleanup: after the user closes the panel, run stops and control returns here
    //     panel.delegate = nil;
    //     panel.dataSource = nil;
    // }
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
