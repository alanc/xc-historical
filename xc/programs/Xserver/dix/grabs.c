/* $XConsortium: grabs.c,v 5.0 89/06/09 14:59:23 keith Exp $ */
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
WHETHER IN AN action OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#include "X.h"
#include "misc.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "windowstr.h"
#include "inputstr.h"
#include "cursorstr.h"

#define BITMASK(i) (((Mask)1) << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))

GrabPtr
CreateGrab(client, device, window, eventMask, ownerEvents, keyboardMode,
	   pointerMode, modifiers, keybut, confineTo, cursor)
    ClientPtr client;
    DeviceIntPtr device;
    WindowPtr window;
    Mask eventMask;
    Bool ownerEvents, keyboardMode, pointerMode;
    unsigned short modifiers;
    KeyCode keybut;	/* key or button */
    WindowPtr confineTo;
    CursorPtr cursor;
{
    GrabPtr grab;

    grab = (GrabPtr)xalloc(sizeof(GrabRec));
    if (!grab)
	return (GrabPtr)NULL;
    grab->client = client;
    grab->resource = FakeClientID(client->index);
    grab->device = device;
    grab->window = window;
    grab->eventMask = eventMask;
    grab->ownerEvents = ownerEvents;
    grab->keyboardMode = keyboardMode;
    grab->pointerMode = pointerMode;
    grab->modifiersDetail.exact = modifiers;
    grab->modifiersDetail.pMask = NULL;
    grab->detail.exact = keybut;
    grab->detail.pMask = NULL;
    grab->confineTo = confineTo;
    grab->cursor = cursor;
    if (cursor)
	cursor->refcnt++;
    return grab;

}

static void
FreeGrab(pGrab)
    GrabPtr pGrab;
{
    if (pGrab->modifiersDetail.pMask != NULL)
	xfree(pGrab->modifiersDetail.pMask);

    if (pGrab->detail.pMask != NULL)
	xfree(pGrab->detail.pMask);

    if (pGrab->cursor)
	FreeCursor(pGrab->cursor, (Cursor)0);

    xfree(pGrab);
}

/*ARGSUSED*/
static
DeletePassiveGrab(pGrab, id)
    GrabPtr pGrab;
    XID   id;
{
    register GrabPtr g, prev;

    /* it is OK if the grab isn't found */
    prev = 0;
    for (g = (wPassiveGrabs (pGrab->window)); g; g = g->next)
    {
	if (pGrab == g)
	{
	    if (prev)
		prev->next = g->next;
	    else
		if (!(pGrab->window->optional->passiveGrabs = g->next))
		    CheckWindowOptionalNeed (pGrab->window);
	    break;
	}
	prev = g;
    }
    FreeGrab(pGrab);
}

static Mask *
DeleteDetailFromMask(pDetailMask, detail)
    Mask *pDetailMask;
    unsigned short detail;
{
    register Mask *mask;
    register int i;

    mask = (Mask *)xalloc(sizeof(Mask) * MasksPerDetailMask);
    if (mask)
    {
	if (pDetailMask)
	    for (i = 0; i < MasksPerDetailMask; i++)
		mask[i]= pDetailMask[i];
	else
	    for (i = 0; i < MasksPerDetailMask; i++)
		mask[i]= ~0L;
	BITCLEAR(mask, detail);
    }
    return mask; 
}

static Bool
IsInGrabMask(firstDetail, secondDetail, exception)
    DetailRec firstDetail, secondDetail;
    unsigned short exception;
{
    if (firstDetail.exact == exception)
    {
	if (firstDetail.pMask == NULL)
	    return TRUE;
	
	/* (at present) never called with two non-null pMasks */
	if (secondDetail.exact == exception)
	    return FALSE;

 	if (GETBIT(firstDetail.pMask, secondDetail.exact))
	    return TRUE;
    }
    
    return FALSE;
}

