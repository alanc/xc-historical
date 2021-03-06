/* $XConsortium: xf86Events.c,v 1.13 95/04/05 19:58:45 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Events.c,v 3.8 1995/01/11 03:50:36 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"
#include "atKeynames.h"

#include "Xpoll.h"
#include "osdep.h"

#define XE_POINTER  1
#define XE_KEYBOARD 2

#ifdef XTESTEXT1

#define	XTestSERVER_SIDE
#include "xtestext1.h"
extern short xtest_mousex;
extern short xtest_mousey;
extern int   on_steal_input;          
extern Bool  XTestStealKeyData();
extern void  XTestStealMotionData();

#define ENQUEUE(ev, code, direction, dev_type) \
  (ev)->u.u.detail = (code); \
  (ev)->u.u.type   = (direction); \
  if (!on_steal_input ||  \
      XTestStealKeyData((ev)->u.u.detail, (ev)->u.u.type, dev_type, \
			xtest_mousex, xtest_mousey)) \
  mieqEnqueue((ev))

#define MOVEPOINTER(dx, dy, time) \
  if (on_steal_input) \
    XTestStealMotionData(dx, dy, XE_POINTER, xtest_mousex, xtest_mousey); \
  miPointerDeltaCursor (dx, dy, time)

#else /* ! XTESTEXT1 */

#define ENQUEUE(ev, code, direction, dev_type) \
  (ev)->u.u.detail = (code); \
  (ev)->u.u.type   = (direction); \
  mieqEnqueue((ev))

#define MOVEPOINTER(dx, dy, time) \
  miPointerDeltaCursor (dx, dy, time)

#endif

Bool xf86VTSema = TRUE;

/*
 * The first of many hack's to get VT switching to work under
 * Solaris 2.1 for x86. The basic problem is that Solaris is supposed
 * to be SVR4. It is for the most part, except where the video interface
 * is concerned.  These hacks work around those problems.
 * See the comments for Linux, and SCO. 
 *
 * This is a toggleling variable:
 *  FALSE = No VT switching keys have been pressed last time around
 *  TRUE  = Possible VT switch Pending
 * (DWH - 12/2/93)
 *
 * This has been generalised to work with Linux and *BSD+syscons (DHD)
 */

#ifdef USE_VT_SYSREQ
static Bool VTSysreqToggle = FALSE;
#endif /* !USE_VT_SYSREQ */
static Bool VTSwitchEnabled = TRUE;   /* Allows run-time disabling for *BSD */

extern fd_set EnabledDevices;

#if defined(CODRV_SUPPORT)
extern unsigned char xf86CodrvMap[];
#endif

static void xf86VTSwitch();

/*
 * Lets create a simple finite-state machine:
 *
 *   state[?][0]: action1
 *   state[?][1]: action2
 *   state[?][2]: next state
 *
 *   action > 0: ButtonPress
 *   action = 0: nothing
 *   action < 0: ButtonRelease
 *
 * Why this stuff ??? Normally you cannot press both mousebuttons together, so
 * the mouse reports both pressed at the same time ...
 */

