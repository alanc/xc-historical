/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**********************************************************************
 *
 * $XConsortium: icons.c,v 1.15 90/03/08 15:19:52 jim Exp $
 *
 * Icon releated routines
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "icons.h"
#include "gram.h"
#include "parse.h"
#include "util.h"

#define iconWidth(w)	(Scr->IconBorderWidth * 2 + w->icon_w_width)
#define iconHeight(w)	(Scr->IconBorderWidth * 2 + w->icon_w_height)

static
splitEntry (ie, grav1, grav2, w, h)
    IconEntry	*ie;
    int		grav1, grav2;
    int		w, h;
{
    IconEntry	*new;

    switch (grav1) {
    case D_NORTH:
    case D_SOUTH:
	if (w != ie->w)
	    splitEntry (ie, grav2, grav1, w, ie->h);
	if (h != ie->h) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->x = ie->x;
	    new->h = (ie->h - h);
	    new->w = ie->w;
	    ie->h = h;
	    if (grav1 == D_SOUTH) {
		new->y = ie->y;
		ie->y = new->y + new->h;
	    } else
		new->y = ie->y + ie->h;
	}
	break;
    case D_EAST:
    case D_WEST:
	if (h != ie->h)
	    splitEntry (ie, grav2, grav1, ie->w, h);
	if (w != ie->w) {
	    new = (IconEntry *)malloc (sizeof (IconEntry));
	    new->twm_win = 0;
	    new->used = 0;
	    new->next = ie->next;
	    ie->next = new;
	    new->y = ie->y;
	    new->w = (ie->w - w);
	    new->h = ie->h;
	    ie->w = w;
	    if (grav1 == D_EAST) {
		new->x = ie->x;
		ie->x = new->x + new->w;
	    } else
		new->x = ie->x + ie->w;
	}
	break;
    }
}

roundUp (v, multiple)
{
    return ((v + multiple - 1) / multiple) * multiple;
}

PlaceIcon(tmp_win, def_x, def_y, final_x, final_y)
TwmWindow *tmp_win;
int def_x, def_y;
int *final_x, *final_y;
{
    IconRegion	*ir;
    IconEntry	*ie;
    int		w = 0, h = 0;

    ie = 0;
    for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	w = roundUp (iconWidth (tmp_win), ir->stepx);
	h = roundUp (iconHeight (tmp_win), ir->stepy);
	for (ie = ir->entries; ie; ie=ie->next) {
	    if (ie->used)
		continue;
	    if (ie->w >= w && ie->h >= h)
		break;
	}
	if (ie)
	    break;
    }
    if (ie) {
	splitEntry (ie, ir->grav1, ir->grav2, w, h);
	ie->used = 1;
	ie->twm_win = tmp_win;
	*final_x = ie->x + (ie->w - iconWidth (tmp_win)) / 2;
	*final_y = ie->y + (ie->h - iconHeight (tmp_win)) / 2;
    } else {
	*final_x = def_x;
	*final_y = def_y;
    }
    return;
}

static IconEntry *
FindIconEntry (tmp_win, irp)
    TwmWindow   *tmp_win;
    IconRegion	**irp;
{
    IconRegion	*ir;
    IconEntry	*ie;

    for (ir = Scr->FirstRegion; ir; ir = ir->next) {
	for (ie = ir->entries; ie; ie=ie->next)
	    if (ie->twm_win == tmp_win) {
		if (irp)
		    *irp = ir;
		return ie;
	    }
    }
    return 0;
}

IconUp (tmp_win)
    TwmWindow   *tmp_win;
{
    int		x, y;
    int		defx, defy;

    /*
     * If the client specified a particular location, let's use it (this might
     * want to be an option at some point).  Otherwise, try to fit within the
     * icon region.
     */
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & IconPositionHint))
      return;

    defx = -100;
    defy = -100;
    PlaceIcon(tmp_win, defx, defy, &x, &y);
    if (x != defx || y != defy)
	XMoveWindow (dpy, tmp_win->icon_w, x, y);
}

