/*
 * $XConsortium: XMultibuf.c,v 1.5 89/09/25 16:08:06 jim Exp $
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
 * Authors:  Jim Fulton, MIT X Consortium
 */

#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "multibufst.h"
#include "extutil.h"

static XExtensionInfo *multibuf_info;	/* starts out NULL */


/*
 * find_display - locate the display info block
 */
static XExtDisplayInfo *find_display (dpy)
    register Display *dpy;
{
    XExtDisplayInfo *dpyinfo;
    static int write_to_event(), event_to_wire();

    if (!multibuf_info) {
	multibuf_info = XextCreateExtension ();
	if (!multibuf_info) return NULL;
    }

    dpyinfo = XextFindDisplay (multibuf_info, dpy);
    if (!dpyinfo) {
	/* create any extension data blocks */
	dpyinfo = XextAddDisplay (multibuf_info, dpy, 
				  MULTIBUF_PROTOCOL_NAME, close_display,
				  wire_to_event, event_to_wire, 
				  MultibufferNumberOfEvents, NULL);
    }

    return dpyinfo;
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
    /* get info pointer and free any extension data, if necessary */
    return XextRemoveDisplay (multibuf_info, dpy);
}



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

    if (!XextHasExtension(info)) return 0;

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

    if (!XextHasExtension(info)) return 0;

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




/*****************************************************************************
 *                                                                           *
 *		    Multibuffering/stereo public interfaces                  *
 *                                                                           *
 *****************************************************************************/


/* 
 * XmbufQueryExtension - 
 * 	Returns True if the multibuffering/stereo extension is available
 * 	on the given display.  If the extension exists, the value of the
 * 	first event code (which should be added to the event type constants
 * 	MultibufferClobberNotify and MultibufferUpdateNotify to get the
 * 	actual values) is stored into event_base and the value of the first
 * 	error code (which should be added to the error type constant
 * 	MultibufferBadBuffer to get the actual value) is stored into 
 * 	error_base.
 */
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


/* 
 * XmbufGetVersion -
 * 	Gets the major and minor version numbers of the extension.  The return
 * 	value is zero if an error occurs or non-zero if no error happens.
 */
