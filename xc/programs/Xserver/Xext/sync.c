/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * 64 bit changes
 * $Log: sync.c,v $
 * Revision 1.1.1.2  1993/05/05  18:03:32  Pete_Snider
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

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#include "misc.h"
#include "os.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#define _SYNC_SERVER
#include "sync.h"
#include "syncstr.h"

/*
 * External routines that should be defined in header files
 */

extern ExtensionEntry *AddExtension(/* LOTSOFARGS */);

/*
 * Local types
 */

 
/*
 * Local Global Variables
 */
static int SyncReqCode;
static int SyncEventBase;
static int SyncErrorBase;
static RESTYPE RTCounter = 0;
static RESTYPE RTAwait;
static RESTYPE RTAlarm;
static RESTYPE RTAlarmClient;

void TriggerAwait();

SysCounter *SysCounterList = NULL;
int NumSystemCounters = 0;

static void ShowAlarm(pAlarm)
SyncAlarm *pAlarm;
{
    fprintf(stderr,"client: 0x%lx id: 0x%lx counter: 0x%lx\n",
	    pAlarm->client, pAlarm->id, pAlarm->counter);
    fprintf(stderr,"value: 0x%lx value_type: 0x%x test_type: 0x%x\n",
	    pAlarm->value, pAlarm->value_type, pAlarm->test_type);
    fprintf(stderr,"delta: 0x%lx events: 0x%x state: 0x%x\n",
	    pAlarm->delta, pAlarm->events, pAlarm->state);
}

static void ShowCounter(pCounter)
SyncCounter *pCounter;
{
    WaitList *pWait = pCounter->waitlist;

    fprintf(stderr, "Counter: id %X owner %X\n");
    while (pWait != NULL) {
	ShowAlarm(pWait->pAlarm);
	pWait = pWait->next;
    }
    fprintf(stderr, "\n");
}

static int CheckCounter(pCounter, where)
SyncCounter *pCounter;
char *where;
{
    WaitList *pWait = pCounter->waitlist;
    WaitList *pNext;

    while ((pNext = pWait->next) != 0) {
#if 0
/* REMOVED PDS  --- add back if ordering of alarms are required
	if (pWait->counter != pCounter) {
	    fprintf(stderr, "%s: Wrong counter value in WaitItem\n", where);
	    ShowCounter(pCounter);
	    return 0;
	}
	if (C64LessThan(pNext->value, pWait->value) && 
	    pWait != &pCounter->wait) {
	    fprintf(stderr, "%s: Badly ordered counter:\n", where);
	    ShowCounter(pCounter);
	    return 0;
	}
	if (pNext->prev != pWait) {
	    fprintf(stderr, "%s: Badly linked counter:\n", where);
	    ShowCounter(pCounter);
	    return 0;
	}
*/
#endif /* PDS 0 */
	pWait = pNext;
    }
    return Success;
}

/*
*** AddWaitItem
*** Add a new waitlist struct. to a counter waitlist
 */
static int AddWaitItem(pitem)
SyncAwait *pitem; /* this can either be a SyncAlarm or SyncAwait */
{
    WaitList *pTmp;

    if (pitem->counter == NULL)
	    return;

    if(!(pTmp = (WaitList *)xalloc(sizeof(WaitList)))){
fprintf(stderr,"DEBUG: AddWaitItem alloc failed for waitlist\n");
    	return BadAlloc;
    }
    if(pitem->counter->waitlist)
	pTmp->next = pitem->counter->waitlist->next;
    else
	pTmp->next = NULL;

    pTmp->pAlarm = (SyncAlarm *)pitem;
    pitem->counter->waitlist = pTmp;


#ifdef CHECK
    if (!CheckCounter(pitem->counter,"AddWaitItem")) {
	ShowWaitItem(pitem);
	fprintf(stderr, "... has just been added\n");
    }
#endif
}

static int InitTrigger(pItem)
SyncAwait *pItem;
{
    if(pItem->value_type == SyncAbsolute)
	pItem->event_value = pItem->value;
    if(pItem->value_type == SyncRelative){
	if(pItem->counter == None)
	    return BadMatch;
	C64Add(pItem->event_value, pItem->counter->value,  pItem->value);
    }
}

#if 0

/*
*** RemoveWaitItem
*** Remove a wait item from a counter
 */
static void RemoveWaitItem(pCounter, pItem)
SyncCounter *pCounter;
WaitItem *pItem;
{
/*
 * If this item is not on a list, there is nothing to do
 */
    if (pCounter == 0 || pItem->prev == 0) {
	return;
    }
/*
 * If this item is first on the list, tell the counter that it has gone
 */
    if (pItem == pCounter->wait.next) {
	if (pItem->next) {
	    (*pCounter->NewWait)((pointer)pCounter, pItem->next->value,
				 SyncAbsolute, &pCounter->value);
	} else {
	    (*pCounter->NoWait)((pointer)pCounter);
	}
    }
/*
 * Remove this from the list
 */
    pItem->prev->next = pItem->next;
    if (pItem->next) {
	pItem->next->prev = pItem->prev;
    }
    pItem->next = pItem->prev = NULL;

#ifdef CHECK
#undef CHECK
    if (!CheckCounter(pCounter, "RemoveWaitItem")) {
	ShowWaitItem(pItem);
	fprintf(stderr, "... has just been removed\n");
    }
#endif
}

Is this functionality needed ?
/*
*** ReorderWaitItem
*** Move a wait item around in the queue, if necessary
 */
static void ReorderWaitItem(pCounter, plist)
SyncCounter *pCounter;
WaitList *plist;
{
    int was_first = pCounter->waitlist == plist;

/*
 * Move item down the queue
 */
    while (plist->next != NULL &&
	   C64GreaterThan(plist->pAlarm->value, plist->next->pAlarm->value)) {
	WaitItem *pNext = plist->next;

	plist->next = pNext->next;
	pNext->next = plist;
    }

/*
 * If we were head or are still head, tell the counter about the change
 * in wait time.
 */
    if ((was_first || pCounter->waitlist == plist)) {
	(*pCounter->NewWait)((pointer)pCounter, 
			     pCounter->waitlist->pAlarm->value,
			     SyncAbsolute, &pCounter->waitlist->pAlarm->value);
    }

#ifdef CHECK
    if (!CheckCounter(pCounter,"ReorderWaitItem")) {
/*	ShowWaitItem(pItem); */
	fprintf(stderr, "... has just been re-ordered\n");
    }
#endif
}
#endif /* 0 PDS */


