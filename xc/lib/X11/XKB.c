/* $XConsortium: XKB.c,v 1.10 94/02/05 13:54:40 rws Exp $ */
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

#include <stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xlibint.h"
#include <X11/extensions/XKBstr.h>
#include "XKBlibint.h"


Bool 
XkbQueryExtension(dpy,opcodeReturn,eventBaseReturn,errorBaseReturn,majorReturn,minorReturn)
    Display *dpy;
    int *opcodeReturn;
    int *eventBaseReturn;
    int *errorBaseReturn;
    int *majorReturn;
    int *minorReturn;
{
    if (!XkbUseExtension(dpy))
	return False;
    if (opcodeReturn)
	*opcodeReturn = dpy->xkb_info->codes->major_opcode;
    if (eventBaseReturn)
	*eventBaseReturn = dpy->xkb_info->codes->first_event;
    if (errorBaseReturn)
	*errorBaseReturn = dpy->xkb_info->codes->first_error;
    if (majorReturn)
	*majorReturn = dpy->xkb_info->srv_major;
    if (minorReturn)
	*minorReturn = dpy->xkb_info->srv_minor;
}

Status XkbLibraryVersion(libMajorRtrn,libMinorRtrn)
    int *libMajorRtrn;
    int *libMinorRtrn;
{
int supported;

    if (*libMajorRtrn != XkbMajorVersion)
	supported = False;
#if XkbMajorVersion==0
    else if (*libMinorRtrn != XkbMinorVersion)
	supported = False;
#endif
    else supported = True;

    *libMajorRtrn = XkbMajorVersion;
    *libMinorRtrn = XkbMinorVersion;
    return supported;
}

