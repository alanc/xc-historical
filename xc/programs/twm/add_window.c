/****************************************************************************/
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

/**********************************************************************
 *
 * $XConsortium: add_window.c,v 1.35 89/04/13 10:00:55 jim Exp $
 *
 * Add a new window, put the titlbar and other stuff around
 * the window
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: add_window.c,v 1.35 89/04/13 10:00:55 jim Exp $";
#endif /* lint */

#include <stdio.h>
#include "twm.h"
#include <X11/Xatom.h>
#include "add_window.h"
#include "util.h"
#include "resize.h"
#include "gram.h"
#include "list.h"
#include "events.h"
#include "menus.h"
#include "screen.h"
#include "iconmgr.h"

#include "hilite.bm"
#include "gray.bm"

int AddingX;
int AddingY;
int AddingW;
int AddingH;

static int PlaceX = 50;
static int PlaceY = 50;

char NoName[] = "No Name"; /* name if no name is specified */

/***********************************************************************
 *
 *  Procedure:
 *	AddWindow - add a new window to the twm list
 *
 *  Returned Value:
 *	(TwmWindow *) - pointer to the TwmWindow structure
 *
 *  Inputs:
 *	w	- the window id of the window to add
 *	iconm	- flag to tell if this is an icon manager window
 *	iconp	- pointer to icon manager struct
 *
 ***********************************************************************
 */

TwmWindow *
AddWindow(w, iconm, iconp)
Window w;
int iconm;
IconMgr *iconp;
{
    TwmWindow *tmp_win;			/* new twm window structure */
    int stat;
    char *prop;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int width, len;			/* tmp variable */
    int junk1, junk2, junk3;
    int x;
    XWindowChanges xwc;		/* change window structure */
    unsigned int xwcm;		/* change window mask */
    int dont_know;		/* don't know where to put the window */
    XColor blob, cret;
    XEvent event;
    XGCValues	    gcv;
    unsigned long   gcm, mask;
    XWindowAttributes gattr;


#ifdef DEBUG
    fprintf(stderr, "AddWindow: w = 0x%x\n", w);
#endif

    /* allocate space for the twm window */
    tmp_win = (TwmWindow *)malloc(sizeof(TwmWindow));
    tmp_win->w = w;
    tmp_win->zoomed = ZOOM_NONE;
    tmp_win->iconmgr = iconm;
    tmp_win->iconmgrp = iconp;

    XSelectInput(dpy, tmp_win->w, PropertyChangeMask);
    XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);
    XFetchName(dpy, tmp_win->w, &tmp_win->name);
    tmp_win->class = NoClass;
    XGetClassHint(dpy, tmp_win->w, &tmp_win->class);

#ifdef DEBUG
    fprintf(stderr, "  name = \"%s\"\n", tmp_win->name);
