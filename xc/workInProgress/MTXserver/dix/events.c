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

Copyright 1992, 1993 Data General Corporation;
Copyright 1992, 1993 OMRON Corporation  

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
neither the name OMRON or DATA GENERAL be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission of the party whose name is to be used.  Neither OMRON or 
DATA GENERAL make any representation about the suitability of this software
for any purpose.  It is provided "as is" without express or implied warranty.  

OMRON AND DATA GENERAL EACH DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OMRON OR DATA GENERAL BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: events.c,v 1.3 94/01/11 20:45:58 rob Exp $ */

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

/* MTX note: MTX included xinInputExt.h here */

#include "dixstruct.h"

#ifdef XKB
#include "XKBsrv.h"
#endif

#include "mtxlock.h"
#include "message.h"

extern WindowPtr *WindowTable;

extern void (* EventSwapVector[128]) ();
extern void (* ReplySwapVector[256]) ();

#ifndef XTHREADS
extern void SetCriticalOutputPending();
#endif /* not XTHREADS */

#define EXTENSION_EVENT_BASE  64

#define NoSuchEvent 0x80000000	/* so doesn't match NoEventMask */
#define StructureAndSubMask ( StructureNotifyMask | SubstructureNotifyMask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define MotionMask ( \
	PointerMotionMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PropagateMask ( \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	MotionMask )
#define PointerGrabMask ( \
	ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | \
	PointerMotionHintMask | KeymapStateMask | \
	MotionMask )
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
#define AllEventMasks (lastEventMask|(lastEventMask-1))
/*
 * The following relies on the fact that the Button<n>MotionMasks are equal
 * to the corresponding Button<n>Masks from the current modifier/button state.
 */
#define Motion_Filter(class) (PointerMotionMask | \
			      (class)->state | (class)->motionMask)


#define WID(w) ((w) ? ((w)->drawable.id) : 0)

#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])

#define DNPMCOUNT 8

Mask DontPropagateMasks[DNPMCOUNT];
static int DontPropagateRefCnts[DNPMCOUNT];

#ifdef DEBUG
static debug_events = 0;
#endif
InputInfo inputInfo;

static struct {
    QdEventPtr		pending, *pendtail;
    DeviceIntPtr	replayDev;	/* kludgy rock to put flag for */
    WindowPtr		replayWin;	/*   ComputeFreezes            */
    Bool		playingEvents;
    TimeStamp		time;
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

typedef struct {
    int		x, y;
    ScreenPtr	pScreen;
} HotSpot;

static  struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints of hot spot */
    Bool	confined;	/* confined to screen */
#ifdef SHAPE
    RegionPtr	hotShape;	/* additional logical shape constraint */
#endif
    BoxRec	physLimits;	/* physical constraints of hot spot */
    WindowPtr	win;		/* window of logical position */
    HotSpot	hot;		/* logical pointer position */
    HotSpot	hotPhys;	/* physical pointer position */
} sprite;			/* info about the cursor sprite */

static void DoEnterLeaveEvents(
#if NeedFunctionPrototypes
    WindowPtr /*fromWin*/,
    WindowPtr /*toWin*/,
    int /*mode*/
#endif
);

static WindowPtr XYToWindow(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/
#endif
);

void DeliverFocusedEvent();
int DeliverDeviceEvents();
void DoFocusEvents();
Mask EventMaskForClient();
Bool CheckDeviceGrabs();
void NewCurrentScreen();
void EnqueueEvent();

extern GrabPtr CreateGrab();		/* Defined in grabs.c */
extern Bool GrabMatchesSecond();
extern Bool DeletePassiveGrabFromList();
extern int AddPassiveGrabToList();

extern Bool permitOldBugs;
extern int lastEvent;
#ifdef XINPUT
extern int DeviceMotionNotify, DeviceButtonPress, DeviceKeyPress;
#endif

static Mask lastEventMask;

#define CantBeFiltered NoEventMask
static Mask filters[128] =
{
	NoSuchEvent,		       /* 0 */
	NoSuchEvent,		       /* 1 */
	KeyPressMask,		       /* KeyPress */
	KeyReleaseMask,		       /* KeyRelease */
	ButtonPressMask,	       /* ButtonPress */
	ButtonReleaseMask,	       /* ButtonRelease */
	PointerMotionMask,	       /* MotionNotify (initial state) */
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
	StructureAndSubMask,	       /* ReparentNotify */
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
	CantBeFiltered,		       /* ClientMessage */
	CantBeFiltered		       /* MappingNotify */
};

static CARD8 criticalEvents[32] =
{
    0x3c				/* key and button events */
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
	FatalError("SetMaskForEvent: bogus event number");
    filters[event] = mask;
}

void
SetCriticalEvent(event)
    int event;
{
    if (event >= 128)
	FatalError("SetCriticalEvent: bogus event number");
    criticalEvents[event >> 3] |= 1 << (event & 7);
}

static void
SyntheticMotion(x, y)
    int x, y;
{
    xEvent xE;

    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;
    if (syncEvents.playingEvents)
	xE.u.keyButtonPointer.time = syncEvents.time.milliseconds;
    else
	xE.u.keyButtonPointer.time = currentTime.milliseconds;
    xE.u.u.type = MotionNotify;
    (*inputInfo.pointer->public.processInputProc)(&xE, inputInfo.pointer, 1);
}

#ifdef SHAPE
static void
ConfineToShape(shape, px, py)
    RegionPtr shape;
    int *px, *py;
{
    BoxRec box;
    int x = *px, y = *py;
    int incx = 1, incy = 1;

    if (POINT_IN_REGION(sprite.hot.pScreen, shape, x, y, &box))
	return;
    box = *REGION_EXTENTS(sprite.hot.pScreen, shape);
    /* this is rather crude */
    do {
	x += incx;
	if (x >= box.x2)
	{
	    incx = -1;
	    x = *px - 1;
	}
	else if (x < box.x1)
	{
	    incx = 1;
	    x = *px;
	    y += incy;
	    if (y >= box.y2)
	    {
		incy = -1;
		y = *py - 1;
	    }
	    else if (y < box.y1)
		return; /* should never get here! */
	}
    } while (!POINT_IN_REGION(sprite.hot.pScreen, shape, x, y, &box));
    *px = x;
    *py = y;
}
#endif

static void
CheckPhysLimits(cursor, generateEvents, confineToScreen, pScreen)
    CursorPtr cursor;
    Bool generateEvents;
    Bool confineToScreen;
    ScreenPtr pScreen;
{
    HotSpot new;

    if (!cursor)
	return;
    new = sprite.hotPhys;
    if (pScreen)
	new.pScreen = pScreen;
    else
	pScreen = new.pScreen;
    (*pScreen->CursorLimits) (pScreen, cursor, &sprite.hotLimits,
			      &sprite.physLimits);
    sprite.confined = confineToScreen;
    (* pScreen->ConstrainCursor)(pScreen, &sprite.physLimits);
    if (new.x < sprite.physLimits.x1)
	new.x = sprite.physLimits.x1;
    else
	if (new.x >= sprite.physLimits.x2)
	    new.x = sprite.physLimits.x2 - 1;
    if (new.y < sprite.physLimits.y1)
	new.y = sprite.physLimits.y1;
    else
	if (new.y >= sprite.physLimits.y2)
	    new.y = sprite.physLimits.y2 - 1;
#ifdef SHAPE
    if (sprite.hotShape)
	ConfineToShape(sprite.hotShape, &new.x, &new.y);
#endif
    if ((pScreen != sprite.hotPhys.pScreen) ||
	(new.x != sprite.hotPhys.x) || (new.y != sprite.hotPhys.y))
    {
	if (pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys = new;
	(*pScreen->SetCursorPosition) (pScreen, new.x, new.y, generateEvents);
	if (!generateEvents)
	    SyntheticMotion(new.x, new.y);
    }
}

static void
CheckVirtualMotion(qe, pWin)
    register QdEventPtr qe;
    register WindowPtr pWin;
{

    if (qe)
    {
	sprite.hot.pScreen = qe->pScreen;
	sprite.hot.x = qe->event->u.keyButtonPointer.rootX;
	sprite.hot.y = qe->event->u.keyButtonPointer.rootY;
	pWin = inputInfo.pointer->grab ? inputInfo.pointer->grab->confineTo :
					 NullWindow;
    }
    if (pWin)
    {
	BoxRec lims;

	if (sprite.hot.pScreen != pWin->drawable.pScreen)
	{
	    sprite.hot.pScreen = pWin->drawable.pScreen;
	    sprite.hot.x = sprite.hot.y = 0;
	}
	lims = *REGION_EXTENTS(pWin->drawable.pScreen, &pWin->borderSize);
	if (sprite.hot.x < lims.x1)
	    sprite.hot.x = lims.x1;
	else if (sprite.hot.x >= lims.x2)
	    sprite.hot.x = lims.x2 - 1;
	if (sprite.hot.y < lims.y1)
	    sprite.hot.y = lims.y1;
	else if (sprite.hot.y >= lims.y2)
	    sprite.hot.y = lims.y2 - 1;
#ifdef SHAPE
	if (wBoundingShape(pWin))
	    ConfineToShape(&pWin->borderSize, &sprite.hot.x, &sprite.hot.y);
#endif
	if (qe)
	{
	    qe->pScreen = sprite.hot.pScreen;
	    qe->event->u.keyButtonPointer.rootX = sprite.hot.x;
	    qe->event->u.keyButtonPointer.rootY = sprite.hot.y;
	}
    }
    ROOT = WindowTable[sprite.hot.pScreen->myNum];
}

static void
ConfineCursorToWindow(pWin, generateEvents, confineToScreen)
    WindowPtr pWin;
    Bool generateEvents;
    Bool confineToScreen;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    if (syncEvents.playingEvents)
    {
	CheckVirtualMotion((QdEventPtr)NULL, pWin);
	SyntheticMotion(sprite.hot.x, sprite.hot.y);
    }
    else
    {
	sprite.hotLimits = *REGION_EXTENTS(pScreen, &pWin->borderSize);
#ifdef SHAPE
	sprite.hotShape = wBoundingShape(pWin) ? &pWin->borderSize
					       : NullRegion;
#endif
	CheckPhysLimits(sprite.current, generateEvents, confineToScreen,
			pScreen);
    }
}

Bool
PointerConfinedToScreen()
{
    return sprite.confined;
}

static void
ChangeToCursor(cursor)
    CursorPtr cursor;
{
    if (cursor != sprite.current)
    {
	if ((sprite.current->bits->xhot != cursor->bits->xhot) ||
		(sprite.current->bits->yhot != cursor->bits->yhot))
	    CheckPhysLimits(cursor, FALSE, PointerConfinedToScreen(),
			    (ScreenPtr)NULL);
	(*sprite.hotPhys.pScreen->DisplayCursor) (sprite.hotPhys.pScreen,
						  cursor);
	sprite.current = cursor;
    }
}

/* returns true if b is a descendent of a */
Bool
IsParent(a, b)
    register WindowPtr a, b;
{
    for (b = b->parent; b; b = b->parent)
	if (b == a) return TRUE;
    return FALSE;
}

