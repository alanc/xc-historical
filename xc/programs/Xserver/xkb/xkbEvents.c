/* $XConsortium$ */
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
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "XI.h"
#include "inputstr.h"
#include "XKBproto.h"
#include "XKBsrv.h"

extern int (*InitialVector[3])();

/***====================================================================***/

void
XKBSendStateNotify(kbd,pSN)
    DeviceIntPtr	 kbd;
    xKBStateNotify	*pSN;
{
    int i;
    xKBStateNotify	sn;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		time;
    register CARD16	changed;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;

    pSN->type = XKBEventCode + XKBEventBase;
    pSN->minor = XKBStateNotify;
    pSN->deviceID = kbd->id;
    pSN->time = time = GetTimeInMillis();
    pSN->mods = xkb->state.mods;
    pSN->baseMods = xkb->state.baseMods;
    pSN->latchedMods = xkb->state.latchedMods;
    pSN->lockedMods = xkb->state.lockedMods;
    pSN->group = xkb->state.group;
    pSN->baseGroup = xkb->state.baseGroup;
    pSN->latchedGroup = xkb->state.latchedGroup;
    pSN->lockedGroup = xkb->state.lockedGroup;
    pSN->compatState = xkb->state.compatState;
    pSN->unlockedMods = xkb->state.unlockedMods;
    pSN->groupsUnlocked = xkb->state.groupsUnlocked;
    changed = pSN->changed;

    interest = xkb->interest;

    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->stateNotifyMask&changed)) {
	    pSN->sequenceNumber = interest->client->sequence;
	    pSN->time = time;
	    pSN->changed = changed;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pSN->sequenceNumber,n);
		swapl(&pSN->time,n);
		swaps(&pSN->changed,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pSN);
	}
	interest= interest->next;
    }
    return;
}

/***====================================================================***/

void
XKBSendMapNotify(kbd,pMN)
    DeviceIntPtr kbd;
    xKBMapNotify *pMN;
{
    int i;
    XKBSrvInfoRec	*xkb;
    CARD16		 changed;

    xkb = kbd->key->xkbInfo;
    pMN->type = XKBEventCode + XKBEventBase;
    pMN->minor = XKBMapNotify;
    pMN->deviceID = kbd->id;

    changed = pMN->changed;

    for (i=1; i<currentMaxClients; i++) {
        if (clients[i] && ! clients[i]->clientGone &&
	    (clients[i]->requestVector != InitialVector) &&
	    (clients[i]->xkbClientFlags&XKB_INITIALIZED) &&
	    (clients[i]->mapNotifyMask&changed))
	{
	    pMN->sequenceNumber = clients[i]->sequence;
	    pMN->changed = changed;
	    if ( clients[i]->swapped ) {
		register int n;
		swaps(&pMN->sequenceNumber,n);
		swaps(&pMN->changed,n);
	    }
	    WriteToClient(clients[i],sizeof(xEvent),pMN);
	}
    }
    return;
}

int
XKBComputeControlsNotify(kbd,old,new,pCN)
    DeviceIntPtr	 kbd;
    XKBControlsRec	*old;
    XKBControlsRec	*new;
    xKBControlsNotify	*pCN;
{
    int			 i,initialized;
    CARD32 		 changedControls;
    CARD32		 enableChanges;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return 0;
 
    changedControls= 0;
    if (old->enabledControls!=new->enabledControls)
	changedControls|= XKBControlsEnabledMask;
    if ((old->repeatDelay!=new->repeatDelay)||
	(old->repeatInterval!=new->repeatInterval))
	changedControls|= XKBRepeatKeysMask;
    if (old->slowKeysDelay!=new->slowKeysDelay)
	changedControls|= XKBSlowKeysMask;
    if (old->debounceDelay!=new->debounceDelay)
	changedControls|= XKBBounceKeysMask;
    if ((old->mouseKeysDelay!=new->mouseKeysDelay)||
	(old->mouseKeysInterval!=new->mouseKeysInterval)||
	(old->mouseKeysDfltBtn!=new->mouseKeysDfltBtn))
	changedControls|= XKBMouseKeysMask;
    if ((old->mouseKeysTimeToMax!=new->mouseKeysTimeToMax)||
	(old->mouseKeysCurve!=new->mouseKeysCurve)||
	(old->mouseKeysMaxSpeed!=new->mouseKeysMaxSpeed))
	changedControls|= XKBMouseKeysAccelMask;
    if ((old->accessXTimeout!=new->accessXTimeout)||
	(old->accessXTimeoutMask!=new->accessXTimeoutMask))
	changedControls|= XKBAccessXKeysMask;
    if (old->numGroups!=new->numGroups)
	changedControls|= XKBKeyboardGroupsMask;
    if (old->internalMods!=new->internalMods)
	changedControls|= XKBInternalModsMask;
    if (old->ignoreLockMods!=new->ignoreLockMods)
	changedControls|= XKBIgnoreLockModsMask;
    if (!changedControls)
	return 0;
    pCN->changedControls = changedControls;
    pCN->enabledControls = new->enabledControls;
    pCN->enabledControlChanges = (new->enabledControls^old->enabledControls);
    return 1;
}

