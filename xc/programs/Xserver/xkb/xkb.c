/* $XConsortium: xkb.c,v 1.1 93/09/26 21:12:46 rws Exp $ */
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

#include "XI.h"
#ifdef XINPUT
DeviceIntPtr LookupDeviceIntRec();
#endif
	int	XkbEventBase;
	int	XkbErrorBase;
	int	XkbReqCode;
	int	XkbKeyboardErrorCode;
Atom	xkbONE_LEVEL;
Atom	xkbTWO_LEVEL;
Atom	xkbKEYPAD;
CARD16	xkbDebugFlags = 0;

RESTYPE	RT_XKBCLIENT;

static DeviceIntPtr
XkbLookupDevice(id)
    int id;
{
DeviceIntPtr dev = NULL;

   if ( id == XKB_USE_CORE_KBD )
	dev= (DeviceIntPtr)LookupKeyboardDevice();
#ifdef XINPUT
   else {
	dev = LookupDeviceIntRec(id);
	if (dev) {
	    if ((!dev->key)||(!dev->key->xkbInfo))
		dev= NULL;
	}
   }
#endif
   return dev;
}

/***====================================================================***/

static int
ProcXkbUseExtension(client)
    ClientPtr client;
{
    REQUEST(xkbUseExtensionReq);
    xkbUseExtensionReply	rep;
    register int n;
    int	supported;

    REQUEST_SIZE_MATCH(xkbUseExtensionReq);
    if (stuff->wantedMajor != XKB_MAJOR_VERSION)
	supported = 0;
#if XKB_MAJOR_VERSION==0
    else if (stuff->wantedMinor != XKB_MINOR_VERSION)
	supported = 0;
#endif
    else supported = 1;

#ifndef _XKB_SWAPPING_FIXED
    if (client->swapped)
	supported= 0;
#endif

    if ((supported) && (!(client->xkbClientFlags&XKB_INITIALIZED))) {
	client->xkbClientFlags= XKB_INITIALIZED;
	client->mapNotifyMask= XkbKeyTypesMask|XkbKeySymsMask;
    }
#if XKB_MAJOR_VERSION==0
    else if (xkbDebugFlags) {
	ErrorF("Rejecting client %d (0x%x) (wants %d.%02d, have %d.%02d)\n",
		client->index, client->clientAsMask,
		stuff->wantedMajor,stuff->wantedMinor,
		XKB_MAJOR_VERSION,XKB_MINOR_VERSION);
    }
#endif
    rep.type = X_Reply;
    rep.supported = supported;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.serverMajor = XKB_MAJOR_VERSION;
    rep.serverMinor = XKB_MINOR_VERSION;
    if ( client->swapped ) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.serverMajor, n);
	swaps(&rep.serverMinor, n);
    }
    WriteToClient(client,sizeof(xkbUseExtensionReply), (char *)&rep);
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbSelectEvents(client)
    ClientPtr client;
{
    int status;
    DeviceIntPtr dev;
    XkbInterestRec *masks;
    REQUEST(xkbSelectEventsReq);

    REQUEST_SIZE_MATCH(xkbSelectEventsReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    if (stuff->affectMap) {
	client->mapNotifyMask&= ~stuff->affectMap;
	client->mapNotifyMask|= (stuff->affectMap&stuff->map);
    }
    masks = XkbFindClientResource((DevicePtr)dev,client);
    if (!masks){
	XID id = FakeClientID(client->index);
	AddResource(id,RT_XKBCLIENT,dev);
	masks= XkbAddClientResource((DevicePtr)dev,client,id);
    }
    if (masks) {
	if (stuff->affectState) {
	    masks->stateNotifyMask&= ~stuff->affectState;
	    masks->stateNotifyMask|= (stuff->affectState&stuff->state);
	}
	if (stuff->affectControls) {
	    masks->controlsNotifyMask&= ~stuff->affectControls;
	    masks->controlsNotifyMask|=(stuff->affectControls&stuff->controls);
	}
	if (stuff->affectNames) {
	    masks->namesNotifyMask&= ~stuff->affectNames;
	    masks->namesNotifyMask|=(stuff->affectNames&stuff->names);
	}
	if (stuff->affectCompatMap) {
	    masks->compatNotifyMask&= ~stuff->affectCompatMap;
	    masks->compatNotifyMask|= (stuff->affectCompatMap&stuff->compatMap);
	}
	if (stuff->affectBell)
	    masks->bellNotifyWanted = stuff->bell;
	if (stuff->affectSlowKey) {
	    masks->slowKeyNotifyMask&= ~stuff->affectSlowKey;
	    masks->slowKeyNotifyMask|= (stuff->affectSlowKey&stuff->slowKey);
	}
	if (stuff->affectIndicatorState) {
	    masks->iStateNotifyMask = ~stuff->affectIndicatorState;
	    masks->iStateNotifyMask |= 
			(stuff->affectIndicatorState&stuff->indicatorState);
	}
	if (stuff->affectIndicatorMap) {
	    masks->iMapNotifyMask = ~stuff->affectIndicatorMap;
	    masks->iMapNotifyMask |= 
			(stuff->affectIndicatorMap&stuff->indicatorMap);
	}
	if (stuff->affectAlternateSyms) {
	    masks->altSymsNotifyMask = ~stuff->affectAlternateSyms;
	    masks->altSymsNotifyMask |= 
			(stuff->affectAlternateSyms&stuff->alternateSyms);
	}
	return client->noClientException;
    }
    return BadAlloc;
}

/***====================================================================***/

static int
ProcXkbSendEvent(client)
    ClientPtr client;
{
    REQUEST(xkbSendEventReq);

    REQUEST_SIZE_MATCH(xkbSendEventReq);
    /* 8/4/93 (ef) -- XXX! Implement this */
    return BadImplementation;
}

/***====================================================================***/

static int
ProcXkbBell(client)
    ClientPtr client;
{
    REQUEST(xkbBellReq);
    DeviceIntPtr keybd;
    int base;
    int newPercent;
    pointer ctrl;
    void (*proc)();

    REQUEST_SIZE_MATCH(xkbBellReq);
    keybd= XkbLookupDevice(stuff->deviceSpec);
    if (!keybd) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }

    if (stuff->percent < -100 || stuff->percent > 100) {
	client->errorValue = XkbError2(0x1,stuff->percent);
	return BadValue;
    }
    if (stuff->bellClass == KbdFeedbackClass) {
	KbdFeedbackPtr	k;
	for (k=keybd->kbdfeed; k; k=k->next) {
	    if (k->ctrl.id == stuff->bellID)
		break;
	}
	if (!k) {
	    client->errorValue = XkbError2(0x2,stuff->bellID);
	    return BadValue;
	}
	base = k->ctrl.bell;
	proc = k->BellProc;
	ctrl = (pointer) &(k->ctrl);
    }
    else if (stuff->bellClass == BellFeedbackClass) {
	BellFeedbackPtr	b;
	for (b=keybd->bell; b; b=b->next) {
	    if (b->ctrl.id == stuff->bellID)
		break;
	}
	if (!b) {
	    client->errorValue = XkbError2(0x3,stuff->bellID);
	    return BadValue;
	}
	base = b->ctrl.percent;
	proc = b->BellProc;
	ctrl = (pointer) &(b->ctrl);
    }
    else {
	client->errorValue = XkbError2(0x4,stuff->bellClass);;
	return BadValue;
    }
    newPercent= (base*stuff->percent)/100;
    if (stuff->percent < 0)
         newPercent= base+newPercent;
    else newPercent= base-newPercent+stuff->percent;
    XkbHandleBell(keybd, newPercent, ctrl, stuff->bellClass, stuff->name);
    return Success;
}

/***====================================================================***/

