/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/


/* $Header: events.c,v 1.88 87/08/11 12:53:29 toddb Locked $ */

#include "X.h"
#include "misc.h"
#include "resource.h"
#define NEED_EVENTS
#define NEED_REPLIES
#include "Xproto.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "dixstruct.h"

extern WindowRec WindowTable[];
extern int swappedClients[];

extern int (* EventSwapVector[128]) ();
extern int (* ReplySwapVector[256]) ();

#define NoSuchEvent 0x80000000	/* so doesn't match NoEventMask */
#define StructureAndSubMask ( StructureNotifyMask | SubstructureNotifyMask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define MotionMask ( \
	PointerMotionMask | PointerMotionHintMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PropagateMask ( \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	MotionMask )
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
#define Motion_Filter(state) (PointerMotionMask | \
		(AllButtonsMask & state) | buttonMotionMask);


#define WID(w) ((w) ? ((w)->wid) : 0)

#define BitOn(ptr, bit) \
	((BYTE *) (ptr))[(bit)>>3] |= (1 << ((bit) & 7))
#define BitOff(ptr, bit) \
	((BYTE *) (ptr))[(bit)>>3] &= ~(1 << ((bit) & 7))
#define IsOn(ptr, bit) \
	(((BYTE *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))

static debug_events = 0;
static InputInfo inputInfo;

static lastInputFocusChangeMode = NotifyNormal; /* Useful for avoiding sending
						 focus events when the input
						 focus is sent twice to the
						 same window. PRH */

static KeySymsRec curKeySyms;

static GrabRec keybdGrab;	/* used for active grabs */
static GrabRec ptrGrab;

#define MAX_QUEUED_EVENTS 100
static struct {
    unsigned int	num;
    QdEventRec		pending, free;	/* only forw, back used */
    DeviceIntPtr	replayDev;	/* kludgy rock to put flag for */
    WindowPtr		replayWin;	/*   ComputeFreezes            */
    Bool		playingEvents;
} syncEvents;

/*
 * The window trace information is used to avoid having to compute all the
 * windows between the root and the current pointer window each time a button
 * or key goes down. The grabs on each of those windows must be checked.
 */
static WindowPtr *spriteTrace = (WindowPtr *)NULL;
#define ROOT spriteTrace[0]
static int spriteTraceSize = 0;
static int spriteTraceGood;

static WindowPtr *focusTrace = (WindowPtr *)NULL;
static int focusTraceSize = 0;
static int focusTraceGood;

static CARD16 keyButtonState = 0;
static int buttonsDown = 0;		/* number of buttons currently down */
static Mask buttonMotionMask = 0;

typedef struct {
    int		x, y;
} HotSpot;

static  struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints */
    BoxRec	physLimits;	/* of hot spot due to hardware limits */
    WindowPtr	win;
    HotSpot	hot;
} sprite;			/* info about the cursor sprite */

static Bool lastWasMotion;

extern void DoEnterLeaveEvents();	/* merely forward declarations */
extern WindowPtr XYToWindow();
extern Bool CheckKeyboardGrabs();
extern void NormalKeyboardEvent();
extern int DeliverDeviceEvents();
extern void DoFocusEvents();

static ScreenPtr currentScreen;

static CARD16 stateMasks[MAP_LENGTH];	/* only for default keyboard ? XXX */

static int lastEventMask;

#define CantBeFiltered NoEventMask
static int filters[128] =
{
	NoSuchEvent,		       /* 0 */
	NoSuchEvent,		       /* 1 */
	KeyPressMask,		       /* KeyPress */
	KeyReleaseMask,		       /* KeyRelease */
	ButtonPressMask,	       /* ButtonPress */
	ButtonReleaseMask,	       /* ButtonRelease */
	MotionMask,		       /* MotionNotify - special cased */
	EnterWindowMask,	       /* EnterNotify */
	LeaveWindowMask,	       /* LeaveNotify */
	FocusChangeMask,	       /* FocusIn */
	FocusChangeMask,	       /* FocusOut */
	KeymapStateMask,	       /* KeymapNotify */
	ExposureMask,		       /* Expose */
	CantBeFiltered,		       /* GraphicsExpose */
	CantBeFiltered,		       /* NoExpose */
	VisibilityChangeMask,	       /* VisibilityNotify */
	SubstructureNotifyMask,	       /* CreateNotify */
	StructureAndSubMask,	       /* DestroyNotify */
	StructureAndSubMask,	       /* UnmapNotify */
	StructureAndSubMask,	       /* MapNotify */
	SubstructureRedirectMask,      /* MapRequest */
	SubstructureNotifyMask,	       /* ReparentNotify */
	StructureAndSubMask,	       /* ConfigureNotify */
	SubstructureRedirectMask,      /* ConfigureRequest */
	StructureAndSubMask,	       /* GravityNotify */
	ResizeRedirectMask,	       /* ResizeRequest */
	StructureAndSubMask,	       /* CirculateNotify */
	SubstructureRedirectMask,      /* CirculateRequest */
	PropertyChangeMask,	       /* PropertyNotify */
	CantBeFiltered,		       /* SelectionClear */
	CantBeFiltered,		       /* SelectionRequest */
	CantBeFiltered,		       /* SelectionNotify */
	ColormapChangeMask,	       /* ColormapNotify */
	CantBeFiltered		       /* InterpretNotify */
};

Mask
GetNextEventMask()
{
    lastEventMask <<= 1;
    return lastEventMask;
}

void
SetMaskForEvent(mask, event)
    Mask mask;
    int event;
{
    if ((event < LASTEvent) || (event >= 128))
	FatalError("MaskForEvent: bogus event number");
    filters[event] = mask;
}

static void
CheckPhysLimits(cursor)
    CursorPtr cursor;
{
    HotSpot old;

    if (!cursor)
	return;
    old = sprite.hot;
    (*currentScreen->CursorLimits) (
	currentScreen, cursor, &sprite.hotLimits, &sprite.physLimits);
/*
 * Notice that the following adjustments leave the hot spot not really where
 * it would be if you looked at the screen. This is probably the right thing
 * to do since the user does not want the hot spot to move just because the
 * hardware cannot display the physical sprite where we would like it. The
 * next time the pointer device moves, the hot spot will then "jump" to its
 * visual position.
 */
    if (sprite.hot.x < sprite.physLimits.x1)
	sprite.hot.x = sprite.physLimits.x1;
    else
	if (sprite.hot.x >= sprite.physLimits.x2)
	    sprite.hot.x = sprite.physLimits.x2 - 1;
    if (sprite.hot.y < sprite.physLimits.y1)
	sprite.hot.y = sprite.physLimits.y1;
    else
	if (sprite.hot.y >= sprite.physLimits.y2)
	    sprite.hot.y = sprite.physLimits.y2 - 1;
    if ((old.x != sprite.hot.x) || (old.y != sprite.hot.y))
	(*currentScreen->SetCursorPosition) (
	    currentScreen, sprite.hot.x, sprite.hot.y);
}

static void
NewCursorConfines(x1, x2, y1, y2)
    int x1, x2, y1, y2;
{
    sprite.hotLimits.x1 = x1;
    sprite.hotLimits.x2 = x2;
    sprite.hotLimits.y1 = y1;
    sprite.hotLimits.y2 = y2;
    CheckPhysLimits(sprite.current);
    (* currentScreen->ConstrainCursor)(currentScreen, &sprite.physLimits);
}

static void
ChangeToCursor(cursor)
    CursorPtr cursor;
{
    if (!cursor)
	FatalError("Somebody is setting NullCursor");
    if (cursor != sprite.current)
    {
	if ((sprite.current->xhot != cursor->xhot) ||
		(sprite.current->yhot != cursor->yhot))
	    CheckPhysLimits(cursor);
	(*currentScreen->DisplayCursor) (
	   currentScreen, cursor, sprite.hot.x, sprite.hot.y);
	sprite.current = cursor;
    }
}

static void
PostNewCursor()
{
    register    WindowPtr win;
    register    GrabPtr grab = inputInfo.pointer->grab;
    if (grab)
    {
	if (grab->u.ptr.cursor)
	{
	    ChangeToCursor(grab->u.ptr.cursor);
	    return;
	}
	if (IsParent(grab->window, sprite.win))
	    win = sprite.win;
	else
	    win = grab->window;
    }
    else
	win = sprite.win;
    for (; win; win = win->parent)
	if (win->cursor != NullCursor)
	{
	    ChangeToCursor(win->cursor);
	    return;
	}
}

/**************************************************************************
 *            The following procedures deal with synchronous events       *
 **************************************************************************/

static void
EnqueueEvent(device, event)
    xEvent		*event;
    DeviceIntPtr	device;
{
    register QdEventPtr tail = syncEvents.pending.back;
    register QdEventPtr new;
/*
 * Collapsing of mouse events does not bother to test if qdEvents.num == 0,
 * since there will never be MotionNotify in the type of the head event which
 * is what last points at when num == 0.
 */
    if ((event->u.u.type == MotionNotify) && 
	(tail->event.u.u.type == MotionNotify))
    {
	tail->event = *event;
	return;
    }
    syncEvents.num++;
    if (syncEvents.free.forw == &syncEvents.free)
	new = (QdEventPtr)Xalloc(sizeof(QdEventRec));
    else
    {
	new = syncEvents.free.forw;
	remque(new);
    }
    new->device = device;
    new->event = *event;
    insque(new, tail);
    if (syncEvents.num > MAX_QUEUED_EVENTS)
    {
	/* XXX here we send all the pending events and break the locks */
	return;
    }
}

static void
PlayReleasedEvents()
{
    register QdEventPtr qe = syncEvents.pending.forw;
    QdEventPtr next;
    while (qe != &syncEvents.pending)
    {
	register DeviceIntPtr device = qe->device;
	if (!device->sync.frozen)
	{
	    next = qe->forw;;
	    remque(qe);
	    (*device->public.processInputProc)(&qe->event, device);
	    insque(qe, &syncEvents.free);
	    qe = next;
	}
	else
	    qe = qe->forw;
    } 
}

static void
ComputeFreezes(dev1, dev2)
    DeviceIntPtr dev1, dev2;
{
    register DeviceIntPtr replayDev = syncEvents.replayDev;
    int i;
    WindowPtr w;
    Bool isKbd ;
    register xEvent *xE ;

    dev1->sync.frozen =
	((dev1->sync.other != NullGrab) || (dev1->sync.state >= FROZEN));
    dev2->sync.frozen =
	((dev2->sync.other != NullGrab) || (dev2->sync.state >= FROZEN));
    if (syncEvents.playingEvents)
	return;
    syncEvents.playingEvents = TRUE;
    if (replayDev)
    {
	isKbd = (replayDev == inputInfo.keyboard);
	xE = &replayDev->sync.event;
	syncEvents.replayDev = (DeviceIntPtr)NULL;
	w = XYToWindow(
	    xE->u.keyButtonPointer.rootX, xE->u.keyButtonPointer.rootY);
	for (i = 0; i < spriteTraceGood; i++)
	    if (syncEvents.replayWin == spriteTrace[i])
	    {
		if (!CheckDeviceGrabs(replayDev, xE, i+1, isKbd))
		    if (isKbd)
			NormalKeyboardEvent(replayDev, xE, w);
		    else
			DeliverDeviceEvents(w, xE, NullGrab, NullWindow);
		goto playmore;
	    }
	/* must not still be in the same stack */
	if (isKbd)
	    NormalKeyboardEvent(replayDev, xE, w);
	else
	    DeliverDeviceEvents(w, xE, NullGrab, NullWindow);
    }
playmore:
    if (!dev1->sync.frozen || !dev2->sync.frozen)
	PlayReleasedEvents();
    syncEvents.playingEvents = FALSE;
}

CheckGrabForSyncs(grab, thisDev, thisMode, otherDev, otherMode)
    GrabPtr grab;
    DeviceIntPtr thisDev, otherDev;
    int thisMode, otherMode;
{
    if (thisMode == GrabModeSync)
	thisDev->sync.state = FROZEN_NO_EVENT;
    else
    {	/* free both if same client owns both */
	thisDev->sync.state = THAWED;
	if (thisDev->sync.other &&
	    (thisDev->sync.other->client == grab->client))
	    thisDev->sync.other = NullGrab;
    }
    if (otherMode == GrabModeSync)
	otherDev->sync.other = grab;
    else
    {	/* free both if same client owns both */
	if (otherDev->sync.other &&
	    (otherDev->sync.other->client == grab->client))
	    otherDev->sync.other = NullGrab;
	if ((otherDev->sync.state >= FROZEN) &&
	    (otherDev->grab->client == grab->client))
	    otherDev->sync.state = THAWED;
    }
    ComputeFreezes(thisDev, otherDev);
}

static void
ActivatePointerGrab(mouse, grab, time, autoGrab)
    GrabPtr grab;
    register DeviceIntPtr mouse;
    TimeStamp time;
    Bool autoGrab;
{
    WindowPtr w;

    mouse->grabTime = time;
    ptrGrab = *grab;
    mouse->grab = &ptrGrab;
    mouse->u.ptr.autoReleaseGrab = autoGrab;
    CheckGrabForSyncs(
	mouse->grab, mouse, grab->pointerMode,
	inputInfo.keyboard, grab->keyboardMode);
    PostNewCursor();
    DoEnterLeaveEvents(sprite.win, grab->window, NotifyGrab);
    if (w = grab->u.ptr.confineTo)
    {
	NewCursorConfines(
	    w->absCorner.x, w->absCorner.x + w->clientWinSize.width,
	    w->absCorner.y, w->absCorner.y + w->clientWinSize.height);
    }
}

static void
DeactivatePointerGrab(mouse)
    DeviceIntPtr mouse;
{
    GrabPtr grab = mouse->grab;
    DeviceIntPtr keybd = inputInfo.keyboard;

    DoEnterLeaveEvents(grab->window, sprite.win, NotifyUngrab);
    mouse->grab = NullGrab;
    mouse->sync.state = NOT_GRABBED;
    mouse->u.ptr.autoReleaseGrab = FALSE;
    if (keybd->sync.other == grab)
	keybd->sync.other = NullGrab;
    ComputeFreezes(keybd, mouse);
    if (grab->u.ptr.confineTo)
	NewCursorConfines(0, currentScreen->width, 0, currentScreen->height);
    PostNewCursor();
}