static IconEntry *
prevIconEntry (ie, ir)
    IconEntry	*ie;
    IconRegion	*ir;
{
    IconEntry	*ip;

    if (ie == ir->entries)
	return 0;
    for (ip = ir->entries; ip->next != ie; ip=ip->next)
	;
    return ip;
}

/* old is being freed; and is adjacent to ie.  Merge
 * regions together
 */

static
mergeEntries (old, ie)
    IconEntry	*old, *ie;
{
    if (old->y == ie->y) {
	ie->w = old->w + ie->w;
	if (old->x < ie->x)
	    ie->x = old->x;
    } else {
	ie->h = old->h + ie->h;
	if (old->y < ie->y)
	    ie->y = old->y;
    }
}

IconDown (tmp_win)
    TwmWindow   *tmp_win;
{
    IconEntry	*ie, *ip, *in;
    IconRegion	*ir;

    ie = FindIconEntry (tmp_win, &ir);
    if (ie) {
	ie->twm_win = 0;
	ie->used = 0;
	ip = prevIconEntry (ie, ir);
	in = ie->next;
	for (;;) {
	    if (ip && ip->used == 0 &&
	       ((ip->x == ie->x && ip->w == ie->w) ||
	        (ip->y == ie->y && ip->h == ie->h)))
	    {
	    	ip->next = ie->next;
	    	mergeEntries (ie, ip);
	    	free ((char *) ie);
		ie = ip;
	    	ip = prevIconEntry (ip, ir);
	    } else if (in && in->used == 0 &&
	       ((in->x == ie->x && in->w == ie->w) ||
	        (in->y == ie->y && in->h == ie->h)))
	    {
	    	ie->next = in->next;
	    	mergeEntries (in, ie);
	    	free ((char *) in);
	    	in = ie->next;
	    } else
		break;
	}
    }
}

AddIconRegion(geom, grav1, grav2, stepx, stepy)
char *geom;
int grav1, grav2;
{
    IconRegion *ir;
    int mask;

    ir = (IconRegion *)malloc(sizeof(IconRegion));
    ir->next = NULL;
    if (Scr->LastRegion)
	Scr->LastRegion->next = ir;
    Scr->LastRegion = ir;
    if (!Scr->FirstRegion)
	Scr->FirstRegion = ir;

    ir->entries = NULL;
    ir->grav1 = grav1;
    ir->grav2 = grav2;
    if (stepx <= 0)
	stepx = 1;
    if (stepy <= 0)
	stepy = 1;
    ir->stepx = stepx;
    ir->stepy = stepy;
    ir->x = ir->y = ir->w = ir->h = 0;

    mask = XParseGeometry(geom, &ir->x, &ir->y, (unsigned int *)&ir->w, (unsigned int *)&ir->h);

    if (mask & XNegative)
	ir->x += Scr->MyDisplayWidth - ir->w;

    if (mask & YNegative)
	ir->y += Scr->MyDisplayHeight - ir->h;
    ir->entries = (IconEntry *)malloc(sizeof(IconEntry));
    ir->entries->next = 0;
    ir->entries->x = ir->x;
    ir->entries->y = ir->y;
    ir->entries->w = ir->w;
    ir->entries->h = ir->h;
    ir->entries->twm_win = 0;
    ir->entries->used = 0;
}

#ifdef comment
FreeIconEntries (ir)
    IconRegion	*ir;
{
    IconEntry	*ie, *tmp;

    for (ie = ir->entries; ie; ie=tmp)
    {
	tmp = ie->next;
	free ((char *) ie);
    }
}
FreeIconRegions()
{
    IconRegion *ir, *tmp;

    for (ir = Scr->FirstRegion; ir != NULL;)
    {
	tmp = ir;
	FreeIconEntries (ir);
	ir = ir->next;
	free((char *) tmp);
    }
    Scr->FirstRegion = NULL;
    Scr->LastRegion = NULL;
}
#endif