static int
ProcXkbGetState(client)
    ClientPtr client;
{
    REQUEST(xkbGetStateReq);
    DeviceIntPtr	keybd;
    xkbGetStateReply	 rep;
    XkbStateRec		*xkb;
    int			n;

    REQUEST_SIZE_MATCH(xkbGetStateReq);
    keybd = XkbLookupDevice(stuff->deviceSpec);
    if ( !keybd ) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb= &keybd->key->xkbInfo->state;
    bzero(&rep,sizeof(rep));
    rep.type= X_Reply;
    rep.sequenceNumber= client->sequence;
    rep.length = 0;
    rep.deviceID = keybd->id;
    rep.mods = keybd->key->state&0xff;
    rep.baseMods = xkb->baseMods;
    rep.lockedMods = xkb->lockedMods;
    rep.latchedMods = xkb->latchedMods;
    rep.group = xkb->group;
    rep.baseGroup = xkb->baseGroup;
    rep.latchedGroup = xkb->latchedGroup;
    rep.lockedGroup = xkb->lockedGroup;
    rep.compatState = xkb->compatState;
    rep.unlockedMods = xkb->unlockedMods;
    rep.groupsUnlocked = xkb->groupsUnlocked;
    WriteToClient(client, sizeof(xkbGetStateReply), (char *)&rep);
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbLatchLockState(client)
    ClientPtr client;
{
    int status;
    DeviceIntPtr dev;
    XkbStateRec	oldState,*newState;
    CARD16 changed;
    REQUEST(xkbLatchLockStateReq);

    REQUEST_SIZE_MATCH(xkbLatchLockStateReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    status = Success;
    oldState= dev->key->xkbInfo->state;
    newState= &dev->key->xkbInfo->state;
    if ( stuff->affectModLocks ) {
	newState->lockedMods&= ~stuff->affectModLocks;
	newState->lockedMods|= (stuff->affectModLocks&stuff->modLocks);
    }
    if (( status == Success ) && stuff->lockGroup )
	newState->lockedGroup = stuff->groupLock;
    if (( status == Success ) && stuff->affectModLatches )
	status=XkbLatchModifiers(dev,stuff->affectModLatches,stuff->modLatches);
    if (( status == Success ) && stuff->latchGroup )
	status=XkbLatchGroup(dev,stuff->groupLatch);

    if ( status != Success )
	return status;

    XkbComputeDerivedState(dev->key->xkbInfo);
    dev->key->state= (dev->key->xkbInfo->lookupState&0xE0FF);

    changed = XkbStateChangedFlags(&oldState,&dev->key->xkbInfo->state);
    if (changed) {
	xkbStateNotify	sn;

	sn.keycode= 0;
	sn.eventType= 0;
	sn.requestMajor = XkbReqCode;
	sn.requestMinor = X_kbLatchLockState;
	sn.changed= changed;
	XkbSendStateNotify(dev,&sn);
	changed= XkbIndicatorsToUpdate(dev,changed);
        XkbUpdateIndicators(dev,changed,NULL);
    }
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbGetControls(client)
    ClientPtr client;
{
    int	status;
    REQUEST(xkbGetControlsReq);
    xkbGetControlsReply rep;
    XkbControlsRec	*xkb;
    DeviceIntPtr dev;
    register int n;

    REQUEST_SIZE_MATCH(xkbGetControlsReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = dev->key->xkbInfo->desc.controls;
    rep.type = X_Reply;
    rep.length = (sizeof(xkbGetControlsReply)-
		  sizeof(xGenericReply)) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.deviceID = ((DeviceIntPtr)dev)->id;
    rep.numGroups = xkb->numGroups;
    rep.internalMods = xkb->internalMods;
    rep.ignoreLockMods = xkb->ignoreLockMods;
    rep.enabledControls = xkb->enabledControls;
    rep.repeatDelay = xkb->repeatDelay;
    rep.repeatInterval = xkb->repeatInterval;
    rep.slowKeysDelay = xkb->slowKeysDelay;
    rep.debounceDelay = xkb->debounceDelay;
    rep.mouseKeysDelay = xkb->mouseKeysDelay;
    rep.mouseKeysInterval = xkb->mouseKeysInterval;
    rep.mouseKeysTimeToMax = xkb->mouseKeysTimeToMax;
    rep.mouseKeysMaxSpeed = xkb->mouseKeysMaxSpeed;
    rep.mouseKeysCurve = xkb->mouseKeysCurve;
    rep.mouseKeysDfltBtn = xkb->mouseKeysDfltBtn;
    rep.accessXTimeout = xkb->accessXTimeout;
    rep.accessXTimeoutMask = xkb->accessXTimeoutMask;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.internalMods, n);
	swaps(&rep.ignoreLockMods, n);
	swapl(&rep.enabledControls, n);
	swaps(&rep.repeatDelay, n);
	swaps(&rep.repeatInterval, n);
	swaps(&rep.slowKeysDelay, n);
	swaps(&rep.debounceDelay, n);
	swaps(&rep.mouseKeysDelay, n);
	swaps(&rep.mouseKeysInterval, n);
	swaps(&rep.mouseKeysTimeToMax, n);
	swaps(&rep.accessXTimeout, n);
	swapl(&rep.accessXTimeoutMask, n);
    }
    WriteToClient(client, sizeof(xkbGetControlsReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXkbSetControls(client)
    ClientPtr client;
{
    int status;
    DeviceIntPtr dev;
    XkbControlsRec *xkb,old;
    xkbControlsNotify	cn;
    REQUEST(xkbSetControlsReq);

    REQUEST_SIZE_MATCH(xkbSetControlsReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = dev->key->xkbInfo->desc.controls;
    old = *xkb;
    xkb->internalMods&=~stuff->affectInternalMods;
    xkb->internalMods|=(stuff->affectInternalMods&stuff->internalMods);
    xkb->ignoreLockMods&=~stuff->affectIgnoreLockMods;
    xkb->ignoreLockMods|=(stuff->affectIgnoreLockMods&stuff->ignoreLockMods);
    xkb->enabledControls&= ~stuff->affectEnabledControls;
    xkb->enabledControls|=(stuff->affectEnabledControls&stuff->enabledControls);
    if (stuff->changeControls&XkbRepeatKeysMask) {
	xkb->repeatDelay = stuff->repeatDelay;
	xkb->repeatInterval = stuff->repeatInterval;
    }
    if (stuff->changeControls&XkbSlowKeysMask)
	xkb->slowKeysDelay = stuff->slowKeysDelay;
    if (stuff->changeControls&XkbBounceKeysMask)
	xkb->debounceDelay = stuff->debounceDelay;
    if (stuff->changeControls&XkbMouseKeysMask) {
	xkb->mouseKeysDelay = stuff->mouseKeysDelay;
	xkb->mouseKeysInterval = stuff->mouseKeysInterval;
	xkb->mouseKeysDfltBtn = stuff->mouseKeysDfltBtn;
    }
    if (stuff->changeControls&XkbMouseKeysAccelMask) {
	xkb->mouseKeysTimeToMax = stuff->mouseKeysTimeToMax;
	xkb->mouseKeysMaxSpeed = stuff->mouseKeysMaxSpeed;
	xkb->mouseKeysCurve = stuff->mouseKeysCurve;
	dev->key->xkbInfo->mouseKeysCurve=
		1.5+(((double)stuff->mouseKeysCurve)*0.0015);
	dev->key->xkbInfo->mouseKeysCurveFactor=
		((double)stuff->mouseKeysMaxSpeed)/
			pow((double)stuff->mouseKeysTimeToMax,
					dev->key->xkbInfo->mouseKeysCurve);
    }
    if (stuff->changeControls&XkbAccessXTimeoutMask) {
	xkb->accessXTimeout = stuff->accessXTimeout;
	xkb->accessXTimeoutMask = stuff->accessXTimeoutMask;
    }
    DDXChangeXkbControls(dev,&old,xkb);
    if (XkbComputeControlsNotify(dev,&old,xkb,&cn)) {
	cn.keycode= 0;
	cn.eventType = 0;
	cn.requestMajor = XkbReqCode;
	cn.requestMinor = X_kbSetControls;
	XkbSendControlsNotify(dev,&cn);
    }
    return client->noClientException;
}

/***====================================================================***/

static int
XkbSizeKeyTypes(xkb,firstKeyType,nKeyTypes)
    XkbDescRec *xkb;
    CARD8 firstKeyType;
    CARD8 nKeyTypes;
{
    XkbKeyTypeRec 	*map;
    int			i,len;

    len= 0;
    map= &xkb->map->keyTypes[firstKeyType];
    for (i=0;i<nKeyTypes;i++,map++){
	len+= sizeof(xkbKeyTypeWireDesc)+(((XkbKTMapWidth(map)+3)/4)*4);
	if (map->preserve)
	    len+= (((XkbKTMapWidth(map)+3)/4)*4);
    }
    return len;
}

static char *
XkbWriteKeyTypes(xkb,rep,buf,client)
    XkbDescRec *xkb;
    xkbGetMapReply *rep;
    char *buf;
    ClientPtr client;
{
    XkbKeyTypeRec 	*map;
    int			i,n;
    xkbKeyTypeWireDesc	*desc;

    map= &xkb->map->keyTypes[rep->firstKeyType];
    for (i=0;i<rep->nKeyTypes;i++,map++) {
	desc= (xkbKeyTypeWireDesc *)buf;
	desc->flags = (map->preserve?xkb_KTHasPreserve:0);
	desc->mask = map->mask;
	desc->groupWidth = map->groupWidth;
	desc->mapWidth = XkbKTMapWidth(map);
	buf= (char *)&desc[1];
	memcpy(buf,map->map,XkbKTMapWidth(map));
	buf+= (((XkbKTMapWidth(map)+3)/4)*4);
	if (map->preserve) {
	    memcpy(buf,map->preserve,XkbKTMapWidth(map));
	    buf+= (((XkbKTMapWidth(map)+3)/4)*4);
	}
    }
    return buf;

}

static int
XkbSizeKeySyms(xkb,firstKey,nKeys,nSymsRtrn)
    XkbDescRec *xkb;
    CARD8 firstKey;
    CARD8 nKeys;
    CARD16 *nSymsRtrn;
{
    XkbSymMapRec	*symMap;
    XkbKeyTypeRec 	*keyType;
    int			i,len;
    unsigned		nSyms;

    len= nKeys*sizeof(xkbSymMapWireDesc);
    symMap = &xkb->map->keySymMap[firstKey];
    for (i=nSyms=0;i<nKeys;i++,symMap++) {
	keyType = &xkb->map->keyTypes[symMap->ktIndex];
	nSyms+= XkbNumGroups(symMap->groupInfo)*keyType->groupWidth;
    }
    len+= nSyms*sizeof(KeySym);
    *nSymsRtrn = nSyms;
    return len;
}

static char *
XkbWriteKeySyms(xkb,rep,buf,client)
    XkbDescRec *xkb;
    xkbGetMapReply *rep;
    char *buf;
    ClientPtr client;
{
register KeySym 	*pSym;
XkbSymMapRec		*symMap;
XkbKeyTypeRec		*keyType;
xkbSymMapWireDesc	*outMap;
register int		 i;

    symMap = &xkb->map->keySymMap[rep->firstKeySym];
    for (i=0;i<rep->nKeySyms;i++,symMap++) {
	keyType = &xkb->map->keyTypes[symMap->ktIndex];
	outMap = (xkbSymMapWireDesc *)buf;
	outMap->ktIndex = symMap->ktIndex;
	outMap->groupInfo = symMap->groupInfo;
	outMap->nSyms = XkbNumGroups(symMap->groupInfo)*keyType->groupWidth;
if (xkbDebugFlags) {
    ErrorF("key %d has %d symbols\n",rep->firstKeySym+i,outMap->nSyms);
    if (outMap->nSyms>4) {
	ErrorF("    ktIndex= %d, groupWidth= %d, nGroups= %d\n",symMap->ktIndex,
			keyType->groupWidth,XkbNumGroups(symMap->groupInfo));
    }
}
	buf= (char *)&outMap[1];
	pSym = &xkb->map->syms[symMap->offset];
	memcpy(buf,pSym,outMap->nSyms*sizeof(KeySym));
	if (client->swapped) {
	    register int n,nSyms= outMap->nSyms;
	    swaps(&outMap->nSyms,n);
	    while (nSyms-->0) {
		swapl(buf,n);
		buf+= sizeof(KeySym);
	    }
	}
	else buf+= outMap->nSyms*sizeof(KeySym);
    }
    return buf;
}

static int
XkbSizeKeyActions(xkb,firstKey,nKeys,nActsRtrn)
    XkbDescRec	*xkb;
    CARD8	 firstKey;
    CARD8	 nKeys;
    CARD16	*nActsRtrn;
{
    int			i,len,nActs;

    for (nActs=i=0;i<nKeys;i++) {
	if (xkb->server->keyActions[i+firstKey]!=0)
	    nActs+= XkbKeyNumActions(xkb,i+firstKey);
    }
    len= (((nKeys+3)/4)*4)+(nActs*sizeof(XkbAction));
    *nActsRtrn = nActs;
    return len;
}

static char *
XkbWriteKeyActions(xkb,rep,buf,client)
    XkbDescRec		*xkb;
    xkbGetMapReply	*rep;
    char		*buf;
    ClientPtr		client;
{
    int			i,ndx,n;
    CARD8		*numDesc;
    XkbAction		*actDesc;

    numDesc = (CARD8 *)buf;
    for (i=0;i<rep->nKeyActions;i++) {
	if (xkb->server->keyActions[i+rep->firstKeyAction]==0)
	     numDesc[i] = 0;
	else numDesc[i] = XkbKeyNumActions(xkb,(i+rep->firstKeyAction));
    }
    buf+= ((rep->nKeyActions+3)/4)*4;

    actDesc = (XkbAction *)buf;
    for (i=0;i<rep->nKeyActions;i++) {
	if (xkb->server->keyActions[i+rep->firstKeyAction]!=0) {
	    n = XkbKeyNumActions(xkb,(i+rep->firstKeyAction));
	    memcpy(actDesc,XkbKeyActionsPtr(xkb,(i+rep->firstKeyAction)),
							n*sizeof(XkbAction));
	    if (client->swapped) {
		register int j;
		for (j=0;j<n;j++) {
		    swapl(&actDesc[j],n);
		}
	    }
	    actDesc+= n;
	}
    }
    buf = (char *)actDesc;
    return buf;
}

static int
XkbSendMap(client,keybd,rep)
    ClientPtr		client;
    DeviceIntPtr	keybd;
    xkbGetMapReply	*rep;
{
XkbDescRec	*xkb;
int		i,n;
char		*desc,*start;

    xkb= &keybd->key->xkbInfo->desc;
    rep->length= XkbSizeKeyTypes(xkb,rep->firstKeyType,rep->nKeyTypes)/4;
    rep->length+= XkbSizeKeySyms(xkb,rep->firstKeySym,rep->nKeySyms,
						    &rep->totalSyms)/4;
    rep->length+= XkbSizeKeyActions(xkb,rep->firstKeyAction,rep->nKeyActions,
						    &rep->totalActions)/4;
    rep->length+= ((rep->nKeyBehaviors*sizeof(XkbAction))+3)/4;
    start= desc= (char *)ALLOCATE_LOCAL(rep->length*4);
    if (!start)
	return BadAlloc;
    if ( rep->nKeyTypes>0 )
	desc = XkbWriteKeyTypes(xkb,rep,desc,client);
    if ( rep->nKeySyms>0 )
	desc = XkbWriteKeySyms(xkb,rep,desc,client);
    if ( rep->nKeyActions>0 )
	desc = XkbWriteKeyActions(xkb,rep,desc,client);
    if ( rep->nKeyBehaviors>0 ) {
	XkbAction	*from;
	XkbAction	*to= (XkbAction *)desc;
	n = rep->firstKeyBehavior;
	from= &xkb->server->keyBehaviors[n];
	for (i=0;i<rep->nKeyBehaviors;i++,to++,from++) {
	    *to = *from;
	    if ( client->swapped ) {
		swaps(&to->type,n);
	    }
	}
	desc= (char *)to;
    }
    if ((desc-start)!=(rep->length*4)) {
	ErrorF("BOGUS LENGTH in write keyboard desc, expected %d, got %d\n",
					rep->length*4, desc-start);
    }
    WriteToClient(client, sizeof(*rep), (char *)rep);
    WriteToClient(client, rep->length*4, start);
    DEALLOCATE_LOCAL((char *)start);
    return client->noClientException;
}

static int
ProcXkbGetMap(client)
    ClientPtr client;
{
    REQUEST(xkbGetMapReq);
    DeviceIntPtr	 keybd;
    xkbGetMapReply	 rep;
    XkbDescRec		*xkb;
    int			 n;

    REQUEST_SIZE_MATCH(xkbGetMapReq);
    keybd = XkbLookupDevice(stuff->deviceSpec);
    if (!keybd) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb= &keybd->key->xkbInfo->desc;
    bzero(&rep,sizeof(rep));
    rep.type= X_Reply;
    rep.sequenceNumber= client->sequence;
    rep.length = 0;
    rep.deviceID = keybd->id;
    rep.present = stuff->partial|stuff->full;
    if ( stuff->full&XkbKeyTypesMask ) {
	rep.firstKeyType = 0;
	rep.nKeyTypes = xkb->map->nKeyTypes;
    }
    else if (stuff->partial&XkbKeyTypesMask) {
	if (stuff->firstKeyType+stuff->nKeyTypes>xkb->map->nKeyTypes) {
	    client->errorValue = XkbError4(0x01,xkb->map->nKeyTypes,
					stuff->firstKeyType,stuff->nKeyTypes);
	    return BadValue;
	}
	rep.firstKeyType = stuff->firstKeyType;
	rep.nKeyTypes = stuff->nKeyTypes;
    }
    else rep.nKeyTypes = 0;
    rep.totalKeyTypes = xkb->map->nKeyTypes;

    if ( stuff->full&XkbKeySymsMask ) {
	rep.firstKeySym = xkb->minKeyCode;
	rep.nKeySyms = XkbNumKeys(xkb);
    }
    else if (stuff->partial&XkbKeySymsMask) {
	if (stuff->firstKeySym+stuff->nKeySyms-1>xkb->maxKeyCode) {
	    client->errorValue = XkbError4(0x02,stuff->firstKeySym,
					stuff->nKeySyms,xkb->maxKeyCode);
	    return BadValue;
	}
	else if (stuff->firstKeySym<xkb->minKeyCode) {
	    client->errorValue = XkbError3(0x03,stuff->firstKeySym,
							xkb->minKeyCode);
	    return BadValue;
	}
	rep.firstKeySym = stuff->firstKeySym;
	rep.nKeySyms = stuff->nKeySyms;
    }
    else rep.nKeySyms = 0;

    if ( stuff->full&XkbKeyActionsMask ) {
	rep.firstKeyAction = xkb->minKeyCode;
	rep.nKeyActions = XkbNumKeys(xkb);
    }
    else if (stuff->partial&XkbKeyActionsMask) {
	if (stuff->firstKeyAction+stuff->nKeyActions-1>xkb->maxKeyCode) {
	    client->errorValue = XkbError4(0x04,stuff->firstKeyAction,
					stuff->nKeyActions,xkb->maxKeyCode);
	    return BadValue;
	}
	else if ( stuff->firstKeyAction<xkb->minKeyCode ) {
	    client->errorValue = XkbError3(0x05,stuff->firstKeyAction,
							xkb->minKeyCode);
	    return BadValue;
	}
	rep.firstKeyAction = stuff->firstKeyAction;
	rep.nKeyActions = stuff->nKeyActions;
    }
    else rep.nKeyActions = 0;

    n= XkbNumKeys(xkb);
    if ( stuff->full&XkbKeyBehaviorsMask ) {
	rep.firstKeyBehavior = xkb->minKeyCode;
	rep.nKeyBehaviors = n;
    }
    else if (stuff->partial&XkbKeyBehaviorsMask) {
	if ((stuff->firstKeyBehavior<xkb->minKeyCode)||
	      (stuff->firstKeyBehavior+stuff->nKeyBehaviors-1>xkb->maxKeyCode)){
	    client->errorValue = XkbError4(0x03,xkb->maxKeyCode,
						stuff->firstKeyBehavior,
						stuff->nKeyBehaviors);
	    return BadValue;
	}
	rep.firstKeyBehavior = stuff->firstKeyBehavior;
	rep.nKeyBehaviors = stuff->nKeyBehaviors;
    }
    else rep.nKeyBehaviors = 0;
    return XkbSendMap(client,keybd,&rep);
}

/***====================================================================***/

static int
CheckKeyTypes(xkb,req,wireRtrn,nMapsRtrn,mapWidthRtrn)
    XkbDescRec	 	 *xkb;
    xkbSetMapReq	 *req;
    xkbKeyTypeWireDesc	**wireRtrn;
    int			 *nMapsRtrn;
    CARD8		 *mapWidthRtrn;
{
int	nMaps,width;
register int i,n;
register CARD8 *map;
register CARD8 *preserve;
register xkbKeyTypeWireDesc	*wire = *wireRtrn;

    if (req->firstKeyType>xkb->map->nKeyTypes+1) {
	*nMapsRtrn = XkbError3(0x01,req->firstKeyType,xkb->map->nKeyTypes);
	return 0;
    }
    if (req->resize&XkbKeyTypesMask)
	nMaps = req->firstKeyType+req->nKeyTypes;
    else if (req->present&XkbKeyTypesMask) {
	nMaps = xkb->map->nKeyTypes;
	if ((req->firstKeyType+req->nKeyTypes)>nMaps) {
	    *nMapsRtrn = req->firstKeyType+req->nKeyTypes;
	    return 0;
	}
    }
    else {
	*nMapsRtrn = xkb->map->nKeyTypes;
	for (i=0;i<xkb->map->nKeyTypes;i++) {
	    mapWidthRtrn[i] = xkb->map->keyTypes[i].groupWidth;
	}
	return 1;
    }

    for (i=0;i<req->firstKeyType;i++) {
	mapWidthRtrn[i] = xkb->map->keyTypes[i].groupWidth;
    }
    for (i=0;i<req->nKeyTypes;i++) {
	/* 8/4/93 (ef) -- XXX!  check for ONE_LEVEL, TWO_LEVEL and KEYPAD */
	/*			group width here. */
	width = (wire->mask&0xff)+1;
	map = (CARD8 *)&wire[1];
	if (wire->flags&xkb_KTHasPreserve)
	     preserve = &map[width];
	else preserve = NULL;
	if (wire->groupWidth<1) {
	    *nMapsRtrn= XkbError2(0x04,i);
	    return 0;
	}
	for (n=0;n<width;n++) {
	    if ((n&wire->mask)==n) {
		if (map[n]>=wire->groupWidth) {
		    *nMapsRtrn= XkbError4(0x07,n,map[n],wire->groupWidth);
		    return 0;
		}
		if (preserve&&(preserve[n]&(~wire->mask))) {
		    *nMapsRtrn= XkbError4(0x08,n,preserve[n],wire->mask);
		    return 0;
		}
	    }
	}
	mapWidthRtrn[i+req->firstKeyType] = wire->groupWidth;
	wire= (xkbKeyTypeWireDesc *)&map[(((width*(preserve?2:1))+3)/4)*4];
    }
    for (i=req->firstKeyType+req->nKeyTypes;i<nMaps;i++) {
	mapWidthRtrn[i] = xkb->map->keyTypes[i].groupWidth;
    }
    *nMapsRtrn = nMaps;
    *wireRtrn = wire;
    return 1;
}

static int
CheckKeySyms(xkb,req,nKeyTypes,mapWidths,symsPerKey,wireRtrn,errorRtrn)
XkbDescRec		 *xkb;
xkbSetMapReq	 *req;
int			  nKeyTypes;
CARD8			 *mapWidths;
CARD16			 *symsPerKey;
xkbSymMapWireDesc	**wireRtrn;
int			 *errorRtrn;
{
register int		 i,tmp;
XkbSymMapRec		*map;
xkbSymMapWireDesc	*wire = *wireRtrn;

    if (!(XkbKeySymsMask&req->present))
	return 1;
    if (req->resize&XkbKeySymsMask) {
        *errorRtrn = XkbError2(0x10,req->resize);
        return 0;
    }
    if (req->firstKeySym<xkb->minKeyCode) {
	*errorRtrn = XkbError3(0x11,req->firstKeySym,xkb->minKeyCode);
	return 0;
    }
    if ((req->firstKeySym+req->nKeySyms-1)>xkb->maxKeyCode) {
	*errorRtrn = XkbError4(0x12,req->firstKeySym,req->nKeySyms,
							xkb->maxKeyCode+1);
	return 0;
    }
    map = &xkb->map->keySymMap[xkb->minKeyCode];
    for (i=xkb->minKeyCode;i<req->firstKeySym;i++,map++) {
	if (map->ktIndex>=nKeyTypes) {
	    *errorRtrn = XkbError3(0x13,i,map->ktIndex);
	    return 0;
	}
	symsPerKey[i] = XkbKeyNumSyms(xkb,i);
    }
    for (i=0;i<req->nKeySyms;i++) {
	KeySym *pSyms;
	register int nG;
	if (wire->ktIndex>=nKeyTypes) {
	    *errorRtrn= XkbError3(0x14,i+req->firstKeySym,wire->ktIndex);
	    return 0;
	}
	nG = XkbNumGroups(wire->groupInfo);
	if ((nG<1)||(nG>8)) {
	    *errorRtrn = XkbError3(0x15,i,nG);
	    return 0;
	}
	symsPerKey[i+req->firstKeySym] = mapWidths[wire->ktIndex]*nG;
	if (symsPerKey[i+req->firstKeySym]!=wire->nSyms) {
	    *errorRtrn = XkbError4(0x16,i+req->firstKeySym,wire->nSyms,
					symsPerKey[i+req->firstKeySym]);
	    return 0;
	}
	pSyms = (KeySym *)&wire[1];
	wire = (xkbSymMapWireDesc *)&pSyms[wire->nSyms];
    }

    map = &xkb->map->keySymMap[i];
    for (;i<=xkb->maxKeyCode;i++,map++) {
	if (map->ktIndex>=nKeyTypes) {
	    *errorRtrn = XkbError3(0x15,i,map->ktIndex);
	    return 0;
	}
	symsPerKey[i] = XkbKeyNumSyms(xkb,i);
    }
    *wireRtrn = wire;
    return 1;
}

static int
CheckKeyActions(xkb,req,nKeyTypes,mapWidths,symsPerKey,wireRtrn,nActsRtrn)
    XkbDescRec		 *xkb;
    xkbSetMapReq	 *req;
    int			  nKeyTypes;
    CARD8		 *mapWidths;
    CARD16		 *symsPerKey;
    CARD8		**wireRtrn;
    int			 *nActsRtrn;
{
int			 nActs;
CARD8	 		*wire = *wireRtrn;
XkbAction		*acts;
register int		 i,tmp;

    if (!(XkbKeyActionsMask&req->present))
	return 1;
    if (req->resize&XkbKeyActionsMask) {
        *nActsRtrn = XkbError2(0x20,req->resize);
        return 0;
    }
    if (req->firstKeyAction<xkb->minKeyCode) {
	*nActsRtrn = XkbError3(0x21,req->firstKeyAction,xkb->minKeyCode);
	return 0;
    }
    if ((req->firstKeyAction+req->nKeyActions-1)>xkb->maxKeyCode) {
	*nActsRtrn = XkbError4(0x22,req->firstKeyAction,req->nKeyActions,
							xkb->maxKeyCode);
	return 0;
    }
    for (nActs=i=0;i<req->nKeyActions;i++) {
	if (wire[0]!=0) {
	    if (wire[0]==symsPerKey[i+req->firstKeyAction])
		nActs+= wire[0];
	    else {
		*nActsRtrn = XkbError3(0x23,i+req->firstKeyAction,wire[0]);
		return 0;
	    }
	}
	wire++;
    }
    if (req->nKeyActions%4)
	wire+= 4-(req->nKeyActions%4);
    /* 8/4/93 (ef) -- XXX! check for legal actions here */
    *wireRtrn = (CARD8 *)(((XkbAction *)wire)+nActs);
    *nActsRtrn = nActs;
    return 1;
}

static int
CheckKeyBehaviors(xkb,req,newBehaviors,wireRtrn,errorRtrn)
    XkbDescRec	 	 *xkb;
    xkbSetMapReq	 *req;
    XkbAction		 *newBehaviors;
    XkbAction		**wireRtrn;
    int			 *errorRtrn;
{
register XkbAction	*wire = *wireRtrn;
register int	 	 i;

    if (!(XkbKeyBehaviorsMask&req->present)) {
	i = XkbNumKeys(xkb);
	memcpy(newBehaviors,xkb->server->keyBehaviors,(i*sizeof(XkbAction)));
	return 1;
    }
    if (XkbKeyBehaviorsMask&req->resize) {
        *errorRtrn = XkbError2(0x30,req->resize);
        return 0;
    }
    if (req->firstKeyBehavior<xkb->minKeyCode) {
	*errorRtrn = XkbError3(0x31,req->firstKeyBehavior,xkb->minKeyCode);
	return 0;
    }
    if ((req->firstKeyBehavior+req->nKeyBehaviors-1)>xkb->maxKeyCode) {
	*errorRtrn = XkbError4(0x32,req->firstKeyBehavior,req->nKeyBehaviors,
							xkb->maxKeyCode);
	return 0;
    }
    i = req->firstKeyBehavior;
    memcpy(newBehaviors,xkb->server->keyBehaviors,i*sizeof(XkbAction));
    /* 8/4/93 (ef) -- XXX! swap here. check for legal behaviors here */
    memcpy(&newBehaviors[req->firstKeyBehavior],wire,
					req->nKeyBehaviors*sizeof(XkbAction));
    i=XkbNumKeys(xkb)-req->nKeyBehaviors;
    if (i>0)
	memcpy(&newBehaviors[xkb->maxKeyCode-i+1],
				&xkb->server->keyBehaviors[xkb->maxKeyCode-i+1],
				i*sizeof(XkbAction));
    *wireRtrn = &wire[req->nKeyBehaviors];
    return 1;
}

static pointer
SetKeyTypes(xkb,req,wire,pChanges)
    XkbDescRec		*xkb;
    xkbSetMapReq	*req;
    xkbKeyTypeWireDesc 	*wire;
    xkbMapNotify	*pChanges;
{
register int 	 i;
int		 width;
CARD8		*map,*preserve;

    if ((req->firstKeyType+req->nKeyTypes)>xkb->map->nKeyTypes) {
	XkbKeyTypeRec	*pNew;
	width = req->firstKeyType+req->nKeyTypes;
	if (xkb->map->keyTypes[0].flags&XKB_KT_DONT_FREE_STRUCT) {
	    pNew = (XkbKeyTypeRec *)Xcalloc(width*sizeof(XkbKeyTypeRec));
	    if (!pNew)
		return NULL;
	    memcpy(pNew,xkb->map->keyTypes,
				req->firstKeyType*sizeof(XkbKeyTypeRec));
	    pNew->flags&= ~XKB_KT_DONT_FREE_STRUCT;
	    xkb->map->nKeyTypes = width;
	    xkb->map->keyTypes= pNew;
	}
	else {
	    pNew = (XkbKeyTypeRec *)Xrealloc(xkb->map->keyTypes,
						width*sizeof(XkbKeyTypeRec));
	    if (!pNew)
		return NULL;
	    bzero(&pNew[xkb->map->nKeyTypes],
			(width-xkb->map->nKeyTypes)*sizeof(XkbKeyTypeRec));
	    xkb->map->nKeyTypes = width;
	    xkb->map->keyTypes= pNew;
	}
    }

    for (i=0;i<req->nKeyTypes;i++) {
	XkbKeyTypeRec	*pOld;

	pOld = &xkb->map->keyTypes[i+req->firstKeyType];
	if (wire->groupWidth!=pOld->groupWidth) {
	    _XkbResizeKeyType(xkb,i+req->firstKeyType,wire->groupWidth);
	    pOld = &xkb->map->keyTypes[i+req->firstKeyType];
	}
	width = (wire->mask&0xff)+1;
	map = (CARD8 *)&wire[1];
	if (wire->flags&xkb_KTHasPreserve)
	     preserve = &map[width];
	else preserve = NULL;

	if (pOld->flags&XKB_KT_DONT_FREE_MAP)
	    pOld->map = NULL;
	if (pOld->preserve&&(pOld->flags&XKB_KT_DONT_FREE_PRESERVE))
	    pOld->preserve = NULL;

	if (!pOld->map)
	    pOld->map = (CARD8 *)Xcalloc(width);
	else if (XkbKTMapWidth(pOld)<width)
	    pOld->map = (CARD8 *)Xrealloc(pOld->map,width);
	if (!pOld->map)
	    return NULL;
	memcpy(pOld->map,map,width);

	if (preserve) {
	    if (!pOld->preserve)
		pOld->preserve = (CARD8 *)Xcalloc(width);
	    else if (XkbKTMapWidth(pOld)<width)
		pOld->preserve = (CARD8 *)Xrealloc(pOld->preserve,width);
	    if (!pOld->preserve)
		return NULL;
	    memcpy(pOld->preserve,preserve,width);
	}
	else if (pOld->preserve) {
	    Xfree(pOld->preserve);
	    pOld->preserve = NULL;
	}
	pOld->flags = (i+req->firstKeyType==0?0:XKB_KT_DONT_FREE_MAP);
	pOld->mask = wire->mask;
	pOld->groupWidth = wire->groupWidth;
	wire = (xkbKeyTypeWireDesc *)&map[(((width*(preserve?2:1))+3)/4)*4];
    }
    if (pChanges->changed&XkbKeyTypesMask) {
	pChanges->firstKeyType = 0;
	pChanges->nKeyTypes = xkb->map->nKeyTypes;
    }
    else {
	pChanges->changed|= XkbKeyTypesMask;
	pChanges->firstKeyType = req->firstKeyType;
	pChanges->nKeyTypes = req->nKeyTypes;
    }
    return (pointer)wire;
}

static pointer
SetKeySyms(xkb,req,wire,pChanges)
    XkbDescRec		*xkb;
    xkbSetMapReq	*req;
    xkbSymMapWireDesc	*wire;
    xkbMapNotify	*pChanges;
{
register int i;
XkbSymMapRec *oldMap;
KeySym *newSyms,*pSyms;

    oldMap = &xkb->map->keySymMap[req->firstKeySym];
    for (i=0;i<req->nKeySyms;i++,oldMap++) {
	newSyms = _XkbNewSymsForKey(xkb,i+req->firstKeySym,wire->nSyms);
	pSyms = (KeySym *)&wire[1];
	memcpy(newSyms,pSyms,wire->nSyms*sizeof(KeySym));
	oldMap->ktIndex = wire->ktIndex;
	oldMap->groupInfo = wire->groupInfo;
	wire= (xkbSymMapWireDesc *)&pSyms[wire->nSyms];
    }
    if (pChanges->changed&XkbKeySymsMask) {
	pChanges->firstKeySym = xkb->minKeyCode;
	pChanges->nKeySyms = xkb->maxKeyCode-xkb->minKeyCode+1;
    }
    else {
	pChanges->changed|= XkbKeySymsMask;
	pChanges->firstKeySym = req->firstKeySym;
	pChanges->nKeySyms = req->nKeySyms;
    }
    return (pointer)wire;
}

static pointer
SetKeyActions(xkb,req,wire,pChanges)
    XkbDescRec		*xkb;
    xkbSetMapReq	*req;
    CARD8		*wire;
    xkbMapNotify	*pChanges;
{
register int	 i;
CARD8		*nActs = wire;
XkbAction	*newActs;
    
    wire+= ((req->nKeyActions+3)/4)*4;;
    for (i=0;i<req->nKeyActions;i++) {
	if (nActs[i]==0)
	    xkb->server->keyActions[i+req->firstKeyAction]= 0;
	else {
	    newActs= _XkbNewActionsForKey(xkb,i+req->firstKeyAction,nActs[i]);
	    memcpy((char *)newActs,(char *)wire,nActs[i]*sizeof(XkbAction));
	    wire+= nActs[i]*sizeof(XkbAction);
	}
    }
    if (pChanges->changed&XkbKeyActionsMask) {
	pChanges->firstKeyAction = xkb->minKeyCode;
	pChanges->nKeyActions = XkbNumKeys(xkb);
    }
    else {
	pChanges->changed|= XkbKeyActionsMask;
	pChanges->firstKeyAction = req->firstKeyAction;
	pChanges->nKeyActions = req->nKeyActions;
    }
    return (pointer)wire;
}

static pointer
SetKeyBehaviors(xkb,req,wire,pChanges)
    XkbDescRec		*xkb;
    xkbSetMapReq	*req;
    XkbAction		*wire;
    xkbMapNotify	*pChanges;
{
    memcpy((char*)&xkb->server->keyBehaviors[req->firstKeyBehavior],
		(char *)wire, req->nKeyBehaviors*sizeof(XkbAction));
    wire+= req->nKeyBehaviors;
    if (pChanges->changed&XkbKeyBehaviorsMask) {
	pChanges->firstKeyBehavior = xkb->minKeyCode;
	pChanges->nKeyBehaviors = XkbNumKeys(xkb);
    }
    else {
	pChanges->changed|= XkbKeyBehaviorsMask;
	pChanges->firstKeyBehavior = req->firstKeyBehavior;
	pChanges->nKeyBehaviors = req->nKeyBehaviors;
    }
    return (pointer)wire;
}


static int
ProcXkbSetMap(client)
    ClientPtr client;
{
    DeviceIntPtr	 dev;
    XkbDescRec		*xkb;
    xkbMapNotify	 changes;
    REQUEST(xkbSetMapReq);
    int	nKeyTypes,nActions,nKeySyms,error;
    char	*tmp;
    CARD8	 mapWidths[256];
    CARD16	 symsPerKey[256];
    XkbAction	 newBehaviors[256];

    REQUEST_AT_LEAST_SIZE(xkbSetMapReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if  (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;

    tmp = (char *)&stuff[1];
    if (!CheckKeyTypes(xkb,stuff,(xkbKeyTypeWireDesc **)&tmp,
						&nKeyTypes,mapWidths)) {
	client->errorValue = nKeyTypes;
	return BadValue;
    }
    if (!CheckKeySyms(xkb,stuff,nKeyTypes,mapWidths,symsPerKey,
						(XkbSymMapRec **)&tmp,&error)) {
	client->errorValue = error;
	return BadValue;
    }

    if (!CheckKeyActions(xkb,stuff,nKeyTypes,mapWidths,symsPerKey,
						(CARD8 **)&tmp,&nActions)){
	client->errorValue = nActions;
	return BadValue;
    }

    if (!CheckKeyBehaviors(xkb,stuff,newBehaviors,(XkbAction **)&tmp,&error)) {
	client->errorValue = error;
	return BadValue;
    }

    if (((tmp-((char *)stuff))/4)!=stuff->length) {
	ErrorF("Internal error! Bad length in XkbSetMap (after check)\n");
	client->errorValue = tmp-((char *)&stuff[1]);
	return BadLength;
    }
    bzero(&changes,sizeof(changes));
    tmp = (char *)&stuff[1];
    if (stuff->present&XkbKeyTypesMask) {
	tmp = SetKeyTypes(xkb,stuff,(xkbKeyTypeWireDesc *)tmp,&changes);
	if (!tmp)	goto allocFailure;
    }
    if (stuff->present&XkbKeySymsMask) {
	tmp = SetKeySyms(xkb,stuff,(xkbSymMapWireDesc *)tmp,&changes);
	if (!tmp)	goto allocFailure;
    }
    if (stuff->present&XkbKeyActionsMask) {
	tmp = SetKeyActions(xkb,stuff,(CARD8 *)tmp,&changes);
	if (!tmp)	goto allocFailure;
    }
    if (stuff->present&XkbKeyBehaviorsMask) {
	tmp = SetKeyBehaviors(xkb,stuff,(XkbAction *)tmp,&changes);
	if (!tmp)	goto allocFailure;
    }
    if (((tmp-((char *)stuff))/4)!=stuff->length) {
	ErrorF("Internal error! Bad length in XkbSetMap (after set)\n");
	client->errorValue = tmp-((char *)&stuff[1]);
	return BadLength;
    }
    if (changes.changed)
	XkbSendMapNotify(dev,&changes);

    XkbUpdateCoreDescription(dev);
    return client->noClientException;
allocFailure:
    return BadAlloc;
}

/***====================================================================***/

static int
ProcXkbGetCompatMap(client)
    ClientPtr client;
{
    REQUEST(xkbGetCompatMapReq);
    xkbGetCompatMapReply rep;
    DeviceIntPtr dev;
    XkbDescRec *xkb;
    XkbCompatRec *compat;
    int		 size,nMods;

    REQUEST_SIZE_MATCH(xkbGetCompatMapReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;
    compat= xkb->compat;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.firstSym = stuff->firstSym;
    rep.nSyms = stuff->nSyms;
    if (stuff->getAllSyms) {
	rep.firstSym = 0;
	rep.nSyms = compat->nSymInterpret;
    }
    else if ((stuff->nSyms>0)&&
		((stuff->firstSym+stuff->nSyms-1)>=compat->nSymInterpret)) {
	client->errorValue = XkbError2(0x05,compat->nSymInterpret);
	return BadValue;
    }
    rep.mods= stuff->mods;
    nMods= 0;
    if (stuff->mods!=0) {
	register int i,bit;
	for (i=0,bit=1;i<8;i++,bit<<=1) {
	    if (stuff->mods&bit)
		nMods++;
	}
    }

    rep.deviceID = dev->id;
    rep.nTotalSyms = compat->nSymInterpret;

    size = ((((nMods*sizeof(XkbModCompatRec))+3)/4)*4);
    size+= (rep.nSyms*sizeof(XkbSymInterpretRec));
    rep.length= size;
    if (size) {
	char	*data;
	data = (char *)ALLOCATE_LOCAL(size);
	if (data) {
	    register int i,bit;
	    XkbModCompatRec *mod;
	    XkbSymInterpretRec *sym= &compat->symInterpret[rep.firstSym];
	    XkbSymInterpretRec *wire = (XkbSymInterpretRec *)data;
	    for (i=0;i<rep.nSyms;i++,sym++,wire++) {
		*wire= *sym;
	    }
	    mod = (XkbModCompatRec *)wire;
	    for (i=0,bit=1;i<8;i++,bit<<=1) {
		if (rep.mods&bit) {
		    *mod= compat->modCompat[i];
		    mod++;
		}
	    }
	    /* 8/4/93 (ef) -- XXX! deal with swapping */
	    WriteToClient(client, sizeof(xkbGetCompatMapReply), (char *)&rep);
	    WriteToClient(client, size, data);
	    DEALLOCATE_LOCAL((char *)data);
	}
	else return BadAlloc;
    }
    else {
	/* 8/4/93 (ef) -- XXX! deal with swapping */
	WriteToClient(client, sizeof(xkbGetCompatMapReply), (char *)&rep);
    }
    return client->noClientException;
}

static int
ProcXkbSetCompatMap(client)
    ClientPtr client;
{
    REQUEST(xkbSetCompatMapReq);
    DeviceIntPtr dev;
    XkbSrvInfoRec *xkb;
    XkbCompatRec *compat;
    char	*data;
    int		 nMods;

    REQUEST_AT_LEAST_SIZE(xkbSetCompatMapReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    data = (char *)&stuff[1];
    xkb = dev->key->xkbInfo;
    compat= xkb->desc.compat;
    if ((stuff->nSyms>0)||(stuff->truncateSyms)) {
	register int i;
	XkbSymInterpretRec *wire = (XkbSymInterpretRec *)data;
	if (stuff->firstSym>compat->nSymInterpret) {
	    client->errorValue = XkbError2(0x02,compat->nSymInterpret);
	    return BadValue;
	}
	for (i=0;i<stuff->nSyms;wire++) {
	    /* 8/4/93 (ef) -- XXX! verify action and behavior here */
	}
	data = (char *)wire;
    }
    nMods=0;
    if (stuff->mods!=0) {
	register int i,bit;
	for (i=0,bit=1;i<8;i++,bit<<=1) {
	    if ( stuff->mods&bit )
		nMods++;
	}
	data+= (((nMods*sizeof(XkbModCompatRec))+3)/4)*4;
    }
    if ((data-((char *)stuff))!=stuff->length) {
	return BadLength;
    }
    data = (char *)&stuff[1];
    if (stuff->nSyms>0) {
	register int i;
	XkbSymInterpretRec *wire = (XkbSymInterpretRec *)data;
	XkbSymInterpretRec *sym;
	if ((stuff->firstSym+stuff->nSyms)>compat->nSymInterpret) {
	    compat->nSymInterpret= stuff->firstSym+stuff->nSyms;
	    compat->symInterpret= (XkbSymInterpretRec *)Xrealloc(
			compat->symInterpret,
			compat->nSymInterpret*sizeof(XkbSymInterpretRec));
	    if (!compat->symInterpret) {
		compat->nSymInterpret= 0;
		return BadAlloc;
	    }
	}
	else if (stuff->truncateSyms) {
	    compat->nSymInterpret = stuff->firstSym+stuff->nSyms;
	}
	sym = &compat->symInterpret[stuff->firstSym];
	/* 9/3/93 (ef) -- XXX! deal with swapping */
	for (i=0;i<stuff->nSyms;i++,wire++,sym++) {
	    *sym= *wire;
	}
	data = (char *)wire;
    }
    else if (stuff->truncateSyms) {
	compat->nSymInterpret = stuff->firstSym;
    }

    if (stuff->mods!=0) {
	register int i,bit=1;
	XkbModCompatRec *wire = (XkbModCompatRec *)data;
	for (i=0,bit=1;i<8;i++,bit<<=1) {
	    if (stuff->mods&bit) {
		*wire= compat->modCompat[i];
		wire++;
	    }
	}
    }
    if (xkb->interest) {
	xkbCompatMapNotify ev;
	ev.deviceID = dev->id;
	ev.changedMods = stuff->mods;
	ev.firstSym = stuff->firstSym;
	ev.nSyms = stuff->nSyms;
	ev.nTotalSyms = compat->nSymInterpret;
	XkbSendCompatMapNotify(xkb,&ev);
    }

    if (stuff->recomputeActions) {
	xkbMapNotify	mn;
	XkbUpdateActions(dev,xkb->desc.minKeyCode,xkb->desc.maxKeyCode,&mn);
	XkbUpdateCoreDescription(dev);
	if (mn.changed)
	    XkbSendMapNotify(dev,&mn);
    }
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbGetIndicatorState(client)
    ClientPtr client;
{
    REQUEST(xkbGetIndicatorStateReq);
    xkbGetIndicatorStateReply rep;
    DeviceIntPtr dev;
    CARD8 *map;
    register int i,bit;
    int nIndicators;

    REQUEST_SIZE_MATCH(xkbGetIndicatorStateReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if ( !dev ) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.deviceID = dev->id;
    rep.state = dev->key->xkbInfo->iState;
    WriteToClient(client, sizeof(xkbGetIndicatorStateReply), (char *)&rep);
    return client->noClientException;
}

static int
ProcXkbGetIndicatorMap(client)
    ClientPtr client;
{
    REQUEST(xkbGetIndicatorMapReq);
    xkbGetIndicatorMapReply rep;
    DeviceIntPtr dev;
    XkbDescRec *xkb;
    XkbIndicatorRec *leds;
    CARD8 *map;
    register int i,bit;
    int nIndicators;

    REQUEST_SIZE_MATCH(xkbGetIndicatorMapReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if ( !dev ) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb= &dev->key->xkbInfo->desc;
    leds= xkb->indicators;

    nIndicators= 0;
    for (i=0,bit=1;i<XKB_NUM_INDICATORS;i++,bit<<=1) {
	if (stuff->which&bit)
	    nIndicators++;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = nIndicators*sz_xkbIndicatorMapWireDesc;
    rep.deviceID = dev->id;
    rep.nRealIndicators = leds->nRealIndicators;
    rep.which = stuff->which;

    if (nIndicators>0) {
	CARD8 *to;
	to= map= (CARD8 *)ALLOCATE_LOCAL(rep.length);
	if (map) {
	    xkbIndicatorMapWireDesc  *wire = (xkbIndicatorMapWireDesc *)to;
	    for (i=0,bit=1;i<XKB_NUM_INDICATORS;i++,bit<<=1) {
		if (rep.which&bit) {
		    wire->whichMods= leds->maps[i].whichMods;
		    wire->mods= leds->maps[i].mods;
		    wire->whichGroups= leds->maps[i].whichGroups;
		    wire->groups= leds->maps[i].groups;
		    wire->controls= leds->maps[i].controls;
		    if (client->swapped) {
			register int n;
			swapl(&wire->controls,n);
		    }
		    wire++;
		}
	    }
	    to = (CARD8 *)wire;
	    if ((to-map)!=rep.length) {
		client->errorValue = XkbError2(0xff,rep.length);
		return BadLength;
	    }
	}
	else return BadAlloc;
    }
    else map = NULL;
    WriteToClient(client, sizeof(xkbGetIndicatorMapReply), (char *)&rep);
    if (map) {
	WriteToClient(client, rep.length, (char *)map);
	DEALLOCATE_LOCAL((char *)map);
    }
    return(client->noClientException);
}

static int
ProcXkbSetIndicatorMap(client)
    ClientPtr client;
{
    register int i,bit;
    int	nIndicators;
    DeviceIntPtr dev;
    XkbIndicatorRec	*leds;
    XkbSrvInfoRec 	*xkb;
    xkbIndicatorMapWireDesc *from;
    REQUEST(xkbSetIndicatorMapReq);

    REQUEST_AT_LEAST_SIZE(xkbSetIndicatorMapReq);
    dev = XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb= dev->key->xkbInfo;
    leds= xkb->desc.indicators;

    if (stuff->which==0) {
	client->errorValue= 0x01;
	return BadValue;
    }
    nIndicators= 0;
    for (i=0,bit=1;i<XKB_NUM_INDICATORS;i++,bit<<=1) {
	if (stuff->which&bit)
	    nIndicators++;
    }
    if (stuff->length!=
      ((sz_xkbSetIndicatorMapReq+(nIndicators*sz_xkbIndicatorMapWireDesc))/4)) {
	return BadLength;
    }

    from = (xkbIndicatorMapWireDesc *)&stuff[1];
    for (i=0,bit=1;i<XKB_NUM_INDICATORS;i++,bit<<=1) {
	if (stuff->which&bit) {
	    CARD8 which= (from->whichMods|from->whichGroups);

	    leds->maps[i].whichMods = from->whichMods;
	    leds->maps[i].mods = from->mods;
	    leds->maps[i].whichGroups = from->whichGroups;
	    leds->maps[i].groups = from->groups;
	    leds->maps[i].controls = from->controls;

	    if (which&XKB_IMUseBase)
		 xkb->iAccel.usesBase|= bit;
	    else xkb->iAccel.usesBase&= ~bit;
	    if (which&XKB_IMUseLatched)
		 xkb->iAccel.usesLatched|= bit;
	    else xkb->iAccel.usesLatched&= ~bit;
	    if (which&XKB_IMUseLocked)
		 xkb->iAccel.usesLocked|= bit;
	    else xkb->iAccel.usesLocked&= ~bit;
	    if (which&XKB_IMUseEffective)
		 xkb->iAccel.usesEffective|= bit;
	    else xkb->iAccel.usesEffective&= ~bit;
	    if (which&XKB_IMUseCompat)
		 xkb->iAccel.usesCompat|= bit;
	    else xkb->iAccel.usesCompat&= ~bit;
	    if (from->controls)
		 xkb->iAccel.usesControls|= bit;
	    else xkb->iAccel.usesControls&= ~bit;
	    from++;
	}
    }
    xkb->iAccel.usedComponents= 0;
    if (xkb->iAccel.usesBase)
	xkb->iAccel.usedComponents|= XkbModifierBaseMask|XkbGroupBaseMask;
    if (xkb->iAccel.usesLatched)
	xkb->iAccel.usedComponents|= XkbModifierLatchMask|XkbGroupLatchMask;
    if (xkb->iAccel.usesLocked)
	xkb->iAccel.usedComponents|= XkbModifierLockMask|XkbGroupLockMask;
    if (xkb->iAccel.usesEffective)
	xkb->iAccel.usedComponents|= XkbModifierStateMask|XkbGroupStateMask;
    if (xkb->iAccel.usesCompat)
	xkb->iAccel.usedComponents|= XkbCompatStateMask;
    if (stuff->which)
	XkbUpdateIndicators(dev,stuff->which,NULL);
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbGetNames(client)
    ClientPtr client;
{
    DeviceIntPtr dev;
    XkbDescRec *xkb;
    xkbGetNamesReply rep;
    register int i;
    char *start,*desc;
    REQUEST(xkbGetNamesReq);

    REQUEST_SIZE_MATCH(xkbGetNamesReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;
    rep.type= X_Reply;
    rep.sequenceNumber= client->sequence;
    rep.length = 0;
    rep.deviceID = dev->id;
    rep.which = stuff->which;
    if (stuff->which&XkbKeycodesNameMask)
	 rep.keycodes= xkb->names->keycodes;
    else rep.keycodes = None;
    if (stuff->which&XkbGeometryNameMask) 
	 rep.geometry = xkb->names->geometry;
    else rep.geometry = None;
    if (stuff->which&XkbSymbolsNameMask)
	 rep.symbols = xkb->names->symbols;
    else rep.symbols = None;
    rep.nKeyTypes = xkb->map->nKeyTypes;
    rep.nRadioGroups = xkb->names->nRadioGroups;
    rep.nCharSets =xkb->names->nCharSets;

    if (stuff->which&XkbKeyTypeNamesMask)
	 rep.length+= xkb->map->nKeyTypes;
    if (stuff->which&XkbKTLevelNamesMask) {
	XkbKeyTypeRec *pType = xkb->map->keyTypes;
	rep.length+= (xkb->map->nKeyTypes+3)/4;
	for (i=0;i<xkb->map->nKeyTypes;i++,pType++) {
	    rep.length+= pType->groupWidth;
	}
    }
    if (stuff->which&XkbRGNamesMask)
	 rep.length+= xkb->names->nRadioGroups;
    if (stuff->which&XkbIndicatorNamesMask)
	rep.length+= XKB_NUM_INDICATORS;
    if (stuff->which&XkbCharSetsMask)
	 rep.length+= xkb->names->nCharSets;
    if (stuff->which&XkbModifierNamesMask)
	rep.length+= (8*sizeof(Atom))/4;

    if (client->swapped) {
	register int n;
	swaps(&rep.sequenceNumber,n);
	swapl(&rep.length,n);
	swaps(&rep.which,n);
	swapl(&rep.keycodes,n);
	swapl(&rep.geometry,n);
	swapl(&rep.symbols,n);
    }

    start = desc = (char *)ALLOCATE_LOCAL(rep.length*4);
    if ( !start )
	return BadAlloc;
    if (stuff->which&XkbKeyTypeNamesMask) {
	memcpy(desc,xkb->names->keyTypes,rep.nKeyTypes*sizeof(Atom));
	if (client->swapped) {
	    register int n;
	    register Atom *atm = (Atom *)desc;
	    for (i=0;i<rep.nKeyTypes;i++,atm++) {
		swapl(atm,n);
	    }
	}
	desc+= rep.nKeyTypes*sizeof(Atom);
    }
    if (stuff->which&XkbKTLevelNamesMask) {
	XkbKeyTypeRec *pType = xkb->map->keyTypes;
	for (i=0;i<rep.nKeyTypes;i++,pType++) {
	    *desc++ = pType->groupWidth;
	}
	desc+= (((rep.nKeyTypes+3)/4)*4) - rep.nKeyTypes;

	pType = xkb->map->keyTypes;
	for (i=0;i<xkb->map->nKeyTypes;i++,pType++){
	    if (xkb->names->levels[i]) {
	      memcpy(desc,xkb->names->levels[i],pType->groupWidth*sizeof(Atom));
		if (client->swapped) {
		    register int n;
		    register Atom *atm = (Atom *)desc;
		    for (i=0;i<pType->groupWidth;i++,atm++) {
			swapl(atm,n);
		    }
		}
	    }
	    else bzero(desc,pType->groupWidth*sizeof(Atom));
	    desc+= pType->groupWidth*sizeof(Atom);
	}
    }
    if ((stuff->which&XkbRGNamesMask)&&(rep.nRadioGroups>0)) {
	memcpy(desc,xkb->names->radioGroups,rep.nRadioGroups*sizeof(Atom));
	if (client->swapped) {
	    register int n;
	    register Atom *atm = (Atom *)desc;
	    for (i=0;i<rep.nRadioGroups;i++,atm++) {
		swapl(atm,n);
	    }
	}
	desc+= rep.nRadioGroups*sizeof(Atom);
    }
    if (stuff->which&XkbIndicatorNamesMask) {
	memcpy(desc,xkb->names->indicators,XKB_NUM_INDICATORS*sizeof(Atom));
	if (client->swapped) {
	    register int n;
	    register Atom *atm = (Atom *)desc;
	    for (i=0;i<XKB_NUM_INDICATORS;i++,atm++) {
		swapl(atm,n);
	    }
	}
	desc+= XKB_NUM_INDICATORS*sizeof(Atom);
    }
    if (stuff->which&XkbModifierNamesMask) {
	memcpy(desc,(char *)xkb->names->modifiers,8*sizeof(Atom));
	if (client->swapped) {
	    register int n;
	    Atom *atm = (Atom *)desc;
	    for (i=0;i<8;i++,atm++) {
		swapl(atm,n);
	    }
	}
	desc+= 8*sizeof(Atom);
    }
    if ((stuff->which&XkbCharSetsMask)&&(rep.nCharSets>0)) {
	memcpy(desc,xkb->names->charSets,rep.nCharSets*sizeof(Atom));
	if (client->swapped) {
	    register int n;
	    register Atom *atm = (Atom *)desc;
	    for (i=0;i<rep.nCharSets;i++,atm++) {
		swapl(atm,n);
	    }
	}
	desc+= rep.nCharSets*sizeof(Atom);
    }
    if ((desc-start)!=(rep.length*4)) {
	ErrorF("BOGUS LENGTH in write names, expected %d, got %d\n",
					rep.length*4, desc-start);
    }
    WriteToClient(client, sizeof(rep), (char *)&rep);
    WriteToClient(client, rep.length*4, start);
    DEALLOCATE_LOCAL((char *)start);
    return client->noClientException;
}

static int
ProcXkbSetNames(client)
    ClientPtr client;
{
    DeviceIntPtr	 dev;
    XkbDescRec		*xkb;
    XkbNamesRec		*names;
    Atom		*tmp;
    REQUEST(xkbSetNamesReq);

    REQUEST_AT_LEAST_SIZE(xkbSetNamesReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if  (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;
    names = xkb->names;
    tmp = (Atom *)&stuff[1];

    if ((stuff->which&XkbKeycodesNameMask)&&(!ValidAtom(stuff->keycodes))) {
	client->errorValue = stuff->keycodes;
	return BadAtom;
    }
    if ((stuff->which&XkbGeometryNameMask)&&(!ValidAtom(stuff->geometry))) {
	client->errorValue = stuff->geometry;
	return BadAtom;
    }
    if ((stuff->which&XkbSymbolsNameMask)&&(!ValidAtom(stuff->symbols))) {
	client->errorValue = stuff->geometry;
	return BadAtom;
    }
    if (stuff->which&XkbKeyTypeNamesMask) {
	register int i;
	if ( stuff->nKeyTypes<1 ) {
	    client->errorValue = XkbError2(0x01,stuff->nKeyTypes);
	    return BadValue;
	}
	if ((stuff->firstKeyType+stuff->nKeyTypes-1)>=xkb->map->nKeyTypes) {
	    client->errorValue = XkbError4(0x02,stuff->firstKeyType,
				stuff->nKeyTypes,xkb->map->nKeyTypes);
	    return BadValue;
	}
	for (i=0;i<stuff->nKeyTypes;i++,tmp++) {
	    if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		client->errorValue = *tmp;
		return BadValue;
	    }
	}
    }
    if (stuff->which&XkbKTLevelNamesMask) {
	register int i,l;
	XkbKeyTypeRec *type;
	CARD8 *width;
	if ( stuff->nKTLevels<1 ) {
	    client->errorValue = XkbError2(0x03,stuff->nKTLevels);
	    return BadValue;
	}
	if ((stuff->firstKTLevel+stuff->nKTLevels-1)>=xkb->map->nKeyTypes) {
	    client->errorValue = XkbError4(0x04,stuff->firstKTLevel,
				stuff->nKTLevels,xkb->map->nKeyTypes);
	    return BadValue;
	}
	width = (CARD8 *)tmp;
	tmp+= ((stuff->nKTLevels+3)/4);
	type = &xkb->map->keyTypes[stuff->firstKeyType];
	for (i=0;i<stuff->nKeyTypes;i++,type++) {
	    if (width[i]!=type->groupWidth) {
		client->errorValue= XkbError4(0x05,i+stuff->firstKeyType,
						type->groupWidth,width[i]);
		return BadMatch;
	    }
	    for (l=0;l<width[i];l++,tmp++) {
		if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		    client->errorValue = *tmp;
		    return BadAtom;
		}
	    }
	}
    }
    if (stuff->which&XkbRGNamesMask) {
	register int i;
	if ( stuff->nRadioGroups<1 ) {
	    client->errorValue= XkbError2(0x06,stuff->nRadioGroups);
	    return BadValue;
	}
	for (i=0;i<stuff->nRadioGroups;i++,tmp++) {
	    if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		client->errorValue = *tmp;
		return BadAtom;
	    }
	}
    }
    if (stuff->which&XkbIndicatorNamesMask) {
	register int i,bit;
	if (stuff->indicators==0) {
	    client->errorValue= 0x08;
	    return BadMatch;
	}
	for (i=0,bit=1;i<32;i++,bit<<=1) {
	    if (stuff->indicators&bit) {
		if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		   client->errorValue = *tmp;
		   return BadAtom;
		}
		tmp++;
	    }
	}
    }
    if (stuff->which&XkbModifierNamesMask) {
	register int i,bit;
	if (stuff->modifiers==0) {
	    client->errorValue= 0x09;
	    return BadMatch;
	}
	for (i=0,bit=1;i<8;i++,bit<<=1) {
	    if (stuff->modifiers&bit) {
		if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		   client->errorValue = *tmp;
		   return BadAtom;
		}
		tmp++;
	    }
	}
    }
    if (stuff->which&XkbCharSetsMask) {
	register int i;
	for (i=0;i<stuff->nCharSets;i++,tmp++) {
	    if ((*tmp!=None)&&(!ValidAtom(*tmp))) {
		client->errorValue = *tmp;
		return BadAtom;
	    }
	}
    }
    if ((tmp-((ATOM *)stuff))!=stuff->length) {
	client->errorValue = stuff->length;
	return BadLength;
    }
    /* everything is okay -- update names */
    if (stuff->which&XkbKeycodesNameMask)
	names->keycodes= stuff->keycodes;
    if (stuff->which&XkbGeometryNameMask)
	names->geometry= stuff->geometry;
    if (stuff->which&XkbSymbolsNameMask)
	names->symbols= stuff->symbols;
    tmp = (Atom *)&stuff[1];
    if (stuff->which&XkbKeyTypeNamesMask) {
	register int i;
	for (i=0;i<stuff->nKeyTypes;i++,tmp++) {
	    xkb->names->keyTypes[i+stuff->firstKeyType] = *tmp;
	}
    }
    if (stuff->which&XkbKTLevelNamesMask) {
	register int i,l;
	CARD8 *width;
	width = (CARD8 *)tmp;
	tmp+= (stuff->nKTLevels+3)/4;
	for (i=0;i<stuff->nKeyTypes;i++) {
	    for (l=0;l<width[i];l++,tmp++) {
		xkb->names->levels[i+stuff->firstKTLevel][l]= *tmp;
	    }
	}
    }
    if (stuff->which&XkbRGNamesMask) {
	if (stuff->nRadioGroups>0) {
	    register int i;
	    if (xkb->names->radioGroups==NULL) {
		xkb->names->radioGroups= (Atom *)
				Xcalloc(sizeof(Atom)*stuff->nRadioGroups);
	    }
	    else if (stuff->nRadioGroups>=xkb->names->nRadioGroups) {
		xkb->names->radioGroups=(Atom*)Xrealloc(xkb->names->radioGroups,
					sizeof(Atom)*stuff->nRadioGroups);
	    }

	    if (xkb->names->radioGroups!=NULL) {
		xkb->names->nRadioGroups= stuff->nRadioGroups;
		for (i=0;i<stuff->nRadioGroups;i++,tmp++) {
		    xkb->names->radioGroups[i]= *tmp;
		}
	    }
	    else {
		xkb->names->nRadioGroups= 0;
		return BadAlloc;
	    }
	}
	else if (xkb->names->radioGroups) {
	    Xfree(xkb->names->radioGroups);
	    xkb->names->radioGroups= NULL;
	    xkb->names->nRadioGroups= 0;
	}
    }
    if (stuff->which&XkbIndicatorNamesMask) {
	register int i,bit;
	for (i=0,bit=1;i<32;i++,bit<<=1) {
	    if (stuff->indicators&bit) {
		xkb->names->indicators[i]= *tmp++;
	    }
	}
    }
    if (stuff->which&XkbModifierNamesMask) {
	register int i,bit;
	for (i=0,bit=1;i<8;i++,bit<<=1) {
	    if (stuff->modifiers&bit) {
		xkb->names->modifiers[i]= *tmp++;
	    }
	}
    }
    if (stuff->which&XkbCharSetsMask) {
	if (stuff->nCharSets==0) {
	    if (xkb->names->charSets) 
		Xfree(xkb->names->charSets);
	    xkb->names->charSets= NULL;
	    xkb->names->nCharSets= 0;
	}
	else {
	    if (stuff->nCharSets>xkb->names->nCharSets) {
		if (xkb->names->nCharSets==0)
		    xkb->names->charSets= (Atom *)Xcalloc(stuff->nCharSets*
								sizeof(Atom));
		else {
		    xkb->names->charSets=(Atom *)Xrealloc(xkb->names->charSets,
						stuff->nCharSets*sizeof(Atom));
		}
	    }
	    if (xkb->names->charSets!=NULL) {
		register int i;
		for (i=0;i<stuff->nCharSets;i++,tmp++) {
		    xkb->names->charSets[i]= *tmp;
		}
	    }
	}
    }
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbListAlternateSyms(client)
    ClientPtr client;
{
    DeviceIntPtr dev;
    XkbDescRec	*xkb;
    XkbAlternateSymsRec *syms;
    xkbListAlternateSymsReply rep;
    int		nMatch,szMatch;
    char	*data;
    REQUEST(xkbListAlternateSymsReq);

    REQUEST_SIZE_MATCH(xkbListAlternateSymsReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if  (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;
    if ((stuff->name!=None)&&(!ValidAtom(stuff->name))) {
	client->errorValue = stuff->name;
	return BadAtom;
    }
    if ((stuff->charset!=None)&&(!ValidAtom(stuff->charset))) {
	client->errorValue = stuff->charset;
	return BadAtom;
    }
    szMatch= nMatch = 0;
    syms = xkb->altSymSets;
    while (syms) {
	if ((stuff->name==None)||(stuff->name==syms->name)) {
	    if (stuff->charset!=None) {
		register int i;
		for (i=0;i<syms->nCharSets;i++) {
		   if (syms->charSets[i]==stuff->charset) {
			nMatch++;
			szMatch+= 4;	/* room for index and nCharSets */
			szMatch+= (1+syms->nCharSets)*sizeof(Atom);
			break;
		   }
		}
	    }
	    else {
		nMatch++;
		szMatch+= 4;	/* room for index and nCharSets */
		szMatch+= (1+syms->nCharSets)*sizeof(Atom);
	    }
	}
	syms= syms->next;
    }
    rep.type= X_Reply;
    rep.sequenceNumber= client->sequence;
    rep.length = szMatch/4;
    rep.deviceID = dev->id;
    rep.nAlternateSyms = nMatch;
    if (nMatch!=0) {
	Atom *out;
	data= (char *)ALLOCATE_LOCAL(szMatch);
	out = (Atom *)data;
	if (data) {
	    syms = xkb->altSymSets;
	    while (syms) {
		int	match;
		match= 0;
		if ((stuff->name==None)||(stuff->name==syms->name)) {
		    if (stuff->charset!=None) {
			register int i;
			for (i=0;(i<syms->nCharSets)&&(!match);i++) {
			   if (syms->charSets[i]==stuff->charset)
				match= 1;
			}
		    }
		}
		else match= 1;
		if (match) {
		    CARD8 *tmp= (CARD8 *)out;
		    register int i;
		    *out++= syms->name;
		    *tmp++= syms->index;
		    *tmp++= syms->nCharSets;
		    out++;
		    for (i=0;i<syms->nCharSets;i++) {
			*out++= syms->charSets[i];
		    }
		}
		syms= syms->next;
	    }
    	}
	else {
	    return BadAlloc;
	}
    }
    else data = NULL;
    WriteToClient(client, sz_xkbListAlternateSymsReply, (char *)&rep);
    if (data) {
	WriteToClient(client, rep.length*4, data);
	DEALLOCATE_LOCAL((char *)data);
    }
    return client->noClientException;
}

static int
ProcXkbGetAlternateSyms(client)
    ClientPtr client;
{
    DeviceIntPtr dev;
    XkbDescRec	*xkb;
    XkbAlternateSymsRec *syms;
    xkbGetAlternateSymsReply rep;
    int		  i,totalSyms;
    XkbSymMapRec *sMap;
    char	 *data,*to;
    REQUEST(xkbGetAlternateSymsReq);

    REQUEST_SIZE_MATCH(xkbGetAlternateSymsReq);
    dev = (DeviceIntPtr)XkbLookupDevice(stuff->deviceSpec);
    if  (!dev) {
	client->errorValue = XkbError2(0xff,stuff->deviceSpec);
	return XkbKeyboardErrorCode;
    }
    xkb = &dev->key->xkbInfo->desc;
    if (stuff->nKeys>0) {
	if (stuff->firstKey<xkb->minKeyCode) {
	    client->errorValue= XkbError4(0x01,stuff->firstKey,stuff->nKeys,
							xkb->minKeyCode);
	    return BadValue;
	}
	if ((stuff->firstKey+stuff->nKeys-1)>xkb->maxKeyCode) {
	    client->errorValue= XkbError4(0x02,stuff->firstKey,stuff->nKeys,
							xkb->maxKeyCode);
	    return BadValue;
	}
    }
    syms= xkb->altSymSets;
    while (syms) {
	if (syms->index==stuff->index)
	     break;
	else syms= syms->next;
    }
    if (!syms) {
	client->errorValue= stuff->index;
	return BadValue;
    }

    rep.type= X_Reply;
    rep.sequenceNumber= client->sequence;
    rep.length = 0;
    rep.deviceID = dev->id;
    rep.name= syms->name;
    rep.index= syms->index;
    rep.nCharSets= syms->nCharSets;
    if (stuff->nKeys>0) {
	int reqLast,setLast;
	if (stuff->firstKey<syms->firstKey)
	     rep.firstKey= syms->firstKey;
	else rep.firstKey= stuff->firstKey;
	setLast= syms->firstKey+syms->nKeys-1;
	reqLast= stuff->firstKey+stuff->nKeys-1;
	if (reqLast>setLast)
	     rep.nKeys= setLast-rep.firstKey+1;
	else rep.nKeys= reqLast-rep.firstKey+1;
    }
    else {
	rep.firstKey= rep.nKeys= 0;
    }
    sMap= &syms->maps[rep.firstKey-syms->firstKey];
    for (totalSyms=i=0;i<rep.nKeys;i++) {
	if (XkbNumGroups(sMap->groupInfo)>0) {
	    totalSyms+= XkbNumGroups(sMap->groupInfo)*
				xkb->map->keyTypes[sMap->ktIndex].groupWidth;
	}
    }
    rep.length= ((totalSyms*sizeof(Atom))+
				(rep.nKeys*sizeof(xkbSymMapWireDesc)))/4;
    if (rep.length) {
	xkbSymMapWireDesc *out;
	KeySym *symOut;
	data= (char *)ALLOCATE_LOCAL(rep.length*4);
	if (!data)
	    return BadAlloc;
	sMap= &syms->maps[rep.firstKey-syms->firstKey];
	out= (xkbSymMapWireDesc *)data;
	for (i=0;i<rep.nKeys;i++) {
	    register int n;
	    if (XkbNumGroups(sMap->groupInfo)==0) {
		out->ktIndex= 0;
		out->groupInfo= 0;
		out->nSyms= 0;
		out++;
		continue;
	    }
	    n= xkb->map->keyTypes[sMap->ktIndex].groupWidth;
	    n*= XkbNumGroups(sMap->groupInfo);
	    out->ktIndex= sMap->ktIndex;
	    out->groupInfo= sMap->groupInfo;
	    out->nSyms= n;
	    symOut= (KeySym *)&out[1];
	    for (n=0;n<out->nSyms;n++) {
		*symOut++= syms->syms[sMap->offset+n];
	    }
	    out= (xkbSymMapWireDesc *)symOut;
	}
    }
    else data= NULL;
    WriteToClient(client, sz_xkbGetAlternateSymsReply, (char *)&rep);
    if (data) {
	WriteToClient(client, rep.length*4, data);
	DEALLOCATE_LOCAL((char *)data);
    }
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbSetDebuggingFlags(client)
    ClientPtr client;
{
    int status;
    DeviceIntPtr dev;
    REQUEST(xkbSetDebuggingFlagsReq);

    REQUEST_SIZE_MATCH(xkbSetDebuggingFlagsReq);
    if ((xkbDebugFlags>0) || (stuff->flags>0))
	ErrorF("Setting debug flags to %d\n",stuff->flags);
    xkbDebugFlags = stuff->flags;
    return client->noClientException;
}

/***====================================================================***/

static int
ProcXkbDispatch (client)
    ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_kbUseExtension:
	return ProcXkbUseExtension(client);
    case X_kbSelectEvents:
	return ProcXkbSelectEvents(client);
    case X_kbSendEvent:
	return ProcXkbSendEvent(client);
    case X_kbBell:
	return ProcXkbBell(client);
    case X_kbGetState:
	return ProcXkbGetState(client);
    case X_kbLatchLockState:
	return ProcXkbLatchLockState(client);
    case X_kbGetControls:
	return ProcXkbGetControls(client);
    case X_kbSetControls:
	return ProcXkbSetControls(client);
    case X_kbGetMap:
	return ProcXkbGetMap(client);
    case X_kbSetMap:
	return ProcXkbSetMap(client);
    case X_kbGetCompatMap:
	return ProcXkbGetCompatMap(client);
    case X_kbSetCompatMap:
	return ProcXkbSetCompatMap(client);
    case X_kbGetIndicatorState:
	return ProcXkbGetIndicatorState(client);
    case X_kbGetIndicatorMap:
	return ProcXkbGetIndicatorMap(client);
    case X_kbSetIndicatorMap:
	return ProcXkbSetIndicatorMap(client);
    case X_kbGetNames:
	return ProcXkbGetNames(client);
    case X_kbSetNames:
	return ProcXkbSetNames(client);
    case X_kbListAlternateSyms:
	return ProcXkbListAlternateSyms(client);
    case X_kbGetAlternateSyms:
	return ProcXkbGetAlternateSyms(client);
#ifdef NOTYET
    case X_kbSetAlternateSyms:
	return ProcXkbSetAlternateSyms(client);
    case X_kbGetGeometry:
	return ProcXkbGetGeometry(client);
    case X_kbSetGeometry:
	return ProcXkbSetGeometry(client);
#endif
    case X_kbSetDebuggingFlags:
	return ProcXkbSetDebuggingFlags(client);
    default:
	return BadRequest;
    }
}

static int
SProcXkbDispatch (client)
    ClientPtr client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_kbUseExtension:
	return ProcXkbUseExtension(client);
    case X_kbSelectEvents:
	return ProcXkbSelectEvents(client);
    case X_kbSendEvent:
	return ProcXkbSendEvent(client);
    case X_kbBell:
	return ProcXkbBell(client);
    case X_kbGetState:
	return ProcXkbGetState(client);
    case X_kbLatchLockState:
	return ProcXkbLatchLockState(client);
    case X_kbGetControls:
	return ProcXkbGetControls(client);
    case X_kbSetControls:
	return ProcXkbSetControls(client);
    case X_kbGetMap:
	return ProcXkbGetMap(client);
    case X_kbSetMap:
	return ProcXkbSetMap(client);
    case X_kbGetCompatMap:
	return ProcXkbGetCompatMap(client);
    case X_kbSetCompatMap:
	return ProcXkbSetCompatMap(client);
    case X_kbGetIndicatorState:
	return ProcXkbGetIndicatorState(client);
    case X_kbGetIndicatorMap:
	return ProcXkbGetIndicatorMap(client);
    case X_kbSetIndicatorMap:
	return ProcXkbSetIndicatorMap(client);
    case X_kbGetNames:
	return ProcXkbGetNames(client);
    case X_kbSetNames:
	return ProcXkbSetNames(client);
    case X_kbListAlternateSyms:
	return ProcXkbListAlternateSyms(client);
    case X_kbGetAlternateSyms:
	return ProcXkbGetAlternateSyms(client);
#ifdef NOTYET
    case X_kbSetAlternateSyms:
	return ProcXkbSetAlternateSyms(client);
    case X_kbGetGeometry:
	return ProcXkbGetGeometry(client);
    case X_kbSetGeometry:
	return ProcXkbSetGeometry(client);
#endif
    case X_kbSetDebuggingFlags:
	return ProcXkbSetDebuggingFlags(client);
    default:
	return BadRequest;
    }
}

static int
XkbClientGone(data,id)
    pointer data;
    XID id;
{
    DevicePtr	pXDev = (DevicePtr)data;

    if (!XkbRemoveResourceClient(pXDev,id)) {
	ErrorF("Internal Error! bad RemoveResourceClient in XkbClientGone\n");
    }
    return 1;
}

static void
XkbResetProc(extEntry)
    ExtensionEntry *extEntry;
{
}

void
XkbExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();

    if (extEntry = AddExtension(XKBNAME, XkbNumberEvents, XkbNumberErrors,
				 ProcXkbDispatch, SProcXkbDispatch,
				 XkbResetProc, StandardMinorOpcode)) {
	XkbReqCode = (unsigned char)extEntry->base;
	XkbEventBase = (unsigned char)extEntry->eventBase;
	XkbErrorBase = (unsigned char)extEntry->errorBase;
	XkbKeyboardErrorCode = XkbErrorBase+XKB_Keyboard;
	RT_XKBCLIENT = CreateNewResourceType(XkbClientGone);
    }
    return;
}

