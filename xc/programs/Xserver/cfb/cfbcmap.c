/*
 * The Sun X drivers are a product of Sun Microsystems, Inc. and are provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify these drivers without charge, but are not authorized
 * to license or distribute them to anyone else except as part of a product or
 * program developed by the user.
 * 
 * THE SUN X DRIVERS ARE PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND
 * INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 * PRACTICE.
 *
 * The Sun X Drivers are provided with no support and without any obligation
 * on the part of Sun Microsystems, Inc. to assist in their use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THE SUN X
 * DRIVERS OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#include "X.h"
#include "scrnintstr.h"
#include "colormap.h"
#include "colormapst.h"
#include "resource.h"

int
cfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    *pmaps = pScreen->defColormap;
    return (1);
}

#ifdef	STATIC_COLOR
void
cfbResolveStaticColor(pred, pgreen, pblue, pVisual)
    unsigned short	*pred, *pgreen, *pblue;
    VisualPtr		pVisual;
{
    /* XXX - this works for the StaticColor visual ONLY */
    *pred &= 0xe000;
    *pgreen &= 0xe000;
    *pblue &= 0xc000;
}
#endif

ColormapPtr
cfbGetStaticColormap(pVisual)
    VisualPtr	pVisual;
{
    return (
	    (ColormapPtr)
		LookupID(screenInfo.screen[pVisual->screen].defColormap,
			 RT_COLORMAP, RC_CORE)
	    );
}

void
cfbInitialize332Colormap(pmap)
    ColormapPtr	pmap;
{
    int	i;

    for(i = 0; i < pmap->pVisual->ColormapEntries; i++)
    {
	/* XXX - assume 256 for now */
	pmap->red[i].co.local.red = (i & 0x7) << 13;
	pmap->red[i].co.local.green = (i & 0x38) << 10;
	pmap->red[i].co.local.blue = (i & 0xc0) << 8;
    }
}
