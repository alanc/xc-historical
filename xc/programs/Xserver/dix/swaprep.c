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

/* $Header: swaprep.c,v 1.23 87/08/14 22:34:48 toddb Locked $ */

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "scrnintstr.h"

void SwapVisual(), SwapConnSetup(), SwapWinRoot(), WriteToClient();

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
    /* brackets are mandatory here, because "swapl" macro expands
       to several statements */
    {   
	swapl(&pbuf[i], n);
    }
    WriteToClient(pClient, size << 2, (char *) pbuf);
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
    /* brackets are mandatory here, because "swaps" macro expands
       to several statements */
    {
	swaps(&pbuf[i], n);
    }
    WriteToClient(pClient, size << 1, (char *) pbuf);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
}

void
SQueryTreeReply(pClient, size, pRep)
    xQueryTreeReply	*pRep;
    ClientPtr		pClient;
    int			size;
{
    int n;

    swaps(&pRep->sequenceNumber, n);
    pClient->pSwapReplyFunc = Swap32Write;
    swapl(&pRep->length, n);
    swapl(&pRep->root, n);
    swapl(&pRep->parent, n);
    swaps(&pRep->nChildren, n);
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
    switch(pRep->format)
    {
	case 8:
            pClient->pSwapReplyFunc = WriteToClient;
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);

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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
}

void
SQueryFontReply(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xQueryFontReply	*pRep;
{
    SwapFont(pRep);
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
}

void
SListFontsWithInfoReply(pClient, size, pRep)
    ClientPtr			pClient;
    int				size;
    xListFontsWithInfoReply	*pRep;
{
    int n;

    SwapFont((xQueryFontReply *)pRep);
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) prgb);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
}

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
void
SLHostsExtend(pClient, size, pRep)
    ClientPtr		pClient;
    int			size;
    xListHostsReply	*pRep;
{
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
    WriteToClient(pClient, size, (char *) pRep);
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
    WriteToClient(pClient, size, (char *) pRep);
}

void
SKeyButtonPtrEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.keyButtonPointer.time,
        to->u.keyButtonPointer.time);
    cpswapl(from->u.keyButtonPointer.root,
        to->u.keyButtonPointer.root);
    cpswapl(from->u.keyButtonPointer.event,
        to->u.keyButtonPointer.event);
    cpswapl(from->u.keyButtonPointer.child,
        to->u.keyButtonPointer.child);
    cpswaps(from->u.keyButtonPointer.rootX,
        to->u.keyButtonPointer.rootX);
    cpswaps(from->u.keyButtonPointer.rootY,
	to->u.keyButtonPointer.rootY);
    cpswaps(from->u.keyButtonPointer.eventX,
        to->u.keyButtonPointer.eventX);
    cpswaps(from->u.keyButtonPointer.eventY,
        to->u.keyButtonPointer.eventY);
    cpswaps(from->u.keyButtonPointer.state,
        to->u.keyButtonPointer.state);
    to->u.keyButtonPointer.sameScreen = 
	from->u.keyButtonPointer.sameScreen;
}

void
SEnterLeaveEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.enterLeave.time, to->u.enterLeave.time);
    cpswapl(from->u.enterLeave.root, to->u.enterLeave.root);
    cpswapl(from->u.enterLeave.event, to->u.enterLeave.event);
    cpswapl(from->u.enterLeave.child, to->u.enterLeave.child);
    cpswaps(from->u.enterLeave.rootX, to->u.enterLeave.rootX);
    cpswaps(from->u.enterLeave.rootY, to->u.enterLeave.rootY);
    cpswaps(from->u.enterLeave.eventX, to->u.enterLeave.eventX);
    cpswaps(from->u.enterLeave.eventY, to->u.enterLeave.eventY);
    cpswaps(from->u.enterLeave.state, to->u.enterLeave.state);
    to->u.enterLeave.mode = from->u.enterLeave.mode;
    to->u.enterLeave.flags = from->u.enterLeave.flags;
}

void
SFocusEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.focus.window, to->u.focus.window);
    to->u.focus.mode = from->u.focus.mode;
}

void
SExposeEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.expose.window, to->u.expose.window);
    cpswaps(from->u.expose.x, to->u.expose.x);
    cpswaps(from->u.expose.y, to->u.expose.y);
    cpswaps(from->u.expose.width, to->u.expose.width);
    cpswaps(from->u.expose.height, to->u.expose.height);
    cpswaps(from->u.expose.count, to->u.expose.count);
}

void
SGraphicsExposureEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.graphicsExposure.drawable,
        to->u.graphicsExposure.drawable);
    cpswaps(from->u.graphicsExposure.x, 
	to->u.graphicsExposure.x);
    cpswaps(from->u.graphicsExposure.y, 
	to->u.graphicsExposure.y);
    cpswaps(from->u.graphicsExposure.width, 
	to->u.graphicsExposure.width);
    cpswaps(from->u.graphicsExposure.height, 
	to->u.graphicsExposure.height);
    cpswaps(from->u.graphicsExposure.minorEvent,
        to->u.graphicsExposure.minorEvent);
    cpswaps(from->u.graphicsExposure.count,
	to->u.graphicsExposure.count);
    to->u.graphicsExposure.majorEvent = 
    	from->u.graphicsExposure.majorEvent;
}