/*
*** TriggerAlarm
 */
void TriggerAlarm(pAlarm)
SyncAlarm *pAlarm;
{
    AlarmClient *pClients;
    xAlarmNotifyEvent e;
    int gone = pAlarm->client->clientGone;
    SyncCounter *pCounter = pAlarm->counter;
    CARD64 a_value;

    /* no need to check alarm if it's Inactive */
    if(pAlarm->state == AlarmInactive)
	return;

    /* Check is trigger TRUE */
    if (!gone && ! (((pAlarm->test_type == PositiveTransition 
	  || pAlarm->test_type == PositiveComparison)
	    &&  C64GreaterOrEqual(pAlarm->counter->value, pAlarm->event_value))
	|| ((pAlarm->test_type == NegativeTransition
	    || pAlarm->test_type == NegativeComparison)
	    && C64LessOrEqual(pAlarm->counter->value, pAlarm->event_value))))
	    	return;

    if( pAlarm->counter == None 
       || (pAlarm->delta == 0 
	   && (pAlarm->test_type == PositiveComparison 
	    || pAlarm->test_type == NegativeComparison)))
	pAlarm->state = AlarmInactive;
    else{
    /* add delta to value until trigger is FALSE */
	while(((pAlarm->test_type == PositiveTransition 
	    || pAlarm->test_type == PositiveComparison)
	    &&  C64GreaterOrEqual(pAlarm->counter->value, pAlarm->event_value))
	    || ((pAlarm->test_type == NegativeTransition 
		 || pAlarm->test_type == NegativeComparison)
		&& C64LessOrEqual(pAlarm->counter->value,pAlarm->event_value))){
	    C64Add( pAlarm->event_value, pAlarm->event_value, pAlarm->delta);
fprintf(stderr,"DEBUG: updating event_value to 0x%lx\n",pAlarm->event_value );
	}

    }
    UpdateCurrentTimeIf();
/*
 * Send an event to all the interested clients
 */
    if(pAlarm->events){
	e.type = SyncEventBase + AlarmNotify;
	e.sequenceNumber = pAlarm->client->sequence;
	e.alarm = pAlarm->id;
	e.counter_value_lo = C64Low32(pAlarm->counter->value);
	e.counter_value_hi = C64High32(pAlarm->counter->value);
	e.alarm_value_lo = C64Low32(pAlarm->event_value);
	e.alarm_value_hi = C64High32(pAlarm->event_value);
	e.time = currentTime.milliseconds;
	e.state = pAlarm->state;
	if(!pAlarm->client->clientGone)
	    WriteEventsToClient(pAlarm->client, 1, (xEvent *)&e);
    }
    if( (pClients = pAlarm->clients) != NULL ){
	while(pClients){
	    if(pClients->events && !pClients->client->clientGone){
/* PDS do the event values need reinitializing??? */
		WriteEventsToClient(pClients->client, 1, (xEvent *)&e);
	    }
	    pClients = pClients->next;
	}
    }
}

/*
*** Wake up all the clients who's value exceeds the counter value
 */
static void SyncReleaseClients(pCounter)
SyncCounter *pCounter;
{
    WaitList *pWait;

    if(!pCounter)
	return;

    /* Loop through the chain and call appropriate trigger check */
    for( pWait = pCounter->waitlist; pWait; pWait = pWait->next){
	if(pWait->is_alarm)
	    TriggerAlarm(pWait->pAlarm);
	else
	    TriggerAwait((SyncAwait *)pWait->pAlarm);
    }
}


/*
*** Check Await for a trigger
 */
/* PDS - is it valid to keep the Await structure after the trigger???? */
void TriggerAwait(pAwait)
SyncAwait *pAwait;
{
    INT64 result;
    int n;
    xCounterNotifyEvent e;
    SyncAwait *pSAwait = pAwait; /* to keep track of the head, not used */
    ClientPtr client = pAwait->client;
    int gone = client->clientGone;

    while( pAwait ){
	if (gone || ((pAwait->test_type == PositiveTransition 
	    || pAwait->test_type == PositiveComparison)
	    &&  C64GreaterOrEqual(pAwait->counter->value, pAwait->event_value))
	|| ((pAwait->test_type == NegativeTransition
	    || pAwait->test_type == NegativeComparison)
	    && C64LessOrEqual(pAwait->counter->value, pAwait->event_value))){

		AttendClient(client);

		e.type = SyncEventBase + CounterNotify;
		e.sequenceNumber = client->sequence;
		e.counter = pAwait->counter->id;
		e.wait_value_lo = C64Low32(pAwait->event_value);
		e.wait_value_hi = C64High32(pAwait->event_value);
		e.counter_value_lo = C64Low32(pAwait->counter->value);
		e.counter_value_hi = C64High32(pAwait->counter->value);
		e.time = currentTime.milliseconds;
		if(gone)
		    e.destroyed = 1;
		else 
		    e.destroyed = 0;

		WriteEventsToClient(client, 1, (xEvent *)&e);
        }
	pAwait = pAwait->next;
    }
}


