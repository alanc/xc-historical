/*
 * $XConsortium: skyScrInit.c,v 1.4 91/12/11 21:25:52 eswu Exp $ 
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991 
 *
 * All Rights Reserved 
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission. 
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS, IN NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 *
 */


/*
 * skyScrInit.c - initialize display and screen structure
 */


#include <sys/types.h>
#include <sys/hft.h>
#include <sys/entdisp.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>

#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "cursorstr.h"
#include "ibmScreen.h"
#include "ibmTrace.h"

#include "aixCursor.h"
#include "mipointer.h"
#include "misprite.h"
#include "OSio.h"

#include "skyProcs.h"
#include "skyHdwr.h"
#include "skyReg.h"
#include "skyPriv.h"


static unsigned long ddpGeneration=0;
unsigned long ddpGCPrivateIndex;


uint		skyHandle[MAXSCREENS];

ulong           SKY_SEGMENT[MAXSCREENS];
ulong           SKY_VRAM_BASE[MAXSCREENS];
ulong           SKY_VRAM_START[MAXSCREENS];
ulong           SKY_DMA0[MAXSCREENS];
ulong           SKY_DMA1[MAXSCREENS];
ulong           SKY_DMA2[MAXSCREENS];
ulong           SKY_DMA3[MAXSCREENS];

skyIORegPtr     IOREG[MAXSCREENS];		/* Skyway IO Registers   */
skyCopRegPtr    COPREG[MAXSCREENS];		/* Coprocessor registers */
skyCopRegRec    COPREG_SHADOW[MAXSCREENS];	/* Shadow of hdwr values */

ulong          *SKY_TILESTIP[MAXSCREENS];
ulong           SKY_TILESTIPID_CTR[MAXSCREENS];


extern void     miRecolorCursor();
extern void     miPaintWindow();