static char stateTab[48][3] = {

/* nothing pressed */
  {  0,  0,  0 },	
  {  0,  0,  8 },	/* 1 right -> delayed right */
  {  0,  0,  0 },       /* 2 nothing */
  {  0,  0,  8 },	/* 3 right -> delayed right */
  {  0,  0, 16 },	/* 4 left -> delayed left */
  {  2,  0, 24 },       /* 5 left & right (middle press) -> middle pressed */
  {  0,  0, 16 },	/* 6 left -> delayed left */
  {  2,  0, 24 },       /* 7 left & right (middle press) -> middle pressed */

/* delayed right */
  {  1, -1,  0 },	/* 8 nothing (right event) -> init */
  {  1,  0, 32 },       /* 9 right (right press) -> right pressed */
  {  1, -1,  0 },	/* 10 nothing (right event) -> init */
  {  1,  0, 32 },       /* 11 right (right press) -> right pressed */
  {  1, -1, 16 },       /* 12 left (right event) -> delayed left */
  {  2,  0, 24 },       /* 13 left & right (middle press) -> middle pressed */
  {  1, -1, 16 },       /* 14 left (right event) -> delayed left */
  {  2,  0, 24 },       /* 15 left & right (middle press) -> middle pressed */

/* delayed left */
  {  3, -3,  0 },	/* 16 nothing (left event) -> init */
  {  3, -3,  8 },       /* 17 right (left event) -> delayed right */
  {  3, -3,  0 },	/* 18 nothing (left event) -> init */
  {  3, -3,  8 },       /* 19 right (left event) -> delayed right */
  {  3,  0, 40 },	/* 20 left (left press) -> pressed left */
  {  2,  0, 24 },	/* 21 left & right (middle press) -> pressed middle */
  {  3,  0, 40 },	/* 22 left (left press) -> pressed left */
  {  2,  0, 24 },	/* 23 left & right (middle press) -> pressed middle */

/* pressed middle */
  { -2,  0,  0 },	/* 24 nothing (middle release) -> init */
  { -2,  0,  0 },	/* 25 right (middle release) -> init */
  { -2,  0,  0 },	/* 26 nothing (middle release) -> init */
  { -2,  0,  0 },	/* 27 right (middle release) -> init */
  { -2,  0,  0 },	/* 28 left (middle release) -> init */
  {  0,  0, 24 },	/* 29 left & right -> pressed middle */
  { -2,  0,  0 },	/* 30 left (middle release) -> init */
  {  0,  0, 24 },	/* 31 left & right -> pressed middle */

/* pressed right */
  { -1,  0,  0 },	/* 32 nothing (right release) -> init */
  {  0,  0, 32 },	/* 33 right -> pressed right */
  { -1,  0,  0 },	/* 34 nothing (right release) -> init */
  {  0,  0, 32 },	/* 35 right -> pressed right */
  { -1,  0, 16 },	/* 36 left (right release) -> delayed left */
  { -1,  2, 24 },	/* 37 left & right (r rel, m prs) -> middle pressed */
  { -1,  0, 16 },	/* 38 left (right release) -> delayed left */
  { -1,  2, 24 },	/* 39 left & right (r rel, m prs) -> middle pressed */

/* pressed left */
  { -3,  0,  0 },	/* 40 nothing (left release) -> init */
  { -3,  0,  8 },	/* 41 right (left release) -> delayed right */
  { -3,  0,  0 },	/* 42 nothing (left release) -> init */
  { -3,  0,  8 },	/* 43 right (left release) -> delayed right */
  {  0,  0, 40 },	/* 44 left -> left pressed */
  { -3,  2, 24 },	/* 45 left & right (l rel, mprs) -> middle pressed */
  {  0,  0, 40 },	/* 46 left -> left pressed */
  { -3,  2, 24 },	/* 47 left & right (l rel, mprs) -> middle pressed */
};


/*
 * Table to allow quick reversal of natural button mapping to correct mapping
 */

static char reverseMap[8] = {0, 4, 2, 6, 1, 5, 3, 7};
static char hitachMap[16] = {  0,  2,  1,  3, 
			       8, 10,  9, 11,
			       4,  6,  5,  7,
			      12, 14, 13, 15 };


/*
 * TimeSinceLastInputEvent --
 *      Function used for screensaver purposes by the os module. Retruns the
 *      time in milliseconds since there last was any input.
 */

int
TimeSinceLastInputEvent()
{
  if (xf86Info.lastEventTime == 0) {
    xf86Info.lastEventTime = GetTimeInMillis();
  }
  return GetTimeInMillis() - xf86Info.lastEventTime;
}



/*
 * SetTimeSinceLastInputEvent --
 *      Set the lastEventTime to now.
 */

void
SetTimeSinceLastInputEvent()
{
  xf86Info.lastEventTime = GetTimeInMillis();
}



/*
 * ProcessInputEvents --
 *      Retrieve all waiting input events and pass them to DIX in their
 *      correct chronological order. Only reads from the system pointer
 *      and keyboard.
 */

void
ProcessInputEvents ()
{
  int x, y;

#ifdef AMOEBA
#define MAXEVENTS	    32
#define BUTTON_PRESS	    0x1000
#define MAP_BUTTON(ev,but)  (((ev) == EV_ButtonPress) ? \
			     ((but) | BUTTON_PRESS) : ((but) & ~BUTTON_PRESS))
