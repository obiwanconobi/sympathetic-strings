/*
 *	File:		SympatheticView.mm
 *
 *	Version:	1.0
 *
 *	Copyright:  Copyright (c) 2026 Panaudio, Panaudio uses the MIT license
 *
 *	Custom AUv2 Cocoa view for Sympathetic.
 *
 *	The palette is drawn from a sitar: tun-wood browns, a warm amber/copper
 *	sheen, and ivory inlay. It is rendered flat and dark with a single thin
 *	"string" motif, so it stays subtle and modern rather than skeuomorphic.
 */

#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AUCocoaUIView.h>

// ---------------------------------------------------------------------------
// palette
// ---------------------------------------------------------------------------

static NSColor *Col(unsigned int hex, CGFloat alpha)
{
	return [NSColor colorWithSRGBRed:((hex >> 16) & 0xFF) / 255.0
							   green:((hex >> 8) & 0xFF) / 255.0
								blue:(hex & 0xFF) / 255.0
							   alpha:alpha];
}

#define kBgTop      0x2B1E18
#define kBgBottom   0x150F0D
#define kEdge       0x4A382F
#define kTitle      0xF2E8D5
#define kSubtitle   0xA88F78
#define kLabel      0xDCCBB2
#define kTrack      0x33241E
#define kTrackEdge  0x4A372D
#define kFillDark   0x8A4A24
#define kFillLight  0xE3A85C
#define kThumb      0xF3E5C9
#define kThumbEdge  0x6B4A2E
#define kAccent     0xC98A3B
#define kPillBg     0x2E211B

// ---------------------------------------------------------------------------
// geometry
// ---------------------------------------------------------------------------

static const CGFloat kViewW   = 470.0;
static const CGFloat kViewH   = 392.0;
static const CGFloat kPadX    = 30.0;
static const CGFloat kRowH    = 40.0;
static const CGFloat kLabelW  = 100.0;
static const CGFloat kValueW  = 54.0;
static const CGFloat kSliderY = 110.0;

static NSString *const kParamNames[5] = { @"Resonance", @"Sustain", @"Damping", @"Detune", @"Dry/Wet" };
static const int       kParamIDs[5]   = { 0, 1, 2, 3, 5 };
static NSString *const kBankNames[4]  = { @"Standard", @"Twelve", @"DADGAD", @"Dm drone" };

// ===========================================================================
// SympatheticView
// ===========================================================================

@interface SympatheticView : NSView
- (instancetype)initWithAudioUnit:(AudioUnit)au;
@end

@implementation SympatheticView {
	AudioUnit _au;
	float     _values[6];
	BOOL      _dragging;
	int       _activeSlider;
	int       _activeBank;
	NSTimer  *_timer;
}

- (instancetype)initWithAudioUnit:(AudioUnit)au
{
	self = [super initWithFrame:NSMakeRect(0, 0, kViewW, kViewH)];
	if (self) {
		_au = au;
		_activeSlider = -1;
		_activeBank = -1;
		for (int i = 0; i < 6; i++) _values[i] = 0.0f;
		_values[1] = 0.5f;
		_values[2] = 0.5f;
		_values[3] = 0.4f;
		_values[0] = 0.5f;
		_values[5] = 1.0f;
		[self refreshFromHost];

		__weak __typeof(self) weakSelf = self;
		_timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 30.0
												  repeats:YES
													block:^(NSTimer *t) { [weakSelf refreshFromHost]; }];
	}
	return self;
}

- (void)dealloc
{
	[_timer invalidate];
}

- (BOOL)isFlipped { return YES; }
- (BOOL)acceptsFirstMouse:(NSEvent *)event { return YES; }

// -- parameter plumbing ----------------------------------------------------

- (void)refreshFromHost
{
	if (_au == NULL || _dragging) return;
	BOOL changed = NO;
	for (int i = 0; i < 6; i++) {
		AudioUnitParameterValue v = 0.0f;
		if (AudioUnitGetParameter(_au, (AudioUnitParameterID)i, kAudioUnitScope_Global, 0, &v) == noErr) {
			if (fabsf(v - _values[i]) > 1e-5f) { _values[i] = v; changed = YES; }
		}
	}
	if (changed) [self setNeedsDisplay:YES];
}

