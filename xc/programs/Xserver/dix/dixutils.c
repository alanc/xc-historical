/***********************************************************
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

******************************************************************/


/* $Header: dixutils.c,v 1.26 88/03/16 10:12:08 rws Exp $ */

#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "window.h"
#include "dixstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"


/*
 * CompareTimeStamps returns -1, 0, or +1 depending on if the first
 * argument is less than, equal to or greater than the second argument.
 */

int
CompareTimeStamps(a, b)
    TimeStamp a, b;
{
    if (a.months < b.months)
	return EARLIER;
    if (a.months > b.months)
	return LATER;
    if (a.milliseconds < b.milliseconds)
	return EARLIER;
    if (a.milliseconds > b.milliseconds)
	return LATER;
    return SAMETIME;
}

/*
 * convert client times to server TimeStamps
 */

#define HALFMONTH ((unsigned long) 1<<31)
TimeStamp
ClientTimeToServerTime(c)
     CARD32 c;
{
    TimeStamp ts;
    if (c == CurrentTime)
	return currentTime;
    ts.months = currentTime.months;
    ts.milliseconds = c;
    if (c > currentTime.milliseconds)
    {
	if (((unsigned long) c - currentTime.milliseconds) > HALFMONTH)
	    ts.months -= 1;
    }
    else if (c < currentTime.milliseconds)
    {
	if (((unsigned long)currentTime.milliseconds - c) > HALFMONTH)
	    ts.months -= 1L;
    }
    return ts;
}

WindowPtr
LookupWindow(rid, client)
    XID rid;
    ClientPtr client;
{
    client->errorValue = rid;
    if(rid == INVALID)
	return NULL;
    if (client->lastDrawableID == rid)
    {
        if (client->lastDrawable->type != DRAWABLE_PIXMAP)
            return ((WindowPtr) client->lastDrawable);
        return (WindowPtr) NULL;
    }
    return (WindowPtr)LookupID(rid, RT_WINDOW, RC_CORE);
}


pointer
LookupDrawable(rid, client)
    XID rid;
    ClientPtr client;
{
    register DrawablePtr pDraw;

    if(rid == INVALID)
	return (pointer) NULL;
    if (client->lastDrawableID == rid)
	return ((pointer) client->lastDrawable);
    pDraw = (DrawablePtr)LookupID(rid, RT_DRAWABLE, RC_CORE);
    if (pDraw && (pDraw->type != UNDRAWABLE_WINDOW))
        return (pointer)pDraw;		
    return (pointer)NULL;
}


int
AlterSaveSetForClient(client, pWin, mode)
    ClientPtr client;
    WindowPtr pWin;
    unsigned mode;
{
    int numnow;
    pointer *pTmp;
    int j;

    numnow = client->numSaved;
    j = 0;
    if (numnow)
    {
	pTmp = client->saveSet;
	while ((j < numnow) && (pTmp[j] != (pointer)pWin))
	    j++;
    }
    if (mode == SetModeInsert)
    {
	if (j < numnow)         /* duplicate */
	   return(Success);
	numnow++;
	client->saveSet = (pointer * )xrealloc(
		  client->saveSet, 
		  sizeof(pointer) * numnow);
       	client->numSaved = numnow;
	client->saveSet[numnow - 1] = (pointer)pWin;
	return(Success);
    }
    else if ((mode == SetModeDelete) && (j < numnow))
    {
	while (j < numnow-1)
	{
           pTmp[j] = pTmp[j+1];
	   j++;
	}
	numnow--;
        if (numnow)
    	    client->saveSet = (pointer * )xrealloc(
		      client->saveSet, 
		      sizeof(pointer) * numnow);
        else
        {
            xfree(client->saveSet);
	    client->saveSet = (pointer *)NULL;
	}
	client->numSaved = numnow;
	return(Success);
    }
    return(Success);
}


DeleteWindowFromAnySaveSet(pWin)
    WindowPtr pWin;
{
    register int i;
    register ClientPtr client;
    
    for (i = 0; i< currentMaxClients; i++)
    {    
	client = clients[i];
	if (client && client->numSaved)
	    AlterSaveSetForClient(client, pWin, SetModeDelete);
    }
}

/* No-op Don't Do Anything : sometimes we need to be able to call a procedure
 * that doesn't do anything.  For example, on screen with only static
 * colormaps, if someone calls install colormap, it's easier to have a dummy
 * procedure to call than to check if there's a procedure 
 */
void
NoopDDA()
{
}


/* called from the OS layer */
BlockHandler(pTimeout, pReadmask)
pointer	pTimeout;	/* DIX doesn't want to know how OS represents time */
pointer pReadmask;	/* nor how it represents the set of descriptors */
{
    register int i;
    for (i = 0; i < screenInfo.numScreens; i++)
	(* screenInfo.screen[i].BlockHandler)(i, 
				screenInfo.screen[i].blockData,
				pTimeout, pReadmask);
}


WakeupHandler(result, pReadmask)
unsigned long	result;	/* 32 bits of undefined result from the wait */
pointer pReadmask;	/* the resulting descriptor mask */
{
    register int i;
    for (i = 0; i < screenInfo.numScreens; i++)
	(* screenInfo.screen[i].WakeupHandler)(i, 
				screenInfo.screen[i].wakeupData,
				result, pReadmask);
}

