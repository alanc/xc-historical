.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 12
.nr PS 12
\&
.sp 8
.ce 1
\s+2\fBX11 NON RECTANGULAR WINDOW SHAPE EXTENSION\fP\s-2
.sp 3
.ce 1
Version 1.0
.sp 6
.ce 5
\s-1Keith Packard
.sp 6p
Massachusetts Institute of Technology
Laboratory for Computer Science\s+1
.bp
\&
.ps 9
.nr PS 9
.sp 8
.LP
Copyright \(co 1989
Massachusetts Institute of Technology
.LP
This document is for internal use by Member and Affiliate organizations
of the MIT X Consortium.  It may be redistributed internally within such
organizations, provided the above copyright notice and this permission
notice appear in all copies, but it is not to be published and it is not
to be redistributed externally.  MIT
makes no representations about the suitability for any purpose of the
information in this document.  It is provided ``as is'' without express or
implied warranty.  The information in this document is a preliminary
draft and therefore subject to change, and does not represent an
approved specification of the MIT X Consortium.
.ps 12
.nr PS 12
.bp 1
.EH ''X11 Non Rectangular Window Shape Extension Proposal''
.OH ''X11 Non Rectangular Window Shape Extension Proposal''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Overview
.LP
This extension provides arbitrary window and border shapes within the X11
protocol.
.LP
The restriction of rectangular windows within the X protocol is a significant
limitation in the implementation of many user interface styles.  For example,
many transient windows would like to display a ``drop shadow'' to give the
illusion of 3 dimensions, and it may be desirable to ``stipple'' this
shadow.  As another example, some user interface style guides call for buttons
with rounded corners; the full simulation of a non-rectangular shape,
particularly with respect to event distribution and cursor shape, is not
possible within the core X protocol.  As a final example, round clocks
and non-rectangular icons are desirable visual addition to the desktop.
.LP
This extension provides mechanisms for changing the visible shape of a
window to an arbitrary non-rectangular form, including a form composed
of disjoint pieces.  The intent of the extension is to supplement the
existing semantics, not replace them.  In particular, it is desirable
for clients which are unaware of the extension to still be able to cope
reasonably with shaped windows.  For example, window managers should still
be able to negotiate screen real-estate in rectangular chunks.  Toward this
end, any shape specified for a window is clipped by the bounding rectangle for
the window as specified by the window's geometry in the core protocol.
An expected convention would be that client programs expand their shape
to fill the area offered by the window manager.
.NH 1
Description
.LP
Each window (even with no shapes specified) is defined by two regions:  the
bounding region and the clip region.  The bounding region is the area of the
parent window which the window will occupy (including border). The clip region
is the subset of the bounding region which is available for sub-windows and
graphics.  The area between the bounding region and the clip region is defined
to be the border of the window.
.LP
A non-shaped window will have a bounding region which is a rectangle
spanning the window including its border; the clip region will be a rectangle
filling the inside dimensions (not including the border).  These default
regions will be referred to later as the default bounding and clip regions.
For a window with inside size given by \fIwidth\fP and \fIheight\fP and
border width given by \fIbwidth\fP, the default bounding and clip regions are
the rectangles (relative to the window origin):
.IP
bounding.x = - \fIbwidth\fP
.br
bounding.y = - \fIbwidth\fP
.br
bounding.width = \fIwidth\fP + 2 * \fIbwidth\fP
.br
bounding.height = \fIheight\fP + 2 * \fIbwidth\fP
.sp
clip.x = 0
.br
clip.y = 0
.br
clip.width = \fIwidth\fP
.br
clip.height = \fIheight\fP
.LP
To shape a window, one or two regions are specified by a client through the
extension.  These regions are called client bounding region and the client
clip region.  These regions are specified relative to the origin of
the window, and are always defined by offsets relative to the window origin
(that is, region adjustments are not required when the window is moved).
Three mechanisms for specifying regions are included in the extension:  a list
of rectangles; a bitmap; and an existing bounding or clip region from a window.
This is consistent with the specification of regions in graphics contexts in
the core protocol, and allows a variety of different uses of the extension.
.LP
If a client bounding region has not been explicitly set, the client bounding
region is defined (dynamically) to be the current default bounding region.
Similarly, if a client clip region has not been explicitly set, the client
clip region is defined (dynamically) to be the current default clip region.
.LP
The actual bounding region of a window is defined to be the intersection of
the client bounding region with the default bounding region.  Any portion of
the client bounding region which is not included in the default bounding
region will not be included in the actual bounding region on the screen.
This means that window managers (or other geometry managers) used to dealing
with rectangular client windows will be able to constrain the client to a
rectangular area of the screen.
.LP
Construction of the actual bounding region is dynamic; the client bounding
region is not mutated to obtain the actual bounding region.  If a client
bounding region is specified which extends beyond the current default bounding
region, and the window is later enlarged, the actual bounding region will
enlarge to include more of the client bounding region.
.LP
The actual clip region of a window is defined to be the intersection of the
client clip region with both the default clip region and the client bounding
region.  Any portion of the client clip region which is not included in both
the default clip region and the client bounding region will not be included in
the actual clip region on the screen.
.LP
Construction of the actual clip region is dynamic; the client clip region is
not mutated to obtain the actual clip region.  If a client clip region is
specified which extends beyond the current default clip region, and the window
or its bounding region is later enlarged, the actual clip region will enlarge
to include more of the client clip region if it is included in the actual
bounding region.
.LP
The actual border of a window is defined to be the difference between the
actual bounding region and the actual clip region.  If this region is empty,
no border is displayed.  If this region is non-empty, the border is filled
as usual using the border-tile or border-pixel of the window.  The border
tile origin is precisely as defined in the core protocol.  Note that a window
with non-zero border width will never be able to draw beyond the default clip
region of the window.  Also note that a zero border width does not prevent
a window from having a border, since the clip shape can still be made smaller
than the bounding shape.
.LP
All output to the window will be clipped to the actual clip region, and
visibility of any subwindows will also be clipped to the actual clip region.
The server must not retain window contents beyond the actual clip region with
backing store.  The window's origin (for graphics operations, background
tiling, and subwindow placement) is not affected by the existence of a
bounding region or clip region.
.LP 
Areas which are inside the default bounding region but outside the actual
bounding region are not part of the window; these areas of the screen will
be occupied by other windows.  Input events which occur within the default
bounding region but outside the actual bounding region will be delivered as
if the window was not occluding the event position.  Events which occur in
a non-rectangular border of a window will be delivered to that window, just
as for events which occur in a normal rectangular border.
.LP
An InputOnly window can have its bounding region set, but it is a
.PN Match
error to attempt to set a clip region on an InputOnly window, or to
specify its clip region as a source to a request in this extension.
.NH 1
Types
.LP
.IN "Types" "SHAPE_KIND" "@DEF@"
SHAPE_KIND:
.Pn { Bounding ,
.PN Clip }
.LP
.IN "Types" "SHAPE_OP" "@DEF@"
SHAPE_OP:
.Pn { ShapeSet ,
.PN ShapeUnion ,
.PN ShapeIntersect ,
.PN ShapeSubtract ,
.PN ShapeInvert }
.LP
.PN ShapeSet
means that the region specified as an explicit source in the request is stored
unaltered as the new destination client region.
.PN ShapeUnion
means that the source and destination regions are unioned together to produce
the new destination client region.
.PN ShapeIntersect
means that the source and destination regions are intersected together to
produce the new destination client region.
.PN ShapeSubtract
means that the source region is subtracted from the destination region to
produce the new destination region.
.PN ShapeInvert
means that the destination region is subtracted from the source region to
produce the new destination region.
.NH 1
Events
.LP
.IN "ShapeNotify" "" "@DEF@"
.PN "ShapeNotify"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIkind\fP\^: SHAPE_KIND
.br
\fIshaped\fP\^: BOOL
.br
\fIx\fP, \fIy\fP\^: INT16
.br
\fIwidth\fP, \fIheight\fP\^: CARD16
.br
\fItime\fP\^: TIMESTAMP
.IP
Whenever the client bounding or clip shape of a window is modified, a
.PN ShapeNotify
event is sent to each client which has used
.PN ShapeSelectInput
to request it.
.IP
\fIkind\fP indicates the affected region of the window.
\fIshaped\fP is
.PN True
when the window has a shape of type \fIkind\fP.
\fIx\fP, \fIy\fP, \fIwidth\fP and \fIheight\fP indicate the extents of the
current shape.  When \fIshaped\fP is
.PN False
these will indicate the extents of the default region.  The timestamp
indicates the server time when the shape was changed.
.NH 1
Requests
.LP
.IN "ShapeQueryVersion" "" "@DEF@"
.PN "ShapeQueryVersion"
.LP
   =>
