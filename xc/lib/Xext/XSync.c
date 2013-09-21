/* $XConsortium: XSync.c,v 1.9 95/06/03 13:35:06 gildea Exp $ */
/*

Copyright (c) 1991, 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/***********************************************************
Copyright 1991,1993 by Digital Equipment Corporation, Maynard, Massachusetts,
and Olivetti Research Limited, Cambridge, England.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or Olivetti
not be used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL AND OLIVETTI DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include <stdio.h>
#define NEED_EVENTS
#define NEED_REPLIES
#include "Xlibint.h"
#include "Xext.h"
#include "extutil.h"
#include "syncstr.h"

static XExtensionInfo _sync_info_data;
static XExtensionInfo *sync_info = &_sync_info_data;
static char    *sync_extension_name = SYNC_NAME;

#define SyncCheckExtension(dpy,i,val) \
		XextCheckExtension(dpy, i, sync_extension_name, val)
#define SyncSimpleCheckExtension(dpy,i) \
		XextSimpleCheckExtension(dpy, i, sync_extension_name)

static int      close_display();
static Bool     wire_to_event();
static Status   event_to_wire();
static char    *error_string();

static XExtensionHooks sync_extension_hooks = {
    NULL,			/* create_gc */
    NULL,			/* copy_gc */
    NULL,			/* flush_gc */
    NULL,			/* free_gc */
    NULL,			/* create_font */
    NULL,			/* free_font */
    close_display,		/* close_display */
    wire_to_event,		/* wire_to_event */
    event_to_wire,		/* event_to_wire */
    NULL,			/* error */
    error_string,		/* error_string */
};

static char    *sync_error_list[] = {
    "BadCounter",
    "BadAlarm",
};

static 
XEXT_GENERATE_FIND_DISPLAY(find_display, sync_info,
			   sync_extension_name,
			   &sync_extension_hooks,
			   XSyncNumberEvents, (XPointer) NULL)

static 
XEXT_GENERATE_CLOSE_DISPLAY(close_display, sync_info)

static 
XEXT_GENERATE_ERROR_STRING(error_string, sync_extension_name,
			   XSyncNumberErrors, sync_error_list)


static Bool     
wire_to_event(dpy, event, wire)
    Display        *dpy;
    XEvent         *event;
    xEvent         *wire;
{
    XExtDisplayInfo *info = find_display(dpy);
    XSyncCounterNotifyEvent *aevent;
    xSyncCounterNotifyEvent *awire;
    XSyncAlarmNotifyEvent *anl;
    xSyncAlarmNotifyEvent *ane;

    SyncCheckExtension(dpy, info, False);

    switch ((wire->u.u.type & 0x7F) - info->codes->first_event)
    {
      case XSyncCounterNotify:
	awire = (xSyncCounterNotifyEvent *) wire;
	aevent = (XSyncCounterNotifyEvent *) event;
	aevent->type = awire->type & 0x7F;
	aevent->serial = _XSetLastRequestRead(dpy,
					      (xGenericReply *) wire);
	aevent->send_event = (awire->type & 0x80) != 0;
	aevent->display = dpy;
	aevent->counter = awire->counter;
	XSyncIntsToValue(&aevent->wait_value, awire->wait_value_lo,
				    awire->wait_value_hi);
	XSyncIntsToValue(&aevent->counter_value,
				    awire->counter_value_lo,
				    awire->counter_value_hi);
	aevent->time = awire->time;
	aevent->count = awire->count;
	aevent->destroyed = awire->destroyed;
	return True;

      case XSyncAlarmNotify:
	ane = (xSyncAlarmNotifyEvent *) wire;	/* ENCODING EVENT PTR */
	anl = (XSyncAlarmNotifyEvent *) event;	/* LIBRARY EVENT PTR */
	anl->type = ane->type & 0x7F;
	anl->serial = _XSetLastRequestRead(dpy,
					   (xGenericReply *) wire);
	anl->send_event = (ane->type & 0x80) != 0;
	anl->display = dpy;
	anl->alarm = ane->alarm;
	XSyncIntsToValue(&anl->counter_value,
				    ane->counter_value_lo,
				    ane->counter_value_hi);
	XSyncIntsToValue(&anl->alarm_value,
				    ane->alarm_value_lo,
				    ane->alarm_value_hi);
	anl->state = ane->state;
	anl->time = ane->time;
	return True;
    }

    return False;
}

