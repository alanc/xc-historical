/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: XSync.c,v $
 * Revision 1.1.1.2  1993/05/05  18:00:16  Pete_Snider
 * 	initial rcs submit
 *
 *
 * $EndLog$
 */

/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
the Massachusetts Institute of Technology, Cambridge, Massachusetts,
and Olivetti Research Limited, Cambridge, England.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital, Olivetti or MIT
not be used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND OLIVETTI DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#ifdef SYNC

#include <stdio.h> 
#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "Xext.h"
#include "extutil.h"
#include "syncstr.h"

static XExtensionInfo _sync_info_data;
static XExtensionInfo *sync_info = &_sync_info_data;
static char *sync_extension_name = SYNCNAME;

#define SyncCheckExtension(dpy,i,val) \
		XextCheckExtension(dpy, i, sync_extension_name, val)
#define SyncSimpleCheckExtension(dpy,i) \
		XextSimpleCheckExtension(dpy, i, sync_extension_name)

static int close_display();
static Bool wire_to_event();
static Status event_to_wire();
static char *error_string();

static XExtensionHooks sync_extension_hooks = {
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

static char *sync_error_list[] = {
    "BadCounter",
    "BadAlarm",
};

static XEXT_GENERATE_FIND_DISPLAY(find_display, sync_info,
				   sync_extension_name, 
				   &sync_extension_hooks,
				   SyncNumberEvents, (caddr_t)NULL)

static XEXT_GENERATE_CLOSE_DISPLAY(close_display, sync_info)


static XEXT_GENERATE_ERROR_STRING (error_string, sync_extension_name,
				   SyncNumberErrors, sync_error_list)


static Bool wire_to_event(dpy, event, wire)
Display *dpy;
XEvent *event;
xEvent *wire;
{
    XExtDisplayInfo *info = find_display(dpy);
    XCounterNotifyEvent *aevent;
    xCounterNotifyEvent *awire;
    XAlarmNotifyEvent *anl;
    xAlarmNotifyEvent *ane;

    SyncCheckExtension(dpy, info, False);

    switch ((wire->u.u.type & 0x7F) - info->codes->first_event) {
    case CounterNotify:
	awire = (xCounterNotifyEvent *)wire;
	aevent = (XCounterNotifyEvent *)event;
	aevent->type = awire->type & 0x7F;
	aevent->serial = _XSetLastRequestRead(dpy,
					      (xGenericReply *)wire);
	aevent->send_event = (awire->type & 0x80) != 0;
	aevent->display = dpy;
	aevent->counter = awire->counter;
	cvtINT32sToC64(aevent->wait_value, awire->wait_value_lo, awire->wait_value_hi);
	cvtINT32sToC64(aevent->counter_value, awire->counter_value_lo, awire->counter_value_hi);
	aevent->time = awire->time;
	return True;

    case AlarmNotify:
	ane = (xAlarmNotifyEvent *)wire; /* ENCODING EVENT PTR */
	anl = (XAlarmNotifyEvent *)event; /* LIBRARY EVENT PTR */
	anl->type = ane->type & 0x7F;
	anl->serial = _XSetLastRequestRead(dpy,
					   (xGenericReply *)wire);
	anl->send_event = (ane->type & 0x80) != 0;
	anl->display = dpy;
	anl->alarm = ane->alarm;
	cvtINT32sToC64(anl->counter_value, ane->counter_value_lo, ane->counter_value_hi);
	cvtINT32sToC64(anl->alarm_value, ane->alarm_value_lo, ane->alarm_value_hi);
	anl->state = ane->state;
	anl->time = ane->time;
	return True;
    }

    return False;
}

static Status event_to_wire(dpy, event, wire)
Display *dpy;
XEvent *event;
xEvent *wire;
{
    XExtDisplayInfo *info = find_display(dpy);
    XCounterNotifyEvent *aevent;
    xCounterNotifyEvent *awire;
    XAlarmNotifyEvent *anl;
    xAlarmNotifyEvent *ane;

    SyncCheckExtension(dpy, info, False);

    switch ((event->type & 0x7F) - info->codes->first_event) {
    case CounterNotify:
	awire = (xCounterNotifyEvent *)wire;
	aevent = (XCounterNotifyEvent *)event;
	awire->type = aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->sequenceNumber = aevent->serial & 0xFFFF;
	awire->counter = aevent->counter;
	awire->wait_value_lo = C64Low32(aevent->wait_value);
	awire->wait_value_hi = C64High32(aevent->wait_value);
	awire->counter_value_lo = C64Low32(aevent->counter_value);
	awire->counter_value_hi = C64High32(aevent->counter_value);
	awire->time = aevent->time;
	return True;

    case AlarmNotify:
	ane = (xAlarmNotifyEvent *)wire; /* ENCODING EVENT PTR */
	anl = (XAlarmNotifyEvent *)event; /* LIBRARY EVENT PTR */
	ane->type = anl->type | (anl->send_event ? 0x80 : 0);
	ane->sequenceNumber = anl->serial & 0xFFFF;
	ane->alarm = anl->alarm;
	ane->counter_value_lo = C64Low32(anl->counter_value);
	ane->counter_value_hi = C64High32(anl->counter_value);
	ane->alarm_value_lo = C64Low32(anl->alarm_value);
	ane->alarm_value_hi = C64High32(anl->alarm_value);
	ane->state = anl->state;
	ane->time = anl->time;
	return True;
    }
    return False;
}

Bool XSyncQueryExtension(dpy, event_base_return, error_base_return)
Display *dpy;
unsigned long *event_base_return, *error_base_return;
{
    XExtDisplayInfo *info = find_display(dpy);

    if (XextHasExtension(info)) {
	*event_base_return = info->codes->first_event;
	*error_base_return = info->codes->first_error;
	return True;
    } else 
	return False;
}

Status XSyncInfo(dpy, major_return, minor_return,
		 counters_return, n_counters_return)
Display *dpy;
int *major_return, *minor_return;
SystemCounterRec **counters_return;
unsigned long *n_counters_return;
{
    xSyncInfoReply rep;
    xSyncInfoReq  *req;
    SystemCounterList list;
    int  len, n;
    int *raw;
    XExtDisplayInfo *info = find_display(dpy);

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncInfo, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncInfo;
    if (!_XReply(dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    if (major_return) *major_return = rep.majorVersion;
    if (minor_return) *minor_return = rep.minorVersion;

    *n_counters_return = rep.nCounters;
    if (rep.nCounters>0) {
	unsigned nbytes = rep.nCounters*(sizeof(SystemCounterRec)+sizeof(int));
/*	unsigned nbytes = rep.length + offset; */

	if (list = (SystemCounterList)Xmalloc(nbytes)) {
	    if(!(raw = (int *)Xmalloc(nbytes)))
		return BadAlloc;

	    _XRead(dpy, (char *)raw, rep.length<<2);

	    for(n = 0; n < rep.nCounters; n++){
		list[n].counter = (CARD32)*raw++;
		cvtINT32sToC64(list[n].resolution, 
			       (CARD32)raw[0], (CARD32)raw[1]);
		raw += 2;
		len = (CARD32)*raw++;
		if(!(list[n].name = Xmalloc(len)))
		    return BadAlloc;
		strcpy(list[n].name, raw);
		raw += len;
	    }
	    *counters_return = list;

	} else {
	    _XEatData(dpy, nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return 0;
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Counter XSyncCreateCounter(dpy, counter_return, initial_value)
Display *dpy;
Counter *counter_return;
CARD64 initial_value;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncCreateCounterReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncCreateCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncCreateCounter;

    req->cid = *counter_return = XAllocID(dpy);
    req->initial_value_lo = C64Low32(initial_value);
    req->initial_value_hi = C64High32(initial_value);

    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status XSyncSetCounter(dpy, counter, value)
Display *dpy;
Counter counter;
CARD64 value;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncSetCounterReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncSetCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncSetCounter;
    req->cid = counter;
    req->value_lo = C64Low32(value);
    req->value_hi = C64High32(value);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status XSyncChangeCounter(dpy, counter, value)
Display *dpy;
Counter counter;
CARD64 value;
{
  XExtDisplayInfo *info = find_display(dpy);
    xSyncChangeCounterReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncChangeCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncChangeCounter;
    req->cid = counter;
    req->value_lo = C64Low32(value);
    req->value_hi = C64High32(value);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status XSyncDestroyCounter(dpy, counter)
Display *dpy;
Counter counter;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncDestroyCounterReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncDestroyCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncDestroyCounter;
    req->counter = counter;
    UnlockDisplay(dpy);
    SyncHandle();

    return 1;
}

Status XSyncQueryCounter(dpy, counter, value_return)
Display *dpy;
Counter counter;
CARD64 *value_return;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncQueryCounterReply rep;
    xSyncQueryCounterReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncQueryCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncQueryCounter;
    req->counter = counter;
    if (!_XReply(dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    cvtINT32sToC64(*value_return, rep.value_lo, rep.value_hi);
    UnlockDisplay(dpy);
    SyncHandle();

    return 1;
}


Status XSyncAwait(dpy, wait_list, n_conditions)
Display *dpy;
XWaitCondition *wait_list;
unsigned long n_conditions; 
{
    XExtDisplayInfo *info = find_display(dpy);
    XWaitCondition *wait_item = wait_list;
    xSyncAwaitReq *req;
    unsigned int *values, *value;
    unsigned nbytes, nvalues;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncAwait, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncAwait;
    nbytes = n_conditions*sizeof(XWaitCondition);

    if (values = value = (unsigned int *)Xmalloc(nbytes*8)) {
	while(n_conditions--) {
	    *value++ = wait_item->counter;
	    *value++ = wait_item->wait_type;
	    *value++ = C64Low32(wait_item->wait_value);
	    *value++ = C64High32(wait_item->wait_value);
	    *value++ = wait_item->test_type;
	    wait_item++; /* get next trigger */
	}
	nvalues = value - values;
	req->length += nvalues;
	Data(dpy, (char *)values, nvalues<<2);
    } else {
fprintf(stderr,"Malloc error goes here -PDS \n");
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    Xfree((char *)values);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Alarm XSyncCreateAlarm(dpy, alarm_return, values_mask, values)
Display *dpy;
Alarm *alarm_return;
unsigned long values_mask;
XSyncAlarmAttributes *values;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncCreateAlarmReq *req;
    Alarm aid;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncCreateAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncCreateAlarm;
    req->id = aid = XAllocID(dpy);
    values_mask &= CACounter | CAValueType | CAValue | CATestType |
	           CADelta | CAEvents;
    if(req->valueMask = values_mask)
        _XProcessAlarmAttributes (dpy,(xSyncChangeAlarmReq *)req, 
				  values_mask, values);
    UnlockDisplay(dpy);
    SyncHandle();
    *alarm_return = aid;
    return 1;
}

_XProcessAlarmAttributes(dpy, req, valuemask, attributes)
Display *dpy;
xSyncChangeAlarmReq *req;
unsigned long valuemask;
XSyncAlarmAttributes *attributes;
{

    CARD32 values[32];
    CARD32 *value = values;
    unsigned int nvalues;

    if (valuemask & CACounter)
	*value++ = attributes->trigger.counter;

    if (valuemask & CAValueType)
    	*value++ = attributes->trigger.wait_type;

    if (valuemask & CAValue) {
	*value++ = C64Low32(attributes->trigger.wait_value);
	*value++ = C64High32(attributes->trigger.wait_value);
    }

    if (valuemask & CATestType) 
	*value++ = attributes->trigger.test_type;

    if (valuemask & CADelta){
	*value++ = C64Low32(attributes->delta);
	*value++ = C64High32(attributes->delta);
    }

    if (valuemask & CAEvents)
	*value++ = attributes->events;

    /* N.B. the 'state' field cannot be set or changed */   
    req->length += (nvalues = value - values);
    nvalues <<= 2;                          /* watch out for macros... */

    Data32 (dpy, (long *) values, (long)nvalues);
}

Status XSyncDestroyAlarm(dpy, alarm)
Display *dpy;
Alarm alarm;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncDestroyAlarmReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncDestroyAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncDestroyAlarm;
    req->alarm = alarm;
    UnlockDisplay(dpy);
    SyncHandle();

    return 1;
}

Status XSyncQueryAlarm(dpy, alarm, values_return)
Display *dpy;
Alarm alarm;
XSyncAlarmAttributes* values_return;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncQueryAlarmReq *req;
    xSyncQueryAlarmReply rep;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncQueryAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncQueryAlarm;
    req->alarm = alarm;

    if (!(_XReply(dpy, (xReply *)&rep,
	  ((SIZEOF(xSyncQueryAlarmReply) - SIZEOF(xGenericReply)) >> 2) , xFalse))) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }

    values_return->trigger.counter = rep.counter;
    values_return->trigger.wait_type = rep.wait_type;
    cvtINT32sToC64(values_return->trigger.wait_value, rep.wait_value_lo, rep.wait_value_hi);
    values_return->trigger.test_type = rep.test_type;
    cvtINT32sToC64(values_return->delta, rep.delta_lo, rep.delta_hi);
    values_return->events = rep.events;
    values_return->state = rep.state;
    UnlockDisplay(dpy);
    SyncHandle();

    return 1;
}

Status XSyncChangeAlarm(dpy, alarm, values_mask, values)
Display *dpy;
Alarm alarm;
unsigned long values_mask;
XSyncAlarmAttributes *values;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncChangeAlarmReq *req;
    Alarm aid;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncChangeAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncChangeAlarm;
    req->alarm = alarm;
    values_mask &= CACounter | CAValueType | CAValue | CATestType |
	           CADelta | CAEvents;
    if (req->valueMask = values_mask) 
	_XProcessAlarmAttributes (dpy, req, values_mask, values);
    UnlockDisplay(dpy);
    SyncHandle();

    return 1;
}

Status XSyncSetPriority(dpy, priority)
Display *dpy;
int	priority;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncSetPriorityReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncSetPriority, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncSetPriority;
    req->priority = priority;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status XSyncGetPriority(dpy, return_priority)
Display *dpy;
int	*return_priority;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncGetPriorityReply rep;
    xSyncGetPriorityReq  *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncGetPriority, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncGetPriority;

    if (!_XReply(dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return 0;
    }
    if(return_priority) *return_priority= rep.priority;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}


#endif /* SYNC */