#define KEY_RELEASE	    0x80
#define MAP_KEY(ev, key)    (((ev) == EV_KeyReleaseEvent) ? \
			     ((key) | KEY_RELEASE) : ((key) & ~KEY_RELEASE))

    register IOPEvent  *e, *elast;
    IOPEvent		events[MAXEVENTS];
    int			dx, dy, nevents;
  
    /*
     * Get all events from the IOP server
     */
    while ((nevents = AmoebaGetEvents(events, MAXEVENTS)) > 0) {
      for (e = &events[0], elast = &events[nevents]; e < elast; e++) {
          xf86Info.lastEventTime = e->time;
          switch (e->type) {
          case EV_PointerDelta:
	      if (e->x != 0 || e->y != 0) {
                  xf86PostMseEvent(0, e->x, e->y);
	      }
              break;
          case EV_ButtonPress:
          case EV_ButtonRelease:
              xf86PostMseEvent(MAP_BUTTON(e->type, e->keyorbut), 0, 0);
              break;
          case EV_KeyPressEvent:
          case EV_KeyReleaseEvent:
              xf86PostKbdEvent(MAP_KEY(e->type, e->keyorbut));
              break;
          default:
              /* this shouldn't happen */
              ErrorF("stray event %d (%d,%d) %x\n",
                      e->type, e->x, e->y, e->keyorbut);
              break;
          }
      }
    }
#endif

  xf86Info.inputPending = FALSE;

  mieqProcessInputEvents();
  miPointerUpdate();

  miPointerPosition(&x, &y);
  xf86SetViewport(xf86Info.currentScreen, x, y);
}



/*
 * xf86PostKbdEvent --
 *	Translate the raw hardware KbdEvent into an XEvent, and tell DIX
 *	about it. Scancode preprocessing and so on is done ...
 */

