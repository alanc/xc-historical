/* $XConsortium: xf86_HWlib.h,v 1.3 95/01/05 20:41:28 kaleb Exp kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/xf86_HWlib.h,v 3.5 1995/01/15 10:33:36 dawes Exp $ */
/*
 * Copyright 1990, 1991 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 * Copyright 1993 by Kevin Martin <martin@cs.unc.edu>
 * Copyright 1993 by Rickard Faith <faith@cs.unc.edu>
 * Copyright 1993 by Scott Laird <lair@midway.uchicago.edu>
 * Copyright 1993 by Tiago Gons <tiago@comosjn.hobby.nl>
 * Copyright 1993 by Jon Tombs <jon@robots.ox.ac.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the above listed copyright holders 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  The above listed
 * copyright holders make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THE ABOVE LISTED COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDERS BE 
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifndef _XF86_HWLIB_H
#define _XF86_HWLIB_H

/***************************************************************************/
/* Macro definitions                                                       */
/***************************************************************************/

/*
 * These are used to tell the clock select functions to save/restore
 * registers they use.
 */
#define CLK_REG_SAVE	-1
#define CLK_REG_RESTORE	-2

/***************************************************************************/
/* Prototypes                                                              */
/***************************************************************************/

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

/* ICD2061Aalt.c */
extern void AltICD2061SetClock(
#if NeedFunctionPrototypes
	long,
	int
#endif
);

/* ICD2061Acal.c */
extern long ICD2061ACalcClock(
#if NeedFunctionPrototypes
	long,
	int
#endif
);

/* ICD2061Aset.c */
extern void ICD2061ASetClock(
#if NeedFunctionPrototypes
	long
#endif
);

/*  SC11412.c */
extern Bool SC11412SetClock(
#if NeedFunctionPrototypes
     long 
#endif
);

/*  ICS2595.c */
extern Bool ICS2595SetClock(
#if NeedFunctionPrototypes
     long
#endif
);


/* xf86_ClkPr.c */
extern void xf86GetClocks(
#if NeedFunctionPrototypes
	int,
	Bool (*)(), 
	void (*)(),
	void (*)(),
	int,
	int,
	int,
	int,
	ScrnInfoRec *
#endif
);

/* ATTDac.c */
extern void xf86dactopel(
#if NeedFunctionPrototypes
	void
#endif
);

extern unsigned char xf86dactocomm(
#if NeedFunctionPrototypes
	void
#endif
);

extern unsigned char xf86getdaccomm(
#if NeedFunctionPrototypes
	void
#endif
);

extern void xf86setdaccomm(
#if NeedFunctionPrototypes
	unsigned char
#endif
);

extern void xf86setdaccommbit(
#if NeedFunctionPrototypes
	unsigned char
#endif
);

extern void xf86clrdaccommbit(
#if NeedFunctionPrototypes
	unsigned char
#endif
);

/* S3gendac.c */
extern int S3gendacSetClock(
#if NeedFunctionPrototypes
	long,
	int
#endif
);

/* Ti3025clk.c */
extern void Ti3025SetClock(
#if NeedFunctionPrototypes
	long,
	int
#endif
);

/* Ch8391clk.c */
extern void Chrontel8391SetClock(
#if NeedFunctionPrototypes
	long,
	int
#endif
);

/* BUSmemcpy.s */
extern void BusToMem(
#if NeedFunctionPrototypes
	void *,
	void *,
	int
#endif
);

extern void MemToBus(
#if NeedFunctionPrototypes
	void *,
	void *,
	int
#endif
);

/* SlowBcopy.s */
extern void SlowBcopy(
#if NeedFunctionPrototypes
	char *,
	char *,
	int
#endif
);

/* IODelay.s */
extern void GlennsIODelay(
#if NeedFunctionPrototypes
	void
#endif
);

_XFUNCPROTOEND

#endif /* _XF86_HWLIB_H */
