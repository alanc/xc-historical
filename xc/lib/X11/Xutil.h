/* $XConsortium: Xutil.h,v 11.48 89/07/25 13:03:44 jim Exp $ */

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XUTIL_H_
#define _XUTIL_H_

/* 
 * Bitmask returned by XParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020

/*
 * new version containing base_width, base_height, and win_gravity fields;
 * used with WM_NORMAL_HINTS.
 */
typedef struct {
    	long flags;	/* marks which fields in this structure are defined */
	int x, y;		/* obsolete for new window mgrs, but clients */
	int width, height;	/* should set so old wm's don't mess up */
	int min_width, min_height;
	int max_width, max_height;
    	int width_inc, height_inc;
	struct {
		int x;	/* numerator */
		int y;	/* denominator */
	} min_aspect, max_aspect;
	int base_width, base_height;		/* added by ICCCM version 1 */
	int win_gravity;			/* added by ICCCM version 1 */
} XSizeHints;

/*
 * The next block of definitions are for window manager properties that
 * clients and applications use for communication.
 */

/* flags argument in size hints */
#define USPosition	(1L << 0) /* user specified x, y */
#define USSize		(1L << 1) /* user specified width, height */

#define PPosition	(1L << 2) /* program specified position */
#define PSize		(1L << 3) /* program specified size */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */
#define PResizeInc	(1L << 6) /* program specified resize increments */
#define PAspect		(1L << 7) /* program specified min and max aspect ratios */
#define PBaseSize	(1L << 8) /* program specified base for incrementing */
#define PWinGravity	(1L << 9) /* program specified window gravity */

/* obsolete */
#define PAllHints (PPosition|PSize|PMinSize|PMaxSize|PResizeInc|PAspect)



typedef struct {
	long flags;	/* marks which fields in this structure are defined */
	Bool input;	/* does this application rely on the window manager to
			get keyboard input? */
	int initial_state;	/* see below */
	Pixmap icon_pixmap;	/* pixmap to be used as icon */
	Window icon_window; 	/* window to be used as icon */
	int icon_x, icon_y; 	/* initial position of icon */
	Pixmap icon_mask;	/* icon mask bitmap */
	XID window_group;	/* id of related window group */
	/* this structure may be extended in the future */
} XWMHints;

/* definition for flags of XWMHints */

#define InputHint 		(1L << 0)
#define StateHint 		(1L << 1)
#define IconPixmapHint		(1L << 2)
#define IconWindowHint		(1L << 3)
#define IconPositionHint 	(1L << 4)
#define IconMaskHint		(1L << 5)
#define WindowGroupHint		(1L << 6)
#define AllHints (InputHint|StateHint|IconPixmapHint|IconWindowHint| \
IconPositionHint|IconMaskHint|WindowGroupHint)

/* definitions for initial window state */
#define WithdrawnState 0	/* for windows that are not mapped */
#define NormalState 1	/* most applications want to start this way */
#define IconicState 3	/* application wants to start as an icon */

/*
 * Obsolete states no longer defined by ICCCM
 */
#define DontCareState 0	/* don't know or care */
#define ZoomState 2	/* application wants to start zoomed */
#define InactiveState 4	/* application believes it is seldom used; some
    			   wm's may put it on inactive menu */


/*
 * new structure for manipulating TEXT properties; used with WM_NAME, 
 * WM_ICON_NAME, WM_CLIENT_MACHINE, and WM_COMMAND.
 */
typedef struct {
    unsigned char *value;		/* same as Property routines */
    Atom encoding;			/* prop type */
    int format;				/* prop data format: 8, 16, or 32 */
    unsigned long nitems;		/* number of data items in value */
} XTextProperty;


typedef struct {
	int min_width, min_height;
	int max_width, max_height;
	int width_inc, height_inc;
} XIconSize;

typedef struct {
	char *res_name;
	char *res_class;
} XClassHint;

