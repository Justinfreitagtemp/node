#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#import <ServiceManagement/ServiceManagement.h>
#import "Desktop.h"

#define INITIAL_WIDTH 700
#define INITIAL_HEIGHT 500
#define PADDING_LEFT_RIGT 50
#define PADDING_TOP_BOTTOM 100
#define MAX_WIDTH 1200
#define MAX_HEIGHT 1000
#define MIN_WIDTH 600
#define MIN_HEIGHT 400

BOOL addSecurityBookmark (NSURL*url);

NSWindow *window;
NSWindow *w;

@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end

@interface LoqurWebView : WebView {
  NSInteger move;
}
- (id)initWithFrame:(NSRect)frameRect;
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;
- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame *)frame;
- (void)shouldMove:(NSInteger)value;
- (id)verifyMove:(NSPoint)point;
- (NSInteger)move;
- (void) closeWindow;
- (void) minimiseWindow;
- (void) maximiseWindow;
@end

LoqurWebView *webView;

@interface LoqurWindow : NSWindow {
  NSPoint initialLocation;
}
@property (assign) NSPoint initialLocation;
@end

@implementation LoqurWindow
@synthesize initialLocation;
- (BOOL)canBecomeMainWindow {
    return YES;
}
- (BOOL)canBecomeKeyWindow {
    return YES;
}
- (void)sendEvent:(NSEvent *)event {
  [super sendEvent:event];
  NSEventType type = [event type];
  if (type == NSLeftMouseDown) {
    [self mouseDown:event];
    NSPoint locationInView = [webView convertPoint:[event locationInWindow] fromView:webView];
    locationInView.y = [self frame].size.height - locationInView.y;
    [webView shouldMove: [[webView verifyMove:locationInView] intValue]];
  }
  else if ([webView move]) {
    if (type == NSLeftMouseDown) [self mouseDown:event];
    else if (type == NSLeftMouseDragged) [self mouseDragged:event];
  }
}

- (void)mouseDown:(NSEvent *)event {
  self.initialLocation = [event locationInWindow];
}
- (void)mouseDragged:(NSEvent *)event {
  NSPoint currentLocation;
  NSPoint newOrigin;
  NSRect  screenFrame = [[NSScreen mainScreen] visibleFrame];
  NSRect  windowFrame = [self frame];
  currentLocation = [NSEvent mouseLocation];
  newOrigin.x = currentLocation.x - initialLocation.x;
  newOrigin.y = currentLocation.y - initialLocation.y;
  if ((newOrigin.y+windowFrame.size.height) > (screenFrame.origin.y+screenFrame.size.height))
    newOrigin.y=screenFrame.origin.y + (screenFrame.size.height-windowFrame.size.height);
  [self setFrameOrigin:newOrigin];
}
@end

@interface WebFrameView (transparent)
@end

@implementation WebFrameView (transparent)

- (BOOL)isOpaque {
  return NO;
}
@end

@implementation LoqurWebView
- (id)initWithFrame:(NSRect)frameRect {
  self = [super initWithFrame:frameRect];
  self.autoresizingMask = (NSViewHeightSizable | NSViewWidthSizable);
  [[[self mainFrame] frameView] setAllowsScrolling:NO];
  [self setFrameLoadDelegate:self];
  [self setUIDelegate:self];
  [self setEditingDelegate:self];
  [self setMaintainsBackForwardList:NO];
  [self setDrawsBackground:NO];
  WebPreferences* prefs = [self preferences];
  [prefs setAutosaves:NO];
  [prefs setJavaEnabled:NO];
  [prefs setJavaScriptEnabled:YES];
  [prefs setPlugInsEnabled:NO];
  [prefs setPrivateBrowsingEnabled:YES];
  //[self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
  return self;
}
- (BOOL)isOpaque {
  return NO;
}
/*
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems {
  return nil;
}
*/
- (NSUInteger)webView:(WebView *)sender dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo {
  return WebDragDestinationActionAny;
}
- (NSUInteger)webView:(WebView *)sender dragSourceActionMaskForPoint:(NSPoint)point {
  return WebDragSourceActionAny;
}

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender{
  [super draggingUpdated:sender];
  return NSDragOperationEvery;
}
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  [super draggingEntered:sender];
  return NSDragOperationEvery;
}
- (void)draggingExited:(id <NSDraggingInfo>)sender {
  [super draggingExisting:sender];
}
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender  {
  [super prepareForDragOperation:sender];
  return YES;
}
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
  [super performDragOperation:sender];
  return YES;
}
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender{
  NSArray *draggedFilenames = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
  for (id filename in draggedFilenames) {
    NSURL *url = [NSURL fileURLWithPath:filename];
    addSecurityBookmark(url);
  }
}
- (NSInteger)move {
  return move;
}
- (void)shouldMove:(NSInteger)value {
  move = value;
}
- (void)closeWindow {
  [[NSRunningApplication currentApplication] hide];
}
- (void)minimiseWindow {
  [[self window] miniaturize:self];
}
- (void)maximiseWindow {
  NSWindow *window = [self window];
  NSRect existingFrame = [window frame];
  NSSize maxSize = [window maxSize];
  NSRect newFrame = NSMakeRect(existingFrame.origin.x, existingFrame.origin.y, maxSize.width, maxSize.height);
  [window setFrame:newFrame display:true animate:true];
}
+ (NSString*)webScriptNameForSelector:(SEL)sel {
  if (sel == @selector(shouldMove:)) return @"shouldMove:";
  if (sel == @selector(closeWindow)) return @"closeWindow";
  if (sel == @selector(minimiseWindow)) return @"minimiseWindow";
  if (sel == @selector(maximiseWindow)) return @"maximiseWindow";
  return nil;
}
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
  if (sel == @selector(shouldMove:)) return NO;
  if (sel == @selector(closeWindow)) return NO;
  if (sel == @selector(minimiseWindow)) return NO;
  if (sel == @selector(maximiseWindow)) return NO;
  return YES;
}
- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame *)frame {
  [windowScriptObject setValue:self forKey:@"Cocoa"];
}
- (id)verifyMove:(NSPoint)point {
  NSArray *args = [NSArray arrayWithObjects:[NSNumber numberWithInt:(NSInteger) point.x],[NSNumber numberWithInt:(NSInteger) point.y], nil];
  @try {
    return [[self windowScriptObject] callWebScriptMethod:@"verifyMove" withArguments:args];
  }
  @catch (NSException *e) {}
  return [NSNumber numberWithInt:0];
}
@end

