/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
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
 * $XConsortium: events.c,v 1.97 89/11/03 13:26:52 jim Exp $
 *
 * twm event handling
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: events.c,v 1.97 89/11/03 13:26:52 jim Exp $";
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
#include "screen.h"
#include "iconmgr.h"
#include "siconify.bm"
#include "version.h"

#define MAX_X_EVENT 256
event_proc EventHandler[MAX_X_EVENT]; /* event handler jump table */
char *Action;
int Context = C_NO_CONTEXT;	/* current button press context */
TwmWindow *ButtonWindow;	/* button press window structure */
XEvent ButtonEvent;		/* button press event */
XEvent Event;			/* the current event */
TwmWindow *Tmp_win;		/* the current twm window */

Window DragWindow;		/* variables used in moving windows */
int origDragX;
int origDragY;
int DragX;
int DragY;
int DragWidth;
int DragHeight;

static int enter_flag;
static TwmWindow *enter_win, *raise_win;

ScreenInfo *FindScreenInfo();
int ButtonPressed = -1;
int ButtonNeeded = -1;
int Cancel = FALSE;

void HandleCreateNotify();

#ifdef SHAPE
void HandleShapeNotify ();
extern int ShapeEventBase, ShapeErrorBase;
#endif

static void do_autoraise_window (tmp)
    TwmWindow *tmp;
{
    XRaiseWindow (dpy, tmp->frame);
    XSync (dpy, 0);
    enter_flag = TRUE;
    raise_win = tmp;
}


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


    ResizeWindow = NULL;
    DragWindow = NULL;
    enter_flag = FALSE;
    enter_win = raise_win = NULL;

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
	EventHandler[ShapeEventBase+ShapeNotify] = HandleShapeNotify;
#endif
}


Time lastTimestamp = CurrentTime;	/* until Xlib does this for us */