static void
ActivateKeyboardGrab(keybd, grab, time, passive)
    GrabPtr grab;
    register DeviceIntPtr keybd;
    TimeStamp time;
    Bool passive;
{
    keybd->grabTime = time;
    keybdGrab = *grab;
    keybd->grab = &keybdGrab;
    keybd->u.keybd.passiveGrab = passive;
    CheckGrabForSyncs(
	keybd->grab, keybd, grab->keyboardMode,
	inputInfo.pointer, grab->pointerMode);
}

static void
DeactivateKeyboardGrab(keybd)
    DeviceIntPtr keybd;
{
    DeviceIntPtr mouse = inputInfo.pointer;
    GrabPtr grab = keybd->grab;

    keybd->grab = NullGrab;
    keybd->sync.state = NOT_GRABBED;
    keybd->u.keybd.passiveGrab = FALSE;
    if (mouse->sync.other == grab)
	mouse->sync.other = NullGrab;
    ComputeFreezes(keybd, mouse);
}

static void
AllowSome(client, time, thisDev, otherDev, minFreeze)
    ClientPtr		client;
    TimeStamp		time;
    DeviceIntPtr	thisDev, otherDev;
    int			minFreeze;
{
    if (!thisDev->sync.frozen)
	return;
    if (CompareTimeStamps(time, thisDev->grabTime) == EARLIER)
	return;
    if (thisDev->sync.state < minFreeze)
	return;
    switch (minFreeze)
    {
	case NOT_GRABBED: 	       /* Async */
	    if (thisDev->grab && (thisDev->grab->client == client))
		thisDev->sync.state = THAWED;
	    if (thisDev->sync.other && (thisDev->sync.other->client == client))
		thisDev->sync.other = NullGrab;
	    ComputeFreezes(thisDev, otherDev);
	    break;
	case FROZEN_NO_EVENT:		/* Sync */
	    if (thisDev->grab->client == client)
		thisDev->sync.state = FREEZE_NEXT_EVENT;
	    ComputeFreezes(thisDev, otherDev);
	    break;
	case FROZEN_WITH_EVENT:		/* Replay */
	    if (thisDev->grab->client == client)
	    {
		syncEvents.replayDev = thisDev;
		syncEvents.replayWin = thisDev->grab->window;
		if (thisDev == inputInfo.pointer)
		    DeactivatePointerGrab(thisDev);
		else
   		{
		    /* Deactivating a keyboard grab should cause focus 	
			events. */
		    DoFocusEvents(thisDev->grab->window,
		    	thisDev->u.keybd.focus.win, NotifyUngrab);
 		    DeactivateKeyboardGrab(thisDev);
		}
		syncEvents.replayDev = (DeviceIntPtr)NULL;
	    }
	    break;
    }
}

int
ProcAllowEvents(client)
    register ClientPtr client;
{
    TimeStamp		time;
    DeviceIntPtr	mouse = inputInfo.pointer;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    REQUEST(xAllowEventsReq);

    REQUEST_SIZE_MATCH(xAllowEventsReq);
    time = ClientTimeToServerTime(stuff->time);
    if (CompareTimeStamps(time, currentTime) == LATER)
	return Success;
    switch (stuff->mode)
    {
	case ReplayPointer:
	    AllowSome(client, time, mouse, keybd, FROZEN_WITH_EVENT);
	    break;
	case SyncPointer: 
	    AllowSome(client, time, mouse, keybd, FROZEN_NO_EVENT);
	    break;
	case AsyncPointer: 
	    AllowSome(client, time, mouse, keybd, NOT_GRABBED);
	    break;
	case ReplayKeyboard: 
	    AllowSome(client, time, keybd, mouse, FROZEN_WITH_EVENT);
	    break;
	case SyncKeyboard: 
	    AllowSome(client, time, keybd, mouse, FROZEN_NO_EVENT);
	    break;
	case AsyncKeyboard: 
	    AllowSome(client, time, keybd, mouse, NOT_GRABBED);
	    break;
	default: 
	    client->errorValue = stuff->mode;
	    return BadValue;
    }
    return Success;
}

/* I don't see this function called from anywhere.  Should it's 
	DeactivateKeyboardGrab cause focus events? PRH. */

void
ReleaseActiveGrabs(client)
    ClientPtr client;
{
    int i;
    register DeviceIntPtr d;
    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if (d->grab && (d->grab->client == client))
	{
	    if (d == inputInfo.keyboard)
		DeactivateKeyboardGrab(d);
	    else if (d == inputInfo.pointer)
		DeactivatePointerGrab(d);
	    else
		d->grab = NullGrab;
	}
    }
}

/**************************************************************************
 *            The following procedures deal with delivering events        *
 **************************************************************************/

int
TryClientEvents (client, pEvents, count, mask, filter, grab)
    ClientPtr client;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask mask, filter;
{
    int i;

    if (debug_events) ErrorF(
	"Event([%d, %d], mask=0x%x), client=%d",
	pEvents->u.u.type, pEvents->u.u.detail, mask, client->index);
    if ((client) && (client != serverClient) && (!client->clientGone) &&
	((filter == CantBeFiltered) || (mask & filter)) &&
	((!grab) || (client == grab->client)))
    {
	for (i = 0; i < count; i++)
	    pEvents[i].u.u.sequenceNumber = client->sequence;
	WriteEventToClient(client, count, pEvents);
	if (debug_events) ErrorF(  " delivered\n");
	return 1;
    }
    else
    {
	if (debug_events) ErrorF("\n");
	return 0;
    }
}

static int
DeliverEventsToWindow(pWin, pEvents, count, filter, grab)
    WindowPtr pWin;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask filter;
{
    int     deliveries = 0;
    OtherClients *other;
    ClientPtr client = NullClient;
    Mask deliveryMask; 	/* If a grab occurs due to a button press, then
		              this mask is the mask of the grab. */

/*
 * The following relies on the fact that the Button<n>MotionMasks are equal
 * to the corresponding Button<n>Masks from the current modifier/button state.
 * If the client only selected one of the Button<n>Motion events, then she
 * should only get those.
 */

    if (pEvents->u.u.type == MotionNotify)
    {
        if (pWin->allEventMasks & PointerMotionHintMask)
	{
    	    if (lastWasMotion)
                return 0;
            else 
    	        pEvents->u.u.detail = NotifyHint;
	}
        lastWasMotion = TRUE;
	filter = Motion_Filter(keyButtonState);
    }

/* if nobody ever wants to see this event, skip some work */
    if ((filter != CantBeFiltered) && !(pWin->allEventMasks & filter))
	return 0;
    if (TryClientEvents(
	pWin->client, pEvents, count, pWin->eventMask, filter, grab))
    {
	deliveries++;
	client = pWin->client;
	deliveryMask = pWin->eventMask;
    }
    if (filter) /* CantBeFiltered means only window owner gets the event */
	for (other = OTHERCLIENTS(pWin); other; other = other->next)
	{
	    if (TryClientEvents(
		  other->client, pEvents, count, other->mask, filter, grab))
	    {
		deliveries++;
		client = other->client;
                deliveryMask = other->mask;
	    }
	}
    if ((pEvents->u.u.type == ButtonPress) && deliveries && (!grab))
    {
	ptrGrab.device = inputInfo.pointer;
	ptrGrab.client = client;
	ptrGrab.window = pWin;
	ptrGrab.ownerEvents = deliveryMask & OwnerGrabButtonMask;
	ptrGrab.eventMask =  deliveryMask;
	ptrGrab.keyboardMode = GrabModeAsync;
	ptrGrab.pointerMode = GrabModeAsync;
	ptrGrab.u.ptr.confineTo = NullWindow;
	ptrGrab.u.ptr.cursor = NullCursor;
	ActivatePointerGrab(inputInfo.pointer, &ptrGrab, currentTime, TRUE);
    }
    return deliveries;
}

/* If the event goes to dontDeliverToMe, don't send it and return 0.  if
   send works,  return 1 or if send didn't work, return 2.
*/

int
MaybeDeliverEventsToClient(pWin, pEvents, count, filter, dontDeliverToMe)
    WindowPtr pWin;
    xEvent *pEvents;
    int count;
    Mask filter;
    ClientPtr dontDeliverToMe;
{
    OtherClients * other;

    if (pWin->eventMask & filter)
    {
        if (pWin->client == dontDeliverToMe)
		return 0;
	return TryClientEvents(
	    pWin->client, pEvents, count, pWin->eventMask, filter, NullGrab);
    }
    for (other = OTHERCLIENTS(pWin); other; other = other->next)
	if (other->mask & filter)
	{
            if (other->client == dontDeliverToMe)
		return 0;
	    return TryClientEvents(
		other->client, pEvents, count, other->mask, filter, NullGrab);
	}
    return 2;
}

static WindowPtr
RootForWindow(pWin)
    WindowPtr pWin;
{
    return &WindowTable[pWin->drawable.pScreen->myNum];
}

static void
FixUpEventFromWindow(xE, pWin, child, calcChild)
    xEvent *xE;
    WindowPtr pWin;
    Window child;
    Bool calcChild;
{
    if (calcChild)
    {
        WindowPtr w=spriteTrace[spriteTraceGood-1];

	/* If the search ends up past the root should the child field be 
	 	set to none or should the value in the argument be passed 
		through. It probably doesn't matter since everyone calls 
		this function with child == None anyway. */

        while (w) 
        {
            /* If the source window is same as event window, child should be
		none.  Don't bother going all all the way back to the root. */

 	    if (w == pWin)
	    { 
   		child = None;
 		break;
	    }
	    
	    if (w->parent == pWin)
	    {
		child = w->wid;
		break;
            }
 	    w = w->parent;
        } 	    
    }
    xE->u.keyButtonPointer.root = ROOT->wid;
    xE->u.keyButtonPointer.child = child;
    xE->u.keyButtonPointer.event = pWin->wid;
    xE->u.keyButtonPointer.eventX =
	xE->u.keyButtonPointer.rootX - pWin->absCorner.x;
    xE->u.keyButtonPointer.eventY =
	xE->u.keyButtonPointer.rootY - pWin->absCorner.y;
}


int
DeliverDeviceEvents(pWin, xE, grab, stopAt)
    register WindowPtr pWin, stopAt;
    register xEvent *xE;
    GrabPtr grab;
{
    Mask filter;
    int     deliveries;
    Window child = None;

    filter = filters[xE->u.u.type];
    if ((filter != CantBeFiltered) && !(filter & pWin->deliverableEvents))
	return 0;
    while (pWin)
    {
	FixUpEventFromWindow(xE, pWin, child, FALSE);
	deliveries = DeliverEventsToWindow(pWin, xE, 1, filter, grab);
	if ((deliveries > 0) || (filter & pWin->dontPropagateMask))
	    return deliveries;
	if (pWin == stopAt)
	    return 0;
	child = pWin->wid;
	pWin = pWin->parent;
    }
/*
 * This point should never be reached. Either stopAt is NullWindow, in which
 * case, the procedure is exited from the middle of the loop above, or it is
 * a window that the caller is asserting is an ancestor of pWin.
 */
    return 0;
}

int
DeliverEvents(pWin, xE, count, otherParent)
/* not useful for events that propagate up the tree */
    register WindowPtr pWin, otherParent;
    register xEvent *xE;
    int count;
{
    Mask filter;
    int     deliveries;

    if (!count)
	return 0;
    filter = filters[xE->u.u.type];
    if ((filter & SubstructureNotifyMask) && (xE->u.u.type != CreateNotify))
	xE->u.destroyNotify.event = pWin->wid;
    if (filter != StructureAndSubMask)
	return DeliverEventsToWindow(pWin, xE, count, filter, NullGrab);
    deliveries = DeliverEventsToWindow(
	    pWin, xE, count, StructureNotifyMask, NullGrab);
    if (pWin->parent)
    {
	xE->u.destroyNotify.event = pWin->parent->wid;
	deliveries += DeliverEventsToWindow(
		pWin->parent, xE, count, SubstructureNotifyMask, NullGrab);
	if (xE->u.u.type == ReparentNotify)
	{
	    xE->u.destroyNotify.event = otherParent->wid;
	    deliveries += DeliverEventsToWindow(
		    otherParent, xE, count, SubstructureNotifyMask, NullGrab);
	}
    }
    return deliveries;
}

/* check root -- this fails in Zaphod mode XXX */
/* 
 * XYToWindow is only called by CheckMotion after it has determined that
 * the current cache is not accurate.
 */
static WindowPtr 
XYToWindow(x, y)
	int x, y;
{
    register WindowPtr  pWin;

    spriteTraceGood = 1;	/* root window still there */
    pWin = ROOT->firstChild;
    while (pWin)
    {
	if ((pWin->mapped) &&
		(x >= pWin->absCorner.x - pWin->borderWidth) &&
		(x < pWin->absCorner.x + pWin->clientWinSize.width +
		    pWin->borderWidth) &&
		(y >= pWin->absCorner.y - pWin->borderWidth) &&
		(y < pWin->absCorner.y + pWin->clientWinSize.height +
		    pWin->borderWidth))
	{
	    if (spriteTraceGood >= spriteTraceSize)
	    {
		spriteTraceSize += 10;
		spriteTrace = (WindowPtr *)Xrealloc(
		    spriteTrace, spriteTraceSize*sizeof(WindowPtr));
	    }
	    spriteTrace[spriteTraceGood] = pWin;
	    pWin = spriteTrace[spriteTraceGood++]->firstChild;
	}
	else
	    pWin = pWin->nextSib;
    }
    return spriteTrace[spriteTraceGood-1];
}

