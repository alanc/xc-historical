#ifndef lint
static char rcsid[] = "$Header: Initialize.c,v 1.86 87/11/12 00:01:26 swick Locked $";
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
/* Make sure all wm properties can make it out of the resource manager */

#ifndef VMS
#include <pwd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/param.h>
#else
#include stdio
#include string
#include <descrip.h>
#include "VMSutil.h"
#endif

#ifdef VMS
#define EVENT_TIMER_FLAG 13
#define MAXPATHLEN   256
#endif

 /* Xlib definitions  */
 /* things like Window, Display, XEvent are defined herein */
#include "Intrinsic.h"
#include "Atoms.h"
#include "Shell.h"
#include "ShellP.h"


/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

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

#define Offset(x)	(XtOffset(ShellWidget, x))
static XtResource resources[]=
{
	{ XtNiconName, XtCIconName, XrmRString, sizeof(caddr_t),
	    Offset(shell.icon_name), XrmRString, (caddr_t) NULL},
	{ XtNallowshellresize, XtCAllowshellresize, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.resizeable), XrmRString, "FALSE"},
	{ XtNgeometry, XtCGeometry, XrmRString, sizeof(caddr_t), 
	    Offset(shell.geostr), XrmRString, (caddr_t) NULL},
	{ XtNiconic, XtCIconic, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.iconic), XrmRString, "FALSE"},
	{ XtNtitle, XtCTitle, XrmRString, sizeof(char *),
	    Offset(shell.title), XrmRString, NULL},
	{ XtNcreatePopupChildProc, XtCCreatePopupChildProc, XtRFunction,
	    sizeof(XtCreatePopupChildProc), Offset(shell.create_popup_child),
	    XtRFunction, NULL},
	{ XtNsaveUnder, XtCSaveUnder, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XrmRString, "FALSE"},
	{ XtNtransient, XtCTransient, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.transient), XrmRString, "FALSE"},
	{ XtNoverrideRedirect, XtCOverrideRedirect, XrmRBoolean, 
	    sizeof(Boolean),
	    Offset(shell.override_redirect), XrmRString, "FALSE"},
/* sizehints minus things stored in core */
	{ XtNminWidth, XtCMinWidth, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.min_width), XrmRString, "-1"},
	{ XtNminHeight, XtCMinHeight, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.min_height), XrmRString, "-1"},
	{ XtNmaxWidth, XtCMaxWidth, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.max_width), XrmRString, "-1"},
	{ XtNmaxHeight, XtCMaxHeight, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.max_height), XrmRString, "-1"},
	{ XtNwidthInc, XtCWidthInc, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.width_inc), XrmRString, "-1"},
	{ XtNheightInc, XtCHeightInc, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.height_inc), XrmRString, "-1"},
	{ XtNminAspectNum, XtCMinAspectNum, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.min_aspect.x), XrmRString, "-1"},
	{ XtNminAspectDenom, XtCMinAspectDenom, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.min_aspect.y), XrmRString, "-1"},
	{ XtNmaxAspectNum, XtCMaxAspectNum, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.max_aspect.x), XrmRString, "-1"},
	{ XtNmaxAspectDenom, XtCMaxAspectDenom, XrmRInt, sizeof(int),
	    Offset(shell.sizehints.max_aspect.y), XrmRString, "-1"},
/* wmhints */
	{ XtNinput, XtCInput, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.wmhints.input), XrmRString, "FALSE"},
	{ XtNinitial, XtCInitial, XrmRInt, sizeof(int),
	    Offset(shell.wmhints.initial_state), XrmRString, "1"},
	{ XtNiconPixmap, XtCIconPixmap, XrmRPixmap, sizeof(caddr_t),
	    Offset(shell.wmhints.icon_pixmap), XrmRPixmap, NULL},
	{ XtNiconWindow, XtCIconWindow, XrmRWindow, sizeof(caddr_t),
	    Offset(shell.wmhints.icon_window), XrmRWindow,   (caddr_t) NULL},
	{ XtNiconX, XtNiconX, XrmRInt, sizeof(int),
	    Offset(shell.wmhints.icon_x), XrmRString, "-1"},
	{ XtNiconY, XtNiconY, XrmRInt, sizeof(int),
	    Offset(shell.wmhints.icon_y), XrmRString, "-1"},
	{ XtNiconMask, XtCIconMask, XrmRPixmap, sizeof(caddr_t),
	    Offset(shell.wmhints.icon_mask), XrmRPixmap, NULL},
	{ XtNwindowGroup, XtCWindowGroup, XrmRWindow, sizeof(XID),
	    Offset(shell.wmhints.window_group), XrmRWindow, NULL},
