/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

/***********************************************************************
 *
 * $XConsortium: twm.c,v 1.42 89/05/04 19:02:57 keith Exp $
 *
 * twm - "Tom's Window Manager"
 *
 * 27-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[] =
"$XConsortium: twm.c,v 1.42 89/05/04 19:02:57 keith Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#ifdef REAL_TIME
#include <sys/rtprio.h>
#include <sys/lock.h>
#endif
#include "twm.h"
#include "add_window.h"
#include "gc.h"
#include "parse.h"
#include "version.h"
#include "menus.h"
#include "events.h"
#include "util.h"
#include "gram.h"
#include "screen.h"
#include "iconmgr.h"

#include "twm.bm"
#include "gray.bm"

Display *dpy;			/* which display are we talking to */
Window ResizeWindow;		/* the window we are resizing */

int MultiScreen = TRUE;		/* try for more than one screen? */
int NumScreens;			/* number of screens in ScreenList */
#ifdef SHAPE
int HasShape;			/* server supports shape extension? */
#endif
ScreenInfo **ScreenList;	/* structures for each screen */
ScreenInfo *Scr = NULL;	/* the current screen */
int FirstScreen;		/* TRUE ==> first screen of display */
static int RedirectError;	/* TRUE ==> another window manager running */
char Info[INFO_LINES][INFO_SIZE];		/* info strings to print */
int InfoLines;
char *InitFile = NULL;

Cursor UpperLeftCursor;		/* upper Left corner cursor */
Cursor RightButt;
Cursor MiddleButt;
Cursor LeftButt;

XContext TwmContext;		/* context for twm windows */
XContext MenuContext;		/* context for all menu windows */
XContext IconManagerContext;	/* context for all window list windows */
XContext ScreenContext;		/* context to get screen data */

XClassHint NoClass;		/* for applications with no class */

XGCValues Gcv;

char Version[100];		/* place to build the version string */

char *Home;			/* the HOME environment variable */
int HomeLen;			/* length of Home */
int ParseError;			/* error parsing the .twmrc file */

int TitleBarX;			/* x coordinate ditto */
int HandlingEvents = FALSE;	/* are we handling events yet? */

Window JunkRoot;		/* junk window */
Window JunkParent;		/* junk window */
Window JunkChild;		/* junk window */
int JunkX;			/* junk variable */
int JunkY;			/* junk variable */
int JunkWidth;			/* junk variable */
int JunkHeight;			/* junk variable */
int JunkDepth;			/* junk variable */
int JunkBW;			/* junk variable */
int JunkMask;			/* junk variable */

int Argc;
char **Argv;
char **Environ;

unsigned long black, white;

/***********************************************************************
 *
 *  Procedure:
 *	main - start of twm
 *
 ***********************************************************************
 */