/* ARGSUSED */
static void FreeAlarm(addr, id)
pointer addr;
CARD32 id;
{    
    
    xAlarmNotifyEvent e;
    WaitList *pwl, *pwaitlist;
    AlarmClient *pClients, *pnClients;
    SyncAlarm *pAlarm = (SyncAlarm *) addr;
    SyncCounter *pCounter = pAlarm->counter;

    if( !pAlarm->client->clientGone){
	UpdateCurrentTimeIf();

	e.type = SyncEventBase + AlarmNotify;
	e.sequenceNumber = pAlarm->client->sequence;
	
	e.alarm = pAlarm->id;
	if (pCounter) {
	    e.counter_value_lo = C64Low32(pCounter->value);
	    e.counter_value_hi = C64High32(pCounter->value);
	} else {
	    e.counter_value_lo = 0;
	    e.counter_value_hi = 0;
	}
	e.alarm_value_lo = C64Low32(pAlarm->event_value);
	e.alarm_value_hi = C64High32(pAlarm->event_value);
	e.state = AlarmDestroyed;
	e.time = currentTime.milliseconds;

fprintf(stderr,"DEBUG: DestroyAlarm: type: 0x%lx sizeof: 0x%lx value: 0x%lx \n",
e.type, sizeof(e), pAlarm->value);
	WriteEventsToClient(pAlarm->client, 1, (xEvent *)&e);
    }
    if( (pClients = pAlarm->clients) != NULL ){
	while(pClients){
	    if( !pAlarm->client->clientGone )
		WriteEventsToClient(pClients->client, 1, (xEvent *)&e);
	
		pnClients = pClients->next;
	    /* if client is not terminated, could be from DestoryAlarm */
	    if((pAlarm->client->clientGone 
		&& pAlarm->client->closeDownMode == DestroyAll)
	        || !pAlarm->client->clientGone)
		xfree(pClients); 
	    pClients = pnClients;
	}
    }
    if(pAlarm->counter && ((pAlarm->client->clientGone 
	&& pAlarm->client->closeDownMode == DestroyAll)
       || !pAlarm->client->clientGone)){
	pwaitlist = pAlarm->counter->waitlist;
	pwl = pAlarm->counter->waitlist;
	while( pAlarm != pwaitlist->pAlarm){
	    if(pwaitlist->next){
		pwl = pwaitlist;
		pwaitlist = pwaitlist->next;
	    }
	    else
		break;
	}
	if( pAlarm == pwaitlist->pAlarm){
	    pwl->next = pwaitlist->next;
	    pwaitlist->next = NULL;
	    pwaitlist->pAlarm = NULL;
	    if(pwaitlist == pAlarm->counter->waitlist){
		xfree(pwaitlist);
		pAlarm->counter->waitlist = NULL;
	    }
	    else
		xfree(pwaitlist);
	}
    }
    if(pAlarm == NULL)
	fprintf(stderr,"DEBUG: pAlarm is NULL!!!");	
    xfree(pAlarm);
}


/*
*** Cleanup after the destruction of a Counter
 */
/* ARGSUSED */

static void FreeCounter(env, id)
pointer env;
CARD32 id;
{    
    SyncAlarm	*pAlarm;
    AlarmClient *pClients;
    WaitList	*pWait, *pnWait;
    xAlarmNotifyEvent ae;
    xCounterNotifyEvent ce;
    SyncCounter *pCounter = (SyncCounter *) env;

    pWait = pCounter->waitlist;
    while( pWait ){
	if(pWait->is_alarm){
	    ae.type = SyncEventBase + AlarmNotify;
	    ae.sequenceNumber = pCounter->owner->sequence;
	    pAlarm = pWait->pAlarm;
	    ae.alarm = pAlarm->id;
	    ae.counter_value_lo = C64Low32(pCounter->value);
	    ae.counter_value_hi = C64High32(pCounter->value);
	    pWait->pAlarm->counter = None;
	    ae.alarm_value_lo = C64Low32(pAlarm->value);
	    ae.alarm_value_hi = C64High32(pAlarm->value);
	    ae.state = AlarmInactive;
	    ae.time = currentTime.milliseconds;
	    fprintf(stderr,"DEBUG: DestoryCounter send AlarmNotify: type: 0x%lx sizeof: 0x%lx value: 0x%lx \n", ae.type, sizeof(ae), pAlarm->value);
	    if(!pCounter->owner->clientGone)
		WriteEventsToClient(pCounter->owner, 1, (xEvent *)&ae);

	    if( (pClients = pWait->pAlarm->clients) != NULL ){
		while(pClients){
/* PDS do the event values need reinitializing??? */
		    if(!pClients->client->clientGone)
			WriteEventsToClient(pClients->client, 1, (xEvent *)&ae);
		    pClients = pClients->next;
		}
	    }
	}
	else {
	    ce.type = SyncEventBase + CounterNotify;
	    ce.sequenceNumber = pCounter->owner->sequence;
	    ce.counter = pWait->pAlarm->counter->id;
	    ce.wait_value_lo = C64Low32(pWait->pAlarm->value);
	    ce.wait_value_hi = C64High32(pWait->pAlarm->value);
	    ce.counter_value_lo = C64Low32(pWait->pAlarm->counter->value);
	    ce.counter_value_hi = C64High32(pWait->pAlarm->counter->value);
	    ce.time = currentTime.milliseconds;
	    ce.count = 0; /* PDS this needs to be verified */
	    ce.destroyed = 1;
	    if(!pCounter->owner->clientGone)
		WriteEventsToClient(pCounter->owner, 1, (xEvent *)&ce);
	    
	}
	pnWait = pWait->next;
	if ((pCounter->owner->clientGone && pCounter->owner->closeDownMode
	    == DestroyAll) || !pCounter->owner->clientGone)
 	    xfree(pWait);
	pWait = pnWait;
    }
/*
    if (pCounter && (pCounter->owner->clientGone 
		&& pCounter->owner->closeDownMode == DestroyAll) 
		|| !pCounter->owner->clientGone){
	pCounter->waitlist = NULL;
	xfree(pCounter);
    }
*/
    if (pCounter)
	if(pCounter->owner)
	if( pCounter->owner->clientGone )
		if( pCounter->owner->closeDownMode == DestroyAll) 
		if( !pCounter->owner->clientGone){
	pCounter->waitlist = NULL;
	xfree(pCounter);
    }
}

/*
*** This client has died...
 */