static Status 
event_to_wire(dpy, event, wire)
    Display        *dpy;
    XEvent         *event;
    xEvent         *wire;
{
    XExtDisplayInfo *info = find_display(dpy);
    XSyncCounterNotifyEvent *aevent;
    xSyncCounterNotifyEvent *awire;
    XSyncAlarmNotifyEvent *anl;
    xSyncAlarmNotifyEvent *ane;

    SyncCheckExtension(dpy, info, False);

    switch ((event->type & 0x7F) - info->codes->first_event)
    {
      case XSyncCounterNotify:
	awire = (xSyncCounterNotifyEvent *) wire;
	aevent = (XSyncCounterNotifyEvent *) event;
	awire->type = aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->sequenceNumber = aevent->serial & 0xFFFF;
	awire->counter = aevent->counter;
	awire->wait_value_lo = XSyncValueLow32(aevent->wait_value);
	awire->wait_value_hi = XSyncValueHigh32(aevent->wait_value);
	awire->counter_value_lo = XSyncValueLow32(aevent->counter_value);
	awire->counter_value_hi = XSyncValueHigh32(aevent->counter_value);
	awire->time = aevent->time;
	awire->count = aevent->count;
	awire->destroyed = aevent->destroyed;
	return True;

      case XSyncAlarmNotify:
	ane = (xSyncAlarmNotifyEvent *) wire;	/* ENCODING EVENT PTR */
	anl = (XSyncAlarmNotifyEvent *) event;	/* LIBRARY EVENT PTR */
	ane->type = anl->type | (anl->send_event ? 0x80 : 0);
	ane->sequenceNumber = anl->serial & 0xFFFF;
	ane->alarm = anl->alarm;
	ane->counter_value_lo = XSyncValueLow32(anl->counter_value);
	ane->counter_value_hi = XSyncValueHigh32(anl->counter_value);
	ane->alarm_value_lo = XSyncValueLow32(anl->alarm_value);
	ane->alarm_value_hi = XSyncValueHigh32(anl->alarm_value);
	ane->state = anl->state;
	ane->time = anl->time;
	return True;
    }
    return False;
}

Status
XSyncQueryExtension(dpy, event_base_return, error_base_return)
    Display        *dpy;
    int  *event_base_return, *error_base_return;
{
    XExtDisplayInfo *info = find_display(dpy);

    if (XextHasExtension(info))
    {
	*event_base_return = info->codes->first_event;
	*error_base_return = info->codes->first_error;
	return True;
    }
    else
	return False;
}

Status
XSyncInitialize(dpy, major_version_return, minor_version_return)
    Display        *dpy;
    int *major_version_return, *minor_version_return;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncInitializeReply rep;
    xSyncInitializeReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncInitialize, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncInitialize;
    req->majorVersion = SYNC_MAJOR_VERSION;
    req->minorVersion = SYNC_MINOR_VERSION;
    if (!_XReply(dpy, (xReply *) & rep, 0, xTrue))
    {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    *major_version_return = rep.majorVersion;
    *minor_version_return = rep.minorVersion;
    return ((rep.majorVersion == SYNC_MAJOR_VERSION)
#if SYNC_MINOR_VERSION > 0	/* avoid compiler warning */
	    && (rep.minorVersion >= SYNC_MINOR_VERSION)
#endif
	    );
}

