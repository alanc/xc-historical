/**********************************************************************
 *
 * $XConsortium: icons.c,v 1.3 89/04/12 18:55:47 jim Exp $
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

PlaceIcon(tmp_win, def_x, def_y, final_x, final_y)
TwmWindow *tmp_win;
int def_x, def_y;
int *final_x, *final_y;
{
    IconRegion *ir;

    /* start with the default position */
    *final_x = def_x;
    *final_y = def_y;
    return;

}

AddIconRegion(geom, grav1, grav2)
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

    ir->first = NULL;
    ir->last = NULL;
    ir->grav1 = grav1;
    ir->grav2 = grav2;
    ir->x = ir->y = ir->w = ir->h = 0;

    mask = XParseGeometry(geom, &ir->x, &ir->y, &ir->w, &ir->h);

    if (mask & XNegative)
	ir->x = Scr->MyDisplayWidth - ir->w;

    if (mask & YNegative)
	ir->y = Scr->MyDisplayHeight - ir->h;

}

FreeIconRegions()
{
    IconRegion *ir, *tmp;

    for (ir = Scr->FirstRegion; ir != NULL;)
    {
	tmp = ir;
	ir = ir->next;
	free(tmp);
    }
    Scr->FirstRegion = NULL;
    Scr->LastRegion = NULL;
}

CreateIconWindow(tmp_win, def_x, def_y)
TwmWindow *tmp_win;
int def_x, def_y;
{
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    Pixmap pm;			/* tmp pixmap variable */
    int final_x, final_y;
    int x;


    FB(tmp_win->iconc.fore, tmp_win->iconc.back);

    pm = NULL;
    tmp_win->forced = FALSE;

    /* now go through the steps to get an icon window,  if ForceIcon is 
     * set, then no matter what else is defined, the bitmap from the
     * .twmrc file is used
     */
    if (Scr->ForceIcon)
    {
	char *icon_name;
	Pixmap bm;
	XImage *image;
	unsigned mask;

	icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if (icon_name == NULL)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
				   &tmp_win->class);

	bm = NULL;
	if (icon_name != NULL)
	{
	    if ((bm = (Pixmap)LookInNameList(Scr->Icons, icon_name)) == NULL)
	    {
		if ((bm = GetBitmap (icon_name, Scr->Root)) != NULL)
		    AddToList(&Scr->Icons, icon_name, (char *)bm);
	    }
	}

	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		&tmp_win->icon_width, &tmp_win->icon_height,
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
    if (pm == NULL && tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPixmapHint)
    {
    
	XGetGeometry(dpy,   tmp_win->wmhints->icon_pixmap,
             &JunkRoot, &JunkX, &JunkY,
	     &tmp_win->icon_width, &tmp_win->icon_height, &JunkBW, &JunkDepth);

	pm = XCreatePixmap(dpy, Scr->Root,
			   tmp_win->icon_width, tmp_win->icon_height,
			   Scr->d_depth);

	XCopyPlane(dpy, tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    /* if we still haven't got an icon, let's look in the Icon list 
     * if ForceIcon is not set
     */
    if (pm == NULL && !Scr->ForceIcon)
    {
	char *icon_name;
	Pixmap bm;
	XImage *image;
	unsigned mask;

	icon_name = LookInNameList(Scr->IconNames, tmp_win->full_name);
        if (icon_name == NULL)
	    icon_name = LookInList(Scr->IconNames, tmp_win->full_name,
				   &tmp_win->class);

	bm = NULL;
	if (icon_name != NULL)
	{
	    if ((bm = (Pixmap)LookInNameList(Scr->Icons, icon_name)) == NULL)
	    {
		if ((bm = GetBitmap (icon_name, Scr->Root)) != NULL)
		    AddToList(&Scr->Icons, icon_name, (char *)bm);
	    }
	}

	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		&tmp_win->icon_width, &tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Scr->Root, tmp_win->icon_width,
		tmp_win->icon_height, Scr->d_depth);

	    /* the copy plane works on color ! */
	    XCopyPlane(dpy, bm, pm, Scr->NormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
	}
    }

    /* if we still don't have an icon, assign the UnknownIcon */

    if (pm == NULL && Scr->UnknownPm != NULL)
    {
	tmp_win->icon_width = Scr->UnknownWidth;
	tmp_win->icon_height = Scr->UnknownHeight;

	pm = XCreatePixmap(dpy, Scr->Root, tmp_win->icon_width,
	    tmp_win->icon_height, Scr->d_depth);

	/* the copy plane works on color ! */
	XCopyPlane(dpy, Scr->UnknownPm, pm, Scr->NormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    if (pm == NULL)
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

    if (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint)
    {
	tmp_win->icon_w = tmp_win->wmhints->icon_window;
	if (tmp_win->forced ||
	    XGetGeometry(dpy, tmp_win->icon_w, &JunkRoot, &JunkX, &JunkY,
		     &tmp_win->icon_w_width, &tmp_win->icon_w_height,
		     &JunkBW, &JunkDepth) == 0)
	{
	    tmp_win->icon_w = NULL;
	    tmp_win->wmhints->flags &= ~IconWindowHint;
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
    }

    XSelectInput(dpy, tmp_win->icon_w,
	KeyPressMask | VisibilityChangeMask |
	ButtonPressMask | ButtonReleaseMask | ExposureMask);

    tmp_win->icon_bm_w = NULL;
    if (pm != NULL &&
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
    XSaveContext(dpy, tmp_win->icon_w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->icon_w, ScreenContext, Scr);
    XDefineCursor(dpy, tmp_win->icon_w, Scr->IconCursor);
}

