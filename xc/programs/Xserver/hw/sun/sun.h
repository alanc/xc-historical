
/* $XConsortium: sun.h,v 5.25 93/10/11 11:51:05 rws Exp $ */

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

#ifndef _SUN_H_ 
#define _SUN_H_

/* X headers */
#include "Xos.h"
#include "X.h"
#include "Xproto.h"

/* general system headers */
#ifndef NOSTDHDRS
# ifndef PSZ
# include <stdlib.h>
# endif
#else
# include <malloc.h>
extern char *getenv();
#endif

/* system headers common to both SunOS and Solaris */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#if !defined(SVR4) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
# include <signal.h>
#undef _POSIX_SOURCE
#endif
#include <fcntl.h>
#ifndef i386
# include <poll.h>
#else
# include <sys/poll.h>
#endif
#include <errno.h>
#include <memory.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

extern int gettimeofday();

/* 
 * Sun specific headers Sun moved in Solaris.
 *
 * Even if only needed by one source file, I have put them here 
 * to simplify finding them...
 */
#ifdef SVR4
# include <sys/fbio.h>
# include <sys/kbd.h>
# include <sys/kbio.h>
# include <sys/msio.h>
# include <sys/vuid_event.h>
# include <sys/memreg.h>
# include <stropts.h>
# define usleep(usec) poll((struct pollfd *) 0, (size_t) 0, usec / 1000)
#else
# include <sun/fbio.h>
# include <sundev/kbd.h>
# include <sundev/kbio.h>
# include <sundev/msio.h>
# include <sundev/vuid_event.h>
# include <pixrect/pixrect.h>
# include <pixrect/memreg.h>
extern int ioctl();
extern int getrlimit();
extern int setrlimit();
extern int getpagesize();
#endif

/* 
 * Server specific headers
 */
#include "misc.h"
#include "scrnintstr.h"
#ifdef NEED_EVENTS
# include "inputstr.h"
#endif
#include "input.h"
#include "colormapst.h"
#include "colormap.h"
#include "cursorstr.h"
#include "cursor.h"
#include "dixstruct.h"
#include "dix.h"
#include "opaque.h"
#include "resource.h"
#include "servermd.h"
#include "windowstr.h"

/* 
 * ddx specific headers 
 */
#define PSZ 8

#include "mipointer.h"

/* 
 * you would think, for as much as these seem to be used, that they'd
 * be prototyped/declared in a header file somewhere...
 */
extern void mieqEnqueue();
extern Bool miDCInitialize();
extern int cfbExpandDirectColors();
extern int monitorResolution;

/*
 * These are needed for the GX
 */
extern Bool cfbSetupScreen();
extern int cfbFinishScreenInit();


/* Frame buffer devices */
#ifdef SVR4
# define CGTWO0DEV	"/dev/fbs/cgtwo0"
# define CGTWO1DEV	"/dev/fbs/cgtwo1"
# define CGTWO2DEV	"/dev/fbs/cgtwo2"
# define CGTHREE0DEV	"/dev/fbs/cgthree0"
# define CGTHREE1DEV	"/dev/fbs/cgthree1"
# define CGTHREE2DEV	"/dev/fbs/cgthree2"
# define CGFOUR0DEV	"/dev/fbs/cgfour0"
# define CGSIX0DEV	"/dev/fbs/cgsix0"
# define CGSIX1DEV	"/dev/fbs/cgsix1"
# define CGSIX2DEV	"/dev/fbs/cgsix2"
# define BWTWO0DEV	"/dev/fbs/bwtwo0"
# define BWTWO1DEV	"/dev/fbs/bwtwo1"
# define BWTWO2DEV	"/dev/fbs/bwtwo2"
#else
# define CGTWO0DEV	"/dev/cgtwo0"
# define CGTWO1DEV	"/dev/cgtwo1"
# define CGTWO2DEV	"/dev/cgtwo2"
# define CGTHREE0DEV	"/dev/cgthree0"
# define CGTHREE1DEV	"/dev/cgthree1"
# define CGTHREE2DEV	"/dev/cgthree2"
# define CGFOUR0DEV	"/dev/cgfour0"
# define CGSIX0DEV	"/dev/cgsix0"
# define CGSIX1DEV	"/dev/cgsix1"
# define CGSIX2DEV	"/dev/cgsix2"
# define BWTWO0DEV	"/dev/bwtwo0"
# define BWTWO1DEV	"/dev/bwtwo1"
# define BWTWO2DEV	"/dev/bwtwo2"
#endif

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any sun keyboard.
 *	GetEvents reads any events which are available for the keyboard
 *	ProcessEvent processes a single event and gives it to DIX
 *	DoneEvents is called when done handling a string of keyboard
 *	    events or done handling all events.
 *	devPrivate is private to the specific keyboard.
 *	map_q is TRUE if the event queue for the keyboard is memory mapped.
 */
