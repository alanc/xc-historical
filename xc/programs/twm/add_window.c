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


/**********************************************************************
 *
 * $XConsortium: add_window.c,v 1.89 89/08/15 13:56:24 jim Exp $
 *
 * Add a new window, put the titlbar and other stuff around
 * the window
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: add_window.c,v 1.89 89/08/15 13:56:24 jim Exp $";
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

char NoName[] = "Untitled"; /* name if no name is specified */


/************************************************************************
 *
 *  Procedure:
 *	GetGravityOffsets - map gravity to (x,y) offset signs for adding
 *		to x and y when window is mapped to get proper placement.
 * 
 ************************************************************************
 */

GetGravityOffsets (tmp, xp, yp)
    TwmWindow *tmp;			/* window from which to get gravity */
    int *xp, *yp;			/* return values */
{
    static struct _gravity_offset {
	int x, y;
    } gravity_offsets[11] = {
	{  0,  0 },			/* ForgetGravity */
	{ -1, -1 },			/* NorthWestGravity */
	{  0, -1 },			/* NorthGravity */
	{  1, -1 },			/* NorthEastGravity */
	{ -1,  0 },			/* WestGravity */
	{  0,  0 },			/* CenterGravity */
	{  1,  0 },			/* EastGravity */
	{ -1,  1 },			/* SouthWestGravity */
	{  0,  1 },			/* SouthGravity */
	{  1,  1 },			/* SouthEastGravity */
	{  0,  0 },			/* StaticGravity */
    };
    register int g = ((tmp->hints.flags & PWinGravity) 
		      ? tmp->hints.win_gravity : NorthWestGravity);

    if (g < ForgetGravity || g > StaticGravity) {
	*xp = *yp = 0;
    } else {
	*xp = gravity_offsets[g].x;
	*yp = gravity_offsets[g].y;
    }
}


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
    int ask_user;		/* don't know where to put the window */
    XColor blob, cret;
    XEvent event;
    XGCValues	    gcv;
    unsigned long   gcm, mask;
    XWindowAttributes gattr;
    long supplied;
    int gravx, gravy;			/* gravity signs for positioning */

#ifdef DEBUG
    fprintf(stderr, "AddWindow: w = 0x%x\n", w);
#endif

    /* allocate space for the twm window */
    tmp_win = (TwmWindow *)calloc(1, sizeof(TwmWindow));
    tmp_win->w = w;
    tmp_win->zoomed = ZOOM_NONE;
    tmp_win->iconmgr = iconm;
    tmp_win->iconmgrp = iconp;

    XSelectInput(dpy, tmp_win->w, PropertyChangeMask);
    XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);
    XFetchName(dpy, tmp_win->w, &tmp_win->name);
    tmp_win->class = NoClass;
    XGetClassHint(dpy, tmp_win->w, &tmp_win->class);
    FetchWmProtocols (tmp_win);
    FetchWmColormapWindows (tmp_win);

#ifdef DEBUG
    fprintf(stderr, "  name = \"%s\"\n", tmp_win->name);