/* ARGSUSED */
static void FreeAwait(addr, id)
pointer addr;
CARD32 id;
{
    xCounterNotifyEvent e;
    SyncAwait *pnAwait;
    WaitList *pcwaitl, *pprvwl; /* pntr to cntr. waitlist & prev. cntr. list */
    SyncAwait *pAwait = (SyncAwait *)addr;

    while (pAwait) {
	if(!pAwait->client->clientGone){
	    e.type = SyncEventBase + CounterNotify;
	    e.sequenceNumber = pAwait->client->sequence;
	    e.counter = pAwait->counter->id;
	    e.wait_value_lo = C64Low32(pAwait->event_value);
	    e.wait_value_hi = C64High32(pAwait->event_value);
	    e.counter_value_lo = C64Low32(pAwait->counter->value);
	    e.counter_value_hi = C64High32(pAwait->counter->value);
	    e.time = currentTime.milliseconds;
	    e.destroyed = 1;
	    AttendClient(pAwait->client);
	    WriteEventsToClient(pAwait->client, 1, (xEvent *)&e);
	}
	/* check the counter waitlist for this await, adj. ptrs, and NULL
	   this entry */
	if(pAwait->counter){
	    pprvwl = pcwaitl = pAwait->counter->waitlist;
	    while(pcwaitl){
		if((SyncAwait *)pcwaitl->pAlarm == pAwait){
		    pcwaitl->pAlarm = NULL;
		    pprvwl->next = pcwaitl->next;
		    pcwaitl->next = NULL;
		    if(pprvwl->next == NULL 
		       && pcwaitl == pAwait->counter->waitlist)
			pAwait->counter->waitlist = NULL;
		    xfree (pcwaitl);
		    break;
		}
		else{
		    pprvwl = pcwaitl;
		    pcwaitl = pcwaitl->next;
		}
	    }
	}
	    
	pnAwait = pAwait->next;
	if((pAwait->client->clientGone 
	    && pAwait->client->closeDownMode == DestroyAll)
	        || !pAwait->client->clientGone)
	    xfree(pAwait);
	pAwait = pnAwait;
    }
}

/*
*** Get info from the extension
 */
static int ProcSyncInfo(client)
ClientPtr client;
{
    REQUEST(xSyncInfoReq);
    xSyncInfoReply rep;
    int len, n;
    unsigned int *list, *tmplist;

    REQUEST_SIZE_MATCH(xSyncInfoReq);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = SYNC_MAJOR_VERSION;
    rep.minorVersion = SYNC_MINOR_VERSION;
    rep.nCounters = NumSystemCounters;

    for (n=len=0; n<NumSystemCounters; n++)
        len += ((sizeof(SystemCounterRec)+sizeof(int)+
		strlen(SysCounterList[n].name)+1)>>2);

    tmplist = list = (unsigned int *)ALLOCATE_LOCAL(len<<2);
    if (!list) return BadAlloc;

    rep.length = len;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.nCounters, n);
    }

    for (n=0; n<NumSystemCounters; n++) {
	int l = strlen(SysCounterList[n].name) +1;
	if (client->swapped) {
	    int x;
	    cpswapl(SysCounterList[n].counter->id, *tmplist);
	    tmplist++;
	    x = C64Low32(SysCounterList[n].resolution);
	    cpswapl(x, *tmplist);
	    tmplist++;
	    x = C64High32(SysCounterList[n].resolution);
	    cpswapl(x, *tmplist);
	    tmplist++;
	    cpswapl(l, *tmplist);
	    tmplist++;
	} else {
	    *tmplist++ = SysCounterList[n].counter->id;
	    *tmplist++ = C64Low32(SysCounterList[n].resolution);
	    *tmplist++ = C64High32(SysCounterList[n].resolution);
	    *tmplist++ = l;
	}
	strncpy(tmplist, SysCounterList[n].name, l);
	tmplist += l;
    }
    
    WriteToClient(client, sizeof(rep), (char *)&rep);
    WriteToClient(client, len<<2, (char *)list);
    DEALLOCATE_LOCAL(list);

    return (client->noClientException);
}

/*
*** Set client Priority - NOT IMPLEMENTED
 */
static int ProcSyncSetPriority(client)
ClientPtr client;
{
    return Success;
}

/*
*** Get client Priority - NOT IMPLEMENTED
 */
static int ProcSyncGetPriority(client)
ClientPtr client;
{
    int n;

    REQUEST(xSyncGetPriorityReq);
    xSyncGetPriorityReply rep;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.priority = 0; /* send back a default priority */

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swaps(&rep.priority, n);
    }

    WriteToClient(client, sizeof(xSyncGetPriorityReply), (char *)&rep);

    return (client->noClientException);
}

static void DefaultQuery(pCounter, value_return)
SyncCounter *pCounter;
CARD64 *value_return;
{
    *value_return = pCounter->value;
}

/*ARGSUSED*/
static void DefaultNewWait(pCounter, wait_value, wait_type, value_return)
SyncCounter *pCounter;
CARD64 wait_value;
int wait_type;
CARD64 *value_return;
{
    *value_return = pCounter->value;
}

/*ARGSUSED*/
static void DefaultNoWait(pCounter)
SyncCounter *pCounter;
{
}

static void InitCounter(pCounter, initial)
SyncCounter *pCounter;
CARD64 initial;
{
    pCounter->value = initial;
    pCounter->owner = NULL;
    pCounter->Query = DefaultQuery;
    pCounter->waitlist = NULL;
    pCounter->NewWait = DefaultNewWait;
    pCounter->NoWait = DefaultNoWait;
}

pointer SyncCreateSystemCounter(name, initial, resolution,
				query, newwait, nowait)
char *name;
CARD64 initial;
CARD64 resolution;
void (*query)();
void (*newwait)();
void (*nowait)();
{
    SyncCounter *pCounter;
    SysCounter *sysList;

    sysList = (SysCounter *)xrealloc(SysCounterList,
			sizeof(SysCounter)*(NumSystemCounters + 1));
    if (sysList == NULL) return NULL;

    pCounter = (SyncCounter *)xalloc(sizeof(SyncCounter));
    if (!pCounter) return NULL;

    if (RTCounter == 0) {
	RTCounter = CreateNewResourceType(FreeCounter);
	if (RTCounter == 0) {
	    return NULL;
	}
    }

    pCounter->id = FakeClientID(0);
    if (!AddResource(pCounter->id, RTCounter, (pointer)pCounter)) {
	xfree((pointer)pCounter);
	return NULL;
    }

    InitCounter(pCounter, initial);

    pCounter->Query = query;
    pCounter->NewWait = newwait;
    pCounter->NoWait = nowait;
    sysList[NumSystemCounters].name = name;
    sysList[NumSystemCounters].counter = pCounter;
    sysList[NumSystemCounters].resolution = resolution;

    SysCounterList = sysList;
    NumSystemCounters++;

    return (pointer) pCounter;
}

/*
*** Create a new counter
 */