@interface WindowDelegate : NSObject
@end

@implementation WindowDelegate
- (void)windowDidResignKey:(NSNotification *)notification {
  @try {
    [[webView windowScriptObject] callWebScriptMethod:@"windowLostFocus" withArguments:nil];
  }
  @catch (NSException *e) {}
}
- (void)windowDidBecomeKey:(NSNotification *)notification {
  @try {
    [[webView windowScriptObject] callWebScriptMethod:@"windowGainedFocus" withArguments:nil];
  }
  @catch (NSException *e) {}
}
@end

void addFiles() {
  NSOpenPanel *openDlg = [NSOpenPanel openPanel];
  [openDlg setPrompt:@"Choose files or folders to add"];
  [openDlg setTitle:@"Title"];
  [openDlg setAllowsMultipleSelection:YES];
  [openDlg setShowsHiddenFiles:YES];
  [openDlg setCanChooseDirectories:YES];
  [openDlg setCanChooseFiles:YES];
  for (id url in [openDlg URLs]) {
    addSecurityBookmark(url);
  }
}

BOOL addSecurityBookmark (NSURL* url) {
  NSError *error = nil;
  NSData *bookmarkData = [url bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&error];
  if (error || (bookmarkData == nil)) {
    NSLog(@"Secure bookmark creation of %@ failed with error: %@",[url path],[error localizedDescription]);
    return NO;
  } else {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults]; 
    NSMutableDictionary *sandboxedBookmarks = [NSMutableDictionary dictionaryWithDictionary:[defaults objectForKey:@"sandboxSecureBookmarks"]];        
    [sandboxedBookmarks setObject:bookmarkData forKey:[url path]];
    [defaults setObject:sandboxedBookmarks forKey:@"sandboxSecureBookmarks"];
    [defaults synchronize];
  }
  return YES;
}

NSURL* getSecurityBookmark(NSURL* url) {
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary *sandboxedBookmarks = [NSMutableDictionary dictionaryWithDictionary:[defaults objectForKey:@"sandboxSecureBookmarks"]];
  NSData *bookmark = [sandboxedBookmarks objectForKey:[[url URLByDeletingLastPathComponent] path]];     //first see if we have a bookmark for the parent directory
  if (bookmark == nil) bookmark = [sandboxedBookmarks objectForKey:[url path]];                         //if not then look for a bookmark of the exact file
  if (bookmark) {
    NSError *error = nil;
    BOOL bookmarkIsStale = NO;
    NSURL *bookmarkURL = [NSURL URLByResolvingBookmarkData:bookmark options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bookmarkIsStale error:&error]; 
    if (bookmarkIsStale || (error != nil)) {
      [sandboxedBookmarks removeObjectForKey:[url path]];
      [defaults setObject:sandboxedBookmarks forKey:@"sandboxSecureBookmarks"];
      [defaults synchronize];
      NSLog(@"Secure bookmark was pruned, resolution of %@ failed with error: %@",[url path],[error localizedDescription]);
    } else {
      return bookmarkURL;
    }
  }
  return nil;
}

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
  NSUInteger windowStyle = NSBorderlessWindowMask; //NSResizableWindowMask;
  window = [[[LoqurWindow alloc] initWithContentRect:NSMakeRect(50, 100, INITIAL_WIDTH, INITIAL_HEIGHT)
    styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window center];
  [[window windowController] setShouldCascadeWindows:NO];
  //[window setFrameAutosaveName:@"loqurWindow"];
  [window setOpaque:NO];
  [window setHasShadow:true];
  [window setBackgroundColor:[NSColor clearColor]];
  [window setContentMaxSize:NSMakeSize(MAX_WIDTH, MAX_HEIGHT)];
  [window setContentMinSize:NSMakeSize(MIN_WIDTH, MIN_HEIGHT)];
  webView = [[LoqurWebView alloc] initWithFrame:[window frame]];
  webView.autoresizesSubviews = YES;
  [window setDelegate:(id) [WindowDelegate alloc]];
  NSURL *url = [NSURL URLWithString:@"https://linux-dev:8000"];
  [NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
  [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];

  [window setContentView:webView];

  while ([webView isLoading]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [webView setNeedsDisplay:NO];
    [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:1.0] inMode:NSDefaultRunLoopMode dequeue:YES];
    [pool drain];
  }
  [window invalidateShadow];
  [webView setNeedsDisplay:true];
  [[NSUserDefaults standardUserDefaults] setBool:true forKey:@"WebKitDeveloperExtras"];
  [[NSUserDefaults standardUserDefaults] synchronize];
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