main(argc, argv, environ)
    int argc;
    char **argv;
    char **environ;
{
    Window w, root, parent, *children;
    TwmWindow *tmp_win;
    int nchildren, i;
    int m, d, y;
    char *display_name;
    unsigned long valuemask;	/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    SigProc old_handler;
    int mask;
    int numManaged, firstscrn, lastscrn, scrnum;

#ifdef REAL_TIME
    int prior;

    prior = rtprio(0, 5);
    if (prior == -1)
	fprintf(stderr, "twm: real time priority failed\n");
    else
	fprintf(stderr, "twm: real time priority set (%d)\n", prior);
    if (plock(PROCLOCK))
	fprintf(stderr, "twm: process not locked\n");
    else
	fprintf(stderr, "twm: process locked\n");
#endif /* REAL_TIME */

    display_name = NULL;

    Argc = argc;
    Argv = argv;
    Environ = environ;

    if (argc > 5)
    {
	fprintf(stderr,
	"Usage: twm [-display display] [-f init_file] [-singlescreen]\n");
	exit(1);
    }

    for (i = 1; i < argc; i++)
    {
	if (strncmp(argv[i], "-d", 2) == 0)
	    display_name = argv[++i];
	else if (strncmp(argv[i], "-s", 2) == 0)
	    MultiScreen = FALSE;
	else if (strncmp(argv[i], "-f", 2) == 0)
	    InitFile = argv[++i];
	else
	{
	    fprintf(stderr,
	    "Usage: twm [-display display] [-f init_file] [-singlescreen]\n");
	    exit(1);
	}
    }

    old_handler = signal(SIGINT, SIG_IGN);
    if (old_handler != SIG_IGN)
	signal(SIGINT, Done);

    old_handler = signal(SIGHUP, SIG_IGN);
    if (old_handler != SIG_IGN)
	signal(SIGHUP, Done);

    signal(SIGQUIT, Done);
    signal(SIGTERM, Done);

    Home = (char *)getenv("HOME");
    if (Home == NULL)
	Home = "./";

    HomeLen = strlen(Home);

    NoClass.res_name = NoName;
    NoClass.res_class = NoName;

    if ((dpy = XOpenDisplay(display_name)) == NULL)
    {
	if (display_name == NULL)
	    fprintf(stderr, "twm: can't open NULL display\n");
	else
	    fprintf(stderr, "twm: can't open display \"%s\"\n",
		display_name);
	exit(1);
    }


    if (fcntl(ConnectionNumber(dpy), F_SETFD, 1) == -1)
    {
	fprintf(stderr, "twm: child cannot disinherit TCP fd\n");
	exit(1);
    }

#ifdef SHAPE
    HasShape = XShapeQueryExtension (dpy);
#endif
    TwmContext = XUniqueContext();
    MenuContext = XUniqueContext();
    IconManagerContext = XUniqueContext();
    ScreenContext = XUniqueContext();

    /* Set up the per-screen global information. */

    NumScreens = ScreenCount(dpy);

    if (MultiScreen)
    {
	firstscrn = 0;
	lastscrn = NumScreens - 1;
    }
    else
    {
	firstscrn = lastscrn = DefaultScreen(dpy);
    }

    /* for simplicity, always allocate NumScreens ScreenInfo struct pointers */
    ScreenList = (ScreenInfo **) calloc (NumScreens, sizeof (ScreenInfo *));
    numManaged = 0;
    FirstScreen = TRUE;
    for (scrnum = firstscrn ; scrnum <= lastscrn; scrnum++)
    {
	RedirectError = FALSE;
	XSetErrorHandler(Other);
	XSelectInput(dpy, RootWindow (dpy, scrnum),
	    ColormapChangeMask |
	    SubstructureRedirectMask | KeyPressMask |
	    ButtonPressMask | ButtonReleaseMask | ExposureMask);
	XSync(dpy, 0);
	XSetErrorHandler(Error);

	if (RedirectError)
	{
	    fprintf(stderr, "twm:  Are you running another window manager");
	    if (MultiScreen && NumScreens > 0)
		fprintf(stderr, " on screen %d?\n", scrnum);
	    else
		fprintf(stderr, "?\n");
	    continue;
	}

	numManaged ++;

	/* Note:  ScreenInfo struct is calloc'ed to initialize to zero. */
	Scr = ScreenList[scrnum] = 
	    (ScreenInfo *) calloc(1, sizeof(ScreenInfo));

	/* initialize list pointers, remember to put an initialization
	 * in InitVariables also
	 */
	Scr->BorderColorL = NULL;
	Scr->IconBorderColorL = NULL;
	Scr->BorderTileForegroundL = NULL;
	Scr->BorderTileBackgroundL = NULL;
	Scr->TitleForegroundL = NULL;
	Scr->TitleBackgroundL = NULL;
	Scr->IconForegroundL = NULL;
	Scr->IconBackgroundL = NULL;
	Scr->NoTitle = NULL;
	Scr->MakeTitle = NULL;
	Scr->AutoRaise = NULL;
	Scr->IconNames = NULL;
	Scr->NoHighlight = NULL;
	Scr->NoTitleHighlight = NULL;
	Scr->DontIconify = NULL;
	Scr->IconMgrNoShow = NULL;
	Scr->IconMgrShow = NULL;
	Scr->IconifyByUn = NULL;
	Scr->IconManagerFL = NULL;
	Scr->IconManagerBL = NULL;
	Scr->IconMgrs = NULL;
	Scr->StartIconified = NULL;
	/* remember to put an initialization in InitVariables also
	 */

	Scr->screen = scrnum;
	Scr->d_depth = DefaultDepth(dpy, scrnum);
	Scr->d_visual = DefaultVisual(dpy, scrnum);
	Scr->Root = RootWindow(dpy, scrnum);
	XSaveContext (dpy, Scr->Root, ScreenContext, Scr);
	Scr->CMap = DefaultColormap(dpy, scrnum);
	Scr->MyDisplayWidth = DisplayWidth(dpy, scrnum);
	Scr->MyDisplayHeight = DisplayHeight(dpy, scrnum);

	/* setup default colors */
	black = Scr->Black = BlackPixel(dpy, scrnum);
	white = Scr->White = WhitePixel(dpy, scrnum);

	if (DisplayCells(dpy, scrnum) < 3)
	    Scr->Monochrome = MONOCHROME;
	else
	    Scr->Monochrome = COLOR;

	if (FirstScreen)
	{
	    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);

	    /* define cursors */

	    NewFontCursor(&UpperLeftCursor, "top_left_corner");
	    NewFontCursor(&RightButt, "rightbutton");
	    NewFontCursor(&LeftButt, "leftbutton");
	    NewFontCursor(&MiddleButt, "middlebutton");
	}

	Scr->iconmgr.x = 0;
	Scr->iconmgr.y = 0;
	Scr->iconmgr.width = 150;
	Scr->iconmgr.height = 5;
	Scr->iconmgr.next = NULL;
	Scr->iconmgr.prev = NULL;
	Scr->iconmgr.lasti = &(Scr->iconmgr);
	Scr->iconmgr.first = NULL;
	Scr->iconmgr.last = NULL;
	Scr->iconmgr.active = NULL;
	Scr->iconmgr.scr = Scr;
	Scr->iconmgr.columns = 1;
	Scr->iconmgr.count = 0;
	Scr->iconmgr.name = "TWM";
	Scr->iconmgr.icon_name = "Icons";

	Scr->IconDirectory = NULL;

	Scr->iconifyPm = NULL;
	Scr->focusPm = NULL;
	Scr->resizePm = NULL;
	Scr->siconifyPm = NULL;

	InitVariables();

	/* Parse it once for each screen. */
	ParseTwmrc(InitFile);
	CreateGCs();
	MakeMenus();

	Scr->TitleHeight = Scr->TitleBarFont.height + 2;
	Scr->TitleBarFont.y -= 1;

	/* the buttons need to be an odd width and height */
	if ((Scr->TitleHeight & 1) == 0)
	{
	    Scr->TitleHeight += 1;
	    Scr->TitleBarFont.y += 1;
	}

	TitleBarX = Scr->TitleHeight + 8;

	XGrabServer(dpy);
	XSync(dpy, 0);

	JunkX = 0;
	JunkY = 0;

	XQueryTree(dpy, Scr->Root, &root, &parent, &children, &nchildren);
	CreateIconManagers();
	if (!Scr->NoIconManagers)
	    Scr->iconmgr.twm_win->icon = TRUE;
	for (i = 0; i < nchildren; i++)
	{
	    if (MappedNotOverride(children[i]))
	    {
		XUnmapWindow(dpy, children[i]);
		Event.xmaprequest.window = children[i];
		HandleMapRequest();
	    }
	}

	if (Scr->ShowIconManager && !Scr->NoIconManagers)
	{
	    Scr->iconmgr.twm_win->icon = FALSE;
	    if (Scr->iconmgr.count)
	    {
		XMapWindow(dpy, Scr->iconmgr.w);
		XMapWindow(dpy, Scr->iconmgr.twm_win->frame);
	    }
	}

	Scr->InfoWindow = XCreateSimpleWindow(dpy, Scr->Root,
		0, 0, 5, 5, BW,
		Scr->DefaultC.fore,Scr->DefaultC.back);
	XSelectInput(dpy, Scr->InfoWindow, ExposureMask |
	    KeyPressMask | ButtonPressMask);

	Scr->InitialWindow = XCreateSimpleWindow(dpy, Scr->Root,
		0, 0, 5, Scr->InitialFont.height + 4, BW,
		Scr->DefaultC.fore,Scr->DefaultC.back);

	/* contruct the version string */
	strcpy(Version, &Date[7]);
	sscanf(Version, "%d/%d/%d", &y, &m, &d);
	sprintf(Version, "%s", &Revision[1]);
	Version[strlen(Version) - 1] = '\0';
	sscanf(&Date[7], "%d/%d/%d", &y, &m, &d);
	sprintf(Version, "%s  Date: %d/%d/%d %s", Version, m, d, y, &Date[16]);
	Version[strlen(Version) - 2] = '\0';

	Scr->VersionWindow = XCreateSimpleWindow(dpy, Scr->Root, 0, 0,
	    twm_width + 
		XTextWidth(Scr->VersionFont.font,Version,strlen(Version)) + 20,
	    Scr->VersionFont.height + 4, BW,
	    Scr->DefaultC.fore, Scr->DefaultC.back);

	FB(Scr->DefaultC.fore, Scr->DefaultC.back);
	valuemask = CWBackPixmap;
	attributes.background_pixmap =
	    XCreatePixmapFromBitmapData(dpy, Scr->VersionWindow,
		twm_bits, twm_width, twm_height,
		Scr->DefaultC.fore, Scr->DefaultC.back,
		Scr->d_depth);

	XCreateWindow(dpy, Scr->VersionWindow,
		      4, 1, twm_width, twm_height,
		      0, Scr->d_depth, CopyFromParent,
		      Scr->d_visual, valuemask, &attributes);

	XSelectInput(dpy, Scr->VersionWindow, ExposureMask);
	XMapSubwindows(dpy, Scr->VersionWindow);
	if (Scr->ShowVersion)
	    XMapWindow(dpy, Scr->VersionWindow);

	Scr->SizeWindow = XCreateSimpleWindow(dpy, Scr->Root,
		0, 0, 100, Scr->SizeFont.height + 4, BW,
		Scr->DefaultC.fore, Scr->DefaultC.back);

	XUngrabServer(dpy);

	FirstScreen = FALSE;
    	Scr->FirstTime = FALSE;
    } /* for */

    if (numManaged == 0)
    {
	if (MultiScreen && NumScreens > 0)
	    fprintf(stderr, "twm:  No unmanaged screens -- exiting.\n");
	exit (1);
    }


    HandlingEvents = TRUE;
    InitEvents();
    HandleEvents();
}

