
#define PSZ 32
#include <stdio.h>
#include "sun.h"
#include "cfb32/cfb.h"

#define PIXPG_24BIT_COLOR 5
#define PIXPG_24BIT_COLOR_INDEX (PIXPG_24BIT_COLOR << 25)
#define PR_FORCE_UPDATE (1 << 24)

static void CG24UpdateColormap(pScreen, index, count, rmap, gmap, bmap)
    ScreenPtr	pScreen;
    int		index, count;
    u_char	*rmap, *gmap, *bmap;
{
    struct fbcmap sunCmap;

    sunCmap.index = index | PIXPG_24BIT_COLOR_INDEX | PR_FORCE_UPDATE;
    sunCmap.count = count;
    sunCmap.red = &rmap[index];
    sunCmap.green = &gmap[index];
    sunCmap.blue = &bmap[index];

    if (ioctl(sunFbs[pScreen->myNum].fd, FBIOPUTCMAP, &sunCmap) == -1)
	FatalError( "CG24UpdateColormap: FBIOPUTCMAP failed\n");
}

static void CG24StoreColors (pmap, ndef, pdefs)
    ColormapPtr pmap;
    int ndef;
    xColorItem* pdefs;
{
  struct fbcmap cmap;
  u_char rmap[256], gmap[256], bmap[256];
  SetupScreen (pmap->pScreen);
  VisualPtr pVisual = pmap->pVisual;
  int i;

  if (pPrivate->installedMap != NULL && pPrivate->installedMap != pmap)
    return;
  for (i = 0; i < 256; i++) {
    rmap[i] = pmap->red[i].co.local.red >> 8;
    gmap[i] = pmap->green[i].co.local.green >> 8;
    bmap[i] = pmap->blue[i].co.local.blue >> 8;
  }
  while (ndef--) {
    i = pdefs->pixel;
    if (pdefs->flags & DoRed)
      rmap[(i & pVisual->redMask) >> pVisual->offsetRed] = (pdefs->red >> 8);
    if (pdefs->flags & DoGreen)
      gmap[(i & pVisual->greenMask) >> pVisual->offsetGreen] = (pdefs->green >> 8);
    if (pdefs->flags & DoBlue)
      bmap[(i & pVisual->blueMask) >> pVisual->offsetBlue] = (pdefs->blue >> 8);
    pdefs++;
  }
  CG24UpdateColormap (pmap->pScreen, 0, 256, rmap, gmap, bmap);
}

#define CG8_COLOR_OFFSET 0x40000

static void CG24ScreenInit (pScreen)
    ScreenPtr pScreen;
{
#ifndef STATIC_COLOR
    SetupScreen (pScreen);
#endif
    int i;

    /* Make sure the overlay plane is disabled */
    for (i = 0; i < CG8_COLOR_OFFSET; i++)
	sunFbs[pScreen->myNum].fb[i] = 0;

#if 0
    pScreen->whitePixel = 0xffffffff;
    pScreen->blackPixel = 0;
#endif

#ifndef STATIC_COLOR
    pScreen->InstallColormap = sunInstallColormap;
    pScreen->UninstallColormap = sunUninstallColormap;
    pScreen->ListInstalledColormaps = sunListInstalledColormaps;
    pScreen->StoreColors = CG24StoreColors;
    pPrivate->UpdateColormap = CG24UpdateColormap;
#endif
}

Bool sunCG8Init (screen, pScreen, argc, argv)
    int		    screen;    	/* what screen am I going to be */
    ScreenPtr	    pScreen;  	/* The Screen to initialize */
    int		    argc;    	/* The number of the Server's arguments. */
    char	    **argv;   	/* The arguments themselves. Don't change! */
{
    sunFbs[screen].EnterLeave = (void (*)())NoopDDA;
    return sunInitCommon (screen, pScreen, (off_t) 0,
	cfb32ScreenInit, CG24ScreenInit,
	cfbCreateDefColormap, sunSaveScreen, CG8_COLOR_OFFSET);
}

