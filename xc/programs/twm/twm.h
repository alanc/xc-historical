/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute Of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/


/***********************************************************************
 *
 * $XConsortium: twm.h,v 1.35 89/07/26 11:02:56 jim Exp $
 *
 * twm include file
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef _TWM_
#define _TWM_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#ifndef WithdrawnState
#define WithdrawnState 0
#endif

#ifdef SIGNALRETURNSINT
typedef int (*SigProc)();	/* type of function returned by signal() */
#else
typedef void (*SigProc)();	/* type of function returned by signal() */
#endif


#define BW 2			/* border width */
#define BW2 4			/* border width  * 2 */

/* directory to look for bitmaps if the file is not found in the current
 * directory 
 */
#define BITMAPS "/usr/include/X11/bitmaps"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define MAX_BUTTONS	5	/* max mouse buttons supported */

/* info stings defines */
#define INFO_LINES 30
#define INFO_SIZE 200

/* contexts for button presses */
#define C_NO_CONTEXT	-1
#define C_WINDOW	0
#define C_TITLE		1
#define C_ICON		2
#define C_ROOT		3
#define C_FRAME		4
#define C_ICONMGR	5
#define C_NAME		6
#define NUM_CONTEXTS	7

#define C_WINDOW_BIT	(1 << C_WINDOW)
#define C_TITLE_BIT	(1 << C_TITLE)
#define C_ICON_BIT	(1 << C_ICON)
#define C_ROOT_BIT	(1 << C_ROOT)
#define C_FRAME_BIT	(1 << C_FRAME)
#define C_ICONMGR_BIT	(1 << C_ICONMGR)
#define C_NAME_BIT	(1 << C_NAME)

#define C_ALL_BITS	(C_WINDOW_BIT | C_TITLE_BIT | C_ICON_BIT |\
			 C_ROOT_BIT | C_FRAME_BIT | C_ICONMGR_BIT)

/* modifiers for button presses */
#define MOD_SIZE	((ShiftMask | ControlMask | Mod1Mask) + 1)

#define TITLE_BAR_SPACE         1	/* 2 pixel space bordering chars */
#define TITLE_BAR_FONT_HEIGHT   15	/* max of 15 pixel high chars */
#define TITLE_BAR_HEIGHT        (TITLE_BAR_FONT_HEIGHT+(2*TITLE_BAR_SPACE))

/* defines for zooming/unzooming */
#define ZOOM_NONE 0

#define FBF(fix_fore, fix_back, fix_font)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    Gcv.font = fix_font;\
    XChangeGC(dpy, Scr->NormalGC, GCFont|GCForeground|GCBackground,&Gcv)

#define FB(fix_fore, fix_back)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    XChangeGC(dpy, Scr->NormalGC, GCForeground|GCBackground,&Gcv)

typedef struct MyFont
{
    char *name;			/* name of the font */
    XFontStruct *font;		/* font structure */
    int height;			/* height of the font */
    int y;			/* Y coordinate to draw characters */
} MyFont;

typedef struct ColorPair
{
    int fore;
    int back;
} ColorPair;


/* for each window that is on the display, one of these structures
 * is allocated and linked into a list 
 */
