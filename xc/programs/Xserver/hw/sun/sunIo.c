/* $Header$ */
/*-
 * sunIo.c --
 *	Functions to handle input from the keyboard and mouse.
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


/*
 * The Sun X drivers are a product of Sun Microsystems, Inc. and are provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify these drivers without charge, but are not authorized
 * to license or distribute them to anyone else except as part of a product or
 * program developed by the user.
 * 
 * THE SUN X DRIVERS ARE PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND
 * INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 * PRACTICE.
 *
 * The Sun X Drivers are provided with no support and without any obligation
 * on the part of Sun Microsystems, Inc. to assist in their use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THE SUN X
 * DRIVERS OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef	lint
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

/*-
 * Copyright (c) 1987 by Sun Microsystems,  Inc.
 */

#include    "sun.h"
#include    "opaque.h"

Bool	    	screenSaved = FALSE;
int	    	lastEventTime = 0;
extern int	sunSigIO;
extern void	SaveScreens();

#ifdef SUN_WINDOWS
int		windowFd = 0;
#define	INPBUFSIZE	128
#endif SUN_WINDOWS

/*-
 *-----------------------------------------------------------------------
 * TimeSinceLastInputEvent --
 *	Function used for screensaver purposes by the os module.
 *
 * Results:
 *	The time in milliseconds since there last was any
 *	input.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
int
TimeSinceLastInputEvent()
{
    struct timeval	now;

    gettimeofday (&now, (struct timezone *)0);

    if (lastEventTime == 0) {
	lastEventTime = TVTOMILLI(now);
    }
    return TVTOMILLI(now) - lastEventTime;
}

/*-
 *-----------------------------------------------------------------------
 * ProcessInputEvents --
 *	Retrieve all waiting input events and pass them to DIX in their
 *	correct chronological order. Only reads from the system pointer
 *	and keyboard.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Events are passed to the DIX layer.
 *
 *-----------------------------------------------------------------------
 */