static void
PostNewCursor()
{
    register    WindowPtr win;
    register    GrabPtr grab = inputInfo.pointer->grab;

    if (syncEvents.playingEvents)
	return;
    if (grab)
    {
	if (grab->cursor)
	{
	    ChangeToCursor(grab->cursor);
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
	if (win->optional && win->optional->cursor != NullCursor)
	{
	    ChangeToCursor(win->optional->cursor);
	    return;
	}
}

WindowPtr
GetCurrentRootWindow()
{
    return ROOT;
}

WindowPtr
GetSpriteWindow()
{
    return sprite.win;
}

CursorPtr
GetSpriteCursor()
{
    return sprite.current;
}

void
GetSpritePosition(px, py)
    int *px, *py;
{
    *px = sprite.hotPhys.x;
    *py = sprite.hotPhys.y;
}

#define TIMESLOP (5 * 60 * 1000) /* 5 minutes */

static void
MonthChangedOrBadTime(xE)
    register xEvent *xE;
{
    /* If the ddx/OS is careless about not processing timestamped events from
     * different sources in sorted order, then it's possible for time to go
     * backwards when it should not.  Here we ensure a decent time.
     */
    if ((currentTime.milliseconds - xE->u.keyButtonPointer.time) > TIMESLOP)
	currentTime.months++;
    else
	xE->u.keyButtonPointer.time = currentTime.milliseconds;
}

#define NoticeTime(xE) { \
    if ((xE)->u.keyButtonPointer.time < currentTime.milliseconds) \
	MonthChangedOrBadTime(xE); \
    currentTime.milliseconds = (xE)->u.keyButtonPointer.time; \
    lastDeviceEventTime = currentTime; }

void
NoticeEventTime(xE)
    register xEvent *xE;
{
    if (!syncEvents.playingEvents)
	NoticeTime(xE);
}

/**************************************************************************
 *            The following procedures deal with synchronous events       *
 **************************************************************************/

void
EnqueueEvent(xE, device, count)
    xEvent		*xE;
    DeviceIntPtr	device;
    int			count;
{
    register QdEventPtr tail = *syncEvents.pendtail;
    register QdEventPtr qe;
    xEvent		*qxE;

    NoticeTime(xE)
    if (xE->u.u.type == MotionNotify)
    {
	sprite.hotPhys.x = xE->u.keyButtonPointer.rootX;
	sprite.hotPhys.y = xE->u.keyButtonPointer.rootY;
	/* do motion compression */
	if (tail &&
	    (tail->event->u.u.type == MotionNotify) &&
	    (tail->pScreen == sprite.hotPhys.pScreen))
	{
	    tail->event->u.keyButtonPointer.rootX = sprite.hotPhys.x;
	    tail->event->u.keyButtonPointer.rootY = sprite.hotPhys.y;
	    tail->event->u.keyButtonPointer.time = xE->u.keyButtonPointer.time;
	    tail->months = currentTime.months;
	    return;
	}
    }
    qe = (QdEventPtr)xalloc(sizeof(QdEventRec) + (count * sizeof(xEvent)));
    if (!qe)
	return;
    qe->next = (QdEventPtr)NULL;
    qe->device = device;
    qe->pScreen = sprite.hotPhys.pScreen;
    qe->months = currentTime.months;
    qe->event = (xEvent *)(qe + 1);
    qe->evcount = count;
    for (qxE = qe->event; --count >= 0; qxE++, xE++)
	*qxE = *xE;
    if (tail)
	syncEvents.pendtail = &tail->next;
    *syncEvents.pendtail = qe;
}

static void
PlayReleasedEvents()
{
    register QdEventPtr *prev, qe;
    register DeviceIntPtr dev;

    prev = &syncEvents.pending;
    while ( (qe = *prev) )
    {
	if (!qe->device->sync.frozen)
	{
	    *prev = qe->next;
	    if (*syncEvents.pendtail == *prev)
		syncEvents.pendtail = prev;
	    if (qe->event->u.u.type == MotionNotify)
		CheckVirtualMotion(qe, NullWindow);
	    syncEvents.time.months = qe->months;
	    syncEvents.time.milliseconds = qe->event->u.keyButtonPointer.time;
	    (*qe->device->public.processInputProc)(qe->event, qe->device,
						   qe->evcount);
	    xfree(qe);
	    for (dev = inputInfo.devices; dev && dev->sync.frozen; dev = dev->next)
		;
	    if (!dev)
		break;
	    /* Playing the event may have unfrozen another device. */
	    /* So to play it safe, restart at the head of the queue */
	    prev = &syncEvents.pending;
	}
	else
	    prev = &qe->next;
    } 
}

static void
FreezeThaw(dev, frozen)
    register DeviceIntPtr dev;
    Bool frozen;
{
    dev->sync.frozen = frozen;
    if (frozen)
	dev->public.processInputProc = dev->public.enqueueInputProc;
    else
	dev->public.processInputProc = dev->public.realInputProc;
}

void
ComputeFreezes()
{
    register DeviceIntPtr replayDev = syncEvents.replayDev;
    register int i;
    WindowPtr w;
    register xEvent *xE;
    int count;
    GrabPtr grab;
    register DeviceIntPtr dev;

    for (dev = inputInfo.devices; dev; dev = dev->next)
	FreezeThaw(dev, dev->sync.other || (dev->sync.state >= FROZEN));
    if (syncEvents.playingEvents || (!replayDev && !syncEvents.pending))
	return;
    syncEvents.playingEvents = TRUE;
    if (replayDev)
    {
	xE = replayDev->sync.event;
	count = replayDev->sync.evcount;
	syncEvents.replayDev = (DeviceIntPtr)NULL;
	w = XYToWindow(
	    xE->u.keyButtonPointer.rootX, xE->u.keyButtonPointer.rootY);
	for (i = 0; i < spriteTraceGood; i++)
	    if (syncEvents.replayWin == spriteTrace[i])
	    {
		if (!CheckDeviceGrabs(replayDev, xE, i+1, count))
		    if (replayDev->focus)
			DeliverFocusedEvent(replayDev, xE, w, count);
		    else
			DeliverDeviceEvents(w, xE, NullGrab, NullWindow,
					    replayDev, count);
		goto playmore;
	    }
	/* must not still be in the same stack */
	if (replayDev->focus)
	    DeliverFocusedEvent(replayDev, xE, w, count);
	else
	    DeliverDeviceEvents(w, xE, NullGrab, NullWindow, replayDev, count);
    }
playmore:
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (!dev->sync.frozen)
	{
	    PlayReleasedEvents();
	    break;
	}
    }
    syncEvents.playingEvents = FALSE;
    /* the following may have been skipped during replay, so do it now */
    if ((grab = inputInfo.pointer->grab) && grab->confineTo)
    {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, TRUE, TRUE);
    }
    else
	ConfineCursorToWindow(WindowTable[sprite.hotPhys.pScreen->myNum],
			      TRUE, FALSE);
    PostNewCursor();
}

void
CheckGrabForSyncs(thisDev, thisMode, otherMode)
    register DeviceIntPtr thisDev;
    Bool thisMode, otherMode;
{
    register GrabPtr grab = thisDev->grab;
    register DeviceIntPtr dev;

    if (thisMode == GrabModeSync)
	thisDev->sync.state = FROZEN_NO_EVENT;
    else
    {	/* free both if same client owns both */
	thisDev->sync.state = THAWED;
	if (thisDev->sync.other &&
	    (CLIENT_BITS(thisDev->sync.other->resource) ==
	     CLIENT_BITS(grab->resource)))
	    thisDev->sync.other = NullGrab;
    }
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev != thisDev)
	{
	    if (otherMode == GrabModeSync)
		dev->sync.other = grab;
	    else
	    {	/* free both if same client owns both */
		if (dev->sync.other &&
		    (CLIENT_BITS(dev->sync.other->resource) ==
		     CLIENT_BITS(grab->resource)))
		    dev->sync.other = NullGrab;
	    }
	}
    }
    ComputeFreezes();
}

void
ActivatePointerGrab(mouse, grab, time, autoGrab)
    register GrabPtr grab;
    register DeviceIntPtr mouse;
    TimeStamp time;
    Bool autoGrab;
{
    WindowPtr oldWin = (mouse->grab) ? mouse->grab->window
				     : sprite.win;

    if (grab->confineTo)
    {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, FALSE, TRUE);
    }
    DoEnterLeaveEvents(oldWin, grab->window, NotifyGrab);
    mouse->valuator->motionHintWindow = NullWindow;
    if (syncEvents.playingEvents)
	mouse->grabTime = syncEvents.time;
    else
	mouse->grabTime = time;
    if (grab->cursor)
	grab->cursor->refcnt++;
    mouse->activeGrab = *grab;
    mouse->grab = &mouse->activeGrab;
    mouse->fromPassiveGrab = autoGrab;
    PostNewCursor();
    CheckGrabForSyncs(mouse,
		      (Bool)grab->pointerMode, (Bool)grab->keyboardMode);
}

void
DeactivatePointerGrab(mouse)
    register DeviceIntPtr mouse;
{
    register GrabPtr grab = mouse->grab;
    register DeviceIntPtr dev;

    mouse->valuator->motionHintWindow = NullWindow;
    mouse->grab = NullGrab;
    mouse->sync.state = NOT_GRABBED;
    mouse->fromPassiveGrab = FALSE;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev->sync.other == grab)
	    dev->sync.other = NullGrab;
    }
    DoEnterLeaveEvents(grab->window, sprite.win, NotifyUngrab);
    if (grab->confineTo)
	ConfineCursorToWindow(ROOT, FALSE, FALSE);
    PostNewCursor();
    if (grab->cursor)
	FreeCursor(grab->cursor, (Cursor)0);
    ComputeFreezes();
}

void
ActivateKeyboardGrab(keybd, grab, time, passive)
    register DeviceIntPtr keybd;
    GrabPtr grab;
    TimeStamp time;
    Bool passive;
{
    WindowPtr oldWin;

    if (keybd->grab)
	oldWin = keybd->grab->window;
    else if (keybd->focus)
	oldWin = keybd->focus->win;
    else
	oldWin = sprite.win;
    if (oldWin == FollowKeyboardWin)
	oldWin = inputInfo.keyboard->focus->win;
    if (keybd->valuator)
	keybd->valuator->motionHintWindow = NullWindow;
    DoFocusEvents(keybd, oldWin, grab->window, NotifyGrab);
    if (syncEvents.playingEvents)
	keybd->grabTime = syncEvents.time;
    else
	keybd->grabTime = time;
    keybd->activeGrab = *grab;
    keybd->grab = &keybd->activeGrab;
    keybd->fromPassiveGrab = passive;
    CheckGrabForSyncs(keybd,
		      (Bool)grab->keyboardMode, (Bool)grab->pointerMode);
}

void
DeactivateKeyboardGrab(keybd)
    register DeviceIntPtr keybd;
{
    register GrabPtr grab = keybd->grab;
    register DeviceIntPtr dev;
    register WindowPtr focusWin = keybd->focus ? keybd->focus->win
					       : sprite.win;

    if (focusWin == FollowKeyboardWin)
	focusWin = inputInfo.keyboard->focus->win;
    if (keybd->valuator)
	keybd->valuator->motionHintWindow = NullWindow;
    keybd->grab = NullGrab;
    keybd->sync.state = NOT_GRABBED;
    keybd->fromPassiveGrab = FALSE;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev->sync.other == grab)
	    dev->sync.other = NullGrab;
    }
    DoFocusEvents(keybd, grab->window, focusWin, NotifyUngrab);
    ComputeFreezes();
}

void
AllowSome(client, time, thisDev, newState)
    ClientPtr		client;
    TimeStamp		time;
    register DeviceIntPtr thisDev;
    int			newState;
{
    Bool thisGrabbed, otherGrabbed, othersFrozen, thisSynced;
    TimeStamp grabTime;
    register DeviceIntPtr dev;

    thisGrabbed = thisDev->grab && SameClient(thisDev->grab, client);
    thisSynced = FALSE;
    otherGrabbed = FALSE;
    othersFrozen = TRUE;
    grabTime = thisDev->grabTime;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev == thisDev)
	    continue;
	if (dev->grab && SameClient(dev->grab, client))
	{
	    if (!(thisGrabbed || otherGrabbed) ||
		(CompareTimeStamps(dev->grabTime, grabTime) == LATER))
		grabTime = dev->grabTime;
	    otherGrabbed = TRUE;
	    if (thisDev->sync.other == dev->grab)
		thisSynced = TRUE;
	    if (dev->sync.state < FROZEN)
		othersFrozen = FALSE;
	}
	else if (!dev->sync.other || !SameClient(dev->sync.other, client))
	    othersFrozen = FALSE;
    }
    if (!((thisGrabbed && thisDev->sync.state >= FROZEN) || thisSynced))
	return;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	(CompareTimeStamps(time, grabTime) == EARLIER))
	return;
    switch (newState)
    {
	case THAWED:	 	       /* Async */
	    if (thisGrabbed)
		thisDev->sync.state = THAWED;
	    if (thisSynced)
		thisDev->sync.other = NullGrab;
	    ComputeFreezes();
	    break;
	case FREEZE_NEXT_EVENT:		/* Sync */
	    if (thisGrabbed)
	    {
		thisDev->sync.state = FREEZE_NEXT_EVENT;
		if (thisSynced)
		    thisDev->sync.other = NullGrab;
		ComputeFreezes();
	    }
	    break;
	case THAWED_BOTH:		/* AsyncBoth */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = THAWED;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
	case FREEZE_BOTH_NEXT_EVENT:	/* SyncBoth */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = FREEZE_BOTH_NEXT_EVENT;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
	case NOT_GRABBED:		/* Replay */
	    if (thisGrabbed && thisDev->sync.state == FROZEN_WITH_EVENT)
	    {
		if (thisSynced)
		    thisDev->sync.other = NullGrab;
		syncEvents.replayDev = thisDev;
		syncEvents.replayWin = thisDev->grab->window;
		(*thisDev->DeactivateGrab)(thisDev);
		syncEvents.replayDev = (DeviceIntPtr)NULL;
	    }
	    break;
	case THAW_OTHERS:		/* AsyncOthers */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev == thisDev)
			continue;
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = THAWED;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
    }
}

int
ProcAllowEvents(client)
    register ClientPtr client;
{
    TimeStamp		time;
    DeviceIntPtr mouse;
    DeviceIntPtr keybd;

    REQUEST(xAllowEventsReq);

    REQUEST_SIZE_MATCH(xAllowEventsReq);
    MTXLockDevicesAndPOQ(client, CM_XAllowEvents);
    mouse = inputInfo.pointer;
    keybd = inputInfo.keyboard;

    time = ClientTimeToServerTime(stuff->time);
    switch (stuff->mode)
    {
	case ReplayPointer:
	    AllowSome(client, time, mouse, NOT_GRABBED);
	    break;
	case SyncPointer: 
	    AllowSome(client, time, mouse, FREEZE_NEXT_EVENT);
	    break;
	case AsyncPointer: 
	    AllowSome(client, time, mouse, THAWED);
	    break;
	case ReplayKeyboard: 
	    AllowSome(client, time, keybd, NOT_GRABBED);
	    break;
	case SyncKeyboard: 
	    AllowSome(client, time, keybd, FREEZE_NEXT_EVENT);
	    break;
	case AsyncKeyboard: 
	    AllowSome(client, time, keybd, THAWED);
	    break;
	case SyncBoth:
	    AllowSome(client, time, keybd, FREEZE_BOTH_NEXT_EVENT);
	    break;
	case AsyncBoth:
	    AllowSome(client, time, keybd, THAWED_BOTH);
	    break;
	default: 
            MTXUnlockDevicesAndPOQ(client);
	    client->errorValue = stuff->mode;
	    return BadValue;
    }

    MTXUnlockDevicesAndPOQ(client);
    return Success;
}

void
ReleaseActiveGrabs(client)
    ClientPtr client;
{
    register DeviceIntPtr dev;
    Bool    done;

    /* XXX CloseDownClient should remove passive grabs before
     * releasing active grabs.
     */

    MTX_LOCK_DEVICES();
    do {
    	done = TRUE;
    	for (dev = inputInfo.devices; dev; dev = dev->next)
    	{
	    if (dev->grab && SameClient(dev->grab, client))
	    {
	    	(*dev->DeactivateGrab)(dev);
	    	done = FALSE;
	    }
    	}
    } while (!done);
    MTX_UNLOCK_DEVICES();
}

/**************************************************************************
 *            The following procedures deal with delivering events        *
 **************************************************************************/

/* MTX note:
 *	In almost cases in this file after the definition, MTX version
 *	should call LockedTryClientEvents instead of TryClientEvents
 *	since the devices are already locked.
 *	Similarly, LockedClientEvents is a static
 */