static WindowPtr 
CheckMotion(x, y, ignoreCache)
    int x, y;
    Bool ignoreCache;
{
    WindowPtr prevSpriteWin = sprite.win;

    if ((x != sprite.hot.x) || (y != sprite.hot.y))
    {
	sprite.win = XYToWindow(x, y);
	sprite.hot.x = x;
	sprite.hot.y = y;
/* XXX Do PointerNonInterestBox here */
/*
	if (!(sprite.win->deliverableEvents & Motion_Filter(keyButtonState)))
        {
	    
	}
*/
    }
    else
    {
	if ((ignoreCache) || (!sprite.win))
	    sprite.win = XYToWindow(x, y);
    }
    if (sprite.win != prevSpriteWin)
    {
	if (prevSpriteWin != NullWindow)
	    DoEnterLeaveEvents(prevSpriteWin, sprite.win, NotifyNormal);
	lastWasMotion = FALSE;
	PostNewCursor();
        return NullWindow;
    }
    return sprite.win;
}

WindowsRestructured()
{
    (void) CheckMotion(sprite.hot.x, sprite.hot.y, TRUE);
}

void
DefineInitialRootWindow(win)
    WindowPtr win;
{
    register CursorPtr c = win->cursor;

    sprite.hot.x = currentScreen->width / 2;
    sprite.hot.y = currentScreen->height / 2;
    sprite.win = win;
    sprite.current = c;
    ROOT = win;
    (*currentScreen->CursorLimits) (
	currentScreen, win->cursor, &sprite.hotLimits, &sprite.physLimits);
    (*currentScreen->SetCursorPosition) (
	currentScreen, sprite.hot.x, sprite.hot.y);
    (*currentScreen->ConstrainCursor) (
	currentScreen, &sprite.physLimits);
    (*currentScreen->DisplayCursor) (
	currentScreen, c, sprite.hot.x, sprite.hot.y);
}

/*
 * This does not take any shortcuts, and even ignores its argument, since
 * it does not happen very often, and one has to walk up the tree since
 * this might be a newly instantiated cursor for an intermediate window
 * between the one the pointer is in and the one that the last cursor was
 * instantiated from.
 */
void
WindowHasNewCursor(pWin)
    WindowPtr pWin;
{
    PostNewCursor();
}

void
NewCurrentScreen(newScreen, x, y)
    ScreenPtr newScreen;
    int x,y;
{
    if (newScreen == currentScreen)
        return;
    ROOT = &WindowTable[newScreen->myNum];
    currentScreen = newScreen;
    (void) CheckMotion(x, y, TRUE);
}

int
ProcWarpPointer(client)
    ClientPtr client;
{
    WindowPtr  source, dest;
    xEvent MotionEvent;		/* Event that is sent to ProcessPointerEvent
					saying the pointer moved. */
    REQUEST(xWarpPointerReq);

    REQUEST_SIZE_MATCH(xWarpPointerReq);
    dest = LookupWindow(stuff->dstWid, client);
    if (!dest)
    {
	client->errorValue = stuff->dstWid;
	return BadWindow;
    }
    if (stuff->srcWid != None)
    {
	int     winX, winY;
        source = LookupWindow(stuff->srcWid, client);
	if (!source)
	{
	    client->errorValue = stuff->srcWid;
	    return BadWindow;
	}
	winX = source->absCorner.x;
	winY = source->absCorner.y;
	if (
		(sprite.hot.x < (winX + stuff->srcX)) ||
		(sprite.hot.y < (winY + stuff->srcY)) ||
		((stuff->srcWidth != 0) &&
		    (winX + stuff->srcX + stuff->srcWidth < sprite.hot.x)) ||
		((stuff->srcHeight != 0) &&
		    (winY + stuff->srcY + stuff->srcHeight < sprite.hot.y)) ||
		(!PointInWindowIsVisible(source, sprite.hot.x, sprite.hot.y)))
	    return Success;
    }
    if (currentScreen != dest->drawable.pScreen)
        NewCurrentScreen(dest->drawable.pScreen, 
	    	         dest->absCorner.x + stuff->dstX,
		         dest->absCorner.y + stuff->dstY);

    /* Send a pointer motion event to ProcessPointerEvent, just as the
	device dependent driver does when the hardware moves the pointer. */

    (*dest->drawable.pScreen->SetCursorPosition)( dest->drawable.pScreen,
            dest->absCorner.x + stuff->dstX, dest->absCorner.y + stuff->dstY);
 
    MotionEvent.u.keyButtonPointer.rootX = dest->absCorner.x + stuff->dstX;
    MotionEvent.u.keyButtonPointer.rootY = dest->absCorner.y + stuff->dstY;
    MotionEvent.u.keyButtonPointer.time = currentTime.milliseconds;
    MotionEvent.u.u.type = MotionNotify;

    ProcessPointerEvent(&MotionEvent, inputInfo.pointer);

    return Success;
}

static Bool
IsGrabbed(key, modifiers, dev, grab, keybd)
    int key, modifiers;
    DeviceIntPtr dev;
    register GrabPtr grab;
    Bool keybd;
{
    if (grab->device != dev)
	return FALSE;
    if ((grab->modifiers != AnyModifier) && (grab->modifiers != modifiers))
	return FALSE;
    if (keybd)
    {
	if ((key != grab->u.keybd.key) &&
	    (grab->u.keybd.key != AnyKey))
	    return FALSE;
    }
    else
    {
	if ((key != grab->u.ptr.button) && (grab->u.ptr.button != AnyButton))
	    return FALSE;
    }
    return TRUE;
}

static Bool
MatchingGrab(key, modifiers, dev, grab, keybd)
    int key, modifiers;
    DeviceIntPtr dev;
    register GrabPtr grab;
    Bool keybd;
{
    if (grab->device != dev)
	return FALSE;
    if ((grab->modifiers != modifiers) &&
	(grab->modifiers != AnyModifier) &&
	(modifiers != AnyModifier))
	return FALSE;
    if (keybd)
    {
	if ((key != grab->u.keybd.key) &&
	    (grab->u.keybd.key != AnyKey) &&
	    (key != AnyKey))
	    return FALSE;
    }
    else
    {
	if ((key != grab->u.ptr.button) &&
	    (grab->u.ptr.button != AnyButton) &&
	    (key != AnyButton))
	    return FALSE;
    }
    return TRUE;
}

static void
NoticeTimeAndState(xE)
    register xEvent *xE;
{
    if (xE->u.keyButtonPointer.time < currentTime.milliseconds)
	currentTime.months++;
    currentTime.milliseconds = xE->u.keyButtonPointer.time;
    xE->u.keyButtonPointer.pad1 = 0;
    xE->u.keyButtonPointer.state = keyButtonState;
    xE->u.keyButtonPointer.sameScreen = TRUE;		/* XXX */
}

/* "CheckPassiveGrabsOnWindow" checks to see if the event passed in causes a
	passive grab set on the window to be activated. */

static Bool
CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard)
    WindowPtr pWin;
    register DeviceIntPtr device;
    register xEvent *xE;
    int isKeyboard;
{
    GrabPtr grab;

    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (IsGrabbed(xE->u.u.detail, keyButtonState & AllModifiersMask, device, grab,
	    isKeyboard))
	{
	    if (isKeyboard)
		ActivateKeyboardGrab(device, grab, currentTime, TRUE);
	    else
		ActivatePointerGrab(device, grab, currentTime, TRUE);
 
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);

	    TryClientEvents(grab->client, xE, 1, grab->eventMask, 
		filters[xE->u.u.type],  grab);

	    if (device->sync.state == FROZEN_NO_EVENT)
	    {
	    	device->sync.event = *xE;
	    	device->sync.state = FROZEN_WITH_EVENT;
            }	

	    return TRUE;
	}
    }

    return FALSE;
}

/* "CheckDeviceGrabs" handles both keyboard and pointer events that may cause a 
passive grab to be activated.  If the event is a keyboard event, the
ancestors of the focus window are traced down and tried to see if they have
any passive grabs to be activated.  If the focus window itself is reached and it's
descendants contain they pointer, the ancestors of the window that the pointer
is in are then traced down starting at the focus window, otherwise no grabs
are activated.  If the event is a pointer event, the ancestors of the window
that the pointer is in are traced down starting at the root until CheckPassiveGrabs
causes a passive grab to activate or all the windows are tried. PRH*/

static Bool
CheckDeviceGrabs(device, xE, checkFirst, isKeyboard)
    register DeviceIntPtr device;
    register xEvent *xE;
    int checkFirst;
{
    int i;
    WindowPtr pWin;

    i = checkFirst;

    if (isKeyboard)
    {
	for (; i < focusTraceGood; i++)
	{
	    pWin = focusTrace[i];
	    if (CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard))
		return TRUE;
	}
  
        if (pWin != spriteTrace[i-1])
	    return FALSE;
    }
        
    for (; i < spriteTraceGood; i++)
    {
	pWin = spriteTrace[i];
	if (CheckPassiveGrabsOnWindow(pWin, device, xE, isKeyboard))
	    return TRUE;
    }

    return FALSE;
}

static void
NormalKeyboardEvent(keybd, xE, window)
    xEvent *xE;
    DeviceIntPtr keybd;
    WindowPtr window;
{
    WindowPtr focus = keybd->u.keybd.focus.win;
    if (focus == NullWindow)
	return;
    if (focus == PointerRootWin)
    {
	DeliverDeviceEvents(window, xE, NullGrab, NullWindow);
	return;
    }
    if ((focus == window) || IsParent(focus, window))
    {
	if (DeliverDeviceEvents(window, xE, NullGrab, focus))
	    return;
    }
 /* just deliver it to the focus window */
    FixUpEventFromWindow(xE, focus, None, FALSE);
    DeliverEventsToWindow(focus, xE, 1, filters[xE->u.u.type], NullGrab);
}

void
ProcessKeyboardEvent (xE, keybd)
    register xEvent *xE;
    register DeviceIntPtr keybd;
{
    register int    key;
    GrabPtr         grab = keybd->grab;
    WindowPtr       pWin;
    Bool            deactiveGrab = FALSE;

    if (keybd->sync.frozen)
    {
	EnqueueEvent(keybd, xE);
	return;
    }
    NoticeTimeAndState(xE);
    key = xE->u.u.detail;
    pWin = sprite.win;
/*
    CheckMotion(
	xE->u.keyButtonPointer.rootX, xE->u.keyButtonPointer.rootY,
	FALSE);
    xE->u.keyButtonPointer.rootX = sprite.hot.x;  ignore x and y for keyboard?
    xE->u.keyButtonPointer.rootY = sprite.hot.y;
*/
    xE->u.u.detail = key;           
    lastWasMotion  = FALSE;
    switch (xE->u.u.type)
    {
	case KeyPress: 
	    BitOn(keybd->down, key);
	    if (!xE->u.u.detail)
		return;
	    BitOn(keybd->down, xE->u.u.detail);
	    keyButtonState |= stateMasks[xE->u.u.detail];
	    if (!grab)
		if (CheckDeviceGrabs(keybd, xE, 0, TRUE))
		    return;
	    break;
	case KeyRelease: 
	    BitOff(keybd->down, key);
	    if (!xE->u.u.detail)
		return;
	    BitOff(keybd->down, xE->u.u.detail);
	    keyButtonState &= ~stateMasks[xE->u.u.detail];
	    if ((keybd->u.keybd.passiveGrab) &&
			(xE->u.u.detail == grab->u.keybd.key))
		deactiveGrab = TRUE;
	    break;
	default: 
	    FatalError("Impossible keyboard event");
    }
    if (grab = keybd->grab)
    {
	Bool syncIt;
	if ((!grab->ownerEvents) ||
		(!(syncIt = DeliverDeviceEvents(pWin, xE, grab, NullWindow))))
	{
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);
	    syncIt = TryClientEvents(
		grab->client, xE, 1, grab->eventMask, 
		filters[xE->u.u.type], grab);
	}
	if (syncIt && (keybd->sync.state == FREEZE_NEXT_EVENT))
	{
	    keybd->sync.state = FROZEN_WITH_EVENT;
	    keybd->sync.frozen = TRUE;
	    keybd->sync.event = *xE;
	}
    }
    else
	NormalKeyboardEvent(keybd, xE, pWin);
    if (deactiveGrab)
        DeactivateKeyboardGrab(keybd);
}