void
SNoExposureEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.noExposure.drawable, to->u.noExposure.drawable);
    cpswaps(from->u.noExposure.minorEvent, to->u.noExposure.minorEvent);
    to->u.noExposure.majorEvent = from->u.noExposure.majorEvent;
}

void
SVisibilityEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.visibility.window, to->u.visibility.window);
    to->u.visibility.state = from->u.visibility.state;
}

void
SCreateNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.createNotify.window, to->u.createNotify.window);
    cpswapl(from->u.createNotify.parent, to->u.createNotify.parent);
    cpswaps(from->u.createNotify.x, to->u.createNotify.x);
    cpswaps(from->u.createNotify.y, to->u.createNotify.y);
    cpswaps(from->u.createNotify.width, to->u.createNotify.width);
    cpswaps(from->u.createNotify.height, to->u.createNotify.height);
    cpswaps(from->u.createNotify.borderWidth,
        to->u.createNotify.borderWidth);
    to->u.createNotify.override = from->u.createNotify.override;
}

void
SDestroyNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.destroyNotify.event, to->u.destroyNotify.event);
    cpswapl(from->u.destroyNotify.window, to->u.destroyNotify.window);
}

void
SUnmapNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.unmapNotify.event, to->u.unmapNotify.event);
    cpswapl(from->u.unmapNotify.window, to->u.unmapNotify.window);
    to->u.unmapNotify.fromConfigure = from->u.unmapNotify.fromConfigure;
}

void
SMapNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.mapNotify.event, to->u.mapNotify.event);
    cpswapl(from->u.mapNotify.window, to->u.mapNotify.window);
    to->u.mapNotify.override = from->u.mapNotify.override;
}

void
SMapRequestEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.mapRequest.parent, to->u.mapRequest.parent);
    cpswapl(from->u.mapRequest.window, to->u.mapRequest.window);
}

void
SReparentEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.reparent.event, to->u.reparent.event);
    cpswapl(from->u.reparent.window, to->u.reparent.window);
    cpswapl(from->u.reparent.parent, to->u.reparent.parent);
    cpswaps(from->u.reparent.x, to->u.reparent.x);
    cpswaps(from->u.reparent.y, to->u.reparent.y);
    to->u.reparent.override = from->u.reparent.override;
}

void
SConfigureNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.configureNotify.event,
        to->u.configureNotify.event);
    cpswapl(from->u.configureNotify.window,
        to->u.configureNotify.window);
    cpswapl(from->u.configureNotify.aboveSibling,
        to->u.configureNotify.aboveSibling);
    cpswaps(from->u.configureNotify.x, to->u.configureNotify.x);
    cpswaps(from->u.configureNotify.y, to->u.configureNotify.y);
    cpswaps(from->u.configureNotify.width, to->u.configureNotify.width);
    cpswaps(from->u.configureNotify.height,
        to->u.configureNotify.height);
    cpswaps(from->u.configureNotify.borderWidth,
        to->u.configureNotify.borderWidth);
    to->u.configureNotify.override = from->u.configureNotify.override;
}

void
SConfigureRequestEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;  /* actually stack-mode */
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.configureRequest.parent,
        to->u.configureRequest.parent);
    cpswapl(from->u.configureRequest.window,
        to->u.configureRequest.window);
    cpswapl(from->u.configureRequest.sibling,
        to->u.configureRequest.sibling);
    cpswaps(from->u.configureRequest.x, to->u.configureRequest.x);
    cpswaps(from->u.configureRequest.y, to->u.configureRequest.y);
    cpswaps(from->u.configureRequest.width,
        to->u.configureRequest.width);
    cpswaps(from->u.configureRequest.height,
        to->u.configureRequest.height);
    cpswaps(from->u.configureRequest.borderWidth,
        to->u.configureRequest.borderWidth);
    cpswaps(from->u.configureRequest.valueMask,
        to->u.configureRequest.valueMask);
}


void
SGravityEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.gravity.event, to->u.gravity.event);
    cpswapl(from->u.gravity.window, to->u.gravity.window);
    cpswaps(from->u.gravity.x, to->u.gravity.x);
    cpswaps(from->u.gravity.y, to->u.gravity.y);
}

void
SResizeRequestEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.resizeRequest.window, to->u.resizeRequest.window);
    cpswaps(from->u.resizeRequest.width, to->u.resizeRequest.width);
    cpswaps(from->u.resizeRequest.height, to->u.resizeRequest.height);
}

void
SCirculateEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.circulate.event, to->u.circulate.event);
    cpswapl(from->u.circulate.window, to->u.circulate.window);
    cpswapl(from->u.circulate.parent, to->u.circulate.parent);
    to->u.circulate.place = from->u.circulate.place;
}