#ifdef XTHREADS
static int
LockedTryClientEvents(client, pEvents, count, mask, filter, grab)
#else /* XTHREADS */
#define LockedTryClientEvents TryClientEvents
int
TryClientEvents(client, pEvents, count, mask, filter, grab)
#endif /* XTHREADS */
    ClientPtr client;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask mask, filter;
{
    int i;
    int type;

#ifdef DEBUG
    if (debug_events) ErrorF(
	"Event([%d, %d], mask=0x%x), client=%d",
	pEvents->u.u.type, pEvents->u.u.detail, mask, client->index);
#endif
    if ((client) && (client != serverClient) && (!client->clientGone) &&
	((filter == CantBeFiltered) || (mask & filter)))
    {
	if (grab && !SameClient(grab, client))
	    return -1; /* don't send, but notify caller */
	type = pEvents->u.u.type;
	if (type == MotionNotify)
	{
	    if (mask & PointerMotionHintMask)
	    {
		if (WID(inputInfo.pointer->valuator->motionHintWindow) ==
		    pEvents->u.keyButtonPointer.event)
		{
#ifdef DEBUG
		    if (debug_events) ErrorF("\n");
#endif
		    return 1; /* don't send, but pretend we did */
		}
		pEvents->u.u.detail = NotifyHint;
	    }
	    else
	    {
		pEvents->u.u.detail = NotifyNormal;
	    }
	}
#ifdef XINPUT
	else
	{
	    /* MTX note: MTX used pXie in this section - why? */
	    if ((type == DeviceMotionNotify) &&
		MaybeSendDeviceMotionNotifyHint (pEvents, mask) != 0)
		return 1;
	}
#endif
	type &= 0177;
	if (type != KeymapNotify)
	{
	    /* all extension events must have a sequence number */
	    for (i = 0; i < count; i++)
		pEvents[i].u.u.sequenceNumber = client->sequence;
	}

#ifndef XTHREADS
	if (BitIsOn(criticalEvents, type))
	    SetCriticalOutputPending();
#endif /* not XTHREADS */

	WriteEventsToClient(client, count, pEvents);
#ifdef DEBUG
	if (debug_events) ErrorF(  " delivered\n");
#endif
	return 1;
    }
    else
    {
#ifdef DEBUG
	if (debug_events) ErrorF("\n");
#endif
	return 0;
    }
}

#ifdef XTHREADS
int
TryClientEvents(client, pEvents, count, mask, filter, grab)
    ClientPtr client;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask mask, filter;
{
    int ret;

    MTX_LOCK_DEVICES();
    ret = LockedTryClientEvents(client, pEvents, count, mask, filter, grab);
    MTX_UNLOCK_DEVICES();
    return ret;
}

/* #define TryClientEvents LockedTryClientEvents */
#endif /* XTHREADS */



int
DeliverEventsToWindow(pWin, pEvents, count, filter, grab, mskidx)
    register WindowPtr pWin;
    GrabPtr grab;
    xEvent *pEvents;
    int count;
    Mask filter;
    int mskidx;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    register InputClients *other;
    ClientPtr client = NullClient;
    Mask deliveryMask; 	/* If a grab occurs due to a button press, then
		              this mask is the mask of the grab. */
    int type = pEvents->u.u.type;

    /* CantBeFiltered means only window owner gets the event */
    if ((filter == CantBeFiltered) || !(type & EXTENSION_EVENT_BASE))
    {
	/* if nobody ever wants to see this event, skip some work */
	if (filter != CantBeFiltered &&
	    !((wOtherEventMasks(pWin)|pWin->eventMask) & filter))
	    return 0;
	if (attempt = LockedTryClientEvents(wClient(pWin), pEvents, count,
				      pWin->eventMask, filter, grab))
	{
	    if (attempt > 0)
	    {
		deliveries++;
		client = wClient(pWin);
		deliveryMask = pWin->eventMask;
	    } else
		nondeliveries--;
	}
    }
    if (filter != CantBeFiltered)
    {
	if (type & EXTENSION_EVENT_BASE)
	{
	    OtherInputMasks *inputMasks;

	    inputMasks = wOtherInputMasks(pWin);
	    if (!inputMasks ||
		!(inputMasks->inputEvents[mskidx] & filter))
		return 0;
	    other = inputMasks->inputClients;
	}
	else
	    other = (InputClients *)wOtherClients(pWin);
	for (; other; other = other->next)
	{
	    if (attempt = LockedTryClientEvents(rClient(other), pEvents, count,
					  other->mask[mskidx], filter, grab))
	    {
		if (attempt > 0)
		{
		    deliveries++;
		    client = rClient(other);
		    deliveryMask = other->mask[mskidx];
		} else
		    nondeliveries--;
	    }
	}
    }
    if ((type == ButtonPress) && deliveries && (!grab))
    {
	GrabRec tempGrab;

	tempGrab.device = inputInfo.pointer;
	tempGrab.resource = client->clientAsMask;
	tempGrab.window = pWin;
	tempGrab.ownerEvents = (deliveryMask & OwnerGrabButtonMask) ? TRUE : FALSE;
	tempGrab.eventMask = deliveryMask;
	tempGrab.keyboardMode = GrabModeAsync;
	tempGrab.pointerMode = GrabModeAsync;
	tempGrab.confineTo = NullWindow;
	tempGrab.cursor = NullCursor;
	(*inputInfo.pointer->ActivateGrab)(inputInfo.pointer, &tempGrab,
					   currentTime, TRUE);
    }
    else if ((type == MotionNotify) && deliveries)
	inputInfo.pointer->valuator->motionHintWindow = pWin;
#ifdef XINPUT
    else
    {
	/* MTX note: MTX used pXie in this section - why? */
	if (((type == DeviceMotionNotify) || (type == DeviceButtonPress)) &&
	    deliveries)
	    CheckDeviceGrabAndHintWindow (pWin, type, pEvents, grab, client, 
					  deliveryMask);
    }
#endif
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/* If the event goes to dontClient, don't send it and return 0.  if
   send works,  return 1 or if send didn't work, return 2.
   Only works for core events.
*/

int
MaybeDeliverEventsToClient(pWin, pEvents, count, filter, dontClient)
    register WindowPtr pWin;
    xEvent *pEvents;
    int count;
    Mask filter;
    ClientPtr dontClient;
{
    register OtherClients *other;

    if (pWin->eventMask & filter)
    {
        if (wClient(pWin) == dontClient)
	    return 0;
	return TryClientEvents(wClient(pWin), pEvents, count,
			       pWin->eventMask, filter, NullGrab);
    }
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (other->mask & filter)
	{
            if (SameClient(other, dontClient))
		return 0;
	    return TryClientEvents(rClient(other), pEvents, count,
				   other->mask, filter, NullGrab);
	}
    }
    return 2;
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
		child = w->drawable.id;
		break;
            }
 	    w = w->parent;
        } 	    
    }
    xE->u.keyButtonPointer.root = ROOT->drawable.id;
    xE->u.keyButtonPointer.event = pWin->drawable.id;
    if (sprite.hot.pScreen == pWin->drawable.pScreen)
    {
	xE->u.keyButtonPointer.sameScreen = xTrue;
	xE->u.keyButtonPointer.child = child;
	xE->u.keyButtonPointer.eventX =
	    xE->u.keyButtonPointer.rootX - pWin->drawable.x;
	xE->u.keyButtonPointer.eventY =
	    xE->u.keyButtonPointer.rootY - pWin->drawable.y;
    }
    else
    {
	xE->u.keyButtonPointer.sameScreen = xFalse;
	xE->u.keyButtonPointer.child = None;
	xE->u.keyButtonPointer.eventX = 0;
	xE->u.keyButtonPointer.eventY = 0;
    }
}

int
DeliverDeviceEvents(pWin, xE, grab, stopAt, dev, count)
    register WindowPtr pWin, stopAt;
    register xEvent *xE;
    GrabPtr grab;
    DeviceIntPtr dev;
    int count;
{
    Window child = None;
    int type = xE->u.u.type;
    Mask filter = filters[type];
    int deliveries = 0;

    if (type & EXTENSION_EVENT_BASE)
    {
	register OtherInputMasks *inputMasks;
	int mskidx = dev->id;

	inputMasks = wOtherInputMasks(pWin);
	if (inputMasks && !(filter & inputMasks->deliverableEvents[mskidx]))
	    return 0;
	while (pWin)
	{
	    if (inputMasks && (inputMasks->inputEvents[mskidx] & filter))
	    {
		FixUpEventFromWindow(xE, pWin, child, FALSE);
		deliveries = DeliverEventsToWindow(pWin, xE, count, filter,
						   grab, mskidx);
		if (deliveries > 0)
		    return deliveries;
	    }
	    if ((deliveries < 0) ||
		(pWin == stopAt) ||
		(inputMasks &&
		 (filter & inputMasks->dontPropagateMask[mskidx])))
		return 0;
	    child = pWin->drawable.id;
	    pWin = pWin->parent;
	    if (pWin)
		inputMasks = wOtherInputMasks(pWin);
	}
    }
    else
    {
	if (!(filter & pWin->deliverableEvents))
	    return 0;
	while (pWin)
	{
	    if ((wOtherEventMasks(pWin)|pWin->eventMask) & filter)
	    {
		FixUpEventFromWindow(xE, pWin, child, FALSE);
		deliveries = DeliverEventsToWindow(pWin, xE, count, filter,
						   grab, 0);
		if (deliveries > 0)
		    return deliveries;
	    }
	    if ((deliveries < 0) ||
		(pWin == stopAt) ||
		(filter & wDontPropagateMask(pWin)))
		return 0;
	    child = pWin->drawable.id;
	    pWin = pWin->parent;
	}
    }
    return 0;
}

/* not useful for events that propagate up the tree or extension events */
int
DeliverEvents(pWin, xE, count, otherParent)
    register WindowPtr pWin, otherParent;
    register xEvent *xE;
    int count;
{
    Mask filter;
    int     deliveries;

    if (!count)
	return 0;

    MTX_LOCK_DEVICES();
    filter = filters[xE->u.u.type];
    if ((filter & SubstructureNotifyMask) && (xE->u.u.type != CreateNotify))
	xE->u.destroyNotify.event = pWin->drawable.id;
    if (filter != StructureAndSubMask)
    {
	MTX_UNLOCK_DEVICES();
	return DeliverEventsToWindow(pWin, xE, count, filter, NullGrab, 0);
    }

    deliveries = DeliverEventsToWindow(pWin, xE, count, StructureNotifyMask,
				       NullGrab, 0);
    if (pWin->parent)
    {
	xE->u.destroyNotify.event = pWin->parent->drawable.id;
	deliveries += DeliverEventsToWindow(pWin->parent, xE, count,
					    SubstructureNotifyMask, NullGrab,
					    0);
	if (xE->u.u.type == ReparentNotify)
	{
	    xE->u.destroyNotify.event = otherParent->drawable.id;
	    deliveries += DeliverEventsToWindow(otherParent, xE, count,
						SubstructureNotifyMask,
						NullGrab, 0);
	}
    }

    MTX_UNLOCK_DEVICES();
    return deliveries;
}

static WindowPtr 
XYToWindow(x, y)
	int x, y;
{
    register WindowPtr  pWin;
#ifdef SHAPE
    BoxRec		box;
#endif

    spriteTraceGood = 1;	/* root window still there */
    pWin = ROOT->firstChild;
    while (pWin)
    {
	if ((pWin->mapped) &&
		(x >= pWin->drawable.x - wBorderWidth (pWin)) &&
		(x < pWin->drawable.x + (int)pWin->drawable.width +
		    wBorderWidth(pWin)) &&
		(y >= pWin->drawable.y - wBorderWidth (pWin)) &&
		(y < pWin->drawable.y + (int)pWin->drawable.height +
		    wBorderWidth (pWin))
#ifdef SHAPE
		/* When a window is shaped, a further check
		 * is made to see if the point is inside
		 * borderSize
		 */
		&& (!wBoundingShape(pWin) ||
		    POINT_IN_REGION(pWin->drawable.pScreen, 
			    &pWin->borderSize, x, y, &box))
#endif
		)
	{
	    if (spriteTraceGood >= spriteTraceSize)
	    {
		spriteTraceSize += 10;
		spriteTrace = (WindowPtr *)xnfrealloc
		    (spriteTrace, spriteTraceSize*sizeof(WindowPtr));
	    }
	    spriteTrace[spriteTraceGood++] = pWin;
	    pWin = pWin->firstChild;
	}
	else
	    pWin = pWin->nextSib;
    }
    return spriteTrace[spriteTraceGood-1];
}

static Bool
CheckMotion(xE)
    xEvent *xE;
{
    WindowPtr prevSpriteWin = sprite.win;

    if (xE && !syncEvents.playingEvents)
    {
	if (sprite.hot.pScreen != sprite.hotPhys.pScreen)
	{
	    sprite.hot.pScreen = sprite.hotPhys.pScreen;
	    ROOT = WindowTable[sprite.hot.pScreen->myNum];
	}
	sprite.hot.x = xE->u.keyButtonPointer.rootX;
	sprite.hot.y = xE->u.keyButtonPointer.rootY;
	if (sprite.hot.x < sprite.physLimits.x1)
	    sprite.hot.x = sprite.physLimits.x1;
	else if (sprite.hot.x >= sprite.physLimits.x2)
	    sprite.hot.x = sprite.physLimits.x2 - 1;
	if (sprite.hot.y < sprite.physLimits.y1)
	    sprite.hot.y = sprite.physLimits.y1;
	else if (sprite.hot.y >= sprite.physLimits.y2)
	    sprite.hot.y = sprite.physLimits.y2 - 1;
#ifdef SHAPE
	if (sprite.hotShape)
	    ConfineToShape(sprite.hotShape, &sprite.hot.x, &sprite.hot.y);
#endif
	sprite.hotPhys = sprite.hot;
	if ((sprite.hotPhys.x != xE->u.keyButtonPointer.rootX) ||
	    (sprite.hotPhys.y != xE->u.keyButtonPointer.rootY))
	    (*sprite.hotPhys.pScreen->SetCursorPosition)(
		sprite.hotPhys.pScreen,
		sprite.hotPhys.x, sprite.hotPhys.y, FALSE);
	xE->u.keyButtonPointer.rootX = sprite.hot.x;
	xE->u.keyButtonPointer.rootY = sprite.hot.y;
    }

    sprite.win = XYToWindow(sprite.hot.x, sprite.hot.y);
#ifdef notyet
    if (!(sprite.win->deliverableEvents &
	  Motion_Filter(inputInfo.pointer->button))
	!syncEvents.playingEvents)
    {
	/* XXX Do PointerNonInterestBox here */
    }
#endif
    if (sprite.win != prevSpriteWin)
    {
	if (prevSpriteWin != NullWindow) {
	    if (!xE)
#ifndef XTHREADS
		UpdateCurrentTimeIf();
#else /* XTHREADS */
		UpdateCurrentTime(); 
#endif /* XTHREADS */
	    DoEnterLeaveEvents(prevSpriteWin, sprite.win, NotifyNormal);
	}
	PostNewCursor();
        return FALSE;
    }
    return TRUE;
}

