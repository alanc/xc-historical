.sp 8
.ce 2
\s+2\fBXmu Library\fP\s-2
.sp 6p
X Version 11, Release 4
.sp 1
.ce 1
``\fIDon't ask.\fP''
.sp 2
.LP
Copyright \(co 1989 by the Massachusetts Institute of Technology
.LP
Permission to use, copy, modify, and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.  MIT makes no
representations about the suitability for any purpose of the information in
this document.  This documentation is provided ``as is'' without express or
implied warranty.
.sp 2
.NH 1
Introduction
.LP
The Xmu Library is a collection of miscellaneous (some might say random)
utility functions that have been useful in building various applications
and widgets.
.LP
.NH 1
Atom Functions
.LP
The use the functions and macros defined in this section, you should include
the header file
.Pn < X11/Xmu/Atoms.h >.
.sp
.FD 0
XA_ATOM_PAIR(\fId\fP)
XA_CHARACTER_POSITION(\fId\fP)
XA_CLASS(\fId\fP)
XA_CLIENT_WINDOW(\fId\fP)
XA_CLIPBOARD(\fId\fP)
XA_COMPOUND_TEXT(\fId\fP)
XA_DECNET_ADDRESS(\fId\fP)
XA_DELETE(\fId\fP)
XA_FILENAME(\fId\fP)
XA_HOSTNAME(\fId\fP)
XA_IP_ADDRESS(\fId\fP)
XA_LENGTH(\fId\fP)
XA_LIST_LENGTH(\fId\fP)
XA_NAME(\fId\fP)
XA_NET_ADDRESS(\fId\fP)
XA_NULL(\fId\fP)
XA_OWNER_OS(\fId\fP)
XA_SPAN(\fId\fP)
XA_TARGETS(\fId\fP)
XA_TEXT(\fId\fP)
XA_TIMESTAMP(\fId\fP)
XA_USER(\fId\fP)
.FN
.LP
These macros take a display as argument and return an
.PN Atom .
The name of the
atom is obtained from the macro name by removing the leading characters
``XA_''.  The
.PN Atom
value is cached, such that subsequent requests do not cause
another round-trip to the server.
.sp
.FD 0
AtomPtr XmuMakeAtom(\fIname\fP)
.br
      char* \fIname\fP;
.FN
.LP
Creates and initializes an opaque object, an
.PN AtomPtr ,
for an
.PN Atom
with the
given name.
.PN XmuInternAtom
can be used to cache the Atom value for one or more displays.
.sp
.FD 0
char *XmuNameOfAtom(\fIatom_ptr\fP)
.br
      AtomPtr \fIatom_ptr\fP;
.FN
.LP
Returns the name of an AtomPtr.
.sp
.FD 0
Atom XmuInternAtom(\fId\fP, \fIatom_ptr\fP)
.br
      Display *\fId\fP;
.br
      AtomPtr \fIatom_ptr\fP;
.FN
.LP
Returns the
.PN Atom
for an
.PN AtomPtr .
The
.PN Atom
is cached,
such that subsequent requests do not cause another round-trip to the server.
.sp
.FD 0
char *XmuGetAtomName(\fId\fP, \fIatom\fP)
.br
      Display *\fId\fP;
.br
      Atom \fIatom\fP;
.FN
.LP
Returns name of an
.PN Atom .
The result is cached, such that subsequent
requests do not cause another round-trip to the server.
.sp
.FD 0
void XmuInternStrings(\fId\fP, \fInames\fP, \fIcount\fP, \fIatoms\fP)
.br
      Display *\fId\fP;
.br
      String *\fInames\fP;
.br
      Cardinal \fIcount\fP;
.br
      Atom *\fIatoms\fP; /* RETURN */
.FN
.LP
Convert a list of atom names into
.PN Atom
values.  The results are cached, such
that subsequent requests do not cause further round-trips to the server.
The caller is responsible for preallocating the array pointed at by atoms.
.LP
.NH 1
Error Handler Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/Error.h >.
.sp
.FD 0
int XmuPrintDefaultErrorMessage(\fIdpy\fP, \fIevent\fP, \fIfp\fP)
.br
      Display *\fIdpy\fP;
.br
      XErrorEvent *\fIevent\fP;
.br
      FILE *\fIfp\fP;
.FN
.LP
This function prints an error message, equivalent to Xlib's default error
message for protocol errors.  It returns 1 if the caller should consider
exiting, otherwise it returns 0.  This function can be used when you need to
write your own error handler, but need to print out an error from within
that handler.
.sp
.FD 0
int XmuSimpleErrorHandler(\fIdpy\fP, \fIerrorp\fP)
.br
      Display *\fIdpy\fP;
.br
      XErrorEvent *\fIerrorp\fP;
.FN
.LP
This function ignores errors for
.PN BadWindow
errors for
.PN XQueryTree
and
.PN XGetWindowAttributes ,
and ignores
.PN BadDrawable
errors for
.PN XGetGeometry ;
it returns 0 in those cases.  Otherwise, it prints the default error message,
and returns 1 if the caller should consider exiting, and 0 if the caller
should not exit.
.LP
.NH 1
System Utility Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/SysUtil.h >.
.sp
.FD 0
int XmuGetHostname(\fIbuf\fP, \fImaxlen\fP)
.br
      char *\fIbuf\fP;
.br
      int \fImaxlen\fP;
.FN
.LP
This function stores the null terminated name of the local host in buf, and
returns length of the name.  This function hides operating system differences,
such as whether to call gethostname or uname.
.LP
.NH 1
Window Utility Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/WinUtil.h >.
.sp
.FD 0
Screen *XmuScreenOfWindow(\fIdpy\fP, \fIw\fP)
.br
      Display *\fIdpy\fP;
.br
      Window \fIw\fP;
.FN
.LP
This function returns the
.PN Screen
on which the specified window was created.
.sp
.FD 0
Window XmuClientWindow(\fIdpy\fP, \fIwin\fP)
.br
      Display *\fIdpy\fP;
.br
      Window \fIwin\fP;
.FN
.LP
This function finds a window, at or below the specified window, which has a
WM_STATE property.  If such a window is found, it is returned, otherwise the
argument window is returned.
.sp
.FD 0
Bool XmuUpdateMapHints(\fIdpy\fP, \fIw\fP, \fIhints\fP)
.br
      Display *\fIdpy\fP;
.br
      Window \fIw\fP;
.br
      XSizeHints *\fIhints\fP;
.FN
.LP
This function clears the
.PN PPosition
and
.PN PSize
flags and sets the
.PN USPosition
and
.PN USSize
flags in the hints structure, and then stores the hints for the
window using
.PN XSetWMNormalHints
and returns
.PN True .
If NULL is passed for the
hints structure, then the current hints are read back from the window using
.PN XGetWMNormalHints
and are used instead, and
.PN True
is returned; otherwise
.PN False
is returned.
.LP
.NH 1
Cursor Utility Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/CurUtil.h >.
.sp
.FD 0
int XmuCursorNameToIndex(\fIname\fP)
.br
      char *\fIname\fP;
.FN
.LP
This function takes the name of a standard cursor and returns its index
in the standard cursor font.  The cursor names are formed by removing the
``XC_'' prefix from the cursor defines listed in Appendix B of the Xlib
manual.
.LP
.NH 1
Graphics Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/Drawing.h >.
.sp
.FD 0
void XmuDrawRoundedRectangle(\fIdpy\fP, \fIdraw\fP, \fIgc\fP, \fIx\fP, \fIy\fP, \fIw\fP, \fIh\fP, \fIew\fP, \fIeh\fP)
.br
      Display *\fIdpy\fP;
.br
      Drawable \fIdraw\fP;
.br
      GC \fIgc\fP;
.br
      int \fIx\fP, \fIy\fP, \fIw\fP, \fIh\fP, \fIew\fP, \fIeh\fP;
.FN
.LP
This function draws a rounded rectangle, where x, y, w, h are the dimensions
of the overall rectangle, and ew and eh are the sizes of a bounding box that
the corners are drawn inside of; ew should be no more than half of w, and eh
should be no more than half of h.  The current GC line attributes control
all attributes of the line.
.sp
.FD 0
void XmuFillRoundedRectangle(\fIdpy\fP, \fIdraw\fP, \fIgc\fP, \fIx\fP, \fIy\fP, \fIw\fP, \fIh\fP, \fIew\fP, \fIeh\fP)
.br
      Display *\fIdpy\fP;
.br
      Drawable \fIdraw\fP;
.br
      GC \fIgc\fP;
.br
      int \fIx\fP, \fIy\fP, \fIw\fP, \fIh\fP, \fIew\fP, \fIeh\fP;
.FN
.LP
This function draws a filled rounded rectangle, where x, y, w, h are the
dimensions of the overall rectangle, and ew and eh are the sizes of a
bounding box that the corners are drawn inside of; ew should be no more than
half of w, and eh should be no more than half of h.  The current GC fill
settings control all attributes of the fill contents.
.sp
.FD 0
XmuDrawLogo(\fIdpy\fP, \fIdrawable\fP, \fIgcFore\fP, \fIgcBack\fP, \fIx\fP, \fIy\fP, \fIwidth\fP, \fIheight\fP)
.br
      Display *\fIdpy\fP;
.br
      Drawable \fIdrawable\fP;
.br
      GC \fIgcFore\fP, \fIgcBack\fP;
.br
      int \fIx\fP, \fIy\fP;
.br
      unsigned int \fIwidth\fP, \fIheight\fP;
.FN
.LP
This function draws the ``official'' X Window System logo.  The bounding box
of the logo in the drawable is given by x, y, width, and height.  The logo
itself is filled using gcFore, and the rest of the rectangle is filled using
gcBack.
.sp
.FD 0
Pixmap XmuCreateStippledPixmap(\fIscreen\fP, \fIfore\fP, \fIback\fP, \fIdepth\fP)
.br
      Screen *\fIscreen\fP;
.br
      Pixel \fIfore\fP, \fIback\fP;
.br
      unsigned int \fIdepth\fP;
.FN
.LP
This function creates a two pixel by one pixel stippled pixmap of specified
depth on the specified screen.  The pixmap is cached so that multiple
requests share the same pixmap.  The pixmap should be freed with
.PN XmuReleaseStippledPixmap
to maintain correct reference counts.
.sp
.FD 0
void XmuReleaseStippledPixmap(\fIscreen\fP, \fIpixmap\fP)
.br
      Screen *\fIscreen\fP;
.br
      Pixmap \fIpixmap\fP;
.FN
.LP
This function frees a pixmap created with
.PN XmuCreateStippledPixmap .
.sp
.FD 0
int XmuReadBitmapData(\fIfstream\fP, \fIwidth\fP, \fIheight\fP, \fIdatap\fP, \fIx_hot\fP, \fIy_hot\fP)
    FILE *\fIfstream\fP;			/* handle on file  */
    unsigned int *\fIwidth\fP, *\fIheight\fP;	/* RETURNED */
    unsigned char **\fIdatap\fP;		/* RETURNED */
    int *\fIx_hot\fP, *\fIy_hot\fP;			/* RETURNED */
.FN
.LP
This function reads a standard bitmap file description from the specified
stream, and returns the parsed data in a format suitable for passing to
.PN XCreateBitmapFromData .
The return value of the function has the same
interpretation as the return value for
.PN XReadBitmapFile .
.sp
.FD 0
int XmuReadBitmapDataFromFile(\fIfilename\fP, \fIwidth\fP, \fIheight\fP, \fIdatap\fP, \fIx_hot\fP, \fIy_hot\fP)
    char *\fIfilename\fP;
    unsigned int *\fIwidth\fP, *\fIheight\fP;	/* RETURNED */
    unsigned char **\fIdatap\fP;		/* RETURNED */
    int *\fIx_hot\fP, *\fIy_hot\fP;			/* RETURNED */
.FN
.LP
This function reads a standard bitmap file description from the specified
file, and returns the parsed data in a format suitable for passing to
.PN XCreateBitmapFromData .
The return value of the function has the same
interpretation as the return value for
.PN XReadBitmapFile .
.sp
.FD 0
Pixmap XmuLocateBitmapFile(\fIscreen\fP, \fIname\fP, \fIsrcname\fP, \fIsrcnamelen\fP, \fIwidthp\fP, \fIheightp\fP, \fIxhotp\fP, \fIyhotp\fP)
    Screen *\fIscreen\fP;
    char *\fIname\fP;
    char *\fIsrcname\fP; /* RETURN */
    int \fIsrcnamelen\fP;
    int *\fIwidthp\fP, *\fIheightp\fP, *\fIxhotp\fP, *\fIyhotp\fP; /* RETURN */
.FN
.LP
This function reads a file in standard bitmap file format, using
.PN XReadBitmapFile ,
and returns the created bitmap.  The filename may be
absolute, or relative to the global resource named bitmapFilePath with class
BitmapFilePath.  If the resource is not defined, the default value is the
build symbol BITMAPDIR, which is typically "/usr/include/X11/bitmaps".  If
srcnamelen is greater than zero and srcname is not NULL, the null terminated
filename will be copied into srcname.  The size of hotspot of the bitmap are
also returned.
.sp
.FD 0
Pixmap XmuCreatePixmapFromBitmap(\fIdpy\fP, \fId\fP, \fIbitmap\fP, \fIwidth\fP, \fIheight\fP, \fIdepth\fP, \fIfore\fP, \fIback\fP)
    Display *\fIdpy\fP;			/* connection to X server */
    Drawable \fId\fP;				/* drawable indicating screen */
    Pixmap \fIbitmap\fP;			/* single plane pixmap */
    unsigned int \fIwidth\fP, \fIheight\fP;		/* dimensions of bitmap and pixmap */
    unsigned int \fIdepth\fP;			/* depth of pixmap to create */
    unsigned long \fIfore\fP, \fIback\fP;		/* colors to use */
.FN
.LP
This function creates a pixmap of the specified width, height, and depth, on
the same screen as the specified drawable, and then performs an
.PN XCopyPlane
from the specified bitmap to the pixmap,
using the specified foreground and background pixel values.
The created pixmap is returned.
.LP
.NH 1
Selection Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/StdSel.h >.
.sp
.FD 0
Boolean XmuConvertStandardSelection(\fIw\fP, \fItime\fP, \fIselection\fP, \fItarget\fP, \fItype\fP, \fIvalue\fP, \fIlength\fP, \fIformat\fP)
.br
      Widget \fIw\fP;
.br
      Time \fItime\fP;
.br
      Atom *\fIselection\fP, *\fItarget\fP, *\fItype\fP;
.br
      caddr_t *\fIvalue\fP;
.br
      unsigned long *\fIlength\fP;
.br
      int *\fIformat\fP;
.FN
.LP
This function converts the following standard selections: CLASS,
CLIENT_WINDOW, DECNET_ADDRESS, HOSTNAME, IP_ADDRESS, NAME, OWNER_OS,
TARGETS, TIMESTAMP, and USER.  It returns
.PN True
if the conversion was successful, else it returns
.PN False.
.LP
.NH 1
Type Converter Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/Converters.h >.
.sp
.FD 0
void XmuCvtFunctionToCallback(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValue *\fIargs\fP; /* unused */
.br
      Cardinal *\fInum_args\fP;	/* unused */
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a callback procedure to a callback list containing
that procedure, with NULL closure data.  To use this converter, include the
following in your ClassInitialize procedure:
.LP
XtAddConverter(XtRCallProc, XtRCallback, XmuCvtFunctionToCallback, NULL, 0);
.sp
.FD 0
void XmuCvtStringToBackingStore(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValue *\fIargs\fP; /* unused */
.br
      Cardinal *\fInum_args\fP; /* unused */
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to a backing-store integer as defined in
.Pn < X11/X.h >.
The string "notUseful" converts to
.PN NotUseful ,
"whenMapped" converts to
.PN WhenMapped ,
and "always" converts to
.PN Always .
The string "default" converts to the value
.PN Always +
.PN WhenMapped +
.PN NotUseful .
The case of the string does not matter.
To use this converter, include the following
in your ClassInitialize procedure:
.LP
XtAddConverter(XtRString, XtRBackingStore, XmuCvtStringToBackingStore, NULL, 0);
.sp
.FD 0
void XmuCvtStringToBitmap(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr \fIargs\fP;
.br
      Cardinal *\fInum_args\fP;
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function creates a bitmap (a Pixmap of depth one) suitable for window
manager icons.  The string argument is the name of a file in standard bitmap
file format.  For the possible filename specifications, see
.PN XmuLocateBitmapFile .
To use this converter, include the following in your
ClassInitialize procedure:
.LP
static XtConvertArgRec screenConvertArg[] = {
.br
  {XtBaseOffset, (XtPointer)XtOffset(Widget, core.screen), sizeof(Screen *)}
.br
};
.LP
XtAddConverter(XtRString, XtRBitmap, XmuCvtStringToBitmap,
.br
		 screenConvertArg, XtNumber(screenConvertArg));
.sp
.FD 0
void XmuCvtStringToCursor(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr \fIargs\fP;
.br
      Cardinal *\fInum_args\fP;
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to a
.PN Cursor .
The string can either be a
standard cursor name formed by removing the ``XC_'' prefix from any of the
cursor defines listed in Appendix B of the Xlib Manual, a font name and
glyph index of the form "FONT fontname index [[font] index]", or a bitmap
filename acceptable to
.PN XmuLocateBitmapFile .
To use this converter, include
the following in your ClassInitialize procedure:
.LP
static XtConvertArgRec screenConvertArg[] = {
.br
  {XtBaseOffset, (XtPointer)XtOffset(Widget, core.screen), sizeof(Screen *)}
.br
};
.LP
XtAddConverter(XtRString, XtRCursor, XmuCvtStringToCursor,
.br
		 screenConvertArg, XtNumber(screenConvertArg));
.sp
.FD 0
void XmuCvtStringToJustify(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr *\fIargs\fP;	/* unused */
.br
      Cardinal *\fInum_args\fP;	/* unused */
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to an
.PN XtJustify
enumeration value.  The string "left" converts to
.PN XtELeft ,
"center" converts to
.PN XtEcenter ,
and "right" converts to
.PN XtERight .
The case of the string does not matter.  To
use this converter, include the following in your ClassInitialize procedure:
.LP
XtAddConverter(XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0);
.sp
.FD 0
void XmuCvtStringToLong(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr \fIargs\fP;
.br
      Cardinal    *\fInum_args\fP;
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to an integer of type long.  It parses the
string using
.PN sscanf
with a format of "%ld".  To use this converter, include
the following in your ClassInitialize procedure:
.LP
XtAddConverter(XtRString, XtRLong, XmuCvtStringToLong, NULL, 0);
.sp
.FD 0
void XmuCvtStringToOrientation(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr *\fIargs\fP; /* unused */
.br
      Cardinal *\fInum_args\fP; /* unused */
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to an
.PN XtOrientation
enumeration value.  The string "horizontal" converts to
.PN XtorientHorizontal
and "vertical" converts to
.PN XtorientVertical .
The case of the string does not matter.  To use this
converter, include the following in your ClassInitialize procedure:
.LP
XtAddConverter(XtRString, XtROrientation, XmuCvtStringToOrientation, NULL, 0);
.sp
.FD 0
Boolean XmuCvtStringToShapeStyle(\fIdpy\fP, \fIargs\fP, \fInum_args\fP, \fIfrom\fP, \fItoVal\fP, data)
.br
      Display *\fIdpy\fP;
.br
      XrmValue *\fIargs\fP;		/* unused */
.br
      Cardinal *\fInum_args\fP;		/* unused */
.br
      XrmValue *\fIfrom\fP;
.br
      XrmValue *\fItoVal\fP;
.br
      XtPointer *\fIdata\fP;		/* unused */
.FN
.LP
This function converts a string to an integer shape style.  The string
"Rectangle" converts to
.PN XmuShapeRectangle ,
"Oval" converts to
.PN XmuShapeOval ,
"Ellipse" converts to
.PN XmuShapeEllipse ,
and "RoundedRectangle" converts to
.PN XmuShapeRoundedRectangle .
The case of the string does not matter.  To use
this converter, include the following in your ClassInitialize procedure:
.LP
XtSetTypeConverter(XtRString, XtRShapeStyle, XmuCvtStringToShapeStyle,
.br
		     NULL, 0, XtCacheNone, NULL);
.LP
Boolean XmuReshapeWidget(w, \fIshape_style\fP, \fIcorner_width\fP, \fIcorner_height\fP)
.br
      Widget \fIw\fP;
.br
      int \fIshape_style\fP;
.br
      int \fIcorner_width\fP, \fIcorner_height\fP;
.LP
This function reshapes the specified widget, using the Shape extension, to a
rectangle, oval, ellipse, or rounded rectangle, as specified by shape_style.
The shape is bounded by the outside edges of the rectangular extents of the
widget.  If the shape is a rounded rectangle, corner_width and corner_height
specify the size of the bounding box that the corners are drawn inside of
(see
.PN XmuFillRoundedRectangle );
otherwise, corner_width and corner_height are ignored.
The origin of the widget within its parent remains unchanged.
.sp
.FD 0
void XmuCvtStringToWidget(\fIargs\fP, \fInum_args\fP, \fIfromVal\fP, \fItoVal\fP)
.br
      XrmValuePtr \fIargs\fP;		/* parent */
.br
      Cardinal *\fInum_args\fP;      /* 1 */
.br
      XrmValuePtr \fIfromVal\fP;
.br
      XrmValuePtr \fItoVal\fP;
.FN
.LP
This function converts a string to an immediate child widget of the parent
widget passed as an argument.  The string is first compared against the
names of the normal and popup children, and if a match is found the
corresponding child is returned.  If no match is found, the string is
compared against the classes of the normal and popup children, and if a
match is found the corresponding child is returned.  The case of the string
is significant.  To use this converter, include the following in your
ClassInitialize procedure:
.LP
static XtConvertArgRec parentCvtArg[] = {
.br
  {XtBaseOffset, (XtPointer)XtOffset(Widget, core.parent), sizeof(Widget)},
.br
};
.LP
XtAddConverter(XtRString, XtRWidget, XmuCvtStringToWidget,
.br
		 parentCvtArg, XtNumber(parentCvtArg));
.LP
.NH 1
Character Set Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/CharSet.h >.
.sp
.FD 0
void XmuCopyISOLatin1Lowered(\fIdst\fP, \fIsrc\fP)
.br
      char *\fIdst\fP, *\fIsrc\fP;
.FN
.LP
This function copies a null terminated string from src to dst (including the
null), changing all Latin-1 uppercase letters to lowercase.  The string is
assumed to be encoded using ISO 8859-1.
.sp
.FD 0
void XmuCopyISOLatin1Uppered(\fIdst\fP, \fIsrc\fP)
.br
      char *\fIdst\fP, *\fIsrc\fP;
.FN
.LP
This function copies a null terminated string from src to dst (including the
null), changing all Latin-1 lowercase letters to uppercase.  The string is
assumed to be encoded using ISO 8859-1.
.sp
.FD 0
int XmuCompareISOLatin1(\fIfirst\fP, \fIsecond\fP)
.br
      char *\fIfirst\fP, *\fIsecond\fP;
.FN
.LP
This function compares two null terminated Latin-1 strings, ignoring case
differences, and returns an integer greater than, equal to, or less than 0,
according to whether first is lexicographically greater than, equal to, or
less than second.  The two strings are assumed to be encoded using ISO
8859-1.
.sp
.FD 0
int XmuLookupLatin1(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is identical to
.PN XLookupString ,
and exists only for naming symmetry with other functions.
.sp
.FD 0
int XmuLookupLatin2(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to an Latin-2 (ISO 8859-2) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupLatin3(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to an Latin-3 (ISO 8859-3) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupLatin4(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to an Latin-4 (ISO 8859-4) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupKana(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a string in an encoding consisting of Latin-1 (ISO 8859-1) and ASCII
control in the Graphics Left half (values 0 to 127), and Katakana in the
Graphics Right half (values 128 to 255), using the values from JIS
X201-1976.
.sp
.FD 0
int XmuLookupJISX0201(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a string in the JIS X0201-1976 encoding, including ASCII control.
.sp
.FD 0
int XmuLookupArabic(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a Latin/Arabic (ISO 8859-6) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupCyrillic(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a Latin/Cyrillic (ISO 8859-5) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupGreek(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a Latin/Greek (ISO 8859-7) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupHebrew(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event
to a Latin/Hebrew (ISO 8859-8) string, or to an ASCII control string.
.sp
.FD 0
int XmuLookupAPL(\fIevent\fP, \fIbuffer\fP, \fInbytes\fP, \fIkeysym\fP, \fIstatus\fP)
.br
      XKeyEvent *\fIevent\fP;
.br
      char *\fIbuffer\fP;
.br
      int \fInbytes\fP;
.br
      KeySym *\fIkeysym\fP;
.br
      XComposeStatus *\fIstatus\fP;
.FN
.LP
This function is similar to
.PN XLookupString ,
except that it maps a key event to an APL string.
.LP
.NH 1
Compound Text Functions
.LP
The functions defined in this section are for parsing Compound Text strings,
decomposing them into individual segments.  Definitions needed to use these
routines are in the include file
.Pn < X11/Xmu/Xct.h >.
A Compound Text string is represented as the following type:
.LP
	typedef unsigned char *XctString;
.sp
.FD 0
XctData XctCreate(\fIstring\fP, \fIlength\fP, \fIflags\fP)
.br
      XctString \fIstring\fP;
.br
      int \fIlength\fP;
.br
      XctFlags \fIflags\fP;
.FN
.IP \fIstring\fP 1i
the Compound Text string
.IP \fIlength\fP 1i
the number of bytes in string
.IP \fIflags\fP 1i
parsing control flags
.LP
This function creates an
.PN XctData
structure for parsing a Compound Text
string.  The string need not be null terminated.  The following flags are
defined to control parsing of the string:
.LP
.PN XctSingleSetSegments
-- This means that returned segments should contain
characters from only one set (C0, C1, GL, GR).  When this is requested,
.PN XctSegment
is never returned by
.PN XctNextItem ,
instead
.PN XctC0Segment ,
.PN XctC1Segment ,
.PN XctGlSegment ,
and
.PN XctGRSegment
are returned.  C0 and C1
segments are always returned as singleton characters.
.LP
.PN XctProvideExtensions
-- This means that if the Compound Text string is from a
higher version than this code is implemented to, then syntactically correct
but unknown control sequences should be returned as
.PN XctExtension
items by
.PN XctNextItem .
If this flag is not set, and the Compound Text string version
indicates that extensions cannot be ignored, then each unknown control
sequence will be reported as an
.PN XctError .
.LP
.PN XctAcceptC0Extensions
-- This means that if the Compound Text string is from
a higher version than this code is implemented to, then unknown C0
characters should be treated as if they were legal, and returned as C0
characters (regardless of how
.PN XctProvideExtensions
is set) by
.PN XctNextItem .
If this flag is not set, then all unknown C0 characters are treated
according to
.PN XctProvideExtensions .
.LP
.PN XctAcceptC1Extensions
-- This means that if the Compound Text string is from
a higher version than this code is implemented to, then unknown C0
characters should be treated as if they were legal, and returned as C0
characters (regardless of how
.PN XctProvideExtensions
is set) by
.PN XctNextItem .
If this flag is not set, then all unknown C0 characters are treated
according to
.PN XctProvideExtensions .
.LP
.PN XctHideDirection
-- This means that horizontal direction changes should be
reported as
.PN XctHorizontal
items by
.PN XctNextItem .  If this flag is not set,
then direction changes are not returned as items, but the current direction
is still maintained and reported for other items.  The current direction is
given as an enumeration, with the values
.PN XctUnspecified ,
.PN XctLeftToRight ,
and
.PN XctRightToLeft .
.LP
.PN XctFreeString
-- This means that
.PN XctFree
should free the Compound Text string that is passed to
.PN XctCreate .
If this flag is not set, the string is not freed.
.LP
.PN XctShiftMultiGRToGL
-- This means that
.PN XctNextItem
should translate GR
segments on-the-fly into GL segments for the GR sets: GB2312.1980-1,
JISX0208.1983-1, and KSC5601.1987-1.
.sp
.FD 0
void XctReset(\fIdata\fP)
.br
      XctData \fIdata\fP;
.FN
.LP
This function resets the
.PN XctData
structure to reparse the Compound Text string from the beginning.
.sp
.FD 0
XctResult XctNextItem(\fIdata\fP)
.br
      XctData \fIdata\fP;
.FN
.LP
This function parses the next ``item'' from the Compound Text string.  The
return value indicates what kind of item is returned.  The item itself, it's
length, and the current contextual state, are reported as components of the
.PN XctData
structure.
.PN XctResult
is an enumeration, with the following values:
.LP
.PN XctSegment
-- the item contains some mixture of C0, GL, GR, and C1 characters.
.LP
.PN XctC0Segment
-- the item contains only C0 characters.
.LP
.PN XctGLSegment
-- the item contains only GL characters.
.LP
.PN XctC1Segment
-- the item contains only C1 characters.
.LP
.PN XctGRSegment
-- the item contains only GR characters.
.LP
.PN XctExtendedSegment
-- the item contains an extended segment.
.LP
.PN XctExtension
-- the item is an unknown extension control sequence.
.LP
.PN XctHorizontal
-- the item indicates a change in horizontal direction or
depth.  The new direction and depth are recorded in the
.PN XctData
structure.
.LP
.PN XctEndOfText
-- The end of the Compound Text string has been reached.
.LP
.PN XctError
-- the string contains a syntactic or semantic error; no further
parsing should be performed.
.LP
The following state values are stored in the
.PN XctData
structure:
.Ds 0
.TA .5i 3i
.ta .5i 3i
	XctString item;	/* the action item */
	int item_length;	/* the length of item in bytes */
	int char_size;	/* the number of bytes per character in
		 * item, with zero meaning variable */
	char *encoding;	/* the XLFD encoding name for item */
	XctHDirection horizontal;	/* the direction of item */
	int horz_depth;	/* the current direction nesting depth */
	char *GL;	/* the "{I} F" string for the current GL */
	char *GL_encoding;	/* the XLFD encoding name for the current GL */
	int GL_set_size;	/* 94 or 96 */
	int GL_char_size;	/* the number of bytes per GL character */
	char *GR;	/* the "{I} F" string for the current GR */
	char *GR_encoding;	/* the XLFD encoding name the for current GR */
	int GR_set_size;	/* 94 or 96 */
	int GR_char_size;	/* the number of bytes per GR character */
	char *GLGR_encoding;	/* the XLFD encoding name for the current
		 * GL+GR, if known */
.De
.sp
.FD 0
void XctFree(\fIdata\fP)
.br
      XctData \fIdata\fP;
.FN
.LP
This function frees all data associated with the
.PN XctData
structure.
.LP
.NH 1
CloseDisplay Hook Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/CloseHook.h >.
.sp
.FD 0
CloseHook XmuAddCloseDisplayHook(\fIdpy\fP, \fIfunc\fP, \fIarg\fP)
.br
      Display *\fIdpy\fP;
.br
      int (*\fIfunc\fP)(); /* function to call on close display */
.br
      caddr_t \fIarg\fP;   /* arg to pass */
.FN
.LP
This function adds a callback for the given display.  When the display is
closed, the given function will be called with the given display and
argument as:
.LP
	(*func)(dpy, arg)
.LP 
The function is declared to return an int even though the value is ignored,
because some compilers have problems with functions returning void.
.LP
This routine returns NULL if it was unable to add the callback, otherwise it
returns an opaque handle that can be used to remove or lookup the callback.
.sp
.FD 0
Bool XmuRemoveCloseDisplayHook(\fIdpy\fP, \fIhandle\fP, \fIfunc\fP, \fIarg\fP)
.br
      Display *\fIdpy\fP;
.br
      CloseHook \fIhandle\fP; /* value from XmuAddCloseDisplayHook */
.br
      int (*\fIfunc\fP)();    /* function to call on close display */
.br
      caddr_t \fIarg\fP;      /* arg to pass */
.FN
.LP
This function deletes a callback that has been added with
.PN XmuAddCloseDisplayHook .
If handle is not NULL, it specifies the callback to
remove, and the func and arg parameters are ignored.  If handle is NULL, the
first callback found to match the specified func and arg will be removed.
Returns
.PN True
if a callback was removed, else returns
.PN False .
.sp
.FD 0
Bool XmuLookupCloseDisplayHook(\fIdpy\fP, \fIhandle\fP, \fIfunc\fP, \fIarg\fP)
.br
      Display *\fIdpy\fP;
.br
      CloseHook \fIhandle\fP; /* value from XmuAddCloseDisplayHook */
.br
      int (*\fIfunc\fP)();    /* function to call on close display */
.br
      caddr_t \fIarg\fP;      /* arg to pass */
.FN
.LP
This function determines if a callback is installed.  If handle is not NULL,
it specifies the callback to look for, and the func and arg parameters are
ignored.  If handle is NULL, the function will look for any callback for the
specified func and arg.  Returns
.PN True
if a matching callback exists, else returns
.PN False .
.LP
.NH 1
Display Queue Functions
.LP
To use the functions and types defined in this section, you should include the
header file
.Pn < X11/Xmu/DisplayQue.h >.
It defines the following types:
.LP
.Ds 0
.TA .5i 3i
.ta .5i 3i
typedef struct _XmuDisplayQueueEntry {
	struct _XmuDisplayQueueEntry *prev, *next;
	Display *display;
	CloseHook closehook;
	caddr_t data;
} XmuDisplayQueueEntry;

typedef struct _XmuDisplayQueue {
	int nentries;
	XmuDisplayQueueEntry *head, *tail;
	int (*closefunc)();
	int (*freefunc)();
	caddr_t data;
} XmuDisplayQueue;
.De
.sp
.FD 0
XmuDisplayQueue *XmuDQCreate(\fIclosefunc\fP, \fIfreefunc\fP, \fIdata\fP)
.br
      int (*\fIclosefunc\fP)();
.br
      int (*\fIfreefunc\fP)();
.br
      caddr_t \fIdata\fP;
.FN
.LP
This function creates and returns an empty
.PN XmuDisplayQueue
(which is really just a set of displays, but is called a queue for
historical reasons).  The queue is initially empty, but displays
can be added using
.PN XmuAddDisplay .
The data value is simply stored in the queue for use by the closefunc
and freefunc callbacks.
Whenever a display in the queue is closed using
.PN XCloseDisplay ,
the closefunc (if non-NULL) is called with the queue and the display's
.PN XmuDisplayQueueEntry
as follows:
.LP
	(*closefunc)(queue, entry)
.LP
The freeproc (if non-NULL) is called whenever the last display in the
queue is closed, as follows:
.LP
	(*freefunc)(queue)
.LP
The appllication is responsible for actually freeing the queue, by calling
.PN XmuDQDestroy .
.sp
.FD 0
XmuDisplayQueueEntry *XmuDQAddDisplay(\fIq\fP, \fIdpy\fP, \fIdata\fP)
.br
      XmuDisplayQueue *\fIq\fP;
.br
      Display *\fIdpy\fP;
.br
      caddr_t \fIdata\fP;
.FN
.LP
This function adds the specified display to the queue.  If successful,
the queue entry is returned, otherwise NULL is returned.
The data value is simply stored in the queue entry for use by the
queue's freefunc callback.  This function does not attempt to prevent
duplicate entries in the queue; the caller should use
.PN XmuDQLookupDisplay
to determine if a display has already been added to a queue.
.sp
.FD 0
XmuDisplayQueueEntry *XmuDQLookupDisplay(\fIq\fP, \fIdpy\fP)
.br
      XmuDisplayQueue *\fIq\fP;
.br
      Display *\fIdpy\fP;
.FN
.LP
This function returns the queue entry for the specified display, or NULL if
the display is not in the queue.
.sp
.FD 0
XmuDQNDisplays(\fIq\fP)
.FN
.LP
This macro returns the number of displays in the specified queue.
.sp
.FD 0
Bool XmuDQRemoveDisplay(\fIq\fP, \fIdpy\fP)
.br
      XmuDisplayQueue *\fIq\fP;
.br
      Display *\fIdpy\fP;
.FN
.LP
This function removes the specified display from the specified queue.
No callbacks are performed.
If the display is not found in the queue,
.PN False
is returned, otherwise
.PN True
is returned.
.sp
.FD 0
Bool XmuDQDestroy(\fIq\fP, \fIdocallbacks\fP)
.br
      XmuDisplayQueue *\fIq\fP;
.br
      Bool \fIdocallbacks\fP;
.FN
.LP
This function releases all memory associated with the specified queue.
If docallbacks is
.PN True ,
then the queue's closefunc callback (if non-NULL) is first called
for each display in the queue, even though
.PN XCloseDisplay
is not called on the display.
.LP
.NH 1
Toolkit Convenience Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/Initer.h >.
.sp
.FD 0
void XmuAddInitializer(\fIfunc\fP, \fIdata\fP) 
.br
      void (*\fIfunc\fP)();
.br
      caddr_t \fIdata\fP;
.FN
.LP
This function registers a callback, to be invoked the first time
.PN XmuCallInitializers
is called on a given application context.  The function
is called with the application context and the specified data:
.LP
	(*func)(app_con, data)
.sp
.FD 0
void XmuCallInitializers(\fIapp_con\fP)
.br
      XtAppContext \fIapp_con\fP;
.FN
.LP
This function calls each of the callbacks that have been registered with
.PN XmuAddInitializer ,
if this is the first time the application context has been passed to
.PN XmuCallInitializers .
Otherwise, this function does nothing.
.LP
.NH 1
Standard Colormap Functions
.LP
To use the functions defined in this section, you should include the header
file
.Pn < X11/Xmu/StdCmap.h >.
.sp
.FD 0
Status XmuAllStandardColormaps(\fIdpy\fP)
.br
      Display *\fIdpy\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection to the X server
.LP
To create all of the appropriate standard colormaps for every visual of
every screen on a given display, use
.PN XmuAllStandardColormaps .
.LP
This function defines and retains as permanent resources all standard
colormaps which are meaningful for the visuals of each screen of the
display.  It returns 0 on failure, non-zero on success.  If the property of
any standard colormap is already defined, this function will redefine it.
.LP
This function is intended to be used by window managers or a special client
at the start of a session.
.LP
The standard colormaps of a screen are defined by properties associated with
the screen's root window.  The property names of standard colormaps are
predefined, and each property name may describe at most one colormap.
.LP  
The standard colormaps are: RGB_BEST_MAP, RGB_RED_MAP, RGB_GREEN_MAP,
RGB_BLUE_MAP, RGB_DEFAULT_MAP, and RGB_GRAY_MAP.  Therefore a screen may
have at most 6 standard colormap properties defined.
.LP
A standard colormap is associated with a particular visual of the screen.  A
screen may have multiple visuals defined, including visuals of the same
class at different depths.  Note that a visual id might be repeated for more
than one depth, so the visual id and the depth of a visual identify the
visual.  The characteristics of the visual will determine which standard
colormaps are meaningful under that visual, and will determine how the
standard colormap is defined.  Because a standard colormap is associated
with a specific visual, there must be a method of determining which visuals
take precedence in defining standard colormaps.
.LP
The method used here is: for the visual of greatest depth, define all
standard colormaps meaningful to that visual class, according to this order
of (descending) precedence:
.PN DirectColor ;
.PN PseudoColor ;
.PN TrueColor
and
.PN GrayScale ;
and finally
.PN StaticColor
and
.PN StaticGray .
.LP
This function allows success, on a per screen basis.  For example, if a map
on screen 1 fails, the maps on screen 0, created earlier, will remain.
However, none on screen 1 will remain.  If a map on screen 0 fails, none
will remain.
.LP
See
.PN XmuVisualStandardColormaps
for which standard colormaps are meaningful under these classes of visuals.
.sp
.FD 0
Status XmuGetColormapAllocation(\fIvinfo\fP, \fIproperty\fP, \fIred_max\fP, \fIgreen_max\fP, \fIblue_max\fP)
.br
      XVisualInfo *\fIvinfo\fP;
.br
      Atom \fIproperty\fP;
.br
      unsigned long *\fIred_max\fP, *\fIgreen_max\fP, *\fIblue_max\fP;
.FN
.IP \fIvinfo\fP 1i
specifies visual information for a chosen visual
.IP \fIproperty\fP 1i
specifies one of the standard colormap property names
.IP \fIred_max\fP 1i
returns maximum red value 
.IP \fIgreen_max\fP 1i
returns maximum green value
.IP \fIblue_max\fP 1i
returns maximum blue value
.LP
To determine the best allocation of reds, greens, and blues in a standard
colormap, use
.PN XmuGetColormapAllocation .
.LP
.PN XmuGetColormapAllocation
returns 0 on failure, non-zero on success.  It is
assumed that the visual is appropriate for the colormap property.
.sp
.FD 0
Status XmuCreateColormap(\fIdpy\fP, \fIcolormap\fP)
.br
      Display *\fIdpy\fP;
.br
      XStandardColormap *\fIcolormap\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection under which the map is created
.IP \fIcolormap\fP 1i
specifies the map to be created
.FN
.LP
To create any one colormap which is described by an
.PN XStandardColormap
structure, use
.PN XmuCreateColormap .
.LP
This function returns 0 on failure, and non-zero on success.  The base_pixel
of the colormap is set on success.  Resources created by this function are
not made permanent.  No argument error checking is provided; use at your own
risk.
.LP
All colormaps are created with read-only allocations, with the exception of
read-only allocations of colors in the default map or which otherwise fail
to return the expected pixel value, and these are individually defined as
read/write allocations.  This is done so that all the cells defined in the
default map are contiguous, for use in image processing.  This typically
happens with White and Black in the default map.
.LP 
Colormaps of static visuals are considered to be successfully created if the
map of the static visual matches the definition given in the standard
colormap structure.
.sp
.FD 0
void XmuDeleteStandardColormap(\fIdpy\fP, \fIscreen\fP, \fIproperty\fP)
.br
      Display *\fIdpy\fP;
.br
      int \fIscreen\fP;
.br
      Atom \fIproperty\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection to the X server
.IP \fIscreen\fP 1i
specifies the screen of the display
.IP \fIproperty\fP 1i
specified the standard colormap property
.LP
To remove any standard colormap property, use
.PN XmuDeleteStandardColormap .
This function will remove the specified property from the specified screen,
releasing any resources used by the colormap(s) of the property, if
possible.
.sp
.FD 0
Status XmuLookupStandardColormap(\fIdpy\fP, \fIscreen\fP, \fIvisualid\fP, \fIdepth\fP, \fIproperty\fP, \fIreplace\fP, \fIretain\fP)
.br
      Display *\fIdpy\fP;
.br
      int \fIscreen\fP;
.br
      VisualID \fIvisualid\fP;
.br
      unsigned int \fIdepth\fP;
.br
      Atom \fIproperty\fP;
.br
      Bool \fIreplace\fP;
.br
      Bool \fIretain\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection to the X server
.IP \fIscreen\fP 1i
specifies the screen of the display
.IP \fIvisualid\fP 1i
specifies the visual type
.IP \fIdepth\fP 1i
specifies the visual type
.IP \fIproperty\fP 1i
specifies the standard colormap property
.IP \fIreplace\fP 1i
specifies whether or not to replace
.IP \fIretain\fP 1i
specifies whether or not to retain
.LP
To create a standard colormap if one does not currently exist, or replace
the currently existing standard colormap, use
.PN XmuLookupStandardColormap .
.LP
Given a screen, a visual, and a property, this function will determine the
best allocation for the property under the specified visual, and determine
the whether to create a new colormap or to use the default colormap of the
screen.
.LP
If replace is True, any previous definition of the property will be
replaced.  If retain is True, the property and the colormap will be made
permanent for the duration of the server session.  However, pre-existing
property definitions which are not replaced cannot be made permanent by a
call to this function; a request to retain resources pertains to newly
created resources.
.LP
This function returns 0 on failure, non-zero on success.  A request to
create a standard colormap upon a visual which cannot support such a map is
considered a failure.  An example of this would be requesting any standard
colormap property on a monochrome visual, or, requesting an RGB_BEST_MAP on
a display whose colormap size is 16.
.sp
.FD 0
XStandardColormap *XmuStandardColormap(\fIdpy\fP, \fIscreen\fP, \fIvisualid\fP, \fIdepth\fP, \fIproperty\fP,
                                       \fIcmap\fP, \fIred_max\fP, \fIgreen_max\fP, \fIblue_max\fP)
.br
      Display \fIdpy\fP;
.br
      int \fIscreen\fP;
.br
      VisualID \fIvisualid\fP;
.br
      unsigned int \fIdepth\fP;
.br
      Atom \fIproperty\fP;
.br
      Colormap \fIcmap\fP;
.br
      unsigned long \fIred_max\fP, \fIgreen_max\fP, \fIblue_max\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection to the X server
.IP \fIscreen\fP 1i
specifies the screen of the display
.IP \fIvisualid\fP 1i
specifies the visual type
.IP \fIdepth\fP 1i
specifies the visual depth
.IP \fIproperty\fP 1i
specifies the standard colormap property
.IP \fIcmap\fP 1i
specifies the colormap ID, or None
.IP \fIred_max\fP 1i
specifies the red allocation
.IP \fIgreen_max\fP 1i
specifies the green allocation
.IP \fIblue_max\fP 1i
specifies the blue allocation
.LP
To create any one standard colormap, use
.PN XmuStandardColormap .
.LP
This function creates a standard colormap for the given screen, visualid,
and visual depth, with the given red, green, and blue maximum values, with
the given standard property name.  Upon success, it returns a pointer to an
.PN XStandardColormap
structure which describes the newly created colormap.
Upon failure, it returns NULL.
.LP 
Resources created by this function are not made permanent; that is the
caller's responsibility.
.sp
.FD 0
Status XmuVisualStandardColormaps(\fIdpy\fP, \fIscreen\fP, \fIvisualid\fP, \fIdepth\fP, \fIreplace\fP, \fIretain\fP)
.br
      Display *\fIdpy\fP;
.br
      int \fIscreen\fP;
.br
      VisualID \fIvisualid\fP;
.br
      unsigned int \fIdepth\fP;
.br
      Bool \fIreplace\fP;
.br
      Bool \fIretain\fP;
.FN
.IP \fIdpy\fP 1i
specifies the connection to the X server
.IP \fIscreen\fP 1i
specifies the screen of the display
.IP \fIvisualid\fP 1i
specifies the visual type
.IP \fIdepth\fP 1i
specifies the visual depth
.IP \fIreplace\fP 1i
specifies whether or not to replace
.IP \fIretain\fP 1i
specifies whether or not to retain
.LP
To create all of the appropriate standard colormaps for a given visual on a
given screen, use
.PN XmuVisualStandardColormaps .
.LP
This function defines all appropriate standard colormap properties for the
given visual.  If replace is
.PN True ,
any previous definition will be removed.
If retain is
.PN True ,
new properties will be retained for the duration of the
server session.  This function returns 0 on failure, non-zero on success.
On failure, no new properties will be defined, but old ones may have been
removed if replace was True.
.LP
Not all standard colormaps are meaningful to all visual classes.  This
routine will check and define the following properties for the following
classes, provided that the size of the colormap is not too small.  For
.PN DirectColor
and
.PN PseudoColor :
RGB_DEFAULT_MAP, RGB_BEST_MAP, RGB_RED_MAP,
RGB_GREEN_MAP, RGB_BLUE_MAP, and RGB_GRAY_MAP.  For
.PN TrueColor
and
.PN StaticColor :
RGB_BEST_MAP.  For
.PN GrayScale
and
.PN StaticGray :
RGB_GRAY_MAP.