static Bool 
IdenticalExactDetails(firstExact, secondExact, exception)
    unsigned short firstExact, secondExact, exception;
{
    if ((firstExact == exception) || (secondExact == exception))
	return FALSE;
   
    if (firstExact == secondExact)
	return TRUE;

    return FALSE;
}

static Bool 
DetailSupersedesSecond(firstDetail, secondDetail, exception)
    DetailRec firstDetail, secondDetail;
    unsigned short exception;
{
    if (IsInGrabMask(firstDetail, secondDetail, exception))
	return TRUE;

    if (IdenticalExactDetails(firstDetail.exact, secondDetail.exact,
			      exception))
	return TRUE;
  
    return FALSE;
}

static Bool
GrabSupersedesSecond(pFirstGrab, pSecondGrab)
    GrabPtr pFirstGrab, pSecondGrab;
{
    if (!DetailSupersedesSecond(pFirstGrab->modifiersDetail,
				pSecondGrab->modifiersDetail, 
				(unsigned short)AnyModifier))
	return FALSE;

    if (DetailSupersedesSecond(pFirstGrab->detail,
			       pSecondGrab->detail, (unsigned short)AnyKey))
	return TRUE;
 
    return FALSE;
}

Bool
GrabMatchesSecond(pFirstGrab, pSecondGrab)
    GrabPtr pFirstGrab, pSecondGrab;
{
    if (pFirstGrab->device != pSecondGrab->device)
	return FALSE;

    if (GrabSupersedesSecond(pFirstGrab, pSecondGrab))
	return TRUE;

    if (GrabSupersedesSecond(pSecondGrab, pFirstGrab))
	return TRUE;
 
    if (DetailSupersedesSecond(pSecondGrab->detail, pFirstGrab->detail,
			       (unsigned short)AnyKey) 
	&& 
	DetailSupersedesSecond(pFirstGrab->modifiersDetail,
			       pSecondGrab->modifiersDetail,
			       (unsigned short)AnyModifier))
	return TRUE;

    if (DetailSupersedesSecond(pFirstGrab->detail, pSecondGrab->detail,
			       (unsigned short)AnyKey)
	&& 
	DetailSupersedesSecond(pSecondGrab->modifiersDetail,
			       pFirstGrab->modifiersDetail,
			       (unsigned short)AnyModifier))
	return TRUE;

    return FALSE;
}

int
AddPassiveGrabToList(pGrab)
    GrabPtr pGrab;
{
    GrabPtr grab;

    for (grab = wPassiveGrabs(pGrab->window); grab; grab = grab->next)
    {
	if (GrabMatchesSecond(pGrab, grab))
	{
	    if (pGrab->client != grab->client)
	    {
		FreeGrab(pGrab);
		return BadAccess;
	    }
	}
    }

    if (!pGrab->window->optional && !MakeWindowOptional (pGrab->window))
    {
	FreeGrab(pGrab);
	return BadAlloc;
    }
    pGrab->next = pGrab->window->optional->passiveGrabs;
    pGrab->window->optional->passiveGrabs = pGrab;
    if (AddResource(pGrab->resource, RT_FAKE, (pointer)pGrab,
		    DeletePassiveGrab, RC_CORE))
	return Success;
    return BadAlloc;
}

/* the following is kinda complicated, because we need to be able to back out
 * if any allocation fails
 */