- (void)sendParameter:(int)paramID value:(float)value
{
	if (_au != NULL)
		AudioUnitSetParameter(_au, (AudioUnitParameterID)paramID, kAudioUnitScope_Global, 0, value, 0);
}

// -- geometry --------------------------------------------------------------

- (CGFloat)trackX1 { return kPadX + kLabelW + 10.0; }
- (CGFloat)trackX2 { return kViewW - kPadX - kValueW - 12.0; }
- (CGFloat)trackW  { return [self trackX2] - [self trackX1]; }
- (CGFloat)contentRight { return kViewW - kPadX; }

- (int)sliderIndexAtPoint:(NSPoint)p
{
	for (int i = 0; i < 5; i++) {
		CGFloat y = kSliderY + i * kRowH;
		if (p.y >= y && p.y < y + kRowH &&
			p.x >= [self trackX1] - 14.0 && p.x <= [self trackX2] + 14.0)
			return i;
	}
	return -1;
}

- (int)bankIndexAtPoint:(NSPoint)p
{
	CGFloat gap = 8.0;
	CGFloat pw = ([self contentRight] - kPadX - 3.0 * gap) / 4.0;
	if (p.y < 346.0 || p.y > 378.0) return -1;
	for (int i = 0; i < 4; i++) {
		CGFloat x = kPadX + i * (pw + gap);
		if (p.x >= x && p.x <= x + pw) return i;
	}
	return -1;
}

// -- drawing ---------------------------------------------------------------

- (void)drawStringLineFrom:(NSPoint)a to:(NSPoint)b alpha:(CGFloat)alpha
{
	NSBezierPath *line = [NSBezierPath bezierPath];
	[line moveToPoint:a];
	[line lineToPoint:b];
	line.lineWidth = 1.0;
	[Col(kFillLight, alpha) setStroke];
	[line stroke];
}

- (void)drawRect:(NSRect)dirtyRect
{
	NSRect bounds = self.bounds;

	// panel
	NSBezierPath *panel = [NSBezierPath bezierPathWithRoundedRect:bounds xRadius:10 yRadius:10];
	[NSGraphicsContext saveGraphicsState];
	[panel addClip];
	NSGradient *bg = [[NSGradient alloc] initWithStartingColor:Col(kBgTop, 1.0)
												  endingColor:Col(kBgBottom, 1.0)];
	[bg drawInRect:bounds angle:90];
	[NSGraphicsContext restoreGraphicsState];

	// faint sympathetic-string motif behind everything
	for (int i = 0; i < 5; i++) {
		CGFloat y = 58.0 + i * 74.0;
		[self drawStringLineFrom:NSMakePoint(10, y) to:NSMakePoint(kViewW - 10, y) alpha:0.018];
	}

	// soft top highlight so the panel reads as a crafted surface
	NSGradient *topGrad = [[NSGradient alloc] initWithColors:@[
		Col(kTitle, 0.0), Col(kTitle, 0.07), Col(kTitle, 0.0)
	]];
	[topGrad drawInBezierPath:[NSBezierPath bezierPathWithRect:NSMakeRect(12, 1, kViewW - 24, 1)]
						angle:0];

	// single highlighted "string" just under the header
	NSGradient *stringGrad = [[NSGradient alloc] initWithColors:@[
		Col(kFillDark, 0.0), Col(kFillLight, 0.55), Col(kFillDark, 0.0)
	]];
	NSBezierPath *stringClip = [NSBezierPath bezierPathWithRect:NSMakeRect(kPadX, 96, [self contentRight] - kPadX, 2)];
	[stringGrad drawInBezierPath:stringClip angle:0];

	// title / subtitle
	NSMutableParagraphStyle *titleStyle = [[NSMutableParagraphStyle alloc] init];
	titleStyle.alignment = NSTextAlignmentLeft;
	NSDictionary *titleAttrs = @{
		NSFontAttributeName: [NSFont systemFontOfSize:16.0 weight:NSFontWeightSemibold],
		NSForegroundColorAttributeName: Col(kTitle, 1.0),
		NSKernAttributeName: @2.6,
		NSParagraphStyleAttributeName: titleStyle
	};
	[@"SYMPATHETIC" drawAtPoint:NSMakePoint(kPadX, 30) withAttributes:titleAttrs];

	NSDictionary *subAttrs = @{
		NSFontAttributeName: [NSFont systemFontOfSize:11.0 weight:NSFontWeightRegular],
		NSForegroundColorAttributeName: Col(kSubtitle, 1.0),
		NSKernAttributeName: @0.4
	};
	[@"sitar sympathetic-string resonator" drawAtPoint:NSMakePoint(kPadX + 0.5, 58) withAttributes:subAttrs];

	// sliders
	for (int i = 0; i < 5; i++) [self drawSliderAtIndex:i];

	// bank
	[self drawBank];

	// border
	[NSGraphicsContext saveGraphicsState];
	NSBezierPath *border = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect(bounds, 0.5, 0.5)
														   xRadius:10 yRadius:10];
	border.lineWidth = 1.0;
	[Col(kEdge, 0.55) setStroke];
	[border stroke];
	[NSGraphicsContext restoreGraphicsState];
}

