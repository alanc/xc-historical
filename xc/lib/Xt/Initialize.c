#ifndef lint
static char *sccsid = "@(#)Initialize.c	1.0	8/2787";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* fix the tilebar name vs internal name junk */

#include <stdio.h>
#include <pwd.h>
#include <sys/param.h>

#include <X11/Xlib.h> 
#include <X11/Xutil.h>
 /* Xlib definitions  */
 /* things like Window, Display, XEvent are defined herein */
#include "Intrinsic.h"
#include "Atoms.h"
/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is appiled before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differening programs at this
 level.
*/

#define XtNinput	"input"
#define XtCInput	"Input"
#define XtNiconic	"iconic"
#define XtCIconic	"Iconic"
#define XtNinitial	"initialstate"
#define XtCInitial	"InitialState"
#define XtNiconName	"iconName"
#define XtCIconName	"IconName"
#define XtNiconPixmap	"iconPixmap"
#define XtCIconPixmap	"IconPixmap"
#define XtNallowtopresize	"allowTopResizeRequest"
#define XtCAllowtopresize	"AllowTopResizeRequest"
#define XtNgeometry	"geometry"
#define XtCGeometry	"Geometry"
#define XtNtitle	"title"
#define XtCTitle	"Title"

static XrmOptionDescRec opTable[] = {
{"=",		XtNgeometry,	XrmoptionIsArg,		(caddr_t) NULL},
{"-bd",		XtNborder,	XrmoptionSepArg,	(caddr_t) NULL},
{"-bordercolor",XtNborder,	XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",		XtNbackground,	XrmoptionSepArg,	(caddr_t) NULL},
{"-background",	XtNbackground,	XrmoptionSepArg,	(caddr_t) NULL},
{"-bw",		XtNborderWidth,	XrmoptionSepArg,	(caddr_t) NULL},
{"-border",	XtNborderWidth,	XrmoptionSepArg,	(caddr_t) NULL},
{"-fg",		XtNforeground,	XrmoptionSepArg,	(caddr_t) NULL},
{"-foreground",	XtNforeground,	XrmoptionSepArg,	(caddr_t) NULL},
{"-fn",		XtNfont,	XrmoptionSepArg,	(caddr_t) NULL},
{"-font",	XtNfont,	XrmoptionSepArg,	(caddr_t) NULL},
{"-rv",		XtNreverseVideo, XrmoptionNoArg,	(caddr_t) "on"},
{"-reverse",	XtNreverseVideo, XrmoptionNoArg,	(caddr_t) "on"},
{"+rv",		XtNreverseVideo, XrmoptionNoArg,	(caddr_t) "off"},
{"-n",		XtNname,	XrmoptionSepArg,	(caddr_t) NULL},
{"-name",	XtNname,	XrmoptionSepArg,	(caddr_t) NULL},
{"-title",	XtNtitle,	XrmoptionSepArg,	(caddr_t) NULL},
{"-t",		XtNtitle,	XrmoptionSepArg,	(caddr_t) NULL}
};

typedef struct {
	int	    argc;
	char      **argv;
	char	   *classname;
	char	   *icon_name;
	char	   *title;
	Pixmap	    icon_pixmap;
	Boolean	    iconic;
	Boolean	    input;
	Boolean	    resizeable;
	char       *geostr;
	int	    initial;
	XSizeHints  hints;
} TopLevel;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef  struct {
	Core core;
	Composite composite;
	TopLevel top;
} TopLevelWidgetData, *TopLevelWidget;

static Resource resources[]=
{
	{ XtNiconName, XtCIconPixmap, XrmRString, sizeof(caddr_t),
	    Offset(TopLevelWidget, top.icon_name), XrmRString, (caddr_t) NULL},
	{ XtNiconPixmap, XtCIconPixmap, XrmRPixmap, sizeof(caddr_t),
	    Offset(TopLevelWidget, top.icon_pixmap), XrmRPixmap, 
	    (caddr_t) NULL},
	{ XtNallowtopresize, XtCAllowtopresize, XrmRBoolean, sizeof(Boolean),
	    Offset(TopLevelWidget, top.resizeable), XrmRString, "FALSE"},
	{ XtNgeometry, XtCGeometry, XrmRString, sizeof(caddr_t), 
	    Offset(TopLevelWidget, top.geostr), XrmRString, (caddr_t) NULL},
	{ XtNinput, XtCInput, XrmRBoolean, sizeof(Boolean),
	    Offset(TopLevelWidget, top.input), XrmRString, "FALSE"},
	{ XtNiconic, XtCIconic, XrmRBoolean, sizeof(Boolean),
	    Offset(TopLevelWidget, top.iconic), XrmRBoolean, "FALSE"},
	{ XtNtitle, XtCTitle, XrmRString, sizeof(char *),
	    Offset(TopLevelWidget, top.title), XrmRString, NULL},
/*	{ XtNinitial, XtCInitial, XrmRInitialstate, sizeof(int),
	    Offset(TopLevelWidget, top.initial), XrmRString, "Normal"} */
};
static void Initialize();
static void Realize();
static void SetValues();
static void Destroy();
static void ChangeManaged(); /* XXX */
static XtGeometryReturnCode GeometryManager();
static void EventHandler();

static struct {
  	CoreClass coreclass;
	CompositeClass compositeclass;
} TopLevelWidgetClassData = {
    /* superclass         */    (WidgetClass) &compositeWidgetClassData,
    /* class_name         */    "TopLevel",
    /* size               */    sizeof(TopLevelWidgetData),
    /* initialize         */    Initialize,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_extra          */    NULL,
    /* xrm_class          */    NULLQUARK,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    NULL,
    /* expose             */    NULL,
    /* set_values         */    SetValues,
    /* accept_focus       */    NULL,
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* move_focus_to_next */    NULL,
    /* move_focus_to_prev */    NULL
};

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

/* this is the old initialize routine */
/* This needs to be updated to reflect the new world. */
static void
DO_Initialize() {
    /* 
     * For Error handling.
     */
/*
    XtErrorFunction = _XtError;
    XtreferenceCount = 1;
XXX */

    /* Resource management initialization */
    QuarkInitialize();
    XrmInitialize();
    ResourceListInitialize();

    /* Other intrinsic intialization */
    EventInitialize();
    ActionsInitialize();
    CursorsInitialize();
    GCManagerInitialize();
    GeometryInitialize();
}

/*
    What this routine does load the resource data base.

    The order that things are loaded into the database follows,
    1) an optional application specific file.
    2) server defaults, generic.
    3) server defaults for machine.
    4) .Xdefaults

 */
static XrmResourceDataBase *
XGetUsersDataBase(dpy, name, userResources )
Display dpy;
char *name;
XrmResourceDataBase *userResources;
{
	XrmResourceDataBase resources;
	int uid;
	extern struct passwd *getpwuid();
	struct passwd *pw;
	char filename[1024];
	FILE *f;
	static int first = 0;
	
	if(name != NULL) { /* application provided a file */
		f = fopen(name, "r");
		if (f) {
			XrmGetCurrentDataBase(&resources);
			XrmGetDataBase(f, userResources);
			XrmMergeDataBases(*userResources, &resources);
			XrmSetCurrentDataBase(*userResources);
			(void) fclose(f);
		}
	} 
	first = 1;
	
	if(! first) {
		/*
		  MERGE server defaults
		 */
	/* Open .Xdefaults file and merge into existing data base */
	
		uid = getuid();
		pw = getpwuid(uid);
		if (pw) {
			(void) strcpy(filename, pw->pw_dir);
			(void) strcat(filename, "/.Xdefaults");
			f = fopen(filename, "r");
			if (f) {
				XrmGetCurrentDataBase(&resources);
				XrmGetDataBase(f, userResources);
				XrmMergeDataBases(*userResources, &resources);
				XrmSetCurrentDataBase(*userResources);
				(void) fclose(f);
			}
		}
	}
}

static void
Initialize(wid) 
Widget wid;
{
	TopLevelWidget w = (TopLevelWidget) wid;
	int flag;

	if(w->top.icon_name == NULL) {
		w->top.icon_name = w->core.name;
	}
	w->core.background_pixel = None;
	w->core.background_pixmap = NULL;
	w->core.border_width = 0;
	w->core.border_pixmap = NULL;

	if(w->top.geostr != NULL) {
		flag = XParseGeometry(w->top.geostr, &w->core.x, &w->core.y,
			       &w->core.width, &w->core.height);

		
		w->top.hints.flags |= PPosition | PSize;
		if(flag & XNegative) 
			w->core.x =
			  DisplayWidth(w->core.display, w->core.screen)
			    - w->core.width - w->core.x;
		if(flag & YNegative) 
			w->core.y = 
			  DisplayHeight(w->core.display,w->core.screen)
			    - w->core.height - w->core.y;
		w->top.hints.flags |= (flag & (XValue|YValue))? USPosition : 0
		  |(flag & (WidthValue & HeightValue))? USSize : 0;
	}

	XtRegisterEventHandler(wid, EventHandler, StructureNotifyMask,
			       FALSE, NULL);
}

static void
Realize(wid, mask, attr)
Widget wid;
Mask mask;
XSetWindowAttributes *attr;
{
	TopLevelWidget w = (TopLevelWidget) wid;
	Window win;
	Display *dpy = w->core.display;
	char hostname[1024];
	XWMHints wmhints;

	win = w->core.window = XCreateWindow( dpy,
	    RootWindow( dpy, w->core.screen), w->core.x, w->core.y,
	    w->core.width, w->core.height,
	    w->core.border_width, w->core.depth, CopyFromParent,
	    CopyFromParent, mask, attr);

	XStoreName(dpy, win, w->top.title);
	XSetIconName(dpy, win, w->top.icon_name);
	XSetCommand(dpy, win, w->top.argv, w->top.argc);
#ifdef UNIX
	gethostname(hostname, sizeof(hostname));
#endif

/* now hide everything needed to set the properties until realize is called */
	wmhints.flags = 0;
	/* tell the window manager that I am interested in input */
	/* output only application should set this to false */
	wmhints.flags |= InputHint ;
	if(w->top.input)
	  wmhints.input = TRUE;
	else
	  wmhints.input = FALSE;

	/* Should I tell the window manager to bring me up iconfied */
	wmhints.initial_state = w->top.initial;
	if(w->top.iconic)
	  wmhints.initial_state = IconicState;
	XSetWMHints( dpy, win, &wmhints);
	XSetHostName(dpy, win, hostname);
	XSetClass(dpy, win, w->top.classname);/* And w->core.name XXX*/
	w->top.hints.x - w->core.x;
	w->top.hints.y - w->core.y;
	w->top.hints.width = w->core.width;
	w->top.hints.height = w->core.height;
	XSetNormalHints(dpy, win, w->top.hints);
	XSetWMHints(dpy, win, wmhints);
}

static void
EventHandler(wid, event, closure)
Widget *wid;
XEvent *event;
caddr_t closure;
{
	Widget childwid;
	int i;
	TopLevelWidget w = (TopLevelWidget) wid;

	if(w->core.window != event->xany.window) {
		XtError("Event with wrong window");
		  return;
	}
	/* I am only interested in resize */
	switch(event->type) {
	      case ConfigureNotify:
		w->core.width = event->xconfigure.width;
		w->core.height = event->xconfigure.height;
		w->core.border_width = event->xconfigure.border_width;
		w->core.x = event->xconfigure.x;
		w->core.y = event->xconfigure.y;

		for(i = 0; i < w->composite.num_children; i++) {
			if(w->composite.children[i]->core.managed) {
			      childwid = w->composite.children[i];
			      childwid->core.width = w->core.width;
			      childwid->core.height = w->core.height;
			      childwid->core.border_width = 
				w->core.border_width;
			      XtWidgetResize(childwid);
			      break;
		        }
		}
		break;
	      default:
		return;
	}  
}

static void
Destroy(wid)
Widget wid;
{
	TopLevelWidget w = (TopLevelWidget) wid;

	if(w->top.argv != NULL)
		XtFree(w->top.argv);
	w->top.argv = NULL;
	if(w->top.classname != NULL)
		XtFree(w->top.classname);
}

static void 
ChangeManaged(wid)
CompositeWidget wid;
{
	TopLevelWidget w = (TopLevelWidget) wid;
	int i;
	Widget childwid;

	for(i = 0; i < w->composite.num_children; i++) {
		if(w->composite.children[i]->core.managed) {
			childwid = w->composite.children[i];
			childwid->core.width = w->core.width;
			childwid->core.height = w->core.height;
			childwid->core.border_width = w->core.border_width;
			XtWidgetResize(childwid);
		}
	}

}
/*
 * This is gross, I can't wait to see if the change happened so I will ask
 * the window manager to change my size and do the appropriate X work.
 * I will then tell the requester that he can't.  Care must be taken because
 * it is possible that some time in the future the request will be
 * asynchronusly granted.
 */
 
static XtGeometryReturnCode
GeometryManager( wid, request, reply )
Widget wid;
WidgetGeometry *request;
WidgetGeometry *reply;
{
  	XWindowChanges values;
	XSizeHints	oldhints;
	TopLevelWidget w = (TopLevelWidget)(wid->core.parent);

	if(w->top.resizeable == FALSE)
		return(XtgeometryNo);
	if(!XtIsRealized(w)){
		if (request->request_mode & (CWX|CWY)) {
			if(request->request_mode & (CWX|CWY) == 
			   request->request_mode) {
				return(XtgeometryNo);
			} else {
				*reply = *request;
				reply->request_mode = request->request_mode &
				  ~(CWX|CWY);
				return(XtgeometryAlmost);
			}
		}
		*reply = *request;
		if(request->request_mode & CWWidth)
		   w->core.width = request->width;
		if(request->request_mode & CWHeight) 
		   w->core.height = request->height;
		if(request->request_mode & CWBorderWidth)
		   w->core.border_width = request->border_width;
		return(XtgeometryYes);
	}
	values = *(XWindowChanges *) (&(request->x));
	XGrabServer(w->core.display);
	XGetNormalHints(w->core.display, w->core.window, &oldhints);
        if(request->request_mode & CWWidth) {
                oldhints.flags &= ~USSize;
                oldhints.flags |= PSize;
                oldhints.width = request->width;
        }
        if(request->request_mode & CWHeight) {
                oldhints.flags &= ~USSize;
                oldhints.flags |= PSize;
                oldhints.height = request->height;
        }
        XSetNormalHints(w->core.display, w->core.window, &oldhints);
	XConfigureWindow(w->core.display, w->core.window,
		 (XWindowChanges *)&(request->x), request->request_mode);
	XUngrabServer(w->core.display);
	return(XtgeometryNo);
}

Setvalues(old,new)
Widget *old, *new;
{
/* I don't let people play with my values */
}

/*
 * This routine creates the desired widget and does the "Right Thing" for
 * the toolkit and for window managers.
 */

Display *
XtInitialize(urlist, urlistCount, argc, argv, name, classname, prdb, root)
Resource *urlist;
int	urlistCount;
Cardinal  *argc;
char *argv[];
char *name;
char *classname;
XrmResourceDataBase *prdb;
Widget root;
{
	char  displayName[256];
	char *displayName_ptr = displayName;
	Arg   args[8];
	int   argCount = 0;
	int i, val;
	int flags = 0;
	char filename[MAXPATHLEN];
	char **saved_argv;
	int    saved_argc = *argc;
	Display *dpy;
	TopLevelWidget w;


	if( name == NULL) {
		name = argv[0];
	}

	/* save away argv and argc so I can set the properties latter */

	saved_argv = (char **) XtCalloc(((*argc) + 1) , sizeof(*saved_argv));
	for (i = 0 ; i < *argc ; i++)
	  saved_argv[i] = argv[i];
	saved_argv[i] = NULL;
	/*
	   Find the display name and open it
	   While we are at it we look for name because that is needed 
	   soon after to do the arguement parsing.
	 */
	displayName[0] = 0;

	for(i = 1; i < *argc; i++) {
	  if(index(argv[i], ':') != NULL) {
		  (void) strncpy(displayName, argv[i], sizeof(displayName));
		  continue;
	  }
	  if(!strcmp("-name", argv[i]) || ! strcmp("-n", argv[i])) {
		  i++;
		  if(i == *argc) break;
		  name = argv[i];
		  continue;
	  }
	}
	/* Open display  */
	if (!(dpy = XOpenDisplay(displayName))) {
		char buf[1024];
		strcpy(buf, "Can't Open display: ");
		strcat(buf, displayName);
		XtError(buf);
	}
        XtSetArg(args[argCount], "display", dpy);
        argCount++;
        XtSetArg(args[argCount], "screen", dpy->default_screen);
        argCount++;
	    
	/* initialize the toolkit */
	DO_Initialize();
#define UNIX
#ifdef UNIX
#define XAPPLOADDIR  "/usr/lib/Xapps/"
#endif	
	strcpy(filename, XAPPLOADDIR);
	strcat(filename, classname);

	/*set up resource database */
	XGetUsersDataBase(dpy, filename, prdb);

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand( opTable, XtNumber(opTable), name, argc, argv);
	
	if(urlistCount >0) {
		/* the application has some more defaults */
		XrmParseCommand( urlist, urlistCount, name, argc, argv);
	}
	/* Resources are initialize and loaded */
	/* I now must handle geometry specs a compond resource */

	/*
	     Create the top level widget.
	 */
	root = XtWidgetCreate(name, &TopLevelWidgetClassData, NULL,
			       args, argCount);

	w = (TopLevelWidget) root;
	w->top.argc = saved_argc;
	w->top.argv = saved_argv;
	strcpy(w->top.classname = (char *)XtMalloc(strlen(classname))
	       ,classname);

	return(dpy);
}