void
WindowsRestructured()
{
    MTX_LOCK_DEVICES();
    (void) CheckMotion((xEvent *)NULL);
    MTX_UNLOCK_DEVICES();
}

void
DefineInitialRootWindow(win)
    register WindowPtr win;
{
    register ScreenPtr pScreen = win->drawable.pScreen;

    sprite.hotPhys.pScreen = pScreen;
    sprite.hotPhys.x = pScreen->width / 2;
    sprite.hotPhys.y = pScreen->height / 2;
    sprite.hot = sprite.hotPhys;
    sprite.hotLimits.x2 = pScreen->width;
    sprite.hotLimits.y2 = pScreen->height;
    sprite.win = win;
    sprite.current = wCursor (win);
    spriteTraceGood = 1;
    ROOT = win;
    (*pScreen->CursorLimits) (
	pScreen, sprite.current, &sprite.hotLimits, &sprite.physLimits);
    sprite.confined = FALSE;
    (*pScreen->ConstrainCursor) (pScreen, &sprite.physLimits);
    (*pScreen->SetCursorPosition) (pScreen, sprite.hot.x, sprite.hot.y, FALSE);
    (*pScreen->DisplayCursor) (pScreen, sprite.current);
}

/*
 * This does not take any shortcuts, and even ignores its argument, since
 * it does not happen very often, and one has to walk up the tree since
 * this might be a newly instantiated cursor for an intermediate window
 * between the one the pointer is in and the one that the last cursor was
 * instantiated from.
 */
/*ARGSUSED*/
void
WindowHasNewCursor(pWin)
    WindowPtr pWin;
{
    MTX_LOCK_DEVICES();
    PostNewCursor();
    MTX_UNLOCK_DEVICES();
}

void
NewCurrentScreen(newScreen, x, y)
    ScreenPtr newScreen;
    int x,y;
{
    sprite.hotPhys.x = x;
    sprite.hotPhys.y = y;
    if (newScreen != sprite.hotPhys.pScreen)
	ConfineCursorToWindow(WindowTable[newScreen->myNum], TRUE, FALSE);
}

int
ProcWarpPointer(client)
    ClientPtr client;
{
    WindowPtr	source = NULL, dest = NULL;
    int		x, y;
    ScreenPtr	newScreen;

    REQUEST(xWarpPointerReq);

    REQUEST_SIZE_MATCH(xWarpPointerReq);
    MTX_LOCK_AND_VERIFY_ALL_WINDOWS(source, dest, stuff->srcWid,
	    stuff->dstWid, client, POQ_BORDER_SIZE, CM_XWarpPointer);
    MTX_LOCK_DEVICES();
    x = sprite.hotPhys.x;
    y = sprite.hotPhys.y;

    if (stuff->srcWid != None)
    {
	int winX = source->drawable.x;
	int winY = source->drawable.y;

	if (source->drawable.pScreen != sprite.hotPhys.pScreen ||
	    (x < winX + stuff->srcX) || (y < winY + stuff->srcY) ||
	    (stuff->srcWidth != 0 &&
	     winX + stuff->srcX + (int)stuff->srcWidth < x) ||
	    (stuff->srcHeight != 0 &&
	     winY + stuff->srcY + (int)stuff->srcHeight < y) ||
	    !PointInWindowIsVisible(source, x, y))
	{
	    MTX_UNLOCK_DEVICES();
	    MTX_UNLOCK_ALL_WINDOWS(source, dest, stuff->srcWid,
		    stuff->dstWid, client);
	    return (Success);
	}
    }
    if (dest)
    {
	x = dest->drawable.x;
	y = dest->drawable.y;
	newScreen = dest->drawable.pScreen;
    }
    else
	newScreen = sprite.hotPhys.pScreen;
    x += stuff->dstX;
    y += stuff->dstY;
    if (x < 0)
	x = 0;
    else if (x >= newScreen->width)
	x = newScreen->width - 1;
    if (y < 0)
	y = 0;
    else if (y >= newScreen->height)
	y = newScreen->height - 1;

    if (newScreen == sprite.hotPhys.pScreen)
    {
	if (x < sprite.physLimits.x1)
	    x = sprite.physLimits.x1;
	else if (x >= sprite.physLimits.x2)
	    x = sprite.physLimits.x2 - 1;
	if (y < sprite.physLimits.y1)
	    y = sprite.physLimits.y1;
	else if (y >= sprite.physLimits.y2)
	    y = sprite.physLimits.y2 - 1;
#ifdef SHAPE
	if (sprite.hotShape)
	    ConfineToShape(sprite.hotShape, &x, &y);
#endif
	(*newScreen->SetCursorPosition)(newScreen, x, y, TRUE);
    }
    else if (!PointerConfinedToScreen())
    {
	NewCurrentScreen(newScreen, x, y);
    }

    MTX_UNLOCK_DEVICES();
    MTX_UNLOCK_ALL_WINDOWS(source, dest, stuff->srcWid, stuff->dstWid, client);
    return Success;
}

/* "CheckPassiveGrabsOnWindow" checks to see if the event passed in causes a
	passive grab set on the window to be activated. */

static Bool
CheckPassiveGrabsOnWindow(pWin, device, xE, count)
    WindowPtr pWin;
    register DeviceIntPtr device;
    register xEvent *xE;
    int count;
{
    register GrabPtr grab = wPassiveGrabs(pWin);
    GrabRec tempGrab;
    register xEvent *dxE;

    if (!grab)
	return FALSE;
    tempGrab.window = pWin;
    tempGrab.device = device;
    tempGrab.type = xE->u.u.type;
    tempGrab.detail.exact = xE->u.u.detail;
    tempGrab.detail.pMask = NULL;
    tempGrab.modifiersDetail.pMask = NULL;
    for (; grab; grab = grab->next)
    {
	tempGrab.modifierDevice = grab->modifierDevice;
	if (device == grab->modifierDevice &&
	    (xE->u.u.type == KeyPress
#ifdef XINPUT
	     || xE->u.u.type == DeviceKeyPress
#endif
	     ))
	    tempGrab.modifiersDetail.exact =
		grab->modifierDevice->key->prev_state;
	else
	    tempGrab.modifiersDetail.exact =
#ifdef XKB
		grab->modifierDevice->key->xkbInfo->grabState;
#else
		grab->modifierDevice->key->state;
#endif
	if (GrabMatchesSecond(&tempGrab, grab) &&
	    (!grab->confineTo ||
	     (grab->confineTo->realized &&
	      REGION_NOTEMPTY(grab->confineTo->drawable.pScreen, 
		&grab->confineTo->borderSize))))
	{
#ifdef XKB
	    xE->u.keyButtonPointer.state &= 0x1f00;
	    xE->u.keyButtonPointer.state |=
		grab->modifierDevice->key->xkbInfo->grabState & 0xe0ff;
#endif
	    (*device->ActivateGrab)(device, grab, currentTime, TRUE);
 
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);

	    (void) LockedTryClientEvents(rClient(grab), xE, count, filters[xE->u.u.type],
				   filters[xE->u.u.type],  grab);

	    if (device->sync.state == FROZEN_NO_EVENT)
	    {
		if (device->sync.evcount < count)
		{
		    device->sync.event = (xEvent *)xnfrealloc
			(device->sync.event, count*sizeof(xEvent));
		}
		device->sync.evcount = count;
		for (dxE = device->sync.event; --count >= 0; dxE++, xE++)
		    *dxE = *xE;
	    	device->sync.state = FROZEN_WITH_EVENT;
            }	
	    return TRUE;
	}
    }
    return FALSE;
}

/*
"CheckDeviceGrabs" handles both keyboard and pointer events that may cause
a passive grab to be activated.  If the event is a keyboard event, the
ancestors of the focus window are traced down and tried to see if they have
any passive grabs to be activated.  If the focus window itself is reached and
it's descendants contain they pointer, the ancestors of the window that the
pointer is in are then traced down starting at the focus window, otherwise no
grabs are activated.  If the event is a pointer event, the ancestors of the
window that the pointer is in are traced down starting at the root until
CheckPassiveGrabs causes a passive grab to activate or all the windows are
tried. PRH
*/

Bool
CheckDeviceGrabs(device, xE, checkFirst, count)
    register DeviceIntPtr device;
    register xEvent *xE;
    int checkFirst;
    int count;
{
    register int i;
    register WindowPtr pWin;
    register FocusClassPtr focus = device->focus;

    if ((xE->u.u.type == ButtonPress
#ifdef XINPUT
	 || xE->u.u.type == DeviceButtonPress
#endif
	 ) && device->button->buttonsDown != 1)
	return FALSE;

    i = checkFirst;

    if (focus)
    {
	for (; i < focus->traceGood; i++)
	{
	    pWin = focus->trace[i];
	    if (pWin->optional &&
		CheckPassiveGrabsOnWindow(pWin, device, xE, count))
		return TRUE;
	}
  
	if ((focus->win == NoneWin) ||
	    (i >= spriteTraceGood) ||
	    ((i > checkFirst) && (pWin != spriteTrace[i-1])))
	    return FALSE;
    }
        
    for (; i < spriteTraceGood; i++)
    {
	pWin = spriteTrace[i];
	if (pWin->optional &&
	    CheckPassiveGrabsOnWindow(pWin, device, xE, count))
	    return TRUE;
    }

    return FALSE;
}

void
DeliverFocusedEvent(keybd, xE, window, count)
    xEvent *xE;
    DeviceIntPtr keybd;
    WindowPtr window;
    int count;
{
    WindowPtr focus = keybd->focus->win;
    int mskidx = 0;

    if (focus == FollowKeyboardWin)
	focus = inputInfo.keyboard->focus->win;
    if (!focus)
	return;
    if (focus == PointerRootWin)
    {
	DeliverDeviceEvents(window, xE, NullGrab, NullWindow, keybd, count);
	return;
    }
    if ((focus == window) || IsParent(focus, window))
    {
	if (DeliverDeviceEvents(window, xE, NullGrab, focus, keybd, count))
	    return;
    }
    /* just deliver it to the focus window */
    FixUpEventFromWindow(xE, focus, None, FALSE);
    if (xE->u.u.type & EXTENSION_EVENT_BASE)
	mskidx = keybd->id;
    (void)DeliverEventsToWindow(focus, xE, count, filters[xE->u.u.type],
				NullGrab, mskidx);
}

void
DeliverGrabbedEvent(xE, thisDev, deactivateGrab, count)
    register xEvent *xE;
    register DeviceIntPtr thisDev;
    Bool deactivateGrab;
    int count;
{
    register GrabPtr grab = thisDev->grab;
    int deliveries = 0;
    register DeviceIntPtr dev;
    register xEvent *dxE;

    if (grab->ownerEvents)
    {
	WindowPtr focus;

	if (thisDev->focus)
	{
	    focus = thisDev->focus->win;
	    if (focus == FollowKeyboardWin)
		focus = inputInfo.keyboard->focus->win;
	}
	else
	    focus = PointerRootWin;
	if (focus == PointerRootWin)
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, NullWindow,
					     thisDev, count);
	else if (focus && (focus == sprite.win || IsParent(focus, sprite.win)))
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, focus,
					     thisDev, count);
	else if (focus)
	    deliveries = DeliverDeviceEvents(focus, xE, grab, focus,
					     thisDev, count);
    }
    if (!deliveries)
    {
	FixUpEventFromWindow(xE, grab->window, None, TRUE);
	deliveries = LockedTryClientEvents(rClient(grab), xE, count, (Mask)grab->eventMask,
				     filters[xE->u.u.type], grab);
	if (deliveries && (xE->u.u.type == MotionNotify
#ifdef XINPUT
			   || xE->u.u.type == DeviceMotionNotify
#endif
			   ))
	    thisDev->valuator->motionHintWindow = grab->window;
    }
    if (deliveries && !deactivateGrab && (xE->u.u.type != MotionNotify
#ifdef XINPUT
					  && xE->u.u.type != DeviceMotionNotify
#endif
					  ))
	switch (thisDev->sync.state)
	{
	case FREEZE_BOTH_NEXT_EVENT:
	    for (dev = inputInfo.devices; dev; dev = dev->next)
	    {
		if (dev == thisDev)
		    continue;
		FreezeThaw(dev, TRUE);
		if ((dev->sync.state == FREEZE_BOTH_NEXT_EVENT) &&
		    (CLIENT_BITS(dev->grab->resource) ==
		     CLIENT_BITS(thisDev->grab->resource)))
		    dev->sync.state = FROZEN_NO_EVENT;
		else
		    dev->sync.other = thisDev->grab;
	    }
	    /* fall through */
	case FREEZE_NEXT_EVENT:
	    thisDev->sync.state = FROZEN_WITH_EVENT;
	    FreezeThaw(thisDev, TRUE);
	    if (thisDev->sync.evcount < count)
	    {
		thisDev->sync.event = (xEvent *)xnfrealloc
		    (thisDev->sync.event, count*sizeof(xEvent));
	    }
	    thisDev->sync.evcount = count;
	    for (dxE = thisDev->sync.event; --count >= 0; dxE++, xE++)
		*dxE = *xE;
	    break;
	}
}