void
XKBSendControlsNotify(kbd,pCN)
    DeviceIntPtr	 kbd;
    xKBControlsNotify	*pCN;
{
    int			 i,initialized;
    CARD32 		 changedControls,enabledControls,enabledChanges;
    xKBControlsNotify	 cn;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;
 
    initialized = 0;
    interest = xkb->interest;
    changedControls = pCN->changedControls;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->controlsNotifyMask&changedControls)) {
	    if (!initialized) {
		pCN->type = XKBEventCode + XKBEventBase;
		pCN->minor = XKBControlsNotify;
		pCN->deviceID = kbd->id;
		pCN->time = time = GetTimeInMillis();
		enabledControls = pCN->enabledControls;
		enabledChanges = pCN->enabledControlChanges;
		initialized= 1;
	    }
	    pCN->changedControls = changedControls;
	    pCN->enabledControls = enabledControls;
	    pCN->enabledControlChanges = enabledChanges;
	    pCN->sequenceNumber = interest->client->sequence;
	    pCN->time = time;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pCN->sequenceNumber,n);
		swapl(&pCN->changedControls,n);
		swapl(&pCN->enabledControls,n);
		swapl(&pCN->enabledControlChanges,n);
		swapl(&pCN->time,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pCN);
	}
	interest= interest->next;
    }
    return;
}

void
XKBSendIndicatorNotify(kbd,pEv)
    DeviceIntPtr	 kbd;
    xKBIndicatorNotify	*pEv;
{
    int			 i,initialized;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;
    CARD32		 state,stateChanged,mapChanged;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;
 
    initialized = 0;
    interest = xkb->interest;
    state = pEv->state;
    stateChanged = pEv->stateChanged;
    mapChanged = pEv->mapChanged;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    ((interest->iStateNotifyMask&stateChanged)||
	     (interest->iMapNotifyMask&mapChanged))) {
	    if (!initialized) {
		pEv->type = XKBEventCode + XKBEventBase;
		pEv->minor = XKBIndicatorNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->stateChanged = stateChanged;
	    pEv->state = state;
	    pEv->mapChanged = mapChanged;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swapl(&pEv->stateChanged,n);
		swapl(&pEv->state,n);
		swapl(&pEv->mapChanged,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}


#if NeedFunctionPrototypes
void
XKBHandleBell(	DeviceIntPtr	 kbd,
		CARD8		 percent,
		pointer		 pCtrl,
		CARD8		 class,
		Atom		 name)
#else
void
XKBHandleBell(kbd,percent,pCtrl,class,name)
    DeviceIntPtr	 kbd;
    CARD8		 percent;
    pointer		*pCtrl;
    CARD8		 class;
    Atom		 name;
#endif
{
    xKBBellNotify	 bn;
    int			 i,initialized;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    CARD8		 id;
    CARD16		 pitch,duration;
    Time 		 time;

    xkb = kbd->key->xkbInfo;

    if (xkb->desc.controls->enabledControls&XKBAudibleBellMask)
	(*kbd->kbdfeed->BellProc)(percent,kbd,(pointer)pCtrl,class);
    if (!xkb->interest)
	return;

    if ((class==0)||(class==KbdFeedbackClass)) {
	KeybdCtrl *pKeyCtrl= (KeybdCtrl *)pCtrl;
	id= pKeyCtrl->id;
	pitch= pKeyCtrl->bell_pitch;
	duration= pKeyCtrl->bell_duration;
    }
    else if (class==BellFeedbackClass) {
	BellCtrl *pBellCtrl= (BellCtrl *)pCtrl;
	id= pBellCtrl->id;
	pitch= pBellCtrl->pitch;
	duration= pBellCtrl->duration;
    }
    else return;
 
    initialized = 0;
    interest = xkb->interest;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->bellNotifyWanted)) {
	    if (!initialized) {
		time = GetTimeInMillis();
		bn.type = XKBEventCode + XKBEventBase;
		bn.minor = XKBBellNotify;
		bn.deviceID = kbd->id;
		bn.bellClass = class;
		bn.bellID = id;
		bn.percent= percent;
		initialized= 1;
	    }
	    bn.sequenceNumber = interest->client->sequence;
	    bn.time = time;
	    bn.pitch = pitch;
	    bn.duration = duration;
	    bn.name = name;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&bn.sequenceNumber,n);
		swapl(&bn.time,n);
		swaps(&bn.pitch,n);
		swaps(&bn.duration,n);
		swapl(&bn.name,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)&bn);
	}
	interest= interest->next;
    }
    return;
}

