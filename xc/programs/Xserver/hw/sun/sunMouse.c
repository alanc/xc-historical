/* $XConsortium: sunMouse.c,v 5.12 91/11/15 18:28:42 gildea Exp $ */
/*-
 * sunMouse.c --
 *	Functions for playing cat and mouse... sorry.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define NEED_EVENTS
#include    "sun.h"
#include    "mipointer.h"

Bool ActiveZaphod = TRUE;

static Bool sunCursorOffScreen();
static void sunCrossScreen();
static void sunWarpCursor();

miPointerScreenFuncRec sunPointerScreenFuncs = {
    sunCursorOffScreen,
    sunCrossScreen,
    sunWarpCursor,
};

typedef struct {
    int	    bmask;	    /* Current button state */
} SunMsPrivRec, *SunMsPrivPtr;

static void 	  	sunMouseCtrl();
static Firm_event 	*sunMouseGetEvents();
static void 	  	sunMouseEnqueueEvent();

static SunMsPrivRec	sunMousePriv;

static PtrPrivRec 	sysMousePriv = {
    -1,				/* Descriptor to device */
    sunMouseGetEvents,		/* Function to read events */
    sunMouseEnqueueEvent,	/* Function to process an event */
    (pointer)&sunMousePriv,	/* Field private to device */
};

/*-
 *-----------------------------------------------------------------------
 * sunMouseProc --
 *	Handle the initialization, etc. of a mouse
 *
 * Results:
 *	none.
 *
 * Side Effects:
 *
 * Note:
 *	When using sunwindows, all input comes off a single fd, stored in the
 *	global windowFd.  Therefore, only one device should be enabled and
 *	disabled, even though the application still sees both mouse and
 *	keyboard.  We have arbitrarily chosen to enable and disable windowFd
 *	in the keyboard routine sunKbdProc rather than in sunMouseProc.
 *
 *-----------------------------------------------------------------------
 */
int
sunMouseProc (pMouse, what)
    DevicePtr	  pMouse;   	/* Mouse to play with */
    int	    	  what;	    	/* What to do with it */
{
    register int  fd;
    int	    	  format;
    static int	  oformat;
    BYTE    	  map[4];
    char	  *device, *getenv ();

    switch (what) {
	case DEVICE_INIT:
	    if (pMouse != LookupPointerDevice()) {
		ErrorF ("Cannot open non-system mouse");	
		return (!Success);
	    }

	    if (! sunUseSunWindows()) {
		if (sysMousePriv.fd >= 0) {
		    fd = sysMousePriv.fd;
		} else {
		    if (!(device = getenv ("MOUSE")) ||
			 (fd = open (device, O_RDWR, 0)) == -1)
			fd = open ("/dev/mouse", O_RDWR, 0);
		    if (fd < 0) {
			Error ("Opening /dev/mouse");
			return (!Success);
		    }
		    if (fcntl (fd, F_SETFL, (FNDELAY|FASYNC)) < 0
			|| fcntl(fd, F_SETOWN, getpid()) < 0) {
			    perror("sunMouseProc");
			    ErrorF("Can't set up mouse on fd %d\n", fd);
			}
		    
		    sysMousePriv.fd = fd;
		}
	    }

	    sunMousePriv.bmask = 0;

	    pMouse->devicePrivate = (pointer) &sysMousePriv;
	    pMouse->on = FALSE;
	    map[1] = 1;
	    map[2] = 2;
	    map[3] = 3;
	    InitPointerDeviceStruct(
		pMouse, map, 3, miPointerGetMotionEvents,
 		sunMouseCtrl, miPointerGetMotionBufferSize());
	    break;

	case DEVICE_ON:
	    if (! sunUseSunWindows()) {
		if (ioctl (((PtrPrivPtr)pMouse->devicePrivate)->fd,
			VUIDGFORMAT, &oformat) < 0) {
		    Error ("VUIDGFORMAT");
		    return(!Success);
		}
		format = VUID_FIRM_EVENT;
		if (ioctl (((PtrPrivPtr)pMouse->devicePrivate)->fd,
			VUIDSFORMAT, &format) < 0) {
		    Error ("VUIDSFORMAT");
		    return(!Success);
		}
		AddEnabledDevice (((PtrPrivPtr)pMouse->devicePrivate)->fd);
	    }

	    pMouse->on = TRUE;
	    break;

	case DEVICE_CLOSE:
	    if (! sunUseSunWindows()) {
		if (ioctl (((PtrPrivPtr)pMouse->devicePrivate)->fd,
			VUIDSFORMAT, &oformat) < 0) {
		    Error ("VUIDSFORMAT");
		}
	    }
	    break;

	case DEVICE_OFF:
	    pMouse->on = FALSE;
	    if (! sunUseSunWindows()) {
		RemoveEnabledDevice (((PtrPrivPtr)pMouse->devicePrivate)->fd);
	    }
	    break;
    }
    return (Success);
}
	    
