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
 * $XConsortium: events.c,v 1.74 89/07/13 09:46:24 jim Exp $
 *
 * twm event handling
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: events.c,v 1.74 89/07/13 09:46:24 jim Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include <X11/Xatom.h>
#include "add_window.h"
#include "menus.h"
#include "events.h"
#include "resize.h"
#include "gram.h"
#include "util.h"
#include "twm.bm"
#include "screen.h"
#include "iconmgr.h"
#include "siconify.bm"

#define MAX_X_EVENT 256
event_proc EventHandler[MAX_X_EVENT]; /* event handler jump table */
char *Action;
int Context = C_NO_CONTEXT;	/* current button press context */
TwmWindow *ButtonWindow;	/* button press window structure */
XEvent ButtonEvent;		/* button press event */
XEvent Event;			/* the current event */
TwmWindow *Tmp_win;		/* the current twm window */

Window DragWindow;		/* variables used in moving windows */
int DragX;
int DragY;
int DragWidth;
int DragHeight;
static int enter_flag;
static Atom wmChangeStateAtom;
static Atom twmRaisingWindowAtom;

ScreenInfo *FindScreenInfo();
int ButtonPressed = -1;
int ButtonNeeded = -1;
int Cancel = FALSE;

void HandleCreateNotify();

#ifdef SHAPE
void HandleShapeNotify ();
#endif

/***********************************************************************
 *
 *  Procedure:
 *	InitEvents - initialize the event jump table
 *
 ***********************************************************************
 */

void
InitEvents()
{
    int i;

    wmChangeStateAtom = XInternAtom(dpy, "WM_CHANGE_STATE", False);
    twmRaisingWindowAtom = XInternAtom(dpy, "TWM_RAISING_WINDOW", False);

    ResizeWindow = NULL;
    DragWindow = NULL;
    enter_flag = FALSE;

    for (i = 0; i < MAX_X_EVENT; i++)
	EventHandler[i] = HandleUnknown;

    EventHandler[Expose] = HandleExpose;
    EventHandler[CreateNotify] = HandleCreateNotify;
    EventHandler[DestroyNotify] = HandleDestroyNotify;
    EventHandler[MapRequest] = HandleMapRequest;
    EventHandler[MapNotify] = HandleMapNotify;
    EventHandler[UnmapNotify] = HandleUnmapNotify;
    EventHandler[MotionNotify] = HandleMotionNotify;
    EventHandler[ButtonRelease] = HandleButtonRelease;
    EventHandler[ButtonPress] = HandleButtonPress;
    EventHandler[EnterNotify] = HandleEnterNotify;
    EventHandler[LeaveNotify] = HandleLeaveNotify;
    EventHandler[ConfigureRequest] = HandleConfigureRequest;
    EventHandler[ClientMessage] = HandleClientMessage;
    EventHandler[PropertyNotify] = HandlePropertyNotify;
    EventHandler[KeyPress] = HandleKeyPress;
    EventHandler[ColormapNotify] = HandleColormapNotify;
    EventHandler[VisibilityNotify] = HandleVisibilityNotify;
#ifdef SHAPE
    if (HasShape)
	EventHandler[XShapeGetEventBase(dpy)+ShapeNotify] = HandleShapeNotify;
#endif
}



/***********************************************************************
 *
 *  Procedure:
 *	DispatchEvent - handle a single X event stored in global var Event
 *
 ***********************************************************************
 */