/*	{ XtNinitial, XtCInitial, XrmRInitialstate, sizeof(int),
	    Offset(shell.initial), XrmRString, "Normal"} */
};
static void Initialize();
static void Realize();
static Boolean SetValues();
static void Destroy();
static void InsertChild();
static void ChangeManaged(); /* XXX */
static XtGeometryResult GeometryManager();
static void EventHandler();
static void ClassInitialize();
static void _popup_set_prop();
static void _ask_wm_for_size();
static void _do_setsave_under();

ShellClassRec shellClassRec = {
    /* superclass         */    (WidgetClass) &compositeClassRec,
    /* class_name         */    "No Name",
				/* shell doesn't have a name it is pass
       				 * to XtInitialize
				 */
    /* size               */    sizeof(ShellRec),
    /* Class Initializer  */	ClassInitialize,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    Initialize,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    NULL,
    /* expose             */    NULL,
    /* set_values         */    SetValues,
    /* accept_focus       */    NULL,
    /* callback offsets   */    NULL,
    /* reserved           */    NULL,
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	InsertChild,
    /* delete_child	  */	NULL,	      /* ||| Inherit from Composite */
    /* move_focus_to_next */    NULL,
    /* move_focus_to_prev */    NULL
};

WidgetClass shellWidgetClass = (WidgetClass) (&shellClassRec);

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

/* this is the old initialize routine */
/* This needs to be updated to reflect the new world. */
static void
DO_Initialize() {

extern void _XtResourceListInitialize();
extern void _XtTranslateInitialize();
extern void _XtCursorsInitialize();

    /* Resource management initialization */
    QuarkInitialize();
    XrmInitialize();
    _XtResourceListInitialize();

    /* Other intrinsic intialization */
    _XtEventInitialize();
    _XtTranslateInitialize();
    _XtCursorsInitialize();

#ifdef VMS
     sys$clref(EVENT_TIMER_FLAG);
     sys$clref(1);    
#endif

}
Atom XtHasInput;
Atom XtTimerExpired;

static void
init_atoms(dpy)
Display *dpy;
{
	XtHasInput = XInternAtom(dpy, "XtHasInput", False);
	XtTimerExpired = XInternAtom(dpy, "XtTimerExpired", False);
}


static void ClassInitialize()
{
    CompositeWidgetClass superclass;
    ShellWidgetClass myclass;

    myclass = (ShellWidgetClass) shellWidgetClass;
    superclass = (CompositeWidgetClass) myclass->core_class.superclass;

    /* Inherit  delete_child from Composite */
    /* I have a insert_child that calls my parents superclasses insert_child */
    /* I can't inherit this one because of the implied add that Joel wants */

    myclass->composite_class.delete_child =
        superclass->composite_class.delete_child;
}

/*
    What this routine does load the resource data base.

    The order that things are loaded into the database follows,
    1) an optional application specific file.
    2) server defaults, generic.
    3) server defaults for machine.
    4) .Xdefaults

 */

static XrmResourceDataBase
XGetUsersDataBase(dpy, name)
Display *dpy;
char *name;
{
	XrmResourceDataBase userResources = NULL;
	XrmResourceDataBase resources = NULL;
#ifndef VMS
	int uid;
	extern struct passwd *getpwuid();
	struct passwd *pw;
#endif VMS
	char filenamebuf[1024];
	char *filename = &filenamebuf[0];
	static Boolean first = TRUE;
	
	if(name != NULL) { /* application provided a file */
		userResources=	XrmGetDataBase(name);
		XrmMergeDataBases(userResources, &XtDefaultDB);
	}

	if (first) {
		first = FALSE;
		/* MERGE server defaults */
		/* Open .Xdefaults file and merge into existing data base */
#ifndef VMS
		uid = getuid();
		pw = getpwuid(uid);
		if (pw) {
			(void) strcpy(filename, pw->pw_dir);
			(void) strcat(filename, "/.Xdefaults");
		}
#else
		filename = "SYS$LOGIN:XDEFAULTS.DAT";
#endif
		userResources = XrmGetDataBase(filename);
		XrmMergeDataBases(userResources,&XtDefaultDB);
/*
		XtDefaultDB = XrmGetDataBase(filename);
*/
	}
    return userResources;
}