void
xf86PostKbdEvent(key)
     unsigned key;
{
  int         scanCode = (key & 0x7f);
  Bool        down = (key & 0x80 ? FALSE : TRUE);
  KeyClassRec *keyc = ((DeviceIntPtr)xf86Info.pKeyboard)->key;
  Bool        updateLeds = FALSE;
  Bool        UsePrefix = FALSE;
  Bool        Direction = FALSE;
  xEvent      kevent;
  KeySym      *keysym;
  int         keycode;
  static int  lockkeys = 0;
#if defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
  static Bool first_time = TRUE;
#endif

#if defined(CODRV_SUPPORT)
  if (xf86Info.consType == CODRV011 || xf86Info.consType == CODRV01X)
    scanCode = xf86CodrvMap[scanCode];
#endif

#if defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
  if (first_time)
  {
    first_time = FALSE;
    VTSwitchEnabled = (xf86Info.consType == SYSCONS)
	    || (xf86Info.consType == PCVT);
  }
#endif
  /*
   * First do some special scancode remapping ...
   */
  if (xf86Info.scanPrefix == 0) {

    switch (scanCode) {
      
    case KEY_Prefix0:
    case KEY_Prefix1:
#if defined(PCCONS_SUPPORT) || defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
      if (xf86Info.consType == PCCONS || xf86Info.consType == SYSCONS
	  || xf86Info.consType == PCVT) {
#endif
        xf86Info.scanPrefix = scanCode;  /* special prefixes */
        return;
#if defined(PCCONS_SUPPORT) || defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)
      }
      break;
#endif
    case KEY_CapsLock:
    case KEY_NumLock:
    case KEY_ScrollLock:
      updateLeds = TRUE;              /* led changes by firmware */
      break;
    }
    if (xf86Info.serverNumLock) {
     if ((!xf86Info.numLock && ModifierDown(ShiftMask)) ||
         (xf86Info.numLock && !ModifierDown(ShiftMask))) {
      /*
       * Hardwired numlock handling ... (Some applications break if they have
       * these keys double defined, like twm)
       */
      switch (scanCode) {
      case KEY_KP_7:        scanCode = KEY_SN_KP_7;   break;  /* curs 7 */
      case KEY_KP_8:        scanCode = KEY_SN_KP_8;   break;  /* curs 8 */
      case KEY_KP_9:        scanCode = KEY_SN_KP_9;   break;  /* curs 9 */
      case KEY_KP_4:        scanCode = KEY_SN_KP_4;   break;  /* curs 4 */
      case KEY_KP_5:        scanCode = KEY_SN_KP_5;   break;  /* curs 5 */
      case KEY_KP_6:        scanCode = KEY_SN_KP_6;   break;  /* curs 6 */
      case KEY_KP_1:        scanCode = KEY_SN_KP_1;   break;  /* curs 1 */
      case KEY_KP_2:        scanCode = KEY_SN_KP_2;   break;  /* curs 2 */
      case KEY_KP_3:        scanCode = KEY_SN_KP_3;   break;  /* curs 3 */
      case KEY_KP_0:        scanCode = KEY_SN_KP_0;   break;  /* curs 0 */
      case KEY_KP_Decimal:  scanCode = KEY_SN_KP_Dec; break;  /* curs decimal */
      }
     } else {
      switch (scanCode) {
      case KEY_KP_7:        scanCode = KEY_SN_KP_Home;  break;  /* curs home */
      case KEY_KP_8:        scanCode = KEY_SN_KP_Up  ;  break;  /* curs up */
      case KEY_KP_9:        scanCode = KEY_SN_KP_Prior; break;  /* curs pgup */
      case KEY_KP_4:        scanCode = KEY_SN_KP_Left;  break;  /* curs left */
      case KEY_KP_5:        scanCode = KEY_SN_KP_Begin; break;  /* curs begin */
      case KEY_KP_6:        scanCode = KEY_SN_KP_Right; break;  /* curs right */
      case KEY_KP_1:        scanCode = KEY_SN_KP_End;   break;  /* curs end */
      case KEY_KP_2:        scanCode = KEY_SN_KP_Down;  break;  /* curs down */
      case KEY_KP_3:        scanCode = KEY_SN_KP_Next;  break;  /* curs pgdn */
      case KEY_KP_0:        scanCode = KEY_SN_KP_Ins;   break;  /* curs ins */
      case KEY_KP_Decimal:  scanCode = KEY_SN_KP_Del;   break;  /* curs del */
      }
     }
    }
  }

  else if (
#ifdef CSRG_BASED
           (xf86Info.consType == PCCONS || xf86Info.consType == SYSCONS
	    || xf86Info.consType == PCVT) &&
#endif
           (xf86Info.scanPrefix == KEY_Prefix0)) {
    xf86Info.scanPrefix = 0;
	  
    switch (scanCode) {
    case KEY_KP_7:        scanCode = KEY_Home;      break;  /* curs home */
    case KEY_KP_8:        scanCode = KEY_Up;        break;  /* curs up */
    case KEY_KP_9:        scanCode = KEY_PgUp;      break;  /* curs pgup */
    case KEY_KP_4:        scanCode = KEY_Left;      break;  /* curs left */
    case KEY_KP_5:        scanCode = KEY_Begin;     break;  /* curs begin */
    case KEY_KP_6:        scanCode = KEY_Right;     break;  /* curs right */
    case KEY_KP_1:        scanCode = KEY_End;       break;  /* curs end */
    case KEY_KP_2:        scanCode = KEY_Down;      break;  /* curs down */
    case KEY_KP_3:        scanCode = KEY_PgDown;    break;  /* curs pgdown */
    case KEY_KP_0:        scanCode = KEY_Insert;    break;  /* curs insert */
    case KEY_KP_Decimal:  scanCode = KEY_Delete;    break;  /* curs delete */
    case KEY_Enter:       scanCode = KEY_KP_Enter;  break;  /* keypad enter */
    case KEY_LCtrl:       scanCode = KEY_RCtrl;     break;  /* right ctrl */
    case KEY_KP_Multiply: scanCode = KEY_Print;     break;  /* print */
    case KEY_Slash:       scanCode = KEY_KP_Divide; break;  /* keyp divide */
    case KEY_Alt:         scanCode = KEY_AltLang;   break;  /* right alt */
    case KEY_ScrollLock:  scanCode = KEY_Break;     break;  /* curs break */
    case 0x5b:            scanCode = KEY_LMeta;     break;
    case 0x5c:            scanCode = KEY_RMeta;     break;
    case 0x5d:            scanCode = KEY_Menu;      break;
      /*
       * Ignore virtual shifts (E0 2A, E0 AA, E0 36, E0 B6)
       */
    default:
      return;                                  /* skip illegal */
    }
  }
  
  else if (xf86Info.scanPrefix == KEY_Prefix1)
    {
      xf86Info.scanPrefix = (scanCode == KEY_LCtrl) ? KEY_LCtrl : 0;
      return;
    }
  
  else if (xf86Info.scanPrefix == KEY_LCtrl)
    {
      xf86Info.scanPrefix = 0;
      if (scanCode != KEY_NumLock) return;
      scanCode = KEY_Pause;       /* pause */
    }
  /*
   * and now get some special keysequences
   */
  if ((ModifierDown(ControlMask | AltMask)) ||
      (ModifierDown(ControlMask | AltLangMask)))
    {
      
      switch (scanCode) {
	
      case KEY_BackSpace:
	if (!xf86Info.dontZap) GiveUp(0);
	break;
	
	/*
	 * The idea here is to pass the scancode down to a list of
	 * registered routines. There should be some standart conventions
	 * for processing certain keys.
	 */
      case KEY_KP_Minus:   /* Keypad - */
	if (!xf86Info.dontZoom) {
	  if (down) xf86ZoomViewport(xf86Info.currentScreen, -1);
	  return;
	}
	break;
	
      case KEY_KP_Plus:   /* Keypad + */
	if (!xf86Info.dontZoom) {
	  if (down) xf86ZoomViewport(xf86Info.currentScreen,  1);
	  return;
	}
	break;

#if defined(linux) || (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)))
	/*
	 * Under Linux, the raw keycodes are consumed before the kernel
	 * does any processing on them, so we must emulate the vt switching
	 * we want ourselves.
	 */
      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
      case KEY_F8:
      case KEY_F9:
      case KEY_F10:
        if (VTSwitchEnabled && !xf86Info.vtSysreq
#if (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)))
	    && (xf86Info.consType == SYSCONS || xf86Info.consType == PCVT)
#endif
	    )
        {
	  if (down)
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, scanCode - KEY_F1 + 1);
          return;
        }
	break;
      case KEY_F11:
      case KEY_F12:
        if (VTSwitchEnabled && !xf86Info.vtSysreq
#if (defined(CSRG_BASED) && (defined(SYSCONS_SUPPORT) || defined(PCVT_SUPPORT)))
	    && (xf86Info.consType == SYSCONS || xf86Info.consType == PCVT)
#endif
	    )
        {
	  if (down)
            ioctl(xf86Info.consoleFd, VT_ACTIVATE, scanCode - KEY_F11 + 11);
          return;
        }
	break;
#endif /* linux || BSD with VTs */

      /* just worth mentioning here: any 386bsd keyboard driver
       * (pccons.c or co_kbd.c) catches CTRL-ALT-DEL and CTRL-ALT-ESC
       * before any application (e.g. XF86) will see it
       * OBS: syscons does not, nor does pcvt !
       */
      } 
    }

    /*
     * Start of actual Solaris VT switching code.  
     * This should pretty much emulate standard SVR4 switching keys.
     * 
     * DWH 12/2/93
     */