#endif
    tmp_win->wmhints = XGetWMHints(dpy, tmp_win->w);
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & WindowGroupHint))
	tmp_win->group = tmp_win->wmhints->window_group;
    else
	tmp_win->group = NULL;

    if (XGetNormalHints(dpy, tmp_win->w, &tmp_win->hints) == 0)
	tmp_win->hints.flags = 0;

    if (strncmp("xterm", tmp_win->class.res_name, 5) == 0 ||
    	strncmp("XTerm", tmp_win->class.res_class, 5) == 0 ||
        strncmp("hpterm", tmp_win->class.res_name, 6) == 0 ||
    	strncmp("HPterm", tmp_win->class.res_class, 6) == 0)
	tmp_win->xterm = TRUE;
    else
	tmp_win->xterm = FALSE;

    /*
     * The July 27, 1988 draft of the ICCCM ignores the size and position
     * fields in the WM_NORMAL_HINTS property.
     */
    XGetGeometry(dpy, tmp_win->w, &JunkRoot,
		 &tmp_win->attr.x, &tmp_win->attr.y,
		 &tmp_win->attr.width, &tmp_win->attr.height,
		 &JunkBW, &JunkDepth);

    if (tmp_win->hints.flags & USPosition)
    {
#ifdef DEBUG
	fprintf(stderr, "	user-specified hints\n");
#endif
	dont_know = FALSE;
    }
    else
    {
#ifdef DEBUG
	fprintf(stderr, "	no user-specified hints\n");
#endif
	dont_know = TRUE;
    }

    if (tmp_win->name == NULL)
	tmp_win->name = NoName;
    if (tmp_win->class.res_name == NULL)
    	tmp_win->class.res_name = NoName;
    if (tmp_win->class.res_class == NULL)
    	tmp_win->class.res_class = NoName;

    tmp_win->full_name = tmp_win->name;

    tmp_win->highlight = Scr->Highlight && 
	(!(short)LookInList(Scr->NoHighlight, tmp_win->full_name, 
	    &tmp_win->class));

    tmp_win->titlehighlight = Scr->TitleHighlight && 
	(!(short)LookInList(Scr->NoTitleHighlight, tmp_win->full_name, 
	    &tmp_win->class));

    tmp_win->auto_raise = (short)LookInList(Scr->AutoRaise, tmp_win->full_name, 
	&tmp_win->class);
    tmp_win->iconify_by_unmapping = Scr->IconifyByUnmapping;
    if (Scr->IconifyByUnmapping)
    {
	tmp_win->iconify_by_unmapping = 
	    !(short)LookInList(Scr->DontIconify, tmp_win->full_name,
		&tmp_win->class);
    }
    tmp_win->iconify_by_unmapping |= 
	(short)LookInList(Scr->IconifyByUn, tmp_win->full_name,
	    &tmp_win->class);

    tmp_win->old_bw = tmp_win->attr.border_width;

    tmp_win->bw = 0;
    tmp_win->frame_bw = Scr->BorderWidth;
    tmp_win->title_bw = Scr->BorderWidth;

    tmp_win->title_height = Scr->TitleHeight + tmp_win->title_bw;
    if (Scr->NoTitlebar)
        tmp_win->title_height = 0;
    if (LookInList(Scr->MakeTitle, tmp_win->full_name, &tmp_win->class))
        tmp_win->title_height = Scr->TitleHeight + tmp_win->title_bw;
    if (LookInList(Scr->NoTitle, tmp_win->full_name, &tmp_win->class))
        tmp_win->title_height = 0;

    if (LookInList(Scr->StartIconified, tmp_win->full_name, &tmp_win->class))
    {
	if (!tmp_win->wmhints)
	{
	    tmp_win->wmhints = (XWMHints *)malloc(sizeof(XWMHints));
	    tmp_win->wmhints->flags = 0;
	}
	tmp_win->wmhints->initial_state = IconicState;
	tmp_win->wmhints->flags |= StateHint;
    }

    if (HandlingEvents && dont_know && !Scr->RandomPlacement)
    {
	if (!(tmp_win->wmhints && tmp_win->wmhints->flags & StateHint &&
	      tmp_win->wmhints->initial_state == IconicState)
#ifdef USE_PROGRAM_SPECIFIED_HINTS
	    && (tmp_win->xterm || (tmp_win->attr.x==0 && tmp_win->attr.y==0))
#endif
	    )
	{
	    /* better wait until all the mouse buttons have been 
	     * released.
	     */
	    while (TRUE)
	    {
		XUngrabServer(dpy);
		XSync(dpy, 0);
#ifndef NO_GRAB
		XGrabServer(dpy);
#endif
		XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		if (JunkMask != 0)
		    continue;

		stat = XGrabPointer(dpy, Scr->Root, False,
		    ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync,
		    Scr->Root, UpperLeftCursor, CurrentTime);

		if (stat == GrabSuccess)
		    break;
	    }

	    width = XTextWidth(Scr->InitialFont.font, tmp_win->name,
		strlen(tmp_win->name)) + 20;
	    XResizeWindow(dpy, Scr->InitialWindow, width, 
			  Scr->InitialFont.height + 4);
	    XMapRaised(dpy, Scr->InitialWindow);

	    FBF(Scr->DefaultC.fore, Scr->DefaultC.back,
		Scr->InitialFont.font->fid);
	    XDrawImageString(dpy, Scr->InitialWindow, Scr->NormalGC,
		10, 2 + Scr->InitialFont.font->ascent,
		tmp_win->name, strlen(tmp_win->name));

	    AddingW = tmp_win->attr.width;
	    AddingH = tmp_win->attr.height;

	    AddingW = tmp_win->attr.width + 2*(tmp_win->bw + tmp_win->frame_bw);
	    AddingH = tmp_win->attr.height + tmp_win->title_height +
		2 * (tmp_win->bw + tmp_win->frame_bw);

	    while (TRUE)
	    {
		XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		MoveOutline(Scr->Root, AddingX, AddingY, AddingW, AddingH);

		if (XCheckTypedEvent(dpy, ButtonPress, &event))
		{
		    XEvent junk;

		    AddingX = event.xbutton.x_root;
		    AddingY = event.xbutton.y_root;
		    XMaskEvent(dpy, ButtonReleaseMask, &junk);
		    break;
		}
	    }

	    if (event.xbutton.button == Button2)
	    {
		XWarpPointer(dpy, None, Scr->Root, 0, 0, 0, 0,
		    AddingX + AddingW/2, AddingY + AddingH/2);
		AddStartResize(tmp_win, AddingX, AddingY, AddingW, AddingH);

		while (TRUE)
		{
		    int lastx, lasty;

		    XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
			&JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		    if (lastx != AddingX || lasty != AddingY)
		    {
			DoResize(AddingX, AddingY, tmp_win);

			lastx = AddingX;
			lasty = AddingY;
		    }

		    if (XCheckTypedEvent(dpy, ButtonRelease, &event))
		    {
			AddEndResize(tmp_win);
			break;
		    }
		}
	    }

	    MoveOutline(Scr->Root, 0, 0, 0, 0);
	    XUnmapWindow(dpy, Scr->InitialWindow);
	    XUngrabPointer(dpy, CurrentTime);

	    tmp_win->attr.x = AddingX;
	    tmp_win->attr.y = AddingY + tmp_win->title_height;
	    tmp_win->attr.width = AddingW - 2*(tmp_win->bw + tmp_win->frame_bw);
	    tmp_win->attr.height = AddingH - tmp_win->title_height -
		2*(tmp_win->bw + tmp_win->frame_bw);

	    XUngrabServer(dpy);
	}
    }
    else if (HandlingEvents && dont_know && Scr->RandomPlacement)
    {
	if ((PlaceX + tmp_win->attr.width) > Scr->MyDisplayWidth)
	    PlaceX = 50;
	if ((PlaceY + tmp_win->attr.height) > Scr->MyDisplayHeight)
	    PlaceY = 50;

	tmp_win->attr.x = PlaceX;
	tmp_win->attr.y = PlaceY;
	PlaceX += 30;
	PlaceY += 30;
    }