.IP
\fImajorVersion\fP\^: CARD16
.br
\fIminorVersion\fP\^: CARD16
.IP
This request can be used to ensure that the server version of the SHAPE
extension is usable by the client.  This document defines  major version one,
minor version zero.
.LP
.IN "ShapeRectangles" "" "@DEF@"
.PN "ShapeRectangles"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.br
\fIordering\fP\^: {UnSorted, YSorted, YXSorted, YXBanded}
.IP
Errors:
.PN Window ,
.PN Length ,
.PN Match ,
.PN Value
.IP
This request specifies an array of rectangles, relative to the origin of the
window plus the specified offset (\fIxOff\fP and \fIyOff\fP) which together
define a region.  This region is combined (as specified by the operator
\fIop\fP) with the existing client region (specified by \fIdestKind\fP) of the
destination window, and the result is stored as the specified client region of
the destination window.
.IP
If known by the client, 
ordering relations on the rectangles can be specified with the ordering 
argument.
This may provide faster operation by the server.
If an incorrect ordering is specified, 
the server may generate a 
.PN Match 
error, but it is not required to do so.
If no error is generated,
the graphics results are undefined.
Except for
.PN UnSorted ,
the rectangles should be nonintersecting, or the resulting region will
be undefined.
.PN UnSorted 
means that the rectangles are in arbitrary order.
.PN YSorted 
means that the rectangles are nondecreasing in their Y origin.
.PN YXSorted 
additionally constrains 
.PN YSorted 
order in that all rectangles with an equal Y origin are
nondecreasing in their X origin.
.PN YXBanded 
additionally constrains 
.PN YXSorted 
by requiring that, for every possible Y scanline,
all rectangles that include that scanline have identical Y origins and Y
extents.
.LP
.IN "ShapeMask" "" "@DEF@"
.PN "ShapeMask"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIsource\fP\^: PIXMAP or None
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match ,
.PN Value
.IP
The source in this request is a 1-bit deep pixmap, or
.PN None .
If \fIsource\fP is 
.PN None ,
the specified client region is removed
from the window (and hence reverts to the default region).
ShapeNotify events and ShapeQuery replies will indicate this fact.
Otherwise, the pixmap is converted to a region, with bits set to one
included in the region and bits set to zero excluded, and offset
as specified by \fIxOff\fP and \fIyOff\fP.  The resulting region is then
combined (as specified by the operator \fIop\fP) with the existing client
region (specified by \fIdestKind\fP) of the destination window, and the result
is stored as the specified client region of the destination window.
.LP
.IN "ShapeCombine" "" "@DEF@"
.PN "ShapeCombine"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIsource\fP\^: WINDOW
.br
\fIsourceKind\fP\^: SHAPE_KIND
.IP
Errors:
.PN Window ,
.PN Match ,
.PN Value
.IP
The client region (specified by \fIsourceKind\fP) of the source window,
offset as specified by \fIxOff\fP and \fIyOff\fP), is combined with the
client region (specified by \fIdestKind\fP) of the destination window,
and the result is stored as the specified client region of the destination
window.
.LP
.IN "ShapeOffset" "" "@DEF@"
.PN "ShapeOffset"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIxOff, yOff\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Match ,
.PN Value
.IP
The client region (specified by \fIdestKind\fP) is moved by the specified
amounts (\fIxOff\fP and \fIyOff\fP).
.LP
.IN "ShapeQueryExtents" "" "@DEF@"
.PN "ShapeQueryExtents"
.IP
\fIdest\fP\^: WINDOW
.LP
   =>