Bool StashEventTime (ev)
    register XEvent *ev;
{
    switch (ev->type) {
      case KeyPress:
      case KeyRelease:
	lastTimestamp = ev->xkey.time;
	return True;
      case ButtonPress:
      case ButtonRelease:
	lastTimestamp = ev->xbutton.time;
	return True;
      case MotionNotify:
	lastTimestamp = ev->xmotion.time;
	return True;
      case EnterNotify:
      case LeaveNotify:
	lastTimestamp = ev->xcrossing.time;
	return True;
      case PropertyNotify:
	lastTimestamp = ev->xproperty.time;
	return True;
      case SelectionClear:
	lastTimestamp = ev->xselectionclear.time;
	return True;
      case SelectionRequest:
	lastTimestamp = ev->xselectionrequest.time;
	return True;
      case SelectionNotify:
	lastTimestamp = ev->xselection.time;
	return True;
    }
    return False;
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
    StashEventTime (&Event);

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
	    if (enter_flag && !QLength(dpy)) {
		if (enter_win && enter_win != raise_win) {
		    do_autoraise_window (enter_win);  /* sets enter_flag T */
		} else {
		    enter_flag = FALSE;
		}
	    }
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

    if (cevent->window == Scr->Root)
    {
	XWindowAttributes attr;

	/* Did the colormap change? */
	if (cevent->new == True)
	    Scr->CMap = cevent->colormap;

	/* If the pointer is not over a child, install it */
	if (cevent->state == ColormapUninstalled)
	{
	    sameScreen = XQueryPointer (dpy, Scr->Root, &JunkRoot, &child,
					&JunkX, &JunkY, &JunkX, &JunkY,
					&JunkMask);
	    if (sameScreen && child == None)
	    {
#ifdef DEBUG_EVENTS
		fprintf(stderr, "    installing 0x%x\n",
		    Tmp_win->attr.colormap);
		printf ("installa from HandleColormapNotify\n");
#endif
		InstallAColormap(dpy, Scr->CMap);
	    }
	}
	return;
    }

    if (Tmp_win == NULL)
	return;

#ifdef DEBUG_EVENTS
    printf ("Colormap:  win 0x%lx, cmap 0x%lx, new %d, state ",
	    cevent->window, cevent->colormap, cevent->new);
    switch (cevent->state) {
      case ColormapInstalled: printf ("Installed"); break;
      case ColormapUninstalled: printf ("Uninstalled"); break;
      default:  printf ("%d", cevent->state); break;
    }
    printf ("; twin 0x%lx\n", Tmp_win->w);
#endif

    if (cevent->window == Tmp_win->w)
    {
	XWindowAttributes attr;

	/* Did the client change its colormap? */
	if (cevent->new == True)
	    Tmp_win->attr.colormap = cevent->colormap;


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
		printf ("installa from HandleColormapNotify\n");
#endif
		InstallAColormap(dpy, Tmp_win->attr.colormap);
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

    if (InfoLines) XUnmapWindow(dpy, Scr->InfoWindow);
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

static void free_window_names (tmp, nukefull, nukename, nukeicon)
    TwmWindow *tmp;
    Bool nukefull, nukename, nukeicon;
{
/*
 * XXX - are we sure that nobody ever sets these to another constant (check
 * twm windows)?
 */
    if (tmp->name == tmp->full_name) nukefull = False;
    if (tmp->icon_name == tmp->name) nukename = False;

#define isokay(v) ((v) && (v) != NoName)
    if (nukefull && isokay(tmp->full_name)) XFree (tmp->full_name);
    if (nukename && isokay(tmp->name)) XFree (tmp->name);
    if (nukeicon && isokay(tmp->icon_name)) XFree (tmp->icon_name);
#undef isokay
    return;
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
    unsigned long supplied;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "PropertyNotify = %d\n", Event.xproperty.atom);
#endif

    /* watch for standard colormap changes */
    if (Event.xproperty.window == Scr->Root) {
	XStandardColormap *maps = NULL;
	int nmaps;

	switch (Event.xproperty.state) {
	  case PropertyNewValue:
	    if (XGetRGBColormaps (dpy, Scr->Root, &maps, &nmaps, 
				  Event.xproperty.atom)) {
		/* if got one, then replace any existing entry */
		InsertRGBColormap (Event.xproperty.atom, maps, nmaps, True);
	    }
	    return;

	  case PropertyDelete:
	    RemoveRGBColormap (Event.xproperty.atom);
	    return;
	}
    }

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
	free_window_names (Tmp_win, True, True, False);

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
	free_window_names (Tmp_win, False, False, True);
	Tmp_win->icon_name = prop;

	RedoIconName();
	break;

    case XA_WM_HINTS:
	if (Tmp_win->wmhints) XFree (Tmp_win->wmhints);
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
	(void) XGetWMNormalHints (dpy, Tmp_win->w, &Tmp_win->hints, &supplied);
	break;

    default:
	if (Event.xproperty.atom == _XA_WM_COLORMAP_WINDOWS) {
	    if (Tmp_win->cmap_windows) {
		if (Tmp_win->xfree_cmap_windows) {
		    XFree ((char *) Tmp_win->cmap_windows);
		} else {
		    free ((char *) Tmp_win->cmap_windows);
		}
	    }
	    FetchWmProtocols (Tmp_win);
	    break;
	}
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

    if (Event.xclient.message_type == _XA_WM_CHANGE_STATE)
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

    if (Event.xany.window == Scr->InfoWindow && InfoLines)
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
    else if (Tmp_win != NULL)
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
	else if (Event.xany.window == Tmp_win->iconify_w)
	{
	    FB(Tmp_win->title.fore, Tmp_win->title.back);
	    XCopyPlane(dpy, Scr->iconifyPm, Tmp_win->iconify_w, Scr->NormalGC,
		0,0, h, h, 0, 0, 1);
	    flush_expose (Event.xany.window);
	    return;
	}
	else if (Event.xany.window == Tmp_win->resize_w)
	{
	    FB(Tmp_win->title.fore, Tmp_win->title.back);
	    XCopyPlane(dpy, Scr->resizePm, Tmp_win->resize_w, Scr->NormalGC,
		0,0, h, h, 0, 0, 1);
	    flush_expose (Event.xany.window);
	    return;
  	}
	else if (Event.xany.window == Tmp_win->icon_w)
	{
	    FBF(Tmp_win->iconc.fore, Tmp_win->iconc.back,
		Scr->IconFont.font->fid);

	    XDrawString (dpy, Tmp_win->icon_w,
		Scr->NormalGC,
		Tmp_win->icon_x, Tmp_win->icon_y,
		Tmp_win->icon_name, strlen(Tmp_win->icon_name));
	    flush_expose (Event.xany.window);
	    return;
	} else {
	    int i;
	    Window w = Event.xany.window;
	    register TBWindow *tbw;

	    for (i = 0, tbw = Tmp_win->titlebuttons; i < Scr->TBInfo.nbuttons;
		 i++, tbw++) {
		if (w == tbw->window) {
		    register TitleButton *tb = tbw->info;

		    FB(Tmp_win->title.fore, Tmp_win->title.back);
		    XCopyPlane (dpy, tb->bitmap, w, Scr->NormalGC,
				tb->srcx, tb->srcy, tb->width, tb->height,
				tb->dstx, tb->dsty, 1);
		    flush_expose (w);
		    return;
		}
	    }
	}
	if (Tmp_win->list) {
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
    int i;

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
	for (i = 0; i < Scr->TBInfo.nbuttons; i++) {
	    XDeleteContext(dpy, Tmp_win->titlebuttons[i].window, TwmContext);
	    XDeleteContext(dpy, Tmp_win->titlebuttons[i].window, ScreenContext);
        }
    }

    /*
     * TwmWindows contain the following pointers
     * 
     *     1.  full_name
     *     2.  name
     *     3.  icon_name
     *     4.  wmhints
     *     5.  class.res_name
     *     6.  class.res_class
     *     7.  list
     *     8.  iconmgrp
     *     9.  cmap_windows
     *     10. titlebuttons
     */
    if (Tmp_win->gray) XFreePixmap (dpy, Tmp_win->gray);

    XDestroyWindow(dpy, Tmp_win->frame);
    if (Tmp_win->icon_w) {
	XDestroyWindow(dpy, Tmp_win->icon_w);
	IconDown (Tmp_win);
    }
    RemoveIconManager(Tmp_win);					/* 7 */
    Tmp_win->prev->next = Tmp_win->next;
    if (Tmp_win->next != NULL)
	Tmp_win->next->prev = Tmp_win->prev;
    if (Tmp_win->auto_raise) Scr->NumAutoRaises--;

    free_window_names (Tmp_win, True, True, True, True);	/* 1, 2, 3 */
    XFree ((char *)Tmp_win->wmhints);				/* 4 */
    if (Tmp_win->class.res_name && Tmp_win->class.res_name != NoName)  /* 5 */
      XFree ((char *)Tmp_win->class.res_name);
    if (Tmp_win->class.res_class && Tmp_win->class.res_class != NoName) /* 6 */
      XFree ((char *)Tmp_win->class.res_class);
    if (Tmp_win->cmap_windows) {				/* 9 */
	if (Tmp_win->xfree_cmap_windows) {
	    XFree ((char *) Tmp_win->cmap_windows);
	} else {
	    free ((char *) Tmp_win->cmap_windows);
	}
    }
    if (Tmp_win->titlebuttons) free ((char *) Tmp_win->titlebuttons);  /* 10 */
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

    if (enter_win == Tmp_win) enter_win = NULL;

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

    if (DragWindow != None)
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
	if (Scr->NumAutoRaises) {
	    XSync (dpy, 0);
	    enter_flag = TRUE;
	    enter_win = NULL;
	    raise_win = ((DragWindow == Tmp_win->frame && !Scr->NoRaiseMove)
			 ? Tmp_win : NULL);
	}
	DragWindow = NULL;
	ConstMove = FALSE;
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
	ResizeWindow != None ||
	DragWindow != None)
	ButtonPressed = -1;

    if (RootFunction == NULL &&
	(Event.xbutton.state & mask) == 0 &&
	DragWindow == None &&
	ResizeWindow == None)
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

static do_menu (menu)
    MenuRoot *menu;
{
    if (!Scr->NoGrabServer)
	XGrabServer(dpy);
    PopUpMenu (menu, Event.xbutton.x_root, Event.xbutton.y_root);
    UpdateMenu();
}


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

    if (InfoLines) {
	XUnmapWindow(dpy, Scr->InfoWindow);
	InfoLines = 0;
    }
    XSync(dpy, 0);			/* XXX - remove? */

    if (ButtonPressed != -1)
    {
	/* we got another butt press in addition to one still held
	 * down, we need to cancel the operation we were doing
	 */
	Cancel = TRUE;
	if (Scr->OpaqueMove && DragWindow != None) {
	    XMoveWindow (dpy, DragWindow, origDragX, origDragY);
	} else {
	    MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
	}
	XUnmapWindow(dpy, Scr->SizeWindow);
	ResizeWindow = None;
	DragWindow = None;
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

    if (ResizeWindow != None ||
	DragWindow != None  ||
	ActiveMenu != NULL)
	return;

    /* check the title bar buttons */
    if (Tmp_win && Tmp_win->title_height)
    {
	int i;
	TBWindow *tbw;

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

	for (i = 0, tbw = Tmp_win->titlebuttons; i < Scr->TBInfo.nbuttons;
	     i++, tbw++) {
	    if (Event.xany.window == tbw->window) {
		if (tbw->info->func == F_MENU) {
		    ButtonEvent = Event;
		    ButtonWindow = Tmp_win;
		    do_menu (tbw->info->menuroot);
		} else {
		    ExecuteFunction (tbw->info->func, tbw->info->action,
				     Event.xany.window, Tmp_win, &Event,
				     C_TITLE, FALSE);
		}
		return;
	    }
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
	do_menu (Scr->Mouse[Event.xbutton.button][Context][modifier].menu);
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
	    do_menu (Scr->DefaultFunction.menu);
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

    if (ActiveMenu == NULL && Event.xcrossing.window == Scr->Root)
    {
#ifdef DEBUG_EVENTS
	printf ("installa from EnterNotify into frame or icon\n");
#endif
	InstallAColormap(dpy, Scr->CMap);
	return;
    }

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
#ifdef DEBUG_EVENTS
		printf ("installa from EnterNotify into frame or icon\n");
#endif
		InstallAColormap(dpy, Scr->CMap);
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
		printf ("installa from window 0x%lx\n", Tmp_win->w);
#endif
		InstallAColormap(dpy, Tmp_win->attr.colormap);
	    }
	}
	if (Tmp_win->auto_raise) {
	    enter_win = Tmp_win;
	    if (enter_flag == FALSE) do_autoraise_window (Tmp_win);
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
#ifdef DEBUG_EVENTS
		    printf ("installa from leaving window 0x%lx\n", Tmp_win->w);
#endif
		    InstallAColormap(dpy, Scr->CMap);
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



InstallWindowColormaps (tmp_win)
    TwmWindow *tmp_win;
{
    Screen *s = ScreenOfDisplay (dpy, Scr->screen);
    int maxcmaps = MaxCmapsOfScreen (s);
    int i, n;

    if (tmp_win->cmap_windows) {
	/*
	 * keep track of how many there are and then only install
	 * the last max colormaps; we'll also need track the colormap
	 * notify events to know which colormaps are present.
	 */
	n = tmp_win->number_cmap_windows;
	for (i = tmp_win->current_cmap_window - 1; i >= 0; i--) {
	    if (n-- <= maxcmaps)
	      InstallWindowColormap (tmp_win->cmap_windows[i]);
	}
	for (i = tmp_win->number_cmap_windows - 1;
	     i >= tmp_win->current_cmap_window; i--) {
	    if (n-- <= maxcmaps)
	      InstallWindowColormap (tmp_win->cmap_windows[i]);
	}
    } else {
	InstallWindowColormap (tmp_win->attr.colormap);
    }
}

InstallWindowColormap (w)
    Window w;
{
    XWindowAttributes attr;

    if (XGetWindowAttributes (dpy, w, &attr)) {
	InstallAColormap (dpy, attr.colormap);
    }
}

InstallAColormap (dpy, cmap)
    Display *dpy;
    Colormap cmap;
{
#ifdef DEBUG_EVENTS
    printf ("Installing colormap 0x%lx\n", cmap);
#endif

    XInstallColormap (dpy, cmap);
}