static int ProcSyncCreateCounter(client)
ClientPtr client;
{
    REQUEST(xSyncCreateCounterReq);
    SyncCounter *pCounter;
    CARD64 initial;

    REQUEST_SIZE_MATCH(xSyncCreateCounterReq);

    LEGAL_NEW_RESOURCE(stuff->cid, client);

    pCounter = (SyncCounter *)xalloc(sizeof(SyncCounter));
    if (!pCounter) return BadAlloc;
    pCounter->id = stuff->cid;
    if (!AddResource(pCounter->id, RTCounter, (pointer)pCounter)) {
	xfree((pointer)pCounter);
	return BadAlloc;
    }

/*
 * Initialise the counter structure
 */
    cvtINT32sToC64(initial, stuff->initial_value_lo, stuff->initial_value_hi);
    InitCounter(pCounter, initial);
    pCounter->owner = client;
    return (client->noClientException);
}

/*
*** Set Counter value
 */
static int ProcSyncSetCounter(client)
ClientPtr client;
{
    REQUEST(xSyncSetCounterReq);
    SyncCounter *pCounter;

    pCounter = (SyncCounter *)LookupIDByType(stuff->cid, RTCounter);
    if (pCounter==NULL || pCounter->owner==NULL) {
	client->errorValue = stuff->cid;
	return SyncErrorBase + BadCounter;
    }

    cvtINT32sToC64(pCounter->value, stuff->value_lo, stuff->value_hi);
/* PDS - should be checking list to see if an event should be sent */
    SyncReleaseClients(pCounter);
    return Success;

}

/*
*** Change Counter value
 */
static int ProcSyncChangeCounter(client)
ClientPtr client;
{
    REQUEST(xSyncChangeCounterReq);
    SyncCounter *pCounter;
    CARD64 amount;

    REQUEST_SIZE_MATCH(xSyncChangeCounterReq);

    pCounter = (SyncCounter *)LookupIDByType(stuff->cid, RTCounter);
    if (pCounter==NULL || pCounter->owner==NULL) {
	client->errorValue = stuff->cid;
	return SyncErrorBase + BadCounter;
    }

    cvtINT32sToC64(amount, stuff->value_lo, stuff->value_hi);
    C64Add(pCounter->value, pCounter->value, amount);

    SyncReleaseClients(pCounter);

    return Success;
}

/*
*** Destroy a counter
 */
static int ProcSyncDestroyCounter(client)
ClientPtr client;
{
    REQUEST(xSyncDestroyCounterReq);
    SyncCounter *pCounter;
    SyncAlarm	*pAlarm;
    WaitList	*pWait, *pnWait;
    xAlarmNotifyEvent ae;
    xCounterNotifyEvent ce;

    REQUEST_SIZE_MATCH(xSyncDestroyCounterReq);

    pCounter = (SyncCounter *)LookupIDByType(stuff->counter, RTCounter);
    if (pCounter==NULL || pCounter->owner==NULL) {
	client->errorValue = stuff->counter;
	return SyncErrorBase + BadCounter;
    }
    FreeResource(pCounter->id, RT_NONE);
    return Success;
}



/*
*** Advance a counter to a new value, waking clients as appropriate.
 */
void SyncChangeCounter(pc, val)
pointer pc;
CARD64 val;
{
    SyncCounter *pCounter = (SyncCounter *)pc;
    pCounter->value = val;

    SyncReleaseClients(pCounter);
}

/*
*** Await
 */
static int ProcSyncAwait(client)
ClientPtr client;
{
    REQUEST(xSyncAwaitReq);
    SyncAwait *pAwait, *pSAwait;
    CARD32 * xid;
    int  len, items;
    int size, n, i, x;
    XID *vlist;
    XID *changes;
    SyncCounter **counters;

    REQUEST_AT_LEAST_SIZE(xSyncAwaitReq);

    len = stuff->length << 2;
    len -= sz_xSyncAwaitReq;
    /*
     * Calc. size of a waitlist sent over from the client side.
     * The client doesn't include event_threshold.
     */
    size = sizeof (WaitCondition) - sizeof(WaitCondition.event_threshold) -4; /* PDS was 6, then change to WAITCONDITION,to now */
/*
 * Each Wait is 6 words:
 */

    items = len/size;

    /* PDS needs work on BadLength error */
    if (items==0 || items*size != len) {
fprintf(stderr,"DEBUG: return BadLength from ProcSyncAwait items: %d items*6: %d len: %d\n", items, items*size, len );
	return BadLength;
    }

    if ( sizeof(XID) != sizeof(CARD32) ){
	if(!(vlist = changes = (XID *)xalloc((stuff->length)*sizeof(long))))
	    return BadAlloc;
	xid = (CARD32 *) &stuff[1];
	for ( i = 0; i < stuff->length; i++ ){
	    changes[i] = *xid++;
	}
	    changes = vlist;
    }
    else
	changes = (XID *)&stuff[1];

    for( x = 0; x < items; x++ ){
    	if(!(pAwait = (SyncAwait *)xalloc(sizeof(SyncAwait))))
    	    return BadAlloc;
    	pAwait->client = client;
    	if(!(pAwait->counter = (SyncCounter *)LookupIDByType(*changes, RTCounter))){
    	    client->errorValue = *changes;
    	    return SyncErrorBase + BadCounter;
    	}
	changes++;
    	pAwait->value_type = *changes++;
	cvtINT32sToC64(pAwait->value, (CARD32)changes[0], (CARD32)changes[1]);
    	changes += 2;
    	pAwait->test_type = *changes++;
	pAwait->next = NULL;

/* need to check for error returns - PDS */
	
    	AddWaitItem(pAwait);
	pAwait->counter->waitlist->is_alarm = 0;
	pAwait->id = FakeClientID(client->index);
    	AddResource(pAwait->id, RTAwait, pAwait);
    	InitTrigger(pAwait);
    	if ( x == 0 )
    	    pSAwait = pAwait;
    	else{
    	    pAwait->next = pSAwait->next;
    	    pSAwait->next = pAwait;
    	}
    }

    xfree(vlist);
    IgnoreClient(client);
    TriggerAwait(pSAwait);
    return Success;
}


/*
*** Query a counter
 */
