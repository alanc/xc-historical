/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/* $Header: swaprep.c,v 1.20 87/08/01 13:32:03 toddb Locked $ */

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "window.h"
#include "font.h"
#include "gc.h"
#include "os.h"
#include "resource.h"
#include "selection.h"
#include "colormap.h"
#include "dixstruct.h"
#include "extension.h"
#include "input.h"
#include "scrnintstr.h"
#include "cursor.h"

void SwapVisual(), SwapConnSetup(), SwapWinRoot();
int (* EventSwapVector[256]) ();
int (* ReplySwapVector[256]) ();
/* copy long from src to dst byteswapping on the way */
#define cpswapl(src, dst) \
                 ((char *)&(dst))[0] = ((char *) &(src))[3];\
                 ((char *)&(dst))[1] = ((char *) &(src))[2];\
                 ((char *)&(dst))[2] = ((char *) &(src))[1];\
                 ((char *)&(dst))[3] = ((char *) &(src))[0];

/* copy short from src to dst byteswapping on the way */
#define cpswaps(src, dst)\
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0];

/* Thanks to Jack Palevich for testing and subsequently rewriting all this */
void
Swap32Write(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;
    long	*pbuf;
{
    int		n, i;

    size >>= 2;
    for(i = 0; i < size; i++)
    {
	swapl(&pbuf[i], n);
    }
    WriteToClient(pClient, size << 2, pbuf);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
Swap16Write(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;
    short	*pbuf;
{
    int		n, i;

    size >>= 1;
    for(i = 0; i < size; i++)
    {
	swaps(&pbuf[i], n);
    }
    WriteToClient(pClient, size << 1, pbuf);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
Write8(pClient, size, pbuf)
    ClientPtr	pClient;
    int		size;
    char	*pbuf;
{
    WriteToClient(pClient, size, pbuf);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

/* Extra-small reply */
void
SGenericReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGenericReply		*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
}

/* Extra-large reply */
void
SGetWindowAttributesReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetWindowAttributesReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->visualID, n);
    swaps(&pRep->class, n);
    swapl(&pRep->backingBitPlanes, n);
    swapl(&pRep->backingPixel, n);
    swapl(&pRep->colormap, n);
    swapl(&pRep->allEventMasks, n);
    swapl(&pRep->yourEventMask, n);
    swaps(&pRep->doNotPropagateMask, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SGetGeometryReply(pClient, size, pRep)
    xGetGeometryReply	*pRep;
    ClientPtr		pClient;
    int			size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->root, n);
    swaps(&pRep->x, n);
    swaps(&pRep->y, n);
    swaps(&pRep->width, n);
    swaps(&pRep->height, n);
    swaps(&pRep->borderWidth, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SQueryTreeReply(pClient, size, pRep)
    xQueryTreeReply	*pRep;
    ClientPtr		pClient;
    int			size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    /* Don't install a SwapReplyFunc unless
     * ProcQueryTree will call it.
     */
    if (pRep->length)
        pClient->pSwapReplyFunc = Swap32Write;
    swapl(&pRep->length, n);
    swapl(&pRep->root, n);
    swapl(&pRep->parent, n);
    swaps(&pRep->nChildren, n);
    WriteToClient(pClient, size, pRep);
}

void
SInternAtomReply(pClient, size, pRep)
    xInternAtomReply	*pRep;
    ClientPtr		pClient;
    int			size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->atom, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SGetAtomNameReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetAtomNameReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nameLength, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}


void
SGetPropertyReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetPropertyReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->propertyType, n);
    swapl(&pRep->bytesAfter, n);
    swapl(&pRep->nItems, n);
    WriteToClient(pClient, size, pRep);
    switch(pRep->format)
    {
	case 8:
            pClient->pSwapReplyFunc = Write8;
	    break;
	case 16:
            pClient->pSwapReplyFunc = Swap16Write;
	    break;
	case 32:
            pClient->pSwapReplyFunc = Swap32Write;
	    break;
    }
}

void
SListPropertiesReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xListPropertiesReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nProperties, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Swap32Write;
}

void
SGetSelectionOwnerReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetSelectionOwnerReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->owner, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SGrabPointerReply(pClient, size, pRep)
    xGrabPointerReply	*pRep;
    ClientPtr		pClient;
    int			size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}