/* ARGSUSED */
static void
Initialize(request, new, args, num_args)
Widget request, new;
ArgList args;
Cardinal *num_args;
{
	ShellWidget w = (ShellWidget) new;
	int flag;

	if(w->shell.icon_name == NULL) {
		strcpy(w->shell.icon_name=
		       (String)XtMalloc((unsigned)strlen(w->core.name)+1)
			,w->core.name);
	}
	if(w->shell.title == NULL) {
		strcpy(w->shell.title = 
			(String)XtMalloc((unsigned)strlen(w->shell.icon_name) + 1)
		       ,w->shell.icon_name);
	}
	w->shell.sizehints.flags = 0;
	w->shell.wmhints.flags = 0;
	w->shell.popped_up = FALSE;
	w->shell.argc = -1;
	w->shell.argv = NULL;
	w->core.background_pixmap = None;

	if(w->shell.geostr != NULL) {
		flag = XParseGeometry(w->shell.geostr, &w->core.x, &w->core.y,
			       &w->core.width, &w->core.height);
		
		w->shell.sizehints.flags |= USPosition | USSize;
		if(flag & XNegative) 
			w->core.x =
			  w->core.screen->width - w->core.width - w->core.x;
		if(flag & YNegative) 
			w->core.y = 
			 w->core.screen->height - w->core.height - w->core.y;
	} else 	w->shell.clientspecified = FALSE;
	if(w->core.width != 0 && w->core.height != 0) {
	  	w->shell.clientspecified = TRUE;
        }
	XtAddEventHandler(
	    new, (EventMask) StructureNotifyMask,
	    FALSE, EventHandler, (Opaque) NULL);
}

static void
Realize(wid, vmask, attr)
Widget wid;
Mask *vmask;
XSetWindowAttributes *attr;
{
	ShellWidget w = (ShellWidget) wid;
        Mask mask = *vmask;

	mask &= ~(CWBackPixel);
	mask |= CWBackPixmap;
	attr->background_pixmap = None;	/* I must have a background pixmap of
					   none, and no background pixel. 
					   This should give me a transparent
					   background so that if there is
					   latency from when I get resized and
					   when my child is resized it won't be
					   as obvious.
					   */
	if(w->shell.save_under) {
		mask |= CWSaveUnder;
		attr->save_under = TRUE;
	}
	if(w->shell.override_redirect) {
		mask |= CWOverrideRedirect;
		attr->override_redirect = TRUE;
	}
	wid->core.window = XCreateWindow(XtDisplay(wid),
		      wid->core.screen->root, wid->core.x, wid->core.y,
		      wid->core.width, wid->core.height,
		      wid->core.border_width, (int) wid->core.depth,
		       (unsigned int) InputOutput, (Visual *) CopyFromParent,
		      mask, attr);
	_popup_set_prop(w);
}