XSyncSystemCounter *
XSyncListSystemCounters(dpy, n_counters_return)
    Display        *dpy;
    int		   *n_counters_return;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncListSystemCountersReply rep;
    xSyncListSystemCountersReq *req;
    XSyncSystemCounter *list = NULL;

    SyncCheckExtension(dpy, info, NULL);

    LockDisplay(dpy);
    GetReq(SyncListSystemCounters, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncListSystemCounters;
    if (!_XReply(dpy, (xReply *) & rep, 0, xFalse))
	goto bail;

    *n_counters_return = rep.nCounters;
    if (rep.nCounters > 0)
    {
	xSyncSystemCounter *pWireSysCounter, *pNextWireSysCounter;
	XSyncCounter counter;
	int replylen;
	int i;

	list = (XSyncSystemCounter *)Xmalloc(
			rep.nCounters * sizeof(XSyncSystemCounter));
	replylen = rep.length << 2;
	pWireSysCounter = (xSyncSystemCounter *) Xmalloc ((unsigned) replylen + 1);
                /* +1 to leave room for last null-terminator */

	if ((!list) || (!pWireSysCounter))
	{
	    if (list) Xfree((char *) list);
	    if (pWireSysCounter)   Xfree((char *) pWireSysCounter);
	    _XEatData(dpy, (unsigned long) replylen);
	    list = NULL;
	    goto bail;
	}

	_XReadPad(dpy, (char *)pWireSysCounter, replylen);

	counter = pWireSysCounter->counter;
	for (i = 0; i < rep.nCounters; i++)
	{
	    list[i].counter = counter;
	    XSyncIntsToValue(&list[i].resolution,
					pWireSysCounter->resolution_lo,
					pWireSysCounter->resolution_hi);

	    /* we may be about to clobber the counter field of the
	     * next syscounter because we have to add a null terminator
	     * to the counter name string.  So we save the next counter
	     * here.
	     */
	    pNextWireSysCounter = (xSyncSystemCounter *)
		(((char *)pWireSysCounter) + ((SIZEOF(xSyncSystemCounter) + 
				     pWireSysCounter->name_length + 3) & ~3));
	    counter = pNextWireSysCounter->counter;

	    list[i].name = ((char *)pWireSysCounter) + 
						SIZEOF(xSyncSystemCounter);
	    /* null-terminate the string */
	    *(list[i].name + pWireSysCounter->name_length) = '\0';
	    pWireSysCounter = pNextWireSysCounter;
	}
    }

bail:
    UnlockDisplay(dpy);
    SyncHandle();
    return list;
}

void
XSyncFreeSystemCounterList(list)
    XSyncSystemCounter *list;
{
    if (list)
    {
	Xfree( ((char *)list[0].name) - SIZEOF(xSyncSystemCounter));
	Xfree(list);
    }
}


XSyncCounter 
XSyncCreateCounter(dpy, initial_value)
    Display        *dpy;
    XSyncValue    initial_value;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncCreateCounterReq *req;

    SyncCheckExtension(dpy, info, None);

    LockDisplay(dpy);
    GetReq(SyncCreateCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncCreateCounter;

    req->cid = XAllocID(dpy);
    req->initial_value_lo = XSyncValueLow32(initial_value);
    req->initial_value_hi = XSyncValueHigh32(initial_value);

    UnlockDisplay(dpy);
    SyncHandle();
    return req->cid;
}

Status 
XSyncSetCounter(dpy, counter, value)
    Display        *dpy;
    XSyncCounter    counter;
    XSyncValue    value;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncSetCounterReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncSetCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncSetCounter;
    req->cid = counter;
    req->value_lo = XSyncValueLow32(value);
    req->value_hi = XSyncValueHigh32(value);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status 
XSyncChangeCounter(dpy, counter, value)
    Display        *dpy;
    XSyncCounter    counter;
    XSyncValue    value;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncChangeCounterReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncChangeCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncChangeCounter;
    req->cid = counter;
    req->value_lo = XSyncValueLow32(value);
    req->value_hi = XSyncValueHigh32(value);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status 
XSyncDestroyCounter(dpy, counter)
    Display        *dpy;
    XSyncCounter    counter;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncDestroyCounterReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncDestroyCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncDestroyCounter;
    req->counter = counter;
    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}

Status 
XSyncQueryCounter(dpy, counter, value_return)
    Display        *dpy;
    XSyncCounter    counter;
    XSyncValue   *value_return;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncQueryCounterReply rep;
    xSyncQueryCounterReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncQueryCounter, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncQueryCounter;
    req->counter = counter;
    if (!_XReply(dpy, (xReply *) & rep, 0, xTrue))
    {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    XSyncIntsToValue(value_return, rep.value_lo, rep.value_hi);
    UnlockDisplay(dpy);
    SyncHandle();

    return True;
}