void
SPropertyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.property.window, to->u.property.window);
    cpswapl(from->u.property.atom, to->u.property.atom);
    cpswapl(from->u.property.time, to->u.property.time);
    to->u.property.state = from->u.property.state;
}

void
SSelectionClearEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.selectionClear.time, to->u.selectionClear.time);
    cpswapl(from->u.selectionClear.window, to->u.selectionClear.window);
    cpswapl(from->u.selectionClear.atom, to->u.selectionClear.atom);
}

void
SSelectionRequestEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.selectionRequest.time, to->u.selectionRequest.time);
    cpswapl(from->u.selectionRequest.owner,
        to->u.selectionRequest.owner);
    cpswapl(from->u.selectionRequest.requestor,
	to->u.selectionRequest.requestor);
    cpswapl(from->u.selectionRequest.selection,
	to->u.selectionRequest.selection);
    cpswapl(from->u.selectionRequest.target,
        to->u.selectionRequest.target);
    cpswapl(from->u.selectionRequest.property,
	to->u.selectionRequest.property);
}

void
SSelectionNotifyEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.selectionNotify.time, to->u.selectionNotify.time);
    cpswapl(from->u.selectionNotify.requestor,
	to->u.selectionNotify.requestor);
    cpswapl(from->u.selectionNotify.selection,
	to->u.selectionNotify.selection);
    cpswapl(from->u.selectionNotify.target,
	to->u.selectionNotify.target);
    cpswapl(from->u.selectionNotify.property,
        to->u.selectionNotify.property);
}

void
SColormapEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.colormap.window, to->u.colormap.window);
    cpswapl(from->u.colormap.colormap, to->u.colormap.colormap);
    to->u.colormap.new = from->u.colormap.new;
    to->u.colormap.state = from->u.colormap.state;
}

void
SMappingEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    to->u.mappingNotify.request = from->u.mappingNotify.request;
    to->u.mappingNotify.firstKeyCode =
	from->u.mappingNotify.firstKeyCode;
    to->u.mappingNotify.count = from->u.mappingNotify.count;
}

void
SClientMessageEvent(from, to)
    xEvent	*from, *to;
{
    to->u.u.type = from->u.u.type;
    to->u.u.detail = from->u.u.detail;  /* actually format */
    cpswaps(from->u.u.sequenceNumber, to->u.u.sequenceNumber);
    cpswapl(from->u.clientMessage.window, to->u.clientMessage.window);
    cpswapl(from->u.clientMessage.u.l.type, 
	    to->u.clientMessage.u.l.type);
    switch (from->u.u.detail) {
       case 8:
          bcopy(from->u.clientMessage.u.b.bytes,
		to->u.clientMessage.u.b.bytes, 20);
	  break;
       case 16:
	  cpswaps(from->u.clientMessage.u.s.shorts0,
	     to->u.clientMessage.u.s.shorts0);
	  cpswaps(from->u.clientMessage.u.s.shorts1,
	     to->u.clientMessage.u.s.shorts1);
	  cpswaps(from->u.clientMessage.u.s.shorts2,
	     to->u.clientMessage.u.s.shorts2);
	  cpswaps(from->u.clientMessage.u.s.shorts3,
	     to->u.clientMessage.u.s.shorts3);
	  cpswaps(from->u.clientMessage.u.s.shorts4,
	     to->u.clientMessage.u.s.shorts4);
	  cpswaps(from->u.clientMessage.u.s.shorts5,
	     to->u.clientMessage.u.s.shorts5);
	  cpswaps(from->u.clientMessage.u.s.shorts6,
	     to->u.clientMessage.u.s.shorts6);
	  cpswaps(from->u.clientMessage.u.s.shorts7,
	     to->u.clientMessage.u.s.shorts7);
	  cpswaps(from->u.clientMessage.u.s.shorts8,
	     to->u.clientMessage.u.s.shorts8);
	  cpswaps(from->u.clientMessage.u.s.shorts9,
	     to->u.clientMessage.u.s.shorts9);
	  break;
       case 32:
	  cpswaps(from->u.clientMessage.u.l.longs0,
	     to->u.clientMessage.u.l.longs0);
	  cpswaps(from->u.clientMessage.u.l.longs1,
	     to->u.clientMessage.u.l.longs1);
	  cpswaps(from->u.clientMessage.u.l.longs2,
	     to->u.clientMessage.u.l.longs2);
	  cpswaps(from->u.clientMessage.u.l.longs3,
	     to->u.clientMessage.u.l.longs3);
	  cpswaps(from->u.clientMessage.u.l.longs4,
	     to->u.clientMessage.u.l.longs4);
	  break;
       }
}

void
SKeymapNotifyEvent(from, to)
    xEvent	*from, *to;
{
    /* Keymap notify events are special; they have no
       sequence number field, and contain entirely 8-bit data */
    *to = *from;
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
    WriteToClient(pClient, size, (char *) pInfoTBase);
    DEALLOCATE_LOCAL(pInfoTBase);
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
    WriteToClient(pClient, sizeof(cspT), (char *) &cspT);
    pClient->pSwapReplyFunc = WriteSConnectionInfo;
}

