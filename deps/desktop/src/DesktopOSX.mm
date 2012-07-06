#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#include "Desktop.h"

@interface LoqurWebView : WebView {}
- (id)initWithFrame:(NSRect)frameRect;
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;
@end

@implementation LoqurWebView
- (id)initWithFrame:(NSRect)frameRect {
  self = [super initWithFrame:frameRect];
  self.autoresizingMask = (NSViewHeightSizable | NSViewWidthSizable);
  [self setMainFrameURL:@"http://www.google.com"];
  [[[self mainFrame] frameView] setAllowsScrolling:NO];
  [self setUIDelegate:self];
  return self;
}
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems {
  return nil;
}
@end

void desktopInit (void) {
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id appName = @"loqur.";
  id quitTitle = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
    action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];
  id window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
    styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
    backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  WebView *webView;
  webView = [[LoqurWebView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 800, 600))];
  webView.autoresizesSubviews = YES;
  [window setContentView:webView];
  [webView release];
  [window setTitle:appName];
  [window makeKeyAndOrderFront:nil];
  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
}

