/* $XConsortium: sunKbd.c,v 5.37 94/02/01 14:57:26 kaleb Exp $ */
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

#ifndef XKB
#define AUTOREPEAT_INITIATE	200
#define AUTOREPEAT_DELAY	50
#endif

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

extern void	ProcessInputEvents();
extern void	miPointerPosition();

#ifndef XKB
long	  	  sunAutoRepeatInitiate = 1000 * AUTOREPEAT_INITIATE;
long	  	  sunAutoRepeatDelay = 1000 * AUTOREPEAT_DELAY;
#endif

#ifndef XKB
static int		autoRepeatKeyDown = 0;
static int		autoRepeatReady;
static int		autoRepeatFirst;
static struct timeval	autoRepeatLastKeyDownTv;
static struct timeval	autoRepeatDeltaTv;
#endif

static KbPrivRec  	sysKbPriv = {
    -1,			/* Type */
    -1,			/* Layout */
    0,			/* click */
    (Leds)0,		/* leds */
};

static void kbdWait()
{
    static struct timeval lastChngKbdTransTv;
    struct timeval tv;
    struct timeval lastChngKbdDeltaTv;
    int lastChngKbdDelta;

    GETTIMEOFDAY(&tv);
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
        GETTIMEOFDAY(&tv);
    }
    lastChngKbdTransTv = tv;
}

static void SwapKeys(keysyms)
    KeySymsRec* keysyms;
{
    unsigned int i;
    KeySym k;

    for (i = 2; i < keysyms->maxKeyCode * keysyms->mapWidth; i++)
	if (keysyms->map[i] == XK_L1 ||
	    keysyms->map[i] == XK_L2 ||
	    keysyms->map[i] == XK_L3 ||
	    keysyms->map[i] == XK_L4 ||
	    keysyms->map[i] == XK_L5 ||
	    keysyms->map[i] == XK_L6 ||
	    keysyms->map[i] == XK_L7 ||
	    keysyms->map[i] == XK_L8 ||
	    keysyms->map[i] == XK_L9 ||
	    keysyms->map[i] == XK_L10) {
	    /* yes, I could have done a clever two line swap! */
	    k = keysyms->map[i - 2];
	    keysyms->map[i - 2] = keysyms->map[i];
	    keysyms->map[i] = k;
	}
}

static void SetLights (ctrl)
    KeybdCtrl*	ctrl;
{
#ifdef KIOCSLED
    static unsigned char led_tab[16] = {
	0,
	LED_NUM_LOCK,
	LED_SCROLL_LOCK,
	LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_COMPOSE,
	LED_COMPOSE | LED_NUM_LOCK,
	LED_COMPOSE | LED_SCROLL_LOCK,
	LED_COMPOSE | LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK,
	LED_CAPS_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_SCROLL_LOCK,
	LED_CAPS_LOCK | LED_SCROLL_LOCK | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE,
	LED_CAPS_LOCK | LED_COMPOSE | LED_NUM_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE | LED_SCROLL_LOCK,
	LED_CAPS_LOCK | LED_COMPOSE | LED_SCROLL_LOCK | LED_NUM_LOCK
    };
    if (ioctl (sunKbdFd, KIOCSLED, (caddr_t)&led_tab[ctrl->leds & 0x0f]) == -1)
	Error("Failed to set keyboard lights");
#endif
}


static void ModLight (device, on, led)
    DeviceIntPtr device;
    Bool	on;
    int		led;
{
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    KbPrivPtr	pPriv = (KbPrivPtr) device->public.devicePrivate;

    if(on) {
	ctrl->leds |= led;
	pPriv->leds |= led;
    } else {
	ctrl->leds &= ~led;
	pPriv->leds &= ~led;
    }
    SetLights (ctrl);
}

/*-
 *-----------------------------------------------------------------------
 * sunBell --
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

#if NeedFunctionPrototypes
static void sunBell (
    int		    percent,
    DeviceIntPtr    device,
    pointer	    ctrl,
    int		    unused)
#else
static void sunBell (percent, device, ctrl, unused)
    int		    percent;	    /* Percentage of full volume */
    DeviceIntPtr    device;	    /* Keyboard to ring */
    pointer	    ctrl;
    int		    unused;