/*-
 *-----------------------------------------------------------------------
 * sunMouseCtrl --
 *	Alter the control parameters for the mouse. Since acceleration
 *	etc. is done from the PtrCtrl record in the mouse's device record,
 *	there's nothing to do here.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
sunMouseCtrl (pMouse)
    DevicePtr	  pMouse;
{
}

/*-
 *-----------------------------------------------------------------------
 * sunMouseGetEvents --
 *	Return the events waiting in the wings for the given mouse.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */
static Firm_event *
sunMouseGetEvents (pMouse, pNumEvents, pAgain)
    DevicePtr	  pMouse;	    /* Mouse to read */
    int	    	  *pNumEvents;	    /* Place to return number of events */
    Bool	  *pAgain;	    /* whether more might be available */
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    register PtrPrivPtr	  pPriv;
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    pPriv = (PtrPrivPtr) pMouse->devicePrivate;

    nBytes = read (pPriv->fd, evBuf, sizeof(evBuf));

    if (nBytes < 0) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading mouse");
	    FatalError ("Could not read from mouse");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return (evBuf);
}


/*-
 *-----------------------------------------------------------------------
 * MouseAccelerate --
 *	Given a delta and a mouse, return the acceleration of the delta.
 *
 * Results:
 *	The corrected delta
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static short
MouseAccelerate (pMouse, delta)
    DevicePtr	  pMouse;
    int	    	  delta;
{
    register int  sgn = sign(delta);
    register PtrCtrl *pCtrl;

    delta = abs(delta);
    pCtrl = &((DeviceIntPtr) pMouse)->ptrfeed->ctrl;

    if (delta > pCtrl->threshold) {
	return ((short) (sgn * (pCtrl->threshold +
				((delta - pCtrl->threshold) * pCtrl->num) /
				pCtrl->den)));
    } else {
	return ((short) (sgn * delta));
    }
}

/*-
 *-----------------------------------------------------------------------
 * sunMouseEnqueueEvent --
 *	Given a Firm_event for a mouse, pass it off the the dix layer
 *	properly converted...
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The cursor may be redrawn...? devPrivate/x/y will be altered.
 *
 *-----------------------------------------------------------------------
 */
static void
sunMouseEnqueueEvent (pMouse, fe)
    DevicePtr	  pMouse;   	/* Mouse from which the event came */
    Firm_event	  *fe;	    	/* Event to process */
{
    xEvent		xE;
    register PtrPrivPtr	pPriv;	/* Private data for pointer */
    register SunMsPrivPtr pSunPriv; /* Private data for mouse */
    register int  	bmask;	/* Temporary button mask */
    register unsigned long  time;
    int			x, y;

    pPriv = (PtrPrivPtr)pMouse->devicePrivate;
    pSunPriv = (SunMsPrivPtr) pPriv->devPrivate;

    time = xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);

    switch (fe->id)
    {
    case MS_LEFT:
    case MS_MIDDLE:
    case MS_RIGHT:
	/*
	 * A button changed state. Sometimes we will get two events
	 * for a single state change. Should we get a button event which
	 * reflects the current state of affairs, that event is discarded.
	 *
	 * Mouse buttons start at 1.
	 */
	xE.u.u.detail = (fe->id - MS_LEFT) + 1;
	bmask = 1 << xE.u.u.detail;
	if (fe->value == VKEY_UP) {
	    if (pSunPriv->bmask & bmask) {
		xE.u.u.type = ButtonRelease;
		pSunPriv->bmask &= ~bmask;
	    } else {
		return;
	    }
	} else {
	    if ((pSunPriv->bmask & bmask) == 0) {
		xE.u.u.type = ButtonPress;
		pSunPriv->bmask |= bmask;
	    } else {
		return;
	    }
	}
	mieqEnqueue (&xE);
	break;
    case LOC_X_DELTA:
	miPointerDeltaCursor (MouseAccelerate(pMouse,fe->value),0,time);
	break;
    case LOC_Y_DELTA:
	/*
	 * For some reason, motion up generates a positive y delta
	 * and motion down a negative delta, so we must subtract
	 * here instead of add...
	 */
	miPointerDeltaCursor (0,-MouseAccelerate(pMouse,fe->value),time);
	break;
    case LOC_X_ABSOLUTE:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (fe->value, y, time);
	break;
    case LOC_Y_ABSOLUTE:
	miPointerPosition (&x, &y);
	miPointerAbsoluteCursor (x, fe->value, time);
	break;
    default:
	FatalError ("sunMouseEnqueueEvent: unrecognized id\n");
	break;
    }
}

