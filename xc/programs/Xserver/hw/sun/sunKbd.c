/* $XConsortium: sunKbd.c,v 5.20 93/08/06 14:29:24 kaleb Exp $ */
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
#ifndef KB_SUN4 /* { */
#define KB_SUN4		4
#endif /* } */

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

typedef struct {
    KeySym	sym1, sym2, result;
    BYTE	key;
} SunComposeMapRec;

typedef struct {
    int		trans;		/* Original translation form */
} SunKbPrivRec, *SunKbPrivPtr;

extern CARD8 *sunModMap[];
extern KeySymsRec sunKeySyms[];
extern KeySym *sunType4Maps[];
extern int sunType4MaxLayout;
extern SunKPmapRec *sunType4KPMaps[];
extern SunModmapRec *sunType4ModMaps[];
extern int *sunType4MapLen[];
extern KeySym sunType4ComposeMap[];

static CARD8 *sunConModMap = 0;
static KeySymsRec *sunConKeySyms;
static BYTE *sunNLscan;
static SunComposeMapRec *sunComposeMap;
static KeySym sunComposeBuild = 0;
static BYTE sunComposeResult = 0;
static BYTE sunComposeLast = 0;
static unsigned int sunModifiers = 0;
static Bool sunConKeySymsAlloc = FALSE;

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
static KeybdCtrl	sysKbCtrl;

static SunKbPrivRec	kbPriv;  
static KbPrivRec  	sysKbPriv = {
    -1,			/* Type of keyboard */
    -1,			/* Layout of keyboard */
    -1,			/* Descriptor open to device */
    kbdGetEvents,	/* Function to read events */
    kbdEnqueueEvent,	/* Function to enqueue an event */
    (pointer)&kbPriv,	/* Private to keyboard device */
    (Bool)0,		/* Mapped queue */
    0,			/* offset for device keycodes */
    &sysKbCtrl,		/* Initial full duration = .25 sec. */
};

/*
 * Tries to find any holes in the map that is limit elements long.
 * The search starts after the start'th element and continues
 * circularly as far as the end'th element
 */
static int findFree(map, width, start, end, limit)
KeySym *map;
int width, start, end, limit;
{
    int i=start;
    int j;

    while(1) {
	if (i == limit)
	    i=0;
	if (map[i*width] == NoSymbol)
	    if (sunProtected) {
		for(j=0; sunProtected[j]; j++)
		    if (i == sunProtected[j]-MIN_KEYCODE)
			break;
		if (i != sunProtected[j]-MIN_KEYCODE)
		    return i;
	    } else
		return i;
	if(++i == end)
	    break;
    }
    FatalError("Not enough space for keysyms in keyboard map\n");
}

/*-
 *-----------------------------------------------------------------------
 * kbdSleep --
 *
 * Workaround for serial driver kernel bug when KIOCTRANS ioctl()s
 * occur too closely together in time.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
static
void kbdSleep()
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

/*-
 *-----------------------------------------------------------------------
 * sunKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
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
sunKbdProc (pKeyboard, what)
    DevicePtr	  pKeyboard;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
{
    KbPrivPtr	  pPriv;
    register int  kbdFd;
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
#if !defined(i386) && !defined(KIOCGKEY) /* { */
#define TYPE4KEYBOARDOVERRIDE
#endif /* } */
	    if (sysKbPriv.fd >= 0) {
		kbdFd = sysKbPriv.fd;
	    } else {
		kbdFd = open ("/dev/kbd", O_RDWR, 0);
		if (kbdFd == -1) {
		    Error ("Opening /dev/kbd");
		    return (!Success);
		}
		sysKbPriv.fd = kbdFd;
		kbdSleep();
		(void) ioctl (kbdFd, KIOCTYPE, &sysKbPriv.type);
		(void) ioctl (kbdFd, KIOCLAYOUT, &sysKbPriv.layout);
		(void) ioctl (kbdFd, KIOCGTRANS, &kbPriv.trans);
#ifdef TYPE4KEYBOARDOVERRIDE /* { */
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
#endif /* } */

		if ((sysKbPriv.type < 0 || sysKbPriv.type > KB_SUN4
		    || sunKeySyms[sysKbPriv.type].map == NULL) &&
		    sysKbPriv.type != KB_SUN4)
		    FatalError("Unsupported keyboard type %d\n", 
			sysKbPriv.type);
		if (sysKbPriv.type == KB_SUN4 &&
		    (sysKbPriv.layout < 0 ||
		     sysKbPriv.layout > sunType4MaxLayout ||
		     sunType4Maps[sysKbPriv.layout] == NULL))
		    FatalError("Unsupported keyboard type 4 layout %d\n",
			sysKbPriv.layout);
		if (fcntl(kbdFd, F_SETFL, FNDELAY | FASYNC) == -1
#ifdef SVR4
		    || ioctl(kbdFd, I_SETSIG, S_INPUT | S_HIPRI) < 0)
#else
		    || fcntl(kbdFd, F_SETOWN, getpid()) == -1)
