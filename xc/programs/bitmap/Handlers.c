/*
 * $XConsortium: Handlers.c,v 1.1 90/06/09 20:20:35 dmatic Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
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
 *
 * Author:  Davor Matic, MIT X Consortium
 */



#include <X11/copyright.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xmu/Converters.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include "BitmapP.h"
    
#include <stdio.h>
#include <string.h>
#include <math.h>

#define XtStrlen(s)                   ((s) ? strlen(s) : 0)
#define abs(x)                        (((x) > 0) ? (x) : -(x))
#define min(x, y)                     (((x) < (y)) ? (x) : (y))
#define max(x, y)                     (((x) > (y)) ? (x) : (y))

#include "Requests.h"

static Boolean DEBUG = False;

/*****************************************************************************
 *                                  Handlers                                 *
 *****************************************************************************/

#define QueryInSquare(BW, x, y, square_x, square_y)\
    ((InBitmapX(BW, x) == (square_x)) &&\
     (InBitmapY(BW, y) == (square_y)))


void DragOnePointHandler(w, status, event)
     Widget       w;
     BWStatus    *status;
     XEvent      *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "D1PH ");

    switch (event->type) {
    
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->at_x, status->at_y)) {
	    BWStoreToBuffer(w);
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, status->value);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->at_x, status->at_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
      
	    BWTerminateRequest(w, TRUE);
	}
	break;

    case MotionNotify:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
			       status->at_x, status->at_y)) {
		status->at_x = InBitmapX(BW, event->xmotion.x);
		status->at_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, status->value);
	    }
	}
	break;

    }
}

void DragOnePointEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    
    status->at_x = NotSet;
    status->at_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;
    
    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		      FALSE, DragOnePointHandler, status);
}

void DragOnePointTerminate(w, status, client_data)
    Widget     w;
    BWStatus  *status;
    caddr_t    client_data;
{
    
    if (status->success) {
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    
    XtRemoveEventHandler(w,
		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		 FALSE, DragOnePointHandler, status);
    
}

void OnePointHandler(w, status, event)
    Widget       w;
    BWStatus    *status;
    XEvent      *event;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (DEBUG)
	fprintf(stderr, "1PH ");

    switch (event->type) {
	
    case Expose:
	if (QuerySet(status->at_x, status->at_y)) {
	    BWClip(w, 
		   InBitmapX(BW, event->xexpose.x), 
		   InBitmapY(BW, event->xexpose.y),
		   InBitmapX(BW, event->xexpose.x + event->xexpose.width),
		   InBitmapY(BW, event->xexpose.y + event->xexpose.height));
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	    
	    BWUnclip(w);
	}
	break;
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->at_x, status->at_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	    
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
			       status->at_x, status->at_y)) {
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, Highlight);
		status->at_x = InBitmapX(BW, event->xmotion.x);
		status->at_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, Highlight);
	    }
	}      
	break;
    }
}

void OnePointEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    status->at_x = NotSet;
    status->at_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, OnePointHandler, status);
}

void OnePointImmediateEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    status->at_x = 0;
    status->at_y = 0;
    status->draw = draw;
    status->success = False;
    status->state = *state;
    
    if (status->draw)
	(*status->draw)(w,
			status->at_x, status->at_y, Highlight);
    
    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, OnePointHandler, status);
}

void OnePointTerminate(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw) {
	BWStoreToBuffer(w);
	(*draw)(w,
		status->at_x, status->at_y,
		status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }    
    else
	if (QuerySet(status->at_x, status->at_y))
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, OnePointHandler, status);
}

void OnePointTerminateTransparent(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->at_x, status->at_y,
		status->value);
    else
	if (QuerySet(status->at_x, status->at_y))
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, OnePointHandler, status);
    
}


void TwoPointsHandler(w, status, event)
    Widget      w;
    BWStatus   *status;
    XEvent     *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "2PH ");
    
    switch (event->type) {
	
    case Expose:
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y)) {
	    BWClip(w, 
		   InBitmapX(BW, event->xexpose.x), 
		   InBitmapY(BW, event->xexpose.y),
		   InBitmapX(BW, event->xexpose.x + event->xexpose.width),
		   InBitmapY(BW, event->xexpose.y + event->xexpose.height));
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	    BWUnclip(w);
	}
	break;
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->from_x, status->from_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->from_x = InBitmapX(BW, event->xbutton.x);
	    status->from_y = InBitmapY(BW, event->xbutton.y);
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;	    
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (QuerySet(status->to_x, status->to_y)) {
		if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
				   status->to_x, status->to_y)) {
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, Highlight);
		    status->to_x = InBitmapX(BW, event->xmotion.x);
		    status->to_y = InBitmapY(BW, event->xmotion.y);
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, Highlight);
		}
	    }
	    else {
		status->to_x = InBitmapX(BW, event->xmotion.x);
		status->to_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->from_x, status->from_y, 
				    status->to_x, status->to_y, Highlight);
	    }
	}
	break;
    }
}