.IP
\fIboundingShaped\fP\^: BOOL
.br
\fIclipShaped\fP\^: BOOL
.br
\fIxBoundingShape\fP\^: INT16
.br
\fIyBoundingShape\fP\^: INT16
.br
\fIwidthBoundingShape\fP\^: CARD16
.br
\fIheightBoundingShape\fP\^: CARD16
.br
\fIxClipShape\fP\^: INT16
.br
\fIyClipShape\fP\^: INT16
.br
\fIwidthClipShape\fP\^: CARD16
.br
\fIheightClipShape\fP\^: CARD16
.IP
Errors:
.PN Window
.IP
\fIboundingShaped\fP is
.PN True
if a client bounding region has been specified; \fIclipShaped\fP is
.PN True
if a client clip region has been specified.
The x, y, width, and height values define the extents of the client region;
when a client region has not been specified, the extents of the default
region are reported.
.LP
.IN "ShapeSelectInput" "" "@DEF@"
.PN "ShapeSelectInput"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIenable\fP\^: BOOL
.IP
Errors:
.PN Window ,
.PN Value
.IP
Specifying \fIenable\fP as
.PN True
causes the server, in the future, to send the requesting client a
.PN ShapeNotify
event whenever the bounding or clip region of the specified window is
altered (by any client).
Specifying \fIenable\fP as
.PN False
causes the server to stop sending such event notifications.
.LP
.IN "ShapeInputSelected" "" "@DEF@"
.PN "ShapeInputSelected"
.IP
\fIwindow\fP\^: WINDOW
.LP
   =>