void
SGrabKeyboardReply(pClient, size, pRep)
    xGrabKeyboardReply	*pRep;
    ClientPtr			pClient;
    int				size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}


void
SQueryPointerReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xQueryPointerReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->root, n);
    swapl(&pRep->child, n);
    swaps(&pRep->rootX, n);
    swaps(&pRep->rootY, n);
    swaps(&pRep->winX, n);
    swaps(&pRep->winY, n);
    swaps(&pRep->mask, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SwapTimeCoordWrite(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xTimecoord			*pRep;
{
    int	i, n;
    xTimecoord			*pRepT;

    n = size / sizeof(xTimecoord);
    pRepT = pRep;
    for(i = 0; i < n; i++)
    {
	SwapTimecoord(pRepT);
	pRepT++;
    }
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;

}
void
SGetMotionEventsReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetMotionEventsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->nEvents, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = SwapTimeCoordWrite;
}

void
STranslateCoordsReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xTranslateCoordsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->child, n);
    swaps(&pRep->dstX, n);
    swaps(&pRep->dstY, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SGetInputFocusReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xGetInputFocusReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->focus, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

/* extra long reply */
void
SQueryKeymapReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xQueryKeymapReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SQueryFontReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xQueryFontReply	*pRep;
{
    SwapFont(pRep);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ()) NULL;
}

void
SQueryTextExtentsReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xQueryTextExtentsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->fontAscent, n);
    swaps(&pRep->fontDescent, n);
    swaps(&pRep->overallAscent, n);
    swaps(&pRep->overallDescent, n);
    swapl(&pRep->overallWidth, n);
    swapl(&pRep->overallLeft, n);
    swapl(&pRep->overallRight, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ()) NULL;
}

void
SListFontsReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xListFontsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nFonts, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SListFontsWithInfoReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xListFontsWithInfoReply	*pRep;
{
    int n;

    /* XXXXXXXXXXXXX When Sue has implemented the unswapped version, . . .  */
    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nFonts, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetFontPathReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xGetFontPathReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nPaths, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetImageReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xGetImageReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->visual, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
    /* Fortunately, image doesn't need swapping */
}

void
SListInstalledColormapsReply(pClient, size, pRep)
    ClientPtr				pClient;
    int					size;
    xListInstalledColormapsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nColormaps, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Swap32Write;
}

void
SAllocColorReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xAllocColorReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->red, n);
    swaps(&pRep->green, n);
    swaps(&pRep->blue, n);
    swapl(&pRep->pixel, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SAllocNamedColorReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xAllocNamedColorReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->pixel, n);
    swaps(&pRep->exactRed, n);
    swaps(&pRep->exactGreen, n);
    swaps(&pRep->exactBlue, n);
    swaps(&pRep->screenRed, n);
    swaps(&pRep->screenGreen, n);
    swaps(&pRep->screenBlue, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SAllocColorCellsReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xAllocColorCellsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nPixels, n);
    swaps(&pRep->nMasks, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Swap32Write;
}


void
SAllocColorPlanesReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xAllocColorPlanesReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nPixels, n);
    swapl(&pRep->redMask, n);
    swapl(&pRep->greenMask, n);
    swapl(&pRep->blueMask, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Swap32Write;
}

void
SQColorsExtend(pClient, size, prgb)
    ClientPtr	pClient;
    int		size;
    xrgb	*prgb;
{
    int		i, n;
    xrgb	*prgbT;

    n = size / sizeof(xrgb);
    prgbT = prgb;
    for(i = 0; i < n; i++)
    {
	SwapRGB(prgbT);
	prgbT++;
    }
    WriteToClient(pClient, size, prgb);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SQueryColorsReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xQueryColorsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nColors, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = SQColorsExtend;
}

void
SLookupColorReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xLookupColorReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->exactRed, n);
    swaps(&pRep->exactGreen, n);
    swaps(&pRep->exactBlue, n);
    swaps(&pRep->screenRed, n);
    swaps(&pRep->screenGreen, n);
    swaps(&pRep->screenBlue, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SQueryBestSizeReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xQueryBestSizeReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->width, n);
    swaps(&pRep->height, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SQueryExtensionReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xQueryExtensionReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SListExtensionsReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xListExtensionsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetKeyboardMappingReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetKeyboardMappingReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Swap32Write;
}

