/*
 * $XConsortium: XMultibuf.c,v 1.3 89/09/22 18:15:12 jim Exp $
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
 * Authors:  Jim Fulton and Keith Packard, MIT X Consortium
 */

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "multibufst.h"
#include "extutil.h"

static XExtDisplayInfo *display_list;	/* starts out NULL */
static XExtDisplayInfo *last_display;	/* most recently used */


/*
 * wire_to_event - convert a wire event in network format to a C 
 * event structure
 */
static int wire_to_event (dpy, re, event)
    Display *dpy;
    XEvent *re;
    xEvent *event;
{
    XExtDisplayInfo *info = find_display (dpy);

    if (!info && !info->codes) return 0;
    switch ((event->u.u.type & 0x7f) - info->codes->first_event) {
      case ClobberNotify:
	{
	    XClobberNotifyEvent	*se;
	    xClobberNotifyEvent	*sevent;
    
    	    se = (XClobberNotifyEvent *) re;
	    sevent = (xClobberNotifyEvent *) event;
    	    se->type = sevent->type & 0x7f;
    	    se->serial = _XSetLastRequestRead(dpy,(xGenericReply *) event);
    	    se->send_event = ((sevent->type & 0x80) != 0);
    	    se->display = dpy;
    	    se->buffer = sevent->buffer;
    	    return 1;
	}
    }
    return 0;
}


/*
 * event_to_wire - convert a C event structure to a wire event in
 * network format
 */
static int event_to_wire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtDisplayInfo *info = find_display (dpy);

    if (!info && !info->codes) return 0;
    switch ((re->type & 0x7f) - info->codes->first_event) {
      case ClobberNotify:
	{
	    XClobberNotifyEvent	*se;
	    xClobberNotifyEvent	*sevent;
    
    	    se = (XClobberNotifyEvent *) re;
	    sevent = (xClobberNotifyEvent *) event;
    	    sevent->type = (se->type | (se->send_event ? 0x80 : 0));
    	    sevent->sequenceNumber = (se->serial & 0xffff);
    	    sevent->buffer = se->buffer;
    	    return 1;
	}
    }
    return 0;
}


/*
 * close_display - called on XCloseDisplay, this should remove the
 * dpyinfo from the display list and clear the cached display, if necessary.
 */

/*ARGSUSED*/
static int close_display (dpy, codes)
    Display *dpy;
    XExtCodes *codes;
{
    /* free any extension data */
    return XextCloseDisplay(&display_list, &last_display, dpy);
}


/*
 * find_display - locate the display info block
 */
static XExtDisplayInfo *find_display (dpy)
    register Display *dpy;
{
    XExtDisplayInfo *dpyinfo = XextFindDisplay (&display_list, &last_display,
						dpy);
    if (!dpyinfo) {
	/* create any extension data blocks */
	dpyinfo = XextInitDisplay (&display_list, &last_display, dpy,
				   MULTIBUF_PROTOCOL_NAME, close_display,
				   wire_to_event, event_to_wire, nevents,
				   NULL);
    }

    return dpyinfo;
}




/*****************************************************************************
 *                                                                           *
 *		    Multibuffering/stereo public interfaces                  *
 *                                                                           *
 *****************************************************************************/


Bool XmbufQueryExtension (dpy, event_base_return, error_base_return)
    Display *dpy;
    int *event_base_return, *error_base_return;
{
    XExtDisplayInfo *info = find_display (dpy);
    
    if (info && info->codes) {
	*event_base_return = info->codes->first_event;
	*error_base_return = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}


Status XmbufGetVersion (dpy, major_version_return, minor_version_return)
    Display *dpy;
    int *major_version_return, *minor_version_return;
{
    XExtDisplayInfo *info = find_display (dpy);
    xGetBufferVersionReply rep;
    register xGetBufferVersionReq *req;

    if (!(info && info->codes)) return 0;

    LockDisplay (dpy);
    GetReq (GetBufferVersion, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_GetBufferVersion;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *major_version_return = rep.majorVersion;
    *minor_version_return = rep.minorVersion;
    UnlockDisplay (dpy);

    SyncHandle ();
    return 1;
}


int XCreateImageBuffers (dpy, w, count, update_action, update_hint,
			 buffers_return)
    Display *dpy;
    Window w;
    int count;
    int update_action, update_hint;
    Multibuffer *buffers;
{
    XExtDisplayInfo *info = find_display (dpy);
    xCreateImageBuffersReply rep;
    register xCreateImageBuffersReq *req;
    int i, result;

    if (!(info && info->codes)) return 0;

    /*
     * allocate the id; hopefully, it would be nice to be able to 
     * get rid of the ones we don't need, but this would require access
     * various Xlib internals.  we could do caching on this side, but the
     * chances of wasting enough resources to really matter makes it not
     * worth the bother.
     */
    for (i = 0; i < count; i++) buffers[i] = XAllocID (dpy);

    LockDisplay (dpy);
    GetReq (CreateImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_CreateImageBuffers;
    req->window = window;
    req->updateAction = updateAction;
    req->updateHint = updateHint;
    req->length += count;
    PackData32 (dpy, buffers, count * sizeof (Buffer));
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    result = rep.numberBuffer;
    UnlockDisplay (dpy);

    SyncHandle ();
    return result;
}


void
XDisplayImageBuffers (dpy, count, buffers, min_delay, max_delay)
    Display *dpy;
    int	    count;
    Buffer  *buffers;
    int	    min_delay, max_delay;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xDisplayImageBuffersReq *req;
    int i;

    if (!(info && info->codes)) return;

    LockDisplay (dpy);
    GetReq (DisplayImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_DisplayImageBuffers;
    req->minDelay = min_delay;
    req->maxDelay = max_delay;
    req->length += count;
    PackData32 (dpy, buffers, count * sizeof (Buffer));
    UnlockDisplay (dpy);
    SyncHandle();
}

void
XDestroyImageBuffers (dpy, window)
    Display *dpy;
    Window  window;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xDestroyImageBuffersReq *req;
    int i;

    if (!(info && info->codes)) return;

    LockDisplay (dpy);
    GetReq (DestroyImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_DestroyImageBuffers;
    req->window = window;
    UnlockDisplay (dpy);
    SyncHandle ();
}