#ifdef USE_VT_SYSREQ
    if (VTSwitchEnabled && xf86Info.vtSysreq)
    {
      switch (scanCode)
      {
      /*
       * syscons on *BSD doesn't have a VT #0  -- don't think Linux does
       * either
       */
#if defined (sun) && defined (i386) && defined (SVR4)
      case KEY_H: 
	if (VTSysreqToggle && down)
        {
          ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0);
          VTSysreqToggle = 0;
          return; 
        }
	break;

      /*
       * Yah, I know the N, and P keys seem backwards, however that's
       * how they work under Solaris
       * XXXX N means go to next active VT not necessarily vtno+1 (or vtno-1)
       */

      case KEY_N:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno - 1 ) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      case KEY_P:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno + 1 ) < 0)
            if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0) < 0)
              ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;
#endif

      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
      case KEY_F8:
      case KEY_F9:
      case KEY_F10:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, scanCode-KEY_F1 + 1) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      case KEY_F11:
      case KEY_F12:
	if (VTSysreqToggle && down)
	{
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, scanCode-KEY_F11 + 11) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
          VTSysreqToggle = FALSE;
          return;
        }
	break;

      /* Ignore these keys -- ie don't let them cancel an alt-sysreq */
      case KEY_Alt:
      case KEY_AltLang:
	break;

      case KEY_SysReqest:
        if (down && (ModifierDown(AltMask) || ModifierDown(AltLangMask)))
          VTSysreqToggle = TRUE;
	break;

      default:
        if (VTSysreqToggle)
	{
	  /*
	   * We only land here when Alt-SysReq is followed by a
	   * non-switching key.
	   */
          VTSysreqToggle = FALSE;

        }
      }
    }

#endif /* USE_VT_SYSREQ */

#ifdef SCO
    /*
     *	With the console in raw mode, SCO will not switch consoles,
     *	you get around this by activating the next console along, if
     *	this fails then go back to console 0, if there is only one
     *	then it doesn't matter, switching to yourself is a nop as far
     *	as the console driver is concerned.
     *	We could do something similar to linux here but SCO ODT uses
     *	Ctrl-PrintScrn, so why change?
     */
    if (scanCode == KEY_Print && ModifierDown(ControlMask)) {
      if (down)
        if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno + 1) < 0)
          if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0) < 0)
            ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
      return;
    }