void
ProcessPointerEvent (xE, mouse)
    register xEvent 		*xE;
    register DeviceIntPtr 	mouse;
{
    Mask    		filterToUse;
    register int    	key;
    register GrabPtr	grab = mouse->grab;
    WindowPtr		pWin;
    Bool		moveIt = FALSE;
    Bool                deactivateGrab = FALSE;

    if (xE->u.keyButtonPointer.rootX < sprite.physLimits.x1)
    {
	xE->u.keyButtonPointer.rootX = sprite.physLimits.x1;
	moveIt = TRUE;
    }
    else if (xE->u.keyButtonPointer.rootX >= sprite.physLimits.x2)
    {
	xE->u.keyButtonPointer.rootX = sprite.physLimits.x2 - 1;
	moveIt = TRUE;
    }
    if (xE->u.keyButtonPointer.rootY < sprite.physLimits.y1)
    {
	xE->u.keyButtonPointer.rootY = sprite.physLimits.y1;
	moveIt = TRUE;
    }
    else if (xE->u.keyButtonPointer.rootY >= sprite.physLimits.y2)
    {
	xE->u.keyButtonPointer.rootY = sprite.physLimits.y2 - 1;
	moveIt = TRUE;
    }
    if (moveIt)
	(*currentScreen->SetCursorPosition)(
	    currentScreen, xE->u.keyButtonPointer.rootX,
	    xE->u.keyButtonPointer.rootY);
    if (mouse->sync.frozen)
    {
	EnqueueEvent(mouse, xE);
	return;
    }
    NoticeTimeAndState(xE);
    key = xE->u.u.detail;
    pWin = sprite.win;
    switch (xE->u.u.type)
    {
	case ButtonPress: 
	    lastWasMotion = FALSE;
	    buttonsDown++;
	    filterToUse = filters[ButtonPress];
	    xE->u.u.detail = mouse->u.ptr.map[key];
	    if (xE->u.u.detail <= 5)
		keyButtonState |= stateMasks[xE->u.u.detail];
	    if (!grab)
		if (CheckDeviceGrabs(mouse, xE, 0, FALSE))
		    return;
	    break;
	case ButtonRelease: 
            lastWasMotion = FALSE;
	    buttonsDown--;
	    filterToUse = filters[ButtonRelease];
	    xE->u.u.detail = mouse->u.ptr.map[key];
	    if (xE->u.u.detail <= 5)
		keyButtonState &= ~stateMasks[xE->u.u.detail];
	    if ((!(keyButtonState & AllButtonsMask)) &&
		(mouse->u.ptr.autoReleaseGrab))
		deactivateGrab = TRUE;
	    break;
	case MotionNotify: 
            pWin = CheckMotion(
	        xE->u.keyButtonPointer.rootX, xE->u.keyButtonPointer.rootY, 
		FALSE);
            if (!pWin)
                return ;
	    filterToUse = Motion_Filter(keyButtonState);
	    break;
	default: 
	    FatalError("bogus pointer event from ddx");
    }
    buttonMotionMask = (buttonsDown) ? ButtonMotionMask : 0;
    if (grab)
    {
	Bool syncIt;
	if ((!grab->ownerEvents) ||
		(!(syncIt = DeliverDeviceEvents(pWin, xE, grab, NullWindow))))
	{
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);
	    syncIt = TryClientEvents(
		grab->client, xE, 1, grab->eventMask, filterToUse, grab);
	}
	if (syncIt && (mouse->sync.state == FREEZE_NEXT_EVENT))
	{
	    mouse->sync.state = FROZEN_WITH_EVENT;
	    mouse->sync.frozen = TRUE;
	    mouse->sync.event = *xE;
	}
    }
    else
	DeliverDeviceEvents(pWin, xE, NullGrab, NullWindow);
    if (deactivateGrab)
        DeactivatePointerGrab(mouse);
}

void
ProcessOtherEvent (xE, pDevice)
    xEvent *xE;
    DevicePtr pDevice;
{
/*	XXX What should be done here ?
    Bool propogate = filters[xE->type];
*/
}

#define AtMostOneClient \
	(SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask)

void
RecalculateDeliverableEvents(pWin)
    WindowPtr pWin;
{
    OtherClients * others;
    WindowPtr child;
    Mask hintMask;

    pWin->allEventMasks = pWin->eventMask;
    hintMask = pWin->eventMask & PointerMotionHintMask;
    for (others = OTHERCLIENTS(pWin); others; others = others->next)
    {
	pWin->allEventMasks |= others->mask;
/*	hintMask &= (others->mask & PointerMotionHintMask); */
    }
/*
    if (!hintMask)
        pWin->allEventMasks &= ~PointerMotionHintMask;
*/
    if (pWin->parent)
	pWin->deliverableEvents = pWin->allEventMasks |
	    (pWin->parent->deliverableEvents & ~pWin->dontPropagateMask &
	     PropagateMask);
    else
	pWin->deliverableEvents = pWin->allEventMasks;
    for (child = pWin->firstChild; child; child = child->nextSib)
	RecalculateDeliverableEvents(child);
}

static void
OtherClientGone(pWin, id)
    WindowPtr pWin;
    long   id;
{
    register OtherClientsPtr *next;
    register OtherClientsPtr other;

    for (next = (OtherClientsPtr *)&(pWin->otherClients);
	 *next; next = &((*next)->next))
    {
	if ((other = *next)->resource == id)
	{
	    *next = other->next;
	    Xfree(other);
	    RecalculateDeliverableEvents(pWin);
	    return;
	}
    }
    FatalError("client not on event list");
}

static void
PassiveClientGone(pWin, id)
    WindowPtr pWin;
    long   id;
{
    register GrabPtr *next;
    register GrabPtr grab;

    for (next = (GrabPtr *)&(pWin->passiveGrabs);
	 *next; next = &((*next)->next))
    {
	if ((grab = *next)->resource == id)
	{
	    *next = grab->next;
	    Xfree(grab);
	    return;
	}
    }
    FatalError("client not on passive grab list");
}

int
EventSelectForWindow(pWin, client, mask)
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
{
    Mask check;
    OtherClients * others;

    check = (mask & AtMostOneClient);
    if (check & pWin->allEventMasks)
    {				       /* It is illegal for two different
				          clients to select on any of the
				          events for AtMostOneClient. However,
				          it is OK, for some client to
				          continue selecting on one of those
				          events.  */
	if ((pWin->client != client) && (check & pWin->eventMask))
	    return BadAccess;
	for (others = OTHERCLIENTS(pWin); others; others = others->next)
	{
	    if ((others->client != client) && (check & others->mask))
		return BadAccess;
	}
    }
    if (pWin->client == client)
	pWin->eventMask = mask;
    else
    {
	for (others = OTHERCLIENTS(pWin); others; others = others->next)
	{
	    if (others->client == client)
	    {
		if (mask == 0)
		{
		    FreeResource(others->resource, RC_NONE);
		    return Success;
		}
		else
		    others->mask = mask;
		goto maskSet;
	    }
	}
	others = (OtherClients *) Xalloc(sizeof(OtherClients));
	others->client = client;
	others->mask = mask;
	others->resource = FakeClientID(client->index);
	others->next = OTHERCLIENTS(pWin);
	pWin->otherClients = (pointer)others;
	AddResource(others->resource, RT_FAKE, pWin, OtherClientGone, RC_CORE);
    }
maskSet: 
    RecalculateDeliverableEvents(pWin);
    return Success;
}

int
EventSuppressForWindow(pWin, client, mask)
	WindowPtr pWin;
	ClientPtr client;
	Mask mask;
{
    pWin->dontPropagateMask = mask;
    RecalculateDeliverableEvents(pWin);
    return Success;
}


/* returns true if b is a descendent of a */
static Bool
IsParent(a, b)
    register WindowPtr a, b;
{
    for (b = b->parent; b; b = b->parent)
	if (b == a) return TRUE;
    return FALSE;
}

static WindowPtr 
CommonAncestor(a, b)
    register WindowPtr a, b;
{
    for (b = b->parent; b; b = b->parent)
	if (IsParent(b, a)) return b;
    return NullWindow;
}

static void
EnterLeaveEvent(type, mode, detail, pWin)
    int type, mode, detail;
    WindowPtr pWin;
{
    xEvent		event;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    WindowPtr		focus = keybd->u.keybd.focus.win;

    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.enterLeave.time = currentTime.milliseconds;
    event.u.enterLeave.rootX = sprite.hot.x;
    event.u.enterLeave.rootY = sprite.hot.y;
    FixUpEventFromWindow(&event, pWin, None, TRUE);		
 /* This call counts on same initial structure beween enter & button events */
    event.u.enterLeave.state = keyButtonState;
    event.u.enterLeave.mode = mode;
    event.u.enterLeave.flags = ELFlagSameScreen;	/* XXX */
    if ((focus != NoneWin) &&
	((pWin == focus) || (focus == PointerRootWin) ||
	 IsParent(focus, pWin)))
	event.u.enterLeave.flags |= ELFlagFocus;
    DeliverEventsToWindow(pWin, &event, 1, filters[type], NullGrab);
    if (type == EnterNotify)
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	bcopy(&keybd->down[1], &ke.map[0], 31);
    }
}

static void
EnterNotifies(ancestor, child, mode, detail)
    WindowPtr ancestor, child;
    int mode, detail;
{
    if (!child || (ancestor == child))
	return;
    EnterNotifies(ancestor, child->parent, mode, detail);
    EnterLeaveEvent(EnterNotify, mode, detail, child);
}

/* dies horribly if ancestor is not an ancestor of child */
static void
LeaveNotifies(child, ancestor, mode, detail, doAncestor)
    WindowPtr child, ancestor;
    int detail, mode;
{
    register WindowPtr  pWin;

    if (ancestor == child)
	return;
    for (pWin = child->parent; pWin != ancestor; pWin = pWin->parent)
	EnterLeaveEvent(LeaveNotify, mode, detail, pWin);
    if (doAncestor)
	EnterLeaveEvent(LeaveNotify, mode, detail, ancestor);
}

static void
DoEnterLeaveEvents(fromWin, toWin, mode)
    WindowPtr fromWin, toWin;
    int mode;
{
    if (fromWin == toWin)
	return;
    if (IsParent(fromWin, toWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyInferior, fromWin);
	EnterNotifies(fromWin, toWin->parent, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyAncestor, toWin);
    }
    else if (IsParent(toWin, fromWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyAncestor, fromWin);
	LeaveNotifies(fromWin, toWin, mode, NotifyVirtual, FALSE);
	EnterLeaveEvent(EnterNotify, mode, NotifyInferior, toWin);
    }
    else
    { /* neither fromWin nor toWin is descendent of the other */
	WindowPtr common = CommonAncestor(toWin, fromWin);
	/* common == NullWindow ==> different screens */
	EnterLeaveEvent(LeaveNotify, mode, NotifyNonlinear, fromWin);
	if (common)
	{
	    LeaveNotifies(
		fromWin, common, mode, NotifyNonlinearVirtual, FALSE);
	    EnterNotifies(common, toWin->parent, mode, NotifyNonlinearVirtual);
	}
	else
	{
	    LeaveNotifies(
		fromWin, RootForWindow(fromWin), mode,
		NotifyNonlinearVirtual, TRUE);
	    EnterNotifies(
		RootForWindow(toWin), toWin->parent, mode, NotifyNonlinearVirtual);
	}
	EnterLeaveEvent(EnterNotify, mode, NotifyNonlinear, toWin);
    }
}

static void
FocusEvent(type, mode, detail, pWin)
    int type, mode, detail;
    WindowPtr pWin;
{
   xEvent	event;
   DeviceIntPtr	keybd = inputInfo.keyboard;

    event.u.focus.mode = mode;
    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.focus.window = pWin->wid;
    DeliverEventsToWindow(pWin, &event, 1, filters[type], NullGrab);
    if (type == FocusIn)
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	bcopy(keybd->down, &ke.map[0], 31);
	DeliverEventsToWindow(pWin, &event, 1, KeymapStateMask, NullGrab);
    }
}

 /*
  * recursive because it is easier
  * no-op if child not descended from ancestor
  */
static Bool
FocusInEvents(ancestor, child, skipChild, mode, detail, doAncestor)
    WindowPtr ancestor, child, skipChild;
    int mode, detail;
    Bool doAncestor;
{
    if (child == NullWindow)
	return FALSE;
    if (ancestor == child)
    {
	if (doAncestor)
	    FocusEvent(FocusIn, mode, detail, child);
	return TRUE;
    }
    if (FocusInEvents(
	ancestor, child->parent, skipChild, mode, detail, doAncestor))
    {
	if (child != skipChild)
	    FocusEvent(FocusIn, mode, detail, child);
	return TRUE;
    }
    return FALSE;
}

/* dies horribly if ancestor is not an ancestor of child */
static void
FocusOutEvents(child, ancestor, mode, detail, doAncestor)
    WindowPtr child, ancestor;
    int detail;
    Bool doAncestor;
{
    register WindowPtr  pWin;

    for (pWin = child; pWin != ancestor; pWin = pWin->parent)
	FocusEvent(FocusOut, mode, detail, pWin);
    if (doAncestor)
	FocusEvent(FocusOut, mode, detail, ancestor);
}

static void
DoFocusEvents(fromWin, toWin, mode)
    WindowPtr fromWin, toWin;
    int mode;
{
    int     out, in;		       /* for holding details for to/from
				          PointerRoot/None */

    out = (fromWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
    in = (toWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
 /* wrong values if neither, but then not referenced */

    if ((toWin == NullWindow) || (toWin == PointerRootWin))
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
   	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE);
	    FocusEvent(FocusOut, mode, out, ROOT);
	}
	else
	{
	    if (IsParent(fromWin, sprite.win))
	      FocusOutEvents(sprite.win, fromWin, mode, NotifyPointer, FALSE);
	    FocusEvent(FocusOut, mode, NotifyNonlinear, fromWin);
	    FocusOutEvents(
	      fromWin->parent, ROOT, mode, NotifyNonlinearVirtual, TRUE);
	}
	FocusEvent(FocusIn, mode, in, ROOT);
	if (toWin == PointerRootWin)
	    FocusInEvents(
		ROOT, sprite.win, NullWindow, mode, NotifyPointer, TRUE);
    }
    else
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(sprite.win, ROOT, mode, NotifyPointer, TRUE);
	    FocusEvent(FocusOut, mode, out, ROOT);
	    FocusInEvents(
		ROOT, toWin, toWin, mode, NotifyNonlinearVirtual, TRUE);
	    FocusEvent(FocusIn, mode, NotifyNonlinear, toWin);
	    if (IsParent(toWin, sprite.win))
    	       FocusInEvents(
		 toWin, sprite.win, NullWindow, mode, NotifyPointer, FALSE);
	}
	else
	{
	    if (IsParent(toWin, fromWin))
	    {
		FocusEvent(FocusOut, mode, NotifyAncestor, fromWin);
		FocusOutEvents(
		    fromWin->parent, toWin, mode, NotifyVirtual, FALSE);
		FocusEvent(FocusIn, mode, NotifyInferior, toWin);
		if ((IsParent(toWin, sprite.win)) &&
			(sprite.win != fromWin) &&
			(!IsParent(fromWin, sprite.win)) &&
			(!IsParent(sprite.win, fromWin)))
		    FocusInEvents(
			toWin, sprite.win, NullWindow, mode,
			NotifyPointer, FALSE);
	    }
	    else
		if (IsParent(fromWin, toWin))
		{
		    if ((IsParent(fromWin, sprite.win)) &&
			    (sprite.win != fromWin) &&
			    (!IsParent(toWin, sprite.win)) &&
			    (!IsParent(sprite.win, toWin)))
			FocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE);
		    FocusEvent(FocusOut, mode, NotifyInferior, fromWin);
		    FocusInEvents(
			fromWin, toWin, toWin, mode, NotifyVirtual, FALSE);
		    FocusEvent(FocusIn, mode, NotifyAncestor, toWin);
		}
		else
		{
		/* neither fromWin or toWin is child of other */
		    WindowPtr common = CommonAncestor(toWin, fromWin);
		/* common == NullWindow ==> different screens XXX */
		    if (IsParent(fromWin, sprite.win))
			FocusOutEvents(
			    sprite.win, fromWin, mode, NotifyPointer, FALSE);
		    FocusEvent(FocusOut, mode, NotifyNonlinear, fromWin);
		    FocusOutEvents(
			fromWin->parent, common, mode, NotifyNonlinearVirtual,
			FALSE);
		    FocusInEvents(
			common, toWin, toWin, mode, NotifyNonlinearVirtual,
			FALSE);
		    FocusEvent(FocusIn, mode, NotifyNonlinear, toWin);
		    if (IsParent(toWin, sprite.win))
			FocusInEvents(
			    toWin, sprite.win, NullWindow, mode,
			    NotifyPointer, FALSE);
		}
	}
    }
}