Status XmbufGetVersion (dpy, major_version_return, minor_version_return)
    Display *dpy;
    int *major_version_return, *minor_version_return;
{
    XExtDisplayInfo *info = find_display (dpy);
    xGetBufferVersionReply rep;
    register xMultibufGetBufferVersionReq *req;

    if (!(info && info->codes)) return 0;

    LockDisplay (dpy);
    GetReq (MbufGetBufferVersion, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufGetBufferVersion;
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


/*
 * XmbufCreateBuffers - 
 * 	Requests that "count" buffers be created with the given update_action
 * 	and update_hint and be associated with the indicated window.  The
 * 	number of buffers created is returned (zero if an error occurred)
 * 	and buffers_return is filled in with that many Multibuffer identifiers.
 */
int XmbufCreateImageBuffers (dpy, w, count, update_action, update_hint,
			     buffers_return)
    Display *dpy;
    Window w;
    int count;
    int update_action, update_hint;
    Multibuffer *buffers;
{
    XExtDisplayInfo *info = find_display (dpy);
    xMbufCreateImageBuffersReply rep;
    register xMbufCreateImageBuffersReq *req;
    int i, result;

    if (!(info && info->codes)) return 0;

    /*
     * allocate the id; hopefully, it would be nice to be able to 
     * get rid of the ones we don't need, but this would require access
     * various Xlib internals.  we could do caching on this side, but the
     * chances of wasting enough resources to really matter is very small
     */
    for (i = 0; i < count; i++) buffers[i] = XAllocID (dpy);

    LockDisplay (dpy);
    GetReq (MbufCreateImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufCreateImageBuffers;
    req->window = w;
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


/*
 * XmbufDestroyBuffers - 
 * 	Destroys the buffers associated with the given window.
 */
void XmbufDestroyBuffers (dpy, window)
    Display *dpy;
    Window  window;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufDestroyImageBuffersReq *req;
    int i;

    if (!XextHasExtension(info)) return;

    LockDisplay (dpy);
    GetReq (MbufDestroyImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufDestroyImageBuffers;
    req->window = window;
    UnlockDisplay (dpy);
    SyncHandle ();
}


/*
 * XmbufDisplayBuffers - 
 * 	Displays the indicated buffers their appropriate windows within
 * 	max_delay milliseconds after min_delay milliseconds have passed.
 * 	No two buffers may be associated with the same window or else a Matc
 * 	error is generated.
 */
void XmbufDisplayBuffers (dpy, count, buffers, min_delay, max_delay)
    Display *dpy;
    int count;
    Multibuffer *buffers;
    int min_delay, max_delay;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufDisplayImageBuffersReq *req;
    int i;

    if (!XextHasExtension(info)) return;

    LockDisplay (dpy);
    GetReq (MbufDisplayImageBuffers, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufDisplayImageBuffers;
    req->minDelay = min_delay;
    req->maxDelay = max_delay;
    req->length += count;
    PackData32 (dpy, buffers, count * sizeof (Buffer));
    UnlockDisplay (dpy);
    SyncHandle();
}

/*
 * XmbufGetWindowAttributes - 
 * 	Gets the multibuffering attributes that apply to all buffers associated
 * 	with the given window.  Returns non-zero on success and zero if an
 * 	error occurs.
 */
Status XmbufGetWindowAttributes (dpy, w, attr)
    Display *dpy;
    Window w;
    XmbufWindowAttributes *attr;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufGetMultiBufferAttributesReq *req;
    xMbufGetMultiBufferAttributesReply rep;

    if (!XextHasExtension(info)) return 0;

    LockDisplay (dpy);
    GetReq (MbufGetMultiBufferAttributes, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufGetMultiBufferAttributes;
    req->window = w;
    if (!_XReply (dpy, (xReply *) &rep, 
		  (SIZEOF(xMbufGetMultiBufferAttributesReply) -
		   SIZEOF(xReply)) >> 2, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    attr->displayed_index = rep.displayedBuffer;
    attr->update_action = rep.updateAction;
    attr->update_hint = rep.updateHint;
    attr->window_mode = rep.windowMode;

    UnlockDisplay (dpy);
    SyncHandle();
    return 1;
}


/*
 * XmbufChangeWindowAttributes - 
 * 	Sets the multibuffering attributes that apply to all buffers associated
 * 	with the given window.  This is currently limited to the update_hint.
 */
void XmbufChangeWindowAttributes (dpy, w, valuemask, attributes)
    Display *dpy;
    Window w;
    unsigned long valuemask;
    XmbufSetWindowAttributes *attributes;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufSetMultiBufferAttributesReq *req;
    xMbufSetMultiBufferAttributesReply rep;

    if (!XextHasExtension(info)) return 0;

    LockDisplay (dpy);
    GetReq (MbufSetMultiBufferAttributes, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufSetMultiBufferAttributes;
    req->window = w;
    if (req->valueMask = valuemask) {	/* stolen from lib/X/XWindow.c */
	unsigned long values[1];	/* one per element in if stmts below */
	unsigned long *v = values;
	unsigned int nvalues;

	if (valuemask & MultibufWindowUpdateHint) 
	  *v++ = attr->update_hint;
	req->length += (nvalues = v - values);
	nvalues <<= 2;			/* watch out for macros... */
	Data32 (dpy, (long *) values, (long)nvalues);
    }
    UnlockDisplay (dpy);
    SyncHandle();
    return;
}


/*
 * XmbufGetBufferAttributes - 
 * 	Gets the attributes for the indicated buffer.  Returns non-zero on
 * 	success and zero if an error occurs.
 */
Status XmbufGetBufferAttributes (dpy, b, attr)
    Display *dpy;
    Buffer b;
    XmbufBufferAttributes *attr;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufGetBufferAttributesReq *req;
    xMbufGetBufferAttributesReply rep;

    if (!XextHasExtension(info)) return 0;

    LockDisplay (dpy);
    GetReq (MbufGetBufferAttributes, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufGetBufferAttributes;
    req->buffer = b;
    if (!_XReply (dpy, (xReply *) &rep, 
		  (SIZEOF(xMbufGetBufferAttributesReply) -
		   SIZEOF(xReply)) >> 2, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    attr->window = rep.window;
    attr->event_mask = rep.eventMask;
    attr->buffer_index = rep.bufferIndex;
    attr->side = rep.side;

    UnlockDisplay (dpy);
    SyncHandle();
    return 1;
}


/*
 * XmbufChangeBufferAttributes - 
 * 	Sets the attributes for the indicated buffer.  This is currently
 * 	limited to the event_mask.
 */
void XmbufChangeBufferAttributes (dpy, b, valuemask, attr)
    Display *dpy;
    Multibuffer b;
    unsigned long valuemask;
    XmbufSetBufferAttributes *attr;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufSetBufferAttributesReq *req;
    xMbufSetBufferAttributesReply rep;

    if (!XextHasExtension(info)) return 0;

    LockDisplay (dpy);
    GetReq (MbufSetBufferAttributes, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufSetBufferAttributes;
    req->buffer = b;
    if (req->valueMask = valuemask) {	/* stolen from lib/X/XWindow.c */
	unsigned long values[1];	/* one per element in if stmts below */
	unsigned long *v = values;
	unsigned int nvalues;

	if (valuemask & MultibufBufferEventMask)
	  *v++ = attr->event_mask;
	req->length += (nvalues = v - values);
	nvalues <<= 2;			/* watch out for macros... */
	Data32 (dpy, (long *) values, (long)nvalues);
    }
    UnlockDisplay (dpy);
    SyncHandle();
    return;
}


/*
 * XmbufGetScreenInfo - 
 * 	Gets the parameters controlling how mono and stereo windows may be
 * 	created on the indicated screen.  The numbers of sets of visual and 
 * 	depths are returned in nmono_return and nstereo_return.  If 
 * 	nmono_return is greater than zero, then mono_info_return is set to
 * 	the address of an array of XmbufBufferInfo structures describing the
 * 	various visuals and depths that may be used.  Otherwise,
 * 	mono_info_return is set to NULL.  Similarly, stereo_info_return is
 * 	set according to nstereo_return.  The storage returned in 
 * 	mono_info_return and stereo_info_return may be released by XFree.
 * 	If no errors are encounted, non-zero will be returned.
 */
Status XmbufGetScreenInfo (dpy, d, nmono_return, mono_info_return,
			   nstereo, stereo_info_return)
    Display *dpy;
    Drawable d;
    int *nmono_return;
    XmbufBufferInfo **mono_info_return;
    int *nstereo_return;
    XmbufBufferInfo **stereo_info_return;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xMbufGetBufferInfoReq *req;
    xMbufGetBufferInfoReply rep;
    int nmono, nstereo;
    XmbufBufferInfo *minfo, *sinfo;
    static XmbufBufferInfo *read_buffer_info ();


    if (!XextHasExtension(info)) return 0;

    LockDisplay (dpy);
    GetReq (MbufGetBufferInfo, req);
    req->reqType = info->codes->major_opcode;
    req->bufferReqType = X_MbufGetBufferInfo;
    req->drawable = d;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
    }
    nmono = rep.normalInfo;
    nstereo = rep.stereoInfo;
    minfo = ((nmono > 0) ? read_buffer_info (dpy, nmono) : NULL);
    sinfo = ((nstereo > 0) ? read_buffer_info (dpy, nstereo) : NULL);

    /* check for bad reads indicating we need to return an error */
    if ((nmono > 0 && !minfo) || (nstereo > 0 && !sinfo)) {
	if (minfo) Xfree ((char *) minfo);
	if (sinfo) Xfree ((char *) sinfo);
	UnlockDisplay (dpy);
	SyncHandle();
	return 0;
    }

    *nmono_return = nmono;
    *mono_info_return = minfo;
    *nstereo_return = nstereo;
    *stereo_info_return = sinfo;

    UnlockDisplay (dpy);
    SyncHandle();
    return 1;
}


/*
 * XmbufCreateStereoWindow - 
 * 	Creates a stereo window in the same way that XCreateWindow creates
 * 	a mono window.
 */
void XmbufCreateStereoWindow (dpy, parent, x, y, width, height, border_width,
			      depth, class, visual, valuemask, attributes)
    Display *dpy;
    Window parent;
    int x, y;
    unsigned int width, height, border_width;
    int depth;
    unsigned int class;
    Visual *visual;
    unsigned long valuemask;
    XSetWindowAttributes *attributes;
{
}



/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/


#define TALLOC(type,count) ((type *) Xmalloc ((unsigned) count * sizeof(type)))


/*
 * read_buffer_info - read Buffer Info descriptors from the net; if unable
 * to allocate memory, read junk to make sure that stream is clear.
 */
static XmbufBufferInfo *read_buffer_info (dpy, nbufs)
    Display *dpy;
    int nbufs;
{
    xMbufBufferInfo *netbuf = TALLOC (xMbufBufferInfo, nbufs);
    XmbufBufferInfo *bufinfo = NULL;
    long netbytes = nbufs * SIZEOF(xMbufBufferInfo);

    if (netbuf) {
	_XRead32 (dpy, (long *) netbuf, netbytes);

	bufinfo = TALLOC (XmbufBufferInfo, nbufs);
	if (bufinfo) {
	    register XmbufBufferInfo *c = bufinfo;
	    register xMbufBufferInfo *net = netbuf;
	    register int i;

	    for (i = 0, c = bufinfo, net = netbuf; i < nbufs;
		 i++, c++, net++) {
		c->visualid = net->visualID;
		c->max_buffers = net->maxBuffers;
		c->depth = net->depth;
	    }
	}
	Xfree ((char *) netbuf);
    } else {				/* eat the data */
	while (netbytes > 0) {
	    char dummy[256];		/* stack size vs loops tradeoff */
	    long nbytes = sizeof dummy;

	    if (nbytes > netbytes) nbytes = netbytes;
	    _XRead (dpy, dummy, nbytes);
	    netbytes -= nbytes;
	}
    }

    return bufinfo;
}
