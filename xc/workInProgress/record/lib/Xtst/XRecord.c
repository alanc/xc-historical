/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 * 
 * $XConsortium: XRecord.c,v 1.1 94/01/29 17:44:56 rws Exp $
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
 *		    X11R6               			             *
 *                                                                           *
 *****************************************************************************/

typedef struct _CGState {
    unsigned long attr_seq; 
    XRecordEnableCGReply *attr;
} _XCGState;

static Bool
_XCGHandler(dpy, rep, buf, len, data)
    register Display *dpy;
    register xReply *rep;
    char *buf;
    int len;
    XPointer data;
{
    register _XCGState 			*state;
    xRecordEnableConfigReply 		replbuf;
    register xRecordEnableConfigReply 	*repl;
    register XRecordEnableCGReply 	*attr;

    state = (_XCGState *)data;
    if (dpy->last_request_read != state->attr_seq) { 
	return False;
    }
    if (rep->generic.type == X_Error) {
	state->attr = (XRecordEnableCGReply *)NULL;
	return False;
    }
    repl = (xRecordEnableConfigReply *)
	_XGetAsyncReply(dpy, (char *)&replbuf, rep, buf, len,
		     (SIZEOF(xRecordEnableConfigReply) - SIZEOF(xReply)) >> 2,
			True); 
    attr = state->attr; 
    attr->replies_following_hint = repl->nReplies;
    attr->id_base = repl->id_base;
    attr->client_seq = repl->client_seq;
    attr->client_swapped = repl->client_swapped; 
    attr->direction = repl->direction;
    memcpy((char *)&attr->data, (char *)&repl->data, sizeof(XRecordDatum)); 
    return True;
}

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
    register xRecordQueryVersionReply rep;

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
XRecordCreateCG(dpy, record_flags)
    Display 		*dpy;
    XRecordFlags  	*record_flags;
{
    XExtDisplayInfo *info = find_display (dpy);
    xRecordCreateConfigReq 	*req; 
 
    XRecordCheckExtension (dpy, info, None);
    LockDisplay(dpy);
    GetReq(RecordCreateConfig, req); 
    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordCreateConfig;   
    req->cid =  XAllocID(dpy); 

    if(record_flags == (XRecordFlags *)NULL)
    { 
         bzero((char *)&(req->record_flags), SIZEOF(XRecordFlags));           
    } 
    else 
    {  
 	memcpy((char *)&(req->record_flags), (char *)record_flags, 
	SIZEOF(XRecordFlags)); 
    } 
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
    memcpy((char *)&(req->record_flags), (char *)record_flags, 
	SIZEOF(XRecordFlags));
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
    register xRecordGetConfigReply 	rep;

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
    bzero((char *)ret, sizeof(XRecordState) );
    memcpy((char *)ret, (char *)&rep.record_state, sizeof(XRecordState)); 
    UnlockDisplay(dpy);
    SyncHandle();    
    return True;
}

Status
XRecordEnableCG(dpy, config, enable, attr)
    Display 		*dpy;
    XRecordConfig 	config;
    Bool 		enable;
    XRecordEnableCGReply *attr;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xRecordEnableConfigReq   	*req;
    register xRecordEnableConfigReply 	rep; 
    Status 				status;
    _XAsyncHandler 			async;
    _XCGState 				async_state;

    XRecordCheckExtension (dpy, info, 0);
    LockDisplay(dpy);
    GetReq(RecordEnableConfig, req);

    req->reqType = info->codes->major_opcode;
    req->minor_opcode = X_RecordEnableConfig;    
    req->cid = config;
    req->enable = enable;

    async_state.attr_seq = dpy->request; 
    async_state.attr = attr;
    async.next = dpy->async_handlers;
    async.handler = _XCGHandler;  /*_XAsyncErrorHandler; */
    async.data = (XPointer)&async_state;
    dpy->async_handlers = &async;

    if (!_XReply (dpy, (xReply *)&rep, 0, xTrue) && rep.nReplies == 0) 
    {
	DeqAsyncHandler(dpy, &async);
	UnlockDisplay(dpy);
	SyncHandle();
	return (0);
    }
    DeqAsyncHandler(dpy, &async);
    if (!async_state.attr) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (0);
    } 
    attr->replies_following_hint = rep.nReplies;
    attr->id_base = rep.id_base;
    attr->client_seq = rep.client_seq;
    attr->client_swapped = rep.client_swapped; 
    attr->direction = rep.direction; 
    attr->data = rep.data; 
   
    UnlockDisplay(dpy);
    SyncHandle();     
    return(1);
}
  