.IP
\fIenable\fP\^: BOOL
Errors:
.PN Window
.IP
This returns a boolean indicating whether the server will send ShapeNotify
events to the requesting client when regions of the specified window
are altered.
.LP
.IN "ShapeGetRectangles" "" "@DEF@"
.PN "ShapeGetRectangles"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIkind\fP\^: SHAPE_KIND
.LP
   =>
.IP
rectangles: LISTofRECTANGLE
.IP
Errors:
.PN Window,
.PN Match
.IP
This request returns the client region (specified by \fIkind\fP) as a list of
nonintersecting rectangles.  The order of the rectangles is unspecified.
.NH 1
Encoding
.LP
Please refer to the X11 Protocol Encoding document as this document uses
conventions established there.
.LP
.SH
\s+2New types\s-2
.LP
.Ds 0
.TA .75i 1.75i
.ta .75i 1.75i
.R
SHAPE_KIND
	0	Bounding
	1	Clip
.De
.LP
.Ds 0
.TA .75i 1.75i
.ta .75i 1.75i
.R
SHAPE_OP
	0	ShapeSet
	1	ShapeUnion
	2	ShapeIntersect
	3	ShapeSubtract
	4	ShapeInvert
.De
.SH
\s+2Requests\s-2
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeQueryVersion
	1	CARD8		opcode
	1	0		shape opcode
	2	1		request length
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
 	1	1		Reply
	1			unused
	2	CARD16		sequence number
	4	0		length
	2	CARD16		major version
	2	CARD16		minor version
	20			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeRectangles
	1	CARD8		opcode
	1	1		shape opcode
	2	4+2n		request length
	1	SHAPE_OP	operation
	1	SHAPE_KIND	destination kind
	1			ordering
		0	UnSorted
		1	YSorted
		2	YXSorted
		3	YXBanded
	1			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	8n	LISTofRECTANGLE		rectangles
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeMask
	1	CARD8		opcode
	1	2		shape opcode
	2	5		request length
	1	SHAPE_OP	operation
	1	SHAPE_KIND	destination kind
	2			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	4	PIXMAP		source bitmap
		0	None
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeCombine
	1	CARD8		opcode
	1	3		shape opcode
	2	5		request length
	1	SHAPE_OP	operation
	1	SHAPE_KIND	destination kind
	1	SHAPE_KIND	source kind
	1			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	4	WINDOW		source window
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeOffset
	1	CARD8		opcode
	1	4		shape opcode
	2	4		request length
	1	SHAPE_KIND	destination kind
	3			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeQueryExtents
	1	CARD8		opcode
	1	5		shape opcode
	2	2		request length
	4	WINDOW		destination window
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1			unused
	2	CARD16		sequence number
	4	0		reply length
	1	BOOL		bounding shaped
	1	BOOL		clip shaped
	2			unused
	2	INT16		bounding shape extents x
	2	INT16		bounding shape extents y
	2	CARD16		bounding shape extents width
	2	CARD16		bounding shape extents height
	2	INT16		clip shape extents x
	2	INT16		clip shape extents y
	2	CARD16		clip shape extents width
	2	CARD16		clip shape extents height
	4			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeSelectInput
	1	CARD8		opcode
	1	6		shape opcode
	2	3		request length
	4	WINDOW		destination window
	1	BOOL		enable
	3			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeInputSelected
	1	CARD8		opcode
	1	6		shape opcode
	2	2		request length
	4	WINDOW		destination window
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1	BOOL		enabled
	2	CARD16		sequence number
	4	0		reply length
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeGetRectangles
	1	CARD8		opcode
	1	7		shape opcode
	2	3		request length
	4	WINDOW		window
	1	SHAPE_KIND	Bounding or Clip
	3			unused
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1			unused
	2	CARD16		sequence number
	4	8n		reply length
	4	CARD32		nrects
	20			unused
	8n	LISTofRECTANGLE		rectangles