void TwoPointsEngage(w, status, draw, state)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
    int       *state;
{
    
    status->from_x = NotSet;
    status->from_y = NotSet;
    status->to_x = NotSet;
    status->to_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminate(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw) {
	BWStoreToBuffer(w);
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminateTransparent(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->value);
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminateTimed(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->btime);
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void Interface(w, status, action)
    Widget     w;
    caddr_t    status;
    void     (*action)();
{
 	(*action)(w);
}

void Paste(w, at_x, at_y, value)
    Widget    w;
    Position  at_x, at_y;
    int       value;
{
    BitmapWidget    BW = (BitmapWidget) w;
    BWStatus       *my_status;
    BWRequest       request;

    my_status = (BWStatus *) 
	BW->bitmap.request_stack[BW->bitmap.current].status;

    my_status->draw = NULL;

   request = (BWRequest)
   BW->bitmap.request_stack[BW->bitmap.current].request->terminate_client_data;
	
    BWTerminateRequest(w, FALSE);
    
    if ((at_x == max(BW->bitmap.mark.from_x, min(at_x, BW->bitmap.mark.to_x)))
	&&
      (at_y == max(BW->bitmap.mark.from_y, min(at_y, BW->bitmap.mark.to_y)))) {
	
	BWStatus *status;
	
	if (DEBUG)
	    fprintf(stderr, "Prepaste request: %s\n", request);
	
	BWEngageRequest(w, request, False, &(my_status->state), sizeof(int));
	
	status = (BWStatus *) 
	    BW->bitmap.request_stack[BW->bitmap.current].status;
	
	status->at_x = at_x;
	status->at_y = at_y;
	status->value = value;
	(*status->draw) (w, at_x, at_y, Highlight);	
    }
    else {

	BWStatus *status;
	
      BWEngageRequest(w, MarkRequest, False, &(my_status->state), sizeof(int));
	
	status = (BWStatus *) 
	    BW->bitmap.request_stack[BW->bitmap.current].status;
	
	status->from_x = status->to_x = at_x;
	status->from_y = status->to_y = at_y;
	status->value = value;
	(*status->draw) (w, at_x, at_y, at_x, at_y, Highlight);
    }
}


void DragTwoPointsHandler(w, status, event)
    Widget      w;
    BWStatus   *status;
    XEvent     *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "D2PH ");

    switch (event->type) {
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->from_x, status->from_y)) {
	    BWStoreToBuffer(w);
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;
	    status->from_x = InBitmapX(BW, event->xbutton.x);
	    status->from_y = InBitmapY(BW, event->xbutton.y);
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, status->value);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->from_x, status->from_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->btime = event->xbutton.time;	    
	    status->from_x = status->to_x;
	    status->from_y = status->to_y;
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (QuerySet(status->to_x, status->to_y)) {
		if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
				   status->to_x, status->to_y)) {
		    status->from_x = status->to_x;
		    status->from_y = status->to_y;
		    status->to_x = InBitmapX(BW, event->xmotion.x);
		    status->to_y = InBitmapY(BW, event->xmotion.y);
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, status->value);
		}
	    }
	}
	break;
    }
}

void DragTwoPointsEngage(w, status, draw, state)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
    int       *state;
{
    
    status->from_x = NotSet;
    status->from_y = NotSet;
    status->to_x = NotSet;
    status->to_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		      FALSE, DragTwoPointsHandler, status);
}

void DragTwoPointsTerminate(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw) {
	if ((status->from_x != status->to_x) 
	    || 
	    (status->from_y != status->to_y))
	    (*draw)(w,
		    status->from_x, status->from_y,
		    status->to_x, status->to_y,
		    status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    
    XtRemoveEventHandler(w,
		         ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			 FALSE, DragTwoPointsHandler, status);
}


void BWTPaste(w, event)
    Widget  w;
    XEvent *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (!BW->bitmap.selection.own)
	BWRequestSelection(w, event->xbutton.time, TRUE);
    else 
	BWStore(w);

    if (!BWQueryStored(w))
	return;
    
    BWEngageRequest(w, RestoreRequest, False, 
		    &(event->xbutton.state), sizeof(int));
    
    OnePointHandler(w,
	       (BWStatus*) BW->bitmap.request_stack[BW->bitmap.current].status,
	       event);
}

void BWTMark(w, event)
    Widget  w;
    XEvent *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    BWEngageRequest(w, MarkRequest, False,
		    &(event->xbutton.state), sizeof(int));
    TwoPointsHandler(w,
            (BWStatus*) BW->bitmap.request_stack[BW->bitmap.current].status,
	     event);
}

void BWTUnmark(w)
    Widget w;
{
    BWUnmark(w);
}


/*****************************************************************************/