/*
 * These macros are used to give some sugar to the image routines so that
 * naive people are more comfortable with them.
 */
#define XDestroyImage(ximage) \
	((*((ximage)->f.destroy_image))((ximage)))
#define XGetPixel(ximage, x, y) \
	((*((ximage)->f.get_pixel))((ximage), (x), (y)))
#define XPutPixel(ximage, x, y, pixel) \
	((*((ximage)->f.put_pixel))((ximage), (x), (y), (pixel)))
#define XSubImage(ximage, x, y, width, height)  \
	((*((ximage)->f.sub_image))((ximage), (x), (y), (width), (height)))
#define XAddPixel(ximage, value) \
	((*((ximage)->f.add_pixel))((ximage), (value)))

/*
 * Compose sequence status structure, used in calling XLookupString.
 */
typedef struct _XComposeStatus {
    char *compose_ptr;		/* state table pointer */
    int chars_matched;		/* match state */
} XComposeStatus;

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 */
#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
  (((unsigned)(keysym) >= XK_Home)     && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_F1)     && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_F1)       && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_Select)   && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
  (((unsigned)(keysym) >= XK_Shift_L)  && ((unsigned)(keysym) <= XK_Hyper_R))

/*
 * opaque reference to Region data type 
 */
typedef struct _XRegion *Region; 

/* Return values from XRectInRegion() */
 
#define RectangleOut 0
#define RectangleIn  1
#define RectanglePart 2
 

/*
 * Information used by the visual utility routines to find desired visual
 * type from the many visuals a display may support.
 */

typedef struct {
  Visual *visual;
  VisualID visualid;
  int screen;
  int depth;
  int class;
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  int colormap_size;
  int bits_per_rgb;
} XVisualInfo;

#define VisualNoMask		0x0
#define VisualIDMask 		0x1
#define VisualScreenMask	0x2
#define VisualDepthMask		0x4
#define VisualClassMask		0x8
#define VisualRedMaskMask	0x10
#define VisualGreenMaskMask	0x20
#define VisualBlueMaskMask	0x40
#define VisualColormapSizeMask	0x80
#define VisualBitsPerRGBMask	0x100
#define VisualAllMask		0x1FF

/*
 * This defines a window manager property that clients may use to
 * share standard color maps of type RGB_COLOR_MAP:
 */
typedef struct {
	Colormap colormap;
	unsigned long red_max;
	unsigned long red_mult;
	unsigned long green_max;
	unsigned long green_mult;
	unsigned long blue_max;
	unsigned long blue_mult;
	unsigned long base_pixel;
	VisualID visualid;		/* added by ICCCM version 1 */
	XID killid;			/* added by ICCCM version 1 */
} XStandardColormap;

#define ReleaseByFreeingColormap ((XID) 1L)  /* for killid field above */


/*
 * return codes for XReadBitmapFile and XWriteBitmapFile
 */
#define BitmapSuccess		0
#define BitmapOpenFailed 	1
#define BitmapFileInvalid 	2
#define BitmapNoMemory		3
/*
 * Declare the routines that don't return int.
 */

/****************************************************************
 *
 * Context Management
 *
 ****************************************************************/


/* Associative lookup table return codes */

#define XCSUCCESS 0	/* No error. */
#define XCNOMEM   1    /* Out of memory */
#define XCNOENT   2    /* No entry in table */

typedef int XContext;

#define XUniqueContext()       ((XContext) XrmUniqueQuark())
#define XStringToContext(string)   ((XContext) XrmStringToQuark(string))

extern int XSaveContext(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XContext		/* context */,
    caddr_t		/* data */
#endif
);

extern int XFindContext(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XContext		/* context */,
    caddr_t*		/* data_return */
#endif
);

extern int XDeleteContext(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XContext		/* context */
#endif
);