.De
.SH
\s+2Events\s-2
.LP
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeNotify
	1	CARD8		type (0 + extension event base)
	1	SHAPE_KIND	Bounding or Clip
	2	CARD16		sequence number
	4	WINDOW		affected window
	2	INT16		x value of extents
	2	INT16		y value of extents
	2	CARD16		width of extents
	2	CARD16		height of extents
	4	TIMESTAMP	server time
	1	BOOL		shaped
	11			unused
.De
.NH 1
Xlib compatible C language Binding
.LP
Please refer to the protocol portion of the document for a description of
the semantics of these routines.  They are a simple veneer over the protocol
and perform no magic.
.LP
Bool
.br
XShapeQueryExtension (display)
.RS
Display	*display;
.RE
.IP
Returns True if the specified display supports the SHAPE extension, False
otherwise.
.LP
int
.br
XShapeGetEventBase (display)
.RS
Display *display;
.RE
.IP
Returns the first event number that this extension will generate from
the specified display.  Use this to gather events from the display.
.LP
Bool
.br
XShapeQueryVersion (display, major_version, minor_version)
.RS
Display *display;
.br
int *major_version, *minor_version;  /* RETURN */
.RE
.IP
Returns the major and minor version numbers for the specified display.
.LP
XShapeCombineRectangles (display, dest, dest_kind, x_off, y_off, rectangles, n_rects, op, ordering)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, n_rects, op, x_off, y_off, ordering;
.br
XRectangle *rectangles;
.RE
.LP
XShapeCombineRegion (display, dest, dest_kind, x_off, y_off, region, op)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off;
.br
REGION *region;
.RE
.IP
Converts the region into a list of rectangles and calls XShapeRectangles.
.LP
XShapeCombineMask (display, dest, dest_kind, x_off, y_off, src, op)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off;
.br
Pixmap	src;
.RE
.LP
XShapeCombineShape (display, dest, dest_kind, x_off, y_off, src, src_kind, op)
.RS
Display *display;
.br
Window dest, src;
.br
int dest_kind, src_kind, op, x_off, y_off;
.RE
.LP
XShapeOffsetShape (display, dest, dest_kind, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, x_off, y_off;
.RE
.LP
XShapeQueryExtents
'in +1i
(display, window, w_shaped, xws, yws, wws, hws, b_shaped, xbs, ybs, wbs, hbs)    
'in 0
.RS
Display *display;
.br
Window window;
.br
Bool *w_shaped, *b_shaped; /* RETURN */
.br
int *xws, *yws, *xbs, *ybs; /* RETURN */
.br
unsigned int *wws, *hws, *wbs, *hbs; /* RETURN */
.RE
.IP
xws, yws, wws, hws are the extents of the window shape.
xbs, ybs, wbs, hbs are the extents of the border shape.
.LP
XShapeSelectInput (display, window, enable)
.RS
Display *display;
.br
Window window;
.br
Bool enable;
.RE
.LP
Bool
.br
XShapeInputSelected (display, window)
.RS
Display *display
.br
Window window;
.RE
.LP
XRectangle *
.br
XShapeGetRectangles (display, window, kind, count)
.RS
Display *display;
.br
Window window;
.br
int kind;
.br
int *count; /* RETURN */
.RE
.LP