void
SGetPointerMappingReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetPointerMappingReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetModifierMappingReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetModifierMappingReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetKeyboardControlReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetKeyboardControlReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swapl(&pRep->ledMask, n);
    swaps(&pRep->bellPitch, n);
    swaps(&pRep->bellDuration, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = Write8;
}

void
SGetPointerControlReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetPointerControlReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->accelNumerator, n);
    swaps(&pRep->accelDenominator, n);
    swaps(&pRep->threshold, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SGetScreenSaverReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xGetScreenSaverReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swaps(&pRep->timeout, n);
    swaps(&pRep->interval, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
void
SLHostsExtend(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xListHostsReply	*pRep;
{
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SListHostsReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xListHostsReply	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->length, n);
    swaps(&pRep->nHosts, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = SLHostsExtend;
}



void
SErrorEvent(pClient, size, pRep)
    ClientPtr	pClient;
    int		size;
    xError	*pRep;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    swapl(&pRep->resourceID, n);
    swaps(&pRep->minorCode, n);
    WriteToClient(pClient, size, pRep);
    pClient->pSwapReplyFunc = (void (*) ())NULL;
}

void
SKeyButtonPtrEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.keyButtonPointer.time,
        pEventT->u.keyButtonPointer.time);
    cpswapl(pEvent->u.keyButtonPointer.root,
        pEventT->u.keyButtonPointer.root);
    cpswapl(pEvent->u.keyButtonPointer.event,
        pEventT->u.keyButtonPointer.event);
    cpswapl(pEvent->u.keyButtonPointer.child,
        pEventT->u.keyButtonPointer.child);
    cpswaps(pEvent->u.keyButtonPointer.rootX,
        pEventT->u.keyButtonPointer.rootX);
    cpswaps(pEvent->u.keyButtonPointer.rootY,
	pEventT->u.keyButtonPointer.rootY);
    cpswaps(pEvent->u.keyButtonPointer.eventX,
        pEventT->u.keyButtonPointer.eventX);
    cpswaps(pEvent->u.keyButtonPointer.eventY,
        pEventT->u.keyButtonPointer.eventY);
    cpswaps(pEvent->u.keyButtonPointer.state,
        pEventT->u.keyButtonPointer.state);
    pEventT->u.keyButtonPointer.sameScreen = 
	pEvent->u.keyButtonPointer.sameScreen;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
    
}
void
SEnterLeaveEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.enterLeave.time, pEventT->u.enterLeave.time);
    cpswapl(pEvent->u.enterLeave.root, pEventT->u.enterLeave.root);
    cpswapl(pEvent->u.enterLeave.event, pEventT->u.enterLeave.event);
    cpswapl(pEvent->u.enterLeave.child, pEventT->u.enterLeave.child);
    cpswaps(pEvent->u.enterLeave.rootX, pEventT->u.enterLeave.rootX);
    cpswaps(pEvent->u.enterLeave.rootY, pEventT->u.enterLeave.rootY);
    cpswaps(pEvent->u.enterLeave.eventX, pEventT->u.enterLeave.eventX);
    cpswaps(pEvent->u.enterLeave.eventY, pEventT->u.enterLeave.eventY);
    cpswaps(pEvent->u.enterLeave.state, pEventT->u.enterLeave.state);
    pEventT->u.enterLeave.mode = pEvent->u.enterLeave.mode;
    pEventT->u.enterLeave.flags = pEvent->u.enterLeave.flags;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}