typedef struct kbPrivate {
    int	    	  type;           	/* Type of keyboard */
    int           layout;		/* The layout of the keyboard */
    int	    	  fd;	    	    	/* Descriptor open to device */
    Firm_event	  *(*GetEvents)();  	/* Function to read events */
    void    	  (*EnqueueEvent)();	/* Function to process an event */
    int		  offset;		/* to be added to device keycodes */
    Leds	  leds;			/* led save state */
    int		  click;		/* kbd click save state */
    Bool	  map_q;		/* TRUE if has a mapped event queue */
} KbPrivRec, *KbPrivPtr;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} SunModmapRec;

/*
 * Data private to any sun pointer device.
 *	GetEvents, ProcessEvent and DoneEvents have uses similar to the
 *	    keyboard fields of the same name.
 *	pScreen is the screen the pointer is on (only valid if it is the
 *	    main pointer device).
 *	dx and dy are relative coordinates on that screen (they may be negative)
 */
typedef struct ptrPrivate {
    int	    	  fd;	    	    	/* Descriptor to device */
    Firm_event 	  *(*GetEvents)(); 	/* Function to read events */
    void    	  (*EnqueueEvent)();	/* Function to process an event */
    pointer    	  devPrivate;	    	/* Field private to device */
} PtrPrivRec, *PtrPrivPtr;

typedef struct {
    int		    width, height;
    Bool	    has_cursor;
    CursorPtr	    pCursor;		/* current cursor */
} sunCursorRec, *sunCursorPtr;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    sunCursorRec    hardwareCursor;
    Bool	    hasHardwareCursor;
} sunScreenRec, *sunScreenPtr;

#define GetScreenPrivate(s)   ((sunScreenPtr) ((s)->devPrivates[sunScreenIndex].ptr))
#define SetupScreen(s)	sunScreenPtr	pPrivate = GetScreenPrivate(s)

/*
 * Frame-buffer-private info.
 *	fd  	  	file opened to the frame buffer device.
 *	info	  	description of the frame buffer -- type, height, depth,
 *	    	  	width, etc.
 *	fb  	  	pointer to the mapped image of the frame buffer. Used
 *	    	  	by the driving routines for the specific frame buffer
 *	    	  	type.
 *	parent	  	set true if the frame buffer is actually a SunWindows
 *	    	  	window.
 */
typedef struct {
    unsigned char *	fb;	/* Frame buffer itself */
    int		fd;		/* frame buffer for ioctl()s, CG2 only */
    struct fbtype info;		/* Frame buffer characteristics */
    void	(*EnterLeave)();/* screen switch, CG4 only */
    unsigned char *	fbPriv;	/* fbattr stuff, for the real type */
} fbFd;

typedef struct _sunFbDataRec {
    Bool	(*init)();	/* init procedure for this fb */
    char	*name;		/* /usr/include/fbio names */
} sunFbDataRec;

#ifndef XKB
extern Bool		sunAutoRepeatHandlersInstalled;
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
#endif
extern sunFbDataRec	sunFbData[];
extern fbFd		sunFbs[];
extern Bool		sunSwapLkeys;
extern int		sunScreenIndex;
extern int*		sunProtected;

extern Bool		sunCursorInitialize();
extern void		sunDisableCursor();
extern int		sunChangeKbdTranslation();
extern void		sunNonBlockConsoleOff();
extern void		sunSetTimeSinceLastInputEvent();
extern void		sunEnqueueEvents();
extern int		sunGXInit();
extern Bool		sunSaveScreen();
extern Bool		sunScreenInit();
extern pointer		sunMemoryMap();
extern Bool		sunScreenAllocate();
extern Bool		sunInitCommon();

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

#endif
