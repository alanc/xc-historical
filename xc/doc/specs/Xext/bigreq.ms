.\" Use -ms and macros.t
.\" $XConsortium: bigreq.ms,v 1.9 93/01/08 09:26:09 rws Exp $
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 10
.nr PS 10
\&
.sp 8
.ce 1
\s+2\fBBig Requests Extension\fP\s-2
.sp 3
.ce 3
Version 2.0
X Consortium Standard
X Version 11, Release 6
.sp 6
.ce 4
Bob Scheifler
.sp 6p
MIT X Consortium
Laboratory for Computer Science
Massachusetts Institute of Technology
.ps 9
.nr PS 9
.sp 8
.LP
Copyright \(co 1993 by the Massachusetts Institute of Technology
.LP
Permission to use, copy, modify, distribute, and sell this documentation
for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the documentation without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this documentation for any purpose.  It is provided "as is"
without express or implied warranty.
.ps 10
.nr PS 10
.bp 1
.EH ''Big Requests Extension''
.OH ''Big Requests Extension''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Overview
.LP
This extension enables the use of protocol requests that exceed 262140
bytes in length.
.LP
The core protocol restricts the maximum length of a protocol request
to 262140 bytes, in that it uses a 16-bit length field specifying the
number of 4-byte units in the request.  This is a problem in the core
protocol when joining large numbers of lines
.Pn ( PolyLine )
or arcs
.Pn ( PolyArc ),
since these requests cannot be broken up into smaller requests without
disturbing the rendering of the join points.  It is also much more of
a problem for protocol extensions, such as the PEX extension
for 3D graphics and the XIE extension for imaging, that need to send
long data lists in output commands.
.LP
This extension defines a mechanism for extending the length field
beyond 16 bits.  If the normal 16-bit length field of the protocol
request is zero, then an additional 32-bit field containing the actual
length (in 4-byte units) is inserted into the request, immediately
following the 16-bit length field.
.LP
For example, a normal
.PN PolyLine
encoding is:
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN PolyLine
	1	65		opcode
	1			coordinate-mode
		0	Origin
		1	Previous
	2	3+n		request length
	4	DRAWABLE		drawable
	4	GCONTEXT		gc
	4n	LISTofPOINT		points
.De
.LP
An extended-length
.PN PolyLine
encoding is:
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN PolyLine
	1	65		opcode
	1			coordinate-mode
		0	Origin
		1	Previous
	2	0		extended length flag
	4	4+n		request length
	4	DRAWABLE		drawable
	4	GCONTEXT		gc
	4n	LISTofPOINT		points
.De
.LP
Extended-length protocol encodings, once enabled, can be used on all
protocol requests, including all extensions.
.NH 1
Requests
.LP
.PN "BigReqEnable"
.LP
   =>
.IP
\fImaximum-request-length\fP\^: CARD32
.IP
This request enables extended-length protocol requests
for the requesting client.  It also returns the maximum length of a
request, in 4-byte units, that can be used in extended-length protocol
requests.  This value will always be greater than the maximum-request-length
returned in the connection setup information.
.NH 1
Events and Errors
.LP
No new events or errors are defined by this extension.
.NH 1
Encoding
.LP
Please refer to the X11 Protocol Encoding document as this document uses
conventions established there.
.LP
The name of this extension is ``BIG-REQUESTS''.
.LP
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN BigReqEnable
	1	CARD8		opcode
	1	0		bigreq opcode
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
	4	CARD32		maximum-request-length
	20			unused
.De
.NH 1
C language binding
.LP
It is desirable for core Xlib, and other extensions,
to use this extension internally when necessary.
It is also desirable to make the use of this extension as transparent as
possible to the X client.  For example, if enabling of the extension were
delayed until the first time it was needed, an application that used
.PN XNextRequest
to determine the sequence number of a request would no longer get the
correct sequence number.
As such,
.PN XOpenDisplay
will determine if the extension is supported by the server and, if it is,
enable extended-length encodings.
.LP
The core Xlib functions
.PN XDrawLines ,
.PN XDrawArcs ,
.PN XFillPolygon ,
.PN XChangeProperty ,
.PN XSetClipRectangles ,
and
.PN XSetRegion
are required to use extended-length encodings when necessary, if supported
by the server.  Use of extended-length encodings in other core Xlib
functions
.Pn ( XDrawPoints ,
.PN XDrawRectangles ,
.PN XDrawSegments ,
.PN XFillArcs ,
.PN XFillRectangles ,
.PN XPutImage )
is permitted but not required; an Xlib implementation may choose to
split the data across multiple smaller requests instead.
.LP
To permit clients to know what the maximum-request-length for extended-length
encodings is, the following function is added to Xlib:
.LP
long
.br
XExtendedMaxRequestSize (display)
.RS
Display	*display;
.RE
.IP
Returns zero (0) if the specified display does not support this
extension, otherwise returns the maximum-request-length (in 4-byte
units) supported by the server through the extended-length encoding.
.NH 1
Acknowledgements
.LP
Clive Feather (IXI) originated the extended-length encoding used in this
extension proposal.