static void _popup_set_prop(w)
ShellWidget w;
{
	Window win;
	Display *dpy = XtDisplay(w);
	Widget	ptr;
#ifdef UNIX
	char hostname[1024];
#endif
#ifndef VMS
	XClassHint classhint;
#endif
	win = XtWindow(w);
	XStoreName(dpy, win, w->shell.title);
	XSetIconName(dpy, win, w->shell.icon_name);
	if(w->shell.argc != -1)
	  XSetCommand(dpy, win, w->shell.argv, w->shell.argc);
#ifdef UNIX
	/* gethostname(hostname, sizeof(hostname)); ||| */
#endif

/* now hide everything needed to set the properties until realize is called */
	w->shell.wmhints.flags = 0;
	w->shell.wmhints.flags |= InputHint ;
	if(w->shell.wmhints.input)
	  w->shell.wmhints.input = FALSE;

	/* Should I tell the window manager to bring me up iconfied */
	w->shell.wmhints.flags |= StateHint | InputHint;
	if(w->shell.iconic) {
	  w->shell.wmhints.initial_state = IconicState;
	}
	if(w->shell.wmhints.icon_x != -1 || w->shell.wmhints.icon_y != -1)
	  w->shell.wmhints.flags |= IconPositionHint;
	if(w->shell.wmhints.icon_pixmap != NULL) {
		w->shell.wmhints.flags |=  IconPixmapHint;
	}
	if(w->shell.wmhints.icon_mask != NULL) {
		w->shell.wmhints.flags |=  IconMaskHint;
	}
	if(w->shell.wmhints.icon_window != NULL) {
		w->shell.wmhints.flags |=  IconWindowHint;
	}
#ifndef VMS     /* beta Xlib's Xutil.h doesn't have .window_group */
	if(w->shell.wmhints.window_group == -1) {
		if(w->core.parent) {
			for(ptr = w->core.parent; ptr->core.parent ;
			    ptr = ptr->core.parent );
			w->shell.wmhints.window_group = XtWindow(ptr);
			w->shell.wmhints.flags |=  WindowGroupHint;
		}
	} else {
		w->shell.wmhints.flags |=  WindowGroupHint;
	}
#endif

	XSetWMHints(dpy, win, &(w->shell.wmhints));
/* ||| this function must still be written
	XSetHostName(dpy, win, hostname); */


	w->shell.sizehints.x = w->core.x;
	w->shell.sizehints.y = w->core.y;
	w->shell.sizehints.width = w->core.width;
	w->shell.sizehints.height = w->core.height;
	w->shell.sizehints.flags |= PSize | PPosition;
	if(w->shell.sizehints.min_aspect.x != -1 || 
	   w->shell.sizehints.min_aspect.y != -1 || 
	   w->shell.sizehints.max_aspect.x != -1 || 
	   w->shell.sizehints.max_aspect.y != -1 )
	  w->shell.sizehints.flags |= PAspect;
	if(w->shell.sizehints.width_inc != -1 || w->shell.sizehints.height_inc != -1)
		 w->shell.sizehints.flags |= PResizeInc;
	if(w->shell.sizehints.max_width != -1 ||w->shell.sizehints.max_height != -1)
		w->shell.sizehints.flags |= PMaxSize;
	if(w->shell.sizehints.min_width != -1 ||w->shell.sizehints.min_height != -1)
		w->shell.sizehints.flags |= PMinSize;

	XSetNormalHints(dpy, win, &w->shell.sizehints);
#ifndef VMS
	if(w->shell.transient)
	  XSetTransientForHint(dpy, win, w->shell.wmhints.window_group);
	classhint.res_name = w->core.name;
	classhint.res_class = XtClass(w)->core_class.class_name;
	XSetClassHint(dpy, win, &classhint);
#endif
	  
}

/* ARGSUSED */
static void
EventHandler(wid, closure, event)
Widget wid;
Opaque closure;
XEvent *event;
{
	Widget childwid;
	int i;
	ShellWidget w = (ShellWidget) wid;

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
			  XtResizeWidget(
			      childwid,
			      w->core.width,
			      w->core.height,
			      (Dimension)0);
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
	ShellWidget w = (ShellWidget) wid;

	if(w->shell.argv != NULL)
		XtFree((char *)w->shell.argv);
	w->shell.argv = NULL;
}

/* ARGSUSED */
static void InsertChild(w, args, num_argsP)
    Widget w;
    ArgList args;
    Cardinal *num_argsP;
{
    Cardinal num_args = *num_argsP;

    if (((CompositeWidget)(w->core.parent))->composite.num_mapped_children > 0) {
      	XtError("The root and popup shells widget only support one child");
	return;
    }

    (*(((CompositeWidgetClass)(shellWidgetClass->core_class.superclass))->
		composite_class.insert_child)) (w, args, &num_args);
    XtManageChild(w);	/* Add to managed set now */
}

/*
 * There is some real ugliness here.  If I have a width and a height which are
 * zero, and as such suspect, and I have not yet been realized then I will 
 * grow to match my child.
 *
 */