/*ARGSUSED*/
static Bool
sunCursorOffScreen (pScreen, x, y)
    ScreenPtr	*pScreen;
    int		*x, *y;
{
    int	    index;

    /*
     * Active Zaphod implementation:
     *    increment or decrement the current screen
     *    if the x is to the right or the left of
     *    the current screen.
     */
    if (ActiveZaphod &&
	screenInfo.numScreens > 1 && (*x >= (*pScreen)->width || *x < 0))
    {
	index = (*pScreen)->myNum;
	if (*x < 0)
	{
	    index = (index ? index : screenInfo.numScreens) - 1;
	    *pScreen = screenInfo.screens[index];
	    *x += (*pScreen)->width;
	}
	else
	{
	    *x -= (*pScreen)->width;
	    index = (index + 1) % screenInfo.numScreens;
	    *pScreen = screenInfo.screens[index];
	}
	return TRUE;
    }
    return FALSE;
}

static void
sunCrossScreen (pScreen, entering)
    ScreenPtr	pScreen;
    Bool	entering;
{
    u_char  select;

    select = 1;
    if (entering)
	select = 0;
    if (sunFbs[pScreen->myNum].EnterLeave)
	(*sunFbs[pScreen->myNum].EnterLeave) (pScreen, select);
}

static void
sunWarpCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
#ifdef SVR4
    sigset_t newmask, oldmask;
#else
    int	    oldmask;
#endif

#ifdef SVR4
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGIO);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
#else
    oldmask = sigblock (sigmask(SIGIO));
#endif
    miPointerWarpCursor (pScreen, x, y);
#ifdef SVR4
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
#else
    sigsetmask (oldmask);
#endif
}

#ifdef SUN_WINDOWS

/*
 * Enqueue a sunwindows mouse event.  The possible events are
 *   LOC_MOVE
 *   MS_LEFT
 *   MS_MIDDLE
 *   MS_RIGHT
 */

void
sunMouseEnqueueEventSunWin(pMouse,se)
    DeviceRec *pMouse;
    register struct inputevent *se;
{   
    xEvent			xE;
    register int	  	bmask;	/* Temporary button mask */
    register PtrPrivPtr		pPriv;	/* Private data for pointer */
    register SunMsPrivPtr	pSunPriv; /* Private data for mouse */
    int				x, y;
    unsigned long		time;

    pPriv = (PtrPrivPtr)pMouse->devicePrivate;
    time = xE.u.keyButtonPointer.time = TVTOMILLI(event_time(se));

    switch (event_id(se)) {
        case MS_LEFT:
        case MS_MIDDLE:
        case MS_RIGHT:
	    /*
	     * A button changed state. Sometimes we will get two events
	     * for a single state change. Should we get a button event which
	     * reflects the current state of affairs, that event is discarded.
	     *
	     * Mouse buttons start at 1.
	     */
	    pSunPriv = (SunMsPrivPtr) pPriv->devPrivate;
	    xE.u.u.detail = (event_id(se) - MS_LEFT) + 1;
	    bmask = 1 << xE.u.u.detail;
	    if (win_inputnegevent(se)) {
		if (pSunPriv->bmask & bmask) {
		    xE.u.u.type = ButtonRelease;
		    pSunPriv->bmask &= ~bmask;
		} else {
		    return;
		}
	    } else {
		if ((pSunPriv->bmask & bmask) == 0) {
		    xE.u.u.type = ButtonPress;
		    pSunPriv->bmask |= bmask;
		} else {
		    return;
		}
	    }
	    mieqEnqueue (&xE);
    	    break;
        case LOC_MOVE:
	    miPointerAbsoluteCursor(event_x(se),event_y(se),time);
	    miPointerPosition (&x, &y);
	    if (x != event_x(se) || y != event_y(se))
	        /*
                 * Tell SunWindows that X is constraining the mouse
                 * cursor so that the server and SunWindows stay in sync.
	         */
	        win_setmouseposition(windowFd, x, y);
	    break;
	default:
	    FatalError ("sunMouseEnqueueEventSunWin: unrecognized id\n");
	    break;
    }
}
#endif /* SUN_WINDOWS */