#ifdef OLD_WAY
    if (tmp_win->attr.y < tmp_win->title_height)
	tmp_win->attr.y = tmp_win->title_height;
#endif

    xwcm = CWX | CWY | CWWidth | CWHeight;

#ifdef DEBUG
	fprintf(stderr, "  position window  %d, %d  %dx%d\n", 
	    tmp_win->attr.x,
	    tmp_win->attr.y,
	    tmp_win->attr.width,
	    tmp_win->attr.height);
#endif
    xwc.x = tmp_win->attr.x + tmp_win->attr.border_width;
    xwc.y = tmp_win->attr.y + tmp_win->attr.border_width;
    xwc.width = tmp_win->attr.width;
    xwc.height = tmp_win->attr.height;
    if (Scr->BorderWidth)
    {
	xwc.border_width = 0;
        xwcm |= CWBorderWidth;
    }

    XConfigureWindow(dpy, tmp_win->w, xwcm, &xwc);

    tmp_win->name_width = XTextWidth(Scr->TitleBarFont.font, tmp_win->name,
	strlen(tmp_win->name));

    if (XGetWindowProperty(dpy, tmp_win->w, XA_WM_ICON_NAME, 0, 200, False,
	XA_STRING, &junk1, &junk2, &junk3, &len, &tmp_win->icon_name))
	tmp_win->icon_name = tmp_win->name;

    if (tmp_win->icon_name == NULL)
	tmp_win->icon_name = tmp_win->name;

    tmp_win->iconified = FALSE;
    tmp_win->icon = FALSE;
    tmp_win->icon_on = FALSE;

