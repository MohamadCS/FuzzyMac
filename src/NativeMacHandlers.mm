#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <string>
#include <vector>

extern "C" void deactivateApp() {

  [NSApp hide:nil]; // This returns focus to the previously active app
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
    const auto MAX = 20;
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
