/*
 * $XConsortium: XFilterEv.c,v 1.2 91/02/04 11:48:32 morisaki Exp $
 */

 /*
  * Copyright 1990, 1991 by OMRON Corporation
  * Copyright 1991 by the Massachusetts Institute of Technology
  *
  * Permission to use, copy, modify, distribute, and sell this software and its
  * documentation for any purpose is hereby granted without fee, provided that
  * the above copyright notice appear in all copies and that both that
  * copyright notice and this permission notice appear in supporting
  * documentation, and that the names of OMRON and MIT not be used in
  * advertising or publicity pertaining to distribution of the software without
  * specific, written prior permission.  OMRON and MIT make no representations
  * about the suitability of this software for any purpose.  It is provided
  * "as is" without express or implied warranty.
  *
  * OMRON AND MIT DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
  * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
  * EVENT SHALL OMRON OR MIT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
  * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
  * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
  * TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  * PERFORMANCE OF THIS SOFTWARE. 
  *
  *	Author:	Seiji Kuwari	OMRON Corporation
  *				kuwa@omron.co.jp
  *				kuwa%omron.co.jp@uunet.uu.net
  */				

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/XIMlib.h>
#include "XIMlibint.h"
#include "XFilter.h"

XFilterEventList filter_list = NULL;

/*
 * Look up if there is a specified filter for the event.
 */
Bool
XFilterEvent(ev, window)
XEvent *ev;
Window window;
{
    XFilterEventList	p;
    Window		win;
    int			ret = False;
    static unsigned long masks[] = {
	0,						/* shouldn't see 0  */
	0,						/* shouldn't see 1  */
	KeyPressMask,					/* KeyPress	    */
	KeyReleaseMask,					/* KeyRelease       */
	ButtonPressMask,				/* ButtonPress      */
	ButtonReleaseMask,				/* ButtonRelease    */
	PointerMotionMask | Button1MotionMask		/* MotionNotify     */
	  | Button2MotionMask | Button3MotionMask
	  | Button4MotionMask | Button5MotionMask,
	EnterWindowMask,				/* EnterNotify      */
	LeaveWindowMask,				/* LeaveNotify      */
	FocusChangeMask,				/* FocusIn          */
	FocusChangeMask,				/* FocusOut         */
	KeymapStateMask,				/* KeymapNotify     */
	ExposureMask,					/* Expose           */
	0,						/* GraphicsExpose   */
	0,						/* NoExpose         */
	VisibilityChangeMask,				/* VisibilityNotify */
	SubstructureNotifyMask,				/* CreateNotify     */
	StructureNotifyMask | SubstructureNotifyMask,	/* DestroyNotify    */
	StructureNotifyMask | SubstructureNotifyMask,	/* UnmapNotify      */
	StructureNotifyMask | SubstructureNotifyMask,	/* MapNotify        */
	SubstructureRedirectMask,			/* MapRequest       */
	StructureNotifyMask | SubstructureNotifyMask,	/* ReparentNotify   */
	StructureNotifyMask | SubstructureNotifyMask,	/* ConfigureNotify  */
	SubstructureRedirectMask,			/* ConfigureRequest */
	StructureNotifyMask | SubstructureNotifyMask,	/* GravityNotify    */
	ResizeRedirectMask,				/* ResizeRequest    */
	StructureNotifyMask | SubstructureNotifyMask,	/* CirculateNotify  */
	SubstructureRedirectMask,			/* CirculateRequest */
	PropertyChangeMask,				/* PropertyNotify   */
	0,						/* SelectionClear   */
	0,						/* SelectionRequest */
	0,						/* SelectionNotify  */
	ColormapChangeMask,				/* ColormapNotify   */
	0,						/* ClientMessage    */
	0,						/* MappingNotify    */
    };

    if (window) {
	win = window;
    } else {
	win = ev->xany.window;
    }
    for (p = filter_list; p != NULL; p = p->next) {
	if (ev->xany.display == p->display && win == p->window) {
	    if ((masks[ev->type] && (masks[ev->type] & p->event_mask)) || (masks[ev->type] == 0 && p->nonmaskable)) {
	      return((*(p->filter))(p->display, p->window, ev,
				    p->client_data));
	    }
	}
    }
    return(False);
}