- (void)drawSliderAtIndex:(int)index
{
	CGFloat y = kSliderY + index * kRowH;
	CGFloat cy = y + 20.0;
	CGFloat x1 = [self trackX1];
	CGFloat tw = [self trackW];
	float v = _values[kParamIDs[index]];

	// label
	NSDictionary *labelAttrs = @{
		NSFontAttributeName: [NSFont systemFontOfSize:12.5 weight:NSFontWeightMedium],
		NSForegroundColorAttributeName: Col(kLabel, 1.0)
	};
	[kParamNames[index] drawInRect:NSMakeRect(kPadX, y + 8.0, kLabelW, 24.0)
					withAttributes:labelAttrs];

	// track
	NSBezierPath *track = [NSBezierPath bezierPathWithRoundedRect:NSMakeRect(x1, cy - 3.0, tw, 6.0)
														 xRadius:3 yRadius:3];
	[Col(kTrack, 1.0) setFill];
	[track fill];
	[Col(kTrackEdge, 0.9) setStroke];
	track.lineWidth = 1.0;
	[track stroke];

	// fill
	CGFloat fx = x1 + v * tw;
	if (fx - x1 > 1.5) {
		NSBezierPath *fill = [NSBezierPath bezierPathWithRoundedRect:NSMakeRect(x1, cy - 3.0, fx - x1, 6.0)
															 xRadius:3 yRadius:3];
		NSGradient *fillGrad = [[NSGradient alloc] initWithStartingColor:Col(kFillDark, 1.0)
															endingColor:Col(kFillLight, 1.0)];
		[fillGrad drawInBezierPath:fill angle:0];
	}

	// thumb
	NSRect thumbRect = NSMakeRect(fx - 8.0, cy - 8.0, 16.0, 16.0);
	NSBezierPath *thumb = [NSBezierPath bezierPathWithOvalInRect:thumbRect];
	NSShadow *shadow = [[NSShadow alloc] init];
	shadow.shadowBlurRadius = 5.0;
	shadow.shadowOffset = NSMakeSize(0, 1.5);
	shadow.shadowColor = Col(0x000000, 0.55);
	[NSGraphicsContext saveGraphicsState];
	[shadow set];
	[Col(kThumb, 1.0) setFill];
	[thumb fill];
	[NSGraphicsContext restoreGraphicsState];
	[Col(kThumbEdge, 1.0) setStroke];
	thumb.lineWidth = 1.0;
	[thumb stroke];

	// value
	NSMutableParagraphStyle *right = [[NSMutableParagraphStyle alloc] init];
	right.alignment = NSTextAlignmentRight;
	NSDictionary *valueAttrs = @{
		NSFontAttributeName: [NSFont monospacedDigitSystemFontOfSize:11.5 weight:NSFontWeightRegular],
		NSForegroundColorAttributeName: Col(kAccent, 1.0),
		NSParagraphStyleAttributeName: right
	};
	NSString *value = [NSString stringWithFormat:@"%.0f%%", v * 100.0f];
	[value drawInRect:NSMakeRect([self contentRight] - kValueW, y + 9.0, kValueW, 22.0)
	   withAttributes:valueAttrs];
}