static void 
ChangeManaged(wid)
Widget wid;
{
    ShellWidget w = (ShellWidget) wid;
    int     i;
    Widget childwid;
    Boolean needresize = FALSE;

    if(w->composite.num_mapped_children > 1) {
      	XtError("The root and popup sheels widget only support one  child");
	return;
    }
    for (i = 0; i < w->composite.num_children; i++) {
	if (w->composite.children[i]->core.managed) {
	    childwid = w->composite.children[i];
	    if (!XtIsRealized ((Widget) wid)) {
		if ((w->core.width == 0	&& w->core.height == 0) ||
		    ! w->shell.clientspecified) {
	            /* we inherit our child's attributes */
		    w->core.width = childwid->core.width;
		    w->core.height = childwid->core.height;
		    w->shell.sizehints.flags |= PSize;
		} else 
		     needresize = TRUE;
		if(childwid->core.border_width != 0)
		     needresize = TRUE;
		if(needresize) {
		    /* our child gets our attributes */
		    XtResizeWidget (
		        childwid,
			w->core.width,
			w->core.height,
			(Dimension) 0);
		    
		}
	    }
	    if(childwid->core.x != -childwid->core.border_width || 
	       childwid->core.y != -childwid->core.border_width) {
		      XtMoveWidget (childwid,
				    (int)(-childwid->core.border_width),
				    (int)(-childwid->core.border_width));
	    }
	}
    }

}

/*
 * This is gross, I can't wait to see if the change happened so I will ask
 * the window manager to change my size and do the appropriate X work.
 * I will then tell the requester that he can.  Care must be taken because
 * it is possible that some time in the future the request will be
 * asynchronusly denied and the window reverted to it's old size/shape.
 */
 
static XtGeometryResult
GeometryManager( wid, request, reply )
Widget wid;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
  	XWindowChanges values;
	ShellWidget w = (ShellWidget)(wid->core.parent);

	if(w->shell.resizeable == FALSE &&
	   ((w->shell.clientspecified == TRUE)||XtIsRealized(wid)))
		return(XtGeometryNo);
	if(!XtIsRealized((Widget)w)){
		if (request->request_mode & (CWX|CWY|CWBorderWidth)) {
			return(XtGeometryNo);
		}
		*reply = *request;
		if(request->request_mode & CWWidth)
		   wid->core.width = w->core.width = request->width;
		if(request->request_mode & CWHeight) 
		   wid->core.height = w->core.height = request->height;
		return(XtGeometryYes);
	}
/* ||| this code depends on the core x,y,width,height,borderwidth fields */
/* being the same size and same order as an XWindowChanges record. Yechh!!! */
	_ask_wm_for_size(w, (XWindowChanges *) &(request->x), request->request_mode);
	return(XtGeometryYes);
}

static void _ask_wm_for_size(w, values, mask)
ShellWidget	w;
XWindowChanges *values;
int mask;
{
	XGrabServer(XtDisplay(w));
	XGetNormalHints(XtDisplay(w), w->core.window, &w->shell.sizehints);
	if(mask & CWX) {
		w->shell.sizehints.flags &= ~USPosition;
                w->shell.sizehints.flags |= PPosition;
                w->core.x = w->shell.sizehints.x = values->x;
	}
	if(mask & CWY) {
		w->shell.sizehints.flags &= ~USPosition;
                w->shell.sizehints.flags |= PPosition;
                w->core.y = w->shell.sizehints.y = values->y;
	}
	if(mask & CWBorderWidth) {
                w->core.border_width = values->border_width;
	}
        if(mask & CWWidth) {
                w->shell.sizehints.flags &= ~USSize;
                w->shell.sizehints.flags |= PSize;
                w->core.width = w->shell.sizehints.width = values->width;
        }
        if(mask & CWHeight) {
                w->shell.sizehints.flags &= ~USSize;
                w->shell.sizehints.flags |= PSize;
                w->core.height = w->shell.sizehints.height = values->height;
        }
        XSetNormalHints(XtDisplay(w), w->core.window, &w->shell.sizehints);

	XConfigureWindow(XtDisplay(w), w->core.window, mask, values);
	XUngrabServer(XtDisplay(w));
}

