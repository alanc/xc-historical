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
 * $XConsortium: resize.c,v 1.75 90/09/17 10:15:14 converse Exp $
 *
 * window resizing borrowed from the "wm" window manager
 *
 * 11-Dec-87 Thomas E. LaStrange                File created
 *
 ***********************************************************************/

#if !defined(lint) && !defined(SABER)
static char RCSinfo[]=
"$XConsortium: resize.c,v 1.75 90/09/17 10:15:14 converse Exp $";
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
    unsigned int junkbw, junkDepth;

    ResizeWindow = tmp_win->frame;
    XGrabServer(dpy);
    XGrabPointer(dpy, Scr->Root, True,
        ButtonPressMask | ButtonReleaseMask |
	ButtonMotionMask | PointerMotionHintMask,
        GrabModeAsync, GrabModeAsync,
        Scr->Root, Scr->ResizeCursor, CurrentTime);

    XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
        &dragx, &dragy, (unsigned int *)&dragWidth, (unsigned int *)&dragHeight, &junkbw,
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
    InstallRootColormap();
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
    XGrabServer(dpy);
    XGrabPointer(dpy, Scr->Root, True,
        ButtonReleaseMask | ButtonMotionMask | PointerMotionHintMask,
        GrabModeAsync, GrabModeAsync,
        Scr->Root, Scr->ResizeCursor, CurrentTime);

    dragx = x + tmp_win->frame_bw;
    dragy = y + tmp_win->frame_bw;
    origx = dragx;
    origy = dragy;
    dragWidth = origWidth = w - 2 * tmp_win->frame_bw;
    dragHeight = origHeight = h - 2 * tmp_win->frame_bw;
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

    dheight = height - tmp_win->title_height;
    dwidth = width;

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

    (void) sprintf (str, " %4d x %-4d ", dwidth, dheight);
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

#ifdef DEBUG
    fprintf(stderr, "EndResize\n");
#endif

    MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
    XUnmapWindow(dpy, Scr->SizeWindow);

    XFindContext(dpy, ResizeWindow, TwmContext, (caddr_t *)&tmp_win);

    ConstrainSize (tmp_win, &dragWidth, &dragHeight);

    if (dragWidth != tmp_win->frame_width ||
        dragHeight != tmp_win->frame_height)
            tmp_win->zoomed = ZOOM_NONE;

    SetupWindow (tmp_win, dragx - tmp_win->frame_bw, dragy - tmp_win->frame_bw,
		 dragWidth, dragHeight, -1);

    if (tmp_win->iconmgr)
    {
	int ncols = tmp_win->iconmgrp->cur_columns;
	if (ncols == 0) ncols = 1;

	tmp_win->iconmgrp->width = (int) ((dragWidth *
					   (long) tmp_win->iconmgrp->columns)
					  / ncols);
        PackIconManager(tmp_win->iconmgrp);
    }

    if (!Scr->NoRaiseResize)
        XRaiseWindow(dpy, tmp_win->frame);

    UninstallRootColormap();

    ResizeWindow = None;
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
    AddingW = dragWidth + (2 * tmp_win->frame_bw);
    AddingH = dragHeight + (2 * tmp_win->frame_bw);
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
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )
#define _min(a,b) (((a) < (b)) ? (a) : (b))

    int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
    int baseWidth, baseHeight;
    int dwidth = *widthp, dheight = *heightp;


    dheight -= tmp_win->title_height;

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
        maxWidth = _min (Scr->MaxWindowWidth, tmp_win->hints.max_width);
        maxHeight = _min (Scr->MaxWindowHeight, tmp_win->hints.max_height);
    } else {
        maxWidth = Scr->MaxWindowWidth;
	maxHeight = Scr->MaxWindowHeight;
    }

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
    /*
     * The math looks like this:
     *
     * minAspectX    dwidth     maxAspectX
     * ---------- <= ------- <= ----------
     * minAspectY    dheight    maxAspectY
     *
     * If that is multiplied out, then the width and height are
     * invalid in the following situations:
     *
     * minAspectX * dheight > minAspectY * dwidth
     * maxAspectX * dheight < maxAspectY * dwidth
     * 
     */
    
    if (tmp_win->hints.flags & PAspect)
    {
        if (minAspectX * dheight > minAspectY * dwidth)
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

        if (maxAspectX * dheight < maxAspectY * dwidth)
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
    }


    /*
     * Fourth, account for border width and title height
     */
    *widthp = dwidth;
    *heightp = dheight + tmp_win->title_height;
}