/* XXX SetInputFocus does not enumerate all roots, handle screen crossings */
int
ProcSetInputFocus(client)
    ClientPtr client;
{
    TimeStamp			time;
    WindowPtr			focusWin;
    int				mode;
    register DeviceIntPtr	kbd = inputInfo.keyboard;
    register FocusPtr		focus = &kbd->u.keybd.focus;
    REQUEST(xSetInputFocusReq);

    REQUEST_SIZE_MATCH(xSetInputFocusReq);
    if ((stuff->revertTo != RevertToParent) &&
	    (stuff->revertTo != RevertToPointerRoot) &&
	    (stuff->revertTo != RevertToNone))
    {
	client->errorValue = stuff->revertTo;
	return BadValue;
    }
    time = ClientTimeToServerTime(stuff->time);
    if ((stuff->focus == None) || (stuff->focus == PointerRoot))
	focusWin = (WindowPtr)(stuff->focus);
    else if (!(focusWin = LookupWindow(stuff->focus, client)))
    {
	client->errorValue = stuff->focus;
	return BadWindow;
    }
    else
    {
 	/* It is a match error to try to set the input focus to an 
	unviewable window. */

	if(!focusWin->realized)
	    return(BadMatch);
    }

    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	    (CompareTimeStamps(time, focus->time) == EARLIER))
	return Success;

    mode = (kbd->grab) ? NotifyWhileGrabbed : NotifyNormal;

    /* If the client attempts to set the input focus to the window that already
	is the focus window and the mode of the focus events that would be
	reported are the same as the the mode of the focus events caused by
	the last ProcSetInputFocus, then this routine does nothing.  It's
	unclear what the protocol document intends when the input focus is set
	to the same window, but at least the number of events reported is kept
	to a mininum. PRH */

    if ((focusWin == focus->win) && (mode == lastInputFocusChangeMode))
	return Success;

    lastInputFocusChangeMode = mode;

    DoFocusEvents(focus->win, focusWin, mode);
    focus->time = time;
    focus->revert = stuff->revertTo;
    focus->win = focusWin;
    if (focusWin == NoneWin)
        focusTraceGood = 0;
    else if (focusWin == PointerRootWin)
    {
        focusTraceGood = 1;
        focusTrace[0] = ROOT;
    }
    else
    {
        int depth=0;
        WindowPtr pWin;
        for (pWin = focusWin; pWin; pWin = pWin->parent) depth++;
        if (depth > focusTraceSize)
        {
	    focusTraceSize = depth+1;
	    focusTrace = (WindowPtr *)Xrealloc(
		    focusTrace, focusTraceSize*sizeof(WindowPtr));
	}

 	focusTraceGood = depth;

        for (pWin = focusWin; pWin; pWin = pWin->parent, depth--) 
	    focusTrace[depth-1] = pWin;
    }
    return Success;
}

int
ProcGetInputFocus(client)
    ClientPtr client;
{
    xGetInputFocusReply rep;
    REQUEST(xReq);
    FocusPtr focus = &(inputInfo.keyboard->u.keybd.focus);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (focus->win == NoneWin)
	rep.focus = None;
    else if (focus->win == PointerRootWin)
	rep.focus = PointerRoot;
    else rep.focus = focus->win->wid;
    rep.revertTo = focus->revert;
    WriteReplyToClient(client, sizeof(xGetInputFocusReply), &rep);
    return Success;
}

int
ProcGrabPointer(client)
    ClientPtr client;
{
    xGrabPointerReply rep;
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab = device->grab;
    WindowPtr pWin, confineTo;
    CursorPtr cursor;
    REQUEST(xGrabPointerReq);
    TimeStamp time;

    REQUEST_SIZE_MATCH(xGrabPointerReq);
    if ((stuff->pointerMode != GrabModeSync) && 
	(stuff->pointerMode != GrabModeAsync) && 
	(stuff->keyboardMode != GrabModeSync) && 
	(stuff->keyboardMode != GrabModeAsync))
        return BadValue;

    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    if (stuff->confineTo == None)
	confineTo = NullWindow;
    else
    {
	confineTo = LookupWindow(stuff->grabWindow, client);
	if (!confineTo)
	{
	    client->errorValue = stuff->grabWindow;
	    return BadWindow;
	}
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	cursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!cursor)
	    return BadCursor;
    }
	/* at this point, some sort of reply is guaranteed. */
    time = ClientTimeToServerTime(stuff->time);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    if ((grab) && (grab->client != client))
	rep.status = AlreadyGrabbed;
    else if (!pWin->realized)
	rep.status = GrabNotViewable;
    else if (device->sync.frozen &&
	     ((device->sync.other && (device->sync.other->client != client)) ||
	     ((device->sync.state >= FROZEN) &&
	      (device->grab->client != client))))
	rep.status = GrabFrozen;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (device->grab &&
	     (CompareTimeStamps(time, device->grabTime) == EARLIER)))
	rep.status = GrabInvalidTime;
    else
    {
	ptrGrab.u.ptr.cursor = cursor;
	ptrGrab.client = client;
	ptrGrab.ownerEvents = stuff->ownerEvents;
	ptrGrab.eventMask = stuff->eventMask;
	ptrGrab.u.ptr.confineTo = confineTo;
	ptrGrab.window = pWin;
	ptrGrab.keyboardMode = stuff->keyboardMode;
	ptrGrab.pointerMode = stuff->pointerMode;
	ptrGrab.device = inputInfo.pointer;
	ActivatePointerGrab(inputInfo.pointer, &ptrGrab, time, FALSE);
	rep.status = GrabSuccess;
    }
    WriteReplyToClient(client, sizeof(xGrabPointerReply), &rep);
    if (cursor)
	ChangeToCursor(cursor);
    return Success;
}

int
ProcChangeActivePointerGrab(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.pointer;
    register GrabPtr grab = device->grab;
    CursorPtr newCursor;
    REQUEST(xChangeActivePointerGrabReq);
    TimeStamp time;

    REQUEST_SIZE_MATCH(xChangeActivePointerGrabReq);
    if (!grab)
	return Success;
    if (grab->client != client)
	return BadAccess;
    if (stuff->cursor == None)
	grab->u.ptr.cursor = NullCursor;
    else
    {
	newCursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!newCursor)
	    return BadCursor;
    }
    time = ClientTimeToServerTime(stuff->time);
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
	return Success;
    grab->u.ptr.cursor = newCursor;
    PostNewCursor();
    /* if mouse motion is newly turned on, it should probably send a motion
       event */
    grab->eventMask = stuff->eventMask;
    return Success;
}

int
ProcUngrabPointer(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab = device->grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	    (CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	    (grab) && (grab->client == client))
	DeactivatePointerGrab(inputInfo.pointer);
    return Success;
}

int
ProcGrabKeyboard(client)
    ClientPtr client;
{
    xGrabKeyboardReply rep;
    DeviceIntPtr device = inputInfo.keyboard;
    GrabPtr grab = device->grab;
    WindowPtr pWin, oldWin;
    TimeStamp time;
    REQUEST(xGrabKeyboardReq);

    REQUEST_SIZE_MATCH(xGrabKeyboardReq);
    if ((stuff->pointerMode != GrabModeSync) && 
	(stuff->pointerMode != GrabModeAsync) && 
	(stuff->keyboardMode != GrabModeSync) && 
	(stuff->keyboardMode != GrabModeAsync))
        return BadValue;
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    time = ClientTimeToServerTime(stuff->time);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    if ((grab) && (grab->client != client))
	rep.status = AlreadyGrabbed;
    else if (!pWin->realized)
	rep.status = GrabNotViewable;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (device->grab &&
	     (CompareTimeStamps(time, device->grabTime) == EARLIER)))
	rep.status = GrabInvalidTime;
    else if (device->sync.frozen &&
	     ((device->sync.other && (device->sync.other->client != client)) ||
	     ((device->sync.state >= FROZEN) &&
	      (device->grab->client != client))))
	rep.status = GrabFrozen;
    else
    {
	/* If a keyboard grab is already in effect, store the old grab window.*/

	oldWin = (grab) ? keybdGrab.window : device->u.keybd.focus.win;
	keybdGrab.window = pWin;
	keybdGrab.client = client;
        keybdGrab.ownerEvents = stuff->ownerEvents;
	keybdGrab.keyboardMode = stuff->keyboardMode;
	keybdGrab.pointerMode = stuff->pointerMode;
	keybdGrab.eventMask = KeyPressMask | KeyReleaseMask;
	keybdGrab.device = inputInfo.keyboard;
	ActivateKeyboardGrab(
	    device, &keybdGrab, ClientTimeToServerTime(stuff->time), FALSE);
	
	/* If the grab is a regrab on the same window as previous grab, don't
		generate any change of focus events. */

        if (oldWin != pWin)
		DoFocusEvents(oldWin, pWin, NotifyGrab);
	rep.status = GrabSuccess;
    }
    WriteReplyToClient(client, sizeof(xGrabKeyboardReply), &rep);
    return Success;
}

int
ProcUngrabKeyboard(client)
    ClientPtr client;
{
    DeviceIntPtr device = inputInfo.keyboard;
    GrabPtr grab = device->grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	(CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	(grab) && (grab->client == client))
    {
	DoFocusEvents(grab->window, device->u.keybd.focus.win, NotifyUngrab);
	DeactivateKeyboardGrab(device);
    }
    return Success;
}

static void
SetPointerStateMasks(ptr)
    DevicePtr ptr;
{
 /* all have to be defined since some button might be mapped here */
    stateMasks[1] = Button1Mask;
    stateMasks[2] = Button2Mask;
    stateMasks[3] = Button3Mask;
    stateMasks[4] = Button4Mask;
    stateMasks[5] = Button5Mask;
}

static void
SetKeyboardStateMasks(keybd)
    DeviceIntPtr keybd;
{
#define SET_MOD_STATE(entry, mask) \
    if (keybd->u.keybd.modMap.entry != NoSymbol)\
        stateMasks[keybd->u.keybd.modMap.entry] = mask;

    /* Note that 1-5 entries are buttons */

    int     i;
    for (i = 8; i < MAP_LENGTH; i++)
	stateMasks[i] = 0;
    SET_MOD_STATE(lock,   LockMask);

    SET_MOD_STATE(shiftA, ShiftMask);
    SET_MOD_STATE(shiftB, ShiftMask);

    SET_MOD_STATE(controlA, ControlMask);
    SET_MOD_STATE(controlB, ControlMask);

    SET_MOD_STATE(mod1A,    Mod1Mask);
    SET_MOD_STATE(mod1B,    Mod1Mask);

    SET_MOD_STATE(mod2A,    Mod2Mask);
    SET_MOD_STATE(mod2B,    Mod2Mask);

    SET_MOD_STATE(mod3A,    Mod3Mask);
    SET_MOD_STATE(mod3B,    Mod3Mask);

    SET_MOD_STATE(mod4A,    Mod4Mask);
    SET_MOD_STATE(mod4B,    Mod4Mask);

    SET_MOD_STATE(mod5A,    Mod5Mask);
    SET_MOD_STATE(mod5B,    Mod5Mask);
#undef SET_MOD_STATE
}

DevicePtr
AddInputDevice(deviceProc, autoStart)
    DeviceProc deviceProc;
    Bool autoStart;
{
    DeviceIntPtr d;
    if (inputInfo.numDevices == inputInfo.arraySize)
    {
	inputInfo.arraySize += 5;
	inputInfo.devices = (DeviceIntPtr *)Xrealloc(
	    inputInfo.devices, inputInfo.arraySize * sizeof(DeviceIntPtr));
    }
    d = (DeviceIntPtr) Xalloc(sizeof(DeviceIntRec));
    inputInfo.devices[inputInfo.numDevices++] = d;
    d->public.on = FALSE;
    d->public.processInputProc = NoopDDA;
    d->deviceProc = deviceProc;
    d->startup = autoStart;
    d->sync.frozen = FALSE;
    d->sync.other = NullGrab;
    d->sync.state = NOT_GRABBED;
    return &d->public;
}

DevicesDescriptor
GetInputDevices()
{
    DevicesDescriptor devs;
    devs.count = inputInfo.numDevices;
    devs.devices = (DevicePtr *)inputInfo.devices;
    return devs;
}