/***********************************************************************
 *
 *  Procedure:
 *	InitVariables - initialize twm variables
 *
 ***********************************************************************
 */

InitVariables()
{
    FreeList(&Scr->BorderColorL);
    FreeList(&Scr->IconBorderColorL);
    FreeList(&Scr->BorderTileForegroundL);
    FreeList(&Scr->BorderTileBackgroundL);
    FreeList(&Scr->TitleForegroundL);
    FreeList(&Scr->TitleBackgroundL);
    FreeList(&Scr->IconForegroundL);
    FreeList(&Scr->IconBackgroundL);
    FreeList(&Scr->NoTitle);
    FreeList(&Scr->MakeTitle);
    FreeList(&Scr->AutoRaise);
    FreeList(&Scr->IconNames);
    FreeList(&Scr->NoHighlight);
    FreeList(&Scr->NoTitleHighlight);
    FreeList(&Scr->DontIconify);
    FreeList(&Scr->IconMgrNoShow);
    FreeList(&Scr->IconMgrShow);
    FreeList(&Scr->IconifyByUn);
    FreeList(&Scr->IconManagerFL);
    FreeList(&Scr->IconManagerBL);
    FreeList(&Scr->IconMgrs);
    FreeList(&Scr->StartIconified);

    NewFontCursor(&Scr->FrameCursor, "top_left_arrow");
    NewFontCursor(&Scr->TitleCursor, "top_left_arrow");
    NewFontCursor(&Scr->IconCursor, "top_left_arrow");
    NewFontCursor(&Scr->IconMgrCursor, "top_left_arrow");
    NewFontCursor(&Scr->MoveCursor, "fleur");
    NewFontCursor(&Scr->ResizeCursor, "fleur");
    NewFontCursor(&Scr->MenuCursor, "sb_left_arrow");
    NewFontCursor(&Scr->ButtonCursor, "center_ptr");
    NewFontCursor(&Scr->WaitCursor, "watch");
    NewFontCursor(&Scr->SelectCursor, "dot");
    NewFontCursor(&Scr->DestroyCursor, "pirate");

    Scr->DefaultC.fore = black;
    Scr->DefaultC.back = white;
    Scr->BorderColor = black;
    Scr->BorderTileC.fore = black;
    Scr->BorderTileC.back = white;
    Scr->TitleC.fore = black;
    Scr->TitleC.back = white;
    Scr->MenuC.fore = black;
    Scr->MenuC.back = white;
    Scr->MenuTitleC.fore = black;
    Scr->MenuTitleC.back = white;
    Scr->MenuShadowColor = black;
    Scr->IconC.fore = black;
    Scr->IconC.back = white;
    Scr->IconBorderColor = black;
    Scr->IconManagerC.fore = black;
    Scr->IconManagerC.back = white;
    Scr->IconManagerHighlight = black;

    Scr->BorderWidth = BW;
    Scr->IconBorderWidth = BW;
    Scr->UnknownWidth = 0;
    Scr->UnknownHeight = 0;
    Scr->FocusRoot = TRUE;
    Scr->Focus = NULL;
    Scr->WarpCursor = FALSE;
    Scr->ForceIcon = FALSE;
    Scr->NoGrabServer = FALSE;
    Scr->NoRaiseMove = FALSE;
    Scr->NoRaiseResize = FALSE;
    Scr->NoRaiseDeicon = FALSE;
    Scr->DontMoveOff = FALSE;
    Scr->DoZoom = FALSE;
    Scr->TitleFocus = TRUE;
    Scr->NoTitlebar = FALSE;
    Scr->DecorateTransients = FALSE;
    Scr->IconifyByUnmapping = FALSE;
    Scr->ShowIconManager = FALSE;
    Scr->IconManagerDontShow =FALSE;
    Scr->BackingStore = TRUE;
    Scr->SaveUnder = TRUE;
    Scr->RandomPlacement = FALSE;
    Scr->OpaqueMove = FALSE;
    Scr->Highlight = TRUE;
    Scr->TitleHighlight = TRUE;
    Scr->MoveDelta = 0;
    Scr->ZoomCount = 8;
    Scr->SortIconMgr = FALSE;
    Scr->Shadow = TRUE;
    Scr->ShowVersion = TRUE;
    Scr->InterpolateMenuColors = FALSE;
    Scr->NoIconManagers = FALSE;
    Scr->ClientBorderWidth = FALSE;
    Scr->FirstRegion = NULL;
    Scr->LastRegion = NULL;
    Scr->FirstTime = TRUE;

    /* setup default fonts */
    Scr->TitleBarFont.font = NULL;
    Scr->TitleBarFont.name = "8x13";		GetFont(&Scr->TitleBarFont);
    Scr->MenuFont.font = NULL;
    Scr->MenuFont.name = "8x13";		GetFont(&Scr->MenuFont);
    Scr->IconFont.font = NULL;
    Scr->IconFont.name = "8x13";		GetFont(&Scr->IconFont);
    Scr->SizeFont.font = NULL;
    Scr->SizeFont.name = "fixed";		GetFont(&Scr->SizeFont);
    Scr->VersionFont.font = NULL;
    Scr->VersionFont.name = "8x13";		GetFont(&Scr->VersionFont);
    Scr->InitialFont.font = NULL;
    Scr->InitialFont.name = "9x15";		GetFont(&Scr->InitialFont);
    Scr->IconManagerFont.font = NULL;
    Scr->IconManagerFont.name = "8x13";		GetFont(&Scr->IconManagerFont);
    Scr->DefaultFont.font = NULL;
    Scr->DefaultFont.name = "fixed";		GetFont(&Scr->DefaultFont);

}

