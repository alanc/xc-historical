/*
 * $XConsortium$
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

/*
 * We need to keep a list of open displays since the Xlib display list isn't
 * public.  We also have to per-display info in a separate block since it isn't
 * stored directly in the Display structure.
 */
typedef struct _XmbufDisplayInfo {
    struct _XmbufDisplayInfo *next;	/* keep a linked list */
    Display *display;			/* which display this is */
    XExtCodes *codes;			/* the extension protocol codes */
} XmbufDisplayInfo;

static XmbufDisplayInfo *mbuf_display_list;	/* starts out NULL */
static XmbufDisplayInfo *mbuf_cached_display;	/* most recently used */


/*
 * Xlib hook routines
 */
static int mbuf_close_display();	/* catch close display */
static int mbuf_wire_to_event();	/* when reading an event */
static int mbuf_event_to_wire();	/* when writing an event */


static XExtCodes *mbuf_find_display_codes (dpy)
    register Display *dpy;
{
    XmbufDisplayInfo *dpyinfo;

    /*
     * see if this was the most recently accessed display
     */
    if (mbuf_cached_display && mbuf_cached_display->display == dpy) {
	return mbuf_cached_display->codes;
    }


    /*
     * look for display in list
     */
    for (dpyinfo = mbuf_display_list; dpyinfo; dpyinfo = dpyinfo->next) {
	if (dpyinfo->display == dpy) break;
    }


    /*
     * if not found, create a connection block
     */
    if (!dpyinfo) {
	dpyinfo = (XmbufDisplayInfo *) Xmalloc (sizeof (XmbufDisplayInfo));
	if (!dpyinfo) return NULL;
	dpyinfo->next = mbuf_display_list;
	dpyinfo->display = dpy;
	dpyinfo->codes = XInitExtension (dpy, MULTIBUF_PROTOCOL_NAME);

	/*
	 * if the server has the extension, then we can initialize the 
	 * appropriate function vectors
	 */
	if (dpyinfo->codes) {
	    int i, j;

	    XESetCloseDisplay (dpy, dpyinfo->codes->extension,
			       mbuf_close_display);
	    for (i = 0, j = dpyinfo->codes->first_event;
		 i < MultibufNumberEvents; i++, j++) {
		XESetWireToEvent (dpy, j, mbuf_wire_to_event);
		XESetEventToWire (dpy, j, mbuf_event_to_wire);
	    }
	}

	/*
	 * now, chain it onto the list
	 */
	mbuf_display_list = dpyinfo;
    }

    /*
     * set this display to be the most recently used and return
     */
    mbuf_cached_display = dpyinfo;
    return dpyinfo->codes;
}

				    
/*
 * Xlib hooks
 */

/*
 * mbuf_close_display - called on XCloseDisplay, this should remove the
 * dpyinfo from the display list and clear the cached display, if necessary.
 */

/*ARGSUSED*/
static int mbuf_close_display (dpy, codes)
    Display *dpy;
    XExtCodes *codes;
{
    XmbufDisplayInfo *dpyinfo, *prev;

    /*
     * locate this display and its back link so that it can be removed
     */
    prev = NULL;
    for (dpyinfo = mbuf_display_list; dpyinfo; dpyinfo = dpyinfo->next) {
	if (dpyinfo->display == dpy) break;
	prev = dpyinfo;
    }
    if (!dpyinfo) return 0;		/* hmm, actually an error */

    if (prev)
	prev->next = dpyinfo->next;
    else
	mbuf_display_list = dpyinfo->next;

    if (dpyinfo == mbuf_cached_display)
	mbuf_cached_display = (XmbufDisplayInfo *) NULL;

    Xfree (dpyinfo);
    return 0;
}


/*
 * mbuf_wire_to_event - convert a wire event in network format to a C 
 * event structure
 */
static int mbuf_wire_to_event (dpy, re, event)
    Display *dpy;
    XEvent *re;
    xEvent *event;
{
    XExtCodes *codes;

    codes = mbuf_find_display_codes (dpy);
    if (!codes) return False;
    switch ((event->u.u.type & 0x7f) - codes->first_event) {
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
 * mbuf_event_to_wire - convert a C event structure to a wire event in
 * network format
 */
static int mbuf_event_to_wire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtCodes *codes;

    codes = mbuf_find_display_codes (dpy);
    if (!codes) return False;
    switch ((re->type & 0x7f) - codes->first_event) {
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
 * Multibuffering/stereo public interfaces
 */


Bool
XBufferQueryExtension (dpy)
    Display *dpy;
{
    return mbuf_find_display_codes (dpy) != 0;
}

int
XBufferGetEventBase (dpy)
    Display *dpy;
{
    XExtCodes	*codes;

    codes = mbuf_find_display_codes (dpy);
    if (!codes)
	return -1;
    return codes->first_event;
}

Bool
XGetBufferVersion(dpy, majorVersion, minorVersion)
    Display *dpy;
    int	    *majorVersion, *minorVersion;
{
    XExtCodes			    *codes;
    xGetBufferVersionReply	    rep;
    register xGetBufferVersionReq  *req;

    if (!(codes = mbuf_find_display_codes (dpy)))
	return 0;
    LockDisplay (dpy);
    GetReq (GetBufferVersion, req);
    req->reqType = codes->major_opcode;
    req->bufferReqType = X_GetBufferVersion;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    UnlockDisplay (dpy);
    SyncHandle ();
    return 1;
}

int
XCreateImageBuffers (dpy, window, updateAction, updateHint, count, buffers)
    Display *dpy;
    Window  window;
    int	    updateAction;
    int	    updateHint;
    int	    count;
    Buffer  *buffers;
{
    XExtCodes			    *codes;
    xCreateImageBuffersReply	    rep;
    register xCreateImageBuffersReq *req;
    int				    i;
    int				    result;

    if (!(codes = mbuf_find_display_codes (dpy)))
	return 0;
    for (i = 0; i < count; i++)
	buffers[i] = XAllocID (dpy);
    LockDisplay (dpy);
    GetReq (CreateImageBuffers, req);
    req->reqType = codes->major_opcode;
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
    XExtCodes			    *codes;
    register xDisplayImageBuffersReq *req;
    int				    i;

    if (!(codes = mbuf_find_display_codes (dpy)))
	return;
    LockDisplay (dpy);
    GetReq (DisplayImageBuffers, req);
    req->reqType = codes->major_opcode;
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
    XExtCodes			    *codes;
    register xDestroyImageBuffersReq *req;
    int				    i;

    if (!(codes = mbuf_find_display_codes (dpy)))
	return;
    LockDisplay (dpy);
    GetReq (DestroyImageBuffers, req);
    req->reqType = codes->major_opcode;
    req->bufferReqType = X_DestroyImageBuffers;
    req->window = window;
    UnlockDisplay (dpy);
    SyncHandle ();
}