#endif
		    FatalError("can't set up kbd\n");
	    }

	    /*
	     * Perform final initialization of the system private keyboard
	     * structure and fill in various slots in the device record
	     * itself which couldn't be filled in before.
	     */
	    pKeyboard->devicePrivate = (pointer)&sysKbPriv;
	    pKeyboard->on = FALSE;
	    sysKbCtrl = defaultKeyboardControl;
	    sysKbPriv.ctrl = &sysKbCtrl;
	    (void) memset ((void *) defaultKeyboardControl.autoRepeats,
			   ~0, sizeof defaultKeyboardControl.autoRepeats);
	    autoRepeatKeyDown = 0;

	    /*
	     * Initialize the keysym map
	     */
	    if (sysKbPriv.type != KB_SUN4) {
		sunConKeySyms = &sunKeySyms[sysKbPriv.type];
		sunConModMap = sunModMap[sysKbPriv.type];
		sunDoCompose = sunDoNumlock = FALSE;
	    } else {
		/*
		 * We have a type 4 keyboard, now construct the tables
		 * according to the layout
		 */
		int i, j, numlen, clen;
		int len = *sunType4MapLen[sysKbPriv.layout];
		int maxlen;
		int lastindex = len-1;
		KeySym lsym, usym, *kmap;
		SunKPmapRec *kpmap = sunType4KPMaps[sysKbPriv.layout];
		SunModmapRec *modmap = sunType4ModMaps[sysKbPriv.layout];

		for(numlen=0; kpmap[numlen].key != 0; numlen++);
		for(clen=0; sunType4ComposeMap[clen*3] != 0; clen++);

		/*
		 * Probably we won't need all this space, be we don't know
		 * how much we need.
		 */
		sunConKeySyms = (KeySymsRec *)xalloc(sizeof(*sunConKeySyms));
		sunConKeySyms->map = (KeySym *)xalloc((len+numlen+clen)*4
							*sizeof(KeySym));
		bzero(sunConKeySyms->map, (len+numlen+clen)*4*sizeof(KeySym));
		sunConKeySyms->minKeyCode = 1;
		sunConKeySyms->mapWidth = 4;

		/*
		 * Initialize the modifier map.
		 */
		sunConModMap=(CARD8 *)xalloc(MAP_LENGTH);
		bzero(sunConModMap, MAP_LENGTH);
		for(i=0; modmap[i].key != 0; i++)
		    sunConModMap[modmap[i].key+MIN_KEYCODE-
			sunConKeySyms->minKeyCode] = modmap[i].modifiers;

		/*
		 * Initialize the bulk of the keycode/keysym table.
		 */
		kmap = sunType4Maps[sysKbPriv.layout];
		if (sunDoF11) {
		    for (i=0; i < len*sunConKeySyms->mapWidth; i++)
			switch(kmap[i]) {
			case SunXK_F36:
			    sunConKeySyms->map[i] = XK_F11;
			    break;
			case SunXK_F37:
			    sunConKeySyms->map[i] = XK_F12;
			    break;
			default:
			    sunConKeySyms->map[i] = kmap[i];
			    break;
			}
		} else
		    for (i=0; i < len*sunConKeySyms->mapWidth; i++)
			sunConKeySyms->map[i] = kmap[i];

		maxlen=len+numlen+clen;
		/* Don't use keycode 255 - breaks some R5 clients */
		if (maxlen > MAX_KEYCODE - MIN_KEYCODE)
		    maxlen=MAX_KEYCODE-MIN_KEYCODE;

		/*
		 * Add the NumLock entries for the numeric keypad.
		 */
		if (sunDoNumlock) {
		    sunNLscan = (BYTE *)xalloc((numlen+1)*2*sizeof(BYTE));
		    for(i = 0; i < numlen; i++) {
			sunNLscan[i*2] = kpmap[i].key;
			lastindex = findFree(sunConKeySyms->map, 4,
					     lastindex+1, len, maxlen);
			sunNLscan[i*2+1] = lastindex+sunConKeySyms->minKeyCode;
			sunConKeySyms->map[lastindex*4 + 0] = kpmap[i].sym1;
			sunConKeySyms->map[lastindex*4 + 1] = kpmap[i].sym2;
			sunConKeySyms->map[lastindex*4 + 2] = NoSymbol;
			sunConKeySyms->map[lastindex*4 + 3] = NoSymbol;
			/* Forget second keysym - breaks some Motif clients */
		    }
		    sunNLscan[numlen*2] = 0;
		}

		/*
		 * Add the Compose entries to the keycode/keysym table
		 */
		if (sunDoCompose) {
		    kmap = sunType4ComposeMap;
		    sunComposeMap = (SunComposeMapRec*)xalloc((clen+1)*
				     sizeof(SunComposeMapRec));
		    for(i = 0; i < clen; i++) {
			sunComposeMap[i].sym1 = kmap[i*3 + 0];
			sunComposeMap[i].sym2 = kmap[i*3 + 1];
			sunComposeMap[i].result = kmap[i*3 + 2];
			for (j = 0; j < i; j++)
			    if (sunComposeMap[j].result ==
					sunComposeMap[i].result)
				break;
			if (j == i) {
			    lastindex = findFree(sunConKeySyms->map, 4,
						 lastindex+1, len, maxlen);
			    sunComposeMap[i].key = 
				lastindex+sunConKeySyms->minKeyCode;
			    sunConKeySyms->map[lastindex*4 + 0] =
				sunConKeySyms->map[lastindex*4 + 1] =
				    kmap[i*3 + 2];
			    sunConKeySyms->map[lastindex*4 + 2] =
				sunConKeySyms->map[lastindex*4 + 3] = NoSymbol;

			    /*
			     * Only use the 2nd keysym if the shift modifier
			     * would change the resulting keysym
			     */
			    ConvertCase(sunConKeySyms->map[lastindex*4 + 0],
					   &lsym, &usym);
			    if (lsym == usym)
				sunConKeySyms->map[lastindex*4 + 1] = NoSymbol;
			} else
			    sunComposeMap[i].key = sunComposeMap[j].key;
		    }
		    sunComposeMap[clen].sym1 = NoSymbol;
		}
		
		i = maxlen - 1;
		while (i > 0 && sunConKeySyms->map[i*4] == NoSymbol)
		    i--;
		sunConKeySyms->maxKeyCode = i + 1;
		sunConKeySymsAlloc = TRUE;
	    }
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

	    InitKeyboardDeviceStruct(
		    pKeyboard,
		    sunConKeySyms,
		    sunConModMap,
		    bell,
		    kbdCtrl);
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
	    /*
	     * Free memory allocated for keymaps (if it is allocated)
	     */
	    if (sunConKeySymsAlloc) {
		xfree(sunConKeySyms->map);
		xfree(sunConKeySyms);
		xfree(sunConModMap);
		if (sunDoNumlock)
		    xfree(sunNLscan);
		if (sunDoCompose)
		    xfree(sunComposeMap);
		sunConKeySymsAlloc = FALSE;
	    }
	    pKeyboard->on = FALSE;
	    break;
    }
    return (Success);
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
    return (evBuf);
}