CreateIconWindow(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{
    unsigned long event_mask;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    Pixmap pm = None;			/* tmp pixmap variable */
    int final_x, final_y;
    int x;


    FB(tmp_win->iconc.fore, tmp_win->iconc.back);

    tmp_win->forced = FALSE;
    tmp_win->icon_not_ours = FALSE;

    /* now go through the steps to get an icon window,  if ForceIcon is 
     * set, then no matter what else is defined, the bitmap from the
     * .twmrc file is used
     */
    if (Scr->ForceIcon)
    {
	char *icon_name;
	Pixmap bm;

	icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if (icon_name == NULL)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
				   &tmp_win->class);

	bm = NULL;
	if (icon_name != NULL)
	{
	    if ((bm = (Pixmap)LookInNameList(Scr->Icons, icon_name)) == None)
	    {
		if ((bm = GetBitmap (icon_name)) != None)
		    AddToList(&Scr->Icons, icon_name, (char *)bm);
	    }
	}

	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		(unsigned int *) &tmp_win->icon_width, (unsigned int *)&tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Scr->Root, tmp_win->icon_width,
		tmp_win->icon_height, Scr->d_depth);

	    /* the copy plane works on color ! */
	    XCopyPlane(dpy, bm, pm, Scr->NormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );

	    tmp_win->forced = TRUE;
	}
    }

    /* if the pixmap is still NULL, we didn't get one from the above code,
     * that could mean that ForceIcon was not set, or that the window
     * was not in the Icons list, now check the WM hints for an icon
     */
    if (pm == None && tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPixmapHint)
    {
    
	XGetGeometry(dpy,   tmp_win->wmhints->icon_pixmap,
             &JunkRoot, &JunkX, &JunkY,
	     (unsigned int *)&tmp_win->icon_width, (unsigned int *)&tmp_win->icon_height, &JunkBW, &JunkDepth);

	pm = XCreatePixmap(dpy, Scr->Root,
			   tmp_win->icon_width, tmp_win->icon_height,
			   Scr->d_depth);

	XCopyPlane(dpy, tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    /* if we still haven't got an icon, let's look in the Icon list 
     * if ForceIcon is not set
     */
    if (pm == None && !Scr->ForceIcon)
    {
	char *icon_name;
	Pixmap bm;

	icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if (icon_name == NULL)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
				   &tmp_win->class);

	bm = NULL;
	if (icon_name != NULL)
	{
	    if ((bm = (Pixmap)LookInNameList(Scr->Icons, icon_name)) == None)
	    {
		if ((bm = GetBitmap (icon_name)) != None)
		    AddToList(&Scr->Icons, icon_name, (char *)bm);
	    }
	}

	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		(unsigned int *)&tmp_win->icon_width, (unsigned int *)&tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Scr->Root, tmp_win->icon_width,
		tmp_win->icon_height, Scr->d_depth);

	    /* the copy plane works on color ! */
	    XCopyPlane(dpy, bm, pm, Scr->NormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
	}
    }

    /* if we still don't have an icon, assign the UnknownIcon */

    if (pm == None && Scr->UnknownPm != NULL)
    {
	tmp_win->icon_width = Scr->UnknownWidth;
	tmp_win->icon_height = Scr->UnknownHeight;

	pm = XCreatePixmap(dpy, Scr->Root, tmp_win->icon_width,
	    tmp_win->icon_height, Scr->d_depth);

	/* the copy plane works on color ! */
	XCopyPlane(dpy, Scr->UnknownPm, pm, Scr->NormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    if (pm == None)
    {
	tmp_win->icon_height = 0;
	tmp_win->icon_width = 0;
	valuemask = 0;
    }
    else
    {
	valuemask = CWBackPixmap;
	attributes.background_pixmap = pm;
    }

    tmp_win->icon_w_width = XTextWidth(Scr->IconFont.font,
	tmp_win->icon_name, strlen(tmp_win->icon_name));

    tmp_win->icon_w_width += 6;
    if (tmp_win->icon_w_width < tmp_win->icon_width)
    {
	tmp_win->icon_x = (tmp_win->icon_width - tmp_win->icon_w_width)/2;
	tmp_win->icon_x += 3;
	tmp_win->icon_w_width = tmp_win->icon_width;
    }
    else
    {
	tmp_win->icon_x = 3;
    }
    tmp_win->icon_y = tmp_win->icon_height + Scr->IconFont.height;
    tmp_win->icon_w_height = tmp_win->icon_height + Scr->IconFont.height + 4;

    event_mask = 0;
    if (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint)
    {
	tmp_win->icon_w = tmp_win->wmhints->icon_window;
	if (tmp_win->forced ||
	    XGetGeometry(dpy, tmp_win->icon_w, &JunkRoot, &JunkX, &JunkY,
		     (unsigned int *)&tmp_win->icon_w_width, (unsigned int *)&tmp_win->icon_w_height,
		     &JunkBW, &JunkDepth) == 0)
	{
	    tmp_win->icon_w = NULL;
	    tmp_win->wmhints->flags &= ~IconWindowHint;
	}
	else
	{
	    tmp_win->icon_not_ours = TRUE;
	}
    }
    else
    {
	tmp_win->icon_w = NULL;
    }

    if (tmp_win->icon_w == NULL)
    {
	tmp_win->icon_w = XCreateSimpleWindow(dpy, Scr->Root,
	    0,0,
	    tmp_win->icon_w_width, tmp_win->icon_w_height,
	    Scr->IconBorderWidth, tmp_win->icon_border, tmp_win->iconc.back);
	event_mask = ExposureMask;
    }

    XSelectInput (dpy, tmp_win->icon_w,
		  KeyPressMask | ButtonPressMask | ButtonReleaseMask |
		  event_mask);

    tmp_win->icon_bm_w = NULL;
    if (pm != None &&
	(! (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint)))
    {
	int y;

	y = 0;
	if (tmp_win->icon_w_width == tmp_win->icon_width)
	    x = 0;
	else
	    x = (tmp_win->icon_w_width - tmp_win->icon_width)/2;

	tmp_win->icon_bm_w = XCreateWindow(dpy, tmp_win->icon_w,
	    x, y, tmp_win->icon_width, tmp_win->icon_height,
	    0, Scr->d_depth, CopyFromParent,
	    Scr->d_visual, valuemask, &attributes);
    }

    /* I need to figure out where to put the icon window now, because 
     * getting here means that I am going to make the icon visible
     */
    if (tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPositionHint)
    {
	final_x = tmp_win->wmhints->icon_x;
	final_y = tmp_win->wmhints->icon_y;
    }
    else
    {
	PlaceIcon(tmp_win, def_x, def_y, &final_x, &final_y);
    }

    if (final_x > Scr->MyDisplayWidth)
	final_x = Scr->MyDisplayWidth - tmp_win->icon_w_width -
	    (2 * Scr->IconBorderWidth);

    if (final_y > Scr->MyDisplayHeight)
	final_y = Scr->MyDisplayHeight - tmp_win->icon_height -
	    Scr->IconFont.height - 4 - (2 * Scr->IconBorderWidth);

    XMoveWindow(dpy, tmp_win->icon_w, final_x, final_y);
    tmp_win->iconified = TRUE;

    XMapSubwindows(dpy, tmp_win->icon_w);
    XSaveContext(dpy, tmp_win->icon_w, TwmContext, (caddr_t)tmp_win);
    XSaveContext(dpy, tmp_win->icon_w, ScreenContext, (caddr_t)Scr);
    XDefineCursor(dpy, tmp_win->icon_w, Scr->IconCursor);
    XFreePixmap (dpy, pm);
    return;
}
