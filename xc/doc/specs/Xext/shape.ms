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
Permission to use, copy, modify, and distribute this document 
for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and 
that both that copyright notice and this permission
notice are retained, and that the name of M.I.T. not be used 
in advertising or publicity pertaining to this document without 
specific, written prior permission.
M.I.T. makes no representations about the suitability of this
document or the protocol defined in this document for any purpose.
It is provided ``as is'' without express or implied warranty.
.ps 12
.nr PS 12
.bp 1
.EH ''X11 Non Rectangular Window Shape Extension Proposal''
.OH ''X11 Non Rectangular Window Shape Extension Proposal''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Overview
.XS
\*(SN Overview
.XE
.LP
This extension provides arbitrary window and border shapes within the X11
protocol.
.LP
The restriction of rectangular windows within the X protocol is a severe
limitation in the implementation of many user interface styles.  Many
transient windows would like to display a "shadow" to give the illusion of 3
dimensions, some objects would like to have rounded corners; the simulation
of which can cause disturbing anomolys in event distribution.
.LP
This extension provides mechanisms for changing the visible shape of a
window.  The intent of the extension is to suplement the existing semantics,
not replace them.  Toward this end, any shape specified by the client is
clipped by the bounding rectangle specified by the core protocol.  This
allows window managers and clients to negotiate screen real-estate in
rectangular chunks.
.LP
An expected convention would be that client programs expand their shape
to fill the area offered by the window manager.
.NH 1
Description
.XS
\*(SN Description
.XE
.LP
Each window (even with no shape specified) is defined by two regions:  the
bounding region and the clip region.  The bounding region is the area of the
parent window which the window will occupy. The clip region is the sub-set
of the bounding region which is availible for sub-windows and graphics.  The
window's size affects the extents of the clip region, the window's position
affects the position of the bounding region.  The area between the bounding
region and the clip region is the border of the window.
.LP
A non-shaped window will have a bounding region which is a rectangle
spanning the window including its border; the clip region will be a rectangle
filling the inside dimensions (not including the border).  These default
regions will be refered to later as the default bounding and clip regions.
.LP
To shape a window, one or both of the bounding or clip regions is specified
through the extension by a client.  These are called client bounding or clip
regions.  These regions are specified relative to the origin of the window,
and need not be reinstalled when the window moves.  Three mechanisms for
specifying regions are included in the extension:  A list of rectangles, a
bitmap and a client region from another window.  This allows a variety of
different uses of the extension.
.LP
A client region is not modified when used to shape the window.  This means
that shapes can be applied to windows and combined with other windows
without worrying about the current size of the window.  As such, portions of
this document which refer to the resultant regions will refer to the actual
bounding and clip regions.
.LP
Any portion of the client bounding region which is not included in the default
bounding region will not be included in the actual bounding region on the
screen.  This means that window managers used to dealing with rectangular
client windows will be able to constrain the client to a rectangular area of
the screen.  The actual bounding region is then the intersection of the
default bounding region with the client bounding region.
.LP
Any portion of the client clip region which is not included in the client
bounding region will not be included in the actual clip region on the screen.
.LP
Additionally, any portion of the client clip region which is not included in
the default clip region will not be included in the actual clip region.  In
particular, a window with non-zero border width will never be able to draw
beyond the default clip region of the window.  The actual clip region is
then the intersection of the client clip region, the client bounding region
and the default clip region.
.LP
Any output to the window will be clipped to the actual clip region, any
sub-windows will also be clipped to the actual clip region.  Areas beyond
the actual clip region are not considered a part of the window.  In
particular, the server is expected to not retain window contents beyond the
clip region with backing store.
.LP 
Input events which occur within the default bounding region but outside the
actual bounding region will be delivered as if this window was not occluding
the event position.  Events which occur outside the actual clip region but
inside the actual bounding region will be directed to the border of the
window.
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
.NH 1
Events
.XS
\*(SN Events
.XE
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
Whenever the window or border shape of a window is modified, a
.PN ShapeNotify
event is sent to each client which has used
.PN ShapeSelectInput
to request it.
.IP
\fIkind\fP indicates the affected region of the window.
\fIshaped\fP is TRUE when the window has a shape of type \fIkind\fP.
\fIx\fP, \fIy\fP, \fIwidth\fP and \fIheight\fP indicate the extents of the
current shape.  When \fIshaped\fP is FALSE these will indicate the extents
of the window size. The timestamp indicates the server time when the shape
was changed.
.NH 1
Requests
.XS
\*(SN Requests
.XE
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
extension is usable by the client.
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
.IP
Errors:
.PN Window ,
.PN Length ,
.PN Match ,
.PN Value
.IP
This request specifies an array of rectangles, relative to the origin of the
window plus the specified offset which (together) affect the shape of the
subset of the area offered by the base protocol window/border size of the
drawable region or the border of the specified window.
.IP
The operation specified will be applied to the specified list of rectangles
and any existing shape.  If no shape has been specified yet, the effect will
be equivalent to the case where the shape was set to the single rectangle
which bounds the core-protocol window size.
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
\fIsource\fP\^: BITMAP or NONE
.IP
The source in this request is a 1-bit deep pixmap instead of a list of
rectangles.  If \fIsource\fP is NONE, any existing shape is removed;
ShapeNotify events and ShapeQuery replies will indicate this fact.
Otherwise, the pixmap is converted to a region with areas of value 1
included in the region and ares of value 0 excluded, and the operation
is as described in
.PN ShapeRectangles .
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
The shape of the specified source window/border is applied to the
destination window/border at the specified offset.
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
The specified window/border shape is moved by the specified amount.
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
boundingShaped and clipShaped are TRUE when a shape has been specified.  The
x,y,width,height fields are bounding boxes for the client region.  When a
shape has not been specified, they are computed from the core-protocol
window size and border width.
.LP
.IN "ShapeSelectInput" "" "@DEF@"
.PN "ShapeSelectInput"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIenable\fP\^: BOOL
.IP
When enable is TRUE the server will send the requesting client a
ShapeNotify event whenever the window or border shape of WINDOW is
set.
.LP
.IN "ShapeSelectInput" "" "@DEF@"
.PN "ShapeSelectInput"
.IP
\fIwindow\fP\^: WINDOW
.LP
   =>
.IP
\fIenable\fP\^: BOOL
.IP
This returns whether the server will send ShapeNotify events to
the requesting client when shape changes occur on the specified window.
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
Return the specified shape as a list of non-overlapping rectangles.  The
order of the rectangles is unspecified.
.NH 1
Encoding
.XS
\*(SN Encoding
.XE
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
	2	4+n		request length
	1	SHAPE_OP	operation
	1	SHAPE_KIND	destination kind
	2	CARD16		unused
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
Xlib compatable C language Binding
.XS
\*(SN C Binding
.XE
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
XShapeCombineRectangles (display, dest, dest_kind, rectangles, n_rects, op, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, n_rects, op, x_off, y_off
.br
XRectangle *rectangles
.RE
.LP
XShapeCombineRegion (display, dest, dest_kind, region, op, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off
.br
REGION *region;
.RE
.IP
Converts the region into a list of rectangles and calls XShapeRectangles.
.LP
XShapeCombineMask (display, dest, dest_kind, src, op, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off
.br
Pixmap	src;
.RE
.LP
XShapeCombineShape (display, dest, dest_kind, src, src_kind, op, x_off, y_off)
.RS
Display *display;
.br
Window dest, src;
.br
int dest_kind, src_kind, op, x_off, y_off
.RE
.LP
XShapeOffsetShape (display, dest, dest_kind, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, x_off, y_off
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
int *w_shaped, *b_shaped; /* RETURN */
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
Window window
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