/***********************************************************************
 *
 *  Procedure:
 *	Done - cleanup and exit twm
 *
 *  Returned Value:
 *	none
 *
 *  Inputs:
 *	none
 *
 *  Outputs:
 *	none
 *
 *  Special Considerations:
 *	none
 *
 ***********************************************************************
 */

void
Reborder ()
{
    TwmWindow *tmp;			/* temp twm window structure */
    unsigned x, y;
    XWindowChanges xwc;		/* change window structure */
    unsigned int xwcm;		/* change window mask */
    int scrnum;

    /* put a border back around all windows */

    for (scrnum = 0; scrnum < NumScreens; scrnum++)
    {
	if ((Scr = ScreenList[scrnum]) == NULL)
	    continue;

	for (tmp = Scr->TwmRoot.next; tmp != NULL; tmp = tmp->next)
	{
	    XGetGeometry(dpy, tmp->w, &JunkRoot, &x,&y, &JunkWidth,&JunkHeight,
			 &JunkBW, &JunkDepth);

	    xwcm = CWX | CWY;
	    xwc.x = x;
	    xwc.y = y;

	    if (JunkBW != tmp->old_bw) {
	    	xwc.x = x - tmp->old_bw;
	    	xwc.y = y - tmp->old_bw;
	    	xwc.border_width = tmp->old_bw;
		xwcm |= CWBorderWidth;
	    }

	    XConfigureWindow(dpy, tmp->w, xwcm, &xwc);
	}
    }

    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void
Done()
{
    Reborder ();
    XCloseDisplay(dpy);
    exit(0);
}

/***********************************************************************
 *
 *  Procedure:
 *	Error - X error handler.  If we got here it is probably,
 *		because the client window went away and we haven't 
 *		got the DestroyNotify yet.
 *
 *  Inputs:
 *	dpy	- the connection to the X server
 *	event	- the error event structure
 *
 ***********************************************************************
 */

void
Error(dpy, event)
Display *dpy;
XErrorEvent *event;
{
    /* do nothing but absorb the error */
    return;
}

/***********************************************************************
 *
 *  Procedure:
 *	Other - error handler called if something else has set 
 *		the attributes on the root window.  Typically
 *		another window manager.
 *
 ***********************************************************************
 */

void
Other(dpy, event)
Display *dpy;
XErrorEvent *event;
{
    RedirectError = TRUE;
}