void
SFocusEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.focus.window, pEventT->u.focus.window);
    pEventT->u.focus.mode = pEvent->u.focus.mode;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SExposeEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.expose.window, pEventT->u.expose.window);
    cpswaps(pEvent->u.expose.x, pEventT->u.expose.x);
    cpswaps(pEvent->u.expose.y, pEventT->u.expose.y);
    cpswaps(pEvent->u.expose.width, pEventT->u.expose.width);
    cpswaps(pEvent->u.expose.height, pEventT->u.expose.height);
    cpswaps(pEvent->u.expose.count, pEventT->u.expose.count);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SGraphicsExposureEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.graphicsExposure.drawable,
        pEventT->u.graphicsExposure.drawable);
    cpswaps(pEvent->u.graphicsExposure.x, 
	pEventT->u.graphicsExposure.x);
    cpswaps(pEvent->u.graphicsExposure.y, 
	pEventT->u.graphicsExposure.y);
    cpswaps(pEvent->u.graphicsExposure.width, 
	pEventT->u.graphicsExposure.width);
    cpswaps(pEvent->u.graphicsExposure.height, 
	pEventT->u.graphicsExposure.height);
    cpswaps(pEvent->u.graphicsExposure.minorEvent,
        pEventT->u.graphicsExposure.minorEvent);
    cpswaps(pEvent->u.graphicsExposure.count,
	pEventT->u.graphicsExposure.count);
    pEventT->u.graphicsExposure.majorEvent = 
    	pEvent->u.graphicsExposure.majorEvent;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SNoExposureEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.noExposure.drawable, pEventT->u.noExposure.drawable);
    cpswaps(pEvent->u.noExposure.minorEvent, pEventT->u.noExposure.minorEvent);
    pEventT->u.noExposure.majorEvent = pEvent->u.noExposure.majorEvent;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SVisibilityEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.visibility.window, pEventT->u.visibility.window);
    pEventT->u.visibility.state = pEvent->u.visibility.state;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SCreateNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.createNotify.window, pEventT->u.createNotify.window);
    cpswapl(pEvent->u.createNotify.parent, pEventT->u.createNotify.parent);
    cpswaps(pEvent->u.createNotify.x, pEventT->u.createNotify.x);
    cpswaps(pEvent->u.createNotify.y, pEventT->u.createNotify.y);
    cpswaps(pEvent->u.createNotify.width, pEventT->u.createNotify.width);
    cpswaps(pEvent->u.createNotify.height, pEventT->u.createNotify.height);
    cpswaps(pEvent->u.createNotify.borderWidth,
        pEventT->u.createNotify.borderWidth);
    pEventT->u.createNotify.override = pEvent->u.createNotify.override;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SDestroyNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.destroyNotify.event, pEventT->u.destroyNotify.event);
    cpswapl(pEvent->u.destroyNotify.window, pEventT->u.destroyNotify.window);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SUnmapNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.unmapNotify.event, pEventT->u.unmapNotify.event);
    cpswapl(pEvent->u.unmapNotify.window, pEventT->u.unmapNotify.window);
    pEventT->u.unmapNotify.fromConfigure = pEvent->u.unmapNotify.fromConfigure;
    DEALLOCATE_LOCAL(pEventT);
}

void
SMapNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.mapNotify.event, pEventT->u.mapNotify.event);
    cpswapl(pEvent->u.mapNotify.window, pEventT->u.mapNotify.window);
    pEventT->u.mapNotify.override = pEvent->u.mapNotify.override;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SMapRequestEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.mapRequest.parent, pEventT->u.mapRequest.parent);
    cpswapl(pEvent->u.mapRequest.window, pEventT->u.mapRequest.window);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SReparentEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.reparent.event, pEventT->u.reparent.event);
    cpswapl(pEvent->u.reparent.window, pEventT->u.reparent.window);
    cpswapl(pEvent->u.reparent.parent, pEventT->u.reparent.parent);
    cpswaps(pEvent->u.reparent.x, pEventT->u.reparent.x);
    cpswaps(pEvent->u.reparent.y, pEventT->u.reparent.y);
    pEventT->u.reparent.override = pEvent->u.reparent.override;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SConfigureNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.configureNotify.event,
        pEventT->u.configureNotify.event);
    cpswapl(pEvent->u.configureNotify.window,
        pEventT->u.configureNotify.window);
    cpswapl(pEvent->u.configureNotify.aboveSibling,
        pEventT->u.configureNotify.aboveSibling);
    cpswaps(pEvent->u.configureNotify.x, pEventT->u.configureNotify.x);
    cpswaps(pEvent->u.configureNotify.y, pEventT->u.configureNotify.y);
    cpswaps(pEvent->u.configureNotify.width, pEventT->u.configureNotify.width);
    cpswaps(pEvent->u.configureNotify.height,
        pEventT->u.configureNotify.height);
    cpswaps(pEvent->u.configureNotify.borderWidth,
        pEventT->u.configureNotify.borderWidth);
    pEventT->u.configureNotify.override = pEvent->u.configureNotify.override;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}
