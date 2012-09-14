#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#import <ServiceManagement/ServiceManagement.h>
#import "Desktop.h"

#define INITIAL_WIDTH 850
#define INITIAL_HEIGHT 650
#define MAX_WIDTH 1200
#define MAX_HEIGHT 1000
#define MIN_WIDTH 600
#define MIN_HEIGHT 400
#define VERTICAL_MARGIN 100
#define HORIZONTAL_MARGIN 50

BOOL addSecurityBookmark (NSURL*url);
void addFiles();

@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end

@interface WebFrameView (transparent)
@end
@implementation WebFrameView (transparent)
- (BOOL)isOpaque {
  return NO;
}
@end

@interface LoqurWebView : WebView
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
  self.autoresizesSubviews = YES;
  WebPreferences* prefs = [self preferences];
  [prefs setAutosaves:NO];
  [prefs setJavaEnabled:NO];
  [prefs setJavaScriptEnabled:YES];
  [prefs setPlugInsEnabled:NO];
  [prefs setPrivateBrowsingEnabled:YES];
  [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
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
- (NSUInteger)webView:(WebView *)sender
    dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo {
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
  [super draggingExited:sender];
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
  NSArray *draggedFilenames = [[sender draggingPasteboard]
    propertyListForType:NSFilenamesPboardType];
  for (id filename in draggedFilenames) {
    NSURL *url = [NSURL fileURLWithPath:filename];
    addSecurityBookmark(url);
  }
}
- (void)webView:(WebView *)sender didClearWindowObject:
    (WebScriptObject *)windowScriptObject forFrame:(WebFrame *)frame {
  [windowScriptObject setValue:[self window] forKey:@"Window"];
}
@end


@interface LoqurWindow : NSWindow {
  NSInteger move;
  NSPoint moveFrom;
  NSInteger resize;
  WebView *webView;
}
@property (assign) NSInteger move;
@property (assign) NSPoint   moveFrom;
@property (assign) NSInteger resize;
@property (assign) WebView   *webView;
@end

@implementation LoqurWindow
@synthesize move;
@synthesize moveFrom;
@synthesize resize;
@synthesize webView;
- (id)init {
  self = [super initWithContentRect:NSMakeRect(0, 0, INITIAL_WIDTH, INITIAL_HEIGHT)
    styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
  [self center];
  [[self windowController] setShouldCascadeWindows:NO];
  [self setFrameAutosaveName:@"loqurWindow"];
  [self setOpaque:NO];
  [self setHasShadow:NO];
  [self setBackgroundColor:[NSColor clearColor]];
  [self setMaxSize:NSMakeSize(MAX_WIDTH, MAX_HEIGHT)];
  [self setMinSize:NSMakeSize(MIN_WIDTH, MIN_HEIGHT)];
  webView = [[LoqurWebView alloc] initWithFrame:[self frame]];
  [self setDelegate:(id)self];
  NSURL *url = [NSURL URLWithString:@"https://linux-dev:8000"];
  [NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
  [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
  [self setContentView:webView];
  return self;
}
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
    [self initMove:[event locationInWindow]];
    NSPoint locationInView = [webView convertPoint:[event locationInWindow]
      fromView:webView];
    locationInView.y = [self frame].size.height - locationInView.y;
    [self shouldMove: [[self verifyMove:locationInView] intValue]];
  }
  else if (type == NSLeftMouseDragged) {
    if (move) [self performMove];
    else if (resize) [self performResize];
  }
}
- (void)performResize {
  NSInteger type = resize;
  NSRect frame = [self frame];
  NSSize minSize = [self minSize];
  NSSize maxSize = [self maxSize];
  NSPoint currentLocation = [NSEvent mouseLocation];

  int height = frame.size.height;
  int width = frame.size.width;
  float x = frame.origin.x;
  float y = frame.origin.y;

  if (type == 1 || type == 5 || type == 6)
    height = height + (currentLocation.y - (y + height - VERTICAL_MARGIN));
  if (type == 2 || type == 6 || type == 7)
    width = width + (currentLocation.x - (x + width - HORIZONTAL_MARGIN));
  if (type == 3 || type == 7 || type == 8) {
    height = height + (y - currentLocation.y + VERTICAL_MARGIN);
    if (height != frame.size.height) {
      if (height <= maxSize.height && height >= minSize.height)
        y = currentLocation.y - VERTICAL_MARGIN;
      else {
        if (height > maxSize.height)
          y = frame.origin.y - (maxSize.height - frame.size.height);
        else if (height < minSize.height)
          y = frame.origin.y - (minSize.height - frame.size.height);
      }
    }
  }
  if (type == 4 || type == 5 || type == 8) {
    width = width + (x - currentLocation.x + HORIZONTAL_MARGIN);
    if (width != frame.size.width) {
      if (width <= maxSize.width && width >= minSize.width)
        x = currentLocation.x - HORIZONTAL_MARGIN;
      else {
        if (width > maxSize.width)
          x = frame.origin.x - (maxSize.width - frame.size.width);
        else if (width < minSize.width)
          x = frame.origin.x - (minSize.width - frame.size.width);
      }
    }
  }
  if (height < minSize.height) height = minSize.height;
  if (height > maxSize.height) height = maxSize.height;
  if (width < minSize.width) width = minSize.width;
  if (width > maxSize.width) width = maxSize.width;

  frame.size.height = height;
  frame.size.width = width;
  frame.origin.x = x;
  frame.origin.y = y;
  [self setFrame:frame display:YES];
}
- (void)shouldResize:(NSInteger)value {
  resize = value;
}
- (void)initMove:(NSPoint)point {
  self.moveFrom = point;
}
- (void)performMove {
  NSPoint currentLocation;
  NSPoint newOrigin;
  NSRect  screenFrame = [[NSScreen mainScreen] visibleFrame];
  NSRect  windowFrame = [self frame];
  currentLocation = [NSEvent mouseLocation];

  newOrigin.x = currentLocation.x - moveFrom.x;
  newOrigin.y = currentLocation.y - moveFrom.y;
  if ((newOrigin.y + windowFrame.size.height - VERTICAL_MARGIN) >
    (screenFrame.origin.y + screenFrame.size.height))
      newOrigin.y = screenFrame.origin.y +
        (screenFrame.size.height - (windowFrame.size.height - VERTICAL_MARGIN));

  [self setFrameOrigin:newOrigin];
}
- (void)shouldMove:(NSInteger)value {
  move = value;
}
- (id)verifyMove:(NSPoint)point {
  NSArray *args = [NSArray arrayWithObjects:
    [NSNumber numberWithInt:(NSInteger) point.x],
      [NSNumber numberWithInt:(NSInteger) point.y], nil];
  @try {
    return [[webView windowScriptObject]
      callWebScriptMethod:@"verifyMove" withArguments:args];
  }
  @catch (NSException *e) {}
  return [NSNumber numberWithInt:NO];
}
- (void)closeWindow {
  [[NSRunningApplication currentApplication] hide];
}
- (void)minimiseWindow {
  [self miniaturize:self];
}
- (void)maximiseWindow {
  NSRect existingFrame = [self frame];
  NSSize maxSize = [self maxSize];
  int yAdjustment = maxSize.height - existingFrame.size.height;
  NSRect newFrame = NSMakeRect(existingFrame.origin.x,
    existingFrame.origin.y - yAdjustment, maxSize.width, maxSize.height);
  [self setFrame:newFrame display:YES animate:YES];
}
- (void)showFileDialog {
  addFiles();
}
+ (NSString*)webScriptNameForSelector:(SEL)sel {
  if (sel == @selector(shouldMove:))    return @"shouldMove";
  if (sel == @selector(closeWindow))    return @"closeWindow";
  if (sel == @selector(minimiseWindow)) return @"minimiseWindow";
  if (sel == @selector(maximiseWindow)) return @"maximiseWindow";
  if (sel == @selector(showFileDialog)) return @"showFileDialog";
  if (sel == @selector(shouldResize:))  return @"shouldResize";
  return nil;
}
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
  if (sel == @selector(shouldMove:))    return NO;
  if (sel == @selector(closeWindow))    return NO;
  if (sel == @selector(minimiseWindow)) return NO;
  if (sel == @selector(maximiseWindow)) return NO;
  if (sel == @selector(showFileDialog)) return NO;
  if (sel == @selector(shouldResize:))  return NO;
  return YES;
}
- (void)windowDidResignKey:(NSNotification *)notification {
  @try {
    [[webView windowScriptObject] callWebScriptMethod:@"windowLostFocus"
      withArguments:nil];
  }
  @catch (NSException *e) {}
}
- (void)windowDidBecomeKey:(NSNotification *)notification {
  @try {
    [[webView windowScriptObject] callWebScriptMethod:@"windowGainedFocus"
      withArguments:nil];
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
  // int result = [openDlg runModalForDirectory:NSHomeDirectory() file:nil types:nil];
  [openDlg beginWithCompletionHandler:^(NSInteger returnCode) {
    if (returnCode == NSOKButton) {
      for (id url in [openDlg URLs]) {
        addSecurityBookmark(url);
      }
    }
  }];
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
  id statusItem = [[[NSStatusBar systemStatusBar]
    statusItemWithLength:NSSquareStatusItemLength] retain];
  NSString *imageName = [[NSBundle mainBundle] pathForResource:@"default" ofType:@"png"];
  id defaultImage = [[NSImage new] initWithContentsOfFile:imageName];
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
  id window = [[LoqurWindow alloc] init];
  id webView = [window webView];
  while ([webView isLoading]) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [webView setNeedsDisplay:NO];
    [NSApp nextEventMatchingMask:NSAnyEventMask
      untilDate:[NSDate dateWithTimeIntervalSinceNow:1.0f]
        inMode:NSDefaultRunLoopMode dequeue:YES];
    [pool drain];
  }
  [webView setNeedsDisplay:YES];
  [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"WebKitDeveloperExtras"];
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