#ifdef XTHREADS
void
LockDeviceAndProcessInputEvent(xE, device, count)
    register xEvent *xE;
    DevicePtr device;
    int count;
{
    MTX_LOCK_DEVICES();
    (*device->processInputProc)(xE, device, count);
    MTX_UNLOCK_DEVICES();
}
#endif /* XTHREADS */

void
#ifdef XKB
CoreProcessKeyboardEvent (xE, keybd, count)
#else
ProcessKeyboardEvent (xE, keybd, count)
#endif
    register xEvent *xE;
    register DeviceIntPtr keybd;
    int count;
{
    int             key, bit;
    register BYTE   *kptr;
    register int    i;
    register CARD8  modifiers;
    register CARD16 mask;
    GrabPtr         grab = keybd->grab;
    Bool            deactivateGrab = FALSE;
    register KeyClassPtr keyc = keybd->key;

    if (!syncEvents.playingEvents)
	NoticeTime(xE)
    xE->u.keyButtonPointer.state = (keyc->state |
				    inputInfo.pointer->button->state);
    xE->u.keyButtonPointer.rootX = sprite.hot.x;
    xE->u.keyButtonPointer.rootY = sprite.hot.y;
    key = xE->u.u.detail;
    kptr = &keyc->down[key >> 3];
    bit = 1 << (key & 7);
    modifiers = keyc->modifierMap[key];
    switch (xE->u.u.type)
    {
	case KeyPress: 
	    if (*kptr & bit) /* allow ddx to generate multiple downs */
	    {   
		if (!modifiers)
		{
		    xE->u.u.type = KeyRelease;
		    ProcessKeyboardEvent(xE, keybd, count);
		    xE->u.u.type = KeyPress;
		    /* release can have side effects, don't fall through */
		    ProcessKeyboardEvent(xE, keybd, count);
		}
		return;
	    }
	    inputInfo.pointer->valuator->motionHintWindow = NullWindow;
	    *kptr |= bit;
	    keyc->prev_state = keyc->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers)
		{
		    /* This key affects modifier "i" */
		    keyc->modifierKeyCount[i]++;
		    keyc->state |= mask;
		    modifiers &= ~mask;
		}
	    }
	    if (!grab && CheckDeviceGrabs(keybd, xE, 0, count))
	    {
		keybd->activatingKey = key;
		return;
	    }
	    break;
	case KeyRelease: 
	    if (!(*kptr & bit)) /* guard against duplicates */
		return;
	    inputInfo.pointer->valuator->motionHintWindow = NullWindow;
	    *kptr &= ~bit;
	    keyc->prev_state = keyc->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers) {
		    /* This key affects modifier "i" */
		    if (--keyc->modifierKeyCount[i] <= 0) {
			keyc->state &= ~mask;
			keyc->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	    }
	    if (keybd->fromPassiveGrab && (key == keybd->activatingKey))
		deactivateGrab = TRUE;
	    break;
	default: 
	    FatalError("Impossible keyboard event");
    }
    if (grab)
	DeliverGrabbedEvent(xE, keybd, deactivateGrab, count);
    else
	DeliverFocusedEvent(keybd, xE, sprite.win, count);
    if (deactivateGrab)
        (*keybd->DeactivateGrab)(keybd);
}

void
#ifdef XKB
CoreProcessPointerEvent (xE, mouse, count)
#else
ProcessPointerEvent (xE, mouse, count)
#endif
    register xEvent 		*xE;
    register DeviceIntPtr 	mouse;
    int				count;
{
    register GrabPtr	grab = mouse->grab;
    Bool                deactivateGrab = FALSE;
    register ButtonClassPtr butc = mouse->button;

    if (!syncEvents.playingEvents)
	NoticeTime(xE)
    xE->u.keyButtonPointer.state = (butc->state |
#ifdef XKB
				    inputInfo.keyboard->key->xkbInfo->grabState
#else
				    inputInfo.keyboard->key->state
#endif
				    );
    if (xE->u.u.type != MotionNotify)
    {
	register int  key;
	register BYTE *kptr;
	int           bit;

	xE->u.keyButtonPointer.rootX = sprite.hot.x;
	xE->u.keyButtonPointer.rootY = sprite.hot.y;
	key = xE->u.u.detail;
	kptr = &butc->down[key >> 3];
	bit = 1 << (key & 7);
	switch (xE->u.u.type)
	{
	case ButtonPress: 
	    mouse->valuator->motionHintWindow = NullWindow;
	    butc->buttonsDown++;
	    butc->motionMask = ButtonMotionMask;
	    *kptr |= bit;
	    xE->u.u.detail = butc->map[key];
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		butc->state |= (Button1Mask >> 1) << xE->u.u.detail;
	    filters[MotionNotify] = Motion_Filter(butc);
	    if (!grab)
		if (CheckDeviceGrabs(mouse, xE, 0, count))
		    return;
	    break;
	case ButtonRelease: 
	    mouse->valuator->motionHintWindow = NullWindow;
	    if (!--butc->buttonsDown)
		butc->motionMask = 0;
	    *kptr &= ~bit;
	    xE->u.u.detail = butc->map[key];
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		butc->state &= ~((Button1Mask >> 1) << xE->u.u.detail);
	    filters[MotionNotify] = Motion_Filter(butc);
	    if (!butc->state && mouse->fromPassiveGrab)
		deactivateGrab = TRUE;
	    break;
	default: 
	    FatalError("bogus pointer event from ddx");
	}
    }
    else if (!CheckMotion(xE))
	return;
    if (grab)
	DeliverGrabbedEvent(xE, mouse, deactivateGrab, count);
    else
	DeliverDeviceEvents(sprite.win, xE, NullGrab, NullWindow,
			    mouse, count);
    if (deactivateGrab)
        (*mouse->DeactivateGrab)(mouse);
}

#define AtMostOneClient \
	(SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask)

void
RecalculateDeliverableEvents(pWin)
    register WindowPtr pWin;
{
    register OtherClients *others;
    register WindowPtr pChild;

    pChild = pWin;
    while (1)
    {
	if (pChild->optional)
	{
	    pChild->optional->otherEventMasks = 0;
	    for (others = wOtherClients(pChild); others; others = others->next)
	    {
		pChild->optional->otherEventMasks |= others->mask;
	    }
	}
	pChild->deliverableEvents = pChild->eventMask|
				    wOtherEventMasks(pChild);
	if (pChild->parent)
	    pChild->deliverableEvents |=
		(pChild->parent->deliverableEvents &
		 ~wDontPropagateMask(pChild) & PropagateMask);
	if (pChild->firstChild)
	{
	    pChild = pChild->firstChild;
	    continue;
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
}

int
OtherClientGone(value, id)
    pointer value; /* must conform to DeleteType */
    XID   id;
{
    register OtherClientsPtr other, prev;
    register WindowPtr pWin = (WindowPtr)value;

    prev = 0;
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
	    {
		if (!(pWin->optional->otherClients = other->next))
		    CheckWindowOptionalNeed (pWin);
	    }
	    xfree(other);
	    RecalculateDeliverableEvents(pWin);
	    return(Success);
	}
	prev = other;
    }
    FatalError("client not on event list");
    /*NOTREACHED*/
    return -1; /* make compiler happy */
}

int
EventSelectForWindow(pWin, client, mask)
    register WindowPtr pWin;
    register ClientPtr client;
    Mask mask;
{
    Mask check;
    OtherClients * others;

    if (mask & ~AllEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    check = (mask & AtMostOneClient);
    if (check & (pWin->eventMask|wOtherEventMasks(pWin)))
    {				       /* It is illegal for two different
				          clients to select on any of the
				          events for AtMostOneClient. However,
				          it is OK, for some client to
				          continue selecting on one of those
				          events.  */
	if ((wClient(pWin) != client) && (check & pWin->eventMask))
	    return BadAccess;
	for (others = wOtherClients (pWin); others; others = others->next)
	{
	    if (!SameClient(others, client) && (check & others->mask))
		return BadAccess;
	}
    }
    if (wClient (pWin) == client)
    {
	check = pWin->eventMask;
	pWin->eventMask = mask;
    }
    else
    {
	for (others = wOtherClients (pWin); others; others = others->next)
	{
	    if (SameClient(others, client))
	    {
		check = others->mask;
		if (mask == 0)
		{
		    FreeResource(others->resource, RT_NONE);
		    return Success;
		}
		else
		    others->mask = mask;
		goto maskSet;
	    }
	}
	check = 0;
	if (!pWin->optional && !MakeWindowOptional (pWin))
	    return BadAlloc;
	others = (OtherClients *) xalloc(sizeof(OtherClients));
	if (!others)
	    return BadAlloc;
	others->mask = mask;
	others->resource = FakeClientID(client->index);
	others->next = pWin->optional->otherClients;
	pWin->optional->otherClients = others;
	if (!AddResource(others->resource, RT_OTHERCLIENT, (pointer)pWin))
	    return BadAlloc;
    }
maskSet: 
    MTX_LOCK_DEVICES();
    if ((inputInfo.pointer->valuator->motionHintWindow == pWin) &&
	(mask & PointerMotionHintMask) &&
	!(check & PointerMotionHintMask) &&
	!inputInfo.pointer->grab)
	inputInfo.pointer->valuator->motionHintWindow = NullWindow;
    MTX_UNLOCK_DEVICES();
    RecalculateDeliverableEvents(pWin);
    return Success;
}

/*ARGSUSED*/
int
EventSuppressForWindow(pWin, client, mask, checkOptional)
    register WindowPtr pWin;
    register ClientPtr client;
    Mask mask;
    Bool *checkOptional;
{
    register int i, free;

    if ((mask & ~PropagateMask) && !permitOldBugs)
    {
	client->errorValue = mask;
	return BadValue;
    }

    MTX_LOCK_DEVICES();
    if (pWin->dontPropagate)
	DontPropagateRefCnts[pWin->dontPropagate]--;
    if (!mask)
	i = 0;
    else
    {
	for (i = DNPMCOUNT, free = 0; --i > 0; )
	{
	    if (!DontPropagateRefCnts[i])
		free = i;
	    else if (mask == DontPropagateMasks[i])
		break;
	}
	if (!i && free)
	{
	    i = free;
	    DontPropagateMasks[i] = mask;
	}
    }
    if (i || !mask)
    {
	pWin->dontPropagate = i;
	if (i)
	    DontPropagateRefCnts[i]++;
	if (pWin->optional)
	{
	    pWin->optional->dontPropagateMask = mask;
	    *checkOptional = TRUE;
	}
    }
    else
    {
	if (!pWin->optional && !MakeWindowOptional (pWin))
	{
	    if (pWin->dontPropagate)
		DontPropagateRefCnts[pWin->dontPropagate]++;
            MTX_UNLOCK_DEVICES();
	    return BadAlloc;
	}
	pWin->dontPropagate = 0;
        pWin->optional->dontPropagateMask = mask;
    }

    MTX_UNLOCK_DEVICES();
    RecalculateDeliverableEvents(pWin);
    return Success;
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
EnterLeaveEvent(type, mode, detail, pWin, child)
    int type, mode, detail;
    register WindowPtr pWin;
    Window child;
{
    xEvent		event;
    register DeviceIntPtr keybd = inputInfo.keyboard;
    WindowPtr		focus;
    register DeviceIntPtr mouse = inputInfo.pointer;
    register GrabPtr	grab = mouse->grab;
    Mask		mask;

    if ((pWin == mouse->valuator->motionHintWindow) &&
	(detail != NotifyInferior))
	mouse->valuator->motionHintWindow = NullWindow;
    if (grab)
    {
	mask = (pWin == grab->window) ? grab->eventMask : 0;
	if (grab->ownerEvents)
	    mask |= EventMaskForClient(pWin, rClient(grab));
    }
    else
    {
	mask = pWin->eventMask | wOtherEventMasks(pWin);
    }
    if (mask & filters[type])
    {
	event.u.u.type = type;
	event.u.u.detail = detail;
	event.u.enterLeave.time = currentTime.milliseconds;
	event.u.enterLeave.rootX = sprite.hot.x;
	event.u.enterLeave.rootY = sprite.hot.y;
	/* Counts on the same initial structure of crossing & button events! */
	FixUpEventFromWindow(&event, pWin, None, FALSE);
	/* Enter/Leave events always set child */
	event.u.enterLeave.child = child;
	event.u.enterLeave.flags = event.u.keyButtonPointer.sameScreen ?
					    ELFlagSameScreen : 0;
#ifdef XKB
	event.u.enterLeave.state = mouse->button->state & 0x1f00;
	event.u.enterLeave.state |= (keybd->key->xkbInfo->grabState & 0xe0ff);
#else
	event.u.enterLeave.state = keybd->key->state | mouse->button->state;
#endif
	event.u.enterLeave.mode = mode;
	focus = keybd->focus->win;
	if ((focus != NoneWin) &&
	    ((pWin == focus) || (focus == PointerRootWin) ||
	     IsParent(focus, pWin)))
	    event.u.enterLeave.flags |= ELFlagFocus;
	if (grab)
	    (void) LockedTryClientEvents(rClient(grab), &event, 1, mask, filters[type], grab);
	else
	    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type],
					NullGrab, 0);
    }
    if ((type == EnterNotify) && (mask & KeymapStateMask))
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	memmove((char *)&ke.map[0], (char *)&keybd->key->down[1], 31);
	if (grab)
	    (void) LockedTryClientEvents(rClient(grab), (xEvent *)&ke, 1, mask,
				  KeymapStateMask, grab);
	else
	    (void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
					KeymapStateMask, NullGrab, 0);
    }
}