/***********************************************************************
 *
 *  Procedure:
 *      SetupWindow - set window sizes, this was called from either
 *              AddWindow, EndResize, or HandleConfigureNotify.
 *
 *  Inputs:
 *      tmp_win - the TwmWindow pointer
 *      x       - the x coordinate of the upper-left outer corner of the frame
 *      y       - the y coordinate of the upper-left outer corner of the frame
 *      w       - the width of the frame window w/o border
 *      h       - the height of the frame window w/o border
 *      bw      - the border width of the frame window or -1 not to change
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

void SetupWindow (tmp_win, x, y, w, h, bw)
    TwmWindow *tmp_win;
    int x, y, w, h, bw;
{
    SetupFrame (tmp_win, x, y, w, h, bw, False);
}

void SetupFrame (tmp_win, x, y, w, h, bw, sendEvent)
    TwmWindow *tmp_win;
    int x, y, w, h, bw;
    Bool sendEvent;			/* whether or not to force a send */
{
    XEvent client_event;
    XWindowChanges frame_wc, xwc;
    unsigned long frame_mask, xwcm;
    int title_width, title_height;
    int reShape;

#ifdef DEBUG
    fprintf (stderr, "SetupWindow: x=%d, y=%d, w=%d, h=%d, bw=%d\n",
	     x, y, w, h, bw);
#endif

    if (x >= Scr->MyDisplayWidth)
      x = Scr->MyDisplayWidth - 16;	/* one "average" cursor width */
    if (y >= Scr->MyDisplayHeight)
      y = Scr->MyDisplayHeight - 16;	/* one "average" cursor width */
    if (bw < 0)
      bw = tmp_win->frame_bw;		/* -1 means current frame width */

    if (tmp_win->iconmgr) {
	tmp_win->iconmgrp->width = w;
        h = tmp_win->iconmgrp->height + tmp_win->title_height;
    }

    /*
     * According to the July 27, 1988 ICCCM draft, we should send a
     * "synthetic" ConfigureNotify event to the client if the window
     * was moved but not resized.
     */
    if (((x != tmp_win->frame_x || y != tmp_win->frame_y) &&
	 (w == tmp_win->frame_width && h == tmp_win->frame_height)) ||
	(bw != tmp_win->frame_bw))
      sendEvent = TRUE;

    xwcm = CWWidth;
    title_width = xwc.width = w;
    title_height = Scr->TitleHeight + bw;

    ComputeWindowTitleOffsets (tmp_win, xwc.width, True);

    reShape = (tmp_win->wShaped ? TRUE : FALSE);
    if (tmp_win->squeeze_info)		/* check for title shaping */
    {
	title_width = tmp_win->rightx + Scr->TBInfo.rightoff;
	if (title_width < xwc.width)
	{
	    xwc.width = title_width;
	    if (tmp_win->frame_height != h ||
	    	tmp_win->frame_width != w ||
		tmp_win->frame_bw != bw ||
	    	title_width != tmp_win->title_width)
	    	reShape = TRUE;
	}
	else
	{
	    if (!tmp_win->wShaped) reShape = TRUE;
	    title_width = xwc.width;
	}
    }

    tmp_win->title_width = title_width;
    if (tmp_win->title_height) tmp_win->title_height = title_height;

    if (tmp_win->title_w) {
	if (bw != tmp_win->frame_bw) {
	    xwc.border_width = bw;
	    tmp_win->title_x = xwc.x = -bw;
	    tmp_win->title_y = xwc.y = -bw;
	    xwcm |= (CWX | CWY | CWBorderWidth);
	}
	
	XConfigureWindow(dpy, tmp_win->title_w, xwcm, &xwc);
    }

    tmp_win->attr.width = w;
    tmp_win->attr.height = h - tmp_win->title_height;

    XMoveResizeWindow (dpy, tmp_win->w, 0, tmp_win->title_height,
		       w, h - tmp_win->title_height);

    /* 
     * fix up frame and assign size/location values in tmp_win
     */
    frame_mask = 0;
    if (bw != tmp_win->frame_bw) {
	frame_wc.border_width = tmp_win->frame_bw = bw;
	frame_mask |= CWBorderWidth;
    }
    frame_wc.x = tmp_win->frame_x = x;
    frame_wc.y = tmp_win->frame_y = y;
    frame_wc.width = tmp_win->frame_width = w;
    frame_wc.height = tmp_win->frame_height = h;
    frame_mask |= (CWX | CWY | CWWidth | CWHeight);
    XConfigureWindow (dpy, tmp_win->frame, frame_mask, &frame_wc);

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

    if (HasShape && reShape) {
	SetFrameShape (tmp_win);
    }

    if (sendEvent)
    {
        client_event.type = ConfigureNotify;
        client_event.xconfigure.display = dpy;
        client_event.xconfigure.event = tmp_win->w;
        client_event.xconfigure.window = tmp_win->w;
        client_event.xconfigure.x = (x + tmp_win->frame_bw - tmp_win->old_bw);
        client_event.xconfigure.y = (y + tmp_win->frame_bw +
				     tmp_win->title_height - tmp_win->old_bw);
        client_event.xconfigure.width = tmp_win->frame_width;
        client_event.xconfigure.height = tmp_win->frame_height -
                tmp_win->title_height;
        client_event.xconfigure.border_width = tmp_win->old_bw;
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
    unsigned int junkbw, junkDepth;

    XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
        &dragx, &dragy, (unsigned int *)&dragWidth, (unsigned int *)&dragHeight, &junkbw,
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
	    }

	    tmp_win->zoomed = flag;
	    switch (flag)
	    {
	    case ZOOM_NONE:
		break;
	    case F_ZOOM:
		dragHeight = Scr->MyDisplayHeight;
		break;
	    case F_HORIZOOM:
		dragWidth = Scr->MyDisplayWidth;
		break;
	    case F_FULLZOOM:
		dragHeight = Scr->MyDisplayHeight;
		dragWidth = Scr->MyDisplayWidth;
		break;
	    case F_LEFTZOOM:
		dragHeight = Scr->MyDisplayHeight;
		dragWidth = Scr->MyDisplayWidth/2;
		break;
	    case F_RIGHTZOOM:
		dragHeight = Scr->MyDisplayHeight;
		dragWidth = Scr->MyDisplayWidth/2;
		break;
	    case F_TOPZOOM:
		dragHeight = Scr->MyDisplayHeight/2;
		dragWidth = Scr->MyDisplayWidth;
		break;
	    case F_BOTTOMZOOM:
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

    if (dragWidth > Scr->MaxWindowWidth)
	dragWidth = Scr->MaxWindowWidth;
    if (dragHeight > Scr->MaxWindowHeight)
	dragHeight = Scr->MaxWindowHeight;
    if (tmp_win->hints.flags&PMaxSize)
    {
	if (dragWidth > tmp_win->hints.max_width)
	    dragWidth = tmp_win->hints.max_width;
	if (dragHeight > tmp_win->hints.max_height)
	    dragHeight = tmp_win->hints.max_height;
    }

    dragHeight += tmp_win->title_height;

    switch (flag)
    {
    case ZOOM_NONE:
	break;
    case F_ZOOM:
	if (dragy + dragHeight > Scr->MyDisplayHeight)
	    dragy = Scr->MyDisplayHeight - dragHeight;
	break;
    case F_HORIZOOM:
	if (dragx + dragWidth > Scr->MyDisplayWidth)
	    dragx = Scr->MyDisplayWidth - dragWidth;
	break;
    case F_FULLZOOM:
	if (dragx + dragWidth > Scr->MyDisplayWidth)
	    dragx = Scr->MyDisplayWidth - dragWidth;
	if (dragy + dragHeight > Scr->MyDisplayHeight)
	    dragy = Scr->MyDisplayHeight - dragHeight;
	break;
    case F_LEFTZOOM:
	dragx = 0;
	if (dragy + dragHeight > Scr->MyDisplayHeight)
	    dragy = Scr->MyDisplayHeight - dragHeight;
	break;
    case F_RIGHTZOOM:
	dragx = Scr->MyDisplayWidth - dragWidth;
	if (dragy + dragHeight > Scr->MyDisplayHeight)
	    dragy = Scr->MyDisplayHeight - dragHeight;
	break;
    case F_TOPZOOM:
	if (dragx + dragWidth > Scr->MyDisplayWidth)
	    dragx = Scr->MyDisplayWidth - dragWidth;
	dragy = 0;
	break;
    case F_BOTTOMZOOM:
	if (dragx + dragWidth > Scr->MyDisplayWidth)
	    dragx = Scr->MyDisplayWidth - dragWidth;
	dragy = Scr->MyDisplayHeight - dragHeight;
	break;
    }
    if (dragx < 0)
	dragx = 0;
    if (dragy < 0)
	dragy = 0;

    SetupWindow (tmp_win, dragx , dragy , dragWidth, dragHeight, -1);
    XUngrabPointer (dpy, CurrentTime);
    XUngrabServer (dpy);
}

SetFrameShape (tmp)
    TwmWindow *tmp;
{
    /*
     * see if the titlebar needs to move
     */
    if (tmp->title_w) {
	int oldx = tmp->title_x, oldy = tmp->title_y;
	ComputeTitleLocation (tmp);
	if (oldx != tmp->title_x || oldy != tmp->title_y)
	  XMoveWindow (dpy, tmp->title_w, tmp->title_x, tmp->title_y);
    }

    /*
     * The frame consists of the shape of the contents window offset by
     * title_height or'ed with the shape of title_w (which is always
     * rectangular).
     */
    if (tmp->wShaped) {
	/*
	 * need to do general case
	 */
	XShapeCombineShape (dpy, tmp->frame, ShapeBounding,
			    0, tmp->title_height, tmp->w,
			    ShapeBounding, ShapeSet);
	if (tmp->title_w) {
	    XShapeCombineShape (dpy, tmp->frame, ShapeBounding,
				tmp->title_x + tmp->frame_bw,
				tmp->title_y + tmp->frame_bw,
				tmp->title_w, ShapeBounding,
				ShapeUnion);
	}
    } else {
	/*
	 * can optimize rectangular contents window
	 */
	if (tmp->squeeze_info) {
	    XRectangle  newBounding[2];
	    XRectangle  newClip[2];
	    int fbw2 = 2 * tmp->frame_bw;

	    /*
	     * Build the border clipping rectangles; one around title, one
	     * around window.  The title_[xy] field already have had frame_bw
	     * subtracted off them so that they line up properly in the frame.
	     *
	     * The frame_width and frame_height do *not* include borders.
	     */
	    /* border */
	    newBounding[0].x = tmp->title_x;
	    newBounding[0].y = tmp->title_y;
	    newBounding[0].width = tmp->title_width + fbw2;
	    newBounding[0].height = tmp->title_height;
	    newBounding[1].x = -tmp->frame_bw;
	    newBounding[1].y = Scr->TitleHeight;
	    newBounding[1].width = tmp->attr.width + fbw2;
	    newBounding[1].height = tmp->attr.height + fbw2;
	    XShapeCombineRectangles (dpy, tmp->frame, ShapeBounding, 0, 0,
				     newBounding, 2, ShapeSet, YXBanded);
	    /* insides */
	    newClip[0].x = tmp->title_x + tmp->frame_bw;
	    newClip[0].y = 0;
	    newClip[0].width = tmp->title_width;
	    newClip[0].height = Scr->TitleHeight;
	    newClip[1].x = 0;
	    newClip[1].y = tmp->title_height;
	    newClip[1].width = tmp->attr.width;
	    newClip[1].height = tmp->attr.height;
	    XShapeCombineRectangles (dpy, tmp->frame, ShapeClip, 0, 0,
				     newClip, 2, ShapeSet, YXBanded);
	} else {
	    (void) XShapeCombineMask (dpy, tmp->frame, ShapeBounding, 0, 0,
 				      None, ShapeSet);
	    (void) XShapeCombineMask (dpy, tmp->frame, ShapeClip, 0, 0,
				      None, ShapeSet);
	}
    }
}

/*
 * Squeezed Title:
 * 
 *                         tmp->title_x
 *                   0     |
 *  tmp->title_y   ........+--------------+.........  -+,- tmp->frame_bw
 *             0   : ......| +----------+ |....... :  -++
 *                 : :     | |          | |      : :   ||-Scr->TitleHeight
 *                 : :     | |          | |      : :   ||
 *                 +-------+ +----------+ +--------+  -+|-tmp->title_height
 *                 | +---------------------------+ |  --+
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | +---------------------------+ |
 *                 +-------------------------------+
 * 
 * 
 * Unsqueezed Title:
 * 
 *                 tmp->title_x
 *                 | 0
 *  tmp->title_y   +-------------------------------+  -+,tmp->frame_bw
 *             0   | +---------------------------+ |  -+'
 *                 | |                           | |   |-Scr->TitleHeight
 *                 | |                           | |   |
 *                 + +---------------------------+ +  -+
 *                 |-+---------------------------+-|
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | |                           | |
 *                 | +---------------------------+ |
 *                 +-------------------------------+
 * 
 * 
 * 
 * Dimensions and Positions:
 * 
 *     frame orgin                 (0, 0)
 *     frame upper left border     (-tmp->frame_bw, -tmp->frame_bw)
 *     frame size w/o border       tmp->frame_width , tmp->frame_height
 *     frame/title border width    tmp->frame_bw
 *     extra title height w/o bdr  tmp->title_height = TitleHeight + frame_bw
 *     title window height         Scr->TitleHeight
 *     title origin w/o border     (tmp->title_x, tmp->title_y)
 *     client origin               (0, Scr->TitleHeight + tmp->frame_bw)
 *     client size                 tmp->attr.width , tmp->attr.height
 * 
 * When shaping, need to remember that the width and height of rectangles
 * are really deltax and deltay to lower right handle corner, so they need
 * to have -1 subtracted from would normally be the actual extents.
 */