- (void)drawBank
{
	NSDictionary *headingAttrs = @{
		NSFontAttributeName: [NSFont systemFontOfSize:10.0 weight:NSFontWeightSemibold],
		NSForegroundColorAttributeName: Col(kSubtitle, 1.0),
		NSKernAttributeName: @1.8
	};
	[@"BANK" drawAtPoint:NSMakePoint(kPadX + 0.5, 324) withAttributes:headingAttrs];

	int selected = (int)(_values[4] + 0.5f);
	if (selected < 0) selected = 0;
	if (selected > 3) selected = 3;

	CGFloat gap = 8.0;
	CGFloat totalW = [self contentRight] - kPadX;
	CGFloat pw = (totalW - 3.0 * gap) / 4.0;

	for (int i = 0; i < 4; i++) {
		NSRect r = NSMakeRect(kPadX + i * (pw + gap), 346.0, pw, 32.0);
		NSBezierPath *pill = [NSBezierPath bezierPathWithRoundedRect:r xRadius:8 yRadius:8];

		BOOL on = (i == selected);
		if (on) {
			NSGradient *g = [[NSGradient alloc] initWithStartingColor:Col(kFillDark, 1.0)
														endingColor:Col(kFillLight, 1.0)];
			[g drawInBezierPath:pill angle:0];
		} else {
			[Col(kPillBg, 1.0) setFill];
			[pill fill];
			[Col(kTrackEdge, 0.9) setStroke];
			pill.lineWidth = 1.0;
			[pill stroke];
		}

		NSMutableParagraphStyle *center = [[NSMutableParagraphStyle alloc] init];
		center.alignment = NSTextAlignmentCenter;
		NSDictionary *attrs = @{
			NSFontAttributeName: [NSFont systemFontOfSize:12.0 weight:(on ? NSFontWeightSemibold : NSFontWeightMedium)],
			NSForegroundColorAttributeName: on ? Col(0x2A1A12, 1.0) : Col(kLabel, 0.85),
			NSParagraphStyleAttributeName: center
		};
		[kBankNames[i] drawInRect:NSMakeRect(r.origin.x, r.origin.y + 8.0, r.size.width, 18.0)
				   withAttributes:attrs];
	}
}

// -- interaction -----------------------------------------------------------

- (void)setSlider:(int)index fromPoint:(NSPoint)p
{
	CGFloat t = (p.x - [self trackX1]) / [self trackW];
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	int pid = kParamIDs[index];
	_values[pid] = (float)t;
	[self sendParameter:pid value:(float)t];
	[self setNeedsDisplay:YES];
}

- (void)mouseDown:(NSEvent *)event
{
	NSPoint p = [self convertPoint:event.locationInWindow fromView:nil];

	int bank = [self bankIndexAtPoint:p];
	if (bank >= 0) {
		_activeBank = bank;
		[self sendParameter:4 value:(float)bank];
		_values[4] = (float)bank;
		[self setNeedsDisplay:YES];
		return;
	}

	int slider = [self sliderIndexAtPoint:p];
	if (slider >= 0) {
		_dragging = YES;
		_activeSlider = slider;
		[self setSlider:slider fromPoint:p];
	}
}

- (void)mouseDragged:(NSEvent *)event
{
	NSPoint p = [self convertPoint:event.locationInWindow fromView:nil];
	if (_dragging && _activeSlider >= 0) {
		[self setSlider:_activeSlider fromPoint:p];
	} else if (_activeBank >= 0) {
		int bank = [self bankIndexAtPoint:p];
		if (bank >= 0 && bank != _activeBank) {
			_activeBank = bank;
			[self sendParameter:4 value:(float)bank];
			_values[4] = (float)bank;
			[self setNeedsDisplay:YES];
		}
	}
}

- (void)mouseUp:(NSEvent *)event
{
	_dragging = NO;
	_activeSlider = -1;
	_activeBank = -1;
	[self setNeedsDisplay:YES];
}

@end

// ===========================================================================
// SympatheticViewFactory
// ===========================================================================

@interface SympatheticViewFactory : NSObject <AUCocoaUIBase>
@end

@implementation SympatheticViewFactory

- (unsigned)interfaceVersion { return 0; }

- (NSString *)description { return @"Sympathetic View"; }

- (NSView *)uiViewForAudioUnit:(AudioUnit)inAudioUnit withSize:(NSSize)inPreferredSize
{
	(void)inPreferredSize;
	return [[SympatheticView alloc] initWithAudioUnit:inAudioUnit];
}

@end