static void
EnterNotifies(ancestor, child, mode, detail)
    WindowPtr ancestor, child;
    int mode, detail;
{
    WindowPtr	parent = child->parent;

    if (ancestor == parent)
	return;
    EnterNotifies(ancestor, parent, mode, detail);
    EnterLeaveEvent(EnterNotify, mode, detail, parent, child->drawable.id);
}

static void
LeaveNotifies(child, ancestor, mode, detail)
    WindowPtr child, ancestor;
    int detail, mode;
{
    register WindowPtr  pWin;

    if (ancestor == child)
	return;
    for (pWin = child->parent; pWin != ancestor; pWin = pWin->parent)
    {
	EnterLeaveEvent(LeaveNotify, mode, detail, pWin, child->drawable.id);
	child = pWin;
    }
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
	EnterLeaveEvent(LeaveNotify, mode, NotifyInferior, fromWin, None);
	EnterNotifies(fromWin, toWin, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyAncestor, toWin, None);
    }
    else if (IsParent(toWin, fromWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyAncestor, fromWin, None);
	LeaveNotifies(fromWin, toWin, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyInferior, toWin, None);
    }
    else
    { /* neither fromWin nor toWin is descendent of the other */
	WindowPtr common = CommonAncestor(toWin, fromWin);
	/* common == NullWindow ==> different screens */
	EnterLeaveEvent(LeaveNotify, mode, NotifyNonlinear, fromWin, None);
	LeaveNotifies(fromWin, common, mode, NotifyNonlinearVirtual);
	EnterNotifies(common, toWin, mode, NotifyNonlinearVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyNonlinear, toWin, None);
    }
}

static void
FocusEvent(dev, type, mode, detail, pWin)
    DeviceIntPtr dev;
    int type, mode, detail;
    register WindowPtr pWin;
{
    xEvent event;

#ifdef XINPUT
    if (dev != inputInfo.keyboard)
    {
	DeviceFocusEvent(dev, type, mode, detail, pWin);
	return;
    }
#endif
    event.u.focus.mode = mode;
    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.focus.window = pWin->drawable.id;
    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type], NullGrab,
				0);
    if ((type == FocusIn) &&
	((pWin->eventMask | wOtherEventMasks(pWin)) & KeymapStateMask))
    {
	xKeymapEvent ke;
	ke.type = KeymapNotify;
	memmove((char *)&ke.map[0], (char *)&dev->key->down[1], 31);
	(void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
				    KeymapStateMask, NullGrab, 0);
    }
}

 /*
  * recursive because it is easier
  * no-op if child not descended from ancestor
  */
static Bool
FocusInEvents(dev, ancestor, child, skipChild, mode, detail, doAncestor)
    DeviceIntPtr dev;
    WindowPtr ancestor, child, skipChild;
    int mode, detail;
    Bool doAncestor;
{
    if (child == NullWindow)
	return ancestor == NullWindow;
    if (ancestor == child)
    {
	if (doAncestor)
	    FocusEvent(dev, FocusIn, mode, detail, child);
	return TRUE;
    }
    if (FocusInEvents(dev, ancestor, child->parent, skipChild, mode, detail,
		      doAncestor))
    {
	if (child != skipChild)
	    FocusEvent(dev, FocusIn, mode, detail, child);
	return TRUE;
    }
    return FALSE;
}

/* dies horribly if ancestor is not an ancestor of child */
static void
FocusOutEvents(dev, child, ancestor, mode, detail, doAncestor)
    DeviceIntPtr dev;
    WindowPtr child, ancestor;
    int mode;
    int detail;
    Bool doAncestor;
{
    register WindowPtr  pWin;

    for (pWin = child; pWin != ancestor; pWin = pWin->parent)
	FocusEvent(dev, FocusOut, mode, detail, pWin);
    if (doAncestor)
	FocusEvent(dev, FocusOut, mode, detail, ancestor);
}

void
DoFocusEvents(dev, fromWin, toWin, mode)
    DeviceIntPtr dev;
    WindowPtr fromWin, toWin;
    int mode;
{
    int     out, in;		       /* for holding details for to/from
				          PointerRoot/None */
    int     i;

    if (fromWin == toWin)
	return;
    out = (fromWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
    in = (toWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
 /* wrong values if neither, but then not referenced */

    if ((toWin == NullWindow) || (toWin == PointerRootWin))
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
   	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(dev, sprite.win, ROOT, mode, NotifyPointer,
			       TRUE);
	    /* Notify all the roots */
	    for (i=0; i<screenInfo.numScreens; i++)
	        FocusEvent(dev, FocusOut, mode, out, WindowTable[i]);
	}
	else
	{
	    if (IsParent(fromWin, sprite.win))
	      FocusOutEvents(dev, sprite.win, fromWin, mode, NotifyPointer,
			     FALSE);
	    FocusEvent(dev, FocusOut, mode, NotifyNonlinear, fromWin);
	    /* next call catches the root too, if the screen changed */
	    FocusOutEvents(dev, fromWin->parent, NullWindow, mode,
			   NotifyNonlinearVirtual, FALSE);
	}
	/* Notify all the roots */
	for (i=0; i<screenInfo.numScreens; i++)
	    FocusEvent(dev, FocusIn, mode, in, WindowTable[i]);
	if (toWin == PointerRootWin)
	    (void)FocusInEvents(dev, ROOT, sprite.win, NullWindow, mode,
				NotifyPointer, TRUE);
    }
    else
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(dev, sprite.win, ROOT, mode, NotifyPointer,
			       TRUE);
	    for (i=0; i<screenInfo.numScreens; i++)
	      FocusEvent(dev, FocusOut, mode, out, WindowTable[i]);
	    if (toWin->parent != NullWindow)
	      (void)FocusInEvents(dev, ROOT, toWin, toWin, mode,
				  NotifyNonlinearVirtual, TRUE);
	    FocusEvent(dev, FocusIn, mode, NotifyNonlinear, toWin);
	    if (IsParent(toWin, sprite.win))
    	       (void)FocusInEvents(dev, toWin, sprite.win, NullWindow, mode,
				   NotifyPointer, FALSE);
	}
	else
	{
	    if (IsParent(toWin, fromWin))
	    {
		FocusEvent(dev, FocusOut, mode, NotifyAncestor, fromWin);
		FocusOutEvents(dev, fromWin->parent, toWin, mode,
			       NotifyVirtual, FALSE);
		FocusEvent(dev, FocusIn, mode, NotifyInferior, toWin);
		if ((IsParent(toWin, sprite.win)) &&
			(sprite.win != fromWin) &&
			(!IsParent(fromWin, sprite.win)) &&
			(!IsParent(sprite.win, fromWin)))
		    (void)FocusInEvents(dev, toWin, sprite.win, NullWindow,
					mode, NotifyPointer, FALSE);
	    }
	    else
		if (IsParent(fromWin, toWin))
		{
		    if ((IsParent(fromWin, sprite.win)) &&
			    (sprite.win != fromWin) &&
			    (!IsParent(toWin, sprite.win)) &&
			    (!IsParent(sprite.win, toWin)))
			FocusOutEvents(dev, sprite.win, fromWin, mode,
				       NotifyPointer, FALSE);
		    FocusEvent(dev, FocusOut, mode, NotifyInferior, fromWin);
		    (void)FocusInEvents(dev, fromWin, toWin, toWin, mode,
					NotifyVirtual, FALSE);
		    FocusEvent(dev, FocusIn, mode, NotifyAncestor, toWin);
		}
		else
		{
		/* neither fromWin or toWin is child of other */
		    WindowPtr common = CommonAncestor(toWin, fromWin);
		/* common == NullWindow ==> different screens */
		    if (IsParent(fromWin, sprite.win))
			FocusOutEvents(dev, sprite.win, fromWin, mode,
				       NotifyPointer, FALSE);
		    FocusEvent(dev, FocusOut, mode, NotifyNonlinear, fromWin);
		    if (fromWin->parent != NullWindow)
		      FocusOutEvents(dev, fromWin->parent, common, mode,
				     NotifyNonlinearVirtual, FALSE);
		    if (toWin->parent != NullWindow)
		      (void)FocusInEvents(dev, common, toWin, toWin, mode,
					  NotifyNonlinearVirtual, FALSE);
		    FocusEvent(dev, FocusIn, mode, NotifyNonlinear, toWin);
		    if (IsParent(toWin, sprite.win))
			(void)FocusInEvents(dev, toWin, sprite.win, NullWindow,
					    mode, NotifyPointer, FALSE);
		}
	}
    }
}

int
SetInputFocus(client, dev, focusID, revertTo, ctime, followOK)
    ClientPtr client;
    DeviceIntPtr dev;
    Window focusID;
    CARD8 revertTo;
    Time ctime;
    Bool followOK;
{
    register FocusClassPtr focus;
    register WindowPtr focusWin;
    int mode;
    TimeStamp time;

    UpdateCurrentTime();
    if ((revertTo != RevertToParent) &&
	(revertTo != RevertToPointerRoot) &&
	(revertTo != RevertToNone) &&
	((revertTo != RevertToFollowKeyboard) || !followOK))
    {
	client->errorValue = revertTo;
	return BadValue;
    }
    time = ClientTimeToServerTime(ctime);
    if ((focusID == None) || (focusID == PointerRoot))
	focusWin = (WindowPtr)focusID;
    else if ((focusID == FollowKeyboard) && followOK)
	focusWin = inputInfo.keyboard->focus->win;
    else if (!(focusWin = LookupWindow(focusID, client)))
	return BadWindow;
    else
    {
 	/* It is a match error to try to set the input focus to an 
	unviewable window. */

	if(!focusWin->realized)
	    return(BadMatch);
    }
    focus = dev->focus;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	(CompareTimeStamps(time, focus->time) == EARLIER))
	return Success;
    mode = (dev->grab) ? NotifyWhileGrabbed : NotifyNormal;
    if (focus->win == FollowKeyboardWin)
	DoFocusEvents(dev, inputInfo.keyboard->focus->win, focusWin, mode);
    else
	DoFocusEvents(dev, focus->win, focusWin, mode);
    focus->time = time;
    focus->revert = revertTo;
    if (focusID == FollowKeyboard)
	focus->win = FollowKeyboardWin;
    else
	focus->win = focusWin;
    if ((focusWin == NoneWin) || (focusWin == PointerRootWin))
	focus->traceGood = 0;
    else
    {
        int depth = 0;
	register WindowPtr pWin;

        for (pWin = focusWin; pWin; pWin = pWin->parent) 
	    depth++;
        if (depth > focus->traceSize)
        {
	    focus->traceSize = depth+1;
	    focus->trace = (WindowPtr *)xnfrealloc
		(focus->trace, focus->traceSize * sizeof(WindowPtr));
	}
	focus->traceGood = depth;
        for (pWin = focusWin, depth--; pWin; pWin = pWin->parent, depth--) 
	    focus->trace[depth] = pWin;
    }
    return Success;
}

int
ProcSetInputFocus(client)
    ClientPtr client;
{
    int retval;

    REQUEST(xSetInputFocusReq);
    REQUEST_SIZE_MATCH(xSetInputFocusReq);
    MTXLockDevicesAndPOQ(client, CM_XSetInputFocus);

    retval = SetInputFocus(client, inputInfo.keyboard, stuff->focus,
			 stuff->revertTo, stuff->time, FALSE);
    MTXUnlockDevicesAndPOQ(client);
    return (retval);
}

int
ProcGetInputFocus(client)
    ClientPtr client;
{
    REPLY_DECL(xGetInputFocusReply,rep);
    FocusClassPtr focus;

    REQUEST(xReq);
    REQUEST_SIZE_MATCH(xReq);

    MTX_REP_CHECK_RETURN(rep,BadAlloc);

    MTXLockDevicesAndPOQ(client, CM_XGetInputFocus);

    focus = inputInfo.keyboard->focus;
    if (focus->win == NoneWin)
	rep->focus = None;
    else if (focus->win == PointerRootWin)
	rep->focus = PointerRoot;
    else 
	rep->focus = focus->win->drawable.id;

    rep->type = X_Reply;
    rep->length = 0;
    rep->sequenceNumber = client->sequence;
    rep->revertTo = focus->revert;

    WriteReplyToClient(client, sizeof(xGetInputFocusReply), rep);

    MTXUnlockDevicesAndPOQ(client);
    return Success;
}

int
ProcGrabPointer(client)
    ClientPtr client;
{
    REPLY_DECL(xGrabPointerReply,rep);
    int retValue = 0;
    DeviceIntPtr device;
    GrabPtr grab;
    WindowPtr pWin, confineTo;
    CursorPtr cursor, oldCursor;
    TimeStamp time;

    REQUEST(xGrabPointerReq);
    REQUEST_SIZE_MATCH(xGrabPointerReq);

    MTX_REP_CHECK_RETURN(rep,BadAlloc);

    MTX_REP_LOCK_AND_VERIFY_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
				stuff->confineTo, client, POQ_BORDER_SIZE, 
				CM_XGrabPointer);

    UpdateCurrentTime();
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        retValue = BadValue;
    }
    else if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        retValue = BadValue;
    }
    else if ((stuff->ownerEvents != xFalse) && (stuff->ownerEvents != xTrue))
    {
	client->errorValue = stuff->ownerEvents;
        retValue = BadValue;
    }
    else if ((stuff->eventMask & ~PointerGrabMask) && !permitOldBugs)
    {
	client->errorValue = stuff->eventMask;
        retValue = BadValue;
    }

    if (retValue != 0)
    {
	MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
                           stuff->confineTo, client);
	MTXReturnPooledMessage;
	return retValue;
    }

    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
#ifndef XTHREADS
	cursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
#else /* XTHREADS */
        LockAndVerifyCursor(&cursor, stuff->cursor, client);