void
SConfigureRequestEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.configureRequest.parent,
        pEventT->u.configureRequest.parent);
    cpswapl(pEvent->u.configureRequest.window,
        pEventT->u.configureRequest.window);
    cpswapl(pEvent->u.configureRequest.sibling,
        pEventT->u.configureRequest.sibling);
    cpswaps(pEvent->u.configureRequest.x, pEventT->u.configureRequest.x);
    cpswaps(pEvent->u.configureRequest.y, pEventT->u.configureRequest.y);
    cpswaps(pEvent->u.configureRequest.width,
        pEventT->u.configureRequest.width);
    cpswaps(pEvent->u.configureRequest.height,
        pEventT->u.configureRequest.height);
    cpswaps(pEvent->u.configureRequest.borderWidth,
        pEventT->u.configureRequest.borderWidth);
    cpswapl(pEvent->u.configureRequest.valueMask,
        pEventT->u.configureRequest.valueMask);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}


void
SGravityEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.gravity.event, pEventT->u.gravity.event);
    cpswapl(pEvent->u.gravity.window, pEventT->u.gravity.window);
    cpswaps(pEvent->u.gravity.x, pEventT->u.gravity.x);
    cpswaps(pEvent->u.gravity.y, pEventT->u.gravity.y);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SResizeRequestEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.resizeRequest.window, pEventT->u.resizeRequest.window);
    cpswaps(pEvent->u.resizeRequest.width, pEventT->u.resizeRequest.width);
    cpswaps(pEvent->u.resizeRequest.height, pEventT->u.resizeRequest.height);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SCirculateEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.circulate.event, pEventT->u.circulate.event);
    cpswapl(pEvent->u.circulate.window, pEventT->u.circulate.window);
    cpswapl(pEvent->u.circulate.parent, pEventT->u.circulate.parent);
    pEventT->u.circulate.place = pEvent->u.circulate.place;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SPropertyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.property.window, pEventT->u.property.window);
    cpswapl(pEvent->u.property.atom, pEventT->u.property.atom);
    cpswapl(pEvent->u.property.time, pEventT->u.property.time);
    pEventT->u.property.state = pEvent->u.property.state;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SSelectionClearEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.selectionClear.time, pEventT->u.selectionClear.time);
    cpswapl(pEvent->u.selectionClear.window, pEventT->u.selectionClear.window);
    cpswapl(pEvent->u.selectionClear.atom, pEventT->u.selectionClear.atom);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SSelectionRequestEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.selectionRequest.time, pEventT->u.selectionRequest.time);
    cpswapl(pEvent->u.selectionRequest.owner,
        pEventT->u.selectionRequest.owner);
    cpswapl(pEvent->u.selectionRequest.requestor,
	pEventT->u.selectionRequest.requestor);
    cpswapl(pEvent->u.selectionRequest.selection,
	pEventT->u.selectionRequest.selection);
    cpswapl(pEvent->u.selectionRequest.target,
        pEventT->u.selectionRequest.target);
    cpswapl(pEvent->u.selectionRequest.property,
	pEventT->u.selectionRequest.property);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SSelectionNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.selectionNotify.time, pEventT->u.selectionNotify.time);
    cpswapl(pEvent->u.selectionNotify.requestor,
	pEventT->u.selectionNotify.requestor);
    cpswapl(pEvent->u.selectionNotify.selection,
	pEventT->u.selectionNotify.selection);
    cpswapl(pEvent->u.selectionNotify.target,
	pEventT->u.selectionNotify.target);
    cpswapl(pEvent->u.selectionNotify.property,
        pEventT->u.selectionNotify.property);
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SColormapEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.colormap.window, pEventT->u.colormap.window);
    cpswapl(pEvent->u.colormap.colormap, pEventT->u.colormap.colormap);
    pEventT->u.colormap.new = pEvent->u.colormap.new;
    pEventT->u.colormap.state = pEvent->u.colormap.state;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SMappingEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    pEventT->u.mappingNotify.request = pEvent->u.mappingNotify.request;
    pEventT->u.mappingNotify.firstKeyCode =
	pEvent->u.mappingNotify.firstKeyCode;
    pEventT->u.mappingNotify.count = pEvent->u.mappingNotify.count;
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SClientMessageEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    xEvent	*pEventT;

    pEventT = (xEvent *) ALLOCATE_LOCAL(sizeof(xEvent));
    pEventT->u.u.type = pEvent->u.u.type;
    pEventT->u.u.detail = pEvent->u.u.detail;  /* actually format */
    cpswaps(pEvent->u.u.sequenceNumber, pEventT->u.u.sequenceNumber);
    cpswapl(pEvent->u.clientMessage.window, pEventT->u.clientMessage.window);
    cpswapl(pEvent->u.clientMessage.u.l.type, 
	    pEventT->u.clientMessage.u.l.type);
    switch (pEvent->u.u.detail) {
       case 8:
          bcopy(pEvent->u.clientMessage.u.b.bytes,
		pEventT->u.clientMessage.u.b.bytes, 20);
	  break;
       case 16:
	  cpswaps(pEvent->u.clientMessage.u.s.shorts0,
	     pEventT->u.clientMessage.u.s.shorts0);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts1,
	     pEventT->u.clientMessage.u.s.shorts1);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts2,
	     pEventT->u.clientMessage.u.s.shorts2);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts3,
	     pEventT->u.clientMessage.u.s.shorts3);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts4,
	     pEventT->u.clientMessage.u.s.shorts4);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts5,
	     pEventT->u.clientMessage.u.s.shorts5);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts6,
	     pEventT->u.clientMessage.u.s.shorts6);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts7,
	     pEventT->u.clientMessage.u.s.shorts7);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts8,
	     pEventT->u.clientMessage.u.s.shorts8);
	  cpswaps(pEvent->u.clientMessage.u.s.shorts9,
	     pEventT->u.clientMessage.u.s.shorts9);
	  break;
       case 32:
	  cpswaps(pEvent->u.clientMessage.u.l.longs0,
	     pEventT->u.clientMessage.u.l.longs0);
	  cpswaps(pEvent->u.clientMessage.u.l.longs1,
	     pEventT->u.clientMessage.u.l.longs1);
	  cpswaps(pEvent->u.clientMessage.u.l.longs2,
	     pEventT->u.clientMessage.u.l.longs2);
	  cpswaps(pEvent->u.clientMessage.u.l.longs3,
	     pEventT->u.clientMessage.u.l.longs3);
	  cpswaps(pEvent->u.clientMessage.u.l.longs4,
	     pEventT->u.clientMessage.u.l.longs4);
	  break;
       }
    WriteToClient(pClient, size, pEventT);
    DEALLOCATE_LOCAL(pEventT);
}

