#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#include <algorithm>
#include <string>
#include <vector>

extern "C" void MakeWindowKey(void *windowPtr) {
  NSWindow *window = (__bridge NSWindow *)windowPtr;

  [NSApp activateIgnoringOtherApps:YES];
  // Optional but useful: keep window above others
  [window setLevel:NSFloatingWindowLevel];
  [window makeKeyAndOrderFront:nil]; // Accept input}
}

extern "C" void DeactivateApp() {
  [NSApp hide:nil]; // This returns focus to the previously active app
}

// spotlight_search.mm

// Convert CFStringRef to std::string
static std::string CFStringToStdString(CFStringRef cfStr) {
  if (!cfStr)
    return "";
  CFIndex length = CFStringGetLength(cfStr);
  CFIndex maxSize =
      CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
  char *buffer = (char *)malloc(maxSize);
  std::string result;
  if (CFStringGetCString(cfStr, buffer, maxSize, kCFStringEncodingUTF8)) {
    result = buffer;
  }
  free(buffer);
  return result;
}

// Perform a Spotlight search with a given query string
extern "C++" std::vector<std::string>
spotlightSearch(const std::string &queryString) {
  std::vector<std::string> results;

  // Create CFString from std::string
  CFStringRef cfQuery = CFStringCreateWithCString(NULL, queryString.c_str(),
                                                  kCFStringEncodingUTF8);
  if (!cfQuery)
    return results;

  // Create Spotlight query
  MDQueryRef query = MDQueryCreate(kCFAllocatorDefault, cfQuery, NULL, NULL);

  NSArray *scopes = @[
    [@"~/Library/Mobile Documents" stringByExpandingTildeInPath],
    NSHomeDirectory()
  ];

  MDQuerySetSearchScope(query, (__bridge CFArrayRef)scopes, 0);

  if (!query)
    return results;

  // Run the query synchronously
  if (!MDQueryExecute(query, kMDQuerySynchronous)) {
    CFRelease(query);
    return results;
  }

  // Collect results
  CFIndex count = MDQueryGetResultCount(query);
  CFIndex max = 30;

  for (CFIndex i = 0; i < std::min(max, count); ++i) {
    MDItemRef item = (MDItemRef)MDQueryGetResultAtIndex(query, i);
    if (!item)
      continue;

    CFStringRef path = (CFStringRef)MDItemCopyAttribute(item, kMDItemPath);
    if (path) {
      results.push_back(CFStringToStdString(path));
      CFRelease(path);
    }
  }

  CFRelease(query);
  return results;
}
