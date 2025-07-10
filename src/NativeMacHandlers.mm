#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <QuickLook/QuickLook.h>
#import <Foundation/Foundation.h>
#import <QuickLook/QuickLook.h>
#import <Foundation/Foundation.h>
#import <QuickLookUI/QuickLookUI.h>

#include <QWidget>
#include <algorithm>
#include <objc/objc-runtime.h>
#include <string>
#include <vector>

extern "C" void deactivateApp() {

  [NSApp hide:nil]; // This returns focus to the previously active app
}

#import <Cocoa/Cocoa.h>
#include <QWidget>

extern "C" void makeWindowFloating(QWidget *widget) {
  // Get the native NSWindow handle
  NSView *native_view = reinterpret_cast<NSView *>(widget->winId());
  NSWindow *window = [native_view window];

  // Set the floating window level
  [window setLevel:NSFloatingWindowLevel];
  [window setHidesOnDeactivate:YES];

  [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                 NSWindowCollectionBehaviorTransient |
                                 NSWindowCollectionBehaviorStationary)];

  [window setStyleMask:(NSWindowStyleMaskUtilityWindow |
                        NSWindowStyleMaskNonactivatingPanel |
                        NSWindowStyleMaskBorderless)];
}

extern "C++" std::vector<std::string>
spotlightSearch(const std::string &query,
                const std::vector<std::string> &dirs) {

  @autoreleasepool {
    NSMutableArray *args = [NSMutableArray arrayWithObject:@"-onlyin"];

    for (const std::string &folder : dirs) {
      [args addObject:[NSString stringWithUTF8String:folder.c_str()]];
    }

    [args addObject:[NSString stringWithUTF8String:query.c_str()]];

    NSTask *task = [[NSTask alloc] init];
    task.launchPath = @"/usr/bin/mdfind";
    task.arguments = args;

    NSPipe *pipe = [NSPipe pipe];
    task.standardOutput = pipe;

    [task launch];
    [task waitUntilExit];

    NSData *data = [[pipe fileHandleForReading] readDataToEndOfFile];
    NSString *output = [[NSString alloc] initWithData:data
                                             encoding:NSUTF8StringEncoding];

    NSArray<NSString *> *lines = [output componentsSeparatedByString:@"\n"];

    std::vector<std::string> results;

    int i = 0;
    const auto MAX = 30;
    for (NSString *line in lines) {
      if (i > MAX) {
        break;
      }

      if ([line length] > 0) {
        results.emplace_back([line UTF8String]);
      }
      ++i;
    }
    return results;
  }
}


@interface PreviewController : NSObject <QLPreviewPanelDataSource>
{
    NSURL *_file;
}
- (instancetype)initWithFile:(NSURL *)file;
- (void)showPreview;
@end

@implementation PreviewController
- (instancetype)initWithFile:(NSURL *)file {
    if ((self = [super init])) {
        _file = file;
    }
    return self;
}

- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *)panel {
    return 1;
}

- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *)panel previewItemAtIndex:(NSInteger)index {
    return _file;
}

- (void)showPreview {
    QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
    panel.dataSource = self;
    [panel makeKeyAndOrderFront:nil];
    [panel reloadData];
}
@end

static PreviewController *gPreviewController = nil;

extern "C" void closeQuickLook() {
    QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
    if ([panel isVisible]) {
        [panel orderOut:nil];  // Close the panel
    }
}

extern "C++" void quickLock(const std::string& filePath) {
    @autoreleasepool {
        closeQuickLook();
        NSString *nsPath = [NSString stringWithUTF8String:filePath.c_str()];
        NSURL *url = [NSURL fileURLWithPath:nsPath];
        if (!url) return;

        gPreviewController = [[PreviewController alloc] initWithFile:url];
        [gPreviewController showPreview];
    }
}

