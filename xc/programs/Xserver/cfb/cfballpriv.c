/*
 * $XConsortium: cfballpriv.c,v 1.3 93/07/12 16:28:41 dpw Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

int cfbWindowPrivateIndex;
int cfbGCPrivateIndex;
#ifdef CFB_NEED_SCREEN_PRIVATE
int cfbScreenPrivateIndex;
#endif

extern RegionPtr (*cfbPuntCopyPlane)();

Bool
cfbAllocatePrivates(pScreen, window_index, gc_index)
    ScreenPtr	pScreen;
    int		*window_index, *gc_index;
{
    if (!window_index || !gc_index ||
	*window_index == -1 && *gc_index == -1)
    {
    	if (!mfbAllocatePrivates(pScreen,
			     	 &cfbWindowPrivateIndex, &cfbGCPrivateIndex))
	    return FALSE;
    	if (window_index)
	    *window_index = cfbWindowPrivateIndex;
    	if (gc_index)
	    *gc_index = cfbGCPrivateIndex;
    }
    else
    {
	cfbWindowPrivateIndex = *window_index;
	cfbGCPrivateIndex = *gc_index;
    }
    if (!AllocateWindowPrivate(pScreen, cfbWindowPrivateIndex,
			       sizeof(cfbPrivWin)) ||
	!AllocateGCPrivate(pScreen, cfbGCPrivateIndex, sizeof(cfbPrivGC)))
	return FALSE;
    cfbPuntCopyPlane = miCopyPlane;
#ifdef CFB_NEED_SCREEN_PRIVATE
    cfbScreenPrivateIndex = AllocateScreenPrivateIndex ();
    if (cfbScreenPrivateIndex == -1)
	return FALSE;
#endif
    return TRUE;
}