typedef struct TwmWindow
{
    struct TwmWindow *next;	/* next twm window */
    struct TwmWindow *prev;	/* previous twm window */
    Window w;			/* the child window */
    int bw;			/* the child window border width */
    int old_bw;			/* border width before reparenting */
    Window frame;		/* the frame window */
    Window title_w;		/* the title bar window */
    Window iconify_w;		/* the iconify button */
    Window resize_w;		/* the resize button */
    Window hilite_w;		/* the hilite window */
    Pixmap gray;
    Window icon_w;		/* the icon window */
    Window icon_bm_w;		/* the icon bitmap window */
    int frame_x;		/* x position of frame */
    int frame_y;		/* y position of frame */
    int frame_width;		/* width of frame */
    int frame_height;		/* height of frame */
    int frame_bw;		/* borderwidth of frame */
    int title_x;
    int title_y;
    int title_bw;		/* borderwidth of titlebar */
    int icon_x;			/* icon text x coordinate */
    int icon_y;			/* icon text y coordiante */
    int icon_w_width;		/* width of the icon window */
    int icon_w_height;		/* height of the icon window */
    int icon_width;		/* width of the icon bitmap */
    int icon_height;		/* height of the icon bitmap */
    int title_height;		/* height of the title bar */
    char *full_name;		/* full name of the window */
    char *name;			/* name of the window */
    char *icon_name;		/* name of the icon */
    int name_width;		/* width of name text */
    XWindowAttributes attr;	/* the child window attributes */
    XSizeHints hints;		/* normal hints */
    XWMHints *wmhints;		/* WM hints */
    int group;			/* group ID */
    int frame_vis;		/* frame visibility */
    int icon_vis;		/* icon visibility */
    XClassHint class;
    struct WList *list;
    /***********************************************************************
     * color definitions per window
     **********************************************************************/
    int border;			/* border color */
    int icon_border;		/* border color */
    ColorPair border_tile;
    ColorPair title;
    ColorPair iconc;
    short iconified;		/* has the window ever been iconified? */
    short icon;			/* is the window an icon now ? */
    short icon_on;		/* is the icon visible */
    short mapped;		/* is the window mapped ? */
    short auto_raise;		/* should we auto-raise this window ? */
    short forced;		/* has had an icon forced upon it */
    short highlight;		/* should highlight this window */
    short iconify_by_unmapping;	/* unmap window to iconify it */
    short iconmgr;		/* this is an icon manager window */
    short transient;		/* this is a transient window */
    short titlehighlight;	/* should I highlight the title bar */
    struct IconMgr *iconmgrp;	/* pointer to it if this is an icon manager */
    int save_frame_x;		/* x position of frame */
    int save_frame_y;		/* y position of frame */
    int save_frame_width;	/* width of frame */
    int save_frame_height;	/* height of frame */
    short zoomed;		/* is the window zoomed? */
#ifdef SHAPE
    short wShaped;		/* this window has a bounding shape */
    short fShaped;		/* regions of the frame have been shaped */
#endif
    unsigned long protocols;	/* which protocols this window handles */
} TwmWindow;

#define DoesWmTakeFocus		(1L << 0)
#define DoesWmSaveYourself	(1L << 1)
#define DoesWmDeleteWindow	(1L << 2)


extern Display *dpy;
extern Window ResizeWindow;	/* the window we are resizing */
#ifdef SHAPE
extern int HasShape;		/* this server supports Shape extension */
#endif

extern int PreviousScreen;

extern Cursor UpperLeftCursor;
extern Cursor RightButt;
extern Cursor MiddleButt;
extern Cursor LeftButt;

extern XClassHint NoClass;

extern XContext TwmContext;
extern XContext MenuContext;
extern XContext IconManagerContext;
extern XContext ScreenContext;

extern char Version[100];

extern char *Home;
extern int HomeLen;
extern int ParseError;

extern int TitleBarX;
extern int HandlingEvents;

extern Window JunkRoot;
extern Window JunkParent;
extern Window JunkChild;
extern int JunkX;
extern int JunkY;
extern int JunkWidth;
extern int JunkHeight;
extern int JunkDepth;
extern int JunkBW;
extern int JunkMask;
extern XGCValues Gcv;
extern int InfoLines;
extern char Info[][INFO_SIZE];
extern int Argc;
extern char **Argv;
extern char **Environ;

extern void Done();
extern int Error();
extern int Other();

extern Bool ErrorOccurred;
extern XErrorEvent LastErrorEvent;

#define ResetError() (ErrorOccurred = False)

extern Bool RestartPreviousState;
extern Bool GetWMState();

extern Atom _XA_WM_CHANGE_STATE;
extern Atom _XA_WM_STATE;
extern Atom _XA_WM_COLORMAP_WINDOWS;
extern Atom _XA_WM_PROTOCOLS;
extern Atom _XA_WM_TAKE_FOCUS;
extern Atom _XA_WM_SAVE_YOURSELF;
extern Atom _XA_WM_DELETE_WINDOW;

#endif /* _TWM_ */
