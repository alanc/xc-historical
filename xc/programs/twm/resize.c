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
 * $XConsortium: resize.c,v 1.48 89/11/05 17:47:13 jim Exp $
 *
 * window resizing borrowed from the "wm" window manager
 *
 * 11-Dec-87 Thomas E. LaStrange                File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: resize.c,v 1.48 89/11/05 17:47:13 jim Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "parse.h"
#include "util.h"
#include "resize.h"
#include "add_window.h"
#include "screen.h"

#define MINHEIGHT 0     /* had been 32 */
#define MINWIDTH 0      /* had been 60 */

static int dragx;       /* all these variables are used */
static int dragy;       /* in resize operations */
static int dragWidth;
static int dragHeight;

static int origx;
static int origy;
static int origWidth;
static int origHeight;

static int clampTop;
static int clampBottom;
static int clampLeft;
static int clampRight;
static int clampDX;
static int clampDY;

static int last_width;
static int last_height;


static void do_auto_clamp (tmp_win, evp)
    TwmWindow *tmp_win;
    XEvent *evp;
{
    Window junkRoot;
    int x, y, h, v, junkbw;
    unsigned int junkMask;

    switch (evp->type) {
      case ButtonPress:
	x = evp->xbutton.x_root;
	y = evp->xbutton.y_root;
	break;
      case KeyPress:
	x = evp->xkey.x_root;
	y = evp->xkey.y_root;
	break;
      default:
	if (!XQueryPointer (dpy, Scr->Root, &junkRoot, &junkRoot,
			    &x, &y, &junkbw, &junkbw, &junkMask))
	  return;
    }

    h = ((x - dragx) / (dragWidth < 3 ? 1 : (dragWidth / 3)));
    v = ((y - dragy - tmp_win->title_height) / 
	 (dragHeight < 3 ? 1 : (dragHeight / 3)));
	
    if (h <= 0) {
	clampLeft = 1;
	clampDX = (x - dragx);
    } else if (h >= 2) {
	clampRight = 1;
	clampDX = (x - dragx - dragWidth);
    }

    if (v <= 0) {
	clampTop = 1;
	clampDY = (y - dragy);
    } else if (v >= 2) {
	clampBottom = 1;
	clampDY = (y - dragy - dragHeight);
    }
}


/***********************************************************************
 *
 *  Procedure:
 *      StartResize - begin a window resize operation
 *
 *  Inputs:
 *      ev      - the event structure (button press)
 *      tmp_win - the TwmWindow pointer
 *      fromtitlebar - action invoked from titlebar button
 *
 ***********************************************************************
 */

void
StartResize(evp, tmp_win, fromtitlebar)
XEvent *evp;
TwmWindow *tmp_win;
Bool fromtitlebar;
{
    Window      junkRoot;
    int         junkbw, junkDepth;

    ResizeWindow = tmp_win->frame;
    XGrabServer(dpy);
    XGrabPointer(dpy, Scr->Root, True,
        ButtonPressMask | ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync,
        Scr->Root, Scr->ResizeCursor, CurrentTime);

    XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
        &dragx, &dragy, &dragWidth, &dragHeight, &junkbw,
                 &junkDepth);
    dragx += tmp_win->frame_bw;
    dragy += tmp_win->frame_bw;
    origx = dragx;
    origy = dragy;
    origWidth = dragWidth;
    origHeight = dragHeight;
    clampTop = clampBottom = clampLeft = clampRight = clampDX = clampDY = 0;

    if (Scr->AutoRelativeResize && !fromtitlebar)
      do_auto_clamp (tmp_win, evp);

    Scr->SizeStringOffset = SIZE_HINDENT;
    XResizeWindow (dpy, Scr->SizeWindow,
		   Scr->SizeStringWidth + SIZE_HINDENT * 2, 
		   Scr->SizeFont.height + SIZE_VINDENT * 2);
    XMapRaised(dpy, Scr->SizeWindow);
    last_width = 0;
    last_height = 0;
    DisplaySize(tmp_win, origWidth, origHeight);
    MoveOutline (Scr->Root, dragx - tmp_win->frame_bw,
		 dragy - tmp_win->frame_bw, dragWidth + 2 * tmp_win->frame_bw,
		 dragHeight + 2 * tmp_win->frame_bw,
		 tmp_win->frame_bw, tmp_win->title_height);
}