Status XkbSetAutoRepeatRate(dpy, deviceSpec, timeout, interval)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int timeout;
    unsigned int interval;
{
    register xkbSetControlsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetControls, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetControls;
    req->deviceSpec = deviceSpec;
    req->affectInternalMods = req->internalMods = 0;
    req->affectIgnoreLockMods = req->ignoreLockMods = 0;
    req->affectEnabledControls = req->enabledControls = 0;
    req->changeControls = XkbRepeatKeysMask;
    req->repeatDelay = timeout;
    req->repeatInterval = interval;
    req->slowKeysDelay = 0;
    req->debounceDelay = 0;
    req->mouseKeysDelay = 0;
    req->mouseKeysInterval = 0;
    req->mouseKeysTimeToMax = 0;
    req->mouseKeysCurve = 0;
    req->accessXTimeout = 0;
    req->mouseKeysDfltBtn = 0;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Bool XkbGetAutoRepeatRate(dpy, deviceSpec, timeoutp, intervalp)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int *timeoutp;
    unsigned int *intervalp;
{
    register xkbGetControlsReq *req;
    xkbGetControlsReply rep;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetControls, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetControls;
    req->deviceSpec = deviceSpec;
    if (!_XReply(dpy, (xReply *)&rep, 
		(SIZEOF(xkbGetControlsReply)-SIZEOF(xReply))>>2, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    *timeoutp = rep.repeatDelay;
    *intervalp = rep.repeatInterval;
    return True;
}

Bool
XkbDeviceBell(dpy,deviceID,bellClass,bellID,percent,name)
    Display *dpy;
    int deviceID;
    int bellClass;
    int bellID;
    int percent;
    Atom name;
{
    register xkbBellReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbBell,req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbBell;
    req->deviceSpec = deviceID;
    req->bellClass = bellClass;
    req->bellID = bellID;
    req->percent = percent;
    req->name = name;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XkbBell(dpy,percent,name)
    Display *dpy;
    int percent;
    Atom name;
{
    /* class 0 = KbdFeedbackClass (X Input Extension) */
    return XkbDeviceBell(dpy,XkbUseCoreKbd,0,0,percent,name);
}


Bool
XkbSelectEvents(dpy,deviceSpec,affect,which)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned long int affect;
    unsigned long int which;
{
    register xkbSelectEventsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    xkbi->selected_events&= ~affect;
    xkbi->selected_events|= (affect&which);
    GetReq(kbSelectEvents, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSelectEvents;
    req->deviceSpec = deviceSpec;
    if (affect&XkbStateNotifyMask) {
	req->affectState = XkbAllStateComponentsMask;
	if (which&XkbStateNotifyMask)
	     req->state = XkbAllStateComponentsMask;
	else req->state = 0;
    }
    if (affect&XkbMapNotifyMask) {
	req->affectMap = XkbAllMapComponentsMask;
	if (which&XkbMapNotifyMask)
	     req->map = XkbAllMapComponentsMask;
	else req->map = 0;
    }
    if (affect&XkbControlsNotifyMask) {
	req->affectControls = XkbAllControlsMask;
	if (which&XkbControlsNotifyMask)
	     req->controls = XkbAllControlsMask;
	else req->controls = 0;
    }
    if (affect&XkbNamesNotifyMask) {
	req->affectNames = XkbAllNamesMask;
	if (which&XkbNamesNotifyMask)
	     req->names = XkbAllNamesMask;
	else req->names = 0;
    }
    if (affect&XkbCompatMapNotifyMask) {
	req->affectCompatMap = XkbAllCompatMask;
	if (which&XkbCompatMapNotifyMask)
	     req->compatMap = XkbAllCompatMask;
	else req->compatMap = 0;
    }
    if (affect&XkbBellNotifyMask) {
	req->affectBell = True;
	req->bell = ((which&XkbBellNotifyMask)?True:False);
    }
    if (affect&XkbSlowKeyNotifyMask) {
	req->affectSlowKey = XkbSKAllEventsMask;
	if (which&XkbSlowKeyNotifyMask)
	     req->slowKey = XkbSKAllEventsMask;
	else req->slowKey = 0;
    }
    if (affect&XkbIndicatorNotifyMask) {
	req->affectIndicatorState = 0xffffffff;
	req->affectIndicatorMap = 0xffffffff;
	if (which&XkbIndicatorNotifyMask) {
	    req->indicatorState = 0xffffffff;
	    req->indicatorMap = 0xffffffff;
	}
	else {
	    req->indicatorState = 0;
	    req->indicatorMap = 0;
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool
XkbSelectEventDetails(dpy,deviceSpec,eventType,affect,details)
    Display *dpy;
    unsigned deviceSpec;
    unsigned eventType;
    unsigned long int affect;
    unsigned long int details;
{
    register xkbSelectEventsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    if (affect&details)	xkbi->selected_events|= (1<<eventType);
    else		xkbi->selected_events&= ~(1<<eventType);
    GetReq(kbSelectEvents, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSelectEvents;
    req->deviceSpec = deviceSpec;
    if (eventType==XkbStateNotify) {
	req->affectState = affect;
	req->state = details;
    }
    else req->affectState = req->state = 0;
    if (eventType==XkbMapNotify) {
	req->affectMap = affect;
	req->map = details;
    }
    else req->affectMap = req->map = 0;
    if (eventType==XkbControlsNotify) {
	req->affectControls = affect;
	req->controls = details;
    }
    else req->affectControls = req->controls = 0;
    if (eventType==XkbNamesNotify) {
	req->affectNames = affect;
	req->names = details;
    }
    else req->affectNames = req->names = 0;
    if (eventType==XkbCompatMapNotify) {
	req->affectCompatMap = affect;
	req->compatMap = details;
    }
    else req->affectCompatMap = req->compatMap = 0;
    if (eventType==XkbBellNotify) {
	req->affectBell = (affect!=0);
	req->bell = (details!=0);
    }
    else req->affectBell = req->bell = False;
    if (eventType==XkbSlowKeyNotify) {
	req->affectSlowKey = affect;
	req->slowKey = details;
    }
    else req->affectSlowKey = req->slowKey = 0;
    if (eventType==XkbIndicatorNotify) {
	req->affectIndicatorState = affect;
	req->indicatorState = details;
	req->affectIndicatorMap = affect;
	req->indicatorMap = details;
    }
    else {
	req->affectIndicatorState = req->indicatorState = 0;
	req->affectIndicatorMap = req->indicatorMap = 0;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbLockModifiers(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int values;
{
    register xkbLatchLockStateReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbLatchLockState, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbLatchLockState;
    req->deviceSpec = deviceSpec;
    req->affectModLocks= affect;
    req->modLocks = values;
    req->lockGroup = False;
    req->groupLock = 0;

    req->affectModLatches = req->modLatches = 0;
    req->latchGroup = False;
    req->groupLatch = 0;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XkbLatchModifiers(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int values;
{
    register xkbLatchLockStateReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbLatchLockState, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbLatchLockState;
    req->deviceSpec = deviceSpec;

    req->affectModLatches= affect;
    req->modLatches = values;
    req->latchGroup = False;
    req->groupLatch = 0;

    req->affectModLocks = req->modLocks = 0;
    req->lockGroup = False;
    req->groupLock = 0;

    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XkbSetServerInternalMods(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned deviceSpec;
    unsigned affect;
    unsigned values;
{
    register xkbSetControlsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetControls, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetControls;
    req->deviceSpec = deviceSpec;
    req->affectInternalMods = affect;
    req->internalMods = values;
    req->affectIgnoreLockMods = req->ignoreLockMods = 0;
    req->affectEnabledControls = req->enabledControls = 0;
    req->changeControls = XkbRepeatKeysMask;
    req->repeatDelay = 0;
    req->repeatInterval = 0;
    req->slowKeysDelay = 0;
    req->debounceDelay = 0;
    req->mouseKeysDelay = 0;
    req->mouseKeysInterval = 0;
    req->mouseKeysTimeToMax = 0;
    req->mouseKeysCurve = 0;
    req->mouseKeysDfltBtn = 0;
    req->accessXTimeout = 0;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XkbSetKeyOnlyMods(dpy,deviceSpec,affect,values)
    Display *dpy;
    unsigned int deviceSpec;
    unsigned int affect;
    unsigned int values;
{
    register xkbSetControlsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetControls, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetControls;
    req->deviceSpec = deviceSpec;
    req->affectInternalMods = req->internalMods = 0;
    req->affectIgnoreLockMods = affect;
    req->ignoreLockMods = values;
    req->affectEnabledControls = req->enabledControls = 0;
    req->changeControls = XkbRepeatKeysMask;
    req->repeatDelay = 0;
    req->repeatInterval = 0;
    req->slowKeysDelay = 0;
    req->debounceDelay = 0;
    req->mouseKeysDelay = 0;
    req->mouseKeysInterval = 0;
    req->mouseKeysTimeToMax = 0;
    req->mouseKeysCurve = 0;
    req->mouseKeysDfltBtn = 0;
    req->accessXTimeout = 0;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

Status
XkbSetDebuggingFlags(dpy,mask,flags,msg,rtrnFlags,disableLocks)
    Display *dpy;
    unsigned int mask;
    unsigned int flags;
    char *msg;
    unsigned int *rtrnFlags;
    Bool *disableLocks;
{
    register xkbSetDebuggingFlagsReq *req;
    xkbSetDebuggingFlagsReply rep;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetDebuggingFlags, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetDebuggingFlags;
    req->flags = flags;
    req->mask = mask;
    if (disableLocks)	req->disableLocks = *disableLocks;
    else 		req->disableLocks = XkbLeaveLocks;

    if (msg) {
	char *out;
	req->msgLength= strlen(msg)+1;
	req->length+= (req->msgLength+(unsigned)3)>>2;
	BufAlloc(char *,out,((req->msgLength+(unsigned)3)/4)*4);
	memcpy(out,msg,req->msgLength);
    }
    else req->msgLength= 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if (rtrnFlags)
	*rtrnFlags= rep.currentFlags;
    if (disableLocks)
	*disableLocks= rep.disableLocks;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

/***====================================================================***/

/* 8/16/94 (ef) -- XXX! Every place that calls this function should deal */
/*                      with failure */

static Bool
_XkbInitServerMap(xkb)
    XkbDescRec *xkb;
{
int	size;
XkbServerMapRec *map;

    if ((!xkb)||(xkb->server)||(xkb->maxKeyCode<8))
	return True;
    size= sizeof(XkbServerMapRec)+((xkb->maxKeyCode+1)*sizeof(XkbBehavior));
    size+= (((((xkb->maxKeyCode+1)*sizeof(CARD16))+3)/4)*4);
    xkb->server= map=(XkbServerMapRec *)Xmalloc(size);
    if (map) {
	bzero((char *)&map[1],size-sizeof(XkbServerMapRec));
	map->nActions = 1;
	map->szActions = 32;
	map->actions= (XkbAction *)Xmalloc(map->szActions*sizeof(XkbAction));
	map->actions[0].type = XkbSANoAction;

	map->keyBehaviors= (XkbBehavior *)&map[1];
	map->keyActions= (CARD16 *)&map->keyBehaviors[xkb->maxKeyCode];
	return True;
    }
    return False;
}

/***====================================================================***/

KeySym *
XkbEnlargeKeySymbols(xkb,key,needed)
    XkbDescRec *xkb;
    int key;
    int needed;
{
register int i,nSyms;
KeySym	*newSyms;

    if (XkbKeyNumSyms(xkb,key)>=(unsigned)needed) {
	return XkbKeySymsPtr(xkb,key);
    }
    if (xkb->map->szSyms-xkb->map->nSyms>=(unsigned)needed) {
	memcpy(&xkb->map->syms[xkb->map->nSyms],XkbKeySymsPtr(xkb,key),
					XkbKeyNumSyms(xkb,key)*sizeof(KeySym));
	xkb->map->keySymMap[key].offset = xkb->map->nSyms;
	xkb->map->nSyms+= needed;
	return &xkb->map->syms[xkb->map->keySymMap[key].offset];
    }
    xkb->map->szSyms+= 128;
    newSyms = (KeySym *)Xmalloc(xkb->map->szSyms*sizeof(KeySym));
    nSyms = 1;
    for (i=xkb->minKeyCode;i<=(int)xkb->maxKeyCode;i++) {
	memcpy(&newSyms[nSyms],XkbKeySymsPtr(xkb,i),
					XkbKeyNumSyms(xkb,i)*sizeof(KeySym));
	xkb->map->keySymMap[i].offset = nSyms;
	if (i!=key)	nSyms+= XkbKeyNumSyms(xkb,i);
	else		nSyms+= needed;
    }
    free(xkb->map->syms);
    xkb->map->syms = newSyms;
    xkb->map->nSyms = nSyms;
    return &xkb->map->syms[xkb->map->keySymMap[key].offset];
}

XkbAction *
XkbEnlargeKeyActions(xkb,key,needed)
    XkbDescRec *xkb;
    int key;
    int needed;
{
register int i,nActs;
XkbAction *newActs;

    if ((xkb->server->keyActions[key]!=0)&&(XkbKeyNumSyms(xkb,key)>=(unsigned)needed)) {
	return XkbKeyActionsPtr(xkb,key);
    }
    if (xkb->server->szActions-xkb->server->nActions>=(unsigned)needed) {
	xkb->server->keyActions[key]= xkb->server->nActions;
	xkb->server->nActions+= needed;
	return &xkb->server->actions[xkb->server->keyActions[key]];
    }
    xkb->server->szActions+= 32;
    newActs = (XkbAction *)Xmalloc(xkb->server->szActions*sizeof(XkbAction));
    newActs[0].type = XkbSANoAction;
    nActs = 1;
    for (i=xkb->minKeyCode;i<=(int)xkb->maxKeyCode;i++) {
	if (xkb->server->keyActions[i]!=0) {
	    memcpy(&newActs[nActs],XkbKeyActionsPtr(xkb,i),
				XkbKeyNumActions(xkb,i)*sizeof(XkbAction));
	    xkb->server->keyActions[i]= nActs;
	    if (i!=key)	nActs+= XkbKeyNumActions(xkb,i);
	    else	nActs+= needed;
	}
    }
    free(xkb->server->actions);
    xkb->server->actions = newActs;
    xkb->server->nActions= nActs;
    return &xkb->server->actions[xkb->server->keyActions[key]];
}

Status
XkbChangeTypeOfKey(xkb,key,newType,pChanges)
    XkbDescRec		*xkb;
    unsigned	 	 key;
    unsigned	 	 newType;
    XkbMapChangesRec	*pChanges;
{
XkbKeyTypeRec	*pOldType,*pNewType;

    if ((!xkb) || (key<xkb->minKeyCode) || (key>xkb->maxKeyCode) ||
	(!xkb->map)||(!xkb->map->keyTypes)||(newType>=xkb->map->nKeyTypes)) {
	return 0;
    }

    pOldType = XkbKeyKeyType(xkb,key);
    pNewType = &xkb->map->keyTypes[newType];
    if (newType==xkb->map->keySymMap[key].ktIndex) {
	return 1;
    }
    else if (pOldType->groupWidth==pNewType->groupWidth) {
	xkb->map->keySymMap[key].ktIndex= newType;
    }
    else if (pOldType->groupWidth>pNewType->groupWidth) {
	int g,l,nGroups;
	int oldWidth,newWidth;
	KeySym	*pSyms;

	xkb->map->keySymMap[key].ktIndex= newType;
	pSyms = XkbKeySymsPtr(xkb,key);
	oldWidth = pOldType->groupWidth;
	newWidth = pNewType->groupWidth;
	nGroups= XkbKeyNumGroups(xkb,key);
	for (g=1;g<nGroups;g++) {
	    for (l=0;l<newWidth;l++) {
		pSyms[g*newWidth+l]= pSyms[g*oldWidth+l];
	    }
	}
    }
    else {
	int g,l,nGroups;
	int oldWidth,newWidth;
	KeySym	*pSyms;

	xkb->map->keySymMap[key].ktIndex= newType;
	oldWidth = pOldType->groupWidth;
	newWidth = pNewType->groupWidth;
	nGroups= XkbKeyNumGroups(xkb,key);
	pSyms = XkbEnlargeKeySymbols(xkb,key,newWidth*nGroups);
	for (g=nGroups-1;g>=0;g--) {
	    for (l=newWidth-1;l>=oldWidth;l--) {
		pSyms[g*newWidth+l]= NoSymbol;
	    }
	    for (l=oldWidth-1;l>=0;l--) {
		pSyms[g*newWidth+l]= pSyms[g*oldWidth+l];
	    }
	}
    }
    if (pChanges->changed&XkbKeySymsMask) {
	int first,last;
	first= pChanges->firstKeySym;
	last= pChanges->firstKeySym+pChanges->nKeySyms-1;
	if (key<first)	first= key;
	if (key>last)	last= key;
	pChanges->firstKeySym = first;
	pChanges->nKeySyms = (last-first)+1;
    }
    else {
	pChanges->changed|= XkbKeySymsMask;
	pChanges->firstKeySym= key;
	pChanges->nKeySyms= 1;
    }
    return 1;
}

Status
XkbChangeSymsForKey(xkb,key,count,syms,pChanges)
    XkbDescRec		*xkb;
    unsigned	 	 key;
    unsigned	 	 count;
    KeySym 		*syms;
    XkbMapChangesRec	*pChanges;
{
XkbKeyTypeRec	*pType;
KeySym *pSyms;
int nGroups,nSyms;

    if ((!xkb) || (key<xkb->minKeyCode) || (key>xkb->maxKeyCode) ||
	(!xkb->map)||(!xkb->map->keyTypes)||(!xkb->map->keySymMap) ||
	(count<1))
	return 0;

    pType = XkbKeyKeyType(xkb,key);
    nGroups = ((count+pType->groupWidth-1)/pType->groupWidth);
    if ((nGroups<1)||(nGroups>8))
	return 0;
    
    nSyms= nGroups*pType->groupWidth;
    pSyms= XkbEnlargeKeySymbols(xkb,key,nSyms);
    if (!pSyms) 
	return 0;

    memcpy(pSyms,syms,count*sizeof(KeySym));
    while (count<nSyms) {
	pSyms[count++]= NoSymbol;
    }
    xkb->map->keySymMap[key].groupInfo= 
		XkbSetNumGroups(xkb->map->keySymMap[key].groupInfo,nGroups);

    if (pChanges->changed&XkbKeySymsMask) {
	int first,last;
	first= pChanges->firstKeySym;
	last= pChanges->firstKeySym+pChanges->nKeySyms-1;
	if (key<first)	first= key;
	if (key>last)	last= key;
	pChanges->firstKeySym = first;
	pChanges->nKeySyms = (last-first)+1;
    }
    else {
	pChanges->changed|= XkbKeySymsMask;
	pChanges->firstKeySym= key;
	pChanges->nKeySyms= 1;
    }
    return 1;
}

/***====================================================================***/

typedef struct _XkbReadBuffer {
	int	 error;
	int	 size;
	char	*start;
	char	*data;
} XkbReadBufferRec,*XkbReadBufferPtr;

static int 
_XkbInitReadBuffer(dpy,buf,size)
    Display		*dpy;
    XkbReadBufferPtr	 buf;
    int			 size;
{
    if ((dpy!=NULL) && (buf!=NULL) && (size>0)) {
	buf->error=  0;
	buf->size=   size;
	buf->start= buf->data= Xmalloc(size);
	if (buf->start) {
	    _XRead(dpy, buf->start, size);
	    return 1;
	}
    }
    return 0;
}

#define	_XkbReadBufferDataLeft(b)	(((b)->size)-((b)->data-(b)->start))

static int
_XkbSkipReadBufferData(from,size)
    XkbReadBufferPtr	 from;
    int			 size;
{
    if (size==0)
	return 1;
    if ((from==NULL)||(from->error)||(size<1)||
					(_XkbReadBufferDataLeft(from)<size))
	return 0;
    from->data+= size;
    return 1;
}

static int
_XkbCopyFromReadBuffer(from,to,size)
    XkbReadBufferPtr	 from;
    char		*to;
    int			 size;
{
    if (size==0)
	return 1;
    if ((from==NULL)||(from->error)||(to==NULL)||(size<1)||
					(_XkbReadBufferDataLeft(from)<size))
	return 0;
    memcpy(to,from->data,size);
    from->data+= size;
    return 1;
}

static int
_XkbFreeReadBuffer(buf)
    XkbReadBufferPtr	buf;
{
    if ((buf!=NULL) && (buf->start!=NULL)) {
	int left;
	left= _XkbReadBufferDataLeft(buf);
	Xfree(buf->start);
	buf->size= 0;
	buf->start= buf->data= NULL;
	return left;
    }
    return 0;
}

static int
_XkbReadKeyTypes(dpy,buf,info,rep,replace)
    Display *		dpy;
    XkbReadBufferPtr	buf;
    XkbDescPtr 		info;
    xkbGetMapReply *	rep;
    int 		replace;
{
int		 i,last;
XkbKeyTypeRec	*map;

    if ( rep->nKeyTypes>0 ) {
	last = rep->firstKeyType+rep->nKeyTypes-1;
	if ( info->map->nKeyTypes==0 ) {
	    info->map->nKeyTypes = last+1;
	    info->map->keyTypes=
		(XkbKeyTypeRec *)Xmalloc((last+1)*sizeof(XkbKeyTypeRec));
	    /* 4/5/93 (ef) -- XXX! deal with alloc failure? */
	    bzero(info->map->keyTypes, 
		  info->map->nKeyTypes*sizeof(XkbKeyTypeRec));
	}
	else if ( last >= (int)info->map->nKeyTypes ) {
	    info->map->keyTypes=(XkbKeyTypeRec *)Xrealloc(info->map->keyTypes,
						(last+1)*sizeof(XkbKeyTypeRec));
	    /* 4/5/93 (ef) -- XXX! deal with realloc failure? */
	    bzero(&info->map->keyTypes[info->map->nKeyTypes],
				(rep->nKeyTypes)*sizeof(XkbKeyTypeRec));
	    info->map->nKeyTypes = last+1;
	}
	else if ( replace && rep->firstKeyType==0 ) {
	    info->map->nKeyTypes = last+1;
	}

	map = &info->map->keyTypes[rep->firstKeyType];
	for (i=0;i<(int)rep->nKeyTypes;i++,map++) {
	    xkbKeyTypeWireDesc	desc;
	    _XkbCopyFromReadBuffer(buf,&desc,SIZEOF(xkbKeyTypeWireDesc));

	    if ( (!(desc.flags&xkb_KTHasPreserve)) && map->preserve ) {
		if (!map->flags&XkbNoFreeKTPreserve)
		    Xfree(map->preserve);
		map->preserve= NULL;
	    }
	    if ( (map->map==NULL) || (desc.mapWidth > XkbKTMapWidth(map)) ) {
		if ( map->map ) {
		     map->map = (CARD8 *)Xrealloc(map->map,desc.mapWidth);
		     if ( map->preserve )
			map->preserve = (CARD8 *)Xrealloc(map->preserve,
							  desc.mapWidth);
		}
		else map->map = (CARD8 *)Xmalloc(desc.mapWidth);
		/* 4/5/93 (ef) -- XXX! deal with realloc failure? */
		/* 8/10/93 (ef) -- XXX! resize symbol lists for keys */
	    }
	    if (( desc.flags&xkb_KTHasPreserve) && (!map->preserve))
		map->preserve = (CARD8 *)Xmalloc(desc.mapWidth);

	    map->flags = XkbNoFreeKTMap;
	    map->mask = desc.mask;
	    map->groupWidth = desc.groupWidth;
	    _XkbCopyFromReadBuffer(buf, (char *)map->map, desc.mapWidth);
	    last = (((desc.mapWidth+(unsigned)3)/4)*4)-desc.mapWidth;
	    if ( last )
		buf->data+= last;
	    if (desc.flags&xkb_KTHasPreserve) {
		if ( map->preserve ) {
		     _XkbCopyFromReadBuffer(buf, (char *)map->preserve,
								desc.mapWidth);
		     buf->data+= last;
		}
		else buf->data+= desc.mapWidth+last;
	    }
	}
	return 1;
    }
    return 0;
}

static int
_XkbReadKeySyms(dpy,buf,xkb,rep,replace)
    Display *		dpy;
    XkbReadBufferPtr	buf;
    XkbDescPtr 		xkb;
    xkbGetMapReply *	rep;
    int 		replace;
{
register int i;

    if (xkb->map->keySymMap==NULL) {
	register int offset;
	XkbSymMapRec *oldMap;
	xkb->map->keySymMap = (XkbSymMapRec *)Xmalloc((xkb->maxKeyCode+1)*
						   sizeof(XkbSymMapRec));
	if (!xkb->map->keySymMap)
	    return 0;
	if (!xkb->map->syms) {
	    int sz = ((rep->totalSyms+(unsigned)128)/128)*128;
	    xkb->map->syms = (KeySym *)Xmalloc(sz*sizeof(KeySym));
	    if (!xkb->map->syms)
		return 1;
	    xkb->map->szSyms = sz;
	}
	offset = 1;
	oldMap = &xkb->map->keySymMap[rep->firstKeySym];
	for (i=0;i<(int)rep->nKeySyms;i++,oldMap++) {
	    register int tmp;
	    _XkbCopyFromReadBuffer(buf,(char *)oldMap,sizeof(XkbSymMapRec));
	    tmp = oldMap->offset;
	    oldMap->offset = offset;
	    _XkbCopyFromReadBuffer(buf,(char *)&xkb->map->syms[offset],
							tmp*sizeof(KeySym));
	    offset+= tmp;
	}
	xkb->map->nSyms= offset;
    }
    else {
	xkbSymMapWireDesc newMap;
	XkbSymMapRec *oldMap;
	KeySym *newSyms;
	oldMap = &xkb->map->keySymMap[rep->firstKeySym];
	for (i=0;i<(int)rep->nKeySyms;i++,oldMap++) {
	    _XkbCopyFromReadBuffer(buf,(char *)&newMap,sizeof(XkbSymMapRec));
	    newSyms = XkbEnlargeKeySymbols(xkb,i+rep->firstKeySym,newMap.nSyms);
	    _XkbCopyFromReadBuffer(buf,(char *)newSyms,
						newMap.nSyms*sizeof(KeySym));
	    oldMap->ktIndex = newMap.ktIndex;
	    oldMap->groupInfo = newMap.groupInfo;
	}
    }
    return 1;
}

static int
_XkbReadKeyActions(dpy,buf,info,rep,replace)
    Display *		dpy;
    XkbDescPtr		info;
    XkbReadBufferPtr	buf;
    xkbGetMapReply *	rep;
    int 		replace;
{
int		 len;
int		 i,last;
CARD8		 numDesc[248];
XkbAction	*actDesc;

    if ( rep->nKeyActions>0 ) {
	XkbSymMapRec *symMap;
	_XkbCopyFromReadBuffer(buf, (char *)numDesc, rep->nKeyActions);
	len = rep->nKeyActions;
	i= ((len+3)/4)*4;
	if (i>len) {
	    buf->data+= i-len;
	    len = i;
	}
	symMap = &info->map->keySymMap[rep->firstKeyAction];
	for (i=0;i<(int)rep->nKeyActions;i++,symMap++) {
	    if (numDesc[i]==0) {
		info->server->keyActions[i+rep->firstKeyAction]= 0;
	    }
	    else {
		XkbAction *newActs;
		/* 8/16/93 (ef) -- XXX! Verify size here (numdesc must be */
		/*                 either zero or XkbKeyNumSyms(info,key) */
		newActs=XkbEnlargeKeyActions(info,i+rep->firstKeyAction,
								numDesc[i]);
		_XkbCopyFromReadBuffer(buf,(char *)newActs,
						numDesc[i]*sizeof(XkbAction));
		len+= numDesc[i]*sizeof(XkbAction);
	    }
	}
	return 1;
    }
    return 0;
}

static int
_XkbReadKeyBehaviors(dpy,buf,xkb,rep)
    Display *		dpy;
    XkbReadBufferPtr	buf;
    XkbDescPtr 		xkb;
    xkbGetMapReply *	rep;
{

    if ( rep->nKeyBehaviors>0 ) {
	if ( xkb->server->keyBehaviors == NULL ) {
	    int size = xkb->maxKeyCode+1;
	    xkb->server->keyBehaviors = 
			(XkbBehavior *)Xmalloc(size*sizeof(XkbBehavior));
	    /* 4/5/93 (ef) -- XXX! deal with alloc failure? */
	    bzero(xkb->server->keyBehaviors,(size*sizeof(XkbBehavior)));
	}
	if (_XkbCopyFromReadBuffer(buf, 
		(char *)&xkb->server->keyBehaviors[rep->firstKeyBehavior],
		rep->nKeyBehaviors*sizeof(XkbBehavior))) {
	    register int nRead;
	    nRead= rep->nKeyBehaviors*sizeof(XkbBehavior);
	    nRead= (((nRead+3)/4)*4)-nRead;
	    if (nRead>0)
		_XkbSkipReadBufferData(buf,nRead);
	    return 1;
	}
	/* 1/10/94 (ef) -- Should report an error here */
	_XkbSkipReadBufferData(buf,rep->nKeyBehaviors*sizeof(XkbBehavior));
    }
    return 0;
}

Status
XkbGetUpdatedMap(dpy,which,xkb)
    Display *dpy;
    unsigned which;
    XkbDescRec *xkb;
{
    XkbInfoPtr  xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if (which) {
	register xkbGetMapReq *req;
	xkbGetMapReply rep;
	LockDisplay(dpy);
	xkbi = dpy->xkb_info;
	GetReq(kbGetMap, req);
	req->reqType = xkbi->codes->major_opcode;
	req->xkbReqType = X_kbGetMap;
	req->deviceSpec = xkb->deviceSpec;
	req->full = which;
	req->partial = 0;
	if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
	if ( xkb->deviceSpec == XkbUseCoreKbd )
	    xkb->deviceSpec = rep.deviceID;
	xkb->minKeyCode = dpy->min_keycode;
	xkb->maxKeyCode = dpy->max_keycode;

	if ((which&XkbFullServerInfoMask)&&(!xkb->server))
	    _XkbInitServerMap(xkb);

	if (rep.length) {
	    XkbReadBufferRec	buf;
	    int			left;
	    if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
		_XkbReadKeyTypes(dpy,&buf,xkb,&rep,1);
		_XkbReadKeySyms(dpy,&buf,xkb,&rep,1);
		_XkbReadKeyActions(dpy,&buf,xkb,&rep,1);
		_XkbReadKeyBehaviors(dpy,&buf,xkb,&rep);
		left= _XkbFreeReadBuffer(&buf);
		if (left) 
		    fprintf(stderr,
			"GetUpdatedMap! Wrong length (%d extra bytes)\n",
			left);
		if (left || buf.error) {
		    UnlockDisplay(dpy);
		    SyncHandle();
		    return False;
		}
	    }
	}
	UnlockDisplay(dpy);
	SyncHandle();
    }
    return True;
}

XkbDescRec	*
XkbGetMap(dpy,which,deviceSpec)
    Display *dpy;
    unsigned which;
    unsigned deviceSpec;
{
XkbDescRec *xkb;

    xkb = (XkbDescRec *)Xmalloc(sizeof(XkbDescRec)+sizeof(XkbClientMapRec));
    if (xkb) {
	bzero(xkb,sizeof(XkbDescRec));
	xkb->deviceSpec = deviceSpec;
	xkb->map = (XkbClientMapRec *)&xkb[1];
	bzero(xkb->map,sizeof(XkbClientMapRec));
	if ((which)&&(!XkbGetUpdatedMap(dpy,which,xkb))) {
	    Xfree(xkb);
	    xkb= NULL;
	}
    }
    return xkb;
}

Status
XkbGetKeyTypes(dpy,first,num,xkb)
    Display *dpy;
    unsigned first;
    unsigned num;
    XkbDescRec *xkb;
{
    register xkbGetMapReq *req;
    xkbGetMapReply rep;
    int				nRead;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetMap;
    req->deviceSpec = XkbUseCoreKbd;
    req->full = 0;
    req->partial = XkbKeyTypesMask;
    req->firstKeyType = first;
    req->nKeyTypes = num;
    req->firstKeySym = req->nKeySyms = 0;
    req->firstKeyAction = req->nKeyActions = 0;
    req->firstKeyBehavior = req->nKeyBehaviors = 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if ( xkb->deviceSpec == XkbUseCoreKbd )
	xkb->deviceSpec = rep.deviceID;
    xkb->minKeyCode = dpy->min_keycode;
    xkb->maxKeyCode = dpy->max_keycode;

    if (rep.length) {
	XkbReadBufferRec	buf;
	int			left;
	if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
	    _XkbReadKeyTypes(dpy,&buf,xkb,&rep,1);
	    left= _XkbFreeReadBuffer(&buf);
	    if (left) 
		fprintf(stderr,"GetKeyTypes! Wrong length (%d extra bytes)\n",
									left);
	    if (left || buf.error) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbGetKeyActions(dpy,first,num,xkb)
    Display *dpy;
    unsigned first;
    unsigned num;
    XkbDescRec *xkb;
{
    register xkbGetMapReq *req;
    xkbGetMapReply rep;
    int				nRead;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetMap;
    req->deviceSpec = XkbUseCoreKbd;
    req->full = 0;
    req->partial = XkbKeyActionsMask;
    req->firstKeyType = req->nKeyTypes = 0;
    req->firstKeySym = req->nKeySyms = 0;
    req->firstKeyAction = first;
    req->nKeyActions = num;
    req->firstKeyBehavior = req->nKeyBehaviors = 0;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if ( xkb->deviceSpec == XkbUseCoreKbd )
	xkb->deviceSpec = rep.deviceID;
    xkb->minKeyCode = dpy->min_keycode;
    xkb->maxKeyCode = dpy->max_keycode;
    if (!xkb->server)
	_XkbInitServerMap(xkb);

    if (rep.length) {
	XkbReadBufferRec	buf;
	int			left;
	if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
	    _XkbReadKeyActions(dpy,&buf,xkb,&rep,0);
	    left= _XkbFreeReadBuffer(&buf);
	    if (left) 
		fprintf(stderr,"GetKeyActions! Wrong length (%d extra bytes)\n",
									left);
	    if (left || buf.error) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbGetKeySyms(dpy,first,num,xkb)
    Display *dpy;
    unsigned first;
    unsigned num;
    XkbDescRec *xkb;
{
    register xkbGetMapReq *req;
    xkbGetMapReply rep;
    int				nRead;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetMap;
    req->deviceSpec = XkbUseCoreKbd;
    req->full = 0;
    req->partial = XkbKeySymsMask;
    req->firstKeyType = req->nKeyTypes = 0;
    req->firstKeySym = first;
    req->nKeySyms = num;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    req->firstKeyAction = req->nKeyActions = 0;
    req->firstKeyBehavior = req->nKeyBehaviors = 0;
    if ( xkb->deviceSpec == XkbUseCoreKbd )
	xkb->deviceSpec = rep.deviceID;
    xkb->minKeyCode = dpy->min_keycode;
    xkb->maxKeyCode = dpy->max_keycode;
    if (rep.length) {
	XkbReadBufferRec	buf;
	int			left;
	if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
	    _XkbReadKeySyms(dpy,&buf,xkb,&rep,0);
	    left= _XkbFreeReadBuffer(&buf);
	    if (left) 
		fprintf(stderr,"GetKeySyms! Wrong length (%d extra bytes)\n",
									left);
	    if (left || buf.error) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbGetKeyBehaviors(dpy,first,num,xkb)
    Display *dpy;
    unsigned first;
    unsigned num;
    XkbDescRec *xkb;
{
    register xkbGetMapReq *req;
    xkbGetMapReply rep;
    int				nRead;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetMap;
    req->deviceSpec = XkbUseCoreKbd;
    req->full = 0;
    req->partial = XkbKeyBehaviorsMask;
    req->firstKeyType = req->nKeyTypes = 0;
    req->firstKeySym = req->nKeySyms = 0;
    req->firstKeyAction = req->nKeyActions = 0;
    req->firstKeyBehavior = first;
    req->nKeyBehaviors = num;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if ( xkb->deviceSpec == XkbUseCoreKbd )
	xkb->deviceSpec = rep.deviceID;
    xkb->minKeyCode = dpy->min_keycode;
    xkb->maxKeyCode = dpy->max_keycode;
    if (!xkb->server)
	_XkbInitServerMap(xkb);

    if (rep.length) {
	XkbReadBufferRec	buf;
	int			left;
	if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
	    _XkbReadKeyBehaviors(dpy,&buf,xkb,&rep);
	    left= _XkbFreeReadBuffer(&buf);
	    if (left) 
		fprintf(stderr,
			"GetKeyBehaviors! Wrong length (%d extra bytes)\n",
			left);
	    if (left || buf.error) {
		UnlockDisplay(dpy);
		SyncHandle();
		return False;
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XkbGetControls(dpy, which, desc)
    Display *dpy;
    unsigned long which;
    XkbDescRec	*desc;
{
    register xkbGetControlsReq *req;
    xkbGetControlsReply rep;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!desc) || (!which))
	return False;

    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetControls, req);
    if (!desc->controls)  {
	desc->controls = (XkbControlsRec *)Xmalloc(sizeof(XkbControlsRec));
	if (!desc->controls) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	} 
	bzero(desc->controls,sizeof(XkbControlsRec));
    }
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetControls;
    req->deviceSpec = desc->deviceSpec;
    if (!_XReply(dpy, (xReply *)&rep, 
		(SIZEOF(xkbGetControlsReply)-SIZEOF(xReply))>>2, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    desc->controls->enabledControls = rep.enabledControls;
    if (which&XkbKeyboardGroupsMask) 
	desc->controls->numGroups= rep.numGroups;
    if (which&XkbInternalModsMask)
	desc->controls->internalMods = rep.internalMods;
    if (which&XkbIgnoreLockModsMask)
	desc->controls->ignoreLockMods = rep.ignoreLockMods;
    if (which&XkbRepeatKeysMask) {
	desc->controls->repeatDelay = rep.repeatDelay;
	desc->controls->repeatInterval = rep.repeatInterval;
    }
    if (which&XkbSlowKeysMask)
	desc->controls->slowKeysDelay = rep.slowKeysDelay;
    if (which&XkbBounceKeysMask)
	desc->controls->debounceDelay = rep.debounceDelay;
    if (which&XkbMouseKeysMask) {
	desc->controls->mouseKeysDfltBtn = rep.mouseKeysDfltBtn;
	desc->controls->mouseKeysDelay = rep.mouseKeysDelay;
	desc->controls->mouseKeysInterval = rep.mouseKeysInterval;
    }
    if (which&XkbMouseKeysAccelMask) {
	desc->controls->mouseKeysTimeToMax = rep.mouseKeysTimeToMax;
	desc->controls->mouseKeysMaxSpeed = rep.mouseKeysMaxSpeed;
	desc->controls->mouseKeysCurve = rep.mouseKeysCurve;
    }
    if (which&XkbAccessXTimeoutMask) {
	desc->controls->accessXTimeout = rep.accessXTimeout;
	desc->controls->accessXTimeoutMask = rep.accessXTimeoutMask;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Bool XkbSetControls(dpy, which, desc)
    Display *dpy;
    unsigned long which;
    XkbDescRec	*desc;
{
    register xkbSetControlsReq *req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!desc)||(!desc->controls))
	return False;

    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetControls, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetControls;
    req->deviceSpec = desc->deviceSpec;
    req->changeControls = which;
    if (which&XkbInternalModsMask) {
	req->affectInternalMods= 0xff;
	req->internalMods= desc->controls->internalMods;
    }
    else {
	req->affectInternalMods = req->internalMods = 0;
    }
    if (which&XkbIgnoreLockModsMask) {
	req->affectIgnoreLockMods= 0xff;
	req->ignoreLockMods= desc->controls->ignoreLockMods;
    }
    else {
	req->affectIgnoreLockMods = req->ignoreLockMods = 0;
    }
    if (which&XkbControlsEnabledMask) {
	req->affectEnabledControls= XkbAllControlsMask;
	req->enabledControls= desc->controls->enabledControls;
    }
    else {
	req->affectEnabledControls = req->enabledControls = 0;
    }
    if (which&XkbRepeatKeysMask) {
	req->repeatDelay = desc->controls->repeatDelay;
	req->repeatInterval = desc->controls->repeatInterval;
    }
    else req->repeatDelay= req->repeatInterval= 0;
    if (which&XkbSlowKeysMask)
	 req->slowKeysDelay = desc->controls->slowKeysDelay;
    else req->slowKeysDelay = 0;
    if (which&XkbBounceKeysMask)
	 req->debounceDelay = desc->controls->debounceDelay;
    else req->debounceDelay = 0;
    if (which&XkbMouseKeysMask) {
	req->mouseKeysDfltBtn = desc->controls->mouseKeysDfltBtn;
	req->mouseKeysDelay = desc->controls->mouseKeysDelay;
	req->mouseKeysInterval = desc->controls->mouseKeysInterval;
	req->mouseKeysTimeToMax = desc->controls->mouseKeysTimeToMax;
	req->mouseKeysCurve = 0;
    }
    else {
	req->mouseKeysDfltBtn = 0;
	req->mouseKeysDelay = req->mouseKeysInterval = 0;
	req->mouseKeysTimeToMax = 0;
	req->mouseKeysCurve = 0;
    }
    if (which&XkbAccessXTimeoutMask) {
	 req->accessXTimeout = desc->controls->accessXTimeout;
	 req->accessXTimeoutMask = desc->controls->accessXTimeoutMask;
    }
    else {
	req->accessXTimeout = 0;
	req->accessXTimeoutMask = 0;
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

/***====================================================================***/

static Status
_XkbReadGetCompatMapReply(dpy,rep,xkb)
    Display *			dpy;
    xkbGetCompatMapReply *	rep;
    XkbDescPtr			xkb;
{
    if (rep->nSyms!=0) {
	XkbSymInterpretRec *syms;
	int lastSym= rep->firstSym+rep->nSyms-1;
	if (xkb->compat->nSymInterpret==0) {
	    syms=(XkbSymInterpretRec *)Xcalloc(xkb->compat->nSymInterpret,sizeof(XkbSymInterpretRec));
	    if (!syms)
		return False;	
	    xkb->compat->nSymInterpret= lastSym+1;
	    xkb->compat->symInterpret= syms;
	}
	else if (lastSym>=(int)xkb->compat->nSymInterpret) {
	    syms= (XkbSymInterpretRec *)Xcalloc(lastSym+1,sizeof(XkbSymInterpretRec));
	    if (!syms)
		return False;
	    if (xkb->compat->symInterpret) {
		memcpy(syms,xkb->compat->symInterpret,
		    xkb->compat->nSymInterpret*sizeof(XkbSymInterpretRec));
		Xfree(xkb->compat->symInterpret);
	    }
	    xkb->compat->nSymInterpret= lastSym+1;
	    xkb->compat->symInterpret= syms;
	}
	_XRead(dpy, &xkb->compat->symInterpret[rep->firstSym],
			rep->nSyms*sizeof(XkbSymInterpretRec));
    }
    if (rep->mods) {
	register int i,bit,n;
	XkbModCompatRec buf[8];
	n= rep->length-(rep->nSyms*sizeof(XkbSymInterpretRec));
	_XRead(dpy,buf,n);
	for (n=i=0,bit=1;i<8;i++,bit<<=1) {
	    if (rep->mods&bit) {
		xkb->compat->modCompat[i]= buf[n++];
	    }
	}
    }
    return True;
}

Status
XkbGetCompatMap(dpy,which,xkb)
    Display *dpy;
    unsigned which;
    XkbDescRec *xkb;
{
    register xkbGetCompatMapReq *req;
    xkbGetCompatMapReply	 rep;
    Status		     ok;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    if (!xkb->compat) {
	xkb->compat = (XkbCompatPtr)Xmalloc(sizeof(XkbCompatRec));
	if (xkb->compat)
	    bzero(xkb->compat,sizeof(XkbCompatRec));
	else {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
    }
    GetReq(kbGetCompatMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetCompatMap;
    req->deviceSpec = xkb->deviceSpec;
    if (which&XkbModCompatMask)
	 req->mods=	0xff;
    else req->mods=  0;
    if (which&XkbSymInterpMask)
	 req->getAllSyms= True;
    else req->getAllSyms= False;
    req->firstSym= req->nSyms= 0;

    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if (xkb->deviceSpec==XkbUseCoreKbd)
	xkb->deviceSpec= rep.deviceID;

    ok = _XkbReadGetCompatMapReply(dpy,&rep,xkb);
    UnlockDisplay(dpy);
    SyncHandle();
    return ok;
}

static Status
_XkbWriteSetCompatMap(dpy,req,xkb)
    Display *		dpy;
    xkbSetCompatMapReq	*req;
    XkbDescPtr		xkb;
{
CARD8	mods = req->mods;
CARD16	firstSym = req->firstSym;
CARD16	nSyms = req->nSyms;
int	size,nMods;
register int i;
char	*buf;

    for (i=0;i<8;i++) {
	if (mods&(1<<1))
	    nMods++;
    }
    size= nSyms*sizeof(XkbSymInterpretRec);
    size+= (((nMods*sizeof(XkbModCompatRec))+3)/4)*4;
    req->length+= (size+3)/4;
    BufAlloc(char *,buf,size);
    if (!buf)
	return False;
    if (nSyms) {
	memcpy(buf,(char *)&xkb->compat->symInterpret[firstSym],
				nSyms*sizeof(XkbSymInterpretRec));
	buf+= nSyms*sizeof(XkbSymInterpretRec);
    }
    if (mods) {
	XkbModCompatPtr out= (XkbModCompatPtr)buf;
	for (i=0;i<8;i++) {
	    if (mods&(1<<i)) {
		*out= xkb->compat->modCompat[i];
		out++;
	    }
	}
    }
    return True;
}

Status
XkbSetCompatMap(dpy,which,xkb,updateActions)
    Display *		dpy;
    unsigned 		which;
    XkbDescPtr		xkb;
    Bool		updateActions;
{
    register xkbSetCompatMapReq *req;
    Status		     ok;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!xkb->compat) ||
	((which&XkbSymInterpMask)&&(!xkb->compat->symInterpret)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetCompatMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetCompatMap;
    req->deviceSpec = xkb->deviceSpec;
    req->recomputeActions = updateActions;
    if (which&XkbModCompatMask)
	 req->mods=	0xff;
    else req->mods=  0;
    if (which&XkbSymInterpMask) {
	req->truncateSyms = True;
	req->firstSym= 0;
	req->nSyms= xkb->compat->nSymInterpret;
    }
    else {
	req->truncateSyms = False;
	req->firstSym= 0;
	req->nSyms= 0;
    }
    ok= _XkbWriteSetCompatMap(dpy,req,xkb);
    UnlockDisplay(dpy);
    SyncHandle();
    return ok;
}

/***====================================================================***/

static Status
_XkbReadGetNamesReply(dpy,rep,pMap)
    Display *dpy;
    xkbGetNamesReply *rep;
    XkbDescRec *pMap;
{
    int				i,len;
    char		 	*start,*desc;

    if ( pMap->deviceSpec == XkbUseCoreKbd )
	pMap->deviceSpec = rep->deviceID;

    if (rep->which&XkbKeycodesNameMask)
	pMap->names->keycodes= rep->keycodes;
    if (rep->which&XkbGeometryNameMask)
	pMap->names->geometry= rep->geometry;

    start = desc = Xmalloc(rep->length*4);
    if (desc==NULL) {
	return False;
    }
    _XRead(dpy, (char *)desc, rep->length*4);
    if ( rep->which & XkbKeyTypeNamesMask ) {
	len = pMap->map->nKeyTypes*sizeof(Atom);
	if (!pMap->names->keyTypes) {
	    pMap->names->keyTypes = (Atom *)Xmalloc(len);
	    if (!pMap->names->keyTypes) {
		Xfree(start);
		return False;
	    }
	}
	len= rep->nKeyTypes*sizeof(Atom);
	memcpy((char *)pMap->names->keyTypes,desc,len);
	desc+= len;
    }
    if ( rep->which&XkbKTLevelNamesMask ) {
	char *nLevels;
	Atom *levelNames;

	len= sizeof(Atom *)*pMap->map->nKeyTypes;
	if (!pMap->names->levels) {
	    pMap->names->levels= (Atom **)Xmalloc(len);
	    if (!pMap->names->levels) {
		Xfree(start);
		return False;
	    }
	    bzero((char *)pMap->names->levels,len);
	}
	nLevels = desc;
	desc+= ((rep->nKeyTypes+(unsigned)3)/4)*4;
	for (i=0;i<(int)rep->nKeyTypes;i++) {
	    if (nLevels[i]!=pMap->map->keyTypes[i].groupWidth) {
		fprintf(stderr,"groupWidth in names doesn't match key types\n");
		Xfree(start);
		return False;
	    }
	}
	for (i=0;i<(int)pMap->map->nKeyTypes;i++) {
	    len = sizeof(Atom)*nLevels[i];
	    levelNames = (Atom *)desc;
	    if (pMap->names->levels[i]==NULL)
		pMap->names->levels[i]=(Atom *)Xmalloc(len);
	    if (pMap->names->levels[i])
	       memcpy(pMap->names->levels[i],levelNames,len);
	    else {
		Xfree(start);
		return False;
	    }
	    desc+= len;
	}
    }
    if ( rep->which&XkbRGNamesMask ) {
	if (rep->nRadioGroups>0) {
	    len= sizeof(Atom)*rep->nRadioGroups;
	    if (pMap->names->radioGroups==NULL) {
		pMap->names->radioGroups = (Atom *)Xmalloc(len);
	    }
	    else if (pMap->names->nRadioGroups<rep->nRadioGroups) {
		pMap->names->radioGroups = (Atom *)
				Xrealloc(pMap->names->radioGroups,len);
	    }
	    if (!pMap->names->radioGroups) {
		fprintf(stderr,"Couldn't allocate radio group names\n");
		Xfree(start);
		return False;
	    }
	    memcpy((char *)pMap->names->radioGroups,desc,len);
	    pMap->names->nRadioGroups= rep->nRadioGroups;
	    desc+= len;
	}
	else {
	}
    }
    if (rep->which & XkbIndicatorNamesMask) {
	memcpy(pMap->names->indicators,desc,XkbNumIndicators*sizeof(Atom));
	desc+= XkbNumIndicators*sizeof(Atom);
    }
    if ( rep->which&XkbModifierNamesMask ) {
	memcpy(&pMap->names->modifiers[0],desc,8*sizeof(Atom));
	desc+= (8*sizeof(Atom));
    }
    if ( (rep->which&XkbCharSetsMask)&&(rep->nCharSets>0) ) {
	len= rep->nCharSets*sizeof(Atom);
	if (pMap->names->charSets) {
	    if (pMap->names->nCharSets<rep->nCharSets) {
		Xfree(pMap->names->charSets);
		pMap->names->charSets= NULL;
		pMap->names->nCharSets= 0;
	    }
	}
	if (!pMap->names->charSets) {
	    pMap->names->charSets= (Atom *)Xmalloc(len);
	    if (!pMap->names->charSets) {
		fprintf(stderr,"Couldn't allocate char set names\n");
		Xfree(start);
		return False;
	    }
	    memcpy((char*)pMap->names->charSets,desc,len);
	    pMap->names->nCharSets= rep->nCharSets;
	    desc+= len;
	}
    }
    if (((desc-start)>>2)!=rep->length) {
	fprintf(stderr,"Warning! Bad length (got %d, not %d) in XkbGetNames\n",
					(desc-start)>>2,rep->length);
    }
    Xfree(start);
    return True;
}

Status
XkbGetNames(dpy,which,pMap)
    Display *dpy;
    unsigned which;
    XkbDescRec *pMap;
{
    register xkbGetNamesReq *req;
    xkbGetNamesReply	     rep;
    Status		     ok;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    if (!pMap->names) {
	pMap->names = (XkbNamesRec *)Xmalloc(sizeof(XkbNamesRec));
	if (pMap->names)
	    bzero(pMap->names,sizeof(XkbNamesRec));
	else {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
    }
    GetReq(kbGetNames, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetNames;
    req->deviceSpec = pMap->deviceSpec;
    req->which = which;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    ok = _XkbReadGetNamesReply(dpy,&rep,pMap);
    UnlockDisplay(dpy);
    SyncHandle();
    return ok;
}

Status
XkbSetNames(dpy,which,firstKeyType,nKeyTypes,pMap)
    Display *dpy;
    unsigned which;
    unsigned firstKeyType;
    unsigned nKeyTypes;
    XkbDescRec *pMap;
{
    register xkbSetNamesReq *req;
    int  nNames;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!pMap)||(!pMap->names))
	return False;
    if (which&XkbKTLevelNamesMask) {
	register int i;
	if((!pMap->names->levels)||(pMap->map->keyTypes==NULL)||(nKeyTypes==0)||
				(firstKeyType+nKeyTypes>pMap->map->nKeyTypes))
	    return False;
 	for (i=nNames=0;i<nKeyTypes;i++) {
	    if (pMap->names->levels[i+firstKeyType]==NULL)
		return False;
	    nNames+= pMap->map->keyTypes[i+firstKeyType].groupWidth;
	}
    }
 
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetNames, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetNames;
    req->deviceSpec = pMap->deviceSpec;
    req->which = which;
    req->keycodes = pMap->names->keycodes;
    req->geometry = pMap->names->geometry;
    req->firstKeyType = firstKeyType;
    req->nKeyTypes = nKeyTypes;
    if (which&XkbModifierNamesMask)
	req->length+= (8*sizeof(Atom))/4;
    if (which&XkbKTLevelNamesMask) 
	req->length+= ((((nKeyTypes+3)/4)*4)+(nNames*sizeof(Atom)))/4;

    if (which&XkbModifierNamesMask) {
	char *buf; 
	BufAlloc(char *,buf,8*sizeof(Atom));
	memcpy(buf,pMap->names->modifiers,8*sizeof(Atom));
    }
    if (which&XkbKTLevelNamesMask) {
	XkbKeyTypeRec *pKeyType;
	char *buf,*tmp;
	int i,sz;
	sz = ((nKeyTypes+3)/4)*4+(nNames*sizeof(Atom));
	BufAlloc(char *,buf,sz);
	tmp = buf;
	pKeyType = &pMap->map->keyTypes[firstKeyType];
	for (i=0;i<nKeyTypes;i++,pKeyType++) {
	    *tmp++ = pKeyType->groupWidth;
	}
	tmp = &buf[((nKeyTypes+3)/4)*4];
	pKeyType = &pMap->map->keyTypes[firstKeyType];
	for (i=0;i<nKeyTypes;i++,pKeyType++) {
	    memcpy(tmp,pMap->names->levels[i+firstKeyType],
					pKeyType->groupWidth*sizeof(Atom));
	    tmp+= pKeyType->groupWidth*sizeof(Atom);
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbGetState(dpy,deviceSpec,rtrn)
    Display *dpy;
    unsigned deviceSpec;
    XkbStateRec *rtrn;
{
    register xkbGetStateReq	*req;
    xkbGetStateReply rep;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetState, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetState;
    req->deviceSpec = deviceSpec;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    rtrn->mods= rep.mods;
    rtrn->baseMods= rep.baseMods;
    rtrn->latchedMods= rep.latchedMods;
    rtrn->lockedMods= rep.lockedMods;
    rtrn->group= rep.group;
    rtrn->baseGroup = rep.baseGroup;
    rtrn->latchedGroup= rep.latchedGroup;
    rtrn->lockedGroup = rep.lockedGroup;
    rtrn->compatState= rep.compatState;
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/***====================================================================***/

static int
_XkbSizeKeyTypes(xkb,firstKeyType,nKeyTypes)
    XkbDescRec *xkb;
    unsigned firstKeyType;
    unsigned nKeyTypes;
{
    XkbKeyTypeRec 	*map;
    int			i,len;

    len= 0;
    map= &xkb->map->keyTypes[firstKeyType];
    for (i=0;i<nKeyTypes;i++,map++){
	len+= SIZEOF(xkbKeyTypeWireDesc)+(((XkbKTMapWidth(map)+(unsigned)3)/4)*4);
	if (map->preserve)
	    len+= (((XkbKTMapWidth(map)+(unsigned)3)/4)*4);
    }
    return len;
}

static void
_XkbWriteKeyTypes(dpy,xkb,firstKeyType,nKeyTypes)
    Display	*dpy;
    XkbDescRec	*xkb;
    unsigned	 firstKeyType;
    unsigned	 nKeyTypes;
{
    char		*buf;
    XkbKeyTypeRec 	*map;
    int			i,sz;
    xkbKeyTypeWireDesc	*desc;

    map= &xkb->map->keyTypes[firstKeyType];
    for (i=0;i<nKeyTypes;i++,map++) {
	sz= (((XkbKTMapWidth(map)*(map->preserve?2:1))+(unsigned)3)/4)*4;
	BufAlloc(xkbKeyTypeWireDesc *,desc,sz+SIZEOF(xkbKeyTypeWireDesc));
	desc->flags = (map->preserve?xkb_KTHasPreserve:0);
	desc->mask = map->mask;
	desc->groupWidth = map->groupWidth;
	desc->mapWidth = XkbKTMapWidth(map);
	buf= (char *)&desc[1];
	memcpy(buf,map->map,XkbKTMapWidth(map));
	buf+= XkbKTMapWidth(map);
	if (map->preserve) {
	    memcpy(buf,map->preserve,XkbKTMapWidth(map));
	}
	buf+= sz-XkbKTMapWidth(map);
    }
    return;
}

static int
_XkbSizeKeySyms(xkb,firstKey,nKeys,nSymsRtrn)
    XkbDescRec  *xkb;
    unsigned 	 firstKey;
    unsigned	 nKeys;
    CARD16	*nSymsRtrn;
{
    XkbSymMapRec	*symMap;
    XkbKeyTypeRec 	*keyType;
    int			i,len;
    unsigned		nSyms;

    len= nKeys*sizeof(XkbSymMapRec);
    symMap = &xkb->map->keySymMap[firstKey];
    for (i=nSyms=0;i<nKeys;i++,symMap++) {
	keyType = &xkb->map->keyTypes[symMap->ktIndex];
	nSyms+= XkbNumGroups(symMap->groupInfo)*keyType->groupWidth;
    }
    len+= nSyms*sizeof(KeySym);
    *nSymsRtrn = nSyms;
    return len;
}

static void
_XkbWriteKeySyms(dpy,xkb,firstKeySym,nKeySyms,totalSyms)
    Display *dpy;
    XkbDescRec *xkb;
    unsigned firstKeySym;
    unsigned nKeySyms;
    unsigned totalSyms;
{
register KeySym *pSym,*outSym;
XkbSymMapRec	*symMap;
XkbKeyTypeRec	*keyType;
xkbSymMapWireDesc *desc;
register int	i;

    i= (nKeySyms*SIZEOF(xkbSymMapWireDesc))+(totalSyms*sizeof(KeySym));
    BufAlloc(xkbSymMapWireDesc *,desc,i);
    symMap = &xkb->map->keySymMap[firstKeySym];
    for (i=0;i<nKeySyms;i++,symMap++) {
	keyType = &xkb->map->keyTypes[symMap->ktIndex];
	desc->ktIndex = symMap->ktIndex;
	desc->groupInfo = symMap->groupInfo;
	desc->nSyms = XkbNumGroups(symMap->groupInfo)*keyType->groupWidth;
	outSym = (KeySym *)&desc[1];
	pSym = &xkb->map->syms[symMap->offset];
	memcpy(outSym,pSym,desc->nSyms*sizeof(KeySym));
	desc = (xkbSymMapWireDesc *)&outSym[desc->nSyms];
    }
    return;
}

static int
_XkbSizeKeyActions(xkb,firstKey,nKeys,nActsRtrn)
    XkbDescRec	*xkb;
    unsigned	 firstKey;
    unsigned	 nKeys;
    CARD16	*nActsRtrn;
{
    int			i,len,nActs;
    unsigned		nSyms;

    for (nActs=i=0;i<nKeys;i++) {
	if (xkb->server->keyActions[i+firstKey]!=0)
	    nActs+= XkbKeyNumActions(xkb,i+firstKey);
    }
    len= (((nKeys+3)/4)*4)+(nActs*sizeof(XkbAction));
    *nActsRtrn = nActs;
    return len;
}

static void
_XkbWriteKeyActions(dpy,xkb,firstKey,nKeys,totalActs)
    Display *dpy;
    XkbDescRec *xkb;
    unsigned firstKey;
    unsigned nKeys;
    unsigned totalActs;
{
    register int	 i;
    int	 		 n;
    CARD8		*numDesc;
    XkbAction		*actDesc;

    n = (((nKeys+3)/4)*4)+totalActs*sizeof(XkbAction);
    BufAlloc(CARD8 *,numDesc,n);
    for (i=0;i<nKeys;i++) {
	if (xkb->server->keyActions[i+firstKey]==0)
	     numDesc[i] = 0;
	else numDesc[i] = XkbKeyNumActions(xkb,(i+firstKey));
    }
    actDesc = (XkbAction *)&numDesc[(((nKeys)+3)/4)*4];
    for (i=0;i<nKeys;i++) {
	if (xkb->server->keyActions[i+firstKey]!=0) {
	    n = XkbKeyNumActions(xkb,(i+firstKey));
	    memcpy(actDesc,XkbKeyActionsPtr(xkb,(i+firstKey)),
                                                        n*sizeof(XkbAction));
	    actDesc+= n;
	}
    }
    return;
}

static void
SendSetMap(dpy,xkb,req)
    Display *dpy;
    XkbDescRec *xkb;
    xkbSetMapReq *req;
{
xkbSetMapReq tmp;

    req->length+= _XkbSizeKeyTypes(xkb,req->firstKeyType,req->nKeyTypes)/4;
    req->length+= _XkbSizeKeySyms(xkb,req->firstKeySym,req->nKeySyms,
						    &req->totalSyms)/4;
    req->length+= _XkbSizeKeyActions(xkb,req->firstKeyAction,req->nKeyActions,
						    &req->totalActions)/4;
    req->length+= ((req->nKeyBehaviors*sizeof(XkbBehavior))+(unsigned)3)/4;

    tmp= *req;
    if ( tmp.nKeyTypes>0 )
	_XkbWriteKeyTypes(dpy,xkb,tmp.firstKeyType,tmp.nKeyTypes);
    if ( tmp.nKeySyms>0 )
	_XkbWriteKeySyms(dpy,xkb,tmp.firstKeySym,tmp.nKeySyms,tmp.totalSyms);
    if ( tmp.nKeyActions )
	_XkbWriteKeyActions(dpy,xkb,tmp.firstKeyAction,tmp.nKeyActions,
							     tmp.totalActions);
    if ( tmp.nKeyBehaviors>0 ) {
	register int sz;
	char	*buf;

	sz = tmp.nKeyBehaviors*sizeof(XkbBehavior);
	BufAlloc(char *,buf,((sz+3)/4)*4);
	memcpy(buf,&xkb->server->keyBehaviors[tmp.firstKeyBehavior],sz);
	/* 11/11/93 (ef) -- XXX!  Is this safe? */
    }
    return;
}

Status
XkbSetMap(dpy,which,xkb)
    Display *dpy;
    unsigned which;
    XkbDescRec *xkb;
{
    register xkbSetMapReq	*req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;

    if (((which&XkbKeyTypesMask)&&(!xkb->map->keyTypes))||
	((which&XkbKeySymsMask)&&((!xkb->map->syms)||(!xkb->map->keySymMap)))||
	((which&XkbKeyActionsMask)&&
				((!xkb->server)||(!xkb->server->keyActions)))||
	((which&XkbKeyBehaviorsMask)&&
			       ((!xkb->server)||(!xkb->server->keyBehaviors))))
	return False;

    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetMap;
    req->deviceSpec = xkb->deviceSpec;
    req->present = which;
    req->resize = which&XkbResizableInfoMask;
    req->firstKeyType = 0;
    if (which&XkbKeyTypesMask)	req->nKeyTypes = xkb->map->nKeyTypes;
    else			req->nKeyTypes = 0;
    req->firstKeySym = 0;
    if (which&XkbKeySymsMask) {
	req->firstKeySym = xkb->minKeyCode;
	req->nKeySyms = xkb->maxKeyCode-xkb->minKeyCode+1;
    }
    if (which&XkbKeyActionsMask) {
	req->firstKeyAction = xkb->minKeyCode;
	req->nKeyActions = xkb->maxKeyCode-xkb->minKeyCode+1;
    }
    if (which&XkbKeyBehaviorsMask) {
	req->firstKeyBehavior = xkb->minKeyCode;
	req->nKeyBehaviors = xkb->maxKeyCode-xkb->minKeyCode+1;
    }
    SendSetMap(dpy,xkb,req);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbChangeMap(dpy,xkb,changes)
    Display *dpy;
    XkbDescRec *xkb;
    XkbMapChangesRec *changes;
{
    register xkbSetMapReq	*req;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;

    if (((changes->changed&XkbKeyTypesMask)&&(!xkb->map->keyTypes))||
	((changes->changed&XkbKeySymsMask)&&((!xkb->map->syms)||
				(!xkb->map->keySymMap)))||
	((changes->changed&XkbKeyActionsMask)&&
				((!xkb->server)||(!xkb->server->keyActions)))||
	((changes->changed&XkbKeyBehaviorsMask)&&
			       ((!xkb->server)||(!xkb->server->keyBehaviors))))
	return False;

    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetMap;
    req->deviceSpec = xkb->deviceSpec;
    req->present = changes->changed;
    req->resize = 0;
    req->firstKeyType = changes->firstKeyType;
    req->nKeyTypes = changes->nKeyTypes;
    req->firstKeySym = changes->firstKeySym;
    req->nKeySyms = changes->nKeySyms;
    req->firstKeyAction = changes->firstKeyAction;
    req->nKeyActions = changes->nKeyActions;
    req->firstKeyBehavior = changes->firstKeyBehavior;
    req->nKeyBehaviors = changes->nKeyBehaviors;
    SendSetMap(dpy,xkb,req);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/***====================================================================***/

Status
XkbGetIndicatorState(dpy,deviceSpec,pStateRtrn)
    Display *dpy;
    unsigned int deviceSpec;
    CARD32 *pStateRtrn;
{
    register xkbGetIndicatorStateReq *req;
    xkbGetIndicatorStateReply	rep;
    XkbInfoPtr xkbi;

    
    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetIndicatorState, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetIndicatorState;
    req->deviceSpec = deviceSpec;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }
    if (pStateRtrn)
	*pStateRtrn= rep.state;
    return True;
}

Status
XkbGetIndicatorMap(dpy,which,xkb)
    Display *dpy;
    unsigned long which;
    XkbDescRec *xkb;
{
    register xkbGetIndicatorMapReq	*req;
    XkbIndicatorRec	*leds;
    xkbGetIndicatorMapReply rep;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!which)||(!xkb))
	return False;

    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbGetIndicatorMap, req);
    if (!xkb->indicators) {
	xkb->indicators = (XkbIndicatorRec *)Xmalloc(sizeof(XkbIndicatorRec));
	if (xkb->indicators)
	    bzero(xkb->indicators,sizeof(XkbIndicatorRec));
    }
    leds = xkb->indicators;
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbGetIndicatorMap;
    req->deviceSpec = xkb->deviceSpec;
    req->which = which;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return False;
    }

    leds->nRealIndicators = rep.nRealIndicators;
    if (rep.which) {
	register int i,bit,left;
	left= which;
	for (i=0,bit=1;(i<32)&&(left);i++,bit<<=1) {
	    if (left&bit) {
		_XRead(dpy,(char *)&leds->maps[i],sizeof(XkbIndicatorMapRec));
		left&= ~bit;
	    }
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

Status
XkbSetIndicatorMap(dpy,which,xkb)
    Display *dpy;
    unsigned long which;
    XkbDescRec *xkb;
{
    register xkbSetIndicatorMapReq	*req;
    register int i,bit;
    int nMaps;
    xkbIndicatorMapWireDesc *wire;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    if ((!xkb)||(!which)||(!xkb->indicators))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    GetReq(kbSetIndicatorMap, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbSetIndicatorMap;
    req->deviceSpec = xkb->deviceSpec;
    req->which = which;
    for (i=nMaps=0,bit=1;i<32;i++,bit<<=1) {
	if (which&bit)
	    nMaps++;
    }
    req->length+= (nMaps*sizeof(XkbIndicatorMapRec))/4;
    BufAlloc(xkbIndicatorMapWireDesc *,wire,(nMaps*sizeof(XkbIndicatorMapRec)));
    for (i=0,bit=1;i<32;i++,bit<<=1) {
	if (which&bit) {
	    wire->whichMods= xkb->indicators->maps[i].whichMods;
	    wire->mods= xkb->indicators->maps[i].mods;
	    wire->whichGroups= xkb->indicators->maps[i].whichGroups;
	    wire->groups= xkb->indicators->maps[i].groups;
	    wire->controls= xkb->indicators->maps[i].controls;
	    wire++;
	}
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}

/***====================================================================***/

Status
XkbRefreshMap(dpy,xkb,changes)
    Display *dpy;
    XkbDescRec *xkb;
    XkbMapChangesRec *changes;
{
    xkbGetMapReq *req;
    xkbGetMapReply rep;
    int nRead;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    LockDisplay(dpy);
    xkbi = dpy->xkb_info;
    if (changes->changed) {
	GetReq(kbGetMap, req);
	req->reqType = xkbi->codes->major_opcode;
	req->xkbReqType = X_kbGetMap;
	req->deviceSpec = XkbUseCoreKbd;
	req->full = 0;
	req->partial = changes->changed;
	req->firstKeyType = changes->firstKeyType;
	req->nKeyTypes = changes->nKeyTypes;
	req->firstKeySym = changes->firstKeySym;
	req->nKeySyms = changes->nKeySyms;
	req->firstKeyAction = changes->firstKeyAction;
	req->nKeyActions = changes->nKeyActions;
	req->firstKeyBehavior = changes->firstKeyBehavior;
	req->nKeyBehaviors = changes->nKeyBehaviors;
	if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return False;
	}
	if ( xkb->deviceSpec == XkbUseCoreKbd )
	    xkb->deviceSpec = rep.deviceID;
	xkb->minKeyCode = dpy->min_keycode;
	xkb->maxKeyCode = dpy->max_keycode;

	if (rep.length) {
	    XkbReadBufferRec	buf;
	    int			left;
	    if (_XkbInitReadBuffer(dpy,&buf,rep.length*4)) {
		_XkbReadKeyTypes(dpy,&buf,xkb,&rep,1);
		_XkbReadKeySyms(dpy,&buf,xkb,&rep,1);
		_XkbReadKeyActions(dpy,&buf,xkb,&rep,1);
		_XkbReadKeyBehaviors(dpy,&buf,xkb,&rep);
		left= _XkbFreeReadBuffer(&buf);
		if (left) 
		    fprintf(stderr,
			"XkbRefresh! Wrong length (%d extra bytes)\n",
			left);
		if (left || buf.error) {
		    UnlockDisplay(dpy);
		    SyncHandle();
		    return False;
		}
	    }
	}
	SyncHandle();
	UnlockDisplay(dpy);
    }
    return True;
}