XWMHints *XGetWMHints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    window		/* w */		      
#endif
);
Region XCreateRegion(
#if NeedFunctionPrototypes
    void
#endif
);
Region XPolygonRegion(
#if NeedFunctionPrototypes
    XPoint*		/* points */,
    int			/* n */,
    int			/* fill_rule */
#endif
);
XImage *XCreateImage(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Visual*		/* visual */,
    unsigned int	/* depth */,
    int			/* format */,
    int			/* offset */,
    char*		/* data */,
    unsigned int	/* width */,
    unsigned int	/* height */,
    int			/* bitmap_pad */,
    int			/* bytes_per_line */
#endif
);

XVisualInfo *XGetVisualInfo(
#if NeedFunctionPrototypes
    Display*		/* display */,
    long		/* vinfo_mask */,
    XVisualInfo*	/* vinfo_template */,
    int*		/* nitems_return */
#endif
);

/* Allocation routines for properties that may get longer */
XSizeHints *XAllocSizeHints (
#if NeedFunctionPrototypes
    void
#endif
);
XStandardColormap *XAllocStandardColormap (
#if NeedFunctionPrototypes
    void
#endif
);
XWMHints *XAllocWMHints (
#if NeedFunctionPrototypes
    void
#endif
);
XClassHint *XAllocClassHint (
#if NeedFunctionPrototypes
    void
#endif
);
XIconSize *XAllocIconSize (
#if NeedFunctionPrototypes
    void
#endif
);

/* ICCCM routines for data structures defined in this file */
Status XGetWMSizeHints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XSizeHints*		/* hints_return */,
    long*		/* supplied_return */,
    Atom		/* property */
#endif
);
Status XGetWMNormalHints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XSizeHints*		/* hints_return */,
    long*		/* supplied_return */ 
#endif
);
Status XGetRGBColormaps(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XStandardColormap** /* stdcmap_return */,
    int*		/* count_return */,
    Atom		/* property */
#endif
);
Status XGetTextProperty(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* window */,
    XTextProperty*	/* text_property_return */,
    Atom		/* property */
#endif
);
Status XGetWMName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property_return */
#endif
);
Status XGetWMIconName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property_return */
#endif
);
Status XGetWMClientMachine(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property_return */
#endif
);
Status XGetWMCommand(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property_return */
#endif
);
Status XGetWMColormapWindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window**		/* colormap_windows_return */,
    int*		/* count_return */
#endif
);
Status XSetWMColormapWindows(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Window*		/* windows */,
    int			/* count */
#endif
);
void XSetWMProperties(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* windowName */,
    XTextProperty*	/* iconName */,
    char**		/* argv */,
    int			/* argc */,
    XSizeHints*		/* sizeHints */,
    XWMHints*		/* wmHints */,
    XClassHint*		/* classHints */
#endif
);
void XSetWMSizeHints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XSizeHints*		/* hints */,
    Atom		/* property */
#endif
);
void XSetWMNormalHints(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XSizeHints*		/* hints */
#endif
);
void XSetRGBColormaps(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XStandardColormap*	/* cmaps */,
    int			/* count */,
    Atom		/* property */
#endif
);
void XSetTextProperty(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    Atom		/* property */,
    XTextProperty*	/* text_property */
#endif
);
void XSetWMName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property */
#endif
);
void XSetWMIconName(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty*	/* text_property */
#endif
);
void XSetWMClientMachine(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty	/* text_property */
#endif
);
void XSetWMCommand(
#if NeedFunctionPrototypes
    Display*		/* display */,
    Window		/* w */,
    XTextProperty	/* text_property */
#endif
);
Status XStringListToTextProperty(
#if NeedFunctionPrototypes
    char**		/* argv */,
    int			/* argc */,
    XTextProperty*	/* text_property */
#endif
);
Status XTextPropertyToStringList(
#if NeedFunctionPrototypes
    XTextProperty*	/* text_property */,
    char***		/* list_return */,
    int*		/* count_return */
#endif
);
void XFreeStringList(
#if NeedFunctionPrototypes
    char**		/* list */
#endif
);

#endif /* _XUTIL_H_ */
