/*-
 * macIIIo.c --
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

#ifndef	lint
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

#include    "macII.h"
#include    "opaque.h"

Bool	    	screenSaved = FALSE;
int	    	lastEventTime = 0;
extern int	macIISigIO;
extern void	SaveScreens();

int		consoleFd = 0;
#define	INPBUFSIZE	3*64

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
#ifdef USE_TOD_CLOCK
    struct timeval	now;

    gettimeofday (&now, (struct timezone *)0);

    if (lastEventTime == 0) {
	lastEventTime = TVTOMILLI(now);
    }
    return TVTOMILLI(now) - lastEventTime;
#else
    struct tms		tms;
    long		now;

    now = times (&tms) << 4; /* XXX approximately milliseconds XXX */
    if (lastEventTime == 0) {
	lastEventTime = now;
    }
    return now - lastEventTime;
#endif USE_TOD_CLOCK
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
    DevicePtr		    pPointer;
    DevicePtr		    pKeyboard;
    register PtrPrivPtr	    ptrPriv;
    register KbPrivPtr      kbdPriv;
    enum {
        NoneYet, Ptr, Kbd
    }                       lastType = NoneYet; /* Type of last event */


    unsigned char macIIevents[INPBUFSIZE];
    register unsigned char *me = macIIevents, *meL;
    int         n;

    /*
     *  Defensive programming - only reset macIIIOPending (preventing
     *  further calls to ProcessInputEvents() until a future SIGIO)
     *  if we have actually received a SIGIO,  so we know it works.
     */
    if (macIISigIO) {
	isItTimeToYield = 0;
    }
    pPointer = LookupPointerDevice();
    pKeyboard = LookupKeyboardDevice();
    ptrPriv = (PtrPrivPtr)pPointer->devicePrivate;
    kbdPriv = (KbPrivPtr)pKeyboard->devicePrivate;


    if ((n = read(consoleFd,macIIevents,INPBUFSIZE*sizeof macIIevents[0])) < 0 
    		    && errno != ENODATA && errno != EWOULDBLOCK) {
        /*
         * Error reading events; should do something. XXX
         */
        return;
    }

    if (autoRepeatKeyDown && autoRepeatReady && 
	kbdPriv->ctrl->autoRepeat == AutoRepeatModeOn && n <= 0) {
        /* fake a macII kbd event */
        n = sizeof macIIevents[0];
        *me = AUTOREPEAT_EVENTID;
        if (autoRepeatDebug)
            ErrorF("ProcessInputEvents: sw auto event\n");
    }

    for (meL = macIIevents + (n/(sizeof macIIevents[0]));  me < meL; me++) {
        if (screenSaved)
    	SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);

#ifdef USE_TOD_CLOCK
	{
	    struct timeval	now;
	    gettimeofday (&now, (struct timezone *)0);
	    lastEventTime = TVTOMILLI(now);
	}
#else
        {
	    struct tms	tms;
            lastEventTime = times (&tms) << 4;
        }
#endif USE_TOD_CLOCK


        /*
         * Figure out the X device this event should be reported on.
         */
	if (KEY_DETAIL(*me) == MOUSE_ESCAPE) { 
	    if (lastType == Kbd) {
    		(* kbdPriv->DoneEvents) (pKeyboard);
	    }
    	    macIIMouseProcessEvent(pPointer,me);
	    me += 2;
	    lastType = Ptr;
	}
	else if (KEY_DETAIL(*me) == PSEUDO_MIDDLE || KEY_DETAIL(*me) == PSEUDO_RIGHT) {
	    if (lastType == Kbd) {
    		(* kbdPriv->DoneEvents) (pKeyboard);
	    }
    	    macIIMouseProcessEvent(pPointer,me);
	    lastType = Ptr;
	}
	else {
	    if (lastType == Ptr) {
    		(* ptrPriv->DoneEvents) (pPointer);
	    }
    	    macIIKbdProcessEvent(pKeyboard,me);
	    lastType = Kbd;
        }
    }

    (* kbdPriv->DoneEvents) (pKeyboard);
    (* ptrPriv->DoneEvents) (pPointer);

    macIIRestoreCursor();

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
#ifdef USE_TOD_CLOCK
    struct timeval now;

    gettimeofday (&now, (struct timezone *)0);
    lastEventTime = TVTOMILLI(now);
#else
    struct tms	tms;

    lastEventTime = times (&tms) << 4;
#endif USE_TOD_CLOCK
}
