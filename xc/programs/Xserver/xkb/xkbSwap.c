/* $XConsortium: xkbSwap.c,v 1.1 93/09/28 22:06:21 rws Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "X.h"
#define	NEED_EVENTS
#define	NEED_REPLIES
#include "Xproto.h"
#include "misc.h"
#include "inputstr.h"
#include "XKBsrv.h"
#include "XKBstr.h"
#include "extnsionst.h"

	/*
	 * REQUEST SWAPPING
	 */
static int
SProcXkbUseExtension(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xkbUseExtensionReq);

    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbUseExtensionReq);
    swaps(&stuff->wantedMajor,n);
    swaps(&stuff->wantedMinor,n);
    return ProcXkbUseExtension(client);
}

static int
SProcXkbSelectEvents(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xkbSelectEventsReq);

    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbSelectEventsReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->affectState,n);
    swaps(&stuff->state,n);
    swaps(&stuff->affectMap,n);
    swaps(&stuff->map,n);
    swapl(&stuff->affectControls,n);
    swapl(&stuff->controls,n);
    swaps(&stuff->affectNames,n);
    swaps(&stuff->names,n);
    swaps(&stuff->affectCompatMap,n);
    swaps(&stuff->compatMap,n);
    swapl(&stuff->affectIndicatorState,n);
    swapl(&stuff->indicatorState,n);
    swapl(&stuff->affectIndicatorMap,n);
    swapl(&stuff->indicatorMap,n);
    swaps(&stuff->affectAlternateSyms,n);
    swaps(&stuff->alternateSyms,n);
    return ProcXkbSelectEvents(client);
}

static int
SProcXkbSendEvent(client)
    ClientPtr client;
{
    extern void (* EventSwapVector[128])();
    register int n;
    xEvent eventT;
    void (*proc)(), NotImplemented();

    REQUEST(xkbSendEventReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbSendEventReq);
    swapl(&stuff->destination,n);
    swapl(&stuff->eventMask,n);

    /* swap event */
    proc = EventSwapVector[stuff->event.u.u.type & 0177];
    if (!proc || (int (*)())proc == (int (*)()) NotImplemented) 
	return BadValue;
    (*proc)(&stuff->event, &eventT);
    stuff->event = eventT;
    return ProcXkbSendEvent(client);
}

static int
SProcXkbBell(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbBellReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbBellReq);
    swaps(&stuff->deviceSpec,n);
    swapl(&stuff->name,n);
    return ProcXkbBell(client);
}

static int
SProcXkbGetState(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetStateReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetStateReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbGetState(client);
}

static int
SProcXkbLatchLockState(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbLatchLockStateReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbLatchLockStateReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbLatchLockState(client);
}

static int
SProcXkbGetControls(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetControlsReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetControlsReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbGetControls(client);
}

static int
SProcXkbSetControls(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetControlsReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbSetControlsReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->affectInternalMods,n);
    swaps(&stuff->internalMods,n);
    swaps(&stuff->affectIgnoreLockMods,n);
    swaps(&stuff->ignoreLockMods,n);
    swapl(&stuff->affectEnabledControls,n);
    swapl(&stuff->enabledControls,n);
    swapl(&stuff->changeControls,n);
    swaps(&stuff->repeatDelay,n);
    swaps(&stuff->repeatInterval,n);
    swaps(&stuff->slowKeysDelay,n);
    swaps(&stuff->debounceDelay,n);
    swaps(&stuff->mouseKeysDelay,n);
    swaps(&stuff->mouseKeysInterval,n);
    swaps(&stuff->mouseKeysTimeToMax,n);
    swaps(&stuff->mouseKeysMaxSpeed,n);
    swaps(&stuff->mouseKeysCurve,n);
    swaps(&stuff->accessXTimeout,n);
    swapl(&stuff->accessXTimeoutMask,n);
    return ProcXkbSetControls(client);
}
static int
SProcXkbGetMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetMapReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetMapReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->full,n);
    swaps(&stuff->partial,n);
    return ProcXkbGetMap(client);
}

static int
SProcXkbSetMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetMapReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetMapReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->present,n);
    swaps(&stuff->resize,n);
    swaps(&stuff->totalSyms,n);
    swaps(&stuff->totalActions,n);
    /* 11/15/93 (ef) -- XXX! what about keysyms and actions? */
    return ProcXkbSetMap(client);
}


static int
SProcXkbGetCompatMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetCompatMapReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetCompatMapReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->firstSym,n);
    swaps(&stuff->nSyms,n);
    return ProcXkbGetCompatMap(client);
}

static int
SProcXkbSetCompatMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetCompatMapReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetCompatMapReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->firstSym,n);
    swaps(&stuff->nSyms,n);
    /* 11/15/93 (ef) -- XXX! What about SymInterps and ModCompats? */
    return ProcXkbSetCompatMap(client);
}