#endif /* XTHREADS */
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
			       stuff->confineTo, client);
	    MTXReturnPooledMessage;
	    return BadCursor;
	}
    }

	/* at this point, some sort of reply is guaranteed. */
    time = ClientTimeToServerTime(stuff->time);
    rep->type = X_Reply;
    rep->sequenceNumber = client->sequence;
    rep->length = 0;

    MTX_LOCK_DEVICES();
    device = inputInfo.pointer;
    grab = device->grab;
    if ((grab) && !SameClient(grab, client))
	rep->status = AlreadyGrabbed;
    else if ((!pWin->realized) ||
	     (confineTo &&
		!(confineTo->realized &&
		  REGION_NOTEMPTY(confineTo->drawable.pScreen, 
			&confineTo->borderSize))))
	rep->status = GrabNotViewable;
    else if (device->sync.frozen &&
	     device->sync.other && !SameClient(device->sync.other, client))
	rep->status = GrabFrozen;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
	rep->status = GrabInvalidTime;
    else
    {
	GrabRec tempGrab;

	oldCursor = NullCursor;
	if (grab)
 	{
	    if (grab->confineTo && !confineTo)
		ConfineCursorToWindow(ROOT, FALSE, FALSE);
	    oldCursor = grab->cursor;
	}
	tempGrab.cursor = cursor;
	tempGrab.resource = client->clientAsMask;
	tempGrab.ownerEvents = stuff->ownerEvents;
	tempGrab.eventMask = stuff->eventMask;
	tempGrab.confineTo = confineTo;
	tempGrab.window = pWin;
	tempGrab.keyboardMode = stuff->keyboardMode;
	tempGrab.pointerMode = stuff->pointerMode;
	tempGrab.device = device;
	(*device->ActivateGrab)(device, &tempGrab, time, FALSE);
	if (oldCursor)
	    FreeCursor (oldCursor, (Cursor)0);
	rep->status = GrabSuccess;
    }
    MTX_UNLOCK_DEVICES();
    WriteReplyToClient(client, sizeof(xGrabPointerReply), rep);
#ifdef XTHREADS
    UnlockCursor(cursor, stuff->cursor);
#endif /* XTHREADS */
    MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
		       stuff->confineTo, client);
    return Success;
}

int
ProcChangeActivePointerGrab(client)
    ClientPtr client;
{
    DeviceIntPtr device;
    register GrabPtr grab;
    CursorPtr newCursor, oldCursor;
    TimeStamp time;

    REQUEST(xChangeActivePointerGrabReq);
    REQUEST_SIZE_MATCH(xChangeActivePointerGrabReq);
    MTXLockDevicesAndPOQ(client, CM_XChangeActivePointerGrab);

    device = inputInfo.pointer;
    grab = device->grab;

    if ((stuff->eventMask & ~PointerGrabMask) && !permitOldBugs)
    {
	client->errorValue = stuff->eventMask;
	MTXUnlockDevicesAndPOQ(client);
        return BadValue;
    }
    if (stuff->cursor == None)
	newCursor = NullCursor;
    else
    {
#ifndef XTHREADS
	newCursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
#else /* XTHREADS */
        /* ZZZ: cannot use MTX_LOCK_AND_VERIFY_CURSOR due to device lock */
	LockAndVerifyCursor(&newCursor, stuff->cursor, client);
#endif /* XTHREADS */
	if (!newCursor)
	{
	    client->errorValue = stuff->cursor;
	    MTXUnlockDevicesAndPOQ(client);
	    return BadCursor;
	}
    }
    if ((!grab) || (!SameClient(grab, client)))
    {
#ifdef XTHREADS
	UnlockCursor(newCursor, stuff->cursor);
#endif /* XTHREADS */
	MTXUnlockDevicesAndPOQ(client);
	return (Success);
    }
    time = ClientTimeToServerTime(stuff->time);
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
    {
#ifdef XTHREADS
	UnlockCursor(newCursor, stuff->cursor);
#endif /* XTHREADS */
	MTXUnlockDevicesAndPOQ(client);
	return Success;
    }
    oldCursor = grab->cursor;
    grab->cursor = newCursor;
    if (newCursor)
	newCursor->refcnt++;
    PostNewCursor();
    if (oldCursor)
	FreeCursor(oldCursor, (Cursor)0);

    grab->eventMask = stuff->eventMask;
#ifdef XTHREADS
    UnlockCursor(newCursor, stuff->cursor);
#endif /* XTHREADS */
    MTXUnlockDevicesAndPOQ(client);
    return Success;
}

int
ProcUngrabPointer(client)
    ClientPtr client;
{
    DeviceIntPtr device;
    GrabPtr grab;
    TimeStamp time;

    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    MTXLockDevicesAndPOQ(client, CM_XUngrabPointer);

    UpdateCurrentTime();
    device = inputInfo.pointer;
    grab = device->grab;
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	    (CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	    (grab) && SameClient(grab, client))
	(*device->DeactivateGrab)(device);
    MTXUnlockDevicesAndPOQ(client);
    return Success;
}

int
GrabDevice(client, dev, this_mode, other_mode, grabWindow, ownerEvents, ctime,
	   mask, status)
    register ClientPtr client;
    register DeviceIntPtr dev;
    unsigned this_mode;
    unsigned other_mode;
    Window grabWindow;
    unsigned ownerEvents;
    Time ctime;
    Mask mask;
    CARD8 *status;
{
    register WindowPtr pWin;
    register GrabPtr grab;
    TimeStamp time;

    UpdateCurrentTime();
    if ((this_mode != GrabModeSync) && (this_mode != GrabModeAsync))
    {
	client->errorValue = this_mode;
        return BadValue;
    }
    if ((other_mode != GrabModeSync) && (other_mode != GrabModeAsync))
    {
	client->errorValue = other_mode;
        return BadValue;
    }
    if ((ownerEvents != xFalse) && (ownerEvents != xTrue))
    {
	client->errorValue = ownerEvents;
        return BadValue;
    }
    pWin = LookupWindow(grabWindow, client);
    if (!pWin)
	return BadWindow;
    time = ClientTimeToServerTime(ctime);
    grab = dev->grab;
    if (grab && !SameClient(grab, client))
	*status = AlreadyGrabbed;
    else if (!pWin->realized)
	*status = GrabNotViewable;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, dev->grabTime) == EARLIER))
	*status = GrabInvalidTime;
    else if (dev->sync.frozen &&
	     dev->sync.other && !SameClient(dev->sync.other, client))
	*status = GrabFrozen;
    else
    {
	GrabRec tempGrab;

	tempGrab.window = pWin;
	tempGrab.resource = client->clientAsMask;
	tempGrab.ownerEvents = ownerEvents;
	tempGrab.keyboardMode = this_mode;
	tempGrab.pointerMode = other_mode;
	tempGrab.eventMask = mask;
	tempGrab.device = dev;
	(*dev->ActivateGrab)(dev, &tempGrab, time, FALSE);
	*status = GrabSuccess;
    }
    return Success;
}

int
ProcGrabKeyboard(client)
    ClientPtr client;
{
    REPLY_DECL(xGrabKeyboardReply,rep);
    int result;

    REQUEST(xGrabKeyboardReq);
    REQUEST_SIZE_MATCH(xGrabKeyboardReq);

    MTX_REP_CHECK_RETURN(rep,BadAlloc);

    MTXLockDevicesAndPOQ(client, CM_XGrabKeyboard);

    result = GrabDevice(client, inputInfo.keyboard, stuff->keyboardMode,
			stuff->pointerMode, stuff->grabWindow,
			stuff->ownerEvents, stuff->time,
			KeyPressMask | KeyReleaseMask, &(rep->status));
    if (result == Success)
    {
        rep->type = X_Reply;
        rep->sequenceNumber = client->sequence;
        rep->length = 0;
	WriteReplyToClient(client, sizeof(xGrabKeyboardReply), rep);
    }
    else
    {
	MTXReturnPooledMessage;
    }
    MTXUnlockDevicesAndPOQ(client);
    return result;
}

int
ProcUngrabKeyboard(client)
    ClientPtr client;
{
    DeviceIntPtr device;
    GrabPtr grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);

    MTXLockDevicesAndPOQ(client, CM_XUngrabKeyboard);

    UpdateCurrentTime();
    device = inputInfo.keyboard;
    grab = device->grab;
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	(CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	(grab) && SameClient(grab, client))
	    (*device->DeactivateGrab)(device);
    MTXUnlockDevicesAndPOQ(client);
    return (Success);
}

int
ProcQueryPointer(client)
    ClientPtr client;
{
    REPLY_DECL(xQueryPointerReply,rep);
    WindowPtr pWin, t;
    DeviceIntPtr mouse;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);

    MTX_REP_CHECK_RETURN(rep,BadAlloc);

    MTX_REP_LOCK_AND_VERIFY_WINDOW(pWin, stuff->id, client, POQ_BORDER_SIZE, 
			   CM_XQueryPointer);
    MTX_REP_LOCK_DEVICES();
    mouse = inputInfo.pointer;
    if (mouse->valuator->motionHintWindow)
	MaybeStopHint(mouse, client);
    rep->type = X_Reply;
    rep->sequenceNumber = client->sequence;
    rep->mask = mouse->button->state | inputInfo.keyboard->key->state;
    rep->length = 0;
    rep->root = (ROOT)->drawable.id;
    rep->rootX = sprite.hot.x;
    rep->rootY = sprite.hot.y;
    rep->child = None;

    if (sprite.hot.pScreen == pWin->drawable.pScreen)
    {
	rep->sameScreen = xTrue;
	rep->winX = sprite.hot.x - pWin->drawable.x;
	rep->winY = sprite.hot.y - pWin->drawable.y;
	for (t = sprite.win; t; t = t->parent)
	    if (t->parent == pWin)
	    {
		rep->child = t->drawable.id;
		break;
	    }
    }
    else
    {
	rep->sameScreen = xFalse;
	rep->winX = 0;
	rep->winY = 0;
    }
    MTX_UNLOCK_DEVICES();
    WriteReplyToClient(client, sizeof(xQueryPointerReply), rep);
    MTX_UNLOCK_WINDOW(pWin, stuff->id, client);
    return(Success);    
}

void
InitEvents()
{
    int i;

    sprite.hot.pScreen = sprite.hotPhys.pScreen = (ScreenPtr)NULL;
    inputInfo.numDevices = 0;
    inputInfo.devices = (DeviceIntPtr)NULL;
    inputInfo.off_devices = (DeviceIntPtr)NULL;
    inputInfo.keyboard = (DeviceIntPtr)NULL;
    inputInfo.pointer = (DeviceIntPtr)NULL;
    if (spriteTraceSize == 0)
    {
	spriteTraceSize = 32;
	spriteTrace = (WindowPtr *)xalloc(32*sizeof(WindowPtr));
	if (!spriteTrace)
	    FatalError("failed to allocate spriteTrace");
    }
    spriteTraceGood = 0;
    lastEventMask = OwnerGrabButtonMask;
    filters[MotionNotify] = PointerMotionMask;
    sprite.win = NullWindow;
    sprite.current = NullCursor;
    sprite.hotLimits.x1 = 0;
    sprite.hotLimits.y1 = 0;
    sprite.hotLimits.x2 = 0;
    sprite.hotLimits.y2 = 0;
    sprite.confined = FALSE;
    syncEvents.replayDev = (DeviceIntPtr)NULL;
    syncEvents.replayWin = NullWindow;
    while (syncEvents.pending)
    {
	QdEventPtr next = syncEvents.pending->next;
	xfree(syncEvents.pending);
	syncEvents.pending = next;
    }
    syncEvents.pendtail = &syncEvents.pending;
    syncEvents.playingEvents = FALSE;
    syncEvents.time.months = 0;
    syncEvents.time.milliseconds = 0;	/* hardly matters */
    currentTime.months = 0;
    currentTime.milliseconds = GetTimeInMillis();
    lastDeviceEventTime = currentTime;
    for (i = 0; i < DNPMCOUNT; i++)
    {
	DontPropagateMasks[i] = 0;
	DontPropagateRefCnts[i] = 0;
    }
}

int
ProcSendEvent(client)
    ClientPtr client;
{
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow; /* only set if dest==InputFocus */

    REQUEST(xSendEventReq);
    REQUEST_SIZE_MATCH(xSendEventReq);

    /* The client's event type must be a core event type or one defined by an
	extension. */
    MTXLockDevicesAndPOQ(client, CM_XSendEvent);

    if ( ! ((stuff->event.u.u.type > X_Reply &&
	     stuff->event.u.u.type < LASTEvent) || 
	    (stuff->event.u.u.type >= EXTENSION_EVENT_BASE &&
	     stuff->event.u.u.type < lastEvent)))
    {
	client->errorValue = stuff->event.u.u.type;
	MTXUnlockDevicesAndPOQ(client);
	return BadValue;
    }
    if (stuff->event.u.u.type == ClientMessage &&
	stuff->event.u.u.detail != 8 &&
	stuff->event.u.u.detail != 16 &&
	stuff->event.u.u.detail != 32 &&
	!permitOldBugs)
    {
	client->errorValue = stuff->event.u.u.detail;
	MTXUnlockDevicesAndPOQ(client);
	return BadValue;
    }
    if ((stuff->eventMask & ~AllEventMasks) && !permitOldBugs)
    {
	client->errorValue = stuff->eventMask;
	MTXUnlockDevicesAndPOQ(client);
	return BadValue;
    }

    if (stuff->destination == PointerWindow)
	pWin = sprite.win;
    else if (stuff->destination == InputFocus)
    {
	WindowPtr inputFocus = inputInfo.keyboard->focus->win;
	if (inputFocus == NoneWin)
	{
	    MTXUnlockDevicesAndPOQ(client);
	    return Success;
	}
	/* If the input focus is PointerRootWin, send the event to where
	the pointer is if possible, then perhaps propogate up to root. */
   	if (inputFocus == PointerRootWin)
	    inputFocus = ROOT;

        effectiveFocus = inputFocus;
	if (IsParent(inputFocus, sprite.win))
	    pWin = sprite.win;
	else
	    pWin = inputFocus;
    }
    else
	pWin = LookupWindow(stuff->destination, client);
    if (!pWin)
    {
        MTXUnlockDevicesAndPOQ(client);
	return BadWindow;
    }
    if ((stuff->propagate != xFalse) && (stuff->propagate != xTrue))
    {
	client->errorValue = stuff->propagate;
	MTXUnlockDevicesAndPOQ(client);
	return BadValue;
    }

    stuff->event.u.u.type |= 0x80;
    if (stuff->propagate)
    {
	for (;pWin; pWin = pWin->parent)
	{
	    if (DeliverEventsToWindow(pWin, &stuff->event, 1, stuff->eventMask,
				      NullGrab, 0))
	    {
                MTXUnlockDevicesAndPOQ(client);
		return Success;
	    }
	    if (pWin == effectiveFocus)
	    {
                MTXUnlockDevicesAndPOQ(client);
		return Success;
	    }
	    stuff->eventMask &= ~wDontPropagateMask(pWin);
	    if (!stuff->eventMask)
		break;
	}
    }
    else
	(void)DeliverEventsToWindow(pWin, &stuff->event, 1, stuff->eventMask,
				    NullGrab, 0);
    MTXUnlockDevicesAndPOQ(client);
    return Success;
}

