/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
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

#include    "macII.h"
#include    "opaque.h"

int	    	lastEventTime = 0;
extern int	macIISigIO;
extern int      screenIsSaved;
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

    static int optionKeyUp = 1;
#define IS_OPTION_KEY(x)	(KEY_DETAIL(x) == 0x3a)
#define IS_LEFT_ARROW_KEY(x)	(KEY_DETAIL(x) == 0x3b)
#define IS_RIGHT_ARROW_KEY(x)	(KEY_DETAIL(x) == 0x3c)
#define IS_DOWN_ARROW_KEY(x)	(KEY_DETAIL(x) == 0x3d)
#define IS_UP_ARROW_KEY(x)	(KEY_DETAIL(x) == 0x3e)
#define IS_ARROW_KEY(x)						\
	(IS_LEFT_ARROW_KEY(x) || IS_RIGHT_ARROW_KEY(x) || 	\
	 IS_DOWN_ARROW_KEY(x) || IS_UP_ARROW_KEY(x))

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
 /*debug*/
        ErrorF("ProcessInputEvents: read(windowFd)  n=%d\n",n);
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
        if (screenIsSaved == SCREEN_SAVER_ON)
            SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);

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

	if (IS_OPTION_KEY(*me)) {
	    optionKeyUp = KEY_UP(*me);
	}

	/*
	 * Patch up the event if the option key is down and an arrow key is hit
	 * in order to generate arrow key codes.
	 */

	if (!optionKeyUp && IS_ARROW_KEY(*me)) {
	    int keyUp = KEY_UP(*me);

	    if (IS_RIGHT_ARROW_KEY(*me)) 	*me = 0x7b;
	    else if (IS_LEFT_ARROW_KEY(*me)) 	*me = 0x7c;
	    else if (IS_DOWN_ARROW_KEY(*me))	*me = 0x7d;
	    else  				*me = 0x70; 	/* code for UP arrow */
	    if (keyUp) *me |= 0x80;
	}

	if (KEY_DETAIL(*me) == MOUSE_ESCAPE) { 
	    if (lastType == Kbd) {
    		(* kbdPriv->DoneEvents) (pKeyboard);
	    }
    	    (* ptrPriv->ProcessEvent) (pPointer,me);
	    me += 2;
	    lastType = Ptr;
	}

	else if (IS_MOUSE_KEY(*me))
        {
	    if (lastType == Kbd) {
    		(* kbdPriv->DoneEvents) (pKeyboard);
	    }
    	    (* ptrPriv->ProcessEvent) (pPointer,me);
	    lastType = Ptr;
	}

	else if (IS_OPTION_KEY(*me)) {
	    /* do nothing */
	}

	else {
	    if (lastType == Ptr) {
    		(* ptrPriv->DoneEvents) (pPointer, FALSE);
	    }
    	    (* kbdPriv->ProcessEvent) (pKeyboard,me);
	    lastType = Kbd;
        }
    }

    (* kbdPriv->DoneEvents) (pKeyboard);
    (* ptrPriv->DoneEvents) (pPointer, FALSE);

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