void
ProcessInputEvents ()
{
    register Firm_event    *ptrEvents,    	/* Current pointer event */
			   *kbdEvents;	    	/* Current keyboard event */
    register int	    numPtrEvents, 	/* Number of remaining pointer
						 * events */
			    numKbdEvents;   	/* Number of remaining
						 * keyboard events */
    int	    	  	    nPE,    	    	/* Original number of pointer
						 * events */
			    nKE;    	    	/* Original number of
						 * keyboard events */
    DevicePtr		    pPointer;
    DevicePtr		    pKeyboard;
    register PtrPrivPtr     ptrPriv;
    register KbPrivPtr	    kbdPriv;
    Firm_event	  	    *lastEvent;	    	/* Last event processed */
    enum {
	NoneYet, Ptr, Kbd
    }			    lastType = NoneYet;	/* Type of last event */

#ifdef SUN_WINDOWS
    struct inputevent sunevents[INPBUFSIZE];
    register struct inputevent *se = sunevents, *seL;
    int         n;
    static int event_ignore = TRUE;
#endif SUN_WINDOWS

    /*
     *  Defensive programming - only reset sunIOPending (preventing
     *  further calls to ProcessInputEvents() until a future SIGIO)
     *  if we have actually received a SIGIO,  so we know it works.
     */
    if (sunSigIO) {
	isItTimeToYield = 0;
    }
    pPointer = LookupPointerDevice();
    pKeyboard = LookupKeyboardDevice();

    if ( sunUseSunWindows() ) {
#ifdef SUN_WINDOWS
	if ((n=read(windowFd,sunevents,INPBUFSIZE*sizeof sunevents[0])) < 0 
			    && errno != EWOULDBLOCK) {
	    /*
	     * Error reading events; should do something. XXX
	     */
	    return;
	}

	for (seL = sunevents + (n/(sizeof sunevents[0]));  se < seL; se++) {
	    if (screenSaved)
		SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
	    lastEventTime = TVTOMILLI(event_time(se));

	    /*
	     * Decide whether or not to pay attention to events.
	     * Ignore the events if the locator has exited X Display.
	     */
	    switch (event_id(se)) {
		case LOC_WINEXIT:
		    event_ignore = TRUE;
		    sunSetUpKbdSunWin(windowFd, FALSE, pKeyboard);
		    break;
		case LOC_WINENTER:
		    event_ignore = FALSE;
		    sunSetUpKbdSunWin(windowFd, TRUE, pKeyboard);
		    break;
	    }

	    if (event_ignore) {
		continue;
	    }

	    /*
	     * Figure out the X device this event should be reported on.
	     */
	    switch (event_id(se)) {
		case LOC_MOVE:
		case MS_LEFT:
		case MS_MIDDLE:
		case MS_RIGHT:
		    sunMouseProcessEventSunWin(pPointer,se);
		    break;
		case LOC_WINEXIT:
		case LOC_WINENTER:
		    break;
		default:
		    sunKbdProcessEventSunWin(pKeyboard,se);
		    break;
	    }
	}
#endif SUN_WINDOWS
    } 
    else {
	ptrPriv = (PtrPrivPtr)pPointer->devicePrivate;
	kbdPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	
	/*
	 * Get events from both the pointer and the keyboard, storing the number
	 * of events gotten in nPE and nKE and keeping the start of both arrays
	 * in pE and kE
	 */
	ptrEvents = (* ptrPriv->GetEvents) (pPointer, &nPE);
	kbdEvents = (* kbdPriv->GetEvents) (pKeyboard, &nKE);
	
	numPtrEvents = nPE;
	numKbdEvents = nKE;
	lastEvent = (Firm_event *)0;

	/*
	 * So long as one event from either device remains unprocess, we loop:
	 * Take the oldest remaining event and pass it to the proper module
	 * for processing. The DDXEvent will be sent to ProcessInput by the
	 * function called.
	 */
	while (numPtrEvents || numKbdEvents) {
	    if (numPtrEvents && numKbdEvents) {
		if (timercmp (&kbdEvents->time, &ptrEvents->time, <)) {
		    if (lastType == Ptr) {
			(* ptrPriv->DoneEvents) (pPointer, FALSE);
		    }
		    (* kbdPriv->ProcessEvent) (pKeyboard, kbdEvents);
		    numKbdEvents--;
		    lastEvent = kbdEvents++;
		    lastType = Kbd;
		} else {
		    if (lastType == Kbd) {
			(* kbdPriv->DoneEvents) (pKeyboard, FALSE);
		    }
		    (* ptrPriv->ProcessEvent) (pPointer, ptrEvents);
		    numPtrEvents--;
		    lastEvent = ptrEvents++;
		    lastType = Ptr;
		}
	    } else if (numKbdEvents) {
		if (lastType == Ptr) {
		    (* ptrPriv->DoneEvents) (pPointer, FALSE);
		}
		(* kbdPriv->ProcessEvent) (pKeyboard, kbdEvents);
		numKbdEvents--;
		lastEvent = kbdEvents++;
		lastType = Kbd;
	    } else {
		if (lastType == Kbd) {
		    (* kbdPriv->DoneEvents) (pKeyboard, FALSE);
		}
		(* ptrPriv->ProcessEvent) (pPointer, ptrEvents);
		numPtrEvents--;
		lastEvent = ptrEvents++;
		lastType = Ptr;
	    }
	}

	if (lastEvent) {
	    lastEventTime = TVTOMILLI(lastEvent->time);
	    if (screenSaved) {
		SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
	    }
	}
	
	(* kbdPriv->DoneEvents) (pKeyboard);
	(* ptrPriv->DoneEvents) (pPointer);

    }

    sunRestoreCursor();

}


/*-
 *-----------------------------------------------------------------------
 * SetTimeSinceLastInputEvent --
 *	Set the lastEventTime to now.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	lastEventTime is altered.
 *
 *-----------------------------------------------------------------------
 */
void
SetTimeSinceLastInputEvent()
{
    struct timeval now;

    gettimeofday (&now, (struct timezone *)0);
    lastEventTime = TVTOMILLI(now);
}