void
XKBSendSlowKeyNotify(kbd,pEv)
    DeviceIntPtr	 kbd;
    xKBSlowKeyNotify	*pEv;
{
    int			 i,initialized;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;
    CARD16		 delay;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;
 
    initialized = 0;
    interest = xkb->interest;
    delay= pEv->delay;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->slowKeyNotifyMask&pEv->slowKeyType)) {
	    if (!initialized) {
		pEv->type = XKBEventCode + XKBEventBase;
		pEv->minor = XKBSlowKeyNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->delay = delay;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->delay,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XKBSendNamesNotify(kbd,pEv)
    DeviceIntPtr	 kbd;
    xKBNamesNotify	*pEv;
{
    int			 i,initialized;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;
    CARD16		 changed;
    CARD32		 changedIndicators;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;
 
    initialized = 0;
    interest = xkb->interest;
    changed= pEv->changed;
    changedIndicators= pEv->changedIndicators;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->namesNotifyMask&pEv->changed)) {
	    if (!initialized) {
		pEv->type = XKBEventCode + XKBEventBase;
		pEv->minor = XKBNamesNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->changed = changed;
	    pEv->changedIndicators = changedIndicators;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->changed,n);
		swapl(&pEv->changedIndicators,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XKBSendCompatMapNotify(kbd,pEv)
    DeviceIntPtr	 kbd;
    xKBCompatMapNotify	*pEv;
{
    int			 i,initialized;
    XKBSrvInfoRec	*xkb;
    XKBInterestRec	*interest;
    Time 		 time;
    CARD16		 firstSym,nSyms,nTotalSyms;

    xkb = kbd->key->xkbInfo;
    if (!xkb->interest)
	return;
 
    initialized = 0;
    interest = xkb->interest;
    while (interest) {
	if ((!interest->client->clientGone) &&
	    (interest->client->requestVector != InitialVector) &&
	    (interest->client->xkbClientFlags&XKB_INITIALIZED) &&
	    (interest->compatNotifyMask)) {
	    if (!initialized) {
		pEv->type = XKBEventCode + XKBEventBase;
		pEv->minor = XKBCompatMapNotify;
		pEv->deviceID = kbd->id;
		pEv->time = time = GetTimeInMillis();
		firstSym= pEv->firstSym;
		nSyms= pEv->nSyms;
		nTotalSyms= pEv->nTotalSyms;
		initialized= 1;
	    }
	    pEv->sequenceNumber = interest->client->sequence;
	    pEv->time = time;
	    pEv->firstSym = firstSym;
	    pEv->nSyms = nSyms;
	    pEv->nTotalSyms = nTotalSyms;
	    if ( interest->client->swapped ) {
		register int n;
		swaps(&pEv->sequenceNumber,n);
		swapl(&pEv->time,n);
		swaps(&pEv->firstSym,n);
		swaps(&pEv->nSyms,n);
		swaps(&pEv->nTotalSyms,n);
	    }
	    WriteToClient(interest->client, sizeof(xEvent), (char *)pEv);
	}
	interest= interest->next;
    }
    return;
}

void
XKBSendNotification(kbd,pChanges)
    DeviceIntPtr	 kbd;
    XKBChangesRec	*pChanges;
{
    if (pChanges->stateChanges) {
	xKBStateNotify sn;
	sn.changed= pChanges->stateChanges;
	XKBSendStateNotify(kbd,&sn);
    }
    if (pChanges->map.changed) {
	xKBMapNotify mn;
	mn.changed= pChanges->map.changed;
	mn.firstKeyType= pChanges->map.firstKeyType;
	mn.nKeyTypes= pChanges->map.nKeyTypes;
	mn.firstKeyAction= pChanges->map.firstKeyAction;
	mn.nKeyActions= pChanges->map.nKeyActions;
	mn.firstKeyBehavior= pChanges->map.firstKeyBehavior;
	mn.nKeyBehaviors= pChanges->map.nKeyBehaviors;
	XKBSendMapNotify(kbd,&mn);
    }
    if ((pChanges->controls.changedControls)||
	(pChanges->controls.enabledControlsChanges)) {
#ifdef NOTYET
	XKBSendControlsNotify(kbd,&cn);
#endif
    }
    if ((pChanges->indicators.stateChanges)||(pChanges->indicators.mapChanges)){
	xKBIndicatorNotify in;
	in.stateChanged= pChanges->indicators.stateChanges;
	in.state= kbd->key->xkbInfo->iState;
	in.mapChanged= pChanges->indicators.mapChanges;
	XKBSendIndicatorNotify(kbd,&in);
    }
    if (pChanges->names.changed) {
	xKBNamesNotify nn;
	nn.changed= pChanges->names.changed;
	nn.firstKeyType= pChanges->names.firstKeyType;
	nn.nKeyTypes= pChanges->names.nKeyTypes;
	nn.firstLevelName= pChanges->names.firstLevel;
	nn.nLevelNames= pChanges->names.nLevels;
	nn.nRadioGroups= pChanges->names.nRadioGroups;
	nn.nCharSets= kbd->key->xkbInfo->desc.names->nCharSets;
	nn.changedMods= pChanges->names.changedMods;
	nn.changedIndicators= pChanges->names.changedIndicators;
	XKBSendNamesNotify(kbd,&nn);
    }
    if ((pChanges->compat.changedMods)||(pChanges->compat.nSyms>0)) {
	xKBCompatMapNotify cmn;
	cmn.changedMods= pChanges->compat.changedMods;
	cmn.firstSym= pChanges->compat.firstSym;
	cmn.nSyms= pChanges->compat.nSyms;
	cmn.nTotalSyms= kbd->key->xkbInfo->desc.compat->nSymInterpret;
	XKBSendCompatMapNotify(kbd,&cmn);
    }
    return;
}

/***====================================================================***/

void
XKBFilterWriteEvents(pClient,nEvents,xE)
    ClientPtr	 pClient;
    int		 nEvents;
    xEvent	*xE;
{
int	i;
DeviceIntPtr pXDev = (DeviceIntPtr)LookupKeyboardDevice();

    if ( pClient->xkbClientFlags & XKB_INITIALIZED ) {
	(void)WriteToClient(pClient,nEvents*sizeof(xEvent),(char *)xE);
    }
    else {
	XKBSrvInfoRec *xkb= pXDev->key->xkbInfo;
	register CARD8 type;
	for (i=0;i<nEvents;i++) {
	    type= xE[i].u.u.type;
	    if ( (type>=KeyPress)&&(type<=MotionNotify) ) {
		unsigned old = xE[i].u.keyButtonPointer.state;
		xE[i].u.keyButtonPointer.state&= 0x1F00;
		if ((old&0xE0FF)==(xkb->lookupState&0xE0FF))
		     xE[i].u.keyButtonPointer.state|= 
						(xkb->compatLookupState&0xff);
		else xE[i].u.keyButtonPointer.state|= 
						(xkb->compatGrabState&0xFF);
		(void)WriteToClient(pClient,sizeof(xEvent),(char *)&xE[i]);
		xE[i].u.keyButtonPointer.state= old;
	    }
	    else if ((type==EnterNotify)||(type==LeaveNotify)) {
		unsigned old = xE->u.enterLeave.state;
		xE->u.enterLeave.state&= 0x1F00;
		xE->u.enterLeave.state|= (xkb->compatGrabState&0xFF);
		(void)WriteToClient(pClient,sizeof(xEvent),(char *)&xE[i]);
		xE->u.enterLeave.state= old;
	    }
	    else WriteToClient(pClient,sizeof(xEvent),(char *)&xE[i]);
	}
    }
    return;
}

/***====================================================================***/

XKBInterestRec *
XKBFindClientResource(inDev,client)
    DevicePtr	 inDev;
    ClientRec	*client;
{
DeviceIntPtr	 dev = (DeviceIntPtr)inDev;
XKBInterestRec	*interest;

    if ( dev->key && dev->key->xkbInfo ) {
	interest = dev->key->xkbInfo->interest;
	while (interest){
	    if (interest->client==client) {
		return interest;
	    }
	    interest = interest->next;
	}
    }
    return NULL;
}

XKBInterestRec *
XKBAddClientResource(inDev,client,id)
    DevicePtr	 inDev;
    ClientRec	*client;
    XID		 id;
{
DeviceIntPtr	 dev = (DeviceIntPtr)inDev;
XKBInterestRec	*interest;

    if ( dev->key && dev->key->xkbInfo ) {
	interest = dev->key->xkbInfo->interest;
	while (interest) {
	    if (interest->client==client)
		return ((interest->resource==id)?interest:NULL);
	    interest = interest->next;
	}
	interest = (XKBInterestRec *)xalloc(sizeof(XKBInterestRec));
	if (interest) {
	    interest->kbd = dev->key->xkbInfo;
	    interest->client = client;
	    interest->resource = id;
	    interest->stateNotifyMask= 0;
	    interest->controlsNotifyMask= 0;
	    interest->namesNotifyMask= 0;
	    interest->compatNotifyMask= 0;
	    interest->bellNotifyWanted= FALSE;
	    interest->slowKeyNotifyMask= 0;
	    interest->iStateNotifyMask= 0;
	    interest->iMapNotifyMask= 0;
	    interest->altSymsNotifyMask= 0;
	    interest->next = dev->key->xkbInfo->interest;
	    dev->key->xkbInfo->interest= interest;
	    return interest;
	}
    }
    return NULL;
}

int
XKBRemoveClient(inDev,client)
DevicePtr	 inDev;
ClientRec	*client;
{
DeviceIntPtr	 dev = (DeviceIntPtr)inDev;
XKBInterestRec	*interest;

    if ( dev->key && dev->key->xkbInfo ) {
	interest = dev->key->xkbInfo->interest;
	if (interest && (interest->client==client)){
	    dev->key->xkbInfo->interest = interest->next;
	    xfree(interest);
	    return 1;
	}
	while (interest->next) {
	    if (interest->next->client==client) {
		XKBInterestRec *victim = interest->next;
		interest->next = victim->next;
		xfree(victim);
		return 1;
	    }
	    interest = interest->next;
	}
    }
    return 0;
}

int
XKBRemoveResourceClient(inDev,id) 
    DevicePtr	 inDev;
    XID		 id;
{
DeviceIntPtr	 dev = (DeviceIntPtr)inDev;
XKBInterestRec	*interest;

    if ( dev->key && dev->key->xkbInfo ) {
	interest = dev->key->xkbInfo->interest;
	if (interest && (interest->resource==id)){
	    dev->key->xkbInfo->interest = interest->next;
	    xfree(interest);
	    return 1;
	}
	while (interest->next) {
	    if (interest->next->resource==id) {
		XKBInterestRec *victim = interest->next;
		interest->next = victim->next;
		xfree(victim);
		return 1;
	    }
	    interest = interest->next;
	}
    }
    return 0;
}

