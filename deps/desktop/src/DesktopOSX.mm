#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>
#include "Desktop.h"

BOOL addSecurityBookmark (NSURL*url);

@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end

@interface LoqurWebView : WebView
- (id)initWithFrame:(NSRect)frameRect;
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems;
- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame *)frame;
@end

@implementation LoqurWebView
- (id)initWithFrame:(NSRect)frameRect {
  self = [super initWithFrame:frameRect];
  self.autoresizingMask = (NSViewHeightSizable | NSViewWidthSizable);
  [[[self mainFrame] frameView] setAllowsScrolling:NO];
  [self setFrameLoadDelegate:self];
  [self setUIDelegate:self];
  [self setMaintainsBackForwardList:NO];
  [self setDrawsBackground:NO];
  WebPreferences* prefs = [self preferences];
  [prefs setAutosaves:NO];
  [prefs setJavaEnabled:NO];
  [prefs setJavaScriptEnabled:YES];
  [prefs setPlugInsEnabled:NO];
  [prefs setPrivateBrowsingEnabled:YES];
  [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
  return self;
}
- (NSArray *)webView:(WebView *)sender contextMenuItemsForElement:(NSDictionary *)element defaultMenuItems:(NSArray *)defaultMenuItems {
  return nil;
}
- (NSUInteger)webView:(WebView *)sender dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo {
  return WebDragDestinationActionDHTML;
}
- (NSUInteger)webView:(WebView *)sender dragSourceActionMaskForPoint:(NSPoint)point {
  return WebDragSourceActionDHTML;
}
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender{
  return NSDragOperationCopy;
}
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  return NSDragOperationEvery;
}
- (void)draggingExited:(id <NSDraggingInfo>)sender {
}
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender  {
  return YES;
}
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
  return YES;
}
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender{
  NSArray *draggedFilenames = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
  for (id filename in draggedFilenames) {
    NSURL *url = [NSURL fileURLWithPath:filename];
    addSecurityBookmark(url);
  }
}
+ (NSString*)webScriptNameForSelector:(SEL)sel {
  if(sel == @selector(logJavaScriptString:))
    return @"log";
  return nil;
}
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
  if(sel == @selector(logJavaScriptString:))
    return NO;
  return YES;
}
- (void)logJavaScriptString:(NSString*) logText {
  NSLog(@"JavaScript: %@",logText);
}
- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame *)frame {
  [windowScriptObject setValue:self forKey:@"Cocoa"];
}
- (void)sendMessage:(NSArray *)args {
  [[self windowScriptObject] callWebScriptMethod:@"populateRepairFields" withArguments:args];
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
 //NSUInteger windowStyle = NSBorderlessWindowMask;
 NSUInteger windowStyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
 id window = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
    styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  WebView *webView;
  webView = [[LoqurWebView alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0, 0, 800, 600))];
  webView.autoresizesSubviews = YES;
  NSString *resourcesPath = [[NSBundle mainBundle] resourcePath];
  NSURL *url = [NSURL URLWithString:@"https://linux-dev:8000"];
  [NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
  [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
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