#endif /* SCO */

  /*
   * Now map the scancodes to real X-keycodes ...
   */
  keycode = scanCode + MIN_KEYCODE;
  keysym = (keyc->curKeySyms.map +
	    keyc->curKeySyms.mapWidth * 
	    (keycode - keyc->curKeySyms.minKeyCode));
  /*
   * Filter autorepeated caps/num/scroll lock keycodes.
   */
#define CAPSFLAG 0x01
#define NUMFLAG 0x02
#define SCROLLFLAG 0x04
#define MODEFLAG 0x08
  if( down ) {
    switch( keysym[0] ) {
        case XK_Caps_Lock :
          if (lockkeys & CAPSFLAG)
              return;
	  else
	      lockkeys |= CAPSFLAG;
          break;

        case XK_Num_Lock :
          if (lockkeys & NUMFLAG)
              return;
	  else
	      lockkeys |= NUMFLAG;
          break;

        case XK_Scroll_Lock :
          if (lockkeys & SCROLLFLAG)
              return;
	  else
	      lockkeys |= SCROLLFLAG;
          break;
    }
    if (keysym[1] == XK_Mode_Lock)
    {
      if (lockkeys & MODEFLAG)
          return;
      else
          lockkeys |= MODEFLAG;
    }
      
  }
  else {
    switch( keysym[0] ) {
        case XK_Caps_Lock :
            lockkeys &= ~CAPSFLAG;
            break;

        case XK_Num_Lock :
            lockkeys &= ~NUMFLAG;
            break;

        case XK_Scroll_Lock :
            lockkeys &= ~SCROLLFLAG;
            break;
    }
    if (keysym[1] == XK_Mode_Lock)
      lockkeys &= ~MODEFLAG;
  }

  /*
   * LockKey special handling:
   * ignore releases, toggle on & off on presses.
   * Don't deal with the Caps_Lock keysym directly, but check the lock modifier
   */
  if (keyc->modifierMap[keycode] & LockMask ||
      keysym[0] == XK_Scroll_Lock ||
      keysym[1] == XK_Mode_Lock ||
      keysym[0] == XK_Num_Lock)
    {
      Bool flag;

      if (!down) return;
      if (KeyPressed(keycode)) {
	down = !down;
	flag = FALSE;
      }
      else
	flag = TRUE;

      if (keyc->modifierMap[keycode] & LockMask)   xf86Info.capsLock   = flag;
      if (keysym[0] == XK_Num_Lock)    xf86Info.numLock    = flag;
      if (keysym[0] == XK_Scroll_Lock) xf86Info.scrollLock = flag;
      if (keysym[1] == XK_Mode_Lock)   xf86Info.modeSwitchLock = flag;
      updateLeds = TRUE;
    }
	
  /*
   * check for an autorepeat-event
   */
  if ((down && KeyPressed(keycode)) &&
      (xf86Info.autoRepeat != AutoRepeatModeOn || keyc->modifierMap[keycode]))
    return;

  xf86Info.lastEventTime = kevent.u.keyButtonPointer.time = GetTimeInMillis();

  /*
   * normal, non-keypad keys
   */
  if (scanCode < KEY_KP_7 || scanCode > KEY_KP_Decimal) {
#if !defined(CSRG_BASED) && !defined(MACH386) && !defined(MINIX) && !defined(__OSF__)
    /*
     * magic ALT_L key on AT84 keyboards for multilingual support
     */
    if (xf86Info.kbdType == KB_84 &&
	ModifierDown(AltMask) &&
	keysym[2] != NoSymbol)
      {
	UsePrefix = TRUE;
	Direction = TRUE;
      }
#endif /* !CSRG_BASED && !MACH386 && !MINIX && !__OSF__ */
  }


  /*
   * And now send these prefixes ...
   * NOTE: There cannot be multiple Mode_Switch keys !!!!
   */
  if (UsePrefix)
    {
      ENQUEUE(&kevent,
	      keyc->modifierKeyMap[keyc->maxKeysPerModifier*7],
	      (Direction ? KeyPress : KeyRelease),
	      XE_KEYBOARD);
      ENQUEUE(&kevent, keycode, (down ? KeyPress : KeyRelease), XE_KEYBOARD);
      ENQUEUE(&kevent,
	      keyc->modifierKeyMap[keyc->maxKeysPerModifier*7],
	      (Direction ? KeyRelease : KeyPress),
	      XE_KEYBOARD);
    }
  else 
    {
      ENQUEUE(&kevent, keycode, (down ? KeyPress : KeyRelease), XE_KEYBOARD);
    }

  if (updateLeds) xf86KbdLeds();
}