int
ProcUngrabKey(client)
    ClientPtr client;
{
    WindowPtr pWin;
    GrabRec tempGrab;
    DeviceIntPtr keybd;
    Bool deleted;

    REQUEST(xUngrabKeyReq);
    REQUEST_SIZE_MATCH(xUngrabKeyReq);

    MTX_LOCK_AND_VERIFY_WINDOW(pWin, stuff->grabWindow, client, 
			   POQ_NULL_REGION, CM_XUngrabKey);

    MTX_LOCK_DEVICES();

    keybd = inputInfo.keyboard;
    if (((stuff->key > keybd->key->curKeySyms.maxKeyCode) ||
	 (stuff->key < keybd->key->curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
    {
	client->errorValue = stuff->key;
        MTX_UNLOCK_DEVICES();
        MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
        return BadValue;
    }
    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
        MTX_UNLOCK_DEVICES();
        MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
	return BadValue;
    }

    tempGrab.resource = client->clientAsMask;
    tempGrab.device = keybd;
    tempGrab.window = pWin;
    tempGrab.modifiersDetail.exact = stuff->modifiers;
    tempGrab.modifiersDetail.pMask = NULL;
    tempGrab.modifierDevice = inputInfo.keyboard;
    tempGrab.type = KeyPress;
    tempGrab.detail.exact = stuff->key;
    tempGrab.detail.pMask = NULL;

    deleted = DeletePassiveGrabFromList(&tempGrab);
    MTX_UNLOCK_DEVICES();
    MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
    return((deleted) ? Success: BadAlloc);
}

int
ProcGrabKey(client)
    ClientPtr client;
{
    WindowPtr pWin;
    GrabPtr grab;
    int retValue = 0;
    DeviceIntPtr keybd;

    REQUEST(xGrabKeyReq);
    REQUEST_SIZE_MATCH(xGrabKeyReq);
    MTX_LOCK_AND_VERIFY_WINDOW(pWin, stuff->grabWindow, client, 
			   POQ_NULL_REGION, CM_XGrabKey);

    MTX_LOCK_DEVICES();
    keybd = inputInfo.keyboard;
    if ((stuff->ownerEvents != xTrue) && (stuff->ownerEvents != xFalse))
    {
	client->errorValue = stuff->ownerEvents;
        retValue = BadValue;
    }
    else if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        retValue = BadValue;
    }
    else if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        retValue = BadValue;
    }
    else if (((stuff->key > keybd->key->curKeySyms.maxKeyCode) ||
	 (stuff->key < keybd->key->curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
    {
	client->errorValue = stuff->key;
        retValue = BadValue;
    }
    else if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	retValue = BadValue;
    }

    if (retValue == 0)
    {
	grab = CreateGrab(client->index, keybd, pWin, 
	    (Mask)(KeyPressMask | KeyReleaseMask), (Bool)stuff->ownerEvents,
	    (Bool)stuff->keyboardMode, (Bool)stuff->pointerMode,
	    keybd,stuff->modifiers,KeyPress,stuff->key,NullWindow,NullCursor);

        if (grab)
            retValue = AddPassiveGrabToList(grab);
	else
	    retValue = BadAlloc;
    }
    MTX_UNLOCK_DEVICES();
    MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
    return retValue;
}

int
ProcGrabButton(client)
    ClientPtr client;
{
    int retValue = 0;
    WindowPtr pWin, confineTo;
    CursorPtr cursor;
    GrabPtr grab;
    Mask eventMask;
    REQUEST(xGrabButtonReq);

    REQUEST_SIZE_MATCH(xGrabButtonReq);
    MTX_LOCK_AND_VERIFY_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
				stuff->confineTo, client, POQ_BORDER_SIZE, 
				CM_XGrabButton);

    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
	retValue = BadValue;
    }
    else if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
	retValue = BadValue;
    }
    else if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	retValue = BadValue;
    }
    else if ((stuff->ownerEvents != xFalse) && (stuff->ownerEvents != xTrue))
    {
	client->errorValue = stuff->ownerEvents;
	retValue = BadValue;
    }
    else if (stuff->eventMask & ~PointerGrabMask)
    {
	client->errorValue = stuff->eventMask;
        retValue = BadValue;
    }
    if (retValue)
    {
	    MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
                           stuff->confineTo, client);
	return retValue;
    }

    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
#ifndef XTHREADS
	cursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
#else /* XTHREADS */
        LockAndVerifyCursor(&cursor, stuff->cursor, client);
#endif /* XTHREADS */
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
			       stuff->confineTo, client);
	    return BadCursor;
	}
    }

    eventMask = permitOldBugs ?
	(Mask)(stuff->eventMask | ButtonPressMask | ButtonReleaseMask) :
	(Mask)stuff->eventMask;

    MTX_LOCK_DEVICES();
    grab = CreateGrab(client->index, inputInfo.pointer, pWin, eventMask, 
	(Bool)stuff->ownerEvents, (Bool) stuff->keyboardMode,
	(Bool)stuff->pointerMode, inputInfo.keyboard, stuff->modifiers,
	ButtonPress, stuff->button, confineTo, cursor);
    if (grab)
	retValue = AddPassiveGrabToList(grab);
    else
	retValue = BadAlloc;

    MTX_UNLOCK_DEVICES();
#ifdef XTHREADS
    UnlockCursor(cursor, stuff->cursor);
#endif /* XTHREADS */
    MTX_UNLOCK_ALL_WINDOWS(pWin, confineTo, stuff->grabWindow, 
		       stuff->confineTo, client);
    return retValue;
}

int
ProcUngrabButton(client)
    ClientPtr client;
{
    WindowPtr pWin;
    GrabRec tempGrab;
    Bool deleted;
    REQUEST(xUngrabButtonReq);
    REQUEST_SIZE_MATCH(xUngrabButtonReq);
    MTX_LOCK_AND_VERIFY_WINDOW(pWin, stuff->grabWindow, client, 
                           POQ_BORDER_SIZE, CM_XUngrabButton);

    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
	return BadValue;
    }

    MTX_LOCK_DEVICES();
    tempGrab.resource = client->clientAsMask;
    tempGrab.device = inputInfo.pointer;
    tempGrab.window = pWin;
    tempGrab.modifiersDetail.exact = stuff->modifiers;
    tempGrab.modifiersDetail.pMask = NULL;
    tempGrab.modifierDevice = inputInfo.keyboard;
    tempGrab.type = ButtonPress;
    tempGrab.detail.exact = stuff->button;
    tempGrab.detail.pMask = NULL;

    deleted = DeletePassiveGrabFromList(&tempGrab);
    MTX_UNLOCK_DEVICES();
    MTX_UNLOCK_WINDOW(pWin, stuff->grabWindow, client);
    return((deleted) ? Success: BadAlloc);
}

void
DeleteWindowFromAnyEvents(pWin, freeResources)
    WindowPtr		pWin;
    Bool		freeResources;
{
    WindowPtr		parent;
    DeviceIntPtr	mouse;
    DeviceIntPtr	keybd;
    FocusClassPtr	focus;
    OtherClientsPtr	oc;
    GrabPtr		passive;


    /* Deactivate any grabs performed on this window, before making any
	input focus changes. */

    MTX_LOCK_DEVICES();
    mouse = inputInfo.pointer;
    keybd = inputInfo.keyboard;
    focus = keybd->focus;

    if (mouse->grab &&
	((mouse->grab->window == pWin) || (mouse->grab->confineTo == pWin)))
	(*mouse->DeactivateGrab)(mouse);

    /* Deactivating a keyboard grab should cause focus events. */

    if (keybd->grab && (keybd->grab->window == pWin))
	(*keybd->DeactivateGrab)(keybd);

    /* If the focus window is a root window (ie. has no parent) then don't 
	delete the focus from it. */
    
    if ((pWin == focus->win) && (pWin->parent != NullWindow))
    {
	int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	if (keybd->grab)
	    focusEventMode = NotifyWhileGrabbed;

	switch (focus->revert)
	{
	case RevertToNone:
	    DoFocusEvents(keybd, pWin, NoneWin, focusEventMode);
	    focus->win = NoneWin;
	    focus->traceGood = 0;
	    break;
	case RevertToParent:
	    parent = pWin;
	    do
	    {
		parent = parent->parent;
		focus->traceGood--;
	    } while (!parent->realized
/* This would be a good protocol change -- windows being reparented
   during SaveSet processing would cause the focus to revert to the
   nearest enclosing window which will survive the death of the exiting
   client, instead of ending up reverting to a dying window and thence
   to None
 */
#ifdef NOTDEF
 	      || clients[CLIENT_ID(parent->drawable.id)]->clientGone
#endif
		);
	    DoFocusEvents(keybd, pWin, parent, focusEventMode);
	    focus->win = parent;
	    focus->revert = RevertToNone;
	    break;
	case RevertToPointerRoot:
	    DoFocusEvents(keybd, pWin, PointerRootWin, focusEventMode);
	    focus->win = PointerRootWin;
	    focus->traceGood = 0;
	    break;
	}
    }

    if (mouse->valuator->motionHintWindow == pWin)
	mouse->valuator->motionHintWindow = NullWindow;

    if (freeResources)
    {
	if (pWin->dontPropagate)
	    DontPropagateRefCnts[pWin->dontPropagate]--;
	while (oc = wOtherClients(pWin))
	    FreeResource(oc->resource, RT_NONE);
	while (passive = wPassiveGrabs(pWin))
	    FreeResource(passive->resource, RT_NONE);
     }
    /* TBD -- is this in the right place??    */
    /* Should we unlock prior to FreeResource */
    /* or after Ext code??                    */
    MTX_UNLOCK_DEVICES();

#ifdef XINPUT
    DeleteWindowFromAnyExtEvents(pWin, freeResources);
#endif
}

/* Call this whenever some window at or below pWin has changed geometry */

/*ARGSUSED*/
void
CheckCursorConfinement(pWin)
    WindowPtr pWin;
{
    GrabPtr grab = inputInfo.pointer->grab;
    WindowPtr confineTo;

    if (grab && (confineTo = grab->confineTo))
    {
	if (!REGION_NOTEMPTY(confineTo->drawable.pScreen, 
			&confineTo->borderSize))
	    (*inputInfo.pointer->DeactivateGrab)(inputInfo.pointer);
	else if ((pWin == confineTo) || IsParent(pWin, confineTo))
	    ConfineCursorToWindow(confineTo, TRUE, TRUE);
    }
}

Mask
EventMaskForClient(pWin, client)
    WindowPtr		pWin;
    ClientPtr		client;
{
    register OtherClientsPtr	other;

    if (wClient (pWin) == client)
	return pWin->eventMask;
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (SameClient(other, client))
	    return other->mask;
    }
    return 0;
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
    MTXLockDevicesAndPOQ(client, CM_XRecolorCursor);
#ifdef XTHREADS
    LockAndVerifyCursor(&pCursor, stuff->cursor, client);
#else /* XTHREADS */
    pCursor = (CursorPtr)LookupIDByType(stuff->cursor, RT_CURSOR);
#endif /* XTHREADS */
    if ( !pCursor) 
    {
	client->errorValue = stuff->cursor;
        MTXUnlockDevicesAndPOQ(client);
	return (BadCursor);
    }

    pCursor->foreRed = stuff->foreRed;
    pCursor->foreGreen = stuff->foreGreen;
    pCursor->foreBlue = stuff->foreBlue;

    pCursor->backRed = stuff->backRed;
    pCursor->backGreen = stuff->backGreen;
    pCursor->backBlue = stuff->backBlue;

    for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
    {
	pscr = screenInfo.screens[nscr];
	( *pscr->RecolorCursor)(pscr, pCursor,
				(pCursor == sprite.current) &&
				(pscr == sprite.hotPhys.pScreen));
    }
#ifdef XTHREADS
    UnlockCursor(pCursor, stuff->cursor);
#endif /* XTHREADS */
    MTXUnlockDevicesAndPOQ(client);
    return (Success);
}

#ifndef XTHREADS
void
WriteEventsToClient(pClient, count, events)
    ClientPtr	pClient;
    int		count;
    xEvent	*events;
{
    if(pClient->swapped)
    {
        int	i;
        xEvent	eventTo, *eventFrom;

	for(i = 0; i < count; i++)
	{
	    eventFrom = &events[i];
	    /* Remember to strip off the leading bit of type in case
	       this event was sent with "SendEvent." */
	    (*EventSwapVector[eventFrom->u.u.type & 0177])
		(eventFrom, &eventTo);
#ifdef XKB
	    (void)XkbFilterWriteEvents(pClient, 1, (char *)&eventTo);
#else
	    (void)WriteToClient(pClient, sizeof(xEvent), (char *)&eventTo);
#endif
	}
    }
    else
    {
#ifdef XKB
	(void)XkbFilterWriteEvents(pClient, count, events);
#else
	(void)WriteToClient(pClient, count * sizeof(xEvent), (char *) events);
#endif
    }
}
#endif /* XTHREADS */