static void _do_setsave_under(w, flag)
ShellWidget w;
Boolean flag;
{
	int mask;
	XSetWindowAttributes attr;

	if(XtIsRealized((Widget)w)) {
		mask = CWSaveUnder;
		attr.save_under = w->shell.save_under;
		XChangeWindowAttributes(XtDisplay(w), XtWindow(w), mask, &attr);
	}
}

static void _do_setoverride_redirect(w, flag)
ShellWidget w;
Boolean flag;
{
	int mask;
	XSetWindowAttributes attr;

	if(XtIsRealized((Widget)w)) {
		mask = CWOverrideRedirect;
		attr.override_redirect = w->shell.override_redirect;
		XChangeWindowAttributes(XtDisplay(w), XtWindow(w), mask, &attr);
	}
}

static Boolean SetValues(current, request, newvals, last)
Widget current, request, newvals;
Boolean last;
{
	XWMHints  *oldhints;
	ShellWidget cur = (ShellWidget) current;
	ShellWidget req = (ShellWidget) request;
	ShellWidget new = (ShellWidget) newvals;
	Boolean name = FALSE;
	Boolean wmhints = FALSE;
	Boolean title = FALSE;
	Boolean size = FALSE;
	XWindowChanges values;
	int mask;
	
	/* I don't let people play with most of my values */
	new = cur;

	/* except these few... */
	new->core.sensitive = req->core.sensitive;
	new->shell.resizeable = req->shell.resizeable;
	new->shell.transient = req->shell.transient;
	new->shell.create_popup_child = req->shell.create_popup_child;

#define	EQ(x) (cur->shell.x == req->shell.x)
#define EQC(x)	(cur->core.x == req->core.x)
	if(! EQ(save_under) ) {
		new->shell.save_under = req->shell.save_under;
		_do_setsave_under(cur, new->shell.save_under) ;
	}
	if(! EQ(override_redirect) ) {
		new->shell.override_redirect = req->shell.override_redirect;
		_do_setoverride_redirect(cur, new->shell.override_redirect) ;
	}
#define EQS(x) (EQ(sizehints.x))

	if(! EQS(min_width)||! EQS(min_height)) {
	  if(req->shell.sizehints.min_width != -1 ||
	     (req->shell.sizehints.min_height != -1)) {
	    new->shell.sizehints.min_width = req->shell.sizehints.min_width;
	    new->shell.sizehints.min_height = req->shell.sizehints.min_height;
	    new->shell.sizehints.flags |= PMinSize;
	  } else
	     new->shell.sizehints.flags &= ~PMinSize;
	  size = TRUE;
	}
	if( ! EQS(max_width) || ! EQS(max_height) ) {
	  if(req->shell.sizehints.max_width != -1 ||
	     (req->shell.sizehints.max_height != -1)) {
	    new->shell.sizehints.max_width = req->shell.sizehints.max_width;
	    new->shell.sizehints.max_height = req->shell.sizehints.max_height;
	    new->shell.sizehints.flags |=PMaxSize;
	  } else
	     new->shell.sizehints.flags &= ~PMaxSize;
	  size = TRUE;
	}
	if( ! EQS(width_inc) || ! EQS(height_inc) ) {
	  if(req->shell.sizehints.width_inc != -1 ||
	     (req->shell.sizehints.height_inc != -1)) {
	    new->shell.sizehints.width_inc = req->shell.sizehints.width_inc;
	    new->shell.sizehints.height_inc = req->shell.sizehints.height_inc;
	    new->shell.sizehints.flags |=PResizeInc;
	  } else
	     new->shell.sizehints.flags &= ~PResizeInc;
	  size = TRUE;
	}
	if( ! EQS(min_aspect.x) ||! EQS(min_aspect.y) ||
	   ! EQS(max_aspect.x) ||! EQS(max_aspect.y)) {
		
	  if((req->shell.sizehints.min_aspect.x != -1) ||
	     (req->shell.sizehints.min_aspect.y != -1) ||
	     (req->shell.sizehints.max_aspect.x != -1) ||
	     (req->shell.sizehints.max_aspect.y != -1)) {
	    new->shell.sizehints.min_aspect.x = req->shell.sizehints.min_aspect.x;
	    new->shell.sizehints.min_aspect.y = req->shell.sizehints.min_aspect.y;
	    new->shell.sizehints.max_aspect.x = req->shell.sizehints.max_aspect.x;
	    new->shell.sizehints.max_aspect.y = req->shell.sizehints.max_aspect.y;
	    new->shell.sizehints.flags |=PAspect;
	  } else
	     new->shell.sizehints.flags &= ~PAspect;
	  size = TRUE;
	}
	if(size && XtIsRealized(cur)) {
	    XSetNormalHints(XtDisplay(cur), XtWindow(cur), &new->shell.sizehints);
	}
	if(! EQ(title) ) {
	  	XtFree(cur->shell.title);
		strcpy(new->shell.title = 
		       (String) XtMalloc(strlen(req->shell.title) + 1),
		       req->shell.title);
		title = TRUE;
        }
	
	if(! EQ(icon_name)) {
	  	XtFree(cur->shell.icon_name);
		strcpy(new ->shell.icon_name = 
		       (String)XtMalloc(strlen(req->shell.icon_name) + 1),
		       req->shell.icon_name);
		name = TRUE;
	}
#define EQW(x)	(EQ(wmhints.x))
#define GETWMHINTS \
	  	if(XtIsRealized((Widget)cur) && !wmhints) { \
		    oldhints = XGetWMHints(XtDisplay(cur), cur->core.window); \
		    new->shell.wmhints = *oldhints; \
		    XtFree((char *)oldhints); \
	        } \
		wmhints = TRUE;

	if(! EQW(initial_state)) {
		GETWMHINTS;
		new->shell.wmhints.initial_state = req->shell.wmhints.initial_state;
		new->shell.wmhints.flags |= StateHint;
	}
	if(! EQW(icon_x)|| !EQW(icon_y)) {
		GETWMHINTS;
		new->shell.wmhints.icon_x = req->shell.wmhints.icon_x;
		new->shell.wmhints.icon_y = req->shell.wmhints.icon_y;
		new->shell.wmhints.flags |= IconPositionHint;
	}
	if(!XtIsRealized((Widget)cur)) { 
	  /*The rest doesn't work until we are realized */
	        if(!cur->shell.clientspecified) {
		  	new->core.x = req->core.x;
			new->core.y = req->core.y;
			new->core.width = req->core.width;
			new->core.height = req->core.height;
			new->core.border_width = req->core.border_width;
		}
	} else {
		if(cur->shell.resizeable) {
		  mask = 0;
		  if(!EQC(x)) {
		  	mask |= CWX;
			values.x = req->core.x;
		  }
		  if(!EQC(y)) {
		  	mask |= CWY;
			values.y = req->core.y;
		  }
		  if(!EQC(width)) {
		  	mask |= CWWidth;
			values.width = req->core.width;
		  }
		  if(!EQC(height)) {
		  	mask |= CWHeight;
			values.height = req->core.height;
		  }
		  if(!EQC(border_width)) {
		  	mask |= CWBorderWidth;
			values.border_width = req->core.border_width;
		  }
		  if(mask)
		    _ask_wm_for_size(cur, &values, mask);
	    }
	}
/*XXX Leak alleret  These should be copied and freed but I am lazy */
	if(!EQW(icon_pixmap)) {
		GETWMHINTS;
		new ->shell.wmhints.icon_pixmap = req->shell.wmhints.icon_pixmap;
		new->shell.wmhints.flags |= IconPixmapHint;
	}
	if(!EQW(icon_mask)) {
		GETWMHINTS;
		new ->shell.wmhints.icon_mask = req->shell.wmhints.icon_mask;
		new->shell.wmhints.flags |= IconMaskHint;
	}
	if(!EQW(icon_window)) {
		GETWMHINTS;
		new ->shell.wmhints.icon_window = req->shell.wmhints.icon_window;
		new->shell.wmhints.flags |= IconWindowHint;
	}
#ifndef VMS    /* Beta Xlib's Xutil.h doesn't have window_group */
	if(!EQW(window_group)) {
	  	GETWMHINTS;
		new ->shell.wmhints.window_group = req->shell.wmhints.window_group;
		new->shell.wmhints.flags |= WindowGroupHint;
	}
#endif

#undef GETWMHINTS

	if(!XtIsRealized((Widget)cur))
		return( FALSE );

	if(name) {
		XSetIconName(XtDisplay(cur), cur->core.window, new->shell.icon_name);
	}
	if( title ) {
		XStoreName(XtDisplay(cur), cur->core.window, new->shell.title);
	}
	if(wmhints) {
		XSetWMHints( XtDisplay(cur), cur->core.window, &(new->shell.wmhints));
	}
#ifdef DECHINTS
	FOO not written yet 
#endif

	return( FALSE );

}