void
InitEvents()
{
    curKeySyms.map = (KeySym *)NULL;
    curKeySyms.minKeyCode = 0;
    curKeySyms.maxKeyCode = 0;
    curKeySyms.mapWidth = 0;

    currentScreen = &screenInfo.screen[0];
    inputInfo.numDevices = 0;
    if (spriteTraceSize == 0)
    {
	spriteTraceSize = 20;
	spriteTrace = (WindowPtr *)Xalloc(20*sizeof(WindowPtr));
    }
    spriteTraceGood = 1;
    if (focusTraceSize == 0)
    {
	focusTraceSize = 20;
	focusTrace = (WindowPtr *)Xalloc(20*sizeof(WindowPtr));
    }
    focusTraceGood = 1;
    lastEventMask = OwnerGrabButtonMask;
    sprite.win = NullWindow;
    sprite.current = NullCursor;
    sprite.hotLimits.x1 = 0;
    sprite.hotLimits.y1 = 0;
    sprite.hotLimits.x2 = currentScreen->width;
    sprite.hotLimits.y2 = currentScreen->height;
    lastWasMotion = FALSE;
    syncEvents.replayDev = (DeviceIntPtr)NULL;
    syncEvents.pending.forw = &syncEvents.pending;
    syncEvents.pending.back = &syncEvents.pending;
    syncEvents.free.forw = &syncEvents.free;
    syncEvents.free.back = &syncEvents.free;
    syncEvents.num = 0;
    syncEvents.playingEvents = FALSE;
    currentTime.months = 0;
    currentTime.milliseconds = GetTimeInMillis();
}

int
InitAndStartDevices(argc, argv)
    int argc;
    char *argv[];
{
    int     i;
    DeviceIntPtr d;

    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if ((*d->deviceProc) (d, DEVICE_INIT, argc, argv) == Success)
	    d->inited = TRUE;
	else
	    d->inited = FALSE;
    }
 /* do not turn any devices on until all have been inited */
    for (i = 0; i < inputInfo.numDevices; i++)
    {
	d = inputInfo.devices[i];
	if ((d->startup) && (d->inited))
	    (*d->deviceProc) (d, DEVICE_ON, argc, argv);
    }
    if (inputInfo.pointer && inputInfo.pointer->inited &&
	    inputInfo.keyboard && inputInfo.keyboard->inited)
	return Success;
    return BadImplementation;
}

void
CloseDownDevices(argc, argv)
    int argc;
    char *argv[];
{
    int     		i;
    DeviceIntPtr	d;

    Xfree(curKeySyms.map);

    for (i = inputInfo.numDevices - 1; i >= 0; i--)
    {
	d = inputInfo.devices[i];
	if (d->inited)
	    (*d->deviceProc) (d, DEVICE_CLOSE, argc, argv);
	Xfree(inputInfo.devices[i]);
    }
   
    /* The array inputInfo.devices doesn't need to be freed here since it
	will be reused when AddInputDevice is called when the server 
	resets again.*/
}

int
NumMotionEvents()
{
    return inputInfo.numMotionEvents;
}

void
RegisterPointerDevice(device, numMotionEvents)
    DevicePtr device;
    int numMotionEvents;
{
    inputInfo.pointer = (DeviceIntPtr)device;
    inputInfo.numMotionEvents = numMotionEvents;
    device->processInputProc = ProcessPointerEvent;
}

void
RegisterKeyboardDevice(device)
    DevicePtr device;
{
    inputInfo.keyboard = (DeviceIntPtr)device;
    device->processInputProc = ProcessKeyboardEvent;
}

void
InitPointerDeviceStruct(device, map, mapLength, motionProc, controlProc)
    DevicePtr device;
    BYTE *map;
    int mapLength;
    void (*controlProc)();
    int (*motionProc)();
{
    int i;
    DeviceIntPtr mouse = (DeviceIntPtr)device;

    mouse->grab = NullGrab;
    mouse->public.on = FALSE;
    mouse->u.ptr.mapLength = mapLength;
    mouse->u.ptr.map[0] = 0;
    for (i = 1; i <= mapLength; i++)
	mouse->u.ptr.map[i] = map[i];
    mouse->u.ptr.ctrl = defaultPointerControl;
    mouse->u.ptr.GetMotionProc = motionProc;
    mouse->u.ptr.CtrlProc = controlProc;
    mouse->u.ptr.autoReleaseGrab = FALSE;
    if (mouse == inputInfo.pointer)
	SetPointerStateMasks(mouse);
}

void
QueryMinMaxKeyCodes(minCode, maxCode)
    KeyCode *minCode, *maxCode;
{
    *minCode = curKeySyms.minKeyCode;
    *maxCode = curKeySyms.maxKeyCode;
}

static void
SetKeySymsMap(pKeySyms)
    KeySymsPtr pKeySyms;
{
    int i, j;
    int rowDif = pKeySyms->minKeyCode - curKeySyms.minKeyCode;
           /* if keysym map size changes, grow map first */

    if (pKeySyms->mapWidth < curKeySyms.mapWidth)
    {
        for (i = pKeySyms->minKeyCode; i <= pKeySyms->maxKeyCode; i++)
	{
#define SI(r, c) (((r-pKeySyms->minKeyCode)*pKeySyms->mapWidth) + (c))
#define DI(r, c) (((r - curKeySyms.minKeyCode)*curKeySyms.mapWidth) + (c))
	    for (j = 0; j < pKeySyms->mapWidth; j++)
		curKeySyms.map[DI(i, j)] = pKeySyms->map[SI(i, j)];
	    for (j = pKeySyms->mapWidth; j < curKeySyms.mapWidth; j++)
		curKeySyms.map[DI(i, j)] = NoSymbol;
#undef SI
#undef DI
	}
	return;
    }
    else if (pKeySyms->mapWidth > curKeySyms.mapWidth)
    {
        KeySym *map;
	int bytes = sizeof(KeySym) * pKeySyms->mapWidth *
               (curKeySyms.maxKeyCode - curKeySyms.minKeyCode + 1);
        map = (KeySym *)Xalloc(bytes);
	bzero(map, bytes);
        if (curKeySyms.map)
	{
            for (i = 0; i <= curKeySyms.maxKeyCode-curKeySyms.minKeyCode; i++)
		bcopy(
		    &curKeySyms.map[i*curKeySyms.mapWidth],
		    &map[i*pKeySyms->mapWidth],
		    curKeySyms.mapWidth * sizeof(KeySym));
	    Xfree(curKeySyms.map);
	}
	curKeySyms.mapWidth = pKeySyms->mapWidth;
        curKeySyms.map = map;
    }
    bcopy(
	pKeySyms->map,
	&curKeySyms.map[rowDif],
	(pKeySyms->maxKeyCode - pKeySyms->minKeyCode + 1) *
	    curKeySyms.mapWidth * sizeof(KeySym));
}


void 
InitKeyboardDeviceStruct(device, pKeySyms, pModifiers,
			      bellProc, controlProc)
    DevicePtr device;
    KeySymsPtr pKeySyms;
    ModifierMapPtr pModifiers;
    void (*bellProc)();
    void (*controlProc)();
{
#define SET_MOD(entry)  (keybd->u.keybd.modMap.entry = pModifiers->entry)

    DeviceIntPtr keybd = (DeviceIntPtr)device;

    keybd->grab = NullGrab;
    keybd->public.on = FALSE;

    keybd->u.keybd.ctrl = defaultKeyboardControl;
    keybd->u.keybd.BellProc = bellProc;
    keybd->u.keybd.CtrlProc = controlProc;
    keybd->u.keybd.focus.win = PointerRootWin;
    keybd->u.keybd.focus.revert = None;
    keybd->u.keybd.focus.time = currentTime;
    keybd->u.keybd.passiveGrab = FALSE;
    curKeySyms.minKeyCode = pKeySyms->minKeyCode;
    curKeySyms.maxKeyCode = pKeySyms->maxKeyCode;
    SET_MOD(lock);
    SET_MOD(shiftA);
    SET_MOD(shiftB);
    SET_MOD(controlA);
    SET_MOD(controlB);
    SET_MOD(mod1A);
    SET_MOD(mod1B);
    SET_MOD(mod2A);
    SET_MOD(mod2B);
    SET_MOD(mod3A);
    SET_MOD(mod3B);
    SET_MOD(mod4A);
    SET_MOD(mod4B);
    SET_MOD(mod5A);
    SET_MOD(mod5B);
    if (keybd == inputInfo.keyboard)
    {
	SetKeyboardStateMasks(keybd);
	SetKeySymsMap(pKeySyms);
	(*keybd->u.keybd.CtrlProc)(keybd, &keybd->u.keybd.ctrl);  
    }
#undef SET_MOD
}

void
InitOtherDeviceStruct(device, map, mapLength)
    DevicePtr device;
    BYTE *map;
    int mapLength;
{
    int i;
    DeviceIntPtr other = (DeviceIntPtr)device;

    other->grab = NullGrab;
    other->public.on = FALSE;
    other->u.other.mapLength = mapLength;
    other->u.other.map[0] =  0;
    for (i = 1; i <= mapLength; i++)
	other->u.other.map[i] = map[i];
    other->u.other.focus.win = NoneWin;
    other->u.other.focus.revert = None;
    other->u.other.focus.time = currentTime;
}

GrabPtr
SetDeviceGrab(device, grab)
    DevicePtr device;
    GrabPtr grab;
{
    register DeviceIntPtr dev = (DeviceIntPtr)device;
    GrabPtr oldGrab = dev->grab;
    dev->grab = grab; /* must not be deallocated */
    return oldGrab;
}

/*
 * Devices can't be resources since the bit patterns don't fit very well.
 * For one, where the client field would be, is random bits and the client
 * object might not be defined. For another, the "server" bit might be on.
 */

#ifdef INPUT_EXTENSION

DevicePtr
LookupInputDevice(deviceID)
    Device deviceID;
{
    int i;
    for (i = 0; i < inputInfo.numDevices; i++)
	if (inputInfo.devices[i]->public.deviceID == deviceID)
	    return &(inputInfo.devices[i]->public);
    return NullDevice;
}
#endif /* INTPUT_EXTENSION */

DevicePtr
LookupKeyboardDevice()
{
    return &inputInfo.keyboard->public;
}

DevicePtr
LookupPointerDevice()
{
    return &inputInfo.pointer->public;
}


static int
SendMappingNotify(request, firstKeyCode, count)
    BYTE request, firstKeyCode, count;
{
    int i;
    xEvent event;

    event.u.u.type = MappingNotify;
    event.u.mappingNotify.request = request;
    if (request == MappingKeyboard)
    {
        event.u.mappingNotify.firstKeyCode = firstKeyCode;
        event.u.mappingNotify.count = count;
    }
    /* 0 is the server client */
    for (i=1; i<currentMaxClients; i++)
        if (! clients[i]->clientGone)
            WriteEventToClient(clients[i], 1, &event);
}

/*
 * n-sqared algorithm. n < 255 and don't want to copy the whole thing and
 * sort it to do the checking. How often is it called? Just being lazy?
 */
static Bool
BadDeviceMap(buff, length, low, high)
    BYTE *buff;
    int length;
    unsigned low, high;
{
    int     i, j;

    for (i = 0; i < length; i++)
	if (buff[i])		       /* only check non-zero elements */
	{
	    if ((low > buff[i]) || (high < buff[i]))
		return TRUE;
	    for (j = i + 1; j < length; j++)
		if (buff[i] == buff[j])
		    return TRUE;
	}
    return FALSE;
}

int 
ProcSetModifierMapping(client)
    ClientPtr client;
{
#define LEGAL_MOD(mod) \
    if (modMap.mod != NoSymbol)\
    {\
        if (!LegalModifier(modMap.mod))\
        {\
            rep.success = MappingFailed;\
	    goto WriteMappingReply;\
	}\
	if ((inputInfo.keyboard->u.keybd.modMap.mod != modMap.mod) &&\
		IsOn(inputInfo.keyboard->down, modMap.mod))\
        {\
            rep.success = MappingBusy;\
	    goto WriteMappingReply;\
	}\
    }\

    xSetModifierMappingReply rep;
    REQUEST(xSetModifierMappingReq);
    ModifierMapRec modMap;
    
    REQUEST_SIZE_MATCH(xSetModifierMappingReq);

    modMap.lock = stuff->lock;
    modMap.shiftA = stuff->shiftA;
    modMap.shiftB = stuff->shiftB;
    modMap.controlA = stuff->controlA;
    modMap.controlB = stuff->controlB;
    modMap.mod1A = stuff->mod1A;
    modMap.mod1B = stuff->mod1B;
    modMap.mod2A = stuff->mod2A;
    modMap.mod2B = stuff->mod2B;
    modMap.mod3A = stuff->mod3A;
    modMap.mod3B = stuff->mod3B;
    modMap.mod4A = stuff->mod4A;
    modMap.mod4B = stuff->mod4B;
    modMap.mod5A = stuff->mod5A;
    modMap.mod5B = stuff->mod5B;

    if (BadDeviceMap (&modMap, 15, curKeySyms.minKeyCode, 
		      curKeySyms.maxKeyCode))
        return BadValue;    

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;

    LEGAL_MOD(lock); 
    LEGAL_MOD(shiftA);
    LEGAL_MOD(shiftB);
    LEGAL_MOD(controlA);
    LEGAL_MOD(controlB);
    LEGAL_MOD(mod1A);
    LEGAL_MOD(mod1B);
    LEGAL_MOD(mod2A);
    LEGAL_MOD(mod2B);
    LEGAL_MOD(mod3A);
    LEGAL_MOD(mod3B);
    LEGAL_MOD(mod4A);
    LEGAL_MOD(mod4B);
    LEGAL_MOD(mod5A);
    LEGAL_MOD(mod5B);

WriteMappingReply:
    WriteReplyToClient(client, sizeof(xSetModifierMappingReply), &rep);

    if (rep.success == MappingSuccess)
    {
	inputInfo.keyboard->u.keybd.modMap = modMap;
	SetKeyboardStateMasks(inputInfo.keyboard);
        SendMappingNotify(MappingModifier, 0, 0);
    }
    return(client->noClientException);
#undef LEGAL_MOD
}