#endif
{
    int		    kbdCmd;   	    /* Command to give keyboard */
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
 
    if (percent == 0 || kctrl->bell == 0)
 	return;

    kbdCmd = KBD_CMD_BELL;
    if (ioctl (sunKbdFd, KIOCCMD, &kbdCmd) == -1) {
 	Error("Failed to activate bell");
	return;
    }
    usleep (kctrl->bell_duration * 1000);
    kbdCmd = KBD_CMD_NOBELL;
    if (ioctl (sunKbdFd, KIOCCMD, &kbdCmd) == -1)
	Error ("Failed to deactivate bell");
}

static void EnqueueEvent (xE)
    xEvent* xE;
{
#ifndef i386
    sigset_t holdmask;

#ifdef SVR4
    (void) sigaddset (&holdmask, SIGPOLL);
#else
    (void) sigaddset (&holdmask, SIGIO);
#endif
    (void) sigprocmask (SIG_BLOCK, &holdmask, (sigset_t*)NULL);
    mieqEnqueue (xE);
    (void) sigprocmask (SIG_UNBLOCK, &holdmask, (sigset_t*)NULL);
#else
    int oldmask = sigblock (sigmask (SIGIO));

    mieqEnqueue (xE);
    sigsetmask (oldmask);
#endif
}

#ifndef XKB

#define XLED_NUM_LOCK    0x1
#define XLED_COMPOSE     0x4
#define XLED_SCROLL_LOCK 0x2
#define XLED_CAPS_LOCK   0x8

static KeyCode LookupKeyCode (keysym, keysymsrec)
    KeySym keysym;
    KeySymsPtr keysymsrec;
{
    KeyCode i;
    int ii, index = 0;

    for (i = keysymsrec->minKeyCode; i < keysymsrec->maxKeyCode; i++)
	for (ii = 0; ii < keysymsrec->mapWidth; ii++)
	    if (keysymsrec->map[index++] == keysym)
		return i;
}