#endif
    tmp_win->wmhints = XGetWMHints(dpy, tmp_win->w);
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & WindowGroupHint))
	tmp_win->group = tmp_win->wmhints->window_group;
    else
	tmp_win->group = NULL;

    if (!XGetWMNormalHints (dpy, tmp_win->w, &tmp_win->hints, &supplied))
      tmp_win->hints.flags = 0;

    /*
     * The July 27, 1988 draft of the ICCCM ignores the size and position
     * fields in the WM_NORMAL_HINTS property.
     */

    tmp_win->transient = Transient(tmp_win->w);
    /*
     * Don't bother user if:
     * 
     *     o  the window is a transient, or
     * 
     *     o  a USPosition was requested, or
     * 
     *     o  a PPosition was requested and UsePPosition is ON or
     *        NON_ZERO if the window is at other than (0,0)
     */
    ask_user = TRUE;
    if (tmp_win->transient || 
	(tmp_win->hints.flags & USPosition) ||
        ((tmp_win->hints.flags & PPosition) && Scr->UsePPosition &&
	 (Scr->UsePPosition == PPOS_ON || 
	  tmp_win->attr.x != 0 || tmp_win->attr.y != 0)))
      ask_user = FALSE;

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
    if (Scr->ClientBorderWidth) {
    	tmp_win->frame_bw = tmp_win->old_bw;
    	tmp_win->title_bw = tmp_win->old_bw;
    } else {
    	tmp_win->frame_bw = Scr->BorderWidth;
    	tmp_win->title_bw = Scr->BorderWidth;
    }

    tmp_win->title_height = Scr->TitleHeight + tmp_win->title_bw;
    if (Scr->NoTitlebar)
        tmp_win->title_height = 0;
    if (LookInList(Scr->MakeTitle, tmp_win->full_name, &tmp_win->class))
        tmp_win->title_height = Scr->TitleHeight + tmp_win->title_bw;
    if (LookInList(Scr->NoTitle, tmp_win->full_name, &tmp_win->class))
        tmp_win->title_height = 0;

    /* if it is a transient window, don't put a title on it */
    if (tmp_win->transient && !Scr->DecorateTransients)
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

    GetGravityOffsets (tmp_win, &gravx, &gravy);


    /*
     * do any prompting for position
     */
    if (HandlingEvents && ask_user) {
      if (Scr->RandomPlacement) {	/* just stick it somewhere */
	if ((PlaceX + tmp_win->attr.width) > Scr->MyDisplayWidth)
	    PlaceX = 50;
	if ((PlaceY + tmp_win->attr.height) > Scr->MyDisplayHeight)
	    PlaceY = 50;

	tmp_win->attr.x = PlaceX;
	tmp_win->attr.y = PlaceY;
	PlaceX += 30;
	PlaceY += 30;
      } else {				/* else prompt */
	if (!(tmp_win->wmhints && tmp_win->wmhints->flags & StateHint &&
	      tmp_win->wmhints->initial_state == IconicState))
	{
	    Bool firsttime = True;

	    /* better wait until all the mouse buttons have been 
	     * released.
	     */
	    while (TRUE)
	    {
		XUngrabServer(dpy);
		XSync(dpy, 0);
		XGrabServer(dpy);

		(void) XQueryPointer (dpy, Scr->Root, &JunkRoot, 
				      &JunkChild, &JunkX, &JunkY,
				      &AddingX, &AddingY, &JunkMask);

		/*
		 * watch out for changing screens
		 */
		if (firsttime) {
		    if (JunkRoot != Scr->Root) {
			register int scrnum;

			for (scrnum = 0; scrnum < NumScreens; scrnum++) {
			    if (JunkRoot == RootWindow (dpy, scrnum)) break;
			}

			if (scrnum != NumScreens) PreviousScreen = scrnum;
		    }
		    firsttime = False;
		}

		/*
		 * wait for buttons to come up; yuck
		 */
		if (JunkMask != 0) continue;

		/* 
		 * this will cause a warp to the indicated root
		 */
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

		MoveOutline(Scr->Root, AddingX, AddingY, AddingW, AddingH,
			    tmp_win->frame_bw, tmp_win->title_height);

		if (XCheckTypedEvent(dpy, ButtonPress, &event))
		{
		    XEvent junk;

		    AddingX = event.xbutton.x_root;
		    AddingY = event.xbutton.y_root;
		    if (!Scr->AutoRelativeResize)
		      XMaskEvent(dpy, ButtonReleaseMask, &junk);
		    break;
		}
	    }

	    if (event.xbutton.button == Button2)
	    {
		if (Scr->AutoRelativeResize) {
		    int dx = (tmp_win->attr.width / 4);
		    int dy = (tmp_win->attr.height / 4);
		    
#define HALF_AVE_CURSOR_SIZE 8		/* so that it is visible */
		    if (dx < HALF_AVE_CURSOR_SIZE) dx = HALF_AVE_CURSOR_SIZE;
		    if (dy < HALF_AVE_CURSOR_SIZE) dy = HALF_AVE_CURSOR_SIZE;
#undef HALF_AVE_CURSOR_SIZE
		    dx += (tmp_win->frame_bw + 1);
		    dy += (tmp_win->frame_bw * 2 + tmp_win->title_height + 1);
		    if (AddingX + dx >= Scr->MyDisplayWidth)
		      dx = Scr->MyDisplayWidth - AddingX - 1;
		    if (AddingY + dy >= Scr->MyDisplayHeight)
		      dy = Scr->MyDisplayHeight - AddingY - 1;
		    if (dx > 0 && dy > 0)
		      XWarpPointer (dpy, None, None, 0, 0, 0, 0, dx, dy);
		} else {
		    XWarpPointer (dpy, None, Scr->Root, 0, 0, 0, 0,
				  AddingX + AddingW/2, AddingY + AddingH/2);
		}
		AddStartResize(tmp_win, AddingX, AddingY, AddingW, AddingH);

		while (TRUE)
		{
		    int lastx, lasty;

		    /*
		     * XXX - if we are going to do a loop, we ought to consider
		     * using multiple GXxor lines so that we don't need to 
		     * grab the server.
		     */
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
	    else if (event.xbutton.button == Button3)
	    {
		int maxw = Scr->MyDisplayWidth - AddingX;
		int maxh = Scr->MyDisplayHeight - AddingY;

		/*
		 * Make window go to bottom of screen, and clip to right edge.
		 * This is useful when popping up large windows and fixed
		 * column text windows.
		 */
		if (AddingW > maxw) AddingW = maxw;
		AddingH = maxh;

		/* includes any border */
		ConstrainSize (tmp_win, &AddingW, &AddingH);
	    }

	    MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
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
    } else {				/* put it where asked, mod title bar */
	/* if the gravity is towards the top, move it by the title height */
	if (gravy < 0) tmp_win->attr.y -= gravy * tmp_win->title_height;
    }


    xwcm = CWX | CWY | CWWidth | CWHeight;

#ifdef DEBUG
	fprintf(stderr, "  position window  %d, %d  %dx%d\n", 
	    tmp_win->attr.x,
	    tmp_win->attr.y,
	    tmp_win->attr.width,
	    tmp_win->attr.height);
#endif

    if (!Scr->ClientBorderWidth) {	/* need to adjust for twm borders */
	int delta = tmp_win->attr.border_width - tmp_win->frame_bw;
	tmp_win->attr.x += gravx * delta;
	tmp_win->attr.y += gravy * delta;
    }

					/* set up the configure */
    xwc.x = tmp_win->attr.x + tmp_win->attr.border_width;
    xwc.y = tmp_win->attr.y + tmp_win->attr.border_width;
    xwc.width = tmp_win->attr.width;
    xwc.height = tmp_win->attr.height;
    if (tmp_win->frame_bw)
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

    XGrabServer(dpy);

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

    valuemask = CWBackPixmap | CWBorderPixel | CWCursor | CWEventMask;
    attributes.background_pixmap = None;
    attributes.border_pixel = tmp_win->border;
    attributes.cursor = Scr->FrameCursor;
    attributes.event_mask = (SubstructureRedirectMask | VisibilityChangeMask |
			     ButtonPressMask | ButtonReleaseMask |
			     EnterWindowMask | LeaveWindowMask);

    tmp_win->frame = XCreateWindow (dpy, Scr->Root, tmp_win->frame_x,
				    tmp_win->frame_y, tmp_win->frame_width,
				    tmp_win->frame_height, tmp_win->frame_bw,
				    Scr->d_depth, CopyFromParent,
				    Scr->d_visual, valuemask, &attributes);
    
    tmp_win->title_x = -tmp_win->title_bw;
    tmp_win->title_y = -tmp_win->title_bw;

    if (tmp_win->title_height)
    {
	valuemask = (CWEventMask | CWBorderPixel | CWBackPixel);
	attributes.event_mask = (KeyPressMask | ButtonPressMask |
				 ButtonReleaseMask | ExposureMask);
	attributes.border_pixel = tmp_win->border;
	attributes.background_pixel = tmp_win->title.back;
	tmp_win->title_w = XCreateWindow (dpy, tmp_win->frame,
					  tmp_win->title_x, tmp_win->title_y,
					  tmp_win->attr.width, 
					  Scr->TitleHeight, tmp_win->title_bw,
					  Scr->d_depth, CopyFromParent,
					  Scr->d_visual, valuemask,
					  &attributes);
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
    else
	tmp_win->gray = None;

    CreateTitleButtons(tmp_win);
	
    if (tmp_win->title_height)
    {
	XDefineCursor(dpy, tmp_win->title_w, Scr->TitleCursor);
	XDefineCursor(dpy, tmp_win->iconify_w, Scr->ButtonCursor);
	XDefineCursor(dpy, tmp_win->resize_w, Scr->ButtonCursor);
    }

    valuemask = (CWEventMask | CWDontPropagate);
    attributes.event_mask = (StructureNotifyMask | PropertyChangeMask |
			     ColormapChangeMask |
			     EnterWindowMask | LeaveWindowMask);
    attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
    XChangeWindowAttributes (dpy, tmp_win->w, valuemask, &attributes);

#ifdef SHAPE
    if (HasShape)
	XShapeSelectInput (dpy, tmp_win->w, True);
#endif
	
    if (tmp_win->title_w) {
	XMapWindow (dpy, tmp_win->title_w);
    }

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
	if (tmp_win->hilite_w)
	{
	    XSaveContext(dpy, tmp_win->hilite_w, TwmContext, tmp_win);
	    XSaveContext(dpy, tmp_win->hilite_w, ScreenContext, Scr);
	}
    }

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

#define TITLEBUTTON_BORDERWIDTH 1


CreateTitleButtons(tmp_win)
TwmWindow *tmp_win;
{
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int x, y, h;
    GC gc;
    XGCValues gcv;

    if (tmp_win->title_height == 0)
    {
	tmp_win->iconify_w = 0;
	tmp_win->resize_w = 0;
	tmp_win->hilite_w = 0;
	return;
    }

    h = (Scr->TitleHeight - 2 *
	 (Scr->FramePadding + Scr->ButtonIndent + TITLEBUTTON_BORDERWIDTH));

    if (Scr->iconifyPm == NULL)
    {
	XSegment segs[4];
	GC gcBack;
	int w;

	Scr->iconifyPm = XCreatePixmap (dpy, tmp_win->title_w, h, h, 1);
	gc = XCreateGC (dpy, Scr->iconifyPm, 0L, NULL);
	XSetForeground(dpy, gc, 0);
	XFillRectangle (dpy, Scr->iconifyPm, gc, 0, 0, h, h);

	Scr->resizePm = XCreatePixmap (dpy, tmp_win->title_w, h, h, 1);
	XFillRectangle (dpy, Scr->resizePm, gc, 0, 0, h, h);

	/* now draw the images in */
	XSetForeground(dpy, gc, 1);

	/* first the iconify button */
	gcBack = XCreateGC (dpy, Scr->iconifyPm, 0L, NULL);
	XSetForeground (dpy, gcBack, 0);

	/*
	 * draw the logo large so that it gets as dense as possible; then white
	 * out the edges so that they look crisp
	 */
	XmuDrawLogo (dpy, Scr->iconifyPm, gc, gcBack, -1, -1, h + 2, h + 2);
	XDrawRectangle (dpy, Scr->iconifyPm, gcBack, 0, 0, h - 1, h - 1);
	XFreeGC (dpy, gcBack);

	/*
	 * draw the resize button, 
	 */
	w = (h * 2) / 3;
	segs[0].x1 = w; segs[0].y1 = 0; segs[0].x2 = w; segs[0].y2 = w;
	segs[1].x1 = 0; segs[1].y1 = w; segs[1].x2 = w; segs[1].y2 = w;
	w = w / 2;
	segs[2].x1 = w; segs[2].y1 = 0; segs[2].x2 = w; segs[2].y2 = w;
	segs[3].x1 = 0; segs[3].y1 = w; segs[3].x2 = w; segs[3].y2 = w;
	XDrawSegments (dpy, Scr->resizePm, gc, segs, 4);


	/*
	 * done drawing
	 */
	XFreeGC(dpy, gc);
    }

    /*
     * create the title bar windows; let the event handler deal with painting
     * so that we don't have to spend two pixmaps (or deal with hashing)
     */
    attributes.win_gravity = NorthWestGravity;
    attributes.background_pixel = tmp_win->title.back;
    attributes.border_pixel = tmp_win->title.fore;
    attributes.event_mask = (ButtonPressMask | ButtonReleaseMask |
			     ExposureMask);
    valuemask = (CWWinGravity | CWBackPixel | CWBorderPixel | CWEventMask);
    x = y = (Scr->FramePadding + Scr->ButtonIndent);
    tmp_win->iconify_w = XCreateWindow (dpy, tmp_win->title_w, x, y,
					h, h, TITLEBUTTON_BORDERWIDTH, 0, 
					CopyFromParent, CopyFromParent,
				        valuemask, &attributes);
    attributes.win_gravity = NorthEastGravity;
    x = (tmp_win->attr.width - Scr->FramePadding - Scr->ButtonIndent - h - 1 -
	 TITLEBUTTON_BORDERWIDTH);
    tmp_win->resize_w = XCreateWindow (dpy, tmp_win->title_w, x, y,
					h, h, 1, 0, 
					CopyFromParent, CopyFromParent,
				        valuemask, &attributes);

    h = (Scr->TitleHeight - 2 * Scr->FramePadding);

    if (tmp_win->titlehighlight) {
	Pixmap pm = None;

	/*
	 * If a special highlight pixmap was given, use that.  Otherwise,
	 * use a nice, even gray pattern.  The old horizontal lines look really
	 * awful on interlaced monitors (as well as resembling other looks a
	 * little bit too closely), but can be used by putting
	 *
	 *                 Pixmaps { TitleHighlight "hline2" }
	 *
	 * (or whatever the horizontal line bitmap is named) in the startup
	 * file.  If all else fails, use the foreground color to look like a 
	 * solid line.
	 */
	if (!Scr->hilitePm) {
	    Scr->hilitePm = XCreateBitmapFromData (dpy, tmp_win->title_w, 
						   gray_bits, gray_width, 
						   gray_height);
	    Scr->hilite_pm_width = gray_width;
	    Scr->hilite_pm_height = gray_height;
	}
	if (Scr->hilitePm) {
	    pm = XCreatePixmap (dpy, tmp_win->title_w,
				Scr->hilite_pm_width, Scr->hilite_pm_height,
				Scr->d_depth);
	    gcv.foreground = tmp_win->title.fore;
	    gcv.background = tmp_win->title.back;
	    gcv.graphics_exposures = False;
	    gc = XCreateGC (dpy, pm,
			    (GCForeground|GCBackground|GCGraphicsExposures),
			    &gcv);
	    if (gc) {
		XCopyPlane (dpy, Scr->hilitePm, pm, gc, 0, 0, 
			    Scr->hilite_pm_width, Scr->hilite_pm_height,
			    0, 0, 1);
		XFreeGC (dpy, gc);
	    } else {
		XFreePixmap (dpy, pm);
		pm = None;
	    }
	}
	if (pm) {
	    valuemask = CWBackPixmap;
	    attributes.background_pixmap = pm;
	} else {
	    valuemask = CWBackPixel;
	    attributes.background_pixel = tmp_win->title.fore;
	}

	tmp_win->hilite_w = XCreateWindow (dpy, tmp_win->title_w,
					   TitleBarX, Scr->FramePadding, 8, h,
					   0, Scr->d_depth, CopyFromParent,
					   Scr->d_visual, valuemask,
					   &attributes);
	if (pm) XFreePixmap (dpy, pm);
    }
    else
	tmp_win->hilite_w = 0;

    XMapSubwindows(dpy, tmp_win->title_w);
    if (tmp_win->hilite_w)
	XUnmapWindow(dpy, tmp_win->hilite_w);
}

SetHighlightPixmap (filename)
    char *filename;
{
    Pixmap pm = GetBitmap (filename);

    if (pm) {
	if (Scr->hilitePm) {
	    XFreePixmap (dpy, Scr->hilitePm);
	}
	Scr->hilitePm = pm;
	Scr->hilite_pm_width = JunkWidth;
	Scr->hilite_pm_height = JunkHeight;
    }
}


FetchWmProtocols (tmp)
    TwmWindow *tmp;
{
    unsigned long flags = 0L;
    Atom *protocols = NULL;
    int n;

    if (XGetWMProtocols (dpy, tmp->w, &protocols, &n)) {
	register int i;
	register Atom *ap;

	for (i = 0, ap = protocols; i < n; i++, ap++) {
	    if (*ap == _XA_WM_TAKE_FOCUS) flags |= DoesWmTakeFocus;
	    if (*ap == _XA_WM_SAVE_YOURSELF) flags |= DoesWmSaveYourself;
	    if (*ap == _XA_WM_DELETE_WINDOW) flags |= DoesWmDeleteWindow;
	}
	if (protocols) XFree ((char *) protocols);
    }
    tmp->protocols = flags;
}


FetchWmColormapWindows (tmp)
    TwmWindow *tmp;
{
    register int i;

    tmp->cmap_windows = NULL;
    tmp->number_cmap_windows = 0;
    tmp->current_cmap_window = 0;
    tmp->xfree_cmap_windows = False;

    if (XGetWMColormapWindows (dpy, tmp->w, &tmp->cmap_windows, 
			       &tmp->number_cmap_windows) &&
	tmp->number_cmap_windows > 0) {
	Bool has_top_cmap = False;

	for (i = 0; i < tmp->number_cmap_windows; i++) {
	    if (tmp->w == tmp->cmap_windows[i]) {
		has_top_cmap = True;
		break;
	    }
	}
	if (has_top_cmap) {
	    tmp->xfree_cmap_windows = True;
	} else {
	    Window *wl = (Window *) malloc ((tmp->number_cmap_windows + 1) *
					    sizeof (Window));

	    if (wl) {				/* insert new element */
		Window *src, *dst;

		wl[0] = tmp->w;
		for (i = 0, src = tmp->cmap_windows, dst = wl + 1; 
		     i < tmp->number_cmap_windows; i++, src++, dst++) {
		    *dst = *src;
		}
	    }
	    XFree ((char*) tmp->cmap_windows);
	    tmp->cmap_windows = wl;
	    tmp->number_cmap_windows++;
	}
    }
    return;
}