int
ProcGetModifierMapping(client)
    ClientPtr client;
{
    xGetModifierMappingReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.lock = inputInfo.keyboard->u.keybd.modMap.lock;
    rep.shiftA = inputInfo.keyboard->u.keybd.modMap.shiftA;
    rep.shiftB = inputInfo.keyboard->u.keybd.modMap.shiftB;
    rep.controlA = inputInfo.keyboard->u.keybd.modMap.controlA;
    rep.controlB = inputInfo.keyboard->u.keybd.modMap.controlB;
    rep.mod1A = inputInfo.keyboard->u.keybd.modMap.mod1A;
    rep.mod1B = inputInfo.keyboard->u.keybd.modMap.mod1B;
    rep.mod2A = inputInfo.keyboard->u.keybd.modMap.mod2A;
    rep.mod2B = inputInfo.keyboard->u.keybd.modMap.mod2B;
    rep.mod3A = inputInfo.keyboard->u.keybd.modMap.mod3A;
    rep.mod3B = inputInfo.keyboard->u.keybd.modMap.mod3B;
    rep.mod4A = inputInfo.keyboard->u.keybd.modMap.mod4A;
    rep.mod4B = inputInfo.keyboard->u.keybd.modMap.mod4B;
    rep.mod5A = inputInfo.keyboard->u.keybd.modMap.mod5A;
    rep.mod5B = inputInfo.keyboard->u.keybd.modMap.mod5B;

    WriteReplyToClient(client, sizeof(xGetModifierMappingReply), &rep);

    return client->noClientException;
}

int
ProcChangeKeyboardMapping(client)
    ClientPtr client;
{
    REQUEST(xChangeKeyboardMappingReq);
    int len;
    int count;
    KeySymsRec keysyms;

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardMappingReq);

    len = stuff->length - (sizeof(xChangeKeyboardMappingReq) >> 2);  
    if (len != (stuff->keyCodes * stuff->keySymsPerKeyCode))
            return BadLength;
    if ((stuff->firstKeyCode < curKeySyms.minKeyCode) ||
	(stuff->firstKeyCode + len > curKeySyms.maxKeyCode) ||
        (stuff->keySymsPerKeyCode == 0))
            return BadValue;
    count = len / stuff->keySymsPerKeyCode;
    keysyms.minKeyCode = stuff->firstKeyCode;
    keysyms.maxKeyCode = stuff->firstKeyCode + count;
    keysyms.mapWidth = stuff->keySymsPerKeyCode;
    keysyms.map = (KeySym *)&stuff[1];
    SetKeySymsMap(&keysyms);
    SendMappingNotify(MappingKeyboard, stuff->firstKeyCode, count);
    return client->noClientException;

}

int
ProcSetPointerMapping(client)
    ClientPtr client;
{
    REQUEST(xSetPointerMappingReq);
    BYTE *map;
    xSetPointerMappingReply rep;
    register int i;

    REQUEST_AT_LEAST_SIZE(xSetPointerMappingReq);
    if (stuff->length != (sizeof(xSetPointerMappingReq) + stuff->nElts + 3)>>2)
	return BadLength;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;
    map = (BYTE *)&stuff[1];
    if (stuff->nElts != inputInfo.pointer->u.ptr.mapLength)
	return BadValue;
    if (BadDeviceMap(&map[0], stuff->nElts, 1, 255))
	return BadValue;
    for (i=0; i < stuff->nElts; i++)
	if ((inputInfo.pointer->u.ptr.map[i + 1] != map[i]) &&
		IsOn(inputInfo.pointer->down, i + 1))
	{
    	    rep.success = MappingBusy;
	    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
            return Success;
	}
    for (i = 0; i < stuff->nElts; i++)
	inputInfo.pointer->u.ptr.map[i + 1] = map[i];
    SetPointerStateMasks(inputInfo.pointer);	   
    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
    SendMappingNotify(MappingPointer, 0, 0);
    return Success;
}

int
ProcGetKeyboardMapping(client)
    ClientPtr client;
{
    xGetKeyboardMappingReply rep;
    REQUEST(xGetKeyboardMappingReq);

    REQUEST_SIZE_MATCH(xGetKeyboardMappingReq);

    if ((stuff->firstKeyCode < curKeySyms.minKeyCode) ||
        (stuff->firstKeyCode > curKeySyms.maxKeyCode) ||
	(stuff->firstKeyCode + stuff->count > curKeySyms.maxKeyCode + 1))
        return BadValue;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.keySymsPerKeyCode = curKeySyms.mapWidth;
/* length is a count of 4 byte quantities and KeySyms are 4 bytes */
    rep.length = (curKeySyms.mapWidth * stuff->count);
    WriteReplyToClient(client, sizeof(xGetKeyboardMappingReply), &rep);

    WriteReplyToClient(
	client,
	curKeySyms.mapWidth * stuff->count * sizeof(KeySym),
	&curKeySyms.map[stuff->firstKeyCode - curKeySyms.minKeyCode]);

    return client->noClientException;
}

int
ProcGetPointerMapping(client)
    ClientPtr client;
{
    xGetPointerMappingReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nElts = inputInfo.pointer->u.ptr.mapLength;
    rep.length = (rep.nElts + (4-1))/4;
    WriteReplyToClient(client, sizeof(xGetPointerMappingReply), &rep);
    WriteReplyToClient(client, rep.nElts, &inputInfo.pointer->u.ptr.map[1]);
    return Success;    
}

int
Ones(mask)                /* HACKMEM 169 */
    Mask mask;
{
    register int y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % 077);
}

void
NoteLedState(keybd, led, on)
    DeviceIntPtr keybd;
    int		led;
    Bool	on;
{
    KeybdCtrl *ctrl = &keybd->u.keybd.ctrl;
    if (on)
	ctrl->leds |= (1 << (led - 1));
    else
	ctrl->leds &= ~(1 << (led - 1)); /* assumes 32-bit longs XXX */
}

int
ProcChangeKeyboardControl (client)
    ClientPtr client;
{
#define DO_ALL    0xffffffff
    KeybdCtrl ctrl;
    DeviceIntPtr keybd = inputInfo.keyboard;
    long *vlist;
    int t;
    int led = DO_ALL;
    int key = DO_ALL;
    REQUEST(xChangeKeyboardControlReq);

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardControlReq);
    if (stuff->length !=(sizeof(xChangeKeyboardControlReq)>>2) + Ones(stuff->mask))
	return BadLength;
    vlist = (long *)&stuff[1];		/* first word of values */
    ctrl = keybd->u.keybd.ctrl;
    if (stuff->mask & KBKeyClickPercent)
    {
	t = (INT8)*vlist;
	vlist++;
	if (t == -1)
	    t = defaultKeyboardControl.click;
	else if (t < 0 || t > 100)
	    return BadValue;
	ctrl.click = t;
    }
    if (stuff->mask & KBBellPercent)
    {
	t = (INT8)*vlist;
	vlist++;
	if (t == -1)
	    t = defaultKeyboardControl.bell;
	else if (t < 0 || t > 100)
	    return BadValue;
	ctrl.bell = t;
    }
    if (stuff->mask & KBBellPitch)
    {
	t = (INT16)*vlist;
	vlist++;
	if (t == -1)
	    t = defaultKeyboardControl.bell_pitch;
	else if (t < 0)
	    return BadValue;
	ctrl.bell_pitch = t;
    }
    if (stuff->mask & KBBellDuration)
    {
	t = (INT16)*vlist;
	vlist++;
	if (t == -1)
	    t = defaultKeyboardControl.bell_duration;
	else if (t < 0)
	    return BadValue;
	ctrl.bell_duration = t;
    }
    if (stuff->mask & KBLed)
    {
	led = (CARD8)*vlist;
	vlist++;
	if (led < 1 || led > 32)
	    return BadValue;
	if (!(stuff->mask & KBLedMode))
	    return BadMatch;
    }
    if (stuff->mask & KBLedMode)
    {
	t = (CARD8)*vlist;
	vlist++;
	if (t == LedModeOff)
	{
	    if (led == DO_ALL)
		ctrl.leds = 0x0;
	    else
		ctrl.leds &= ~(1 << (led - 1)); /* assumes 32-bit longs XXX */
	}
	else if (t == LedModeOn)
	{
	    if (led == DO_ALL)
		ctrl.leds = DO_ALL;
	    else
		ctrl.leds |= (1 << (led - 1));
	}
	else
	    return BadValue;
    }
    if (stuff->mask & KBKey)
    {
	key = (KeyCode)*vlist;
	vlist++;
	if (key < 8 || key > 255)
	    return BadValue;
	if (!(stuff->mask & KBAutoRepeatMode))
	    return BadMatch;
    }
    if (stuff->mask & KBAutoRepeatMode)
    {
	int index = (key >> 3);
	int mask = (1 << (key & 7));
	t = (CARD8)*vlist;
	vlist++;
	if (t == AutoRepeatModeOff)
	{
	    if (key == DO_ALL)
		ctrl.autoRepeat = FALSE;
	    else
		ctrl.autoRepeats[index] &= ~mask;
	}
	else if (t == AutoRepeatModeOn)
	{
	    if (key == DO_ALL)
		ctrl.autoRepeat = TRUE;
	    else
		ctrl.autoRepeats[index] |= mask;
	}
	else if (t == AutoRepeatModeDefault)
	{
	    if (key == DO_ALL)
		ctrl.autoRepeat = defaultKeyboardControl.autoRepeat;
	    else
		ctrl.autoRepeats[index] &= ~mask;
		ctrl.autoRepeats[index] =
			(ctrl.autoRepeats[index] & ~mask) |
			(defaultKeyboardControl.autoRepeats[index] & mask);
	}
	else
	    return BadValue;
    }
    keybd->u.keybd.ctrl = ctrl;
    (*keybd->u.keybd.CtrlProc)(keybd, &keybd->u.keybd.ctrl);
    return Success;
#undef DO_ALL
} 

int
ProcGetKeyboardControl (client)
    ClientPtr client;
{
    int i;
    DeviceIntPtr keybd = inputInfo.keyboard;
    xGetKeyboardControlReply rep;
    REQUEST(xReq);

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 5;
    rep.sequenceNumber = client->sequence;
    rep.globalAutoRepeat = keybd->u.keybd.ctrl.autoRepeat;
    rep.keyClickPercent = keybd->u.keybd.ctrl.click;
    rep.bellPercent = keybd->u.keybd.ctrl.bell;
    rep.bellPitch = keybd->u.keybd.ctrl.bell_pitch;
    rep.bellDuration = keybd->u.keybd.ctrl.bell_duration;
    rep.ledMask = keybd->u.keybd.ctrl.leds;
    for (i = 0; i < 32; i++)
	rep.map[i] = keybd->u.keybd.ctrl.autoRepeats[i];
    WriteReplyToClient(client, sizeof(xGetKeyboardControlReply), &rep);
    return Success;
} 

int
ProcBell(client)
    ClientPtr client;
{
    register DeviceIntPtr keybd = inputInfo.keyboard;
    int base = keybd->u.keybd.ctrl.bell;
    int newpercent;
    REQUEST(xBellReq);
    REQUEST_SIZE_MATCH(xBellReq);
    if (stuff->percent < -100 || stuff->percent > 100)
	return BadValue;
    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;
    (*keybd->u.keybd.BellProc)(newpercent, keybd);
    return Success;
} 

int
ProcChangePointerControl(client)
    ClientPtr client;
{
    DeviceIntPtr mouse = inputInfo.pointer;
    PtrCtrl ctrl;		/* might get BadValue part way through */
    REQUEST(xChangePointerControlReq);

    REQUEST_SIZE_MATCH(xChangePointerControlReq);
    ctrl = mouse->u.ptr.ctrl;
    if (stuff->doAccel)
    {
	if (stuff->accelNum == -1)
	    ctrl.num = defaultPointerControl.num;
	else if (stuff->accelNum < 0)
	{
	    client->errorValue = stuff->accelNum;
	    return BadValue;
	}
	else ctrl.num = stuff->accelNum;
	if (stuff->accelDenum == -1)
	    ctrl.den = defaultPointerControl.den;
	else if (stuff->accelDenum <= 0)
	{
	    client->errorValue = stuff->accelDenum;
	    return BadValue;
	}
	else ctrl.den = stuff->accelDenum;
    }
    if (stuff->doThresh)
    {
	if (stuff->threshold == -1)
	    ctrl.threshold = defaultPointerControl.threshold;
	else if (stuff->threshold <= 0)
	{
	    client->errorValue = stuff->threshold;
	    return BadValue;
	}
	else ctrl.threshold = stuff->threshold;
    }
    mouse->u.ptr.ctrl = ctrl;
    (*mouse->u.ptr.CtrlProc)(mouse, &mouse->u.ptr.ctrl);
    return Success;
} 

int
ProcGetPointerControl(client)
    ClientPtr client;
{
    register DeviceIntPtr mouse = inputInfo.pointer;
    REQUEST(xReq);
    xGetPointerControlReply rep;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.threshold = mouse->u.ptr.ctrl.threshold;
    rep.accelNumerator = mouse->u.ptr.ctrl.num;
    rep.accelDenominator = mouse->u.ptr.ctrl.den;
    WriteReplyToClient(client, sizeof(xGenericReply), &rep);
    return Success;
}