/***********************************************************************
 *
 *  Procedure:
 *      AddStartResize - begin a window resize operation from AddWindow
 *
 *  Inputs:
 *      tmp_win - the TwmWindow pointer
 *
 ***********************************************************************
 */

void
AddStartResize(tmp_win, x, y, w, h)
TwmWindow *tmp_win;
int x, y, w, h;
{
    Window      junkRoot;
    int         junkbw, junkDepth;

    XGrabServer(dpy);
    XGrabPointer(dpy, Scr->Root, True,
        ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync,
        Scr->Root, Scr->ResizeCursor, CurrentTime);

    dragx = x + tmp_win->frame_bw;
    dragy = y + tmp_win->frame_bw;
    origx = dragx;
    origy = dragy;
    dragWidth = origWidth = w - 2 * (tmp_win->bw + tmp_win->frame_bw);
    dragHeight = origHeight = h - 2 * (tmp_win->bw + tmp_win->frame_bw);
    clampTop = clampBottom = clampLeft = clampRight = clampDX = clampDY = 0;

    if (Scr->AutoRelativeResize) {
	clampRight = clampBottom = 1;
    }

    last_width = 0;
    last_height = 0;
    DisplaySize(tmp_win, origWidth, origHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *      DoResize - move the rubberband around.  This is called for
 *                 each motion event when we are resizing
 *
 *  Inputs:
 *      x_root  - the X corrdinate in the root window
 *      y_root  - the Y corrdinate in the root window
 *      tmp_win - the current twm window
 *
 ***********************************************************************
 */

void
DoResize(x_root, y_root, tmp_win)
int x_root;
int y_root;
TwmWindow *tmp_win;
{
    int action;

    action = 0;

    x_root -= clampDX;
    y_root -= clampDY;

    if (clampTop) {
        int         delta = y_root - dragy;
        if (dragHeight - delta < MINHEIGHT) {
            delta = dragHeight - MINHEIGHT;
            clampTop = 0;
        }
        dragy += delta;
        dragHeight -= delta;
        action = 1;
    }
    else if (y_root <= dragy/* ||
             y_root == findRootInfo(root)->rooty*/) {
        dragy = y_root;
        dragHeight = origy + origHeight -
            y_root;
        clampBottom = 0;
        clampTop = 1;
	clampDY = 0;
        action = 1;
    }
    if (clampLeft) {
        int         delta = x_root - dragx;
        if (dragWidth - delta < MINWIDTH) {
            delta = dragWidth - MINWIDTH;
            clampLeft = 0;
        }
        dragx += delta;
        dragWidth -= delta;
        action = 1;
    }
    else if (x_root <= dragx/* ||
             x_root == findRootInfo(root)->rootx*/) {
        dragx = x_root;
        dragWidth = origx + origWidth -
            x_root;
        clampRight = 0;
        clampLeft = 1;
	clampDX = 0;
        action = 1;
    }
    if (clampBottom) {
        int         delta = y_root - dragy - dragHeight;
        if (dragHeight + delta < MINHEIGHT) {
            delta = MINHEIGHT - dragHeight;
            clampBottom = 0;
        }
        dragHeight += delta;
        action = 1;
    }
    else if (y_root >= dragy + dragHeight - 1/* ||
           y_root == findRootInfo(root)->rooty
           + findRootInfo(root)->rootheight - 1*/) {
        dragy = origy;
        dragHeight = 1 + y_root - dragy;
        clampTop = 0;
        clampBottom = 1;
	clampDY = 0;
        action = 1;
    }
    if (clampRight) {
        int         delta = x_root - dragx - dragWidth;
        if (dragWidth + delta < MINWIDTH) {
            delta = MINWIDTH - dragWidth;
            clampRight = 0;
        }
        dragWidth += delta;
        action = 1;
    }
    else if (x_root >= dragx + dragWidth - 1/* ||
             x_root == findRootInfo(root)->rootx +
             findRootInfo(root)->rootwidth - 1*/) {
        dragx = origx;
        dragWidth = 1 + x_root - origx;
        clampLeft = 0;
        clampRight = 1;
	clampDX = 0;
        action = 1;
    }

    if (action) {
        ConstrainSize (tmp_win, &dragWidth, &dragHeight);
        if (clampLeft)
            dragx = origx + origWidth - dragWidth;
        if (clampTop)
            dragy = origy + origHeight - dragHeight;
        MoveOutline(Scr->Root,
            dragx - tmp_win->frame_bw,
            dragy - tmp_win->frame_bw,
            dragWidth + 2 * tmp_win->frame_bw,
            dragHeight + 2 * tmp_win->frame_bw,
	    tmp_win->frame_bw, tmp_win->title_height);
    }

    DisplaySize(tmp_win, dragWidth, dragHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *      DisplaySize - display the size in the dimensions window
 *
 *  Inputs:
 *      tmp_win - the current twm window
 *      width   - the width of the rubber band
 *      height  - the height of the rubber band
 *
 ***********************************************************************
 */

void
DisplaySize(tmp_win, width, height)
TwmWindow *tmp_win;
int width;
int height;
{
    char str[100];
    int dwidth;
    int dheight;

    if (last_width == width && last_height == height)
        return;

    last_width = width;
    last_height = height;

    dheight = height - tmp_win->title_height - 2*tmp_win->bw;
    dwidth = width - 2*tmp_win->bw;

    /*
     * ICCCM says that PMinSize is the default is no PBaseSize is given,
     * and vice-versa.
     */
    if (tmp_win->hints.flags&(PMinSize|PBaseSize) && tmp_win->hints.flags & PResizeInc)
    {
	if (tmp_win->hints.flags & PBaseSize) {
	    dwidth -= tmp_win->hints.base_width;
	    dheight -= tmp_win->hints.base_height;
	} else {
	    dwidth -= tmp_win->hints.min_width;
	    dheight -= tmp_win->hints.min_height;
	}
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
        dwidth /= tmp_win->hints.width_inc;
        dheight /= tmp_win->hints.height_inc;
    }

    sprintf(str, " %4d x %-4d ", dwidth, dheight);
    XRaiseWindow(dpy, Scr->SizeWindow);
    FBF(Scr->DefaultC.fore, Scr->DefaultC.back, Scr->SizeFont.font->fid);
    XDrawImageString (dpy, Scr->SizeWindow, Scr->NormalGC,
		      Scr->SizeStringOffset,
		      Scr->SizeFont.font->ascent + SIZE_VINDENT,
		      str, 13);
}

/***********************************************************************
 *
 *  Procedure:
 *      EndResize - finish the resize operation
 *
 ***********************************************************************
 */

void
EndResize()
{
    TwmWindow *tmp_win;
    Window w;

#ifdef DEBUG
    fprintf(stderr, "EndResize\n");
#endif

    XUnmapWindow(dpy, Scr->SizeWindow);
    MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);

    XFindContext(dpy, ResizeWindow, TwmContext, &tmp_win);

    ConstrainSize (tmp_win, &dragWidth, &dragHeight);

    if (dragWidth != tmp_win->frame_width ||
        dragHeight != tmp_win->frame_height)
            tmp_win->zoomed = ZOOM_NONE;

    SetupWindow(tmp_win,
        dragx - tmp_win->frame_bw,
        dragy - tmp_win->frame_bw,
        dragWidth, dragHeight);

    if (tmp_win->iconmgr)
    {
        PackIconManager(tmp_win->iconmgrp);
    }
#ifdef SUN386
    /* This is a kludge to fix a problem in the Sun 386 server which
     * causes windows to not be repainted after a resize operation.
     */
    w = XCreateSimpleWindow(dpy, tmp_win->frame,
        0, 0, 9999, 9999, 0, Scr->Black, Scr->Black);

    XMapWindow(dpy, w);
    XDestroyWindow(dpy, w);
    XFlush(dpy);
#endif

    if (!Scr->NoRaiseResize)
        XRaiseWindow(dpy, tmp_win->frame);

    ResizeWindow = NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *      AddEndResize - finish the resize operation for AddWindow
 *
 ***********************************************************************
 */

void
AddEndResize(tmp_win)
TwmWindow *tmp_win;
{

#ifdef DEBUG
    fprintf(stderr, "AddEndResize\n");
#endif

    ConstrainSize (tmp_win, &dragWidth, &dragHeight);
    AddingX = dragx;
    AddingY = dragy;
    AddingW = dragWidth + (2 * (tmp_win->bw + tmp_win->frame_bw));
    AddingH = dragHeight + (2 * (tmp_win->bw + tmp_win->frame_bw));
}

/***********************************************************************
 *
 *  Procedure:
 *      ConstrainSize - adjust the given width and height to account for the
 *              constraints imposed by size hints
 *
 *      The general algorithm, especially the aspect ratio stuff, is
 *      borrowed from uwm's CheckConsistency routine.
 * 
 ***********************************************************************/

ConstrainSize (tmp_win, widthp, heightp)
    TwmWindow *tmp_win;
    int *widthp, *heightp;
{
#define MAXSIZE 32767
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )

    int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
    int baseWidth, baseHeight;
    int dwidth = *widthp, dheight = *heightp;


    dheight -= tmp_win->title_height + 2*tmp_win->bw;
    dwidth -= 2*tmp_win->bw;

    if (tmp_win->hints.flags & PMinSize) {
        minWidth = tmp_win->hints.min_width;
        minHeight = tmp_win->hints.min_height;
    } else if (tmp_win->hints.flags & PBaseSize) {
        minWidth = tmp_win->hints.base_width;
        minHeight = tmp_win->hints.base_height;
    } else
        minWidth = minHeight = 1;

    if (tmp_win->hints.flags & PBaseSize) {
	baseWidth = tmp_win->hints.base_width;
	baseHeight = tmp_win->hints.base_height;
    } else if (tmp_win->hints.flags & PMinSize) {
	baseWidth = tmp_win->hints.min_width;
	baseHeight = tmp_win->hints.min_height;
    } else
	baseWidth = baseHeight = 0;


    if (tmp_win->hints.flags & PMaxSize) {
        maxWidth = tmp_win->hints.max_width;
        maxHeight = tmp_win->hints.max_height;
    } else
        maxWidth = maxHeight = MAXSIZE;

    if (tmp_win->hints.flags & PResizeInc) {
        xinc = tmp_win->hints.width_inc;
        yinc = tmp_win->hints.height_inc;
    } else
        xinc = yinc = 1;

    /*
     * First, clamp to min and max values
     */
    if (dwidth < minWidth) dwidth = minWidth;
    if (dheight < minHeight) dheight = minHeight;

    if (dwidth > maxWidth) dwidth = maxWidth;
    if (dheight > maxHeight) dheight = maxHeight;


    /*
     * Second, fit to base + N * inc
     */
    dwidth = ((dwidth - baseWidth) / xinc * xinc) + baseWidth;
    dheight = ((dheight - baseHeight) / yinc * yinc) + baseHeight;


    /*
     * Third, adjust for aspect ratio
     */
#define maxAspectX tmp_win->hints.max_aspect.x
#define maxAspectY tmp_win->hints.max_aspect.y
#define minAspectX tmp_win->hints.min_aspect.x
#define minAspectY tmp_win->hints.min_aspect.y
    if (tmp_win->hints.flags & PAspect)
    {
        if (dwidth * maxAspectX > dheight * maxAspectY)
        {
            delta = makemult(dwidth * maxAspectY / maxAspectX - dheight,
                             yinc);
            if (dheight + delta <= maxHeight) dheight += delta;
            else
            {
                delta = makemult(dwidth - maxAspectX*dheight/maxAspectY,
                                 xinc);
                if (dwidth - delta >= minWidth) dwidth -= delta;
            }
        }

        if (dwidth * minAspectX < dheight * minAspectY)
        {
            delta = makemult(minAspectX * dheight / minAspectY - dwidth,
                             xinc);
            if (dwidth + delta <= maxWidth) dwidth += delta;
            else
            {
                delta = makemult(dheight - dwidth*minAspectY/minAspectX,
                                 yinc);
                if (dheight - delta >= minHeight) dheight -= delta;
            }
        }
    }


    /*
     * Fourth, account for border width and title height
     */
    *widthp = dwidth + 2*tmp_win->bw;
    *heightp = dheight + tmp_win->title_height + 2*tmp_win->bw;
}


/***********************************************************************
 *
 *  Procedure:
 *      SetupWindow - set window sizes, this was called from either
 *              AddWindow, EndResize, or HandleConfigureNotify.
 *
 *  Inputs:
 *      tmp_win - the TwmWindow pointer
 *      x       - the x coordinate of the frame window
 *      y       - the y coordinate of the frame window
 *      w       - the width of the frame window
 *      h       - the height of the frame window
 *
 *  Special Considerations:
 *      This routine will check to make sure the window is not completely
 *      off the display, if it is, it'll bring some of it back on.
 *
 *      The tmp_win->frame_XXX variables should NOT be updated with the
 *      values of x,y,w,h prior to calling this routine, since the new
 *      values are compared against the old to see whether a synthetic
 *      ConfigureNotify event should be sent.  (It should be sent if the
 *      window was moved but not resized.)
 *
 ***********************************************************************
 */

void
SetupWindow(tmp_win, x, y, w, h)
TwmWindow *tmp_win;
int x, y, w, h;
{
    XEvent client_event;
    XWindowChanges xwc;
    unsigned int   xwcm;
    int title_width;
    int sendEvent;
#ifdef SHAPE
    int reShape;
#endif

#ifdef DEBUG
    fprintf(stderr, "SetupWindow: x=%d, y=%d, w=%d, h=%d\n",
        x, y, w, h);
#endif

    if (x > Scr->MyDisplayWidth)
        x = Scr->MyDisplayWidth - 64;
    if (y > Scr->MyDisplayHeight)
        y = Scr->MyDisplayHeight - 64;

    if (tmp_win->iconmgr)
    {
        tmp_win->iconmgrp->width = w - 2*tmp_win->bw;
        h = tmp_win->iconmgrp->height + tmp_win->title_height;
    }

    /*
     * According to the July 27, 1988 ICCCM draft, we should send a
     * "synthetic" ConfigureNotify event to the client if the window
     * was moved but not resized.
     */
    if ((x != tmp_win->frame_x || y != tmp_win->frame_y) &&
        (w == tmp_win->frame_width && h == tmp_win->frame_height))
        sendEvent = TRUE;
    else
        sendEvent = FALSE;

    xwcm = CWWidth;

    if (tmp_win->title_x < 0)
        xwc.width = w;
    else
	xwc.width = w - 2 * tmp_win->title_bw;
    title_width = xwc.width;

#ifdef SHAPE
    if (!HasShape)
	Scr->SqueezeTitle = FALSE;
#endif

    ComputeWindowTitleOffsets (tmp_win, xwc.width, True);

#ifdef SHAPE
    reShape = FALSE;
    if (tmp_win->wShaped && w != tmp_win->frame_width)
	reShape = TRUE;
    if (Scr->SqueezeTitle)
    {
	title_width = tmp_win->rightx + Scr->TBInfo.rightoff;
	if (title_width < xwc.width)
	{
	    xwc.width = title_width;
	    if (tmp_win->frame_height != h ||
	    	tmp_win->frame_width != w ||
	    	title_width != tmp_win->title_width)
	    	reShape = TRUE;
	}
	else
	{
	    if (!tmp_win->wShaped && tmp_win->fShaped)
		reShape = TRUE;
	    title_width = xwc.width;
	}
    }
#endif

    tmp_win->title_width = title_width;
    if (tmp_win->title_w)
	XConfigureWindow(dpy, tmp_win->title_w, xwcm, &xwc);

    tmp_win->attr.width = w;
    tmp_win->attr.height = h - tmp_win->title_height;

    XMoveResizeWindow(dpy, tmp_win->w, 0, tmp_win->title_height,
        w - 2 * tmp_win->bw,
        h - tmp_win->title_height - 2 * tmp_win->bw);

    tmp_win->frame_x = x;
    tmp_win->frame_y = y;
    tmp_win->frame_width = w;
    tmp_win->frame_height = h;

    XMoveResizeWindow(dpy, tmp_win->frame, x, y, w, h);

    /*
     * fix up highlight window
     */
    if (tmp_win->title_height && tmp_win->hilite_w)
    {
	xwc.width = (tmp_win->rightx - tmp_win->highlightx);
	if (Scr->TBInfo.nright > 0) xwc.width -= Scr->TitlePadding;
        if (xwc.width <= 0) {
            xwc.x = Scr->MyDisplayWidth;	/* move offscreen */
            xwc.width = 1;
        } else {
            xwc.x = tmp_win->highlightx;
        }

        xwcm = CWX | CWWidth;
        XConfigureWindow(dpy, tmp_win->hilite_w, xwcm, &xwc);
    }

#ifdef SHAPE
    if (HasShape && (reShape || tmp_win->fShaped == -1)) {
	tmp_win->fShaped = 0;
	SetFrameShape (tmp_win);
    }
#endif

    if (sendEvent)
    {
        client_event.type = ConfigureNotify;
        client_event.xconfigure.display = dpy;
        client_event.xconfigure.event = tmp_win->w;
        client_event.xconfigure.window = tmp_win->w;
        client_event.xconfigure.x = x;
        client_event.xconfigure.y = y + tmp_win->title_height;
        client_event.xconfigure.width = tmp_win->frame_width - 2 * tmp_win->bw;
        client_event.xconfigure.height = tmp_win->frame_height -
                tmp_win->title_height - 2 * tmp_win->bw;
        client_event.xconfigure.border_width = tmp_win->bw;
        /* Real ConfigureNotify events say we're above title window, so ... */
	/* what if we don't have a title ????? */
        client_event.xconfigure.above = tmp_win->frame;
        client_event.xconfigure.override_redirect = False;
        XSendEvent(dpy, tmp_win->w, False, StructureNotifyMask, &client_event);
    }
}


/**********************************************************************
 *  Rutgers mod #1   - rocky.
 *  Procedure:
 *         fullzoom - zooms window to full height of screen or
 *                    to full height and width of screen. (Toggles
 *                    so that it can undo the zoom - even when switching
 *                    between fullzoom and vertical zoom.)
 *
 *  Inputs:
 *         tmp_win - the TwmWindow pointer
 *
 *
 **********************************************************************
 */

void
fullzoom(tmp_win,flag)
TwmWindow *tmp_win;
int flag;
{
    Window      junkRoot;
    int         junkbw, junkDepth;
    TwmWindow   *test_win;

    XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
        &dragx, &dragy, &dragWidth, &dragHeight, &junkbw,
        &junkDepth);

        if (tmp_win->zoomed == flag)
        {
            dragHeight = tmp_win->save_frame_height;
            dragWidth = tmp_win->save_frame_width;
            dragx = tmp_win->save_frame_x;
            dragy = tmp_win->save_frame_y;
            tmp_win->zoomed = ZOOM_NONE;
        }
        else
        {
                if (tmp_win->zoomed == ZOOM_NONE)
                {
                        tmp_win->save_frame_x = dragx;
                        tmp_win->save_frame_y = dragy;
                        tmp_win->save_frame_width = dragWidth;
                        tmp_win->save_frame_height = dragHeight;
                        tmp_win->zoomed = flag;
                 }
                  else
                            tmp_win->zoomed = flag;


        switch (flag)
        {
        case ZOOM_NONE:
            break;
        case F_ZOOM:
            dragHeight = Scr->MyDisplayHeight;
            dragy=0;
            break;
        case F_HORIZOOM:
            dragx = 0;
            dragWidth = Scr->MyDisplayWidth;
            break;
        case F_FULLZOOM:
            dragx = 0;
            dragy = 0;
            dragHeight = Scr->MyDisplayHeight;
            dragWidth = Scr->MyDisplayWidth;
            break;
        case F_LEFTZOOM:
            dragx = 0;
            dragy = 0;
            dragHeight = Scr->MyDisplayHeight;
            dragWidth = Scr->MyDisplayWidth/2;
            break;
        case F_RIGHTZOOM:
            dragx = Scr->MyDisplayWidth/2;
            dragy = 0;
            dragHeight = Scr->MyDisplayHeight;
            dragWidth = Scr->MyDisplayWidth/2;
            break;
        case F_TOPZOOM:
            dragx = 0;
            dragy = 0;
            dragHeight = Scr->MyDisplayHeight/2;
            dragWidth = Scr->MyDisplayWidth;
            break;
        case F_BOTTOMZOOM:
            dragx = 0;
            dragy = Scr->MyDisplayHeight/2;
            dragHeight = Scr->MyDisplayHeight/2;
            dragWidth = Scr->MyDisplayWidth;
            break;
         }
      }

    if (!Scr->NoRaiseResize)
        XRaiseWindow(dpy, tmp_win->frame);

    dragHeight -= tmp_win->title_height;

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
        dragWidth -= tmp_win->hints.min_width;
        dragHeight -= tmp_win->hints.min_height;
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
        dragWidth /= tmp_win->hints.width_inc;
        dragHeight /= tmp_win->hints.height_inc;

        dragWidth *= tmp_win->hints.width_inc;
        dragHeight *= tmp_win->hints.height_inc;
    }

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
        dragWidth += tmp_win->hints.min_width;
        dragHeight += tmp_win->hints.min_height;
    }

    dragHeight += tmp_win->title_height;

    SetupWindow(tmp_win, dragx , dragy , dragWidth, dragHeight);
    XUngrabPointer(dpy, CurrentTime);
    XUngrabServer(dpy);
}