Status
XSyncAwait(dpy, wait_list, n_conditions)
    Display        *dpy;
    XSyncWaitCondition *wait_list;
    int   n_conditions;
{
    XExtDisplayInfo *info = find_display(dpy);
    XSyncWaitCondition *wait_item = wait_list;
    xSyncAwaitReq  *req;
    unsigned int    len;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncAwait, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncAwait;
    len = (n_conditions * SIZEOF(xSyncWaitCondition)) >> 2;
    SetReqLen(req, len, len /* XXX */ );

    while (n_conditions--)
    {
	xSyncWaitCondition  wc;
	wc.counter = wait_item->trigger.counter;
	wc.value_type = wait_item->trigger.value_type;
	wc.wait_value_lo = XSyncValueLow32(wait_item->trigger.wait_value);
	wc.wait_value_hi = XSyncValueHigh32(wait_item->trigger.wait_value);
	wc.test_type = wait_item->trigger.test_type;
	wc.event_threshold_lo = XSyncValueLow32(wait_item->event_threshold);
	wc.event_threshold_hi = XSyncValueHigh32(wait_item->event_threshold);
	Data(dpy, (char *)&wc, SIZEOF(xSyncWaitCondition));
	wait_item++;		/* get next trigger */
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

static void
_XProcessAlarmAttributes(dpy, req, valuemask, attributes)
    Display        *dpy;
    xSyncChangeAlarmReq *req;
    unsigned long   valuemask;
    XSyncAlarmAttributes *attributes;
{

    unsigned long  values[32];
    unsigned long *value = values;
    unsigned int    nvalues;

    if (valuemask & XSyncCACounter)
	*value++ = attributes->trigger.counter;

    if (valuemask & XSyncCAValueType)
	*value++ = attributes->trigger.value_type;

    if (valuemask & XSyncCAValue)
    {
	*value++ = XSyncValueHigh32(attributes->trigger.wait_value);
	*value++ = XSyncValueLow32(attributes->trigger.wait_value);
    }

    if (valuemask & XSyncCATestType)
	*value++ = attributes->trigger.test_type;

    if (valuemask & XSyncCADelta)
    {
	*value++ = XSyncValueHigh32(attributes->delta);
	*value++ = XSyncValueLow32(attributes->delta);
    }

    if (valuemask & XSyncCAEvents)
	*value++ = attributes->events;

    /* N.B. the 'state' field cannot be set or changed */
    req->length += (nvalues = value - values);
    nvalues <<= 2;		/* watch out for macros... */

    Data32(dpy, (long *) values, (long) nvalues);
}

XSyncAlarm 
XSyncCreateAlarm(dpy, values_mask, values)
    Display        *dpy;
    unsigned long   values_mask;
    XSyncAlarmAttributes *values;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncCreateAlarmReq *req;
    XSyncAlarm      aid;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncCreateAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncCreateAlarm;
    req->id = aid = XAllocID(dpy);
    values_mask &= XSyncCACounter | XSyncCAValueType | XSyncCAValue
    			| XSyncCATestType | XSyncCADelta | XSyncCAEvents;
    if (req->valueMask = values_mask)
	_XProcessAlarmAttributes(dpy, (xSyncChangeAlarmReq *) req,
				 values_mask, values);
    UnlockDisplay(dpy);
    SyncHandle();
    return aid;
}

Status
XSyncDestroyAlarm(dpy, alarm)
    Display        *dpy;
    XSyncAlarm      alarm;
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
    return True;
}

