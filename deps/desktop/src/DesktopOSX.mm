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
  [[[self mainFrame] frameView] setAllowsScrolling:NO];
  [self setUIDelegate:self];
  WebPreferences* prefs = [self preferences];
  [prefs setAutosaves:NO];
  [prefs setJavaEnabled:NO];
  [prefs setJavaScriptEnabled:YES];
  [prefs setPlugInsEnabled:NO];
  [prefs setPrivateBrowsingEnabled:YES];
  return self;
}
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems {
  return nil;
}
@end

void statusBarInit() {
  id statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
  id defaultImage = [[NSImage new] initWithContentsOfFile:@"images/default.png"];
  [statusItem setImage:defaultImage];
  [statusItem setHighlightMode:YES];
}

void menuBarInit() {
  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
    action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  id closeMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Hide"
    action:@selector(hide) keyEquivalent:@"w"] autorelease];
  [closeMenuItem setTarget:[NSRunningApplication currentApplication]];
  [appMenu addItem:closeMenuItem];
  [appMenuItem setSubmenu:appMenu];
}

void windowInit() {
 id window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
    styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
    backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  WebView *webView;
  webView = [[LoqurWebView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 800, 600))];
  webView.autoresizesSubviews = YES;
  [webView setMainFrameURL:@"file:///Users/justin/dev/node/deps/desktop/html/index.html"];
  [window setContentView:webView];
  [webView release];
  [window setTitle:@"loqur."];
  [window makeKeyAndOrderFront:nil];
}

void desktopInit () {
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  menuBarInit();
  statusBarInit();
  windowInit();
  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
}

