
/* $XConsortium: sunFbs.c,v 1.3 93/09/26 12:27:57 rws Exp $ */

/*
 * Copyright 1990, 1993 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

/*
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/****************************************************************/
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

#include "sun.h"
#include <sys/mman.h>

int sunScreenIndex;

static unsigned long generation = 0;

pointer sunMemoryMap (len, off, fd)
    size_t	len;
    off_t	off;
    int		fd;

{
    int		pagemask, mapsize;
    caddr_t	addr;
    pointer	mapaddr;

#ifdef SVR4
    pagemask = sysconf(_SC_PAGESIZE) - 1;
#else 
    pagemask = getpagesize() - 1;
#endif
    mapsize = ((int) len + pagemask) & ~pagemask;
    addr = 0;

#ifndef _MAP_NEW
    if ((addr = (caddr_t) valloc (mapsize)) == NULL) {
	Error ("Couldn't allocate frame buffer memory");
	(void) close (fd);
	return NULL;
    }
#endif

    /* 
     * try and make it private first, that way once we get it, an
     * interloper, e.g. another server, can't get this frame buffer,
     * and if another server already has it, this one won't.
     */
    if ((int)(mapaddr = (pointer) mmap (addr,
		mapsize,
		PROT_READ | PROT_WRITE, MAP_PRIVATE,
		fd, off)) == -1)
	mapaddr = (pointer) mmap (addr,
		    mapsize,
		    PROT_READ | PROT_WRITE, MAP_SHARED,
		    fd, off);
    if (mapaddr == (pointer) -1) {
	Error ("mapping frame buffer memory");
	(void) close (fd);
	mapaddr = (pointer) NULL;
    }
    return mapaddr;
}


Bool sunScreenAllocate (pScreen)
    ScreenPtr	pScreen;
{
    sunScreenPtr    pPrivate;
    extern int AllocateScreenPrivateIndex();

    if (generation != serverGeneration)
    {
	sunScreenIndex = AllocateScreenPrivateIndex();
	if (sunScreenIndex < 0)
	    return FALSE;
	generation = serverGeneration;
    }
    pPrivate = (sunScreenPtr) xalloc (sizeof (sunScreenRec));
    if (!pPrivate)
	return FALSE;

    pScreen->devPrivates[sunScreenIndex].ptr = (pointer) pPrivate;
    return TRUE;
}

Bool sunSaveScreen (pScreen, on)
    ScreenPtr	pScreen;
    int		on;
{
    int		state;

    if (on != SCREEN_SAVER_FORCER)
    {
	if (on == SCREEN_SAVER_ON)
	    state = 0;
	else
	    state = 1;
	(void) ioctl(sunFbs[pScreen->myNum].fd, FBIOSVIDEO, &state);
    }
    return( TRUE );
}

static Bool closeScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
    Bool    ret;

    (void) OsSignal (SIGIO, SIG_IGN);
    sunDisableCursor (pScreen);
    pScreen->CloseScreen = pPrivate->CloseScreen;
    ret = (*pScreen->CloseScreen) (i, pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    xfree ((pointer) pPrivate);
    return ret;
}

Bool sunScreenInit (pScreen)
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);
#ifndef XKB
    extern void   sunBlockHandler();
    extern void   sunWakeupHandler();
    static ScreenPtr autoRepeatScreen;
#endif
    extern miPointerScreenFuncRec   sunPointerScreenFuncs;

    pPrivate->installedMap = 0;
    pPrivate->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = closeScreen;
    pScreen->SaveScreen = sunSaveScreen;
#ifndef XKB
    /*
     *	Block/Unblock handlers
     */
    if (sunAutoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	sunAutoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = sunBlockHandler;
        pScreen->WakeupHandler = sunWakeupHandler;
    }
#endif
    if (!sunCursorInitialize (pScreen))
	miDCInitialize (pScreen, &sunPointerScreenFuncs);
    return TRUE;
}

Bool sunInitCommon (scrn, pScrn, offset, init1, init2, cr_cm, save, fb_off)
    int		scrn;
    ScreenPtr	pScrn;
    off_t	offset;
    Bool	(*init1)();
    void	(*init2)();
    Bool	(*cr_cm)();
    Bool	(*save)();
    int		fb_off;
{
    unsigned char*	fb = sunFbs[scrn].fb;

    if (!sunScreenAllocate (pScrn))
	return FALSE;
    if (!fb) {
	if ((fb = sunMemoryMap ((size_t) sunFbs[scrn].info.fb_size, 
			     offset, 
			     sunFbs[scrn].fd)) == NULL)
	    return FALSE;
	sunFbs[scrn].fb = fb;
    }
    /* mfbScreenInit() or cfbScreenInit() */
    if (!(*init1)(pScrn, fb + fb_off,
	    sunFbs[scrn].info.fb_width,
	    sunFbs[scrn].info.fb_height,
	    monitorResolution, monitorResolution,
	    sunFbs[scrn].info.fb_width))
	    return FALSE;
    /* sunCGScreenInit() if cfb... */
    if (init2)
	(*init2)(pScrn);
    if (!sunScreenInit(pScrn))
	return FALSE;
    (void) (*save) (pScrn, SCREEN_SAVER_OFF);
    return (*cr_cm)(pScrn);
}

