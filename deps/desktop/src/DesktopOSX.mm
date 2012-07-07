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
  [self setMainFrameURL:@"http://www.google.com"];
  return self;
}
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems {
  return nil;
}
@end

static id window;

@interface LoqurWindow : NSWindow {}
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation;
@end

@implementation LoqurWindow
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation {
  self = [super initWithContentRect:contentRect styleMask:windowStyle backing:bufferingType defer:deferCreation];
  return self;
}

@end

void desktopInit () {
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  window = [[[LoqurWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
    styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask)
    backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  WebView *webView;
  webView = [[LoqurWebView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 800, 600))];
  webView.autoresizesSubviews = YES;
  [window setContentView:webView];
  [webView release];
  [window setTitle:@"loqur."];
  [window makeKeyAndOrderFront:nil];
  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
    action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  id closeMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Close"
    action:@selector(close) keyEquivalent:@"w"] autorelease];
  [closeMenuItem setTarget:window];
  [appMenu addItem:closeMenuItem];
  [appMenuItem setSubmenu:appMenu];
  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
}

