/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* RANDOM CRUFT! THIS HAS NO OFFICIAL X CONSORTIUM BLESSING */

/* $XConsortium: mitmisc.c,v 1.1 89/10/08 19:41:20 rws Exp $ */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#define _MITMISC_SERVER_
#include "mitmiscstr.h"

extern Bool permitOldBugs;

static unsigned char MITReqCode;

void
MITMiscExtensionInit()
{
    ExtensionEntry *extEntry, *AddExtension();
    static int ProcMITDispatch(), SProcMITDispatch();
    static void MITResetProc();

    if (extEntry = AddExtension(MITMISCNAME, 0, 0,
				 ProcMITDispatch, SProcMITDispatch,
				 MITResetProc, StandardMinorOpcode))
	MITReqCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
MITResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcMITSetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITSetBugModeReq);

    REQUEST_SIZE_MATCH(xMITSetBugModeReq);
    if ((stuff->onOff != xTrue) && (stuff->onOff != xFalse))
    {
	client->errorValue = stuff->onOff;
	return BadValue;
    }
    permitOldBugs = stuff->onOff;
    return(client->noClientException);
}

static int
ProcMITGetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITGetBugModeReq);
    xMITGetBugModeReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xMITGetBugModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.onOff = permitOldBugs;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xMITGetBugModeReply), (char *)&rep);
    return(client->noClientException);
}

int
ProcMITPushPixels (client)
    register ClientPtr client;
{
    register DrawablePtr    pDst;
    register DrawablePtr    pSrc;
    register GCPtr	    pGC;
    REQUEST(xCopyAreaReq);
    unsigned		    width, height;
    int			    dstx, dsty;

    REQUEST_SIZE_MATCH(xMITPushPixelsReq);

    VALIDATE_DRAWABLE_AND_GC(stuff->dstDrawable, pDst, pGC, client); 
    if (!(pSrc = LOOKUP_DRAWABLE(stuff->srcDrawable, client)) ||
	pSrc->type != DRAWABLE_PIXMAP ||
	pSrc->depth != 1)
    {
	client->errorValue = stuff->srcDrawable;
	return(BadPixmap);
    }
    if (pDst->pScreen != pSrc->pScreen)
    {
	client->errorValue = stuff->dstDrawable;
	return (BadMatch);
    }

    dstx = stuff->dstX;
    dsty = stuff->dstY;
    width = stuff->width;
    height = stuff->height;
    if (width > pSrc->width)
	width = pSrc->width;
    if (height > pSrc->height)
	height = pSrc->height;
    if (dstx + width > pDst->width)
    {
	if (dstx > pDst->width)
	    width = 0;
	else
	    width = pDst->width - dstx;
    }
    if (dsty + height > pDst->height)
    {
	if (dsty > pDst->height)
	    height = 0;
	else
	    height = pDst->height - dsty;
    }
    if (width && height)
    {
    	(*pGC->ops->PushPixels)(pSrc, pDst, pGC, 
				     width, height, 
				     dstx, dsty);
    }

    return(client->noClientException);
}

static int
ProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return ProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return ProcMITGetBugMode(client);
    case X_MITPushPixels:
	return ProcMITPushPixels(client);
    default:
	return BadRequest;
    }
}

static int
SProcMITSetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITSetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITSetBugMode(client);
}

static int
SProcMITGetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITGetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITGetBugMode(client);
}

static int
SProcMITPushPixels (client)
    register ClientPtr	client;
{
    register int    n;
    REQUEST (xMITPushPixelsReq);

    swaps(&stuff->length, n);
    swapl(&stuff->srcDrawable, n);
    swapl(&stuff->dstDrawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
}

static int
SProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return SProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return SProcMITGetBugMode(client);
    case X_MITPushPixels:
	return SProcMITPushPixels(client);
    default:
	return BadRequest;
    }
}
