.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
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
.ps 11
.nr PS 11
.bp 1
.EH ''X11 Non Rectangular Window Shape Extension Proposal''
.OH ''X11 Non Rectangular Window Shape Extension Proposal''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Description
.XS
\*(SN Description
.XE
.LP
This extension provides arbitrary window and border shapes within the X11
protocol.
.IP
It retains all semantics of rectangular windows, the resultant windows and
borders remain clipped inside the appropriate rectangular bounding box.
.IP
It supplies two mechanisms for creating window shapes, either as a list of
rectangles or by using a bitmap; similar to the options availible for
GC clipping.
.LP
The restriction of rectangular windows within the X protocol is a severe
limitation in the implementation of many user interface styles.  Many
transient windows would like to display a "shadow" to give the illusion of 3
dimensions, some objects would like to have rounded corners; the simulation
of which can cause disturbing anomolys in event distribution.
.LP
This extension requires no modification to device-depedent code in the
server as it uses only existing ddx/dix interfaces.  Therefore it is a
portable extension as defined by the X11 Server Extensions document.
Portions of the actual implementation do currently assume the usage of mfb
for 1-bit off-screen pixmaps, but those areas are minor and may eventually
be eliminated with a small change in the ddx/dix specification.
.NH 1
Requests
.XS
\*(SN Requests
.XE
.LP
.IN "SetWindowShapeRectangles" "" "@DEF@"
.PN "SetWindowShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) describe the desired shape of the drawable region of
the window.  If a zero length list of rectangles is specified, the window
will be restored to full size.
.LP
.IN "IntersectWindowShapeRectangles" "" "@DEF@"
.PN "IntersectWindowShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) modify the shape of the drawable region of the
window by intersecting with the current shape of the window.
.LP
.IN "UnionWindowShapeRectangles" "" "@DEF@"
.PN "UnionWindowShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) modify the shape of the drawable region of the
window by unioning with the current shape of the window.
.LP
.IN "SetWindowShapeMask" "" "@DEF@"
.PN "SetWindowShapeMask"
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request specifies the shape of a window using a pixmap.  The
given pixmap must be of depth 1 (or a
.PN Match
error results).
The resultant shape is offset from the origin of the window by
x-offset, y-offset.  A Pixmap of None restores the window to full size.
.LP
.IN "IntersectWindowShapeMask" "" "@DEF@"
.PN "IntersectWindowShapeMask"
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request modifies the shape of a window by intersecting it with a
pixmap.  The given pixmap must be of depth 1 (or a
.PN Match
error results).  The pixmap is offset from the origin of the window by
x-offset, y-offset before applying it to the current shape.
.LP
.IN "UnionWindowShapeMask" "" "@DEF@"
.PN UnionWindowShapeMask
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request modifies the shape of a window by adding it to a pixmap.  The
given pixmap must be of depth 1 (or a
.PN Match
error results).  The pixmap is offset from the origin of the window by
x-offset, y-offset before applying it to the current shape.
.LP
.IN "GetWindowShapeRectangles" "" "@DEF@"
.PN GetWindowShapeRectangles
.IP
\fIwid\fP\^: WINDOW
.LP
   =>
.IP
rectangles: LISTofRECTANGLE
.IP
Errors:
.PN Window
.IP
This request returns the shape of the window as a list of non-intersecting
rectangles relative to the upper left corner.  This is not necessarily
equivalent to the currently visible portions of the window, rather it is
equivalent to whatever shape was specified with other extension requests.
.LP
.IN "GetWindowShapeMask" "" "@DEF@"
.PN GetWindowShapeMask
.IP
\fIwid\fP\^: WINDOW
\fIpixmap\fP\^: PIXMAP
.IP
Errors:
.PN Window ,
.PN Pixmap
.IP
This request paints the current shape of the window into the pixmap.  The
pixmap must be of depth 1 (or a
.PN Match
error results).
Areas included in the window shape are painted with 1's, the rest of the
pixmap is filled with 0's.  This request should probably take a GC and a
drawable instead.
.LP
.IN "SetBorderShapeRectangles" "" "@DEF@"
.PN "SetBorderShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) describe the desired shape of the border of the
window.  If a zero length list of rectangles is specified, the border will
be restored to full size.
.LP
.IN "IntersectBorderShapeRectangles" "" "@DEF@"
.PN "IntersectBorderShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) modify the shape of the border of the
window by intersecting with the current shape of the border.
.LP
.IN "UnionBorderShapeRectangles" "" "@DEF@"
.PN "UnionBorderShapeRectangles"
.IP
\fIwid\fP\^: WINDOW
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.IP
Errors:
.PN Window ,
.PN Length
.IP
This request specifies an array of rectangles, relative to the origin of the
window which (together) modify the shape of the border of the
window by unioning with the current shape of the border.
.LP
.IN "SetBorderShapeMask" "" "@DEF@"
.PN "SetBorderShapeMask"
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request specifies the shape of a border using a pixmap.  The
given pixmap must be of depth 1 (or a
.PN Match
error results).
The resultant shape is offset from the origin of the window by
x-offset, y-offset.  A Pixmap of None restores the border to full size.
.LP
.IN "IntersectBorderShapeMask" "" "@DEF@"
.PN "IntersectBorderShapeMask"
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request modifies the shape of a border by intersecting it with a
pixmap.  The given pixmap must be of depth 1 (or a
.PN Match
error results).  The pixmap is offset from the origin of the window by
x-offset, y-offset before applying it to the current shape.
.LP
.IN "UnionBorderShapeMask" "" "@DEF@"
.PN UnionBorderShapeMask
.IP
\fIwid\fP\^: WINDOW
.br
\fImask\fP\^: PIXMAP
.br
\fIx-offset, y-offset\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match
.IP
This request modifies the shape of a border by adding it to a pixmap.  The
given pixmap must be of depth 1 (or a
.PN Match
error results).  The pixmap is offset from the origin of the window by
x-offset, y-offset before applying it to the current shape.
.LP
.IN "GetBorderShapeRectangles" "" "@DEF@"
.PN GetBorderShapeRectangles
.IP
\fIwid\fP\^: WINDOW
.LP
   =>
.IP
rectangles: LISTofRECTANGLE
.IP
Errors:
.PN Window
.IP
This request returns the shape of the border as a list of non-intersecting
rectangles relative to the upper left corner.  This is not necessarily
equivalent to the currently visible portions of the border, rather it is
equivalent to whatever shape was specified with other extension requests.
.LP
.IN "GetBorderShapeMask" "" "@DEF@"
.PN GetBorderShapeMask
.IP
\fIwid\fP\^: WINDOW
\fIpixmap\fP\^: PIXMAP
.IP
Errors:
.PN Window ,
.PN Pixmap
.IP
This request paints the current shape of the border into the pixmap.  The
pixmap must be of depth 1 (or a
.PN Match
error results).
Areas included in the border shape are painted with 1's, the rest of the
pixmap is filled with 0's.  This request should probably take a GC and a
drawable instead.
