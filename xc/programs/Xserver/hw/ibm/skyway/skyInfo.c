/*
 * $XConsortium: ibmColor.c,v 1.1 91/05/10 08:59:17 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/*
 * skyInfo.c - screen data structure
 */

#include "X.h"
#include "misc.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "ibmScreen.h"
#include "skyProcs.h"
#include "skyHdwr.h"

#include "hftUtils.h"

#ifndef SOFTWARE_CURSOR
extern void skyMoveCursor();   /*in skyCur.c */
extern void skyRemoveCursor();   /*in skyCur.c */
#endif

PixmapFormatRec skyPixmapFormats[]= { { 8, 8, 32 } };

ibmPerScreenInfo skywayScreenInfoStruct = {
	{ 0, 0, SKYWAY_WIDTH - 1, SKYWAY_HEIGHT - 1 },
	1,
	skyPixmapFormats,       /* pixmap formats */
	skyScreenIO,
	skyProbe,
#ifndef SOFTWARE_CURSOR
	skyRemoveCursor,    /*(*ibm_HideCursor)()*/
#else
        0,
#endif
	"-colorgda",
	"",
	"",
	skySaveState,
	skyRestoreState,
	0,                      /* ibm_Screen */
	0,                      /* ibm_ScreenFD */
	0,                      /* ibm_Wanted */
	0,0,                    /* ibm_CursorHotX,ibm_CursorHotY */
#ifndef SOFTWARE_CURSOR
	skyMoveCursor,          /* (*ibm_CursorShow)() */
#else
	0,                      /* (*ibm_CursorShow)() */
#endif
	0,                      /* ibm_CurrentCursor */
	0,                      /* ibm_ScreenState */
	HFT_SKYWAY_ID           /* ibm_DeviceID */
} ;