void
SKeymapNotifyEvent(pClient, size, pEvent)
    ClientPtr	pClient;
    int		size;
    xEvent	*pEvent;
{
    WriteToClient(pClient, size, pEvent);
}

void
WriteSConnectionInfo(pClient, size, pInfo)
    ClientPtr		pClient;
    int			size;
    char 		*pInfo;
{
    int		i, j, k;
    ScreenPtr	pScreen;
    DepthPtr	pDepth;
    char	*pInfoT, *pInfoTBase;
    xConnSetup	*pConnSetup = (xConnSetup *)pInfo;

    pInfoT = pInfoTBase = (char *) ALLOCATE_LOCAL(size);

    SwapConnSetup(pInfo, pInfoT);
    pInfo += sizeof(xConnSetup);
    pInfoT += sizeof(xConnSetup);

    /* Copy the vendor string */
    i = (pConnSetup->nbytesVendor + 3) & ~3;
    bcopy(pInfo, pInfoT, i);
    pInfo += i;
    pInfoT += i;

    /* The Pixmap formats don't need to be swapped, just copied. */
    i = sizeof(xPixmapFormat) * screenInfo.numPixmapFormats;
    bcopy(pInfo, pInfoT, i);
    pInfo += i;
    pInfoT += i;

    for(i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = (ScreenPtr)&screenInfo.screen[i];
	SwapWinRoot(pInfo, pInfoT);
	pInfo += sizeof(xWindowRoot);
	pInfoT += sizeof(xWindowRoot);
	pDepth = pScreen->allowedDepths;
	for(j = 0; j < pScreen->numDepths; j++, pDepth++)
	{
            ((xDepth *)pInfoT)->depth = ((xDepth *)pInfo)->depth;
	    cpswaps(((xDepth *)pInfo)->nVisuals, ((xDepth *)pInfoT)->nVisuals);
	    pInfo += sizeof(xDepth);
	    pInfoT += sizeof(xDepth);
	    for(k = 0; k < pDepth->numVids; k++)
	    {
		SwapVisual(pInfo, pInfoT);
		pInfo += sizeof(xVisualType);
		pInfoT += sizeof(xVisualType);
	    }
	}
    }
    WriteToClient(pClient, size, pInfoTBase);
    DEALLOCATE_LOCAL(pInfoTBase);
    pClient->pSwapReplyFunc = (void (*) ()) NULL;
}