#ifdef SHAPE
SetFrameShape (tmp_win)
TwmWindow   *tmp_win;
{
    Window  dest = tmp_win->frame;
    int	    op;
    int	    expect_title_width;

    if (tmp_win->wShaped) {
	op = ShapeSet;
	if (tmp_win->title_height) {
	    XShapeCombineShape (dpy, dest, ShapeBounding,
			    	tmp_win->title_x + tmp_win->title_bw,
			    	tmp_win->title_y + tmp_win->title_bw,
			    	tmp_win->title_w, ShapeBounding,
			    	ShapeSet);
	    op = ShapeUnion;
	}
	XShapeCombineShape (dpy, dest, ShapeBounding,
			    0,
			    tmp_win->title_height,
			    tmp_win->w, ShapeBounding,
			    op);
	if (tmp_win->fShaped == 2)
	    XShapeCombineMask (dpy, dest, ShapeClip, 0, 0,
			       None, ShapeSet);
	tmp_win->fShaped = 1;
    } else {
	expect_title_width = tmp_win->frame_width;
	if (tmp_win->title_x >= 0)
	    expect_title_width -= 2 * tmp_win->title_bw;
	if (Scr->SqueezeTitle && tmp_win->title_width != expect_title_width)
	{
	    XRectangle  newBounding[2];
	    XRectangle  newClip[2];
    
	    newBounding[0].x = -tmp_win->frame_bw;
	    newBounding[0].y = -tmp_win->frame_bw;
	    newBounding[0].width = tmp_win->title_width + tmp_win->frame_bw + tmp_win->title_bw;
	    newBounding[0].height = tmp_win->title_height;
	    newBounding[1].x = newBounding[0].x;
	    newBounding[1].y = newBounding[0].y + newBounding[0].height;
	    newBounding[1].width = tmp_win->frame_bw * 2 + tmp_win->frame_width;
	    newBounding[1].height = tmp_win->frame_height - newBounding[1].y + tmp_win->frame_bw;
	    XShapeCombineRectangles (dpy, dest, ShapeBounding, 0, 0,
				     newBounding, 2, ShapeSet, YXBanded);
	    newClip[0].x = 0;
	    newClip[0].y = 0;
	    newClip[0].width = tmp_win->title_width;
	    newClip[0].height = tmp_win->title_height;
	    newClip[1].x = 0;
	    newClip[1].y = newClip[0].y + newClip[0].height;
	    newClip[1].width = tmp_win->frame_width;
	    newClip[1].height = tmp_win->frame_height - newClip[1].y;
	    XShapeCombineRectangles (dpy, dest, ShapeClip, 0, 0,
				     newClip, 2, ShapeSet, YXBanded);
	    tmp_win->fShaped = 2;
	}
	else
	{
	    if (tmp_win->fShaped)
	    {
		XShapeCombineMask (dpy, dest, ShapeBounding, 0, 0,
				   None, ShapeSet);
		tmp_win->fShaped = 0;
	    }
	}
    }
}
#endif