Bool DispatchEvent ()
{
    if (XFindContext (dpy, Event.xany.window,
		      TwmContext, &Tmp_win) == XCNOENT)
      Tmp_win = NULL;

    if (XFindContext (dpy, Event.xany.window,
		      ScreenContext, &Scr) == XCNOENT)
      Scr = FindScreenInfo(Event.xany.window);

    if (Scr == NULL) return False;

#ifdef DEBUG_EVENTS
    if (Tmp_win != NULL) {
	fprintf (stderr,"Event w=%x, t->w=%x, t->frame=%x, t->title=%x, ",
		 Event.xany.window, Tmp_win->w,
		 Tmp_win->frame, Tmp_win->title_w);
    } else {
	fprintf (stderr, "Event w=%x, ", Event.xany.window);
    }
#endif

    if (Event.type >= 0 && Event.type < MAX_X_EVENT)
      (*EventHandler[Event.type])();

#ifdef DEBUG_EVENTS
    fflush(stderr);
#endif

    return True;
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleEvents - handle X events
 *
 ***********************************************************************
 */

void
HandleEvents()
{
    while (TRUE)
    {
	if (ResizeWindow && !XPending(dpy) )
	{
	    Event.xany.window = ResizeWindow;
	    XQueryPointer( dpy, Event.xany.window,
		&(Event.xmotion.root), &JunkChild,
		&(Event.xmotion.x_root), &(Event.xmotion.y_root),
		&(Event.xmotion.x), &(Event.xmotion.y),
		&JunkMask);
	    XFindContext(dpy, Event.xany.window, ScreenContext, &Scr);

	    (*EventHandler[MotionNotify])();
	}
	else
	{
	    WindowMoved = FALSE;
	    XNextEvent(dpy, &Event);
	    (void) DispatchEvent ();
	}
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleVisiblityNotify - visibility notify event handler
 *
 ***********************************************************************
 */

void
HandleVisibilityNotify()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "VisibilityNotify\n");
    fprintf(stderr, "  visibility = %d\n", Event.xvisibility.state);
#endif
    if (Tmp_win == NULL)
	return;

    if (Event.xany.window == Tmp_win->frame)
	Tmp_win->frame_vis = Event.xvisibility.state;
    else if (Event.xany.window == Tmp_win->icon_w)
	Tmp_win->icon_vis = Event.xvisibility.state;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleColormapNotify - colormap notify event handler
 *
 * This procedure handles both a client changing its own colormap, and
 * a client explicitly installing its colormap itself (only the window
 * manager should do that, so we must set it correctly).
 *
 ***********************************************************************
 */

void
HandleColormapNotify()
{
    XColormapEvent *cevent = (XColormapEvent *) &Event;
    Bool sameScreen;
    Window child;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "ColormapNotify\n");
#endif

    if (Tmp_win == NULL)
	return;

#ifdef DEBUG_EVENTS
	fprintf(stderr, "    new=%d, state=%d, cmap=0x%x\n",
	    cevent->new, cevent->state, cevent->colormap);
#endif

    if (cevent->window == Tmp_win->w)
    {
	XWindowAttributes attr;

	/* Did the client change its colormap? */
	if (cevent->new == True)
	    Tmp_win->attr.colormap = cevent->colormap;


	/* this corrects a dix server (at least on HP) bug which sends
	 * the previous colormap.  I don't know if this has been fixed in R3
	 */
	XGetWindowAttributes(dpy, Tmp_win->w, &attr);
	Tmp_win->attr.colormap = attr.colormap;

	/*
	 * Either somebody changed it, or somebody did an explicit install.
	 * It isn't enough to know that the window has focus;  we also need
	 * to know that the pointer isn't in the title bar (in which case
	 * the twm colormap should be used).  If the pointer is in the client
	 * window, install the correct colormap.
	 */
	if (cevent->state == ColormapUninstalled && Tmp_win == Scr->Focus)
	{
	    sameScreen = XQueryPointer (dpy, Tmp_win->frame, &JunkRoot, &child,
					&JunkX, &JunkY, &JunkX, &JunkY,
					&JunkMask);
	    if (sameScreen && child == Tmp_win->w)
	    {
#ifdef DEBUG_EVENTS
		fprintf(stderr, "    installing 0x%x\n",
		    Tmp_win->attr.colormap);
#endif
		XInstallColormap(dpy, Tmp_win->attr.colormap);
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleKeyPress - key press event handler
 *
 ***********************************************************************
 */

void
HandleKeyPress()
{
    FuncKey *key;
    int len;

    XUnmapWindow(dpy, Scr->InfoWindow);
    Context = C_NO_CONTEXT;

    if (Event.xany.window == Scr->Root)
	Context = C_ROOT;
    if (Tmp_win)
    {
	if (Event.xany.window == Tmp_win->title_w)
	    Context = C_TITLE;
	if (Event.xany.window == Tmp_win->w)
	    Context = C_WINDOW;
	if (Event.xany.window == Tmp_win->icon_w)
	    Context = C_ICON;
	if (Event.xany.window == Tmp_win->frame)
	    Context = C_FRAME;
	if (Tmp_win->list && Event.xany.window == Tmp_win->list->w)
	    Context = C_ICONMGR;
	if (Tmp_win->list && Event.xany.window == Tmp_win->list->icon)
	    Context = C_ICONMGR;
    }

    for (key = Scr->FuncKeyRoot.next; key != NULL; key = key->next)
    {
	if (key->keycode == Event.xkey.keycode &&
	    key->mods == Event.xkey.state &&
	    (key->cont == Context || key->cont == C_NAME))
	{
	    /* weed out the functions that don't make sense to execute
	     * from a key press 
	     */
	    if (key->func == F_MOVE || key->func == F_RESIZE)
		return;

	    if (key->cont != C_NAME)
	    {
		ExecuteFunction(key->func, key->action, Event.xany.window,
		    Tmp_win, &Event, Context, FALSE);
		XUngrabPointer(dpy, CurrentTime);
		return;
	    }
	    else
	    {
		int matched = FALSE;
		len = strlen(key->win_name);

		/* try and match the name first */
		for (Tmp_win = Scr->TwmRoot.next; Tmp_win != NULL;
		    Tmp_win = Tmp_win->next)
		{
		    if (!strncmp(key->win_name, Tmp_win->name, len))
		    {
			matched = TRUE;
			ExecuteFunction(key->func, key->action, Tmp_win->frame,
			    Tmp_win, &Event, C_FRAME, FALSE);
			XUngrabPointer(dpy, CurrentTime);
		    }
		}

		/* now try the res_name */
		if (!matched)
		for (Tmp_win = Scr->TwmRoot.next; Tmp_win != NULL;
		    Tmp_win = Tmp_win->next)
		{
		    if (!strncmp(key->win_name, Tmp_win->class.res_name, len))
		    {
			matched = TRUE;
			ExecuteFunction(key->func, key->action, Tmp_win->frame,
			    Tmp_win, &Event, C_FRAME, FALSE);
			XUngrabPointer(dpy, CurrentTime);
		    }
		}

		/* now try the res_class */
		if (!matched)
		for (Tmp_win = Scr->TwmRoot.next; Tmp_win != NULL;
		    Tmp_win = Tmp_win->next)
		{
		    if (!strncmp(key->win_name, Tmp_win->class.res_class, len))
		    {
			matched = TRUE;
			ExecuteFunction(key->func, key->action, Tmp_win->frame,
			    Tmp_win, &Event, C_FRAME, FALSE);
			XUngrabPointer(dpy, CurrentTime);
		    }
		}
		if (matched)
		    return;
	    }
	}
    }

    /* if we get here, no function key was bound to the key.  Send it
     * to the client if it was in a window we know about.
     */
    if (Tmp_win)
    {
        if (Event.xany.window == Tmp_win->icon_w ||
	    Event.xany.window == Tmp_win->frame ||
	    Event.xany.window == Tmp_win->title_w ||
	    (Tmp_win->list && (Event.xany.window == Tmp_win->list->w)))
        {
            Event.xkey.window = Tmp_win->w;
            XSendEvent(dpy, Tmp_win->w, False, KeyPressMask, &Event);
        }
    }

}

/***********************************************************************
 *
 *  Procedure:
 *	HandlePropertyNotify - property notify event handler
 *
 ***********************************************************************
 */

void
HandlePropertyNotify()
{
    char *prop;
    XWMHints *wmhints;
    XSizeHints hints;
    Atom actual;
    int junk1, junk2, len;
    int width, height, x, y;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    Pixmap pm;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "PropertyNotify = %d\n", Event.xproperty.atom);
#endif

    if (Tmp_win == NULL)
	return;

    XGetWindowProperty(dpy, Tmp_win->w, Event.xproperty.atom, 0, 200, False,
	XA_STRING, &actual, &junk1, &junk2, &len, &prop);

    if (actual == None)
	return;

    if (prop == NULL)
	prop = NoName;

    switch (Event.xproperty.atom)
    {
    case XA_WM_NAME:
	Tmp_win->full_name = prop;
	Tmp_win->name = prop;

	Tmp_win->name_width = XTextWidth(Scr->TitleBarFont.font, Tmp_win->name,
	    strlen(Tmp_win->name));

	SetupWindow(Tmp_win,
	    Tmp_win->frame_x, Tmp_win->frame_y,
	    Tmp_win->frame_width, Tmp_win->frame_height);

	if (Tmp_win->title_w)
	{
	    XClearArea(dpy, Tmp_win->title_w, 0,0,0,0, True);
	}


	/* if the icon name is NoName, set the name of the icon to be
	 * the same as the window 
	 */
	if (Tmp_win->icon_name == NoName)
	{
	    Tmp_win->icon_name = Tmp_win->name;
	    RedoIconName();
	}
	break;

    case XA_WM_ICON_NAME:
	Tmp_win->icon_name = prop;

	RedoIconName();
	break;

    case XA_WM_HINTS:
	Tmp_win->wmhints = XGetWMHints(dpy, Event.xany.window);

	if (Tmp_win->wmhints && (Tmp_win->wmhints->flags & WindowGroupHint))
	    Tmp_win->group = Tmp_win->wmhints->window_group;

	if (!Tmp_win->forced && Tmp_win->wmhints &&
	    Tmp_win->wmhints->flags & IconWindowHint)
	{
	    Tmp_win->icon_w = Tmp_win->wmhints->icon_window;
	}

	if (Tmp_win->icon_w && !Tmp_win->forced && Tmp_win->wmhints &&
	    (Tmp_win->wmhints->flags & IconPixmapHint))
	{
	    XGetGeometry(dpy, Tmp_win->wmhints->icon_pixmap, &JunkRoot,
		&JunkX, &JunkY, &Tmp_win->icon_width, &Tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Scr->Root, Tmp_win->icon_width,
		Tmp_win->icon_height, Scr->d_depth);

	    FB(Tmp_win->iconc.fore, Tmp_win->iconc.back);
	    XCopyPlane(dpy, Tmp_win->wmhints->icon_pixmap, pm,
		Scr->NormalGC,
		0,0, Tmp_win->icon_width, Tmp_win->icon_height, 0, 0, 1 );

	    valuemask = CWBackPixmap;
	    attributes.background_pixmap = pm;

	    if (Tmp_win->icon_bm_w)
		XDestroyWindow(dpy, Tmp_win->icon_bm_w);

	    Tmp_win->icon_bm_w = XCreateWindow(dpy, Tmp_win->icon_w,
		0, 0, Tmp_win->icon_width, Tmp_win->icon_height,
		0, Scr->d_depth, CopyFromParent, Scr->d_visual,
		valuemask, &attributes);

	    RedoIconName();
	}
	break;

    case XA_WM_NORMAL_HINTS:
	XGetNormalHints(dpy, Tmp_win->w, &Tmp_win->hints);
	break;

    default:
#ifdef DEBUG_EVENTS
	fprintf(stderr, "TWM Not handling property %d\n",Event.xproperty.atom);
#endif
	break;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	RedoIconName - procedure to re-position the icon window and name
 *
 ***********************************************************************
 */

RedoIconName()
{
    int x, y;

    if (Tmp_win->list)
    {
	/* let the expose event cause the repaint */
	XClearArea(dpy, Tmp_win->list->w, 0,0,0,0, True);

	if (Scr->SortIconMgr)
	    SortIconManager(Tmp_win->list->iconmgr);
    }

    if (Tmp_win->icon_w == NULL)
	return;

    Tmp_win->icon_w_width = XTextWidth(Scr->IconFont.font,
	Tmp_win->icon_name, strlen(Tmp_win->icon_name));

    Tmp_win->icon_w_width += 6;
    if (Tmp_win->icon_w_width < Tmp_win->icon_width)
    {
	Tmp_win->icon_x = (Tmp_win->icon_width - Tmp_win->icon_w_width)/2;
	Tmp_win->icon_x += 3;
	Tmp_win->icon_w_width = Tmp_win->icon_width;
    }
    else
    {
	Tmp_win->icon_x = 3;
    }

    if (Tmp_win->icon_w_width == Tmp_win->icon_width)
	x = 0;
    else
	x = (Tmp_win->icon_w_width - Tmp_win->icon_width)/2;

    y = 0;

    Tmp_win->icon_w_height = Tmp_win->icon_height + Scr->IconFont.height + 4;
    Tmp_win->icon_y = Tmp_win->icon_height + Scr->IconFont.height;

    XResizeWindow(dpy, Tmp_win->icon_w, Tmp_win->icon_w_width,
	Tmp_win->icon_w_height);
    if (Tmp_win->icon_bm_w)
    {
	XMoveWindow(dpy, Tmp_win->icon_bm_w, x, y);
	XMapWindow(dpy, Tmp_win->icon_bm_w);
    }
    if (Tmp_win->icon)
    {
	XClearArea(dpy, Tmp_win->icon_w, 0, 0, 0, 0, True);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleClientMessage - client message event handler
 *
 ***********************************************************************
 */

void
HandleClientMessage()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "ClientMessage = 0x%x\n", Event.xclient.message_type);
#endif

    if (Event.xclient.message_type == twmRaisingWindowAtom)
    {
#ifdef DEBUG_EVENTS
	fprintf(stderr, "TWM_RAISING_WINDOW message client received.\n");
#endif
	enter_flag = FALSE;
    }
    else if (Event.xclient.message_type == wmChangeStateAtom)
    {
#ifdef DEBUG_EVENTS
	fprintf(stderr, "WM_CHANGE_STATE client message received.\n");
#endif
	if (Tmp_win != NULL)
	{
	    if (Event.xclient.data.l[0] == IconicState && !Tmp_win->icon)
	    {
		XEvent button;

		XQueryPointer( dpy, Scr->Root, &JunkRoot, &JunkChild,
			      &(button.xmotion.x_root),
			      &(button.xmotion.y_root),
			      &JunkX, &JunkY, &JunkMask);

		ExecuteFunction(F_ICONIFY, NULL, Event.xany.window,
		    Tmp_win, &button, FRAME, FALSE);
		XUngrabPointer(dpy, CurrentTime);
	    }
	}
    }
    else
    {
#ifdef DEBUG_EVENTS
	fprintf(stderr, "Unknown client message received.\n");
#endif
    }
}


/***********************************************************************
 *
 *  Procedure:
 *	HandleExpose - expose event handler
 *
 ***********************************************************************
 */

void
HandleExpose()
{
    MenuRoot *tmp;
    static void flush_expose();

#ifdef DEBUG_EVENTS
    fprintf(stderr, "Expose %d\n", Event.xexpose.count);
#endif

    if (XFindContext(dpy, Event.xany.window, MenuContext, &tmp) == 0)
    {
	PaintMenu(tmp, &Event);
	return;
    }

    if (Event.xexpose.count != 0)
	return;

    if (Event.xany.window == Scr->InfoWindow)
    {
	int i;
	int height;

	FBF(Scr->DefaultC.fore, Scr->DefaultC.back,
	    Scr->DefaultFont.font->fid);

	height = Scr->DefaultFont.height+2;
	for (i = 0; i < InfoLines; i++)
	{
	    XDrawString(dpy, Scr->InfoWindow, Scr->NormalGC,
		5, (i*height) + Scr->DefaultFont.y, Info[i], strlen(Info[i]));
	}
	flush_expose (Event.xany.window);
    }

    if (Tmp_win != NULL)
    {
	int h = Scr->TitleHeight - Scr->FramePadding * 2;

	if (Event.xany.window == Tmp_win->title_w)
	{
	    FBF(Tmp_win->title.fore, Tmp_win->title.back,
		Scr->TitleBarFont.font->fid);

	    XDrawString (dpy, Tmp_win->title_w, Scr->NormalGC,
		TitleBarX, Scr->TitleBarFont.y,
		Tmp_win->name, strlen(Tmp_win->name));
	    flush_expose (Event.xany.window);
	}

	if (Event.xany.window == Tmp_win->icon_w)
	{
	    FBF(Tmp_win->iconc.fore, Tmp_win->iconc.back,
		Scr->IconFont.font->fid);

	    XDrawString (dpy, Tmp_win->icon_w,
		Scr->NormalGC,
		Tmp_win->icon_x, Tmp_win->icon_y,
		Tmp_win->icon_name, strlen(Tmp_win->icon_name));
	    flush_expose (Event.xany.window);
	    return;
	}

	if (Tmp_win->list)
	{
	    if (Event.xany.window == Tmp_win->list->w)
	    {
		FBF(Tmp_win->list->fore, Tmp_win->list->back,
		    Scr->IconManagerFont.font->fid);
		XDrawString (dpy, Event.xany.window, Scr->NormalGC, 
		    iconmgr_textx, Scr->IconManagerFont.y+4,
		    Tmp_win->icon_name, strlen(Tmp_win->icon_name));
		DrawIconManagerBorder(Tmp_win->list);
		flush_expose (Event.xany.window);
		return;
	    }
	    if (Event.xany.window == Tmp_win->list->icon)
	    {
		FB(Tmp_win->list->fore, Tmp_win->list->back);
		XCopyPlane(dpy, Scr->siconifyPm, Tmp_win->list->icon,
		    Scr->NormalGC,
		    0,0, siconify_width, siconify_height, 0, 0, 1);
		flush_expose (Event.xany.window);
		return;
	    }
	}
    }

    if (Event.xany.window == Scr->VersionWindow)
    {
	FBF(Scr->DefaultC.fore, Scr->DefaultC.back, Scr->VersionFont.font->fid);
	XDrawString (dpy, Scr->VersionWindow, Scr->NormalGC,
	    twm_width + 10,
	    2 + Scr->VersionFont.font->ascent, Version, strlen(Version));
	flush_expose (Event.xany.window);
	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleDestroyNotify - DestroyNotify event handler
 *
 ***********************************************************************
 */

void
HandleDestroyNotify()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "DestroyNotify\n");
#endif
    
    /*
     * Warning, this is also called by HandleUnmapNotify; if it ever needs to
     * look at the event, HandleUnmapNotify will have to mash the UnmapNotify
     * into a DestroyNotify.
     */

    if (Tmp_win == NULL)
	return;

    if (Tmp_win == Scr->Focus)
    {
	FocusOnRoot();
    }
    XDeleteContext(dpy, Tmp_win->w, TwmContext);
    XDeleteContext(dpy, Tmp_win->w, ScreenContext);
    XDeleteContext(dpy, Tmp_win->frame, TwmContext);
    XDeleteContext(dpy, Tmp_win->frame, ScreenContext);
    if (Tmp_win->icon_w)
    {
	XDeleteContext(dpy, Tmp_win->icon_w, TwmContext);
	XDeleteContext(dpy, Tmp_win->icon_w, ScreenContext);
    }
    if (Tmp_win->title_height)
    {
	XDeleteContext(dpy, Tmp_win->title_w, TwmContext);
	XDeleteContext(dpy, Tmp_win->title_w, ScreenContext);
	XDeleteContext(dpy, Tmp_win->iconify_w, TwmContext);
	XDeleteContext(dpy, Tmp_win->iconify_w, ScreenContext);
	XDeleteContext(dpy, Tmp_win->resize_w, TwmContext);
	XDeleteContext(dpy, Tmp_win->resize_w, ScreenContext);
	if (Tmp_win->hilite_w)
	{
	    XDeleteContext(dpy, Tmp_win->hilite_w, TwmContext);
	    XDeleteContext(dpy, Tmp_win->hilite_w, ScreenContext);
	}
    }

    XDestroyWindow(dpy, Tmp_win->frame);
    if (Tmp_win->icon_w) {
	XDestroyWindow(dpy, Tmp_win->icon_w);
	IconDown (Tmp_win);
    }
    RemoveIconManager(Tmp_win);
    Tmp_win->prev->next = Tmp_win->next;
    if (Tmp_win->next != NULL)
	Tmp_win->next->prev = Tmp_win->prev;

    free((char *)Tmp_win);
}

void
HandleCreateNotify()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "CreateNotify w = 0x%x\n", Event.xcreatewindow.window);
    fflush(stderr);
    XBell(dpy, 0);
    XSync(dpy, 0);
#endif
    /*
    XSelectInput(dpy, Event.xcreatewindow.window, ~0);
    */
}
/***********************************************************************
 *
 *  Procedure:
 *	HandleMapRequest - MapRequest event handler
 *
 ***********************************************************************
 */

void
HandleMapRequest()
{
    int stat;
    XSizeHints hints;
    int zoom_save;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "MapRequest w = 0x%x\n", Event.xmaprequest.window);
#endif

    Event.xany.window = Event.xmaprequest.window;
    stat = XFindContext(dpy, Event.xany.window, TwmContext, &Tmp_win);
    if (stat == XCNOENT)
	Tmp_win = NULL;

    /* If the window has never been mapped before ... */
    if (Tmp_win == NULL)
    {
	/* Add decorations. */
	Tmp_win = AddWindow(Event.xany.window, FALSE, 0);
	if (Tmp_win == NULL)
	    return;
    }
    else
    {
	/*
	 * If the window has been unmapped by the client, it won't be listed
	 * in the icon manager.  Add it again, if requested.
	 */
	if (Tmp_win->list == NULL)

	    AddIconManager(Tmp_win);
    }

    /* If it's not merely iconified, and we have hints, use them. */
    if ((! Tmp_win->icon) &&
	Tmp_win->wmhints && (Tmp_win->wmhints->flags & StateHint))
    {
	int state;
	Window icon;

	/* use WM_STATE if enabled */
	if (!(RestartPreviousState && GetWMState(Tmp_win->w, &state, &icon) &&
	      (state == NormalState || state == IconicState)))
	  state = Tmp_win->wmhints->initial_state;

	switch (state) 
	{
	    case DontCareState:
	    case NormalState:
	    case ZoomState:
	    case InactiveState:
		XMapWindow(dpy, Tmp_win->w);
		XMapWindow(dpy, Tmp_win->frame);
		SetMapStateProp(Tmp_win, NormalState);
		break;

	    case IconicState:
		zoom_save = Scr->DoZoom;
		Scr->DoZoom = FALSE;
		Iconify(Tmp_win, 0, 0);
		Scr->DoZoom = zoom_save;
		break;
	}
    }
    /* If no hints, or currently an icon, just "deiconify" */
    else
    {
	DeIconify(Tmp_win);
    }
    XRaiseWindow(dpy, Scr->VersionWindow);
    SetHints(Tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMapNotify - MapNotify event handler
 *
 ***********************************************************************
 */

void
HandleMapNotify()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "MapNotify\n");
#endif
    if (Tmp_win == NULL)
	return;

    if (Tmp_win->icon_w)
	XUnmapWindow(dpy, Tmp_win->icon_w);
    if (Tmp_win->title_w)
	XMapSubwindows(dpy, Tmp_win->title_w);
    XMapSubwindows(dpy, Tmp_win->frame);
    if (Scr->Focus != Tmp_win && Tmp_win->hilite_w)
	XUnmapWindow(dpy, Tmp_win->hilite_w);

    XMapWindow(dpy, Tmp_win->frame);
    Tmp_win->mapped = TRUE;
    Tmp_win->icon = FALSE;
    Tmp_win->icon_on = FALSE;

    XRaiseWindow(dpy, Scr->VersionWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleUnmapNotify - UnmapNotify event handler
 *
 ***********************************************************************
 */

void
HandleUnmapNotify()
{
    int dstx, dsty, dumint;
    unsigned int dumuint, bw;
    Window dumwin;
    int gravx, gravy;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "UnmapNotify\n");
#endif

    /*
     * The July 27, 1988 ICCCM spec states that a client wishing to switch
     * to WithdrawnState should send a synthetic UnmapNotify with the
     * event field set to (pseudo-)root, in case the window is already
     * unmapped (which is the case for twm for IconicState).  Unfortunately,
     * we looked for the TwmContext using that field, so try the window
     * field also.
     */
    if (Tmp_win == NULL)
    {
	Event.xany.window = Event.xunmap.window;
	if (XFindContext(dpy, Event.xany.window,
	    TwmContext, &Tmp_win) == XCNOENT)
	    Tmp_win = NULL;
    }

    if (Tmp_win == NULL || (!Tmp_win->mapped && !Tmp_win->icon))
	return;

    /*
     * The program may have unmapped the client window, from either
     * NormalState or IconicState.  Handle the transition to WithdrawnState.
     *
     * We need to reparent the window back to the root (so that twm exiting 
     * won't cause it to get mapped) and then throw away all state (pretend 
     * that we've received a DestroyNotify).
     */

    if (XTranslateCoordinates (dpy, Event.xunmap.window, Tmp_win->attr.root,
			       0, 0, &dstx, &dsty, &dumwin)) {
	SetMapStateProp(Tmp_win, WithdrawnState);
	XUnmapWindow (dpy, Event.xunmap.window);
	XReparentWindow (dpy, Event.xunmap.window, Tmp_win->attr.root,
			 dstx, dsty);
	RestoreWithdrawnLocation (Tmp_win);
	XRemoveFromSaveSet (dpy, Event.xunmap.window);
	/* do not need to mash the event */
	HandleDestroyNotify();
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMotionNotify - MotionNotify event handler
 *
 ***********************************************************************
 */

void
HandleMotionNotify()
{
#ifdef DEBUG_EVENTS
    /*
    fprintf(stderr, "MotionNotify\n");
    */
#endif

    if (ResizeWindow != NULL)
    {
	XFindContext(dpy, ResizeWindow, TwmContext, &Tmp_win);
	DoResize(Event.xmotion.x_root, Event.xmotion.y_root, Tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonRelease - ButtonRelease event handler
 *
 ***********************************************************************
 */

void
HandleButtonRelease()
{
    int xl, xr, yt, yb, w, h;
    unsigned mask;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "ButtonRelease\n");
#endif

    if (DragWindow != NULL)
    {
	XEvent client_event;

	MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);

	XFindContext(dpy, DragWindow, TwmContext, &Tmp_win);
	if (DragWindow == Tmp_win->frame)
	{
	    xl = Event.xbutton.x_root - DragX - Tmp_win->frame_bw;
	    yt = Event.xbutton.y_root - DragY - Tmp_win->frame_bw;
	    w = DragWidth + 2 * Tmp_win->frame_bw;
	    h = DragHeight + 2 * Tmp_win->frame_bw;
	}
	else
	{
	    xl = Event.xbutton.x_root - DragX - BW;
	    yt = Event.xbutton.y_root - DragY - BW;
	    w = DragWidth + 2 * BW;
	    h = DragHeight + 2 * BW;
	}

	if (ConstMove)
	{
	    if (ConstMoveDir == MOVE_HORIZ)
		yt = ConstMoveY;

	    if (ConstMoveDir == MOVE_VERT)
		xl = ConstMoveX;

	    if (ConstMoveDir == MOVE_NONE)
	    {
		yt = ConstMoveY;
		xl = ConstMoveX;
	    }
	}
	
	if (Scr->DontMoveOff && MoveFunction != F_FORCEMOVE)
	{
	    xr = xl + w;
	    yb = yt + h;

	    if (xl < 0)
		xl = 0;
	    if (xr > Scr->MyDisplayWidth)
		xl = Scr->MyDisplayWidth - w;

	    if (yt < 0)
		yt = 0;
	    if (yb > Scr->MyDisplayHeight)
		yt = Scr->MyDisplayHeight - h;
	}

	if (DragWindow == Tmp_win->frame)
	    SetupWindow(Tmp_win, xl, yt,
		Tmp_win->frame_width, Tmp_win->frame_height);
	else
	    XMoveWindow(dpy, DragWindow, xl, yt);

	if (!Scr->NoRaiseMove)
	    XRaiseWindow(dpy, DragWindow);
	DragWindow = NULL;
	ConstMove = FALSE;

	enter_flag = TRUE;
	client_event.type = ClientMessage;
	client_event.xclient.window = Tmp_win->frame;
	client_event.xclient.message_type = twmRaisingWindowAtom;
	client_event.xclient.format = 32;
	XSendEvent(dpy, Tmp_win->frame, False, 0, &client_event);
	SetHints(Tmp_win);
    }

    if (ResizeWindow != NULL)
    {
	EndResize();
    }

    if (ActiveMenu != NULL && RootFunction == NULL)
    {
	MenuRoot *tmp;

	if (ActiveItem != NULL)
	{
	    Action = ActiveItem->action;
	    if (ActiveItem->func == F_MOVE ||
		ActiveItem->func == F_FORCEMOVE)
		    ButtonPressed = -1;
	    ExecuteFunction(ActiveItem->func, ActiveItem->action,
		ButtonWindow ? ButtonWindow->frame : NULL,
		ButtonWindow, &ButtonEvent, Context, TRUE);
	    Context = C_NO_CONTEXT;
	    ButtonWindow = NULL;

	    /* if we are not executing a defered command, then take down the
	     * menu
	     */
	    if (RootFunction == NULL)
	    {
		PopDownMenu();
	    }
	}
	else
	    PopDownMenu();

	if (RestorePointer)
	    XWarpPointer(dpy, None, Scr->Root, 0,0,0,0, StartingX, StartingY);

    }

    mask = (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask);
    switch (Event.xbutton.button)
    {
	case Button1: mask &= ~Button1Mask; break;
	case Button2: mask &= ~Button2Mask; break;
	case Button3: mask &= ~Button3Mask; break;
	case Button4: mask &= ~Button4Mask; break;
	case Button5: mask &= ~Button5Mask; break;
    }

    if (RootFunction != NULL ||
	ResizeWindow != NULL ||
	DragWindow != NULL)
	ButtonPressed = -1;

    if (RootFunction == NULL &&
	(Event.xbutton.state & mask) == 0 &&
	DragWindow == NULL &&
	ResizeWindow == NULL)
    {
	XUngrabPointer(dpy, CurrentTime);
	XUngrabServer(dpy);
	EventHandler[EnterNotify] = HandleEnterNotify;
	EventHandler[LeaveNotify] = HandleLeaveNotify;
	ButtonPressed = -1;
	if (DownIconManager)
	{
	    DownIconManager->down = FALSE;
	    if (Scr->Highlight) DrawIconManagerBorder(DownIconManager);
	    DownIconManager = NULL;
	}
	Cancel = FALSE;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonPress - ButtonPress event handler
 *
 ***********************************************************************
 */

void
HandleButtonPress()
{
    int modifier;
    Cursor cur;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "ButtonPress\n");
#endif

    /* pop down the menu, if any */
    if (ActiveMenu != NULL)
	PopDownMenu();

    XUnmapWindow(dpy, Scr->InfoWindow);
    XUnmapWindow(dpy, Scr->VersionWindow);
    XSync(dpy, 0);

    if (ButtonPressed != -1)
    {
	/* we got another butt press in addition to one still held
	 * down, we need to cancel the operation we were doing
	 */
	Cancel = TRUE;
	MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
	XUnmapWindow(dpy, Scr->SizeWindow);
	ResizeWindow = NULL;
	DragWindow = NULL;
	cur = LeftButt;
	if (Event.xbutton.button == Button2)
	    cur = MiddleButt;
	else if (Event.xbutton.button >= Button3)
	    cur = RightButt;

	XGrabPointer(dpy, Scr->Root, True,
	    ButtonReleaseMask | ButtonPressMask,
	    GrabModeAsync, GrabModeAsync,
	    Scr->Root, cur, CurrentTime);

	return;
    }
    else
	ButtonPressed = Event.xbutton.button;

    if (ResizeWindow != NULL ||
	DragWindow != NULL  ||
	ActiveMenu != NULL)
	return;

    /* check the title bar buttons */
    if (Tmp_win && Tmp_win->title_height)
    {
	if (Event.xany.window == Tmp_win->iconify_w)
	{
	    ExecuteFunction(F_ICONIFY, NULL, Event.xany.window,
		Tmp_win, &Event, C_TITLE, FALSE);
	    return;
	}

	if (Event.xany.window == Tmp_win->resize_w)
	{
	    ExecuteFunction(F_RESIZE, NULL, Event.xany.window, Tmp_win,
		&Event, C_TITLE, FALSE);
	    return;
	}
    }

    Context = C_NO_CONTEXT;

    if (Event.xany.window == Scr->Root)
	Context = C_ROOT;
    if (Tmp_win)
    {
	if (Tmp_win->list && RootFunction != NULL &&
	    (Event.xany.window == Tmp_win->list->w ||
		Event.xany.window == Tmp_win->list->icon))
	{
	    Tmp_win = Tmp_win->list->iconmgr->twm_win;
	    XTranslateCoordinates(dpy, Event.xany.window, Tmp_win->w,
		Event.xbutton.x, Event.xbutton.y, 
		&JunkX, &JunkY, &JunkChild);

	    Event.xbutton.x = JunkX;
	    Event.xbutton.y = JunkY - Tmp_win->title_height;
	    Event.xany.window = Tmp_win->w;
	    Context = C_WINDOW;
	}
	else if (Event.xany.window == Tmp_win->title_w)
	{
	    Context = C_TITLE;
	}
	else if (Event.xany.window == Tmp_win->w)
	    Context = C_WINDOW;
	else if (Event.xany.window == Tmp_win->icon_w)
	{
	    Context = C_ICON;
	}
	else if (Event.xany.window == Tmp_win->frame)
	    Context = C_FRAME;
	else if (Tmp_win->list &&
	    (Event.xany.window == Tmp_win->list->w ||
		Event.xany.window == Tmp_win->list->icon))
	{
	    Tmp_win->list->down = TRUE;
	    if (Scr->Highlight) DrawIconManagerBorder(Tmp_win->list);
	    DownIconManager = Tmp_win->list;
	    Context = C_ICONMGR;
	}
    }

    /* this section of code checks to see if we were in the middle of
     * a command executed from a menu
     */
    if (RootFunction != NULL)
    {
	if (Event.xany.window == Scr->Root)
	{
	    /* if the window was the Root, we don't know for sure it
	     * it was the root.  We must check to see if it happened to be
	     * inside of a client that was getting button press events.
	     */
	    XTranslateCoordinates(dpy, Scr->Root, Scr->Root,
		Event.xbutton.x, 
		Event.xbutton.y, 
		&JunkX, &JunkY, &Event.xany.window);

	    if (Event.xany.window == 0 ||
		(XFindContext(dpy, Event.xany.window, TwmContext,
		    &Tmp_win) == XCNOENT))
	    {
		RootFunction = NULL;
		XBell(dpy, 0);
		return;
	    }

	    XTranslateCoordinates(dpy, Scr->Root, Event.xany.window,
		Event.xbutton.x, 
		Event.xbutton.y, 
		&JunkX, &JunkY, &JunkChild);

	    Event.xbutton.x = JunkX;
	    Event.xbutton.y = JunkY;
	    Context = C_WINDOW;
	}

	ExecuteFunction(RootFunction, Action, Event.xany.window,
	    Tmp_win, &Event, Context, FALSE);

	RootFunction = NULL;
	return;
    }

    ButtonEvent = Event;
    ButtonWindow = Tmp_win;

    /* if we get to here, we have to execute a function or pop up a 
     * menu
     */
    modifier = Event.xbutton.state & (ShiftMask | ControlMask | Mod1Mask);

    if (Context == C_NO_CONTEXT)
	return;

    RootFunction = NULL;
    if (Scr->Mouse[Event.xbutton.button][Context][modifier].func == F_MENU)
    {
	if (!Scr->NoGrabServer)
	    XGrabServer(dpy);
	PopUpMenu(Scr->Mouse[Event.xbutton.button][Context][modifier].menu, 
	    Event.xbutton.x_root, Event.xbutton.y_root);
	UpdateMenu();
    }
    else if (Scr->Mouse[Event.xbutton.button][Context][modifier].func != NULL)
    {
	Action = Scr->Mouse[Event.xbutton.button][Context][modifier].item ?
	    Scr->Mouse[Event.xbutton.button][Context][modifier].item->action : NULL;
	ExecuteFunction(Scr->Mouse[Event.xbutton.button][Context][modifier].func,
	    Action, Event.xany.window, Tmp_win, &Event, Context, FALSE);
    }
    else if (Scr->DefaultFunction.func != NULL)
    {
	if (Scr->DefaultFunction.func == F_MENU)
	{
	    if (!Scr->NoGrabServer)
		XGrabServer(dpy);
	    PopUpMenu(Scr->DefaultFunction.menu, 
		Event.xbutton.x_root, Event.xbutton.y_root);
	    UpdateMenu();
	}
	else
	{
	    Action = Scr->DefaultFunction.item ?
		Scr->DefaultFunction.item->action : NULL;
	    ExecuteFunction(Scr->DefaultFunction.func, Action,
	       Event.xany.window, Tmp_win, &Event, Context, FALSE);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleEnterNotify - EnterNotify event handler
 *
 ***********************************************************************
 */

void
HandleEnterNotify()
{
    MenuRoot *mr;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "EnterNotify\n");
#endif

    if (ActiveMenu == NULL && Tmp_win != NULL)
    {
	if (Scr->FocusRoot && Tmp_win->list)
	{
	    ActiveIconManager(Tmp_win->list);
	}
	if (Scr->FocusRoot && Tmp_win->mapped)
	{
	    if (Scr->Focus != NULL && Scr->Focus != Tmp_win &&Tmp_win->hilite_w)
		XUnmapWindow(dpy, Scr->Focus->hilite_w);

	    if (Event.xcrossing.window == Tmp_win->frame ||
		(Tmp_win->list && Event.xcrossing.window == Tmp_win->list->w))
	    {
		if (Tmp_win->hilite_w)
		    XMapWindow(dpy, Tmp_win->hilite_w);
		XInstallColormap(dpy, Scr->CMap);
		XSetWindowBorder(dpy, Tmp_win->frame, Tmp_win->border);
		if (Tmp_win->title_w)
		    XSetWindowBorder(dpy, Tmp_win->title_w, Tmp_win->border);
		if(Tmp_win->title_w && Scr->TitleFocus)
		    XSetInputFocus(dpy, Tmp_win->w, RevertToPointerRoot,
				   CurrentTime);
		Scr->Focus = Tmp_win;
	    }
	    if (Event.xcrossing.window == Tmp_win->w)
	    {
#ifdef DEBUG_EVENTS
		fprintf(stderr, "    installing cmap 0x%x\n",
		    Tmp_win->attr.colormap);
#endif
		XInstallColormap(dpy, Tmp_win->attr.colormap);
	    }
	}
	if (enter_flag == FALSE && Tmp_win->auto_raise)
	{
	    XEvent client_event;

	    XRaiseWindow(dpy, Tmp_win->frame);
	    enter_flag = TRUE;
	    client_event.type = ClientMessage;
	    client_event.xclient.window = Tmp_win->frame;
	    client_event.xclient.message_type = twmRaisingWindowAtom;
	    client_event.xclient.format = 32;
	    XSendEvent(dpy, Tmp_win->frame, False, 0, &client_event);
	}
	return;
    }


    if (XFindContext(dpy, Event.xany.window, MenuContext, &mr) != 0)
	return;

    mr->entered = TRUE;
    if (ActiveMenu && mr == ActiveMenu->prev && RootFunction == NULL)
    {
	if (Scr->Shadow)
	    XUnmapWindow(dpy, ActiveMenu->shadow);
	XUnmapWindow(dpy, ActiveMenu->w);
	ActiveMenu->mapped = UNMAPPED;
	if (ActiveItem)
	    ActiveItem->state = 0;
	ActiveItem = NULL;
	ActiveMenu = mr;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleLeaveNotify - LeaveNotify event handler
 *
 ***********************************************************************
 */

void
HandleLeaveNotify()
{
    MenuRoot *mr;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "LeaveNotify\n");
    fprintf(stderr, "\tmode %d, detail %d\n", 
	    Event.xcrossing.mode, Event.xcrossing.detail);
#endif
    if (Tmp_win != NULL)
    {
	if (Event.xcrossing.mode == NotifyNormal &&
	    Event.xcrossing.detail != NotifyInferior)
		XUnmapWindow(dpy, Scr->VersionWindow);

	if (Scr->FocusRoot)
	{
	    if (Event.xcrossing.detail != NotifyInferior)
	    {
		if (Event.xcrossing.window == Tmp_win->frame ||
		  (Tmp_win->list && Event.xcrossing.window == Tmp_win->list->w))

		{
		    if (Tmp_win->list)
		    {
			NotActiveIconManager(Tmp_win->list);
		    }
		    if (Tmp_win->hilite_w)
			XUnmapWindow(dpy, Tmp_win->hilite_w);
		    XUninstallColormap(dpy, Scr->CMap);
		    if (Tmp_win->highlight)
		    {
			XSetWindowBorderPixmap(dpy, 
			   Tmp_win->frame, Tmp_win->gray);
			if (Tmp_win->title_w)
			    XSetWindowBorderPixmap(dpy,
			       Tmp_win->title_w,Tmp_win->gray);
		    }
		    if (Scr->TitleFocus)
			XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
				       CurrentTime);
		    Scr->Focus = NULL;
		}
		else if (Event.xcrossing.window == Tmp_win->w)
		{
		    XInstallColormap(dpy, Scr->CMap);
		}
	    }
	}
	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleConfigureRequest - ConfigureRequest event handler
 *
 ***********************************************************************
 */

void
HandleConfigureRequest()
{
    XWindowChanges xwc;
    unsigned int   xwcm;
    int x, y, width, height;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "ConfigureRequest\n");
    if (Event.xconfigurerequest.value_mask & CWX)
	fprintf(stderr, "  x = %d\n", Event.xconfigurerequest.x);
    if (Event.xconfigurerequest.value_mask & CWY)
	fprintf(stderr, "  y = %d\n", Event.xconfigurerequest.y);
    if (Event.xconfigurerequest.value_mask & CWWidth)
	fprintf(stderr, "  width = %d\n", Event.xconfigurerequest.width);
    if (Event.xconfigurerequest.value_mask & CWHeight)
	fprintf(stderr, "  height = %d\n", Event.xconfigurerequest.height);
    if (Event.xconfigurerequest.value_mask & CWSibling)
	fprintf(stderr, "  above = 0x%x\n", Event.xconfigurerequest.above);
    if (Event.xconfigurerequest.value_mask & CWStackMode)
	fprintf(stderr, "  stack = %d\n", Event.xconfigurerequest.detail);
#endif

    Event.xany.window = Event.xconfigurerequest.window;

    /*
     * According to the July 27, 1988 ICCCM draft, we should ignore size and
     * position fields in the WM_NORMAL_HINTS property when we map a window.
     * Instead, we'll read the current geometry.  Therefore, we should respond
     * to configuration requests for windows which have never been mapped.
     */
    if (Tmp_win == NULL)
    {
#ifdef DEBUG_EVENTS
	fprintf(stderr, "  Transient or never-before-mapped window\n");
#endif
	xwcm = Event.xconfigurerequest.value_mask & 
	    (CWX | CWY | CWWidth | CWHeight);
	xwc.x = Event.xconfigurerequest.x;
	xwc.y = Event.xconfigurerequest.y;
	xwc.width = Event.xconfigurerequest.width;
	xwc.height = Event.xconfigurerequest.height;
	XConfigureWindow(dpy, Event.xany.window, xwcm, &xwc);
	return;
    }

    if (Tmp_win == NULL)
	return;


    if (Event.xconfigurerequest.value_mask & CWStackMode)
    {
	if (Event.xconfigurerequest.detail == Above)
	    XRaiseWindow(dpy, Tmp_win->frame);
	else if (Event.xconfigurerequest.detail == Below)
	    XLowerWindow(dpy, Tmp_win->frame);

	return;
    }

    /* Don't modify frame_XXX fields before calling SetupWindow! */
    x = Tmp_win->frame_x;
    y = Tmp_win->frame_y;
    width = Tmp_win->frame_width;
    height = Tmp_win->frame_height;

    if (Event.xconfigurerequest.value_mask & CWX)
	x = Event.xconfigurerequest.x - Tmp_win->title_height;
    if (Event.xconfigurerequest.value_mask & CWY)
	y = Event.xconfigurerequest.y;
    if (Event.xconfigurerequest.value_mask & CWWidth)
	width = Event.xconfigurerequest.width;
    if (Event.xconfigurerequest.value_mask & CWHeight)
	height = Event.xconfigurerequest.height + Tmp_win->title_height;

    if (width != Tmp_win->frame_width || height != Tmp_win->frame_height)
	Tmp_win->zoomed = ZOOM_NONE;

    SetupWindow(Tmp_win, x, y, width, height);
}

#ifdef SHAPE
/***********************************************************************
 *
 *  Procedure:
 *	HandleShapeNotify - shape notification event handler
 *
 ***********************************************************************
 */
void
HandleShapeNotify ()
{
    XShapeEvent	    *sev;
    Window	    w;
    int		    reshape = 0;

    sev = (XShapeEvent *) &Event;
    if (Tmp_win == NULL)
	return;
    if (sev->kind != ShapeBounding)
	return;
    Tmp_win->wShaped = sev->shaped;
    SetFrameShape (Tmp_win);
}
#endif

/***********************************************************************
 *
 *  Procedure:
 *	HandleUnknown - unknown event handler
 *
 ***********************************************************************
 */

void
HandleUnknown()
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "type = %d\n", Event.type);
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *	Transient - checks to see if the window is a transient
 *
 *  Returned Value:
 *	TRUE	- window is a transient
 *	FALSE	- window is not a transient
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int
Transient(w)
    Window w;
{
    Window propw;

    return (XGetTransientForHint(dpy, w, &propw));
}

/***********************************************************************
 *
 *  Procedure:
 *	FindScreenInfo - get ScreenInfo struct associated with a given window
 *
 *  Returned Value:
 *	ScreenInfo struct
 *
 *  Inputs:
 *	w	- the window
 *
 ***********************************************************************
 */

ScreenInfo *
FindScreenInfo(w)
    Window w;
{
    XWindowAttributes attr;
    int scrnum;
    Status stat;

    attr.screen = NULL;
    stat = XGetWindowAttributes(dpy, w, &attr);
#ifdef DEBUG_EVENTS
    fprintf(stderr, "FindScreenInfo stat = %d\n", stat);
#endif

    for (scrnum = 0; scrnum < NumScreens; scrnum++)
    {
	if (ScreenList[scrnum] != NULL &&
	    ScreenOfDisplay(dpy, ScreenList[scrnum]->screen) == attr.screen)
		return(ScreenList[scrnum]);
    }

#ifdef DEBUG_EVENTS
    fprintf(stderr, "FindScreenInfo(0x%x) returning NULL\n", w);
    fprintf(stderr, "  attr.screen = %d\n", attr.screen);
#endif
    return(NULL);
}


static void flush_expose (w)
    Window w;
{
    XEvent dummy;

    while (XCheckTypedWindowEvent (dpy, w, Expose, &dummy)) ;
}