void
SwapConnSetup(pConnSetup, pConnSetupT)
    xConnSetup 	*pConnSetup, *pConnSetupT;
{
    cpswapl(pConnSetup->release, pConnSetupT->release);
    cpswapl(pConnSetup->ridBase, pConnSetupT->ridBase);
    cpswapl(pConnSetup->ridMask, pConnSetupT->ridMask);
    cpswapl(pConnSetup->motionBufferSize, pConnSetupT->motionBufferSize);
    cpswaps(pConnSetup->nbytesVendor, pConnSetupT->nbytesVendor);
    cpswaps(pConnSetup->maxRequestSize, pConnSetupT->maxRequestSize);
    pConnSetupT->minKeyCode = pConnSetup->minKeyCode;
    pConnSetupT->maxKeyCode = pConnSetup->maxKeyCode;
    pConnSetupT->numRoots = pConnSetup->numRoots;
    pConnSetupT->numFormats = pConnSetup->numFormats;
    pConnSetupT->imageByteOrder = pConnSetup->imageByteOrder;
    pConnSetupT->bitmapBitOrder = pConnSetup->bitmapBitOrder;
    pConnSetupT->bitmapScanlineUnit = pConnSetup->bitmapScanlineUnit;
    pConnSetupT->bitmapScanlinePad = pConnSetup->bitmapScanlinePad;
}

void
SwapWinRoot(pRoot, pRootT)
    xWindowRoot	*pRoot, *pRootT;
{
    cpswapl(pRoot->windowId, pRootT->windowId);
    cpswapl(pRoot->defaultColormap, pRootT->defaultColormap);
    cpswapl(pRoot->whitePixel, pRootT->whitePixel);
    cpswapl(pRoot->blackPixel, pRootT->blackPixel);
    cpswapl(pRoot->currentInputMask, pRootT->currentInputMask);
    cpswaps(pRoot->pixWidth, pRootT->pixWidth);
    cpswaps(pRoot->pixHeight, pRootT->pixHeight);
    cpswaps(pRoot->mmWidth, pRootT->mmWidth);
    cpswaps(pRoot->mmHeight, pRootT->mmHeight);
    cpswaps(pRoot->minInstalledMaps, pRootT->minInstalledMaps);
    cpswaps(pRoot->maxInstalledMaps, pRootT->maxInstalledMaps);
    cpswapl(pRoot->rootVisualID, pRootT->rootVisualID);
    pRootT->backingStore = pRoot->backingStore;
    pRootT->saveUnders = pRoot->saveUnders;
    pRootT->rootDepth = pRoot->rootDepth;
    pRootT->nDepths = pRoot->nDepths;
}

void
SwapVisual(pVis, pVisT)
    xVisualType 	*pVis, *pVisT;
{
    cpswapl(pVis->visualID, pVisT->visualID);
    pVisT->class = pVis->class;
    pVisT->bitsPerRGB = pVis->bitsPerRGB;
    cpswaps(pVis->colormapEntries, pVisT->colormapEntries);
    cpswapl(pVis->redMask, pVisT->redMask);
    cpswapl(pVis->greenMask, pVisT->greenMask);
    cpswapl(pVis->blueMask, pVisT->blueMask);
}

void
WriteSConnSetupPrefix(pClient, pcsp)
    ClientPtr		pClient;
    xConnSetupPrefix	*pcsp;
{
    xConnSetupPrefix	cspT;

    cspT.success = pcsp->success;
    cpswaps(pcsp->majorVersion, cspT.majorVersion);
    cpswaps(pcsp->minorVersion, cspT.minorVersion);
    cpswaps(pcsp->length, cspT.length);
    WriteToClient(pClient, sizeof(cspT), &cspT);
    pClient->pSwapReplyFunc = WriteSConnectionInfo;
}