/*
 * This routine bears a striking resemblance to the XTranslateKey routine,
 * it has however been modified to work in the server.
 */
static KeySym TranslateKey(pKeyboard, keycode, modifiers)
DevicePtr pKeyboard;
unsigned int keycode, modifiers;
{
    KeySymsPtr ksymsrec;
    int per;
    register KeySym *syms;
    KeySym sym, lsym, usym, rsym;

    ksymsrec = &((DeviceIntPtr)pKeyboard)->key->curKeySyms;
    per = ksymsrec->mapWidth;
    syms = &ksymsrec->map[(keycode+sysKbPriv.offset-ksymsrec->minKeyCode)*per];
    while ((per > 2) && (syms[per - 1] == NoSymbol))
	per--;
    if ((per > 2) && (modifiers & Mod2Mask)) {
	syms += 2;
	per -= 2;
    }
    if (!(modifiers & ShiftMask) && !(modifiers & LockMask)) {
	if ((per == 1) || (syms[1] == NoSymbol))
	    ConvertCase(syms[0], &rsym, &usym);
	else
	    rsym = syms[0];
    } else if (!(modifiers & LockMask)) {
	if ((per == 1) || ((usym = syms[1]) == NoSymbol))
	    ConvertCase(syms[0], &lsym, &usym);
	rsym = usym;
    } else {
	if ((per == 1) || ((sym = syms[1]) == NoSymbol))
	    sym = syms[0];
	ConvertCase(sym, &lsym, &usym);
	if (!(modifiers & ShiftMask) && (sym != syms[0]) &&
	    ((sym != usym) || (lsym == usym)))
	ConvertCase(syms[0], &lsym, &usym);
	rsym = usym;
    }
    if (rsym == XK_VoidSymbol)
	rsym = NoSymbol;

    return rsym;
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
    int			k;

    key = fe->id;
    ksym = TranslateKey(pKeyboard, key, sunModifiers);
    pPriv = (KbPrivPtr)pKeyboard->devicePrivate;

    /*
     * First check if numlock is active and should affect this key
     */
    if (sunDoNumlock && pPriv->ctrl->leds & LED_NUM_LOCK)
	for(c=sunNLscan; *c!=0; c+=2)
	    if (key == *c) {
		key = *(c+1);
		break;
	    }
    /*
     * Now check for compose combinations
     */
    if (sunDoCompose && 
	pPriv->ctrl->leds & LED_COMPOSE && 
	fe->value == VKEY_DOWN &&
	!((DeviceIntPtr)pKeyboard)->key->modifierMap[key+sysKbPriv.offset]) {
	if (sunComposeBuild == NoSymbol) {
	    /* This is the first key */
	    sunComposeLast = key;
	    for(k=0; sunComposeMap[k].sym1 != NoSymbol; k++)
		if (sunComposeMap[k].sym1 = ksym || 
		    sunComposeMap[k].sym2 == ksym) {
		    sunComposeBuild = ksym;
		    break;
		}
	    if (sunComposeMap[k].sym1 == NoSymbol)
		sunKbdModLight (pKeyboard, 0, LED_COMPOSE);
	    return;
	} else {
	    /* This is the second key */
	    switch(sunComposeBuild) {
		case SunXK_FA_Grave:
		    sunComposeBuild = ksym;
		    ksym = XK_grave;
		    break;
		case SunXK_FA_Circum:
		    sunComposeBuild = ksym;
		    ksym = XK_asciicircum;
		    break;
		case SunXK_FA_Tilde:
		    sunComposeBuild = ksym;
		    ksym = XK_asciitilde;
		    break;
		case SunXK_FA_Acute:
		    sunComposeBuild = ksym;
		    ksym = XK_apostrophe;
		    break;
		case SunXK_FA_Diaeresis:
		    sunComposeBuild = ksym;
		    ksym = XK_quotedbl;
		    break;
		case SunXK_FA_Cedilla:
		    sunComposeBuild = ksym;
		    ksym = XK_comma;
		    break;
		default:
		    break;
	    }
	    sunComposeLast = key;
	    for(k = 0; sunComposeMap[k].sym1 != NoSymbol; k++)
		if ((sunComposeMap[k].sym1 == sunComposeBuild && 
		     sunComposeMap[k].sym2 == ksym) ||
		    (sunComposeMap[k].sym2 == sunComposeBuild &&
		     sunComposeMap[k].sym1 == ksym)) {
		    key = sunComposeMap[k].key;
		    sunComposeResult = key;
		    break;
		}
	    sunKbdModLight (pKeyboard, 0, LED_COMPOSE);
	    sunComposeBuild = NoSymbol;
	    if (sunComposeMap[k].sym1 == NoSymbol)
		return;
	}
    }
    if (sunDoCompose && fe->value == VKEY_DOWN && 
	(ksym == SunXK_FA_Grave || ksym == SunXK_FA_Circum || 
	 ksym == SunXK_FA_Tilde || ksym == SunXK_FA_Acute || 
	 ksym == SunXK_FA_Diaeresis || ksym == SunXK_FA_Cedilla)) {
	sunKbdModLight(pKeyboard, 1, LED_COMPOSE);
	sunComposeBuild = ksym;
	sunComposeLast = key;
	return;
    }
    if (sunDoCompose && fe->value == VKEY_UP && key == sunComposeLast) {
	sunComposeLast = 0;
	if ( 0 == (key = sunComposeResult))
	    return;
	sunComposeResult = 0;
    }
    key += sysKbPriv.offset;
    keyModifiers = ((DeviceIntPtr)pKeyboard)->key->modifierMap[key];
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (key == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }

    if (keyModifiers != 0) {
	if (fe->value == VKEY_DOWN)
	    sunModifiers |= keyModifiers;
	else
	    sunModifiers &= ~keyModifiers;
    }

    xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);
    xE.u.u.type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = key;

    if (keyModifiers & LockMask) {
	if (xE.u.u.type == KeyRelease)
	    return; /* this assumes autorepeat is not desired */
	if (pPriv->ctrl->leds & LED_CAPS_LOCK)
	    xE.u.u.type = KeyRelease;
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_CAPS_LOCK);
    }
    if (ksym == XK_Num_Lock) {
	if (xE.u.u.type == KeyRelease)
	    return; /* this assumes autorepeat is not desired */
	if (pPriv->ctrl->leds & LED_NUM_LOCK)
	    xE.u.u.type = KeyRelease;
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_NUM_LOCK);
    }

    if (ksym == XK_Scroll_Lock) {
	if (xE.u.u.type == KeyRelease)
	    return; /* this assumes autorepeat is not desired */
	if (pPriv->ctrl->leds & LED_SCROLL_LOCK)
	    xE.u.u.type = KeyRelease;
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_SCROLL_LOCK);
    }

    if (ksym == SunXK_Compose) {
	if (sunDoCompose) {
	    if (xE.u.u.type == KeyRelease)
		return; /* this assumes autorepeat is not desired */
	    if (pPriv->ctrl->leds & LED_COMPOSE)
		xE.u.u.type == KeyRelease;
	}
	sunKbdModLight (pKeyboard, xE.u.u.type == KeyPress, LED_COMPOSE);
	if (sunDoCompose)
	    return; /* this assumes that we never want to see this key */
    }

    if ((xE.u.u.type == KeyPress) && (keyModifiers == 0)) {
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
    kbdSleep();
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
    return (TRUE);
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

