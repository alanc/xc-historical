/* $XConsortium: sunKbd.c,v 5.25 93/09/09 17:07:40 kaleb Exp $ */
/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
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
#include "sun.h"
#include "keysym.h"
#include "Sunkeysym.h"

#define SUN_LED_MASK	0x0f
#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */
#define MAX_KEYCODE	255	/* limited by the protocol */
#ifndef KB_SUN4 
#define KB_SUN4		4
#endif

#define AUTOREPEAT_INITIATE	200
#define AUTOREPEAT_DELAY	50

#define tvminus(tv, tv1, tv2)   /* tv = tv1 - tv2 */ \
		if ((tv1).tv_usec < (tv2).tv_usec) { \
		    (tv1).tv_usec += 1000000; \
		    (tv1).tv_sec -= 1; \
		} \
		(tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
		(tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;

#define tvplus(tv, tv1, tv2)    /* tv = tv1 + tv2 */ \
		(tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
		(tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
		if ((tv).tv_usec > 1000000) { \
		    (tv).tv_usec -= 1000000; \
		    (tv).tv_sec += 1; \
		}

extern KeySymsRec sunKeySyms[];
extern SunModmapRec *sunModMaps[];
extern int sunMaxLayout;
extern KeySym *sunType4KeyMaps[];
extern SunModmapRec *sunType4ModMaps[];

static CARD8 *sunConModMap = 0;
static KeySymsRec *sunConKeySyms;

extern void	ProcessInputEvents();
extern void	miPointerPosition();

long	  	  sunAutoRepeatInitiate = 1000 * AUTOREPEAT_INITIATE;
long	  	  sunAutoRepeatDelay = 1000 * AUTOREPEAT_DELAY;

static void		bell();
static void		kbdCtrl();
static Firm_event	*kbdGetEvents();
static void		kbdEnqueueEvent();
static int		autoRepeatKeyDown = 0;
static int		autoRepeatReady;
static int		autoRepeatFirst;
static struct timeval	autoRepeatLastKeyDownTv;
static struct timeval	autoRepeatDeltaTv;

static KbPrivRec  	sysKbPriv = {
    -1,			/* Type of keyboard */
    -1,			/* Layout of keyboard */
    -1,			/* Descriptor open to device */
    kbdGetEvents,	/* Function to read events */
    kbdEnqueueEvent,	/* Function to enqueue an event */
    (Bool)0,		/* Mapped queue */
    0,			/* offset for device keycodes */
    &defaultKeyboardControl/* Initial full duration = .25 sec. */
};

static void kbdWait()
{
    static struct timeval lastChngKbdTransTv;
    struct timeval tv;
    struct timeval lastChngKbdDeltaTv;
    int lastChngKbdDelta;

    gettimeofday(&tv, (struct timezone *) NULL);
    if (!lastChngKbdTransTv.tv_sec)
	lastChngKbdTransTv = tv;
    tvminus(lastChngKbdDeltaTv, tv, lastChngKbdTransTv);
    lastChngKbdDelta = TVTOMILLI(lastChngKbdDeltaTv);
    if (lastChngKbdDelta < 750) {
	unsigned wait;
	/*
         * We need to guarantee at least 750 milliseconds between
	 * calls to KIOCTRANS. YUCK!
	 */
	wait = (750L - lastChngKbdDelta) * 1000L;
        usleep (wait);
        gettimeofday(&tv, (struct timezone *) NULL);
    }
    lastChngKbdTransTv = tv;
}

/*
 * Convert case, assuming Latin-1 alphabet only
 */
static void ConvertCase(sym, lower, upper)
    register KeySym sym;
    KeySym *lower;
    KeySym *upper;
{
    *lower = sym;
    *upper = sym;
    if ((sym >= XK_A) && (sym <= XK_Z))
	*lower += (XK_a - XK_A);
    else if ((sym >= XK_a) && (sym <= XK_z))
	*upper -= (XK_a - XK_A);
    else if ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
	*lower += (XK_agrave - XK_Agrave);
    else if ((sym >= XK_agrave) && (sym <= XK_odiaeresis))
	*upper -= (XK_agrave - XK_Agrave);
    else if ((sym >= XK_Ooblique) && (sym <= XK_Thorn))
	*lower += (XK_oslash - XK_Ooblique);
    else if ((sym >= XK_oslash) && (sym <= XK_thorn))
	*upper -= (XK_oslash - XK_Ooblique);
}

static void SwapKeys()
{
    int i;
    KeySym k;

    for (i = 2; i < sunConKeySyms->maxKeyCode * sunConKeySyms->mapWidth; i++)
	if (sunConKeySyms->map[i] == XK_L1 ||
	    sunConKeySyms->map[i] == XK_L2 ||
	    sunConKeySyms->map[i] == XK_L3 ||
	    sunConKeySyms->map[i] == XK_L4 ||
	    sunConKeySyms->map[i] == XK_L5 ||
	    sunConKeySyms->map[i] == XK_L6 ||
	    sunConKeySyms->map[i] == XK_L7 ||
	    sunConKeySyms->map[i] == XK_L8 ||
	    sunConKeySyms->map[i] == XK_L9 ||
	    sunConKeySyms->map[i] == XK_L10) {
	    /* yes, I could have done a clever two line swap! */
	    k = sunConKeySyms->map[i - 2];
	    sunConKeySyms->map[i - 2] = sunConKeySyms->map[i];
	    sunConKeySyms->map[i] = k;
	}
}

/*-
 *-----------------------------------------------------------------------
 * sunKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
int
sunKbdProc (pKeyboard, what)
    DevicePtr	  pKeyboard;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
{
    KbPrivPtr pPriv;
    int kbdFd, i;
    extern int AddEnabledDevice(), RemoveEnabledDevice();

    switch (what) {
    case DEVICE_INIT:
	if (pKeyboard != LookupKeyboardDevice()) {
	    ErrorF ("Cannot open non-system keyboard\n");
	    return (!Success);
	}
	    
	/*
	 * First open and find the current state of the keyboard.
	 */
/*
 * The Sun 386i has system include files that preclude this pre SunOS 4.1
 * test for the presence of a type 4 keyboard however it really doesn't
 * matter since no 386i has ever been shipped with a type 3 keyboard.
 * SunOS 4.1 no longer needs this kludge.
 */
#if !defined(i386) && !defined(KIOCGKEY)
#define TYPE4KEYBOARDOVERRIDE
#endif
	if (sysKbPriv.fd >= 0) {
	    kbdFd = sysKbPriv.fd;
	} else {
	    kbdFd = open ("/dev/kbd", O_RDWR, 0);
	    if (kbdFd == -1) {
		Error ("Opening /dev/kbd");
		return (!Success);
	    }
	    sysKbPriv.fd = kbdFd;
	    kbdWait();
	    (void) ioctl (kbdFd, KIOCTYPE, &sysKbPriv.type);
#ifdef TYPE4KEYBOARDOVERRIDE
            /*
             * Magic. Look for a key which is non-existent on a real type
             * 3 keyboard but does exist on a type 4 keyboard.
             */
	    if (sysKbPriv.type == KB_SUN3) {
		struct kiockey key;

		key.kio_tablemask = 0;
		key.kio_station = 118;
		if (ioctl(kbdFd, KIOCGETKEY, &key) == -1) {
		    Error( "ioctl KIOCGETKEY" );
		    FatalError("Can't KIOCGETKEY on fd %d\n", kbdFd);
                }
                if (key.kio_entry != HOLE)
		    sysKbPriv.type = KB_SUN4;
	    }
#endif
	    switch (sysKbPriv.type) {
	    case KB_SUN2:
	    case KB_SUN3:
	    case KB_SUN4: break;
	    default: FatalError ("Unsupported keyboard type %d\n", 
				 sysKbPriv.type);
	    }

	    if (sysKbPriv.type == KB_SUN4) {
		(void) ioctl (kbdFd, KIOCLAYOUT, &sysKbPriv.layout);
		if (sysKbPriv.layout < 0 ||
		    sysKbPriv.layout > sunMaxLayout ||
		    sunType4KeyMaps[sysKbPriv.layout] == NULL)
		    FatalError ("Unsupported keyboard type 4 layout %d\n",
				sysKbPriv.layout);
		sunKeySyms[KB_SUN4].map = sunType4KeyMaps[sysKbPriv.layout];
		sunModMaps[KB_SUN4] = sunType4ModMaps[sysKbPriv.layout];
	    }

	    if (fcntl(kbdFd, F_SETFL, FNDELAY | FASYNC) == -1
#ifdef SVR4
		|| ioctl(kbdFd, I_SETSIG, S_INPUT | S_HIPRI) == -1)
#else
		|| fcntl(kbdFd, F_SETOWN, getpid()) == -1)
#endif
		FatalError("can't set up kbd\n");

	    /*
	     * Perform final initialization of the system private keyboard
	     * structure and fill in various slots in the device record
	     * itself which couldn't be filled in before.
	     */
	    (void) memset ((void *) defaultKeyboardControl.autoRepeats,
			   ~0, sizeof defaultKeyboardControl.autoRepeats);
	    autoRepeatKeyDown = 0;

	    /*
	     * Initialize the keysym map
	     */
	    sunConKeySyms = &sunKeySyms[sysKbPriv.type];

	    /*
	     * Create and initialize the modifier map.
	     */
	    sunConModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) bzero(sunConModMap, MAP_LENGTH);
	    for(i=0; sunModMaps[sysKbPriv.type][i].key != 0; i++)
		sunConModMap[sunModMaps[sysKbPriv.type][i].key + 
			     MIN_KEYCODE - sunConKeySyms->minKeyCode] = 
		    sunModMaps[sysKbPriv.type][i].modifiers;

	    if (sysKbPriv.type == KB_SUN4 && sunSwapLkeys)
		SwapKeys();
	    /*
	     * ensure that the keycodes on the wire are >= MIN_KEYCODE
	     * and <= MAX_KEYCODE
	     */
	    if (sunConKeySyms->minKeyCode < MIN_KEYCODE) {
		sysKbPriv.offset = MIN_KEYCODE - sunConKeySyms->minKeyCode;
		sunConKeySyms->minKeyCode += sysKbPriv.offset;
		sunConKeySyms->maxKeyCode += sysKbPriv.offset;
	    }
	    if (sunConKeySyms->maxKeyCode > MAX_KEYCODE)
		sunConKeySyms->maxKeyCode = MAX_KEYCODE;
	}
	pKeyboard->devicePrivate = (pointer)&sysKbPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct (pKeyboard, sunConKeySyms, sunConModMap,
				  bell, kbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	kbdFd = pPriv->fd;

	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
	if (sunChangeKbdTranslation(pKeyboard,TRUE) == -1)
	    FatalError("Can't set keyboard translation\n");
	(void) AddEnabledDevice(kbdFd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	kbdFd = pPriv->fd;
	/*
	 * Restore original keyboard directness and translation.
	 */
	if (sunChangeKbdTranslation(pKeyboard,FALSE) == -1)
	    FatalError("Can't reset keyboard translation\n");
	RemoveEnabledDevice(kbdFd);
#if 0
	xfree(sunConModMap);
#endif
	pKeyboard->on = FALSE;
	break;
    default:
	FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * bell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	Ring the keyboard bell for an amount of time proportional to
 *	"loudness."
 *
 * Side Effects:
 *	None, really...
 *
 *-----------------------------------------------------------------------
 */
static void bell (loudness, pKeyboard)
    int	    	  loudness;	    /* Percentage of full volume */
    DevicePtr	  pKeyboard;	    /* Keyboard to ring */
{
    KbPrivPtr	  pPriv = (KbPrivPtr) pKeyboard->devicePrivate;
    int	  	  kbdCmd;   	    /* Command to give keyboard */
    int	 	  kbdOpenedHere; 
 
    if (loudness == 0) {
 	return;
    }

    kbdOpenedHere = (pPriv->fd == -1);
    if ( kbdOpenedHere ) {
	pPriv->fd = open("/dev/kbd", O_RDWR, 0);
	if (pPriv->fd == -1) {
	    ErrorF("bell: can't open keyboard\n");
	    return;
	}
    }	
 
    kbdCmd = KBD_CMD_BELL;
    if (ioctl (pPriv->fd, KIOCCMD, &kbdCmd) == -1) {
 	Error("Failed to activate bell");
	goto bad;
    }
 
    /*
     * Leave the bell on for a while == duration (ms) proportional to
     * loudness desired with a 10 thrown in to convert from ms to usecs.
     */
    usleep (pPriv->ctrl->bell_duration * 1000);
 
    kbdCmd = KBD_CMD_NOBELL;
    if (ioctl (pPriv->fd, KIOCCMD, &kbdCmd) == -1) {
	Error ("Failed to deactivate bell");
	goto bad;
    }

bad:
    if ( kbdOpenedHere ) {
	(void) close(pPriv->fd);
	pPriv->fd = -1;
    }
}

/*
 * The LEDs are coded in the ctrl->leds byte as their real values
 * as per /usr/include/sundev/kbd.h
 */
void sunKbdSetLights (pKeyboard)
    DevicePtr	pKeyboard;
{
    KbPrivPtr	pPriv = (KbPrivPtr) pKeyboard->devicePrivate;
    char	request;

    request = pPriv->ctrl->leds;

#ifdef KIOCSLED /* { */
    if (ioctl (pPriv->fd, KIOCSLED, &request) == -1)
	Error("Failed to set keyboard lights");
#endif /* } */
}

/*-
 *-----------------------------------------------------------------------
 * kbdCtrl --
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

static void kbdCtrl (pKeyboard, ctrl)
    DevicePtr	  pKeyboard;	    /* Keyboard to alter */
    KeybdCtrl     *ctrl;
{
    KbPrivPtr	  pPriv = (KbPrivPtr) pKeyboard->devicePrivate;
    int	 	  kbdOpenedHere; 
    int		  i;

    kbdOpenedHere = ( pPriv->fd < 0 );
    if ( kbdOpenedHere ) {
	if ((pPriv->fd = open("/dev/kbd", O_WRONLY, 0)) == -1) {
	    Error("kbdCtrl: can't open keyboard");
	    return;
	}
    }

    if (ctrl->click != pPriv->ctrl->click)
    {
    	int kbdClickCmd;

	pPriv->ctrl->click = ctrl->click;
	kbdClickCmd = pPriv->ctrl->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (pPriv->fd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
 
    if (ctrl->leds != pPriv->ctrl->leds)
    {
	pPriv->ctrl->leds = ctrl->leds & SUN_LED_MASK;
	sunKbdSetLights (pKeyboard);
    }

    pPriv->ctrl->bell = ctrl->bell;
    pPriv->ctrl->bell_pitch = ctrl->bell_pitch;
    pPriv->ctrl->bell_duration = ctrl->bell_duration;
    pPriv->ctrl->autoRepeat = ctrl->autoRepeat;
    for (i = 0; i < sizeof ctrl->autoRepeats / sizeof ctrl->autoRepeats[0]; i++)
	pPriv->ctrl->autoRepeats[i] = ctrl->autoRepeats[i];

    if ( kbdOpenedHere ) {
	(void) close(pPriv->fd);
	pPriv->fd = -1;
    }
}

void sunKbdModLight (pKeyboard, on, led)
    DevicePtr	pKeyboard;
    Bool	on;
    int		led;
{
    KbPrivPtr	pPriv = (KbPrivPtr) pKeyboard->devicePrivate;

    if(on)
	pPriv->ctrl->leds |= led;
    else
	pPriv->ctrl->leds &= ~led;
    sunKbdSetLights (pKeyboard);
}

/*-
 *-----------------------------------------------------------------------
 * kbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
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
static Firm_event *kbdGetEvents (pKeyboard, pNumEvents, pAgain)
    DevicePtr	  pKeyboard;	    /* Keyboard to read */
    int	    	  *pNumEvents;	    /* Place to return number of events */
    Bool	  *pAgain;	    /* whether more might be available */
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    KbPrivPtr	  pPriv;
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    pPriv = (KbPrivPtr) pKeyboard->devicePrivate;
    if ((nBytes = read (pPriv->fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * kbdEnqueueEvent --
 *
 * Results:
 *
 * Side Effects:
 *
 * Caveat:
 *      To reduce duplication of code and logic (and therefore bugs), the
 *      sunwindows version of kbd processing (kbdEnqueueEventSunWin())
 *      counterfeits a firm event and calls this routine.  This
 *      couunterfeiting relies on the fact this this routine only looks at the
 *      id, time, and value fields of the firm event which it is passed.  If
 *      this ever changes, the kbdEnqueueEventSunWin will also have to
 *      change.
 *
 *-----------------------------------------------------------------------
 */

static xEvent	autoRepeatEvent;

static void kbdEnqueueEvent (pKeyboard, fe)
    DevicePtr	  pKeyboard;
    Firm_event	  *fe;
{
    xEvent		xE;
    KbPrivPtr		pPriv;
    BYTE		*c;
    BYTE		key;
    CARD8		keyModifiers;
    KeySym		ksym;
    int			led;

    key = (fe->id & 0x7f) + sysKbPriv.offset;
    pPriv = (KbPrivPtr)pKeyboard->devicePrivate;

    keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (key == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }

    xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);
    xE.u.u.type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = key;

    /* 
     * toggling keys are usually found in the first column, but Scroll_Lock
     * is in the second column on Type_4 keyboards. So for the moment
     * anyway, we'll only be lighting the Scroll_Lock LED on Type_5 kbds.
     * Who knows what Scroll_Lock is used for anyway?
     */
    ksym = sunConKeySyms->map[(fe->id - 1) * sunConKeySyms->mapWidth];

    /*
     * Toggle functionality is hardcoded. This is achieved by always
     * discarding KeyReleases on these keys, and converting every other
     * KeyPress into a KeyRelease. There is a more generic way to test
     * for Caps_Lock, but since there is currently never more than one
     * locking key on Sun keyboards, we'll just take the simple way.
     *
     * Note that the "support" here does nothing more than toggle the 
     * LEDS.  Caps_Lock has the LockMask attribute, and is handled thusly.
     * Xlib and Xt now grok the Num_Lock key and are handled accordingly.
     * Compose processing is handled by an input method. The input method
     * can turn the LED off when it is appropriate to, otherwise the LED
     * will remain lit until the user turns it off by pressing the Compose
     * key again.  Scroll_Lock is a Ginsu feature -- the LED is toggled on 
     * and off. Who knows how or where it's actually used.
     */
    if (xE.u.u.type == KeyRelease 
	&& (ksym == XK_Num_Lock 
	|| ksym == XK_Scroll_Lock 
	|| ksym == SunXK_Compose
	|| ksym == XK_Caps_Lock)) 
	return;

    if ((ksym == XK_Num_Lock && pPriv->ctrl->leds & LED_NUM_LOCK) 
	|| (ksym == XK_Scroll_Lock && pPriv->ctrl->leds & LED_SCROLL_LOCK) 
	|| (ksym == SunXK_Compose && pPriv->ctrl->leds & LED_COMPOSE)
	|| (ksym == XK_Caps_Lock && pPriv->ctrl->leds & LED_CAPS_LOCK))
	xE.u.u.type = KeyRelease;

    if (ksym == XK_Num_Lock)
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_NUM_LOCK);
    else if (ksym == XK_Scroll_Lock)
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_SCROLL_LOCK);
    else if (ksym == SunXK_Compose)
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_COMPOSE);
    else if (ksym == XK_Caps_Lock)
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_CAPS_LOCK);
    else if ((xE.u.u.type == KeyPress) && (keyModifiers == 0)) {
	/* initialize new AutoRepeater event & mark AutoRepeater on */
	autoRepeatEvent = xE;
	autoRepeatFirst = TRUE;
	autoRepeatKeyDown++;
	autoRepeatLastKeyDownTv = fe->time;
    }
    mieqEnqueue (&xE);
}

void sunEnqueueAutoRepeat ()
{
#ifndef i386 /* { */
    sigset_t holdmask;
#else /* }{ */
    int oldmask;
#endif /* } */
    int	delta;
    int	i, mask;

    if (sysKbPriv.ctrl->autoRepeat != AutoRepeatModeOn) {
	autoRepeatKeyDown = 0;
	return;
    }
    i=(autoRepeatEvent.u.u.detail >> 3);
    mask=(1 << (autoRepeatEvent.u.u.detail & 7));
    if (!(sysKbPriv.ctrl->autoRepeats[i] & mask)) {
	autoRepeatKeyDown = 0;
	return;
    }

    /*
     * Generate auto repeat event.	XXX one for now.
     * Update time & pointer location of saved KeyPress event.
     */

    delta = TVTOMILLI(autoRepeatDeltaTv);
    autoRepeatFirst = FALSE;

    /*
     * Fake a key up event and a key down event
     * for the last key pressed.
     */
    autoRepeatEvent.u.keyButtonPointer.time += delta;
    autoRepeatEvent.u.u.type = KeyRelease;

    /*
     * hold off any more inputs while we get these safely queued up
     * further SIGIO are 
     */
#ifndef i386 /* { */
    (void) sigemptyset (&holdmask);
#ifdef SVR4
    (void) sigaddset (&holdmask, SIGPOLL);
#else
    (void) sigaddset (&holdmask, SIGIO);
#endif
    (void) sigprocmask (SIG_BLOCK, &holdmask, (sigset_t *)NULL);
    mieqEnqueue (&autoRepeatEvent);
    autoRepeatEvent.u.u.type = KeyPress;
    mieqEnqueue (&autoRepeatEvent);
    (void) sigprocmask (SIG_UNBLOCK, &holdmask, (sigset_t *)NULL);
#else /* }{ */
    oldmask = sigblock (sigmask (SIGIO));
    mieqEnqueue (&autoRepeatEvent);
    autoRepeatEvent.u.u.type = KeyPress;
    mieqEnqueue (&autoRepeatEvent);
    sigsetmask (oldmask);
#endif /* } */

    /* Update time of last key down */
    tvplus(autoRepeatLastKeyDownTv, autoRepeatLastKeyDownTv, 
		    autoRepeatDeltaTv);
}

/*-
 *-----------------------------------------------------------------------
 * sunChangeKbdTranslation
 *	Makes operating system calls to set keyboard translation 
 *	and direction on or off.
 *
 * Results:
 *	-1 if failure, else 0.
 *
 * Side Effects:
 * 	Changes kernel management of keyboard.
 *
 *-----------------------------------------------------------------------
 */
int sunChangeKbdTranslation(pKeyboard,makeTranslated)
    DevicePtr pKeyboard;
    Bool makeTranslated;
{   
    KbPrivPtr	pPriv;
    int 	kbdFd;
    int 	tmp;
    int		kbdOpenedHere;
#ifndef i386 /* { */
    sigset_t	hold_mask, old_mask;
#else /* }{ */
    int		old_mask;
#endif /* } */
    int		toread;
    char	junk[8192];
    int		ret = 0;

#ifndef i386 /* { */
    (void) sigfillset(&hold_mask);
    (void) sigprocmask(SIG_BLOCK, &hold_mask, &old_mask);
#else /* }{ */
    old_mask = sigblock (~0);
#endif /* } */
    kbdWait();
    kbdFd = -1;
    if (pKeyboard) {
    	pPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	if (pPriv)
	    kbdFd = pPriv->fd;
    }
    kbdOpenedHere = (kbdFd == -1);
    if (kbdOpenedHere) {
	if ((kbdFd = open("/dev/kbd", O_RDONLY, 0)) == -1) {
	    Error("sunChangeKbdTranslation: Can't open keyboard" );
	    ret = -1;
	    goto bad;
	}
    }
    if (makeTranslated) {
        /*
         * Next set the keyboard into "direct" mode and turn on
         * event translation. If either of these fails, we can't go
         * on.
         */
	tmp = 1;
	if (ioctl (kbdFd, KIOCSDIRECT, &tmp) == -1) {
	    Error ("Setting keyboard direct mode");
	    ret = -1;
	    goto bad;
	}
	tmp = TR_UNTRANS_EVENT;
	if (ioctl (kbdFd, KIOCTRANS, &tmp) == -1) {
	    Error ("Setting keyboard translation");
	    ErrorF ("sunChangeKbdTranslation: kbdFd=%d\n",kbdFd);
	    ret = -1;
	    goto bad;
	}
    } else {
        /*
         * Next set the keyboard into "indirect" mode and turn off
         * event translation.
         */
	tmp = 0;
	(void)ioctl (kbdFd, KIOCSDIRECT, &tmp);
	tmp = TR_ASCII;
	(void)ioctl (kbdFd, KIOCTRANS, &tmp);
    }
    if (ioctl (kbdFd, FIONREAD, &toread) != -1 && toread > 0) {
	while (toread) {
	    tmp = toread;
	    if (toread > sizeof (junk))
		tmp = sizeof (junk);
	    (void) read (kbdFd, junk, tmp);
	    toread -= tmp;
	}
    }
bad:
    if (kbdOpenedHere)
	(void) close(kbdFd);
#ifndef i386 /* { */
    (void) sigprocmask(SIG_SETMASK, &old_mask, (sigset_t *)NULL);
#else /* }{ */
    sigsetmask (old_mask);
#endif /* } */
    return ret;
}

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    BYTE    key;
    DevicePtr	pDev;
{
    return TRUE;
}

static KeybdCtrl *pKbdCtrl = (KeybdCtrl *) 0;

/*ARGSUSED*/
void sunBlockHandler(nscreen, pbdata, pptv, pReadmask)
    int nscreen;
    pointer pbdata;
    struct timeval **pptv;
    pointer pReadmask;
{
    static struct timeval artv = { 0, 0 };	/* autorepeat timeval */

    if (!autoRepeatKeyDown)
	return;

    if (pKbdCtrl == (KeybdCtrl *) 0)
	pKbdCtrl = ((KbPrivPtr) LookupKeyboardDevice()->devicePrivate)->ctrl;

    if (pKbdCtrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatFirst == TRUE)
	artv.tv_usec = sunAutoRepeatInitiate;
    else
	artv.tv_usec = sunAutoRepeatDelay;
    *pptv = &artv;

}

/*ARGSUSED*/
void sunWakeupHandler(nscreen, pbdata, err, pReadmask)
    int nscreen;
    pointer pbdata;
    unsigned long err;
    pointer pReadmask;
{
    struct timeval tv;

    if (pKbdCtrl == (KeybdCtrl *) 0)
	pKbdCtrl = ((KbPrivPtr) LookupKeyboardDevice()->devicePrivate)->ctrl;

    if (pKbdCtrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatKeyDown) {
	gettimeofday(&tv, (struct timezone *) NULL);
	tvminus(autoRepeatDeltaTv, tv, autoRepeatLastKeyDownTv);
	if (autoRepeatDeltaTv.tv_sec > 0 ||
			(!autoRepeatFirst && autoRepeatDeltaTv.tv_usec >
				sunAutoRepeatDelay) ||
			(autoRepeatDeltaTv.tv_usec >
				sunAutoRepeatInitiate))
		autoRepeatReady++;
    }
    
    if (autoRepeatReady)
    {
	sunEnqueueAutoRepeat ();
	autoRepeatReady = 0;
    }
}