int
ProcGetMotionEvents(client)
    ClientPtr client;
{
    WindowPtr pWin;
    xTimecoord * coords;
    xGetMotionEventsReply rep;
    int     i, count, xmin, xmax, ymin, ymax;
    DeviceIntPtr mouse = inputInfo.pointer;
    TimeStamp start, stop;
    REQUEST(xGetMotionEventsReq);

    REQUEST_SIZE_MATCH(xGetMotionEventsReq);
    pWin = LookupWindow(stuff->window, client);
    if (!pWin)
    {
	client->errorValue = stuff->window;
	return BadWindow;
    }
    lastWasMotion = FALSE;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nEvents = 0;
    start = ClientTimeToServerTime(stuff->start);
    stop = ClientTimeToServerTime(stuff->stop);
    if (CompareTimeStamps(start, stop) == LATER)
        return Success;
    if (CompareTimeStamps(start, currentTime) == LATER)
        return Success;
    if (CompareTimeStamps(stop, currentTime) == LATER)
        stop = currentTime;
    if (inputInfo.numMotionEvents)
    {
	coords = (xTimecoord *) Xalloc(
		inputInfo.numMotionEvents * sizeof(xTimecoord));
	count = (*mouse->u.ptr.GetMotionProc) (
		mouse, coords, start.milliseconds, stop.milliseconds);
	xmin = pWin->absCorner.x - pWin->borderWidth;
	xmax =
	    pWin->absCorner.x + pWin->clientWinSize.width + pWin->borderWidth;
	ymin = pWin->absCorner.y - pWin->borderWidth;
	ymax =
	    pWin->absCorner.y + pWin->clientWinSize.height + pWin->borderWidth;
	for (i = 0; i < count; i++)
	    if ((xmin <= coords[i].x) && (coords[i].x < xmax) &&
		    (ymin <= coords[i].y) && (coords[i].y < ymax))
	    {
		coords[rep.nEvents].x = coords[i].x - pWin->absCorner.x;
		coords[rep.nEvents].y = coords[i].y - pWin->absCorner.y;
		rep.nEvents++;
	    }
    }
    rep.length = rep.nEvents * sizeof(xTimecoord) / 4;
    WriteReplyToClient(client, sizeof(xGetMotionEventsReply), &rep);
    if (inputInfo.numMotionEvents)
    {
	WriteReplyToClient( client, rep.nEvents * sizeof(xTimecoord), coords);
	Xfree(coords);
    }
    return Success;
}

int
ProcQueryPointer(client)
    ClientPtr client;
{
    xQueryPointerReply rep;
    WindowPtr pWin, t;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pWin = LookupWindow(stuff->id, client);
    if (!pWin)
    {
	client->errorValue = stuff->id;
	return BadWindow;
    }
    lastWasMotion = FALSE;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.sameScreen = xTrue;		/* XXX */
    rep.mask = keyButtonState;
    rep.length = 0;
    rep.root = (ROOT)->wid;
    rep.rootX = sprite.hot.x;
    rep.rootY = sprite.hot.y;
    rep.winX = sprite.hot.x - pWin->absCorner.x;
    rep.winY = sprite.hot.y - pWin->absCorner.y;
    rep.child = None;
    for (t = sprite.win; t; t = t->parent)
	if (t->parent == pWin)
	{
	    rep.child = t->wid;
	    break;
	}
    WriteReplyToClient(client, sizeof(xQueryPointerReply), &rep);

    return(Success);    
}

int
ProcQueryKeymap(client)
    ClientPtr client;
{
    xQueryKeymapReply rep;
    int i;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 2;
    for (i = 0; i<32; i++)
	rep.map[i] = inputInfo.keyboard->down[i];
    WriteReplyToClient(client, sizeof(xQueryKeymapReply), &rep);
    return Success;
}

/* This define is taken from extension.c and must be consistent with it.
	This is probably not the best programming practice. PRH */

#define EXTENSION_EVENT_BASE  64
int
ProcSendEvent(client)
    ClientPtr client;
{
    extern int lastEvent; 		/* Defined in extension.c */
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow; /* only set if dest==InputFocus */
    REQUEST(xSendEventReq);

    REQUEST_SIZE_MATCH(xSendEventReq);

    /* The client's event type must be a core event type or one defined by an
	extension. */

    if ( ! ((stuff->event.u.u.type < LASTEvent) || 
	((EXTENSION_EVENT_BASE  <= stuff->event.u.u.type) &&
	(stuff->event.u.u.type < lastEvent))) )
	return BadValue;

    if (stuff->destination == PointerWindow)
	pWin = sprite.win;
    else if (stuff->destination == InputFocus)
    {
	WindowPtr inputFocus = inputInfo.keyboard->u.keybd.focus.win;

	if (inputFocus == NoneWin)
	    return Success;

	/* If the input focus is PointerRootWin, send the event to where
	the pointer is if possible, then perhaps propogate up to root. */
   	if (inputFocus == PointerRootWin)
	    inputFocus = ROOT;

	if (IsParent(inputFocus, sprite.win))
	{
	    effectiveFocus = inputFocus;
	    pWin = sprite.win;
	}
	else
	    effectiveFocus = pWin = inputFocus;
    }
    else
	pWin = LookupWindow(stuff->destination, client);
    if (!pWin)
    {
	client->errorValue = stuff->destination;
	return BadWindow;
    }
    stuff->event.u.u.type |= 0x80;
    if (stuff->propagate)
    {
	for (;pWin; pWin = pWin->parent)
	{
	    if (DeliverEventsToWindow(
			pWin, &stuff->event, 1, stuff->eventMask, NullGrab))
		return Success;
	    if (pWin == effectiveFocus)
		return Success;
	    stuff->eventMask &= ~pWin->dontPropagateMask;
	}
    }
    else
	DeliverEventsToWindow(
	    pWin, &stuff->event, 1, stuff->eventMask, NullGrab);
    return Success;
}

int
ProcUngrabKey(client)
    ClientPtr client;
{
    REQUEST(xUngrabKeyReq);
    WindowPtr pWin;
    GrabPtr grab;

    REQUEST_SIZE_MATCH(xUngrabKeyReq);
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (MatchingGrab(
	    stuff->key, stuff->modifiers, inputInfo.keyboard, grab, TRUE))
	{
	    if (client == grab->client)
		FreeResource(grab->resource, RC_NONE);
		/*
		 * Might need to unlink multiple requests, the keys might
		 * be put on individually, but taken off with an AnyMumble.
		 */
	}
    }
    return(Success);
}

int
ProcGrabKey(client)
    ClientPtr client;
{
    WindowPtr pWin;
    REQUEST(xGrabKeyReq);
    GrabPtr grab;
    Bool useOld = FALSE;

    REQUEST_SIZE_MATCH(xGrabKeyReq);
    if (((stuff->key > curKeySyms.maxKeyCode) || (stuff->key < curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
        return BadValue;
    pWin = LookupWindow(stuff->grabWindow, client);
    client->errorValue = stuff->grabWindow;
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (MatchingGrab(
	    stuff->key, stuff->modifiers, inputInfo.keyboard, grab, TRUE))
	{
	    if (client == grab->client)
	    {
		useOld = TRUE;
		break;
	    }
	    else
		return BadAccess;
	}
    }
    if (!useOld)
    {
	grab = (GrabPtr)Xalloc(sizeof(GrabRec));
	grab->next = PASSIVEGRABS(pWin);
	grab->resource = FakeClientID(client->index);
	pWin->passiveGrabs = (pointer)grab;
	AddResource(grab->resource, RT_FAKE, pWin, PassiveClientGone, RC_CORE);
    }
    grab->client = client;
    grab->device = inputInfo.keyboard;
    grab->window = pWin;
    grab->eventMask = KeyPressMask | KeyReleaseMask;
    grab->ownerEvents = stuff->ownerEvents;
    grab->keyboardMode = stuff->keyboardMode;
    grab->pointerMode = stuff->pointerMode;
    grab->modifiers = stuff->modifiers;
    grab->u.keybd.key = stuff->key;
    return(Success);
}

int
ProcGrabButton(client)
    ClientPtr client;
{
    WindowPtr pWin, confineTo;
    REQUEST(xGrabButtonReq);
    GrabPtr grab;
    Bool useOld = FALSE;
    CursorPtr cursor;

    REQUEST_SIZE_MATCH(xGrabButtonReq);
    if ((stuff->pointerMode != GrabModeSync) && 
	(stuff->pointerMode != GrabModeAsync) && 
	(stuff->keyboardMode != GrabModeSync) && 
	(stuff->keyboardMode != GrabModeAsync))
        return BadValue;

    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    if (stuff->confineTo == None)
	confineTo = NullWindow;
    else
    {
	confineTo = LookupWindow(stuff->confineTo, client);
	if (!confineTo)
	{
	    client->errorValue = stuff->confineTo;
	    return BadWindow;
	}
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	cursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    return BadCursor;
	}
    }
    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (MatchingGrab(
	    stuff->button, stuff->modifiers, inputInfo.pointer, grab, FALSE))
	{
	    if (client == grab->client)
	    {
		useOld = TRUE;
		break;
	    }
	    else
		return BadAccess;
	}
    }
    if (!useOld)
    {
	grab = (GrabPtr)Xalloc(sizeof(GrabRec));
	grab->next = PASSIVEGRABS(pWin);
	grab->resource = FakeClientID(client->index);
	pWin->passiveGrabs = (pointer)grab;
	AddResource(grab->resource, RT_FAKE, pWin, PassiveClientGone, RC_CORE);
    }
    grab->client = client;
    grab->device = inputInfo.pointer;
    grab->window = pWin;
    grab->eventMask = stuff->eventMask | ButtonPress | ButtonRelease;
    grab->ownerEvents = stuff->ownerEvents;
    grab->keyboardMode = stuff->keyboardMode;
    grab->pointerMode = stuff->pointerMode;
    grab->modifiers = stuff->modifiers;
    grab->u.ptr.button = stuff->button;
    grab->u.ptr.confineTo = confineTo;
    grab->u.ptr.cursor = cursor;
    return(Success);
}

int
ProcUngrabButton(client)
    ClientPtr client;
{
    REQUEST(xUngrabButtonReq);
    WindowPtr pWin;
    GrabPtr grab;

    REQUEST_SIZE_MATCH(xUngrabButtonReq);
    pWin = LookupWindow(stuff->grabWindow, client);
    if (!pWin)
    {
	client->errorValue = stuff->grabWindow;
	return BadWindow;
    }
    for (grab = PASSIVEGRABS(pWin); grab; grab = grab->next)
    {
	if (MatchingGrab(
	    stuff->button, stuff->modifiers, inputInfo.pointer, grab, FALSE))
	{
	    if (client == grab->client)
		FreeResource(grab->resource, RC_NONE);
		/*
		 * Might need to unlink multiple requests, the buttons might
		 * be put on individually, but taken off with an AnyMumble.
		 */
	}
    }
    return(Success);
}

void
DeleteWindowFromAnyEvents(pWin, freeResources)
    WindowPtr		pWin;
    Bool		freeResources;
{
    WindowPtr		parent;
    FocusPtr		focus = &inputInfo.keyboard->u.keybd.focus;
    DeviceIntPtr	mouse = inputInfo.pointer;
    OtherClientsPtr	oc;
    GrabPtr		passive;


    /* Deactivate any grabs performed on this window, before making any
	input focus changes. */

    if ((mouse->grab) && (mouse->grab->window == pWin))
	DeactivatePointerGrab(mouse);

    /* Deactivating a keyboard grab should cause focus events. */

    if ((inputInfo.keyboard->grab) &&
	(inputInfo.keyboard->grab->window == pWin))
    {
	DoFocusEvents(inputInfo.keyboard->grab->window, focus->win, NotifyUngrab);
	DeactivateKeyboardGrab(inputInfo.keyboard);
    }

    /* If the focus window is a root window (ie. has no parent) then don't 
	delete the focus from it. */
    
    if ((pWin == focus->win) && (pWin->parent != NullWindow))
    {
	int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	if (inputInfo.keyboard->grab)
		focusEventMode = NotifyWhileGrabbed;

	switch (focus->revert)
	{
	    case RevertToNone:
		DoFocusEvents(pWin, NoneWin, focusEventMode);
		focus->win = NoneWin;
		break;
	    case RevertToParent:
		for (
		    parent = pWin->parent; 
		    !parent->realized; 
		    parent = parent->parent);
		DoFocusEvents(pWin, parent, focusEventMode);
		focus->win = parent;
		focus->revert = RevertToNone;
		break;
	    case RevertToPointerRoot:
		DoFocusEvents(pWin, PointerRootWin, focusEventMode);
		focus->win = PointerRootWin;
		break;
	}
    }

    if (freeResources)
    {
	while (oc = OTHERCLIENTS(pWin))
	    FreeResource(oc->resource, RC_NONE);
	while (passive = PASSIVEGRABS(pWin))
	    FreeResource(passive->resource, RC_NONE);
     }
}

Mask
EventMaskForClient(win, client, allMask)
    WindowPtr		win;
    ClientPtr		client;
    Mask		*allMask;
{
    OtherClientsPtr	other;
    Mask		him;
    if (win->client == client)
	him = win->eventMask;
    *allMask = win->eventMask;
    for (other = OTHERCLIENTS(win); other; other = other->next)
    {
	if (other->client == client)
	    him = other->mask;
	*allMask |= other->mask;
    }
    return him;
}


int
ProcRecolorCursor(client)
    ClientPtr client;
{
    CursorPtr pCursor;
    int		nscr;
    ScreenPtr	pscr;
    REQUEST(xRecolorCursorReq);

    REQUEST_SIZE_MATCH(xRecolorCursorReq);
    pCursor = (CursorPtr)LookupID(stuff->cursor, RT_CURSOR, RC_CORE);
    if ( !pCursor) 
    {
	client->errorValue = stuff->cursor;
	return (BadCursor);
    }

    pCursor->foreRed = stuff->foreRed;
    pCursor->foreGreen = stuff->foreGreen;
    pCursor->foreBlue = stuff->foreBlue;

    pCursor->backRed = stuff->backRed;
    pCursor->backGreen = stuff->backGreen;
    pCursor->backBlue = stuff->backBlue;

    for ( nscr=0, pscr=screenInfo.screen;
	  nscr<screenInfo.numScreens;
	  nscr++, pscr++)
    {
	( *pscr->RecolorCursor)(pscr, pCursor,
		(pCursor == sprite.current) && (pscr == currentScreen));
    }
    return (Success);
}

void
WriteEventToClient(pClient, count, pbuf)
    ClientPtr	pClient;
    int		count;
    char	*pbuf;
{
    if(pClient->swapped)
    {
    int iEvent;
      for(iEvent = 0; iEvent < count; iEvent++)
	/* Remember to stip off the leading bit of type in case this event was sent
		with "SendEvent" */

      (*EventSwapVector[(((xEvent *)pbuf) + iEvent)->u.u.type & 0177])
		(pClient, sizeof(xEvent), pbuf);
    }
    else
    {
	WriteToClient(pClient, count * sizeof(xEvent), pbuf);
    }
}