Status
XSyncQueryAlarm(dpy, alarm, values_return)
    Display        *dpy;
    XSyncAlarm      alarm;
    XSyncAlarmAttributes *values_return;
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

    if (!(_XReply(dpy, (xReply *) & rep,
    ((SIZEOF(xSyncQueryAlarmReply) - SIZEOF(xGenericReply)) >> 2), xFalse)))
    {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    values_return->trigger.counter = rep.counter;
    values_return->trigger.value_type = rep.value_type;
    XSyncIntsToValue(&values_return->trigger.wait_value,
				rep.wait_value_lo, rep.wait_value_hi);
    values_return->trigger.test_type = rep.test_type;
    XSyncIntsToValue(&values_return->delta, rep.delta_lo,
				rep.delta_hi);
    values_return->events = rep.events;
    values_return->state = rep.state;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XSyncChangeAlarm(dpy, alarm, values_mask, values)
    Display        *dpy;
    XSyncAlarm      alarm;
    unsigned long   values_mask;
    XSyncAlarmAttributes *values;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncChangeAlarmReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncChangeAlarm, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncChangeAlarm;
    req->alarm = alarm;
    values_mask &= XSyncCACounter | XSyncCAValueType | XSyncCAValue
		 | XSyncCATestType | XSyncCADelta | XSyncCAEvents;
    if (req->valueMask = values_mask)
	_XProcessAlarmAttributes(dpy, req, values_mask, values);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XSyncSetPriority(dpy, client_resource_id, priority)
    Display        *dpy;
    XID             client_resource_id;
    int             priority;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncSetPriorityReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncSetPriority, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncSetPriority;
    req->id = client_resource_id;
    req->priority = priority;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status 
XSyncGetPriority(dpy, client_resource_id, return_priority)
    Display        *dpy;
    XID             client_resource_id;
    int            *return_priority;
{
    XExtDisplayInfo *info = find_display(dpy);
    xSyncGetPriorityReply rep;
    xSyncGetPriorityReq *req;

    SyncCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(SyncGetPriority, req);
    req->reqType = info->codes->major_opcode;
    req->syncReqType = X_SyncGetPriority;
    req->id = client_resource_id;

    if (!_XReply(dpy, (xReply *) & rep, 0, xFalse))
    {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if (return_priority)
	*return_priority = rep.priority;

    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/*
 *  Functions corresponding to the macros for manipulating 64-bit values
 */

/* get rid of macros so we can define corresponding functions */
#undef XSyncIntToValue
#undef XSyncIntsToValue
#undef XSyncValueGreaterThan
#undef XSyncValueLessThan
#undef XSyncValueGreaterOrEqual
#undef XSyncValueLessOrEqual
#undef XSyncValueEqual
#undef XSyncValueIsNegative
#undef XSyncValueIsZero
#undef XSyncValueIsPositive
#undef XSyncValueLow32
#undef XSyncValueHigh32
#undef XSyncValueAdd
#undef XSyncValueSubtract
#undef XSyncMaxValue
#undef XSyncMinValue

void
XSyncIntToValue(pv, i)
    XSyncValue *pv;
    int i;
{
    _XSyncIntToValue(pv,i);
}

void
XSyncIntsToValue(pv, l, h)
    XSyncValue *pv;
    unsigned int l;
    int h;
{
    _XSyncIntsToValue(pv, l, h);
}

Bool
XSyncValueGreaterThan(a, b)
    XSyncValue a, b;
{
    return _XSyncValueGreaterThan(a, b);
}

Bool
XSyncValueLessThan(a, b)
    XSyncValue a, b;
{
    return _XSyncValueLessThan(a, b);
}

Bool
XSyncValueGreaterOrEqual(a, b)
    XSyncValue a, b;
{
    return _XSyncValueGreaterOrEqual(a, b);
}

Bool
XSyncValueLessOrEqual(a, b)
    XSyncValue a, b;
{
    return _XSyncValueLessOrEqual(a, b);
}

Bool
XSyncValueEqual(a, b)
    XSyncValue a, b;
{
    return _XSyncValueEqual(a, b);
}

Bool
XSyncValueIsNegative(v)
    XSyncValue v;
{
    return _XSyncValueIsNegative(v);
}

Bool
XSyncValueIsZero(a)
    XSyncValue a;
{
    return _XSyncValueIsZero(a);
}

Bool
XSyncValueIsPositive(v)
    XSyncValue v;
{
    return _XSyncValueIsPositive(v);
}

unsigned int
XSyncValueLow32(v)
    XSyncValue v;
{
    return _XSyncValueLow32(v);
}

int
XSyncValueHigh32(v)
    XSyncValue v;
{
    return _XSyncValueHigh32(v);
}

void
XSyncValueAdd(presult, a, b, poverflow)
    XSyncValue *presult, a, b;
    Bool *poverflow;
{
    _XSyncValueAdd(presult, a, b, poverflow);
}

void
XSyncValueSubtract(presult, a, b, poverflow)
    XSyncValue *presult, a, b;
    Bool *poverflow;
{
    _XSyncValueSubtract(presult, a, b, poverflow);
}

void
XSyncMaxValue(pv)
    XSyncValue *pv;
{
    _XSyncMaxValue(pv);
}

void
XSyncMinValue(pv)
    XSyncValue *pv;
{
    _XSyncMinValue(pv);
}