static void pseudoKey(device, down, keycode)
    DeviceIntPtr device;
    Bool down;
    KeyCode keycode;
{
    int bit;
    CARD8 modifiers;
    CARD16 mask;
    BYTE* kptr;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    modifiers = device->key->modifierMap[keycode];
    if (down) {
	/* fool dix into thinking this key is now "down" */
	int i;
	*kptr |= bit;
	device->key->prev_state = device->key->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    if (mask & modifiers) {
		device->key->modifierKeyCount[i]++;
		device->key->state += mask;
		modifiers &= ~mask;
	    }
    } else {
	/* fool dix into thinking this key is now "up" */
	if (*kptr & bit) {
	    int i;
	    *kptr &= ~bit;
	    device->key->prev_state = device->key->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
		if (mask & modifiers) {
		    if (--device->key->modifierKeyCount[i] <= 0) {
			device->key->state &= ~mask;
			device->key->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	}
    }
}

static void DoLEDs(device, ctrl, pPriv)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl* ctrl;
    KbPrivPtr  pPriv;
{
    if ((ctrl->leds & XLED_CAPS_LOCK) && !(pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_CAPS_LOCK) && (pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_NUM_LOCK) && !(pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_NUM_LOCK) && (pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_SCROLL_LOCK) && !(pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_SCROLL_LOCK) && (pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_COMPOSE) && !(pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, TRUE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_COMPOSE) && (pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, FALSE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));

    pPriv->leds = ctrl->leds & 0x0f;
    SetLights (ctrl);
}
#endif

/*-
 *-----------------------------------------------------------------------
 * sunKbdCtrl --
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

#if NeedFunctionPrototypes
static void sunKbdCtrl (
    DeviceIntPtr    device,
    KeybdCtrl*	    ctrl)
#else
static void sunKbdCtrl (device, ctrl)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl*	    ctrl;
#endif
{
    KbPrivPtr pPriv = (KbPrivPtr) device->public.devicePrivate;

    if (sunKbdFd < 0) return;

    if (ctrl->click != pPriv->click) {
    	int kbdClickCmd;

	pPriv->click = ctrl->click;
	kbdClickCmd = pPriv->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (sunKbdFd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
#ifndef XKB
    if (pPriv->type == KB_SUN4 && pPriv->leds != ctrl->leds & 0x0f)
	DoLEDs(device, ctrl, pPriv);
#endif
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

#if NeedFunctionPrototypes
int sunKbdProc (
    DeviceIntPtr  device,
    int	    	  what)
#else
int sunKbdProc (device, what)
    DeviceIntPtr  device;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
#endif
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    KbPrivPtr pPriv;
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

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
	if (!workingKeySyms) {
	    kbdWait();
	    (void) ioctl (sunKbdFd, KIOCTYPE, &sysKbPriv.type);
#ifdef TYPE4KEYBOARDOVERRIDE
            /*
             * Magic. Look for a key which is non-existent on a real type
             * 3 keyboard but does exist on a type 4 keyboard.
             */
	    if (sysKbPriv.type == KB_SUN3) {
		struct kiockey key;

		key.kio_tablemask = 0;
		key.kio_station = 118;
		if (ioctl(sunKbdFd, KIOCGETKEY, &key) == -1) {
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
		(void) ioctl (sunKbdFd, KIOCLAYOUT, &sysKbPriv.layout);
		if (sysKbPriv.layout < 0 ||
		    sysKbPriv.layout > sunMaxLayout ||
		    sunType4KeyMaps[sysKbPriv.layout] == NULL)
		    FatalError ("Unsupported keyboard type 4 layout %d\n",
				sysKbPriv.layout);
		sunKeySyms[KB_SUN4].map = sunType4KeyMaps[sysKbPriv.layout];
		sunModMaps[KB_SUN4] = sunType4ModMaps[sysKbPriv.layout];
	    }

	    /*
	     * Perform final initialization of the system private keyboard
	     * structure and fill in various slots in the device record
	     * itself which couldn't be filled in before.
	     */
	    (void) memset ((void *) defaultKeyboardControl.autoRepeats,
			   ~0, sizeof defaultKeyboardControl.autoRepeats);
#ifndef XKB
	    autoRepeatKeyDown = 0;
#endif
	    /*
	     * Initialize the keysym map
	     */
	    workingKeySyms = &sunKeySyms[sysKbPriv.type];

	    /*
	     * Create and initialize the modifier map.
	     */
	    if (!workingModMap) {
		workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
		(void) memset(workingModMap, 0, MAP_LENGTH);
		for(i=0; sunModMaps[sysKbPriv.type][i].key != 0; i++)
		    workingModMap[sunModMaps[sysKbPriv.type][i].key + MIN_KEYCODE] = 
			sunModMaps[sysKbPriv.type][i].modifiers;
	    }

	    if (sysKbPriv.type == KB_SUN4 && sunSwapLkeys)
		SwapKeys(workingKeySyms);
	    /*
	     * ensure that the keycodes on the wire are >= MIN_KEYCODE
	     * and <= MAX_KEYCODE
	     */
	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}
	pKeyboard->devicePrivate = (pointer)&sysKbPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct(pKeyboard, 
				 workingKeySyms, workingModMap,
				 sunBell, sunKbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
	if (sunChangeKbdTranslation(pKeyboard,TRUE) == -1)
	    FatalError("Can't set keyboard translation\n");
	(void) AddEnabledDevice(sunKbdFd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (KbPrivPtr)pKeyboard->devicePrivate;
	if (pPriv->type == KB_SUN4) {
	    /* dumb bug in Sun's keyboard! Turn off LEDS before resetting */
	    pPriv->leds = 0;
	    ctrl->leds = 0;
	    SetLights(ctrl);
	}
	/*
	 * Restore original keyboard directness and translation.
	 */
	if (sunChangeKbdTranslation(pKeyboard,FALSE) == -1)
	    FatalError("Can't reset keyboard translation\n");
	RemoveEnabledDevice(sunKbdFd);
	pKeyboard->on = FALSE;
	break;
    default:
	FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * sunKbdGetEvents --
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

#if NeedFunctionPrototypes
Firm_event* sunKbdGetEvents (
    int	    	  *pNumEvents,
    Bool	  *pAgain)
#else
Firm_event* sunKbdGetEvents (pNumEvents, pAgain)
    int	    	  *pNumEvents;	    /* Place to return number of events */
    Bool	  *pAgain;	    /* whether more might be available */
#endif
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (sunKbdFd, evBuf, sizeof(evBuf))) == -1) {
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
 * sunKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */
#ifndef XKB
static xEvent	autoRepeatEvent;

static Bool DoSpecialKeys(device, xE, fe)
    DeviceIntPtr  device;
    xEvent*       xE;
    Firm_event* fe;
{
    int	shift_index, map_index, bit;
    KeySym ksym;
    BYTE* kptr;
    KbPrivPtr pPriv = (KbPrivPtr)device->public.devicePrivate;
    BYTE keycode = xE->u.u.detail;
    CARD8 keyModifiers = device->key->modifierMap[keycode];

    /* look up the present idea of the keysym */
    shift_index = 0;
    if (device->key->state & ShiftMask) 
	shift_index ^= 1;
    if (device->key->state & LockMask) 
	shift_index ^= 1;
    map_index = (fe->id - 1) * device->key->curKeySyms.mapWidth;
    ksym = device->key->curKeySyms.map[shift_index + map_index];
    if (ksym == NoSymbol)
	ksym = device->key->curKeySyms.map[map_index];

    /*
     * Toggle functionality is hardcoded. This is achieved by always
     * discarding KeyReleases on these keys, and converting every other
     * KeyPress into a KeyRelease.
     */
    if (xE->u.u.type == KeyRelease 
	&& (ksym == XK_Num_Lock 
	|| ksym == XK_Scroll_Lock 
	|| ksym == SunXK_Compose
	|| (keyModifiers & LockMask))) 
	return TRUE;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    if ((*kptr & bit) &&
	(ksym == XK_Num_Lock || ksym == XK_Scroll_Lock ||
	ksym == SunXK_Compose || (keyModifiers & LockMask)))
	xE->u.u.type = KeyRelease;

    if (ksym == XK_Num_Lock) {
	if (pPriv->type == KB_SUN4)
	    ModLight (device, xE->u.u.type == KeyPress, XLED_NUM_LOCK);
    } else if (ksym == XK_Scroll_Lock) {
	if (pPriv->type == KB_SUN4)
	    ModLight (device, xE->u.u.type == KeyPress, XLED_SCROLL_LOCK);
    } else if (ksym == SunXK_Compose) {
	if (pPriv->type == KB_SUN4)
	    ModLight (device, xE->u.u.type == KeyPress, XLED_COMPOSE);
    } else if (keyModifiers & LockMask) {
	if (pPriv->type == KB_SUN4)
	    ModLight (device, xE->u.u.type == KeyPress, XLED_CAPS_LOCK);
    } else if ((xE->u.u.type == KeyPress) && (keyModifiers == 0)) {
	/* initialize new AutoRepeater event & mark AutoRepeater on */
	autoRepeatEvent = *xE;
	autoRepeatFirst = TRUE;
	autoRepeatKeyDown++;
	autoRepeatLastKeyDownTv = fe->time;
    }
    return FALSE;
}

#endif

#if NeedFunctionPrototypes
void sunKbdEnqueueEvent (
    DeviceIntPtr  device,
    Firm_event	  *fe)
#else
void sunKbdEnqueueEvent (device, fe)
    DeviceIntPtr  device;
    Firm_event	  *fe;
#endif
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;

    keycode = (fe->id & 0x7f) + MIN_KEYCODE;

    keyModifiers = device->key->modifierMap[keycode];
#ifndef XKB
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (keycode == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }
#endif
    xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);
    xE.u.u.type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = keycode;
#ifndef XKB
    if (DoSpecialKeys(device, &xE, fe))
	return;
#endif /* ! XKB */
    mieqEnqueue (&xE);
}

#ifndef XKB
void sunEnqueueAutoRepeat ()
{
    int	delta;
    int	i, mask;
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;

    if (ctrl->autoRepeat != AutoRepeatModeOn) {
	autoRepeatKeyDown = 0;
	return;
    }
    i=(autoRepeatEvent.u.u.detail >> 3);
    mask=(1 << (autoRepeatEvent.u.u.detail & 7));
    if (!(ctrl->autoRepeats[i] & mask)) {
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
    EnqueueEvent (&autoRepeatEvent);
    autoRepeatEvent.u.u.type = KeyPress;
    EnqueueEvent (&autoRepeatEvent);

    /* Update time of last key down */
    tvplus(autoRepeatLastKeyDownTv, autoRepeatLastKeyDownTv, 
		    autoRepeatDeltaTv);
}
#endif /* ! XKB */

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
#if NeedFunctionPrototypes
int sunChangeKbdTranslation(
    DevicePtr pKeyboard,
    Bool makeTranslated)

#else
int sunChangeKbdTranslation(pKeyboard, makeTranslated)
    DevicePtr pKeyboard;
    Bool makeTranslated;
#endif
{   
    int 	tmp;
#ifndef i386 /* { */
    sigset_t	hold_mask, old_mask;
#else /* }{ */
    int		old_mask;
#endif /* } */
    int		toread;
    char	junk[8192];

#ifndef i386 /* { */
    (void) sigfillset(&hold_mask);
    (void) sigprocmask(SIG_BLOCK, &hold_mask, &old_mask);
#else /* }{ */
    old_mask = sigblock (~0);
#endif /* } */
    kbdWait();
    if (makeTranslated) {
        /*
         * Next set the keyboard into "direct" mode and turn on
         * event translation. If either of these fails, we can't go
         * on.
         */
	tmp = 1;
	if (ioctl (sunKbdFd, KIOCSDIRECT, &tmp) == -1) {
	    Error ("Setting keyboard direct mode");
	    return -1;
	}
	tmp = TR_UNTRANS_EVENT;
	if (ioctl (sunKbdFd, KIOCTRANS, &tmp) == -1) {
	    Error ("Setting keyboard translation");
	    ErrorF ("sunChangeKbdTranslation: kbdFd=%d\n",sunKbdFd);
	    return -1;
	}
    } else {
        /*
         * Next set the keyboard into "indirect" mode and turn off
         * event translation.
         */
	tmp = 0;
	(void)ioctl (sunKbdFd, KIOCSDIRECT, &tmp);
	tmp = TR_ASCII;
	(void)ioctl (sunKbdFd, KIOCTRANS, &tmp);
    }
    if (ioctl (sunKbdFd, FIONREAD, &toread) != -1 && toread > 0) {
	while (toread) {
	    tmp = toread;
	    if (toread > sizeof (junk))
		tmp = sizeof (junk);
	    (void) read (sunKbdFd, junk, tmp);
	    toread -= tmp;
	}
    }
#ifndef i386 /* { */
    (void) sigprocmask(SIG_SETMASK, &old_mask, (sigset_t *)NULL);
#else /* }{ */
    sigsetmask (old_mask);
#endif /* } */
    return 0;
}

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    BYTE    key;
    DevicePtr	pDev;
{
    return TRUE;
}

#ifndef XKB
/*ARGSUSED*/
void sunBlockHandler(nscreen, pbdata, pptv, pReadmask)
    int nscreen;
    pointer pbdata;
    struct timeval **pptv;
    pointer pReadmask;
{
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    static struct timeval artv = { 0, 0 };	/* autorepeat timeval */

    if (!autoRepeatKeyDown)
	return;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
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
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    struct timeval tv;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatKeyDown) {
	GETTIMEOFDAY(&tv);
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
#endif /* ! XKB */