/*      
 * xf86PostMseEvent --
 *	Translate the raw hardware MseEvent into an XEvent(s), and tell DIX
 *	about it. Perform a 3Button emulation if required.
 */

void
xf86PostMseEvent(buttons, dx, dy)
     int buttons, dx, dy;
{
  int         id, change;
  int         truebuttons;
  xEvent      mevent;
#ifdef AMOEBA
  int	      pressed;

  pressed = ((buttons & BUTTON_PRESS) != 0);
  buttons &= ~BUTTON_PRESS;
#endif

  xf86Info.lastEventTime = mevent.u.keyButtonPointer.time = GetTimeInMillis();

  truebuttons = buttons;
  if (xf86Info.mseType == P_MMHIT)
    buttons = hitachMap[buttons];
  else
    buttons = reverseMap[buttons];

  if (dx || dy) {
    
    /*
     * accelerate the baby now if sqrt(dx*dx + dy*dy) > threshold !
     * but do some simpler arithmetic here...
     */
    if ((abs(dx) + abs(dy)) >= xf86Info.threshold) {
      dx = (dx * xf86Info.num) / xf86Info.den;
      dy = (dy * xf86Info.num)/ xf86Info.den;
    }

    MOVEPOINTER(dx, dy, mevent.u.keyButtonPointer.time);
  }

  if (xf86Info.emulate3Buttons)
    {
      
      /*
       * emulate the third button by the other two
       */
      if (id = stateTab[buttons + xf86Info.emulateState][0])
	{
	  ENQUEUE(&mevent,
		  abs(id), (id < 0 ? ButtonRelease : ButtonPress), 
		  XE_POINTER);
	}

      if (id = stateTab[buttons + xf86Info.emulateState][1])
	{
	  ENQUEUE(&mevent,
		  abs(id), (id < 0 ? ButtonRelease : ButtonPress), 
		  XE_POINTER);
	}

      xf86Info.emulateState = stateTab[buttons + xf86Info.emulateState][2];
    }
  else
    {
#ifdef AMOEBA
      if (truebuttons != 0) {
	  ENQUEUE(&mevent,
		  truebuttons, (pressed ? ButtonPress : ButtonRelease),
                  XE_POINTER);
      }
#else
      /*
       * real three button event
       * Note that xf86Info.lastButtons has the hardware button mapping which
       * is the reverse of the button mapping reported to the server.
       */
      if (xf86Info.mseType == P_MMHIT)
        change = buttons ^ hitachMap[xf86Info.lastButtons];
      else
        change = buttons ^ reverseMap[xf86Info.lastButtons];

      while (change)
	{
	  id = ffs(change);
	  change &= ~(1 << (id-1));
	  ENQUEUE(&mevent,
		  id, (buttons&(1<<(id-1)))? ButtonPress : ButtonRelease,
		  XE_POINTER);
	}
#endif
    }
    xf86Info.lastButtons = truebuttons;
}



/*
 * xf86Block --
 *      Os block handler.
 */

/* ARGSUSED */
void
xf86Block(blockData, pTimeout, pReadmask)
     pointer blockData;
     OSTimePtr pTimeout;
     pointer  pReadmask;
{
}


#ifndef AMOEBA

/*
 * xf86Wakeup --
 *      Os wakeup handler.
 */

