/* $XConsortium: xf86Priv.h,v 1.1 94/10/05 13:34:15 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Priv.h,v 3.1 1994/07/24 11:49:20 dawes Exp $ */
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

#ifndef _XF86PRIV_H
#define _XF86PRIV_H

#include "Xproto.h"
#include "xf86_OSlib.h"

typedef struct {

  /* keyboard part */
  DevicePtr     pKeyboard;
  int           (* kbdProc)();        /* procedure for initializing */
  void          (* kbdEvents)();      /* proc for processing events */
#ifndef MINIX
  int           consoleFd;
#else
  int		kbdFd;
#endif /* MINIX */
#if defined(MACH386) || defined(__OSF__)
  int           kbdFd;
#endif /* MACH386 || __OSF__ */
  int           vtno;
  int           kbdType;              /* AT84 / AT101 */
  int           kbdRate;
  int           kbdDelay;
  int           bell_pitch;
  int           bell_duration;
  Bool          autoRepeat;
  unsigned long leds;
  unsigned long xleds;
  char          *vtinit;
  int           *specialKeyMap;
  int           scanPrefix;           /* scancode-state */
  Bool          capsLock;
  Bool          numLock;
  Bool          scrollLock;
  Bool          modeSwitchLock;
  Bool          serverNumLock;
  Bool          composeLock;
  Bool          vtSysreq;

  /* pointer part */
  DevicePtr     pPointer;
  int           (* mseProc)();        /* procedure for initializing */
  void          (* mseEvents)();      /* proc for processing events */
  int           mseFd;
  char          *mseDevice;
  int           mseType;
  int           baudRate;
  int           oldBaudRate;
  int           sampleRate;
  int           lastButtons;
  int           threshold, num, den;  /* acceleration */
  int           emulateState;         /* automata state for 2 button mode */
  Bool          emulate3Buttons;
  Bool          chordMiddle;
  int           mouseFlags;        /* Flags to Clear after opening mouse dev */

  /* xque part */
  int           xqueFd;
  int           xqueSema;

  /* event handler part */
  int           lastEventTime;
  Bool          vtRequestsPending;
  Bool          inputPending;
  Bool          dontZap;
  Bool          notrapSignals;           /* don't exit cleanly - die at fault */
  Bool          caughtSignal;

  /* graphics part */
  Bool          sharedMonitor;
  ScreenPtr     currentScreen;
#if defined(__BSD__)
  int           screenFd;	/* fd for memory mapped access to vga card */
#endif
#ifdef __BSD__
  int		consType;	/* Which console driver? */
#endif
#if defined(AMOEBA)
  void		*screenPtr;
#endif

} xf86InfoRec, *xf86InfoPtr;

extern xf86InfoRec xf86Info;

/* ISC's cc can't handle ~ of UL constants, so explicitly type cast them. */
#define XLED1   ((unsigned long) 0x00000001)
#define XLED2   ((unsigned long) 0x00000002)
#define XLED3   ((unsigned long) 0x00000004)
#define XCAPS   ((unsigned long) 0x20000000)
#define XNUM    ((unsigned long) 0x40000000)
#define XSCR    ((unsigned long) 0x80000000)
#define XCOMP	((unsigned long) 0x00008000)

/* 386BSD console driver types (consType) */
#ifdef __BSD__
#define PCCONS		   0
#define CODRV011	   1
#define CODRV01X	   2
#define SYSCONS		   8
#endif

/* Values of xf86Info.mouseFlags */
#define MF_CLEAR_DTR       1
#define MF_CLEAR_RTS       2

extern int xf86ScreenIndex;

#define XF86SCRNINFO(p) ((ScrnInfoPtr)((p)->devPrivates[xf86ScreenIndex].ptr))

extern int xf86MaxScreens;
extern ScrnInfoPtr xf86Screens[];
extern int xf86ScreenNames[];

extern char xf86ConfigFile[];
extern Bool xf86Verbose;
extern unsigned short xf86MouseCflags[];
extern Bool xf86SupportedMouseTypes[];
extern int xf86NumMouseTypes;
extern int xf86bpp;
extern xrgb xf86weight;

#endif /* _XF86PRIV_H */