#ifndef NO_GRAB
    XGrabServer(dpy);
#endif

    /*
     * Make sure the client window still exists.  We don't want to leave an
     * orphan frame window if it doesn't.  Since we now have the server 
     * grabbed, the window can't disappear later without having been 
     * reparented, so we'll get a DestroyNotify for it.  We won't have 
     * gotten one for anything up to here, however.
     */
    if (XGetGeometry(dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
		     &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
    {
	free((char *)tmp_win);
	XUngrabServer(dpy);
	return(NULL);
    }

    /* add the window into the twm list */
    tmp_win->next = Scr->TwmRoot.next;
    if (Scr->TwmRoot.next != NULL)
	Scr->TwmRoot.next->prev = tmp_win;
    tmp_win->prev = &Scr->TwmRoot;
    Scr->TwmRoot.next = tmp_win;

    /* get all the colors for the window */

    tmp_win->border = Scr->BorderColor;
    tmp_win->icon_border = Scr->IconBorderColor;
    tmp_win->border_tile.fore = Scr->BorderTileC.fore;
    tmp_win->border_tile.back = Scr->BorderTileC.back;
    tmp_win->title.fore = Scr->TitleC.fore;
    tmp_win->title.back = Scr->TitleC.back;
    tmp_win->iconc.fore = Scr->IconC.fore;
    tmp_win->iconc.back = Scr->IconC.back;

    GetColorFromList(Scr->BorderColorL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->border);
    GetColorFromList(Scr->IconBorderColorL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->icon_border);
    GetColorFromList(Scr->BorderTileForegroundL, tmp_win->full_name,
	&tmp_win->class, &tmp_win->border_tile.fore);
    GetColorFromList(Scr->BorderTileBackgroundL, tmp_win->full_name,
	&tmp_win->class, &tmp_win->border_tile.back);
    GetColorFromList(Scr->TitleForegroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->title.fore);
    GetColorFromList(Scr->TitleBackgroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->title.back);
    GetColorFromList(Scr->IconForegroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->iconc.fore);
    GetColorFromList(Scr->IconBackgroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->iconc.back);


    /* create windows */

    tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw - tmp_win->frame_bw;
    tmp_win->frame_y = tmp_win->attr.y - tmp_win->title_height +
	tmp_win->old_bw - tmp_win->frame_bw;
    tmp_win->frame_width = tmp_win->attr.width + 2 * tmp_win->bw;
    tmp_win->frame_height = tmp_win->attr.height + tmp_win->title_height +
        2 * tmp_win->bw;

    valuemask = CWBackPixmap | CWBorderPixel;
    attributes.background_pixmap = None;
    attributes.border_pixel = tmp_win->border;

    tmp_win->frame = XCreateWindow(dpy, Scr->Root,
	tmp_win->frame_x, tmp_win->frame_y,
	tmp_win->frame_width, tmp_win->frame_height,
	tmp_win->frame_bw,
	Scr->d_depth, CopyFromParent,
	Scr->d_visual, valuemask, &attributes);
    
    tmp_win->title_x = -Scr->BorderWidth;
    tmp_win->title_y = -Scr->BorderWidth;

    if (tmp_win->title_height)
    {
	tmp_win->title_w = XCreateSimpleWindow(dpy, tmp_win->frame,
	    tmp_win->title_x, tmp_win->title_y,
	    tmp_win->attr.width, Scr->TitleHeight,
	    tmp_win->title_bw,
	    tmp_win->border, tmp_win->title.back);
    }
    else
	tmp_win->title_w = 0;

    if (tmp_win->highlight)
    {
	tmp_win->gray = XCreatePixmapFromBitmapData(dpy, Scr->Root, 
	    gray_bits, gray_width, gray_height, 
	    tmp_win->border_tile.fore, tmp_win->border_tile.back,
	    Scr->d_depth);

	XSetWindowBorderPixmap(dpy, tmp_win->frame, tmp_win->gray);
	if (tmp_win->title_w)
	    XSetWindowBorderPixmap(dpy, tmp_win->title_w, tmp_win->gray);
    }

    CreateTitleButtons(tmp_win);
	
    XDefineCursor(dpy, tmp_win->frame, Scr->FrameCursor);
    if (tmp_win->title_height)
    {
	XDefineCursor(dpy, tmp_win->title_w, Scr->TitleCursor);
	XDefineCursor(dpy, tmp_win->iconify_w, Scr->ButtonCursor);
#ifndef NOFOCUS
	XDefineCursor(dpy, tmp_win->focus_w, Scr->ButtonCursor);
#endif
	XDefineCursor(dpy, tmp_win->resize_w, Scr->ButtonCursor);
    }

    XSelectInput(dpy, tmp_win->w, StructureNotifyMask | PropertyChangeMask |
	ColormapChangeMask | EnterWindowMask | LeaveWindowMask);

    attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
    XChangeWindowAttributes(dpy, tmp_win->w, CWDontPropagate, &attributes);

#ifdef SHAPE
    if (HasShape)
	XShapeSelectInput (dpy, tmp_win->w, True);
#endif
	
    /* find out what his color map really is */
    XGetWindowAttributes(dpy, tmp_win->w, &gattr);
    tmp_win->attr.colormap = gattr.colormap;

    XSelectInput(dpy, tmp_win->frame,
	SubstructureRedirectMask | VisibilityChangeMask |
	ButtonPressMask | ButtonReleaseMask |
	EnterWindowMask | LeaveWindowMask);

    if (tmp_win->title_w)
	XSelectInput(dpy, tmp_win->title_w, 
	    KeyPressMask |
	    ButtonPressMask | ButtonReleaseMask | ExposureMask);

#ifdef SHAPE
    if (HasShape) {
	int xws, yws, xbs, ybs;
	unsigned wws, hws, wbs, hbs;
	int boundingShaped, clipShaped;

	XShapeSelectInput (dpy, tmp_win->w, True);
	XShapeQueryExtents (dpy, tmp_win->w,
			    &boundingShaped, &xws, &yws, &wws, &hws,
			    &clipShaped, &xbs, &ybs, &wbs, &hbs);
	tmp_win->wShaped = boundingShaped;
	tmp_win->fShaped = -1;
    }
#endif

    if (!tmp_win->iconmgr)
	XAddToSaveSet(dpy, tmp_win->w);
	
    XReparentWindow(dpy, tmp_win->w, tmp_win->frame, 0, tmp_win->title_height);
    /*
     * Reparenting generates an UnmapNotify event, followed by a MapNotify.
     * Set the map state to FALSE to prevent a transition back to
     * WithdrawnState in HandleUnmapNotify.  Map state gets set correctly
     * again in HandleMapNotify.
     */
    tmp_win->mapped = FALSE;

    SetupWindow(tmp_win,
	tmp_win->frame_x, tmp_win->frame_y,
	tmp_win->frame_width, tmp_win->frame_height);

    /* wait until the window is iconified and the icon window is mapped
     * before creating the icon window 
     */
    tmp_win->icon_w = NULL;

    if (!tmp_win->iconmgr)
    {
	GrabButtons(tmp_win);
	GrabKeys(tmp_win);
    }

    AddIconManager(tmp_win);

    XSaveContext(dpy, tmp_win->w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->w, ScreenContext, Scr);
    XSaveContext(dpy, tmp_win->frame, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->frame, ScreenContext, Scr);
    if (tmp_win->title_height)
    {
	XSaveContext(dpy, tmp_win->title_w, TwmContext, tmp_win);
	XSaveContext(dpy, tmp_win->title_w, ScreenContext, Scr);
	XSaveContext(dpy, tmp_win->iconify_w, TwmContext, tmp_win);
	XSaveContext(dpy, tmp_win->iconify_w, ScreenContext, Scr);
	XSaveContext(dpy, tmp_win->resize_w, TwmContext, tmp_win);
	XSaveContext(dpy, tmp_win->resize_w, ScreenContext, Scr);
#ifndef NOFOCUS
	XSaveContext(dpy, tmp_win->focus_w, TwmContext, tmp_win);
	XSaveContext(dpy, tmp_win->focus_w, ScreenContext, Scr);
#endif
	if (tmp_win->hilite_w)
	{
	    XSaveContext(dpy, tmp_win->hilite_w, TwmContext, tmp_win);
	    XSaveContext(dpy, tmp_win->hilite_w, ScreenContext, Scr);
	}
    }

    SetHints(tmp_win);
    XUngrabServer(dpy);

    /* if we were in the middle of a menu activated function, regrab
     * the pointer 
     */
    if (RootFunction)
	ReGrab();

    return (tmp_win);
}


/***********************************************************************
 *
 *  Procedure:
 *	MappedNotOverride - checks to see if we should really
 *		put a twm frame on the window
 *
 *  Returned Value:
 *	TRUE	- go ahead and frame the window
 *	FALSE	- don't frame the window
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int
MappedNotOverride(w)
    Window w;
{
    XWindowAttributes wa;

    XGetWindowAttributes(dpy, w, &wa);
    return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabAllButtons - grab needed buttons for all windows
 *
 ***********************************************************************
 */

void
GrabAllButtons()
{
    TwmWindow *tmp_win;

    for (tmp_win = Scr->TwmRoot.next;
	 tmp_win != NULL;
	 tmp_win = tmp_win->next)
    {
	GrabButtons(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabAllKeys - grab needed keys for all windows
 *
 ***********************************************************************
 */

void
GrabAllKeys()
{
    TwmWindow *tmp_win;

    for (tmp_win = Scr->TwmRoot.next;
	 tmp_win != NULL; 
	 tmp_win = tmp_win->next)
    {
	GrabKeys(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabAllButtons - ungrab buttons for all windows
 *
 ***********************************************************************
 */

void
UngrabAllButtons()
{
    TwmWindow *tmp_win;

    for (tmp_win = Scr->TwmRoot.next;
	 tmp_win != NULL;
	 tmp_win = tmp_win->next)
    {
	UngrabButtons(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabAllKeys - ungrab keys for all windows
 *
 ***********************************************************************
 */

void
UngrabAllKeys()
{
    TwmWindow *tmp_win;

    for (tmp_win = Scr->TwmRoot.next;
	 tmp_win != NULL;
	 tmp_win = tmp_win->next)
    {
	UngrabKeys(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabButtons - grab needed buttons for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
GrabButtons(tmp_win)
TwmWindow *tmp_win;
{
    int i, j;

    for (i = 0; i < MAX_BUTTONS+1; i++)
    {
	for (j = 0; j < MOD_SIZE; j++)
	{
	    if (Scr->Mouse[i][C_WINDOW][j].func != NULL)
	    {
		XGrabButton(dpy, i, j, tmp_win->w,
		    True, ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync, None, Scr->FrameCursor);
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabKeys - grab needed keys for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
GrabKeys(tmp_win)
TwmWindow *tmp_win;
{
    FuncKey *tmp;
    IconMgr *p;

    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	switch (tmp->cont)
	{
	case C_WINDOW:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w, True,
		GrabModeAsync, GrabModeAsync);
	    break;

	case C_ICON:
	    if (tmp_win->icon_w)
		XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w, True,
		    GrabModeAsync, GrabModeAsync);

	case C_TITLE:
	    if (tmp_win->title_w)
		XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w, True,
		    GrabModeAsync, GrabModeAsync);
	    break;

	case C_NAME:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w, True,
		GrabModeAsync, GrabModeAsync);
	    if (tmp_win->icon_w)
		XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w, True,
		    GrabModeAsync, GrabModeAsync);
	    if (tmp_win->title_w)
		XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w, True,
		    GrabModeAsync, GrabModeAsync);
	    break;
	/*
	case C_ROOT:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, Scr->Root, True,
		GrabModeAsync, GrabModeAsync);
	    break;
	*/
	}
    }
    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->cont == C_ICONMGR && !Scr->NoIconManagers)
	{
	    for (p = &Scr->iconmgr; p != NULL; p = p->next)
	    {
		XUngrabKey(dpy, tmp->keycode, tmp->mods, p->twm_win->w);
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabButtons - ungrab buttons for windows
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
UngrabButtons(tmp_win)
TwmWindow *tmp_win;
{
    int i;

    for (i = 0; i < MAX_BUTTONS+1; i++)
    {
	XUngrabButton(dpy, i, AnyModifier, tmp_win->w);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabKeys - ungrab keys for windows
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
UngrabKeys(tmp_win)
TwmWindow *tmp_win;
{
    FuncKey *tmp;

    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	switch (tmp->cont)
	{
	case C_WINDOW:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w);
	    break;

	case C_ICON:
	    if (tmp_win->icon_w)
		XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w);

	case C_TITLE:
	    if (tmp_win->title_w)
		XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w);
	    break;

	case C_ROOT:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, Scr->Root);
	    break;
	}
    }
}

CreateTitleButtons(tmp_win)
TwmWindow *tmp_win;
{
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int x, y;

    if (tmp_win->title_height == 0)
    {
	tmp_win->iconify_w = 0;
#ifndef NOFOCUS
	tmp_win->focus_w = 0;
#endif
	tmp_win->resize_w = 0;
	tmp_win->hilite_w = 0;
	return;
    }

    if (Scr->iconifyPm == NULL)
    {
	GC gc;
	int w, x1, x2, y1, y2;

	Scr->iconifyPm = XCreatePixmap(dpy, tmp_win->title_w,
	    Scr->TitleHeight, Scr->TitleHeight, 1);
	gc = XCreateGC(dpy, Scr->iconifyPm, (unsigned long)0, (XGCValues *)0);
	XSetForeground(dpy, gc, 0);
	XFillRectangle(dpy, Scr->iconifyPm, gc, 0,0,
	    Scr->TitleHeight, Scr->TitleHeight);

#ifndef NOFOCUS
	Scr->focusPm = XCreatePixmap(dpy, tmp_win->title_w,
	    Scr->TitleHeight, Scr->TitleHeight, 1);
	XFillRectangle(dpy, Scr->focusPm, gc, 0,0,
	    Scr->TitleHeight, Scr->TitleHeight);
#endif
	Scr->resizePm = XCreatePixmap(dpy, tmp_win->title_w,
	    Scr->TitleHeight, Scr->TitleHeight, 1);
	XFillRectangle(dpy, Scr->iconifyPm, gc, 0,0,
	    Scr->TitleHeight, Scr->TitleHeight);

	/* now draw the images in */
	XSetForeground(dpy, gc, 1);

	/* first the iconify button */
	XDrawRectangle(dpy, Scr->iconifyPm, gc, 1, 1,
	    Scr->TitleHeight - 3, Scr->TitleHeight - 3);
	w = (Scr->TitleHeight - 6) / 2;
	XFillRectangle(dpy, Scr->iconifyPm, gc, 3, 3, w, w);
	XFillRectangle(dpy, Scr->iconifyPm, gc, 3+w+1, 3, w, w);
	XFillRectangle(dpy, Scr->iconifyPm, gc, 3, 3+w+1, w, w);
	XFillRectangle(dpy, Scr->iconifyPm, gc, 3+w+1, 3+w+1, w, w);

#ifndef NOFOCUS
	/* draw the focus button */
	XDrawRectangle(dpy, Scr->focusPm, gc, 1, 1,
	    Scr->TitleHeight - 3, Scr->TitleHeight - 3);
	XDrawArc(dpy, Scr->focusPm, gc, 3, 3,
	    Scr->TitleHeight - 7, Scr->TitleHeight - 7,
	    0, 360*64);
	XDrawPoint(dpy, Scr->focusPm, gc, 
	    Scr->TitleHeight/2, Scr->TitleHeight/2);
#endif

	/* now the resize button */
	XDrawRectangle(dpy, Scr->resizePm, gc, 1, 1,
	    Scr->TitleHeight - 3, Scr->TitleHeight - 3);
	w = (Scr->TitleHeight - 4) / 3;
	x1 = 2;
	y1 = 1+w;
	x2 = Scr->TitleHeight - 2 - w;
	y2 = y1;
	XDrawLine(dpy, Scr->resizePm, gc, x1, y1, x2, y2);
	x1 = x2;
	y2 = Scr->TitleHeight - 2;
	XDrawLine(dpy, Scr->resizePm, gc, x1, y1, x2, y2);
	x1 = 2;
	y1 = 1+(2*w);
	x2 = Scr->TitleHeight - 2 - (2*w);
	y2 = y1;
	XDrawLine(dpy, Scr->resizePm, gc, x1, y1, x2, y2);
	x1 = x2;
	y2 = Scr->TitleHeight - 2;
	XDrawLine(dpy, Scr->resizePm, gc, x1, y1, x2, y2);

	XFreeGC(dpy, gc);
    }

    tmp_win->iconify_w = XCreateSimpleWindow(dpy, tmp_win->title_w,
	0, 0, Scr->TitleHeight, Scr->TitleHeight,
	0, Scr->Black, tmp_win->title.back);
    XSelectInput(dpy, tmp_win->iconify_w, ButtonPressMask |
	ButtonReleaseMask | ExposureMask);

#ifndef NOFOCUS
    tmp_win->focus_w = XCreateSimpleWindow(dpy, tmp_win->title_w,
	0, 0, Scr->TitleHeight, Scr->TitleHeight,
	0, Scr->Black, tmp_win->title.back);
    XSelectInput(dpy, tmp_win->focus_w, ButtonPressMask |
	ButtonReleaseMask | ExposureMask);
#endif

    tmp_win->resize_w = XCreateSimpleWindow(dpy, tmp_win->title_w,
	0, 0, Scr->TitleHeight, Scr->TitleHeight,
	0, Scr->Black, tmp_win->title.back);
    XSelectInput(dpy, tmp_win->resize_w, ButtonPressMask |
	ButtonReleaseMask | ExposureMask);

    if (tmp_win->titlehighlight)
    {
	GC gc;
	int h;

	h = Scr->TitleHeight - 4;
	x = 2;

	tmp_win->hilite_pm = XCreatePixmap(dpy, tmp_win->title_w,
	    8, h, Scr->d_depth);
	gc = XCreateGC(dpy, tmp_win->hilite_pm,(unsigned long)0,(XGCValues *)0);
	XSetForeground(dpy, gc, tmp_win->title.back);
	XFillRectangle(dpy, tmp_win->hilite_pm, gc, 0,0, 8, h);
	XSetForeground(dpy, gc, tmp_win->title.fore);
	for (y = 1; y < h; y += 2)
	    XDrawLine(dpy, tmp_win->hilite_pm, gc, 0, y, 10, y);
	valuemask = CWBackPixmap;
	attributes.background_pixmap = tmp_win->hilite_pm;

	tmp_win->hilite_w = XCreateWindow(dpy, tmp_win->title_w,
	    TitleBarX, x,
	    8, h,
	    0, Scr->d_depth, CopyFromParent,
	    Scr->d_visual, valuemask, &attributes);

	XFreeGC(dpy, gc);
    }
    else
	tmp_win->hilite_w = 0;

    XMapSubwindows(dpy, tmp_win->title_w);
    if (tmp_win->hilite_w)
	XUnmapWindow(dpy, tmp_win->hilite_w);
}

