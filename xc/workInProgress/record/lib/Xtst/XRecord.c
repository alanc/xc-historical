/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 *
 * $XConsortium: XRecord.c,v 1.3 94/01/30 13:58:38 rws Exp $
 */

#include <stdio.h>
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include "Xext.h"
#include "extutil.h"
#include "recordstr.h"
#include "record.h"

extern unsigned long _XSetLastRequestRead();

static XExtensionInfo _xrecord_info_data;
static XExtensionInfo *xrecord_info = &_xrecord_info_data;
static /* const */ char *xrecord_extension_name = XRecordExtName;

#define XRecordCheckExtension(dpy,i,val) \
    XextCheckExtension(dpy, i, xrecord_extension_name, val)
#define XRecordSimpleCheckExtension(dpy,i) \
    XextSimpleCheckExtension(dpy, i, xrecord_extension_name)

/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display();
static Bool wire_to_event();
static Status event_to_wire();
static char *error_string();
static XExtensionHooks xrecord_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    close_display,                      /* close_display */
    NULL,                      		/* wire_to_event */
    NULL,                      		/* event_to_wire */
    NULL,                               /* error */
    error_string                        /* error_string */
};

static char *xrecord_error_list[] = {
    "BadConfiguration (Invalid configuration)",
};

XEXT_GENERATE_FIND_DISPLAY (find_display, xrecord_info,
	xrecord_extension_name, &xrecord_extension_hooks, XRecordNumErrors, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, xrecord_info)

static XEXT_GENERATE_ERROR_STRING (error_string, xrecord_extension_name,
                                   XRecordNumErrors, xrecord_error_list)

/*****************************************************************************
 *                                                                           *
 *		    public routines               			     *
 *                                                                           *
 *****************************************************************************/

Status
XRecordQueryVersion (dpy, cmajor, cminor, ret)
    Display 	*dpy;
    int 	cmajor, cminor;
    XRecordQueryVersionReply *ret;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordQueryVersionReq   *req;
    xRecordQueryVersionReply rep;

    XRecordCheckExtension (dpy, info, False);

    LockDisplay(dpy);
    GetReq(RecordQueryVersion, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordQueryVersion;
    req->majorVersion = cmajor;
    req->minorVersion = cminor;
    if (!_XReply(dpy,(xReply *)&rep, 0, True)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    ret->majorVersion = rep.majorVersion;
    ret->minorVersion = rep.minorVersion;
    return True;
}

XRecordConfig
XRecordCreateCG(dpy)
    Display 		*dpy;
{
    XExtDisplayInfo *info = find_display (dpy);
    xRecordCreateConfigReq 	*req;

    XRecordCheckExtension (dpy, info, None);
    LockDisplay(dpy);
    GetReq(RecordCreateConfig, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordCreateConfig;
    req->cid =  XAllocID(dpy);

    UnlockDisplay(dpy);
    SyncHandle();
    return req->cid;
}

Status
XRecordFreeCG(dpy, config)
    Display 		*dpy;
    XRecordConfig	config;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordFreeConfigReq 	*req;

    XRecordCheckExtension (dpy, info, 0);
    LockDisplay(dpy);
    GetReq(RecordFreeConfig, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordFreeConfig;
    req->cid = config;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XRecordChangeCG(dpy, config, id_base, record_flags, add)
    Display 		*dpy;
    XRecordConfig	config;
    XID      	        id_base;
    XRecordFlags  	*record_flags;
    Bool		add;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordChangeConfigReq 	*req;

    XRecordCheckExtension (dpy, info, 0);
    LockDisplay(dpy);
    GetReq(RecordChangeConfig, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordChangeConfig;
    req->cid = config;
    req->id_base = id_base;
    req->record_flags = *record_flags;
    req->add = add;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XRecordGetCG(dpy, config, ret)
    Display 		*dpy;
    XRecordConfig	config;
    XRecordState 	*ret;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordGetConfigReq   	*req;
    xRecordGetConfigReply 	rep;

    XRecordCheckExtension (dpy, info, 0);
    LockDisplay(dpy);
    GetReq(RecordGetConfig, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordGetConfig;
    req->cid = config;
    if (!_XReply(dpy,(xReply *)&rep, 0, True)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    ret->enabled = rep.enabled;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XRecordDisableCG(dpy, config)
    Display 		*dpy;
    XRecordConfig 	config;
{
    XExtDisplayInfo *info = find_display (dpy);
    xRecordDisableConfigReq 	*req;

    XRecordCheckExtension (dpy, info, None);
    LockDisplay(dpy);
    GetReq(RecordDisableConfig, req);
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordDisableConfig;
    req->cid =  config;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XRecordEnableCG(dpy, config, func, arg)
    Display 		*dpy;
    XRecordConfig 	config;
    void (*func)(
#if NeedNestedPrototypes
		 Display*		/* display */,
		 XRecordInterceptData*	/* attr */,
		 XPointer		/* arg */
#endif
		 );
    XPointer arg;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordEnableConfigReq   	*req;
    xRecordEnableConfigReply 	rep;
    XRecordInterceptData data;

    XRecordCheckExtension (dpy, info, 0);
    LockDisplay(dpy);
    GetReq(RecordEnableConfig, req);

    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordEnableConfig;
    req->cid = config;

    while (1)
    {
	if (!_XReply (dpy, (xReply *)&rep, 0, xFalse))
	{
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (0);
	}
	if (!rep.nReplies)
	    break;
	data.id_base = rep.id_base;
	data.direction = rep.direction;
	data.client_swapped = rep.client_swapped;
	data.client_seq = rep.client_seq;
	data.data = (XRecordDatum *)Xmalloc(rep.length << 2);
	_XRead (dpy, (char *)data.data, (long) rep.length << 2);
	(*func)(dpy, &data, arg);
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return(1);
}