/*
 * This routine creates the desired widget and does the "Right Thing" for
 * the toolkit and for window managers.
 */

Widget
XtInitialize(name, classname, urlist, num_urs, argc, argv)
    char *name;
    char *classname;
    XrmOptionDescRec *urlist;
    Cardinal num_urs;
    Cardinal *argc;
    char *argv[];
{
	char  displayName[256];
	Arg   args[8];
	Cardinal num_args = 0;
	int i;
	char filename[MAXPATHLEN];
	char **saved_argv;
	Cardinal saved_argc = *argc;
	Display *dpy;
	char *ptr, *rindex();
	ShellWidget w;
	Widget root;
	int squish = -1;
	Boolean dosync = FALSE;

	if( name == NULL) {
	  	ptr = rindex(argv[0], '/');
		if(ptr)
		  name = ++ ptr;
		else
		  name = argv[0];
	}

	/* save away argv and argc so I can set the properties latter */

	saved_argv = (char **) XtCalloc(
	    (unsigned) ((*argc) + 1) , (unsigned)sizeof(*saved_argv));
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
	  if (index(argv[i], ':') != NULL) {
		  (void) strncpy(displayName, argv[i], sizeof(displayName));
		  if( *argc == i + 1) {
		    (*argc)--;
		  } else {  /* need to squish this one out of the list */
		    squish = i;
		  }
		  continue;
	  }
	  if(!strcmp("-name", argv[i]) || ! strcmp("-n", argv[i])) {
		  i++;
		  if(i == *argc) break;
		  name = argv[i];
		  continue;
	  }
	  if (!strcmp("-sync", argv[i])) {
		  dosync = TRUE;
		  continue;
	  }
	}
	if(squish != -1) {
		(*argc)--;
		for(i = squish; i < *argc; i++) {
			argv[i] = argv[i+1];
		}
	}
	/* Open display  */
	if (!(dpy = XOpenDisplay(displayName))) {
		char buf[1024];
		(void) strcpy(buf, "Can't Open display: ");
		(void) strcat(buf, displayName);
		XtError(buf);
	}
        toplevelDisplay = dpy;
	if (dosync) XSynchronize(dpy, TRUE);

        XtSetArg(args[num_args], "display", dpy);
        num_args++;
        XtSetArg(args[num_args], "screen", dpy->default_screen);
        num_args++;
	    
	/* initialize the toolkit */
	DO_Initialize();