static int ProcSyncQueryCounter(client)
ClientPtr client;
{
    REQUEST(xSyncQueryCounterReq);
    xSyncQueryCounterReply rep;
    SyncCounter *pCounter;

    REQUEST_SIZE_MATCH(xSyncQueryCounterReq);

    pCounter = (SyncCounter *)LookupIDByType(stuff->counter, RTCounter);
    if (pCounter==NULL) {
	client->errorValue = stuff->counter;
	return SyncErrorBase + BadCounter;
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if (pCounter->Query) (*pCounter->Query)((pointer)pCounter,
					    &pCounter->value);

    rep.value_lo = C64Low32(pCounter->value);
    rep.value_hi = C64High32(pCounter->value);
    if (client->swapped) {
	register long n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.value_lo, n);
	swapl(&rep.value_hi, n);
    }
    WriteToClient(client, sizeof(xSyncQueryCounterReply), (char *)&rep);
    return (client->noClientException);
}

/*
*** ParseAlarmAttributes
*** This is used by CreateAlarm and ChangeAlarm
 */
static int ParseAlarmAttributes(client, pAlarm, mask, values)
ClientPtr client;
SyncAlarm *pAlarm;
unsigned long mask;
XID *values;
{
    AlarmClient *pClients;
    WaitList *wl, *pWaitList, *powl;
    SyncCounter *newcounter = NULL;
    int events = 1;

    if(client != pAlarm->client){
	for (pClients = pAlarm->clients; pClients; pClients = pClients->next){
	    if (pClients->client == client) break;
	}
	if (!pClients) { 
	    pClients = (AlarmClient *)xalloc(sizeof(AlarmClient));
	    if (!pClients)  return BadAlloc;
	    pClients->next = pAlarm->clients;
	    pAlarm->clients = pClients;
	    pClients->client = client;
	    pClients->events = 1;
	    pClients->id = FakeClientID(client->index);
	    if (!AddResource(pClients->id, RTAlarmClient, pClients))
		return BadAlloc;
	}
    }

    while (mask) {
	int index = lowbit (mask);  
	mask &= ~index; 
	switch (index) {

/* PDS review this section carefully */
	case CACounter:
	    mask &= ~CACounter;

	    /* quick check for NULL changing to NULL */
	    if( pAlarm->counter == NULL && *values == NULL )
		break;
	    if( *values ){
		if(!(newcounter = (SyncCounter *)LookupIDByType(*values, RTCounter))){
		    client->errorValue = *values;
		    return SyncErrorBase + BadCounter;
		}
	    }
	    values++;
	    /* quick check for changing the counter to itself */
	    if (pAlarm->counter == newcounter)
		break;
	    pAlarm->counter = newcounter;

	    /* special case for new WaitList for the counter */
	    if(!pAlarm->counter->waitlist){
		if(!(pAlarm->counter->waitlist = (WaitList *)xalloc(sizeof(WaitList))))
		    return BadAlloc;

		pAlarm->counter->waitlist->pAlarm = pAlarm;
		pAlarm->counter->waitlist->next = NULL;
		pAlarm->counter->waitlist->is_alarm = xTrue;
		break;
	    }
		    
	    /* special case for changing first alarm on WaitList */
	    if(pAlarm == pAlarm->counter->waitlist->pAlarm){
		powl = pAlarm->counter->waitlist;
		pAlarm->counter->waitlist = pAlarm->counter->waitlist->next;
		if(newcounter){
		    powl->next = newcounter->waitlist;
		    newcounter->waitlist = powl;
		}
		else 
		    xfree(powl); /* no counter to attach the waitlist to */
		pAlarm->counter = newcounter;
		break;
	    }

	    /* look for the alarm on the waitlist */
	    for( pWaitList = pAlarm->counter->waitlist; pWaitList; pWaitList = pWaitList->next ){
		if(pAlarm == pWaitList->next->pAlarm){
		    powl = pWaitList->next;
		    pWaitList->next = pWaitList->next->next; /* removed from WaitList */
		    if(newcounter){
			powl->next = newcounter->waitlist;
			newcounter->waitlist = powl;
		    }
		    else 
			xfree(powl); /* no counter to attach the waitlist to */
		    pAlarm->counter = newcounter;
		    break;
	        }
    	    }
	    break;

	case CAValueType:
	    mask &= ~CAValueType;
	    pAlarm->value_type = (CARD32)*values++;
	    break;

	case CAValue:
	    mask &= ~CAValue;
	    cvtINT32sToC64(pAlarm->value,(CARD32)values[0], (CARD32)values[1]);
	    values += 2;
	    break;

	case CATestType:
	    mask &= ~CATestType;
	    pAlarm->test_type = (CARD32)*values++;
	    break;

	case CADelta:
	    mask &= ~CADelta;
	    cvtINT32sToC64(pAlarm->delta,(CARD32)values[0], (CARD32)values[1]);
	    values += 2;
	    break;

	case CAEvents:
	    mask &= ~CAEvents;
	    if(client != pAlarm->client)
		pClients->events = (CARD32)*values++;
	    else
		pAlarm->events = (CARD32)*values++;
	    break;


	default:
	    client->errorValue = mask;
	    return BadValue;
	}
    }
    return Success;
}

/*
*** Create Alarm
 */
static int ProcSyncCreateAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncCreateAlarmReq);
    SyncAlarm *pAlarm;
    CARD32 * xid;
    XID changes[32];  /* PDS double check, make sure this is max. size. */    
    int status;
    int i;
    unsigned long len, vmask; 
    XID *vlist;

    REQUEST_AT_LEAST_SIZE(xSyncCreateAlarmReq);

    LEGAL_NEW_RESOURCE(stuff->id, client);

    vmask = stuff->valueMask;
    len = stuff->length - (sizeof(xSyncCreateAlarmReq) >> 2);