static int
SProcXkbGetIndicatorState(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetIndicatorStateReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetIndicatorStateReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbGetIndicatorState(client);
}

static int
SProcXkbGetIndicatorMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetIndicatorMapReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetIndicatorMapReq);
    swaps(&stuff->deviceSpec,n);
    swapl(&stuff->which,n);
    return ProcXkbGetIndicatorMap(client);
}

static int
SProcXkbSetIndicatorMap(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetIndicatorMapReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetIndicatorMapReq);
    swaps(&stuff->deviceSpec,n);
    swapl(&stuff->which,n);
    return ProcXkbSetIndicatorMap(client);
}

static int
SProcXkbGetNames(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetNamesReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetNamesReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->which,n);
    return ProcXkbGetNames(client);
}

static int
SProcXkbSetNames(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetNamesReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetNamesReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->which,n);
    swapl(&stuff->keycodes,n);
    swapl(&stuff->geometry,n);
    swapl(&stuff->symbols,n);
    swapl(&stuff->indicators,n);
    swaps(&stuff->resize,n);
    return ProcXkbSetNames(client);
}

static int
SProcXkbListAlternateSyms(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbListAlternateSymsReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbListAlternateSymsReq);
    swaps(&stuff->deviceSpec,n);
    swapl(&stuff->name,n);
    swapl(&stuff->charset,n);
    return ProcXkbListAlternateSyms(client);
}


static int
SProcXkbGetAlternateSyms(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetAlternateSymsReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetAlternateSymsReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbGetAlternateSyms(client);
}

#ifdef NOTYET
static int
SProcXkbSetAlternateSyms(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetAlternateSymsReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetAlternateSymsReq);
    swaps(&stuff->deviceSpec,n);
    swaps(&stuff->present,n);
    swapl(&stuff->name,n);
    return ProcXkbSetAlternateSyms(client);
}

static int
SProcXkbGetGeometry(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbGetGeometryReq);
    swaps(&stuff->length,n);
    REQUEST_SIZE_MATCH(xkbGetGeometryReq);
    swaps(&stuff->deviceSpec,n);
    return ProcXkbGetGeometry(client);
}

static int
SProcXkbSetGeometry(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetGeometryReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetGeometryReq);
    swaps(&stuff->deviceSpec,n);
    swapl(&stuff->name,n);
    swaps(&stuff->widthMM,n);
    swaps(&stuff->heightMM,n);
    return ProcXkbSetGeometry(client);
}
#endif

static int
SProcXkbSetDebuggingFlags(client)
    ClientPtr client;
{
    register int n;

    REQUEST(xkbSetDebuggingFlagsReq);
    swaps(&stuff->length,n);
    REQUEST_AT_LEAST_SIZE(xkbSetDebuggingFlagsReq);
    swaps(&stuff->mask,n);
    swaps(&stuff->flags,n);
    swaps(&stuff->msgLength,n);
    return ProcXkbSetDebuggingFlags(client);
}

int
SProcXkbDispatch (client)
    ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_kbUseExtension:
	return SProcXkbUseExtension(client);
    case X_kbSelectEvents:
	return SProcXkbSelectEvents(client);
    case X_kbSendEvent:
	return SProcXkbSendEvent(client);
    case X_kbBell:
	return SProcXkbBell(client);
    case X_kbGetState:
	return SProcXkbGetState(client);
    case X_kbLatchLockState:
	return SProcXkbLatchLockState(client);
    case X_kbGetControls:
	return SProcXkbGetControls(client);
    case X_kbSetControls:
	return SProcXkbSetControls(client);
    case X_kbGetMap:
	return SProcXkbGetMap(client);
    case X_kbSetMap:
	return SProcXkbSetMap(client);
    case X_kbGetCompatMap:
	return SProcXkbGetCompatMap(client);
    case X_kbSetCompatMap:
	return SProcXkbSetCompatMap(client);
    case X_kbGetIndicatorState:
	return SProcXkbGetIndicatorState(client);
    case X_kbGetIndicatorMap:
	return SProcXkbGetIndicatorMap(client);
    case X_kbSetIndicatorMap:
	return SProcXkbSetIndicatorMap(client);
    case X_kbGetNames:
	return SProcXkbGetNames(client);
    case X_kbSetNames:
	return SProcXkbSetNames(client);
    case X_kbListAlternateSyms:
	return SProcXkbListAlternateSyms(client);
    case X_kbGetAlternateSyms:
	return SProcXkbGetAlternateSyms(client);
#ifdef NOTYET
    case X_kbSetAlternateSyms:
	return SProcXkbSetAlternateSyms(client);
    case X_kbGetGeometry:
	return SProcXkbGetGeometry(client);
    case X_kbSetGeometry:
	return SProcXkbSetGeometry(client);
#endif
    case X_kbSetDebuggingFlags:
	return SProcXkbSetDebuggingFlags(client);
    default:
	return BadRequest;
    }
}
