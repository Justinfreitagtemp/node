#import <Cocoa/Cocoa.h>
#import <Webkit/Webkit.h>

@interface Window1 : NSWindow
@end
@implementation Window1
- (BOOL)canBecomeMainWindow {
    return YES;
}
- (BOOL)canBecomeKeyWindow {
    return YES;
}
- (void)sendEvent:(NSEvent *)event {
  [super sendEvent:event];
}
@end

@interface Window2 : NSWindow
@end
@implementation Window2
- (BOOL)canBecomeMainWindow {
    return YES;
}
- (BOOL)canBecomeKeyWindow {
    return YES;
}
- (void)sendEvent:(NSEvent *)event {
  [super sendEvent:event];
}
@end

@interface View: WebView
@end
@implementation View
-(void)mouseDown:(NSEvent *)event {
  [super mouseDown:event];
  NSLog(@"down");
}
- (NSView *)hitTest:(NSPoint)point {
  return nil;
}
- (void)drawRect:(NSRect)rect {
  rect.size.height = 50;
  [[NSColor redColor] set];
  NSRectFill(rect);
}
@end

@interface NSURLRequest (DummyInterface)
+ (BOOL)allowsAnyHTTPSCertificateForHost:(NSString*)host;
+ (void)setAllowsAnyHTTPSCertificate:(BOOL)allow forHost:(NSString*)host;
@end

int main(int argc, char *argv[]) {
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  NSUInteger windowStyle = NSBorderlessWindowMask;
  NSWindow *window1 = [[[Window1 alloc]
    initWithContentRect:NSMakeRect(500, 500, 300, 300)
    styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window1 setOpaque:NO];
  [window1 setBackgroundColor:[NSColor clearColor]];
  
  WebView *view1 = [[View alloc] initWithFrame:[window1 frame]];
  [view1 setDrawsBackground:false];
  NSURL *url = [NSURL URLWithString:@"https://linux-dev:8000"];
  [NSURLRequest setAllowsAnyHTTPSCertificate:YES forHost:[url host]];
  [[view1 mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
  [window1 setContentView:view1];

  NSWindow *window2 = [[[Window2 alloc]
    initWithContentRect:NSMakeRect(400,400, 200, 200)
    styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO] autorelease];
  [window2 setOpaque:NO];
  [window2 setBackgroundColor:[NSColor clearColor]];

  //[window2 addChildWindow:window1 ordered:NSWindowAbove];

  [window1 makeKeyAndOrderFront:nil];

  [NSApp activateIgnoringOtherApps:YES];
  [NSApp run];
  return 0;
}
