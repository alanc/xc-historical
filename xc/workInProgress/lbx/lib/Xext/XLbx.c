/*
 * $NCDOr$
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */
#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "XLbx.h"
#include "lbxstr.h"
#include "Xext.h"
#include "extutil.h"

static XExtensionInfo _lbx_info_data;
static XExtensionInfo *lbx_info = &_lbx_info_data;
static /* const */ char *lbx_extension_name = LBXNAME;

#define LbxCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, lbx_extension_name, val)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display();
static char *error_string();
static Bool wire_to_event();
static Status event_to_wire();
static /* const */ XExtensionHooks lbx_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    wire_to_event,			/* wire_to_event */
    event_to_wire,			/* event_to_wire */
    NULL,				/* error */
    error_string,			/* error_string */
};

static /* const */ char *lbx_error_list[] = {
    "BadLbxClient",			/* BadLbxClient */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, lbx_info, lbx_extension_name, 
				   &lbx_extension_hooks, LbxNumberEvents, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, lbx_info)

static XEXT_GENERATE_ERROR_STRING (error_string, lbx_extension_name,
				   LbxNumberErrors, lbx_error_list)


static Bool wire_to_event (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtDisplayInfo *info = find_display (dpy);
    XLbxEvent	*se;
    xLbxEvent	*sevent;

    LbxCheckExtension (dpy, info, False);

    switch ((event->u.u.type & 0x7f) - info->codes->first_event) {
    case LbxEvent:
	se = (XLbxEvent *) re;
	sevent = (xLbxEvent *) event;
	se->type = sevent->type & 0x7f;
	se->serial = _XSetLastRequestRead(dpy,(xGenericReply *) event);
	se->send_event = (sevent->type & 0x80) != 0;
	se->display = dpy;
	se->client = sevent->client;
	se->lbxType = sevent->lbxType;
    	return True;
    }
    return False;
}

static Status event_to_wire (dpy, re, event)
    Display *dpy;
    XEvent  *re;
    xEvent  *event;
{
    XExtDisplayInfo *info = find_display (dpy);
    XLbxEvent	*se;
    xLbxEvent	*sevent;

    LbxCheckExtension (dpy, info, 0);

    switch ((re->type & 0x7f) - info->codes->first_event) {
    case LbxEvent:
    	se = (XLbxEvent *) re;
	sevent = (xLbxEvent *) event;
    	sevent->type = se->type | (se->send_event ? 0x80 : 0);
    	sevent->sequenceNumber = se->serial & 0xffff;
    	sevent->client = se->client;
	sevent->lbxType = se->lbxType;
    	return True;
    }
    return False;
}

/*****************************************************************************
 *                                                                           *
 *		    public Shared Memory Extension routines                  *
 *                                                                           *
 *****************************************************************************/

Bool XLbxQueryExtension (dpy, requestp, event_basep, error_basep)
    Display *dpy;
    int *requestp, *event_basep, *error_basep;
{
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
	*requestp = info->codes->major_opcode;
	*event_basep = info->codes->first_event;
	*error_basep = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}


int XLbxGetEventBase(dpy)
    Display *dpy;
{
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
	return info->codes->first_event;
    } else {
	return -1;
    }
}


Bool XLbxQueryVersion(dpy, majorVersion, minorVersion)
    Display *dpy;
    int	    *majorVersion, *minorVersion;
{
    XExtDisplayInfo *info = find_display (dpy);
    xLbxQueryVersionReply rep;
    register xLbxQueryVersionReq *req;

    LbxCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(LbxQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->lbxReqType = X_LbxQueryVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    *majorVersion = rep.majorVersion;
    *minorVersion = rep.minorVersion;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/* XXX all other requests will run after Xlib has lost the wire ... */