/* ARGSUSED */
void
xf86Wakeup(blockData, err, pReadmask)
     pointer blockData;
     int err;
     pointer pReadmask;
{
#ifdef	__OSF__
  fd_set kbdDevices;
  fd_set mseDevices;
#endif	/* __OSF__ */
  fd_set devicesWithInput;

  fd_set* fdp = (fd_set*) pReadmask;

  if ((int)err >= 0) {
#ifdef	__OSF__
   /*
     * Until the two devices are made nonblock on read, we have to do this.
     */

    XFD_ANDSET(&devicesWithInput, fdp, &EnabledDevices);

    FD_ZERO(&kbdDevices);
    FD_SET(xf86Info.consoleFd, &kbdDevices);
    XFD_ANDSET(&kbdDevices, &kbdDevices, &devicesWithInput);

    FD_ZERO(&mseDevices);
    FD_SET(xf86Info.mseFd, &mseDevices);
    XFD_ANDSET(&mseDevices, &mseDevices, &devicesWithInput);

    if (XFD_ANYSET(&kbdDevices) || xf86Info.kbdRate)
        (xf86Info.kbdEvents)(XFD_ANYSET(&kbdDevices));
    if (XFD_ANYSET(&mseDevices))
        (xf86Info.mseEvents)(1);

#else
    XFD_ANDSET(&devicesWithInput, fdp, &EnabledDevices);
    if (XFD_ANYSET(&devicesWithInput))
      {
	(xf86Info.kbdEvents)();
	(xf86Info.mseEvents)();
      }
#endif	/* __OSF__ */
  }

  if (xf86VTSwitchPending()) xf86VTSwitch();

  if (xf86Info.inputPending) ProcessInputEvents();
}

#endif /* AMOEBA */


/*
 * xf86SigHandler --
 *    Catch unexpected signals and exit cleanly.
 */
void
xf86SigHandler(signo)
     int signo;
{
  signal(signo,SIG_IGN);
  xf86Info.caughtSignal = TRUE;
  FatalError("Caught signal %d.  Server aborting\n", signo);
}

/*
 * xf86VTSwitch --
 *      Handle requests for switching the vt.
 */
static void
xf86VTSwitch()
{
  int j;

  if (xf86VTSema) {
    for (j = 0; j < screenInfo.numScreens; j++)
      (XF86SCRNINFO(screenInfo.screens[j])->EnterLeaveVT)(LEAVE, j);
      
    DisableDevice((DeviceIntPtr)xf86Info.pKeyboard);
    DisableDevice((DeviceIntPtr)xf86Info.pPointer);
      
    if (!xf86VTSwitchAway()) {
      /*
       * switch failed 
       */

      for (j = 0; j < screenInfo.numScreens; j++)
        (XF86SCRNINFO(screenInfo.screens[j])->EnterLeaveVT)(ENTER, j);
      SaveScreens(SCREEN_SAVER_FORCER,ScreenSaverReset);
                       
      EnableDevice((DeviceIntPtr)xf86Info.pKeyboard);
      EnableDevice((DeviceIntPtr)xf86Info.pPointer);

    } else {
      xf86VTSema = FALSE;
    }
  } else {
    if (!xf86VTSwitchTo()) return;
      
    xf86VTSema = TRUE;
    for (j = 0; j < screenInfo.numScreens; j++)
      (XF86SCRNINFO(screenInfo.screens[j])->EnterLeaveVT)(ENTER, j);
      
    /* Turn screen saver off when switching back */
    SaveScreens(SCREEN_SAVER_FORCER,ScreenSaverReset);

    EnableDevice((DeviceIntPtr)xf86Info.pKeyboard);
    EnableDevice((DeviceIntPtr)xf86Info.pPointer);
      
  }
}

#ifdef XTESTEXT1

void
XTestGetPointerPos(fmousex, fmousey)
     short *fmousex;
     short *fmousey;
{
  int x,y;

  miPointerPosition(&x, &y);
  *fmousex = x;
  *fmousey = y;
}



void
XTestJumpPointer(jx, jy, dev_type)
     int jx;
     int jy;
     int dev_type;
{
  miPointerAbsoluteCursor(jx, jy, GetTimeInMillis() );
}



void
XTestGenerateEvent(dev_type, keycode, keystate, mousex, mousey)
     int dev_type;
     int keycode;
     int keystate;
     int mousex;
     int mousey;
{
  xEvent tevent;
  
  tevent.u.u.type = (dev_type == XE_POINTER) ?
    (keystate == XTestKEY_UP) ? ButtonRelease : ButtonPress :
      (keystate == XTestKEY_UP) ? KeyRelease : KeyPress;
  tevent.u.u.detail = keycode;
  tevent.u.keyButtonPointer.rootX = mousex;
  tevent.u.keyButtonPointer.rootY = mousey;
  tevent.u.keyButtonPointer.time = xf86Info.lastEventTime = GetTimeInMillis();
  mieqEnqueue(&tevent);
  xf86Info.inputPending = TRUE;               /* virtual event */
}

#endif /* XTESTEXT1 */