/*
 * Check the length
 * The Value, Increment and Limit fields need 2 words each
 *
 * -2 because stuff->length includes 2 words that are not part of the mask.
 */
    if(len){
	    if ((len-2) != Ones(vmask))
		    return BadLength; 
    }else
	    if ( len != Ones(vmask))
		    return BadLength;

    if (!(pAlarm = (SyncAlarm *)xalloc(sizeof(SyncAlarm)))) {
	return BadAlloc;
    }

    pAlarm->id = stuff->id;
    pAlarm->client = client;
    pAlarm->clients = NULL;
    pAlarm->state = AlarmActive;
    pAlarm->events = TRUE;
    cvtLongToC64(pAlarm->delta, 1L);
    pAlarm->value_type = SyncAbsolute;
    pAlarm->test_type = PositiveComparison;
    cvtLongToC64(pAlarm->value, 0L);
    pAlarm->counter = None;
    pAlarm->value_type = SyncAbsolute;
    cvtLongToC64(pAlarm->value, 0L);

    if ( sizeof(XID) != sizeof(CARD32) ){
	    xid = (CARD32 *) &stuff[1];
	    for ( i = 0; i < len; i ++ ){
		    changes[i] = *xid++;
	    }
	    status = ParseAlarmAttributes(client, pAlarm, vmask, changes);
    }
    else
	    status = ParseAlarmAttributes(client, pAlarm, vmask, 
					  (XID *)&stuff[1]);
	    
    if (status != Success) {
	xfree(pAlarm);
	return status;
    }

    if (!AddResource(stuff->id, RTAlarm, pAlarm)) {
	xfree(pAlarm);
	return BadAlloc;
    }

    AddWaitItem(pAlarm);
    if(pAlarm->counter)
	pAlarm->counter->waitlist->is_alarm = 1;
    InitTrigger(pAlarm);
    SyncReleaseClients(pAlarm->counter);
    return Success;
}

/*
*** Change Alarm
 */
static int ProcSyncChangeAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncChangeAlarmReq);
    SyncAlarm *pAlarm;
    CARD32 * xid;
    XID changes[32];
    long vmask;
    XID *vlist;
    int len, status, i;
    SyncCounter *pOldCounter;

    REQUEST_AT_LEAST_SIZE(xSyncChangeAlarmReq);

    pAlarm = (SyncAlarm *)LookupIDByType(stuff->alarm, RTAlarm);
    if (!pAlarm) {
	client->errorValue = stuff->alarm;
	return SyncErrorBase + BadAlarm;
    }

    vmask = stuff->valueMask;
    len = stuff->length - (sizeof(xSyncChangeAlarmReq) >> 2);

/*
 * Check the length
 * The Value, Increment and Limit fields need 2 words each
 *
 * -2 because stuff->length includes 2 words that are not part of the mask.
 */
    if(len){
	    if ((len-2) != Ones(vmask))
		    return BadLength; 
    }else
	    if ( len != Ones(vmask))
		    return BadLength;

    vmask = stuff->valueMask;
    vlist = (XID *)&stuff[1];

    pOldCounter = pAlarm->counter;

    if ( sizeof(XID) != sizeof(CARD32) ){
	    xid = (CARD32 *) &stuff[1];
	    for ( i = 0; i < len; i ++ ){
		    changes[i] = *xid++;
	    }
	    status = ParseAlarmAttributes(client, pAlarm, vmask, changes);
    }
    else
	    status = ParseAlarmAttributes(client, pAlarm, vmask, 
					  (XID *)&stuff[1]);

    if (status != Success) {
	return status;
    }

    InitTrigger(pAlarm);
    SyncReleaseClients(pAlarm->counter);
    return Success;
}

static int ProcSyncQueryAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncQueryAlarmReq);
    SyncAlarm *pAlarm;
    xSyncQueryAlarmReply rep;

    REQUEST_SIZE_MATCH(xSyncQueryAlarmReq);
  
    pAlarm = (SyncAlarm *)LookupIDByType(stuff->alarm, RTAlarm);
    if (!pAlarm) {
	client->errorValue = stuff->alarm;
	return (SyncErrorBase + BadAlarm);
    }

    rep.type = X_Reply;
    rep.length = sizeof(xSyncQueryAlarmReply) - sizeof(xGenericReply) >> 2;
    rep.sequenceNumber = client->sequence;

    if( pAlarm->counter != NULL )
	    rep.counter = pAlarm->counter->id;
    else
	    rep.counter = 0;

    rep.wait_type = pAlarm->value_type;
    rep.wait_value_lo = C64Low32(pAlarm->value);
    rep.wait_value_hi = C64High32(pAlarm->value);
    rep.test_type = pAlarm->test_type;
    rep.delta_lo = C64Low32(pAlarm->delta);
    rep.delta_hi = C64High32(pAlarm->delta);
    rep.events = pAlarm->events;
    rep.state = pAlarm->state;

    if (client->swapped) {
	register long n;
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.counter, n);
	swapl(&rep.wait_value_lo, n);
	swapl(&rep.wait_value_hi, n);
	swapl(&rep.delta_lo, n);
	swapl(&rep.delta_hi, n);
    }

    WriteToClient(client, sizeof(xSyncQueryAlarmReply), (char *)&rep);
    return (client->noClientException);

}


static int ProcSyncDestroyAlarm(client)
ClientPtr client;
{
    SyncAlarm *pAlarm;
    REQUEST(xSyncDestroyAlarmReq);
    
    REQUEST_SIZE_MATCH(xSyncDestroyAlarmReq);
  
    if (!(pAlarm = (SyncAlarm *)LookupIDByType(stuff->alarm, RTAlarm))) {
	client->errorValue = stuff->alarm;
	return SyncErrorBase + BadAlarm;
    }

    FreeResource(stuff->alarm, RT_NONE);      
    return (client->noClientException);
}

/*
*** Given an extension request, call the appropriate request procedure
 */
static int ProcSyncDispatch(client)
ClientPtr client;
{
    REQUEST(xReq);

    switch (stuff->data) {

    case X_SyncInfo:
	return ProcSyncInfo(client);
    case X_SyncCreateCounter:
	return ProcSyncCreateCounter(client);
    case X_SyncSetCounter:
	return ProcSyncSetCounter(client);
    case X_SyncChangeCounter:
	return ProcSyncChangeCounter(client);
    case X_SyncQueryCounter:
	return ProcSyncQueryCounter(client);
    case X_SyncDestroyCounter:
	return ProcSyncDestroyCounter(client);
    case X_SyncAwait:
	return ProcSyncAwait(client);
    case X_SyncCreateAlarm:
	return ProcSyncCreateAlarm(client);
    case X_SyncChangeAlarm:
	return ProcSyncChangeAlarm(client);
    case X_SyncQueryAlarm:
	return ProcSyncQueryAlarm(client);
    case X_SyncDestroyAlarm:
	return ProcSyncDestroyAlarm(client);
    case X_SyncSetPriority:
	return ProcSyncSetPriority(client);
    case X_SyncGetPriority:
	return ProcSyncGetPriority(client);
    default:
	return BadRequest;
    }
}

/*
 * Boring Swapping stuff ...
 */

