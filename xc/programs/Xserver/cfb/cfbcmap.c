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


#include "X.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"

#ifdef	STATIC_COLOR

static ColormapPtr InstalledMaps[MAXSCREENS];

int
cfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}


void
cfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];

    if(pmap != oldpmap)
    {
	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */
	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

    }
}

void
cfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr curpmap = InstalledMaps[index];

    if(pmap == curpmap)
    {
        /* Uninstall pmap */
	WalkTree(pmap->pScreen, TellLostMap, (char *)&pmap->mid);
	curpmap = (ColormapPtr) LookupID(pmap->pScreen->defColormap,
					 RT_COLORMAP, RC_CORE);
	/* Install default map */
	InstalledMaps[index] = curpmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&curpmap->mid);
    }
	
}

#endif

void
cfbResolveColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred, *pgreen, *pblue;
    register VisualPtr	pVisual;
{
    int shift = 16 - pVisual->bitsPerRGBValue;
    unsigned lim = (1 << pVisual->bitsPerRGBValue) - 1;

    if ((pVisual->class == PseudoColor) || (pVisual->class == DirectColor))
    {
	*pred = ((*pred >> shift) * 65535) / lim;
	*pgreen = ((*pgreen >> shift) * 65535) / lim;
	*pblue = ((*pblue >> shift) * 65535) / lim;
    }
    else if ((pVisual->class == GrayScale) || (pVisual->class == StaticGray))
    {
	*pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
	*pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
    }
    else
    {
	*pred = ((*pred >> shift) * 65535) / lim;
	*pgreen = ((*pgreen >> shift) * 65535) / lim;
	lim = pVisual->nplanes - (2 * pVisual->bitsPerRGBValue);
	*pgreen = ((*pgreen >> (16 - lim)) * 65535) / ((1 << lim) - 1);
    }
}

Bool
cfbInitializeColormap(pmap)
    register ColormapPtr	pmap;
{
    register unsigned i;
    register VisualPtr pVisual;
    unsigned lim;

    pVisual = pmap->pVisual;
    if (pVisual->class == TrueColor)
    {
	lim = (1 << pVisual->bitsPerRGBValue) - 1;
	for(i = 0; i <= lim; i++)
	{
	    pmap->red[i].co.local.red = (i * 65535) / lim;
	    pmap->green[i].co.local.green = pmap->red[i].co.local.red;
	}
	lim = (1 << (pVisual->nplanes - (2 * pVisual->bitsPerRGBValue))) - 1;
	for(i = 0; i <= lim; i++)
	    pmap->blue[i].co.local.blue = (i * 65535) / lim;
    }
    else if (pVisual->class == StaticColor)
    {
	unsigned limrg, limb;

	lim = pVisual->ColormapEntries - 1;
	limrg = (1 << pVisual->bitsPerRGBValue) - 1;
	limb = (1 << (pVisual->nplanes - (2 * pVisual->bitsPerRGBValue))) - 1;
	for(i = 0; i <= lim; i++)
	{
	    pmap->red[i].co.local.red = (((i & pVisual->redMask) >>
					  pVisual->offsetRed) * 65535) /
					    limrg;
	    pmap->red[i].co.local.green = (((i & pVisual->greenMask) >>
					    pVisual->offsetGreen) * 65535) /
					      limrg;
	    pmap->red[i].co.local.blue = (((i & pVisual->blueMask) >>
					   pVisual->offsetBlue) * 65535) /
					     limb;
	}
    }
    else if (pVisual->class == StaticGray)
    {
	lim = pVisual->ColormapEntries - 1;
	for(i = 0; i <= lim; i++)
	{
	    pmap->red[i].co.local.red = (i * 65535) / lim;
	    pmap->red[i].co.local.green = pmap->red[i].co.local.red;
	    pmap->red[i].co.local.blue = pmap->red[i].co.local.red;
	}
    }
    return TRUE;
}