Bool
skyScreenInit(scrnNum, pScreen, argc, argv)
    int             scrnNum;
    ScreenPtr       pScreen;
    int             argc;	/* these two may NOT be changed */
    char          **argv;
{

    ColormapPtr     pColormap;
    Bool            retval;
    int             i;

    struct sky_map  skyRegMap;
    make_gp         skyHdwrInfo;
    ulong           segAddr;


    TRACE(("skyIO(%d,0x%x,%d,0x%x)\n", scrnNum, pScreen, argc, argv));


    /*************************************************************************
    *                                                                        *
    *                        Allocate Private Areas                          *
    *                                                                        *
    *************************************************************************/

    if (ddpGeneration != serverGeneration)
    {
	ddpGCPrivateIndex  = AllocateGCPrivateIndex();
	ddpGeneration      = serverGeneration;
    }

    if (!AllocateGCPrivate(pScreen, ddpGCPrivateIndex, sizeof(skyPrivGCRec)))
	return FALSE;


    /*************************************************************************
    *                                                                        *
    *          Go into HFT monitor mode to gain access to skyway             *
    *                                                                        *
    *************************************************************************/

    if (ioctl(ibmScreenFD(scrnNum), HFHANDLE, &skyHandle[scrnNum]) < 0)
    {
	perror("ioctl on display");
	return FALSE;
    }

    skyHdwrInfo.pData  = (genericPtr) &skyRegMap;
    skyHdwrInfo.length = sizeof(skyRegMap);

    if (aixgsc(skyHandle[scrnNum], MAKE_GP, &skyHdwrInfo))
    {
	perror("cannot enter HFT monitor mode");
	return FALSE;
    }


    /*************************************************************************
    *                                                                        *
    *                     Initialize skyway hardware                         *
    *                                                                        *
    *************************************************************************/

    segAddr = (ulong) skyHdwrInfo.segment;

    SKY_SEGMENT[scrnNum]    = segAddr;
    SKY_VRAM_BASE[scrnNum]  = skyRegMap.vr_addr;
    SKY_VRAM_START[scrnNum] = segAddr | skyRegMap.vr_addr;
    IOREG[scrnNum]          = (skyIORegPtr) (segAddr | skyRegMap.io_addr);
    COPREG[scrnNum]         = (skyCopRegPtr) (segAddr | skyRegMap.cp_addr);

    /* Initialize installed TileStip list */

    SKY_TILESTIPID_CTR[scrnNum] = 0;

    SKY_TILESTIP[scrnNum] = xalloc(sizeof(ulong)*SKY_TILESTIP_AREAS);
    if (SKY_TILESTIP[scrnNum] == NULL)
	return FALSE;

    for (i=0; i<SKY_TILESTIP_AREAS; i++)
	SKY_TILESTIP[scrnNum][i] = 0;


    TRACE(("SEGADDR[%d]      = %x\n", scrnNum, segAddr));
    TRACE(("VRAM_SEGMENT[%d] = %x\n", scrnNum, SKY_SEGMENT[scrnNum]));
    TRACE(("VRAM_BASE[%d]    = %x\n", scrnNum, SKY_VRAM_BASE[scrnNum]));
    TRACE(("VRAM_START[%d]   = %x\n", scrnNum, SKY_VRAM_START[scrnNum]));
    TRACE(("IOREG[%d]        = %x\n", scrnNum, IOREG[scrnNum]));
    TRACE(("COPREG[%d]       = %x\n", scrnNum, COPREG[scrnNum]));

    skyHdwrInit(scrnNum);




    /*************************************************************************
    *                                                                        *
    *               Setup skyway dependent data and vectors                  *
    *                                                                        *
    *************************************************************************/

    if (!cfbSetupScreen(pScreen, SKY_VRAM_START[scrnNum],
			SKY_WIDTH, SKY_HEIGHT, 92, 92, SKY_WIDTH))
	return FALSE;

    pScreen->CreateGC = skyCreateGC;

    pScreen->RealizeCursor = skyRealizeCursor;
    pScreen->UnrealizeCursor = skyUnrealizeCursor;
    pScreen->DisplayCursor = skyDisplayCursor;
    pScreen->RecolorCursor = miRecolorCursor;

    pScreen->PaintWindowBackground = miPaintWindow;
    pScreen->PaintWindowBorder = miPaintWindow;
    pScreen->CopyWindow = skyCopyWindow;

    pScreen->InstallColormap = skyInstallColormap;
    pScreen->UninstallColormap = skyUninstallColormap;
    pScreen->ListInstalledColormaps = skyListInstalledColormaps;
    pScreen->StoreColors = skyStoreColors;

    pScreen->QueryBestSize = skyQueryBestSize;

    if (!cfbFinishScreenInit(pScreen, SKY_VRAM_START[scrnNum],
	       SKY_WIDTH, SKY_HEIGHT, 92, 92, SKY_WIDTH))
	return FALSE;


    if (!cfbCreateDefColormap(pScreen))
	return FALSE;


    /* XXX - Should wrap or else BackingStore will leak memory */
    pScreen->CloseScreen = skyScreenClose;


    /*************************************************************************
    *                                                                        *
    *                Setup system-dependent data and vectors                 *
    *                                                                        *
    *************************************************************************/

    ibmScreen(scrnNum) = pScreen;
    pScreen->SaveScreen = ibmSaveScreen;


    /* Warning: miScreenInit sets BlockHandler and WakeHandler to NoopDDA,
     * so do not move this before cfbFinishScreenInit().
     */

    pScreen->BlockHandler = AIXBlockHandler;
    pScreen->WakeupHandler = AIXWakeupHandler;
    pScreen->blockData = (pointer) 0;
    pScreen->wakeupData = (pointer) 0;

    pScreen->SetCursorPosition = AIXSetCursorPosition;
    pScreen->CursorLimits = AIXCursorLimits;
    pScreen->PointerNonInterestBox = AIXPointerNonInterestBox;
    pScreen->ConstrainCursor = AIXConstrainCursor;

    return TRUE;
}



/*
 * skyScreenClose Screen is being destroyed. Release its resources. 
 */

Bool
skyScreenClose(scrnNum, pScreen)
    int             scrnNum;
    ScreenPtr       pScreen;
{
    unmake_gp       skyInfo;

    TRACE(("skyScreenClose(scrnNum=%d, pScreen=0x%x)\n", scrnNum, pScreen));

    if (pScreen->devPrivate)
	Xfree(pScreen->devPrivate);

    if (aixgsc(skyHandle[scrnNum], UNMAKE_GP, &skyInfo))
    {
	TRACE(("aixgsc UNMAKE_GP failed\n"));
	return -1;
    }

    return TRUE;
}