SProcSyncCreateCounter(client)
ClientPtr client;
{
    REQUEST(xSyncCreateCounterReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->cid, n);
    swapl(&stuff->initial_value_lo, n);
    swapl(&stuff->initial_value_hi, n);

    return ProcSyncCreateCounter(client);
}
	
static int SProcSyncQueryCounter(client)
ClientPtr client;
{
    REQUEST(xSyncQueryCounterReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->counter, n);

    return ProcSyncQueryCounter(client);
}

static int SProcSyncDestroyCounter(client)
ClientPtr client;
{
    REQUEST(xSyncDestroyCounterReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->counter, n);

    return ProcSyncDestroyCounter(client);
}

static int SProcSyncAwait(client)
ClientPtr client;
{
    REQUEST(xSyncAwaitReq);
    register char n;

    swaps(&stuff->length, n);
    SwapRestL(stuff);

    return ProcSyncAwait(client);
}


static int SProcSyncCreateAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncCreateAlarmReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->id, n);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);

    return ProcSyncCreateAlarm(client);
}

static int SProcSyncChangeAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncChangeAlarmReq);
    register char n;

    REQUEST_AT_LEAST_SIZE(xSyncChangeAlarmReq);
    swaps(&stuff->length, n);
    swapl(&stuff->alarm, n);
    swapl(&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSyncChangeAlarm(client);
}

static int SProcSyncQueryAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncQueryAlarmReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->alarm, n);

    return ProcSyncQueryAlarm(client);
}

static int SProcSyncDestroyAlarm(client)
ClientPtr client;
{
    REQUEST(xSyncDestroyAlarmReq);
    register char n;

    swaps(&stuff->length, n);
    swapl(&stuff->alarm, n);

    return ProcSyncDestroyAlarm(client);
}


static int SProcSyncDispatch(client)
ClientPtr client;
{
int n;

    REQUEST(xReq);

    switch (stuff->data) {

    case X_SyncInfo:
      swaps(&stuff->length, n);
      return ProcSyncInfo(client);

    case X_SyncCreateCounter:
	return SProcSyncCreateCounter(client);

    case X_SyncSetCounter:
	return ProcSyncSetCounter(client);

    case X_SyncChangeCounter:
	return ProcSyncChangeCounter(client);

    case X_SyncQueryCounter:
	return SProcSyncQueryCounter(client);

    case X_SyncDestroyCounter:
	return SProcSyncDestroyCounter(client);

    case X_SyncAwait:
	return SProcSyncAwait(client);

    case X_SyncCreateAlarm:
	return SProcSyncCreateAlarm(client);

    case X_SyncChangeAlarm:
	return SProcSyncChangeAlarm(client);

    case X_SyncQueryAlarm:
	return SProcSyncQueryAlarm(client);

    case X_SyncDestroyAlarm:
	return SProcSyncDestroyAlarm(client);

    case X_SyncSetPriority:
	return ProcSyncSetPriority(client);

    case X_SyncGetPriority:
	return ProcSyncGetPriority(client);

    default:
	return BadRequest;
    }
}

/*
 * Event Swapping
 */

static void SCounterNotifyEvent(from, to)
xCounterNotifyEvent *from, *to;
{
    to->type = from->type;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->counter, to->counter);
    cpswapl(from->wait_value_lo, to->wait_value_lo);
    cpswapl(from->wait_value_hi, to->wait_value_hi);
    cpswapl(from->counter_value_lo, to->counter_value_lo);
    cpswapl(from->counter_value_hi, to->counter_value_hi);
    cpswapl(from->time, to->time);
}


static void SAlarmNotifyEvent(from, to)
xAlarmNotifyEvent *from, *to;
{
    to->type = from->type;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->alarm, to->alarm);
    cpswapl(from->alarm_value_lo, to->alarm_value_lo);
    cpswapl(from->alarm_value_hi, to->alarm_value_hi);
    cpswapl(from->counter_value_lo, to->counter_value_lo);
    cpswapl(from->counter_value_hi, to->counter_value_hi);
    cpswapl(from->time, to->time);
    to->state = from->state;
}

/*
*** Close everything down.
*** This is fairly simple for now.
 */
/*ARGSUSED*/
static void SyncResetProc(extEntry)
ExtensionEntry	*extEntry;
{
    xfree(SysCounterList);
    SysCounterList = NULL;
    NumSystemCounters = 0;
    RTCounter = 0;
}


/*
*** Initialise the extension.
 */
void SyncExtensionInit()
{
    ExtensionEntry *extEntry;

    if (RTCounter == 0) {
	RTCounter = CreateNewResourceType(FreeCounter);
    }
    RTAlarm = CreateNewResourceType(FreeAlarm);
    RTAwait = CreateNewResourceType(FreeAwait);
/* was FreeAlarmClient, but all work is done in FreeAlarm */
    RTAlarmClient = CreateNewResourceType(NULL);

    if (RTCounter == NULL ||
	RTAwait == NULL ||
	RTAlarm == NULL ||
	RTAlarmClient == NULL ||
	(extEntry = AddExtension(SYNCNAME,
				 SyncNumberEvents, SyncNumberErrors,
				 ProcSyncDispatch, SProcSyncDispatch,
				 SyncResetProc,
				 StandardMinorOpcode))==NULL) {
	fprintf(stderr, "Sync Extension %d.%d failed to Initialise\n",
		SYNC_MAJOR_VERSION, SYNC_MINOR_VERSION);
	return;
    }

    SyncReqCode = extEntry->base;
    SyncEventBase = extEntry->eventBase;
    SyncErrorBase = extEntry->errorBase;
    EventSwapVector[SyncEventBase + CounterNotify] = SCounterNotifyEvent;
    EventSwapVector[SyncEventBase + AlarmNotify] = SAlarmNotifyEvent;

/*
 * Although SERVERTIME is implemented by the OS layer, we initialise it here
 * because doing it in OsInit() is too early. The resource database is not
 * initialised when OsInit() is called.
 * This is just about OK because there is always a servertime counter.
 */
    SyncInitServertime();

#ifdef DEBUG
    fprintf(stderr, "Sync Extension %d.%d\n",
	    SYNC_MAJOR_VERSION, SYNC_MINOR_VERSION);
#endif
}
#endif /* SYNC */