#ifndef VMS
#define XAPPLOADDIR  "/usr/lib/X11/app-defaults/"
#else
#define XAPPLOADDIR  "SYS$LIBRARY:"
#endif VMS
	(void) strcpy(filename, XAPPLOADDIR);
	(void) strcat(filename, classname);
#ifdef VMS
	(void) strcat(filename, ".DAT");
#endif VMS

	/*set up resource database */
	(void) XGetUsersDataBase(dpy, filename);

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand( &XtDefaultDB,opTable,
                           XtNumber(opTable), name, argc, argv);
	
	if(num_urs >0) {
		/* the application has some more defaults */
		XrmParseCommand( &XtDefaultDB, urlist,
                           num_urs, name, argc, argv);
	}
	/* Resources are initialize and loaded */
	/* I now must handle geometry specs a compond resource */

	/*
	     Create the shell level widget.
	     Unlike most classes the shell widget class has no classname
	     The name is supplied in the call to XtInitialize.
	 */
	(void) strcpy(
	    ((ShellClassRec *)(shellWidgetClass))->core_class.class_name
	        = (String)XtMalloc((unsigned)strlen(classname)+1),
	       classname);
        root = XtCreateApplicationShell(name,shellWidgetClass,
                  args,num_args);


	w = (ShellWidget) root;
	w->shell.argc = saved_argc;
	w->shell.argv = saved_argv;

	init_atoms(dpy);

	return(root);
}
