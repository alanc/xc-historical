/*-
 * macIIKbd.c --
 *	Functions for retrieving data from a keyboard.
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

#define NEED_EVENTS
#include "macII.h"
#include <stdio.h>
#include "Xproto.h"
#include "keysym.h"
#include "inputstr.h"

extern CARD16 keyModifiersList[];

typedef struct {
    int	    	  trans;          	/* Original translation form */
} macIIKbPrivRec, *macIIKbPrivPtr;

extern CARD8 *macIIModMap[];
extern KeySymsRec macIIKeySyms[];

static void 	  macIIBell();
static void 	  macIIKbdCtrl();
void 	  	  macIIKbdProcessEvent();
static void 	  macIIKbdDoneEvents();
int	  	  autoRepeatKeyDown = 0;
int	  	  autoRepeatDebug = 0;
int	  	  autoRepeatReady;
static int	  autoRepeatFirst;
long 		  autoRepeatLastKeyDownTv;
long 		  autoRepeatDeltaTv;

#ifdef notdef
static struct timeval autoRepeatLastKeyDownTv;
static struct timeval autoRepeatDeltaTv;
#define	tvminus(tv, tv1, tv2) 	/* tv = tv1 - tv2 */ \
		if ((tv1).tv_usec < (tv2).tv_usec) { \
		    (tv1).tv_usec += 1000000; \
		    (tv1).tv_sec -= 1; \
		} \
		(tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
		(tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;
#define tvplus(tv, tv1, tv2) 	/* tv = tv1 + tv2 */ \
		(tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
		(tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
		if ((tv).tv_usec > 1000000) { \
			(tv).tv_usec -= 1000000; \
			(tv).tv_sec += 1; \
		}
#endif notdef


static macIIKbPrivRec	macIIKbPriv;  
static KbPrivRec  	sysKbPriv = {
    -1,				/* Type of keyboard */
    -1,				/* Descriptor open to device */
    macIIKbdProcessEvent,		/* Function to process an event */
    macIIKbdDoneEvents,		/* Function called when all events */
				/* have been handled. */
    (pointer)&macIIKbPriv,	/* Private to keyboard device */
    (Bool)0,			/* Mapped queue */
    0,				/* offset for device keycodes */
};

extern int consoleFd;

/*-
 *-----------------------------------------------------------------------
 * macIIKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 * Note:
 *	When using macII, all input comes off a single fd, stored in the
 *	global consoleFd.  Therefore, only one device should be enabled and
 *	disabled, even though the application still sees both mouse and
 *	keyboard.  We have arbitrarily chosen to enable and disable consoleFd
 *	in the keyboard routine macIIKbdProc rather than in macIIMouseProc.
 *
 *-----------------------------------------------------------------------
 */

int
macIIKbdProc (pKeyboard, what)
    DevicePtr	  pKeyboard;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
{
    switch (what) {
	case DEVICE_INIT:
	    if (pKeyboard != LookupKeyboardDevice()) {
		ErrorF ("Cannot open non-system keyboard");
		return (!Success);
	    }
	    if (consoleFd == 0) {
		if ((consoleFd = open("/dev/console", O_RDWR|O_NDELAY)) < 0) {
			FatalError("Could not open /dev/console. \n");
			return (!Success);
		}
            }

	    /*
	     * Perform final initialization of the system private keyboard
	     * structure and fill in various slots in the device record
	     * itself which couldn't be filled in before.
	     */
	    pKeyboard->devicePrivate = (pointer)&sysKbPriv;

	    pKeyboard->on = FALSE;
	    /*
	     * ensure that the keycodes on the wire are >= MIN_KEYCODE
	     */

	    sysKbPriv.type = KBTYPE_MACII;  /* XXX ioctl to detect type XXX */

	    if (macIIKeySyms[sysKbPriv.type].minKeyCode < MIN_KEYCODE) {
		int offset = MIN_KEYCODE -macIIKeySyms[sysKbPriv.type].minKeyCode;

		macIIKeySyms[sysKbPriv.type].minKeyCode += offset;
		macIIKeySyms[sysKbPriv.type].maxKeyCode += offset;
		sysKbPriv.offset = offset;
	    }
	    InitKeyboardDeviceStruct(
		    pKeyboard,
		    &(macIIKeySyms[sysKbPriv.type]),
		    (macIIModMap[sysKbPriv.type]),
		    macIIBell,
		    macIIKbdCtrl);
	    break;

	case DEVICE_ON:
	    macIIKbdSetUp(consoleFd, TRUE);

	    AddEnabledDevice(consoleFd);

	    pKeyboard->on = TRUE;
	    break;

	case DEVICE_CLOSE:
	case DEVICE_OFF:
	    macIIKbdSetUp(consoleFd, FALSE);

	    RemoveEnabledDevice(consoleFd);

	    pKeyboard->on = FALSE;
	    break;

    }
    return (Success);
}

#include <sys/stropts.h>
#include <sys/termio.h>
#include <sys/video.h>

static struct termio d_tio = {
	(BRKINT|IGNPAR|ISTRIP|ICRNL|IXON)&(~IGNBRK)&(~PARMRK)&(~INPCK)&(~INLCR)&
	(~IGNCR)&(~IUCLC)&(~IXANY)&(~IXOFF),
	(OPOST|ONLCR)&(~OLCUC)&(~OCRNL)&(~ONOCR)&(~ONLRET)&(~OFILL)&(~OFDEL),
	(B9600|CS7|CREAD)&(~CSTOPB)&(~PARENB)&(~PARODD)&(~HUPCL)&(~CLOCAL)&(~LOBLK),
	(ISIG|ICANON|ECHO|ECHOE|ECHOK)&(~XCASE)&(~ECHONL)&(~NOFLSH),
	0,
	{CINTR, CQUIT, CERASE, CKILL, CEOF, CNUL, CNUL, CNUL}
};

int
macIIKbdSetUp(fd, openClose)
    int		fd;
    Bool	openClose;
{
    struct strioctl ctl;
    struct termio tio;
    int status;

    if (openClose) {

	ioctl(fd, I_POP, 0);
		
	status = 1;
	if (ioctl(fd, FIONBIO, &status) < 0) {
		FatalError("Could not ioctl FIONBIO. \n");
		return (!Success);
	}

#ifdef	notdef
	status = 1;
	if (ioctl(fd, FIOASYNC, &status) < 0) {
		FatalError("Could not ioctl FIOASYNC. \n");
		return (!Success);
	}
#endif

	tio.c_iflag = (IGNPAR|IGNBRK) & (~PARMRK) & (~ISTRIP);
	tio.c_oflag = 0;
	tio.c_cflag = CREAD|CS8|B9600;
	tio.c_lflag = 0;
	if (ioctl(fd, TCSETA, &tio) < 0) {
		FatalError("Failed to ioctl TCSETA.\n");
		return (!Success);
	}

	ctl.ic_len = 0;
	ctl.ic_cmd = VIDEO_RAW;
	if (ioctl(fd, I_STR, &ctl) < 0) {
		FatalError("Failed to ioctl I_STR VIDEO_RAW.\n");
		return(!Success);
	}

	ctl.ic_len = 0;
	ctl.ic_cmd = VIDEO_MOUSE;
	if (ioctl(fd, I_STR, &ctl) < 0) {

#define MSG "Failed to ioctl I_STR VIDEO_MOUSE.\n Run Xrepair and try again.\n"
		FatalError(MSG);
#undef MSG
		return(!Success);
	}

	if (ioctl(fd, I_FLUSH, FLUSHRW) < 0) {
		FatalError("Failed to ioctl I_FLUSH FLUSHRW.\n");
		return (!Success);
	}

    } else {
	status = 0;
	if (ioctl(fd, FIONBIO, &status) < 0) {
		FatalError("Could not ioctl FIONBIO. \n");
		return (!Success);
	}

#ifdef notdef
	status = 0;
	if (ioctl(fd, FIOASYNC, &status) < 0) {
		FatalError("Could not ioctl FIOASYNC. \n");
		return (!Success);
	}
#endif

	ioctl(fd, I_POP, 0); /* remove line discipline */
	ioctl(fd, I_FLUSH, FLUSHRW); /* flush input to get known state */
    
	ctl.ic_len = 0;
	ctl.ic_cmd = VIDEO_NOMOUSE;
	ioctl(fd, I_STR, &ctl);

	ctl.ic_len = 0;
	ctl.ic_cmd = VIDEO_ASCII;
	ioctl(fd, I_STR, &ctl);

	ioctl(fd, I_PUSH, "line");

	ioctl(fd, TCSETA, &d_tio);
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIBell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	None, really...
 *
 *-----------------------------------------------------------------------
 */
static void
macIIBell (loudness, pKeyboard)
    int	    	  loudness;	    /* Percentage of full volume */
    DevicePtr	  pKeyboard;	    /* Keyboard to ring */
{
#ifdef notdef
    struct strioctl ctl;

    ctl.ic_len = 0;
    ctl.ic_cmd = VIDEO_BELL;
    if (ioctl(consoleFd, I_STR, &ctl) < 0) {
	    FatalError("Failed to ioctl I_STR VIDEO_BELL.\n");
    }
#endif
}

/*-
 *-----------------------------------------------------------------------
 * macIIKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 *
 *-----------------------------------------------------------------------
 */
static void
macIIKbdCtrl (pKeyboard)
    DevicePtr	  pKeyboard;	    /* Keyboard to alter */
{
}

/*-
 *-----------------------------------------------------------------------
 * macIIDoneEvents --
 *	Nothing to do, here...
 *
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static void
macIIKbdDoneEvents (pKeyboard)
    DevicePtr	  pKeyboard;
{
}

/*-
 *-----------------------------------------------------------------------
 * macIIKbdProcessEvent
 *	Process macIIevent destined for the keyboard.
 * 	
 * Results:
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */

void
macIIKbdProcessEvent(pKeyboard,me)
    DeviceRec *pKeyboard;
    register unsigned char *me;
{   
    xEvent		xE;
    PtrPrivPtr	  	ptrPriv;
    int                 delta;
    static xEvent       autoRepeatEvent;
    BYTE		key;

    ptrPriv = (PtrPrivPtr) LookupPointerDevice()->devicePrivate;

    if (autoRepeatKeyDown && *me == AUTOREPEAT_EVENTID) {
	/*
	 * Generate auto repeat event.	XXX one for now.
	 * Update time & pointer location of saved KeyPress event.
	 */
	if (autoRepeatDebug)
	    ErrorF("macIIKbdProcessEvent: autoRepeatKeyDown = %d\n",
			autoRepeatKeyDown);

	delta = autoRepeatDeltaTv >> 1;
	if (autoRepeatFirst == TRUE)
		autoRepeatFirst = FALSE;

	/*
	 * Fake a key up event and a key down event
	 * for the last key pressed.
	 */
	autoRepeatEvent.u.keyButtonPointer.time += delta;
	autoRepeatEvent.u.keyButtonPointer.rootX = ptrPriv->x;
	autoRepeatEvent.u.keyButtonPointer.rootY = ptrPriv->y;
	autoRepeatEvent.u.u.type = KeyRelease;
	(* pKeyboard->processInputProc) (&autoRepeatEvent, pKeyboard);

	autoRepeatEvent.u.keyButtonPointer.time += delta;
	autoRepeatEvent.u.u.type = KeyPress;
	(* pKeyboard->processInputProc) (&autoRepeatEvent, pKeyboard);

	/* Update time of last key down */
	autoRepeatLastKeyDownTv += autoRepeatDeltaTv;

	return;
    }

    key = KEY_DETAIL(*me) + sysKbPriv.offset;
    if (!keyModifiersList[key])
    {
        /*
         * Kill AutoRepeater on any real Kbd event.
         */
        autoRepeatKeyDown = 0;
        if (autoRepeatDebug)
            ErrorF("macIIKbdProcessEvent: autoRepeat off\n");
    }

    xE.u.keyButtonPointer.time = lastEventTime;
    xE.u.keyButtonPointer.rootX = ptrPriv->x;
    xE.u.keyButtonPointer.rootY = ptrPriv->y;
    xE.u.u.type = (KEY_UP(*me) ? KeyRelease : KeyPress);
    xE.u.u.detail = key;

    if (keyModifiersList[key] & LockMask)
    {
	if (xE.u.u.type == KeyRelease)
	    return; /* this assumes autorepeat is not desired */
	if (((DeviceIntPtr)pKeyboard)->down[key>>3] & (1 << (key & 7)))
	    xE.u.u.type = KeyRelease;
    }

    if (!KEY_UP(*me) && !keyModifiersList[key]) 
    {   /* turn on AutoRepeater */
        if (autoRepeatDebug)
            ErrorF("macIIKbdProcessEvent: KEY_DOWN\n");
        autoRepeatEvent = xE;
        autoRepeatFirst = TRUE;
        autoRepeatKeyDown++;
        autoRepeatLastKeyDownTv = lastEventTime;
    }

    (* pKeyboard->processInputProc) (&xE, pKeyboard);
}


Bool
LegalModifier(key)
{
    return (TRUE);
}

#include <sys/time.h>
/*ARGSUSED*/
void
macIIBlockHandler(nscreen, pbdata, pptv, pReadmask)
    int nscreen;
    pointer pbdata;
    struct timeval **pptv;
    pointer pReadmask;
{
    static struct timeval artv; /* autorepeat timeval */
    static sec1 = 0;                    /* tmp for patching */
    static sec2 = AUTOREPEAT_INITIATE;
    static sec3 = AUTOREPEAT_DELAY;

    if (!autoRepeatKeyDown)
        return;

    artv.tv_sec = sec1;
    if (autoRepeatFirst == TRUE)
        artv.tv_usec = 1000 * sec2;
    else
        artv.tv_usec = 1000 * sec3;
    *pptv = &artv;
    if (autoRepeatDebug)
        ErrorF("macIIBlockHandler(%d,%d): \n", artv.tv_sec, artv.tv_usec);

}

/*ARGSUSED*/
void
macIIWakeupHandler(nscreen, pbdata, err, pReadmask)
    int nscreen;
    pointer pbdata;
    unsigned long err;
    pointer pReadmask;
{
    struct tms tms;
    long now;

    if (autoRepeatDebug)
        ErrorF("macIIWakeupHandler(ar=%d, err=%d):\n", autoRepeatKeyDown, err);

    if (autoRepeatKeyDown) {
        now = times(&tms) << 4;
        autoRepeatDeltaTv = now - autoRepeatLastKeyDownTv;
        if ((!autoRepeatFirst && autoRepeatDeltaTv > AUTOREPEAT_DELAY) ||
            (autoRepeatDeltaTv > AUTOREPEAT_INITIATE))
                autoRepeatReady++;
    }

    if (autoRepeatReady)
        ProcessInputEvents();
    autoRepeatReady = 0;

}