Bool
DeletePassiveGrabFromList(pMinuendGrab)
    GrabPtr pMinuendGrab;
{
    register GrabPtr grab;
    GrabPtr *deletes, *adds;
    Mask ***updates, **details;
    int i, ndels, nadds, nups;
    Bool ok;

#define UPDATE(mask,exact) \
	if (!(details[nups] = DeleteDetailFromMask(mask, exact))) \
	  ok = FALSE; \
	else \
	  updates[nups++] = &(mask)

    i = 0;
    for (grab = wPassiveGrabs(pMinuendGrab->window); grab; grab = grab->next)
	i++;
    deletes = (GrabPtr *)ALLOCATE_LOCAL(i * sizeof(GrabPtr));
    adds = (GrabPtr *)ALLOCATE_LOCAL(i * sizeof(GrabPtr));
    updates = (Mask ***)ALLOCATE_LOCAL(i * sizeof(Mask **));
    details = (Mask **)ALLOCATE_LOCAL(i * sizeof(Mask *));
    if (!deletes || !adds || !updates || !details)
    {
	if (details) DEALLOCATE_LOCAL(details);
	if (updates) DEALLOCATE_LOCAL(updates);
	if (adds) DEALLOCATE_LOCAL(adds);
	if (deletes) DEALLOCATE_LOCAL(deletes);
	return FALSE;
    }
    ndels = nadds = nups = 0;
    ok = TRUE;
    for (grab = wPassiveGrabs(pMinuendGrab->window);
	 grab && ok;
	 grab = grab->next)
    {
	if ((grab->client != pMinuendGrab->client) ||
	    !GrabMatchesSecond(grab, pMinuendGrab))
	    continue;
	if (GrabSupersedesSecond(pMinuendGrab, grab))
	{
	    deletes[ndels++] = grab;
	}
	else if ((grab->detail.exact == AnyKey)
		 && (grab->modifiersDetail.exact != AnyModifier))
	{
	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);
	}
	else if ((grab->modifiersDetail.exact == AnyModifier) 
		 && (grab->detail.exact != AnyKey))
	{
	    UPDATE(grab->modifiersDetail.pMask,
		   pMinuendGrab->modifiersDetail.exact);
	}
	else if ((pMinuendGrab->detail.exact != AnyKey)
		 && (pMinuendGrab->modifiersDetail.exact != AnyModifier))
	{
	    GrabPtr pNewGrab;

	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);

	    pNewGrab = CreateGrab(grab->client, grab->device,
				  grab->window,
				  grab->eventMask, grab->ownerEvents,
				  grab->keyboardMode, grab->pointerMode,
				  AnyModifier, pMinuendGrab->detail.exact,
				  grab->confineTo, grab->cursor);
	    if (!pNewGrab)
		ok = FALSE;
	    else if (!(pNewGrab->modifiersDetail.pMask =
		       DeleteDetailFromMask(grab->modifiersDetail.pMask,
					 pMinuendGrab->modifiersDetail.exact))
		     ||
		     (!pNewGrab->window->optional &&
		      !MakeWindowOptional(pNewGrab->window)))
	    {
		FreeGrab(pNewGrab);
		ok = FALSE;
	    }
	    else if (!AddResource(pNewGrab->resource, RT_FAKE,
				  (pointer)pNewGrab, DeletePassiveGrab,
				  RC_CORE))
		ok = FALSE;
	    else
		adds[nadds++] = pNewGrab;
	}   
	else if (pMinuendGrab->detail.exact == AnyKey)
	{
	    UPDATE(grab->modifiersDetail.pMask,
		   pMinuendGrab->modifiersDetail.exact);
	}
	else
	{
	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);
	}
    }

    if (!ok)
    {
	for (i = 0; i < nadds; i++)
	    FreeResource(adds[i]->resource, RC_NONE);
	for (i = 0; i < nups; i++)
	    xfree(details[i]);
    }
    else
    {
	for (i = 0; i < ndels; i++)
	    FreeResource(deletes[i]->resource, RC_NONE);
	for (i = 0; i < nadds; i++)
	{
	    grab = adds[i];
	    grab->next = grab->window->optional->passiveGrabs;
	    grab->window->optional->passiveGrabs = grab;
	}
	for (i = 0; i < nups; i++)
	{
	    xfree(*updates[i]);
	    *updates[i] = details[i];
	}
    }
    DEALLOCATE_LOCAL(details);
    DEALLOCATE_LOCAL(updates);
    DEALLOCATE_LOCAL(adds);
    DEALLOCATE_LOCAL(deletes);
    return ok;

#undef UPDATE
}
