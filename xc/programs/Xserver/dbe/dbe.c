/* $XConsortium: dbe.c,v 1.2 95/06/07 20:19:57 dpw Exp $ */
/******************************************************************************
 * 
 * Copyright (c) 1994, 1995  Hewlett-Packard Company
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Hewlett-Packard
 * Company shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Hewlett-Packard Company.
 * 
 * $Header: /x/programs/Xserver/dbe/RCS/dbe.c,v 1.2 95/06/07 20:19:57 dpw Exp $
 *
 *     DIX DBE code
 *
 *****************************************************************************/


/* INCLUDES */

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "extnsionst.h"
#include "gcstruct.h"
#include "dixstruct.h"
#define NEED_DBE_PROTOCOL
#include "dbestruct.h"
#include "midbe.h"


/* DEFINES */


/* TYPEDEFS */


/* GLOBALS */

#ifdef DBE_SWAP_SCREEN_BUFFERS
/* This is a function pointer to a routine that can be used to swap buffers
 * across multiple screens.  To set it, a driver needs to call
 * DbeRegisterSwapFunction().  ProcDbeSwapBuffers() will call the registered
 * routine instead if the pointer is non-NULL.
 */
static int (*swapScreenBuffers)() = NULL; /* client, nStuff, pWins, actions */
#endif /* DBE_SWAP_SCREEN_BUFFERS */

/* Per-screen initialization functions [init'ed by DbeRegisterFunction()] */
static Bool	(* DbeInitFunct[MAXSCREENS])();	/* pScreen, pDbeScreenPriv */

/* These are static globals copied to DBE's screen private for use by DDX */
static int	dbeScreenPrivIndex;
static int	dbeWindowPrivIndex;

/* These are static globals copied to DBE's screen private for use by DDX */
static RESTYPE	dbeDrawableResType;
static RESTYPE	dbeWindowPrivResType;

/* This global is used by DbeAllocWinPrivPrivIndex() */
static int	winPrivPrivCount = 0;

/* This global is used by DbeAllocWinPrivPrivIndex() */
static int      scrnPrivPrivCount = 0;

/* Used to generate DBE's BadBuffer error. */
static int	dbeErrorBase;

/* Used by DbeRegisterFunction() to initialize the initialization function
 * table only once per server lifetime.
 */
static Bool	firstRegistrationPass = TRUE;


/* EXTERNS */

/* FORWARDS */

static DbeWindowPrivPtr DbeAllocWinPriv(
    ScreenPtr   pScreen);

static DbeScreenPrivPtr DbeAllocScrnPriv(
    DbeScreenPrivPtr	pOldDbeScreenPriv);

static Bool DbeAllocScrnPrivPriv(
    register ScreenPtr	pScreen,
    int			index,
    unsigned int	amount);

static int DbeAllocScrnPrivPrivIndex();

static Bool DbeAllocWinPrivPriv(
    register ScreenPtr	pScreen,
    int			index,
    unsigned int	amount);

static int DbeAllocWinPrivPrivIndex();

static Bool DbeDestroyWindow(
    WindowPtr	pWin);

static int DbeDrawableDelete(
    pointer	pDrawable,
    XID		id);

void DbeExtensionInit();

static DbeScreenPrivPtr DbeFallbackAllocScrnPriv(
    DbeScreenPrivPtr	pDbeScreenPriv);

static DbeWindowPrivPtr DbeFallbackAllocWinPriv(
    ScreenPtr   pScreen);

void DbeRegisterFunction(
    ScreenPtr	pScreen,
    Bool	(* funct)(ScreenPtr, DbeScreenPrivPtr));

#ifdef DBE_SWAP_SCREEN_BUFFERS
void DbeRegisterSwapFunction(
    int (*funct)());
#endif /* DBE_SWAP_SCREEN_BUFFERS */

static void DbeResetProc(
    ExtensionEntry	*extEntry);

static Bool DbeSetupBackgroundPainter(
    WindowPtr	pWin,
    GCPtr	pGC);

static void DbeStubScreen(
    DbeScreenPrivPtr	pDbeScreenPriv,
    int			*nStubbedScreens);

static int DbeWindowPrivDelete(
    pointer	pDbeWindowPriv,
    XID		id);

static Bool NoopDDABoolFalse();

static int NoopDDAInt0();

static int ProcDbeAllocateBackBufferName(
    ClientPtr	client);

static int ProcDbeBeginIdiom(
    ClientPtr	client);

static int ProcDbeDeallocateBackBufferName(
    ClientPtr	client);

static int ProcDbeDispatch(
    ClientPtr	client);

static int ProcDbeEndIdiom(
    ClientPtr	client);

static int ProcDbeGetBackBufferAttributes(
    ClientPtr	client);

static int ProcDbeGetVersion(
    ClientPtr	client);

static int ProcDbeGetVisualInfo(
    ClientPtr	client);

static int ProcDbeSwapBuffers(
    ClientPtr	client);

static int SProcDbeAllocateBackBufferName(
    ClientPtr	client);

static int SProcDbeBeginIdiom(
    ClientPtr	client);

static int SProcDbeDeallocateBackBufferName(
    ClientPtr	client);

static int SProcDbeDispatch(
    ClientPtr	client);

static int SProcDbeEndIdiom(
    ClientPtr	client);

static int SProcDbeGetBackBufferAttributes(
    ClientPtr	client);

static int SProcDbeGetVersion(
    ClientPtr	client);

static int SProcDbeGetVisualInfo(
    ClientPtr	client);

static int SProcDbeSwapBuffers(
    ClientPtr	client);


#ifdef DBE_SWAP_SCREEN_BUFFERS
/******************************************************************************
 *
 * DBE DIX Procedure: DbeRegisterSwapFunction
 *
 * Description:
 *
 *     This function registers the DBE init function for the specified screen.
 *
 *****************************************************************************/

void
DbeRegisterSwapFunction(
    int (*funct)())
{
    swapScreenBuffers = funct;
      
} /* DbeRegisterSwapFunction() */

#endif /* DBE_SWAP_SCREEN_BUFFERS */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeRegisterFunction
 *
 * Description:
 *
 *     This function registers the DBE init function for the specified screen.
 *
 *****************************************************************************/

void
DbeRegisterFunction(
    ScreenPtr	pScreen,
    Bool	(*funct)(ScreenPtr, DbeScreenPrivPtr))
{
    int	i;

    /* Initialize the initialization function table if it has not been
     * already.
     */
    if (firstRegistrationPass)
    {
        for (i = 0; i < MAXSCREENS; i++)
        {
            DbeInitFunct[i] = NULL;
        }

        firstRegistrationPass = FALSE;
    }

    DbeInitFunct[pScreen->myNum] = funct;

} /* DbeRegisterFunction() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeExtensionInit
 *
 * Description:
 *
 *     Called from InitExtensions in main()
 *
 *****************************************************************************/

void
DbeExtensionInit()
{
    ExtensionEntry	*extEntry;
    register int	i, j;
    ScreenPtr		pScreen;
    DbeScreenPrivPtr	pDbeScreenPriv;
    int			nStubbedScreens = 0;
    Bool		ddxInitSuccess;


    /* Allocate private pointers in windows and screens. */

    if ((dbeScreenPrivIndex = AllocateScreenPrivateIndex()) < 0)
    {
	return;
    }

    if ((dbeWindowPrivIndex = AllocateWindowPrivateIndex()) < 0)
    {
	return;
    }

    /* Initialize the priv priv counts between server generations. */
    winPrivPrivCount  = 0;
    scrnPrivPrivCount = 0;

    /* Create the resource types. */
    dbeDrawableResType =
        CreateNewResourceType(DbeDrawableDelete) | RC_CACHED | RC_DRAWABLE;
    dbeWindowPrivResType =
        CreateNewResourceType(DbeWindowPrivDelete);

    for (i = 0; i < screenInfo.numScreens; i++)
    {
        /* For each screen, set up DBE screen privates and init DIX and DDX
         * interface.
         */

	pScreen = screenInfo.screens[i];

	if (!AllocateWindowPrivate(pScreen, dbeWindowPrivIndex, 0) ||
	    !(pDbeScreenPriv =
             (DbeScreenPrivPtr)Xcalloc(sizeof(DbeScreenPrivRec))))
	{
            /* If we can not alloc a window or screen private,
             * then free any privates that we already alloc'ed and return
             */

	    for (j = 0; j < i; j++)
	    {
	      xfree(screenInfo.screens[j]->devPrivates[dbeScreenPrivIndex].ptr);
              screenInfo.screens[j]->devPrivates[dbeScreenPrivIndex].ptr = NULL;
	    }
	    return;
	}

	pScreen->devPrivates[dbeScreenPrivIndex].ptr = (pointer)pDbeScreenPriv;

        /* Store the DBE priv priv size info for later use when allocating
         * priv privs at the driver level.
         */
        pDbeScreenPriv->winPrivPrivLen   = 0;
        pDbeScreenPriv->winPrivPrivSizes = (unsigned *)NULL;
        pDbeScreenPriv->totalWinPrivSize = sizeof(DbeWindowPrivRec);

        /* Store the DBE screen priv priv size info for later use when
         * allocating screen priv privs at the driver level.
         */
        pDbeScreenPriv->scrnPrivPrivLen   = 0;
        pDbeScreenPriv->scrnPrivPrivSizes = (unsigned *)NULL;
        pDbeScreenPriv->totalScrnPrivSize = sizeof(DbeScreenPrivRec);

        /* Copy the resource types */
        pDbeScreenPriv->dbeDrawableResType   = dbeDrawableResType;
        pDbeScreenPriv->dbeWindowPrivResType = dbeWindowPrivResType;

        /* Copy the private indices */
        pDbeScreenPriv->dbeScreenPrivIndex = dbeScreenPrivIndex;
        pDbeScreenPriv->dbeWindowPrivIndex = dbeWindowPrivIndex;

        if(DbeInitFunct[i])
        {
            /* This screen supports DBE. */

            /* Setup DIX. */
            pDbeScreenPriv->SetupBackgroundPainter = DbeSetupBackgroundPainter; 
            pDbeScreenPriv->AllocWinPriv           = DbeAllocWinPriv;
            pDbeScreenPriv->AllocWinPrivPrivIndex  = DbeAllocWinPrivPrivIndex;
            pDbeScreenPriv->AllocWinPrivPriv       = DbeAllocWinPrivPriv;
            pDbeScreenPriv->AllocScrnPriv          = DbeAllocScrnPriv;
            pDbeScreenPriv->AllocScrnPrivPrivIndex = DbeAllocScrnPrivPrivIndex;
            pDbeScreenPriv->AllocScrnPrivPriv      = DbeAllocScrnPrivPriv;

            /* Setup DDX. */
            ddxInitSuccess = (*DbeInitFunct[i])(pScreen, pDbeScreenPriv);

            /* DDX DBE initialization may have the side affect of
             * reallocating pDbeScreenPriv, so we need to update it.
             */
            pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

            if (ddxInitSuccess)
            {
                /* Wrap DestroyWindow.  The DDX initialization function
                 * already wrapped PositionWindow for us.
                 */

                pDbeScreenPriv->DestroyWindow = pScreen->DestroyWindow;
                pScreen->DestroyWindow        = DbeDestroyWindow;
            }
            else
            {
                /* DDX initialization failed.  Stub the screen. */
                DbeStubScreen(pDbeScreenPriv, &nStubbedScreens);
            }
        }
        else
        {
            /* This screen does not support DBE. */

#ifndef DISABLE_MI_DBE_BY_DEFAULT
            /* Setup DIX. */
            pDbeScreenPriv->SetupBackgroundPainter = DbeSetupBackgroundPainter; 
            pDbeScreenPriv->AllocWinPriv           = DbeAllocWinPriv;
            pDbeScreenPriv->AllocWinPrivPrivIndex  = DbeAllocWinPrivPrivIndex;
            pDbeScreenPriv->AllocWinPrivPriv       = DbeAllocWinPrivPriv;
            pDbeScreenPriv->AllocScrnPriv          = DbeAllocScrnPriv;
            pDbeScreenPriv->AllocScrnPrivPrivIndex = DbeAllocScrnPrivPrivIndex;
            pDbeScreenPriv->AllocScrnPrivPriv      = DbeAllocScrnPrivPriv;

            /* Setup DDX. */
            ddxInitSuccess = miDbeInit(pScreen, pDbeScreenPriv);

            /* DDX DBE initialization may have the side affect of
             * reallocating pDbeScreenPriv, so we need to update it.
             */
            pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

            if (ddxInitSuccess)
            {
                /* Wrap DestroyWindow.  The DDX initialization function
                 * already wrapped PositionWindow for us.
                 */

                pDbeScreenPriv->DestroyWindow = pScreen->DestroyWindow;
                pScreen->DestroyWindow        = DbeDestroyWindow;
            }
            else
            {
                /* DDX initialization failed.  Stub the screen. */
                DbeStubScreen(pDbeScreenPriv, &nStubbedScreens);
            }
#else
            DbeStubScreen(pDbeScreenPriv, &nStubbedScreens);
#endif

        } /* else -- this screen does not support DBE. */

    } /* for (i = 0; i < screenInfo.numScreens; i++) */


    if (nStubbedScreens == screenInfo.numScreens)
    {
	/* All screens stubbed.  Clean up and return. */

        for (i = 0; i < screenInfo.numScreens; i++)
        {
             xfree(screenInfo.screens[i]->devPrivates[dbeScreenPrivIndex].ptr);
             pScreen->devPrivates[dbeScreenPrivIndex].ptr = NULL;
        }
        return;
    }


    /* Now add the extension. */
    extEntry = AddExtension(DBE_PROTOCOL_NAME, DbeNumberEvents, 
                            DbeNumberErrors, ProcDbeDispatch, SProcDbeDispatch,
                            DbeResetProc, StandardMinorOpcode);

    dbeErrorBase = extEntry->errorBase;

} /* DbeExtensionInit() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeStubScreen
 *
 * Description:
 *
 *     This is function stubs the function pointers in the given DBE screen
 *     private and increments the number of stubbed screens.
 *
 *****************************************************************************/

static void
DbeStubScreen(
    DbeScreenPrivPtr	pDbeScreenPriv,
    int			*nStubbedScreens)
{
    /* Stub DIX. */
    pDbeScreenPriv->SetupBackgroundPainter = NoopDDABoolFalse;
    pDbeScreenPriv->AllocWinPriv           = DbeFallbackAllocWinPriv;
    pDbeScreenPriv->AllocWinPrivPrivIndex  = NoopDDAInt0;
    pDbeScreenPriv->AllocWinPrivPriv       = NoopDDABoolFalse;
    pDbeScreenPriv->AllocScrnPriv          = DbeFallbackAllocScrnPriv;
    pDbeScreenPriv->AllocScrnPrivPrivIndex = NoopDDAInt0;
    pDbeScreenPriv->AllocScrnPrivPriv      = NoopDDABoolFalse;

    /* Do not unwrap PositionWindow nor DestroyWindow.  If the DDX
     * initialization function failed, we assume that it did not wrap
     * PositionWindow.  Also, DestroyWindow is only wrapped if the DDX
     * initialization function succeeded.
     */

    /* Stub DDX. */
    pDbeScreenPriv->GetVisualInfo       = NoopDDAInt0;
    pDbeScreenPriv->AllocBackBufferName = NoopDDAInt0;
    pDbeScreenPriv->SwapBuffers         = NoopDDAInt0;
    pDbeScreenPriv->BeginIdiom          = (void (*)())NoopDDA;
    pDbeScreenPriv->EndIdiom            = (void (*)())NoopDDA;
    pDbeScreenPriv->WinPrivDelete       = (void (*)())NoopDDA;
    pDbeScreenPriv->ResetProc           = (void (*)())NoopDDA;

    (*nStubbedScreens)++;

} /* DbeStubScreen() */


/******************************************************************************
 *
 * DBE DIX Procedure: NoopDDABoolFalse
 *
 * Description:
 *
 *     This is a noop, don't-do-anything  function that returns a Bool value
 *     of FALSE.
 *
 *****************************************************************************/

static Bool
NoopDDABoolFalse()
{
    return (FALSE);

} /* NoopDDABoolFalse() */


/******************************************************************************
 *
 * DBE DIX Procedure: NoopDDAInt0
 *
 * Description:
 *
 *     This is a noop, don't-do-anything  function that returns an int value
 *     of 0.
 *
 *****************************************************************************/

static int
NoopDDAInt0()
{
    return (0);

} /* NoopDDAInt0() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeDispatch
 *
 * Description:
 *
 *     This function dispatches DBE requests.
 *
 *****************************************************************************/

static int
ProcDbeDispatch(
    ClientPtr	client)
{
    REQUEST(xReq);


    switch (stuff->data)
    {
        case X_DbeGetVersion:
            return(ProcDbeGetVersion(client));

        case X_DbeAllocateBackBufferName:
            return(ProcDbeAllocateBackBufferName(client));

        case X_DbeDeallocateBackBufferName:
            return(ProcDbeDeallocateBackBufferName(client));

        case X_DbeSwapBuffers:
            return(ProcDbeSwapBuffers(client));

        case X_DbeBeginIdiom:
            return(ProcDbeBeginIdiom(client));

        case X_DbeEndIdiom:
            return(ProcDbeEndIdiom(client));

        case X_DbeGetVisualInfo:
            return(ProcDbeGetVisualInfo(client));

        case X_DbeGetBackBufferAttributes:
            return(ProcDbeGetBackBufferAttributes(client));

        default:
            return(BadRequest);
    }

} /* ProcDbeDispatch() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeGetVersion
 *
 * Description:
 *
 *     This function is for processing a DbeGetVersion request.
 *     This request returns the major and minor version numbers of this
 *     extension.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeGetVersion(
    ClientPtr	client)
{
    REQUEST(xDbeGetVersionReq);
    xDbeGetVersionReply	rep;
    register int	n;


    REQUEST_SIZE_MATCH(xDbeGetVersionReq);

    rep.type           = X_Reply;
    rep.length         = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion   = DBE_MAJOR_VERSION;
    rep.minorVersion   = DBE_MINOR_VERSION;

    if (client->swapped)
    {
        swaps(&rep.sequenceNumber, n);
    }

    WriteToClient(client, sizeof(xDbeGetVersionReply), (char *)&rep);

    return(client->noClientException);

} /* ProcDbeGetVersion() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeAllocateBackBufferName
 *
 * Description:
 *
 *     This function is for processing a DbeAllocateBackBufferName request.
 *     This request allocates a drawable ID used to refer to the back buffer
 *     of a window.
 *
 * Return Values:
 *
 *     BadAlloc    - server can not allocate resources
 *     BadIDChoice - id is out of range for client; id is already in use
 *     BadMatch    - window is not an InputOutput window;
 *                   visual of window is not on list returned by
 *                   DBEGetVisualInfo; 
 *     BadValue    - invalid swap action is specified
 *     BadWindow   - window is not a valid window
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeAllocateBackBufferName(
    ClientPtr	client)
{
    REQUEST(xDbeAllocateBackBufferNameReq);
    WindowPtr			pWin;
    DbeScreenPrivPtr		pDbeScreenPriv;
    XdbeScreenVisualInfo	scrVisInfo;
    register int		i;
    Bool			visualMatched = FALSE;
    xDbeSwapAction		swapAction;
    VisualID			visual;


    REQUEST_SIZE_MATCH(xDbeAllocateBackBufferNameReq);

    /* The window must be valid. */
    if (!(pWin = LookupWindow(stuff->window, client)))
    {
	return(BadWindow);
    }

    /* The window must be InputOutput. */
    if (pWin->drawable.class != InputOutput)
    {
	return(BadMatch);
    }

    /* The swap action must be valid. */
    swapAction = stuff->swapAction;	/* use local var for performance. */
    if ((swapAction != XdbeUndefined ) &&
        (swapAction != XdbeBackground) &&
        (swapAction != XdbeUntouched ) &&
        (swapAction != XdbeCopied    ))
    {
        return(BadValue);
    }

    /* The id must be in range and not already in use. */
    LEGAL_NEW_RESOURCE(stuff->buffer, client);

    /* The visual of the window must be in the list returned by
     * GetVisualInfo.
     */
    pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW(pWin);
    if (!(*pDbeScreenPriv->GetVisualInfo)(pWin->drawable.pScreen, &scrVisInfo))
    {
        /* GetVisualInfo() failed to allocate visual info data. */
        return(BadAlloc);
    }

    /* See if the window's visual is on the list. */
    visual = wVisual(pWin);
    for (i = 0; (i < scrVisInfo.count) && !visualMatched; i++)
    {
        if (scrVisInfo.visinfo[i].visual == visual)
	{
	    visualMatched = TRUE;
	}
    }

    /* Free what was allocated by the GetVisualInfo() call above. */
    xfree(scrVisInfo.visinfo);

    if (!visualMatched)
    {
	return(BadMatch);
    }

    return((*pDbeScreenPriv->AllocBackBufferName)(pWin,
        stuff->buffer, stuff->swapAction));

} /* ProcDbeAllocateBackBufferName() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeDeallocateBackBufferName
 *
 * Description:
 *
 *     This function is for processing a DbeDeallocateBackBufferName request.
 *     This request frees a drawable ID that was obtained by a
 *     DbeAllocateBackBufferName request.
 *
 * Return Values:
 *
 *     BadBuffer - buffer to deallocate is not associated with a window
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeDeallocateBackBufferName(client)
    ClientPtr	client;
{
    REQUEST(xDbeDeallocateBackBufferNameReq);
    DbeWindowPrivPtr	pDbeWindowPriv;
    DbeBufferIdPtr      pDbeBufferId;


    REQUEST_SIZE_MATCH(xDbeDeallocateBackBufferNameReq);

    /* Buffer name must be valid */
    if (!(pDbeWindowPriv = (DbeWindowPrivPtr)LookupIDByType(stuff->buffer,
                                                    dbeWindowPrivResType)) ||
        !(LookupIDByType(stuff->buffer, dbeDrawableResType)))
    {
        client->errorValue = stuff->buffer;
        return(dbeErrorBase + DbeBadBuffer);
    }

    /* Make sure that the id is valid for the window.
     * This is paranoid code since we already looked up the ID by type
     * above.
     */
    pDbeBufferId = &pDbeWindowPriv->idList;

    while (pDbeBufferId && (pDbeBufferId->id != stuff->buffer))
    {
        pDbeBufferId = pDbeBufferId->next;
    }

    if (!pDbeBufferId)
    {
        client->errorValue = stuff->buffer;
        return(dbeErrorBase + DbeBadBuffer);
    }
    
    FreeResource(pDbeBufferId->id, RT_NONE);

    return(Success);

} /* ProcDbeDeallocateBackBufferName() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeSwapBuffers
 *
 * Description:
 *
 *     This function is for processing a DbeSwapBuffers request.
 *     This request swaps the buffers for all windows listed, applying the
 *     appropriate swap action for each window.
 *
 * Return Values:
 *
 *     BadAlloc  - local allocation failed; this return value is not defined
 *                 by the protocol
 *     BadMatch  - a window in request is not double-buffered; a window in
 *                 request is listed more than once
 *     BadValue  - invalid swap action is specified; no swap action is
 *                 specified
 *     BadWindow - a window in request is not valid
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeSwapBuffers(
    ClientPtr	client)
{
    REQUEST(xDbeSwapBuffersReq);
    WindowPtr		pWin;
    WindowPtr		*pWins;
    DbeScreenPrivPtr	pDbeScreenPriv;
    xDbeSwapInfo	*dbeSwapInfo;
    xDbeSwapAction	*actions;
    int			error;
    register int	i, j, nStuff;


    REQUEST_AT_LEAST_SIZE(xDbeSwapBuffersReq);
    nStuff = stuff->n;	/* use local variable for performance. */

    if (nStuff == 0)
    {
        return(Success);
    }

    /* Get to the swap info appended to the end of the request. */
    dbeSwapInfo = (xDbeSwapInfo *)&stuff[1];

    /* Allocate arrays for window pointers and swap actions. */
    pWins = (WindowPtr *)ALLOCATE_LOCAL(nStuff * sizeof(WindowPtr));
    if (pWins == NULL)
    {
        return(BadAlloc);
    }

    actions = (xDbeSwapAction *)ALLOCATE_LOCAL(nStuff * sizeof(xDbeSwapAction));
    if (actions == NULL)
    {
        DEALLOCATE_LOCAL(pWins);
        return(BadAlloc);
    }

    for (i = 0; i < nStuff; i++)
    {
        /* Check all windows to swap. */

        /* Each window must be a valid window - BadWindow. */
        if (!(pWin = LookupWindow(dbeSwapInfo[i].window, client)))
        {
            DEALLOCATE_LOCAL(pWins);
            DEALLOCATE_LOCAL(actions);
	    return(BadWindow);
        }

        /* Each window must be double-buffered - BadMatch. */
        if (DBE_WINDOW_PRIV(pWin) == NULL)
        {
            DEALLOCATE_LOCAL(pWins);
            DEALLOCATE_LOCAL(actions);
            return(BadMatch);
        }

        /* Each window must only be specified once - BadMatch. */
        for (j = i + 1; j < nStuff; j++)
        {
            if (dbeSwapInfo[i].window == dbeSwapInfo[j].window)
            {
                DEALLOCATE_LOCAL(pWins);
                DEALLOCATE_LOCAL(actions);
                return(BadMatch);
	    }
        }

        /* Each swap action must be valid - BadValue. */
        if ((dbeSwapInfo[i].swapAction != XdbeUndefined ) &&
            (dbeSwapInfo[i].swapAction != XdbeBackground) &&
            (dbeSwapInfo[i].swapAction != XdbeUntouched ) &&
            (dbeSwapInfo[i].swapAction != XdbeCopied    ))
        {
            DEALLOCATE_LOCAL(pWins);
            DEALLOCATE_LOCAL(actions);
            return(BadValue);
        }

        /* Everything checks out OK.
         * Fill in window pointer and swap action arrays.
         */
        pWins[i]   = pWin;
        actions[i] = (xDbeSwapAction)dbeSwapInfo[i].swapAction;

    } /* for (i = 0; i < nStuff; i++) */


    /*## This a brute force method for handling swaps across multiple screens.
     *## The code below could be simplified by sorting "pWins" and "actions"
     *## in place in the code above.  "pWins" and "actions" would be sorted
     *## by screen.  We could count the number of swap requests per screen.
     *## This is all the information we need to call each screen's
     *## SwapBuffers().
     *##*/

    /* We have determined the the buffer(s) can be swapped.  If the swap
     * request spans more than one screen (i.e., we need to swap multiple
     * buffers that exists on different screens), then we need to call each
     * screen's SwapBuffers() with the appropriate array of window pointers and
     * actions.  Otherwise, we can just call a single screen's SwapBuffers()
     * with the "pWins" and "actions" arrays that we filled out above.
     */
  
    if (nStuff == 1)
    {
        /* We have only one buffer to swap, so we only need to deal with
         * one screen.  This is probably the most common case, so we check it
         * first.
         */
        pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW(pWin);
        error = (*pDbeScreenPriv->SwapBuffers)(client, nStuff, pWins, actions);

        DEALLOCATE_LOCAL(pWins);
        DEALLOCATE_LOCAL(actions);

        return(error);
    }

#ifdef DBE_SWAP_SCREEN_BUFFERS
    else if (swapScreenBuffers)
    {
        error = swapScreenBuffers(client, nStuff, pWins, actions);

        DEALLOCATE_LOCAL(pWins);
        DEALLOCATE_LOCAL(actions);

        return(error);
    }
#endif /* DBE_SWAP_SCREEN_BUFFERS */

    else
    {
        /* We have more that one buffer to swap, so we might need to call
         * SwapBuffers() for more than one screen.  Count the number of screens
         * we need to deal with.  If we only need to deal with one screen, then
         * call SwapBuffers() as in the "if" clause.  Otherwise we call the
         * swapScreenBuffers routine if one is registered, or we collect
         * windows and swap actions for each screen and call the SwapBuffers()
         * function for each screen individually.
         */

        static int	buffsPerScrn[MAXSCREENS];
        int     	screenCount = 0;
        register int	k;


        /* Initialize the array in which we record the number of swap requests
         * for each screen.
         */
        memset(buffsPerScrn, 0, sizeof(int) * MAXSCREENS);

        /* Count the number of swap requests for each screen. */
        for (i = 0; i < nStuff; i++)
        {
           buffsPerScrn[pWins[i]->drawable.pScreen->myNum]++;
        }

        /* Count the number of screens we need to deal with. */
        for (i = 0; i < MAXSCREENS; i++)
        {
           if (buffsPerScrn[i] != 0)
           {
               screenCount++;
           }
        }
        
        if (screenCount == 1)
        {
            /* We have only one screen to deal with, so we can make a single
             * SwapBuffers() call.
             */
            pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW(pWin);
            error = (*pDbeScreenPriv->SwapBuffers)(client, nStuff,pWins,
                                                   actions);

            DEALLOCATE_LOCAL(pWins);
            DEALLOCATE_LOCAL(actions);

            return(error);
        }
        else
        {
            /* We have more than one screen to deal with.  We need to group
             * windows and swap actions to pass to each screen's
             * SwapBuffers() function.
             */

            WindowPtr		*pWinsMS;	/* multiple screens */
            xDbeSwapAction	*actionsMS;	/* multiple screens */

            for (i = 0; i < MAXSCREENS; i++)
            {
                if (buffsPerScrn[i] != 0)
                {
                    pWinsMS = (WindowPtr *)ALLOCATE_LOCAL(buffsPerScrn[i] *
                                                          sizeof(WindowPtr));
                    if (pWinsMS == NULL)
                    {
                        DEALLOCATE_LOCAL(pWins);
                        DEALLOCATE_LOCAL(actions);
                        return(BadAlloc);
                    }

                    actionsMS = (xDbeSwapAction *)ALLOCATE_LOCAL(
                                buffsPerScrn[i] * sizeof(xDbeSwapAction));

                    if (actionsMS == NULL)
                    {
                        DEALLOCATE_LOCAL(pWins);
                        DEALLOCATE_LOCAL(actions);
                        DEALLOCATE_LOCAL(pWinsMS);
                        return(BadAlloc);
                    }

                    /* Get the screen's windows and swap actions. */
                    k = 0;
                    for (j = 0; k < buffsPerScrn[i]; j++)
                    {
                        if (pWins[j]->drawable.pScreen->myNum == i)
                        {
                            pWinsMS[k]   = pWins[j]; 
                            actionsMS[k] = actions[j]; 
                            k++;
                        }

                    } /* for j */

                    /* Call the screen's SwapBuffers() function. */
                    pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW(pWinsMS[0]);
                    error = (*pDbeScreenPriv->SwapBuffers)(client, k ,pWinsMS,
                                                           actionsMS);

                    DEALLOCATE_LOCAL(pWinsMS);
                    DEALLOCATE_LOCAL(actionsMS);

                    if (error != Success)
                    {
                        DEALLOCATE_LOCAL(pWins);
                        DEALLOCATE_LOCAL(actions);
                        return(error);
                    }

                } /* if */

            } /* for i */

            DEALLOCATE_LOCAL(pWins);
            DEALLOCATE_LOCAL(actions);

            return(Success);

        } /* else -- swaps across more than one screen */

    } /* else -- more than one buffer to swap */

} /* ProcDbeSwapBuffers() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeBeginIdiom
 *
 * Description:
 *
 *     This function is for processing a DbeBeginIdiom request.
 *     This request informs the server that a complex swap will immediately
 *     follow this request.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeBeginIdiom(
    ClientPtr	client)
{
    REQUEST(xDbeBeginIdiomReq);
    DbeScreenPrivPtr	pDbeScreenPriv;
    register int	i;


    REQUEST_SIZE_MATCH(xDbeBeginIdiomReq);

    for (i = 0; i < screenInfo.numScreens; i++)
    {
        pDbeScreenPriv = DBE_SCREEN_PRIV(screenInfo.screens[i]);
        (*pDbeScreenPriv->BeginIdiom)(client);
    }

    return(Success);

} /* ProcDbeBeginIdiom() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeEndIdiom
 *
 * Description:
 *
 *     This function is for processing a DbeEndIdiom request.
 *     This request informs the server that a complex swap has concluded.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeEndIdiom(
    ClientPtr	client)
{
    REQUEST(xDbeEndIdiomReq);
    DbeScreenPrivPtr	pDbeScreenPriv;
    register int	i;


    REQUEST_SIZE_MATCH(xDbeEndIdiomReq);

    for (i = 0; i < screenInfo.numScreens; i++)
    {
        pDbeScreenPriv = DBE_SCREEN_PRIV(screenInfo.screens[i]);
        (*pDbeScreenPriv->EndIdiom)(client);
    }

    return(Success);

} /* ProcDbeEndIdiom() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeGetVisualInfo
 *
 * Description:
 *
 *     This function is for processing a ProcDbeGetVisualInfo request.
 *     This request returns information about which visuals support
 *     double buffering.
 *
 * Return Values:
 *
 *     BadDrawable - value in screen specifiers is not a valid drawable
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeGetVisualInfo(
    ClientPtr	client)
{
    REQUEST(xDbeGetVisualInfoReq);
    DbeScreenPrivPtr		pDbeScreenPriv;
    xDbeGetVisualInfoReply	rep;
    Drawable			*drawables;
    DrawablePtr			*pDrawables = NULL;
    register int		i, j, n;
    register int		count;  /* number of visual infos in reply */
    register int		length; /* length of reply */
    ScreenPtr			pScreen;
    XdbeScreenVisualInfo	*pScrVisInfo;


    REQUEST_AT_LEAST_SIZE(xDbeGetVisualInfoReq);

    /* Make sure any specified drawables are valid. */
    if (stuff->n != 0)
    {
        if (!(pDrawables = (DrawablePtr *)ALLOCATE_LOCAL(stuff->n *
                                                 sizeof(DrawablePtr))))
        {
            return(BadAlloc);
        }

        drawables = (Drawable *)&stuff[1];

        for (i = 0; i < stuff->n; i++)
        {
            if (!(pDrawables[i] = (DrawablePtr)LookupDrawable(drawables[i],
                                                              client)))
            {
                DEALLOCATE_LOCAL(pDrawables);
                return(BadDrawable);
            }
        }
    }

    count = (stuff->n == 0) ? screenInfo.numScreens : stuff->n;
    if (!(pScrVisInfo = (XdbeScreenVisualInfo *)xalloc(count *
                        sizeof(XdbeScreenVisualInfo))))
    {
        if (pDrawables)
        {
            DEALLOCATE_LOCAL(pDrawables);
        }

        return(BadAlloc);
    }

    length = 0;

    for (i = 0; i < count; i++)
    {
        pScreen = (stuff->n == 0) ? screenInfo.screens[i] :
                                    pDrawables[i]->pScreen;
        pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

        if (!(*pDbeScreenPriv->GetVisualInfo)(pScreen, &pScrVisInfo[i]))
        {
            /* We failed to alloc pScrVisInfo[i].visinfo. */

            /* Free visinfos that we allocated for previous screen infos.*/
            for (j = 0; j < i; j++)
            {
                xfree(pScrVisInfo[j].visinfo);
            }

            /* Free pDrawables if we needed to allocate it above. */
            if (pDrawables)
            {
                DEALLOCATE_LOCAL(pDrawables);
            }

            return(BadAlloc);
        }

        /* Account for n, number of xDbeVisInfo items in list. */
        length += sizeof(CARD32);

        /* Account for n xDbeVisInfo items */
        length += pScrVisInfo[i].count * sizeof(xDbeVisInfo);
    }

    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = length;
    rep.m              = count;

    if (client->swapped)
    {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.m, n);
    }

    /* Send off reply. */
    WriteToClient(client, sizeof(xDbeGetVisualInfoReply), (char *)&rep);

    for (i = 0; i < count; i++)
    {
        CARD32	data32;

        /* For each screen in the reply, send off the visual info */

        /* Send off number of visuals. */
        data32 = (CARD32)pScrVisInfo[i].count;

        if (client->swapped)
        {
            swapl(&data32, n);
        }

        WriteToClient(client, sizeof(CARD32), (char *)&data32);

        /* Now send off visual info items. */
        for (j = 0; j < pScrVisInfo[i].count; j++)
        {
            xDbeVisInfo		visInfo;

            /* Copy the data in the client data structure to a protocol
             * data structure.  We will send data to the client from the
             * protocol data structure.
             */

            visInfo.visualID  = (CARD32)pScrVisInfo[i].visinfo[j].visual;
            visInfo.depth     = (CARD8) pScrVisInfo[i].visinfo[j].depth;
            visInfo.perfLevel = (CARD8) pScrVisInfo[i].visinfo[j].perflevel;

            if (client->swapped)
            {
                swapl(&visInfo.visualID, n);

                /* We do not need to swap depth and perfLevel since they are
                 * already 1 byte quantities.
                 */
            }

            /* Write visualID(32), depth(8), perfLevel(8), and pad(16). */
            WriteToClient(client, 2*sizeof(CARD32), (char *)&visInfo.visualID);
        }
    }

    /* Clean up memory. */
    for (i = 0; i < count; i++)
    {
        xfree(pScrVisInfo[i].visinfo);
    }
    xfree(pScrVisInfo);

    if (pDrawables)
    {
        DEALLOCATE_LOCAL(pDrawables);
    }

    return(client->noClientException);

} /* ProcDbeGetVisualInfo() */


/******************************************************************************
 *
 * DBE DIX Procedure: ProcDbeGetbackBufferAttributes
 *
 * Description:
 *
 *     This function is for processing a ProcDbeGetbackBufferAttributes
 *     request.  This request returns information about a back buffer.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
ProcDbeGetBackBufferAttributes(
    ClientPtr	client)
{
    REQUEST(xDbeGetBackBufferAttributesReq);
    xDbeGetBackBufferAttributesReply	rep;
    DbeWindowPrivPtr			pDbeWindowPriv;
    int					n;


    REQUEST_SIZE_MATCH(xDbeGetBackBufferAttributesReq);

    if (!(pDbeWindowPriv = (DbeWindowPrivPtr)LookupIDByType(stuff->buffer,
        dbeWindowPrivResType)))
    {
        rep.attributes = None;
    }
    else
    {
        rep.attributes = pDbeWindowPriv->pWindow->drawable.id;
    }
        
    rep.type           = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length         = 0;
    
    if (client->swapped)
    {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swapl(&rep.attributes, n);
    }

    WriteToClient(client, sizeof(xDbeGetBackBufferAttributesReply),
                  (char *)&rep);
    return(client->noClientException);

} /* ProcDbeGetbackBufferAttributes() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeDispatch
 *
 * Description:
 *
 *     This function dispatches DBE requests on a swapped server.
 *
 *****************************************************************************/

static int
SProcDbeDispatch(
    ClientPtr	client)
{
    REQUEST(xReq);


    switch (stuff->data)
    {
        case X_DbeGetVersion:
            return(SProcDbeGetVersion(client));

        case X_DbeAllocateBackBufferName:
            return(SProcDbeAllocateBackBufferName(client));

        case X_DbeDeallocateBackBufferName:
            return(SProcDbeDeallocateBackBufferName(client));

        case X_DbeSwapBuffers:
            return(SProcDbeSwapBuffers(client));

        case X_DbeBeginIdiom:
            return(SProcDbeBeginIdiom(client));

        case X_DbeEndIdiom:
            return(SProcDbeEndIdiom(client));

        case X_DbeGetVisualInfo:
            return(SProcDbeGetVisualInfo(client));

        case X_DbeGetBackBufferAttributes:
            return(SProcDbeGetBackBufferAttributes(client));

        default:
            return (BadRequest);
    }

} /* SProcDbeDispatch() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeGetVersion
 *
 * Description:
 *
 *     This function is for processing a DbeGetVersion request on a swapped
 *     server.  This request returns the major and minor version numbers of
 *     this extension.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeGetVersion(
    ClientPtr	client)
{
    REQUEST(xDbeGetVersionReq);
    register int	n;


    swaps(&stuff->length, n);
    return(ProcDbeGetVersion(client));

} /* SProcDbeGetVersion() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeAllocateBackBufferName
 *
 * Description:
 *
 *     This function is for processing a DbeAllocateBackBufferName request on
 *     a swapped server.  This request allocates a drawable ID used to refer
 *     to the back buffer of a window.
 *
 * Return Values:
 *
 *     BadAlloc    - server can not allocate resources
 *     BadIDChoice - id is out of range for client; id is already in use
 *     BadMatch    - window is not an InputOutput window;
 *                   visual of window is not on list returned by
 *                   DBEGetVisualInfo; 
 *     BadValue    - invalid swap action is specified
 *     BadWindow   - window is not a valid window
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeAllocateBackBufferName(
    ClientPtr	client)
{
    REQUEST(xDbeAllocateBackBufferNameReq);
    register int	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDbeAllocateBackBufferNameReq);

    swapl(&stuff->window, n);
    swapl(&stuff->buffer, n);
    /* stuff->swapAction is a byte.  We do not need to swap this field. */

    return(ProcDbeAllocateBackBufferName(client));

} /* SProcDbeAllocateBackBufferName() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeDeallocateBackBufferName
 *
 * Description:
 *
 *     This function is for processing a DbeDeallocateBackBufferName request
 *     on a swapped server.  This request frees a drawable ID that was
 *     obtained by a DbeAllocateBackBufferName request.
 *
 * Return Values:
 *
 *     BadBuffer - buffer to deallocate is not associated with a window
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeDeallocateBackBufferName(
    ClientPtr	client)
{
    REQUEST (xDbeDeallocateBackBufferNameReq);
    register int	n;


    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDbeDeallocateBackBufferNameReq);

    swapl(&stuff->buffer, n);

    return(ProcDbeDeallocateBackBufferName(client));

} /* SProcDbeDeallocateBackBufferName() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeSwapBuffers
 *
 * Description:
 *
 *     This function is for processing a DbeSwapBuffers request on a swapped
 *     server.  This request swaps the buffers for all windows listed,
 *     applying the appropriate swap action for each window.
 *
 * Return Values:
 *
 *     BadMatch  - a window in request is not double-buffered; a window in
 *                 request is listed more than once; all windows in request do
 *                 not have the same root
 *     BadValue  - invalid swap action is specified
 *     BadWindow - a window in request is not valid
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeSwapBuffers(
    ClientPtr	client)
{
    REQUEST(xDbeSwapBuffersReq);
    register int	i, n;
    xDbeSwapInfo	*pSwapInfo;


    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDbeSwapBuffersReq);

    swapl(&stuff->n, n);

    if (stuff->n != 0)
    { 
        pSwapInfo = (xDbeSwapInfo *)stuff+1;

        /* The swap info following the fix part of this request is a window(32)
         * followed by a 1 byte swap action and then 3 pad bytes.  We only need
         * to swap the window information.
         */
        for (i = 0; i < stuff->n; i++)
        {
            swapl(&pSwapInfo->window, n);
        }
    }

    return(ProcDbeSwapBuffers(client));

} /* SProcDbeSwapBuffers() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeBeginIdiom
 *
 * Description:
 *
 *     This function is for processing a DbeBeginIdiom request on a swapped
 *     server.  This request informs the server that a complex swap will
 *     immediately follow this request.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeBeginIdiom(
    ClientPtr	client)
{
    REQUEST(xDbeBeginIdiomReq);
    register int	n;

    swaps(&stuff->length, n);
    return(ProcDbeBeginIdiom(client));

} /* SProcDbeBeginIdiom() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeEndIdiom
 *
 * Description:
 *
 *     This function is for processing a DbeEndIdiom request.
 *     This request informs the server that a complex swap has concluded.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeEndIdiom(
    ClientPtr	client)
{
    REQUEST(xDbeEndIdiomReq);
    register int	n;

    swaps(&stuff->length, n);
    return(ProcDbeEndIdiom(client));

} /* SProcDbeEndIdiom() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeGetVisualInfo
 *
 * Description:
 *
 *     This function is for processing a ProcDbeGetVisualInfo request on a
 *     swapped server.  This request returns information about which visuals
 *     support double buffering.
 *
 * Return Values:
 *
 *     BadDrawable - value in screen specifiers is not a valid drawable
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeGetVisualInfo(
    ClientPtr	client)
{
    REQUEST(xDbeGetVisualInfoReq);
    register int	n;


    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xDbeGetVisualInfoReq);

    swapl(&stuff->n, n);
    SwapRestL(stuff);

    return(ProcDbeGetVisualInfo(client));

} /* SProcDbeGetVisualInfo() */


/******************************************************************************
 *
 * DBE DIX Procedure: SProcDbeGetbackBufferAttributes
 *
 * Description:
 *
 *     This function is for processing a ProcDbeGetbackBufferAttributes
 *     request on a swapped server.  This request returns information about a
 *     back buffer.
 *
 * Return Values:
 *
 *     Success
 *
 *****************************************************************************/

static int
SProcDbeGetBackBufferAttributes(
    ClientPtr	client)
{
    REQUEST (xDbeGetBackBufferAttributesReq);
    register int	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDbeGetBackBufferAttributesReq);

    swapl(&stuff->buffer, n);

    return(ProcDbeGetBackBufferAttributes(client));

} /* SProcDbeGetBackBufferAttributes() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeSetupBackgroundPainter
 *
 * Description:
 *
 *     This function sets up pGC to clear pixmaps.
 *
 * Return Values:
 *
 *     TRUE  - setup was successful
 *     FALSE - the window's background state is NONE
 * 
 *****************************************************************************/

static Bool
DbeSetupBackgroundPainter(
    WindowPtr	pWin,
    GCPtr	pGC)
{
    pointer	gcvalues[4];
    int		ts_x_origin, ts_y_origin;
    PixUnion	background;
    int		backgroundState;
    Mask	gcmask;


    /* First take care of any ParentRelative stuff by altering the
     * tile/stipple origin to match the coordinates of the upper-left
     * corner of the first ancestor without a ParentRelative background.
     * This coordinate is, of course, negative.
     */
    ts_x_origin = ts_y_origin = 0;
    while (pWin->backgroundState == ParentRelative)
    {
        ts_x_origin -= pWin->origin.x;
        ts_y_origin -= pWin->origin.y;

        pWin = pWin->parent;
    }
    backgroundState = pWin->backgroundState;
    background      = pWin->background;

    switch (backgroundState)
    {
        case BackgroundPixel:
            gcvalues[0] = (pointer)background.pixel;
            gcvalues[1] = (pointer)FillSolid;
            gcmask = GCForeground|GCFillStyle;
            break;

        case BackgroundPixmap:
            gcvalues[0] = (pointer)FillTiled;
            gcvalues[1] = (pointer)background.pixmap;
            gcvalues[2] = (pointer)ts_x_origin;
            gcvalues[3] = (pointer)ts_y_origin;
            gcmask = GCFillStyle|GCTile|GCTileStipXOrigin|GCTileStipYOrigin;
            break;

        default:
            /* pWin->backgroundState == None */
            return(FALSE);
    }

    if (DoChangeGC(pGC, gcmask, (XID *)gcvalues, TRUE) != 0)
    {
        return(FALSE);
    }

    return(TRUE);

} /* DbeSetupBackgroundPainter() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeDrawableDelete
 *
 * Description:
 *
 *     This is the resource delete function for dbeDrawableResType.
 *     It is registered when the drawable resource type is created in
 *     DbeExtensionInit().
 *
 *     To make resource deletion simple, we do not do anything in this function
 *     and leave all resource deleteion to DbeWindowPrivDelete(), which will
 *     eventually be called or already has been called.  Deletion functions are
 *     not guaranteed to be called in any particular order.
 *
 *****************************************************************************/
static int
DbeDrawableDelete(
    pointer	pDrawable,
    XID		id)
{
    return Success;
} /* DbeDrawableDelete() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeWindowPrivDelete
 *
 * Description:
 *
 *     This is the resource delete function for dbeWindowPrivResType.
 *     It is registered when the drawable resource type is created in
 *     DbeExtensionInit().
 *
 *****************************************************************************/
static int
DbeWindowPrivDelete(
    pointer	pDbeWindowPriv,
    XID		id)
{
    DbeScreenPrivPtr pDbeScreenPriv;


    pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW_PRIV(
                         (DbeWindowPrivPtr)pDbeWindowPriv);
    (*pDbeScreenPriv->WinPrivDelete)((DbeWindowPrivPtr)pDbeWindowPriv, id);
    return Success;

} /* DbeWindowPrivDelete() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeResetProc
 *
 * Description:
 *
 *     This routine is called at the end of every server generation.
 *     It deallocates any memory reserved for the extension and performs any
 *     other tasks related to shutting down the extension.
 *
 *****************************************************************************/
static void
DbeResetProc(
    ExtensionEntry	*extEntry)
{
    int			i;
    ScreenPtr		pScreen;
    DbeScreenPrivPtr	pDbeScreenPriv;
    

    if (dbeScreenPrivIndex < 0)
    {
	return;
    }

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

	if (pDbeScreenPriv)
	{
            /* Unwrap DestroyWindow, which was wrapped in DbeExtensionInit().*/
            pScreen->DestroyWindow = pDbeScreenPriv->DestroyWindow;

	    (*pDbeScreenPriv->ResetProc)(pScreen);

            if (pDbeScreenPriv->winPrivPrivSizes)
            {
	        xfree(pDbeScreenPriv->winPrivPrivSizes);
            }

            if (pDbeScreenPriv->scrnPrivPrivSizes)
            {
	        xfree(pDbeScreenPriv->scrnPrivPrivSizes);
            }

	    xfree(pDbeScreenPriv);
	}
    }

    /* We want to init the initialization function table after every server
     * reset in DbeRegisterFunction().
     */
    firstRegistrationPass = TRUE;

} /* DbeResetProc() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeDestroyWindow
 *
 * Description:
 *
 *     This is the wrapper for pScreen->DestroyWindow.
 *     This function frees buffer resources for a window before it is
 *     destroyed.
 *
 *****************************************************************************/

static Bool
DbeDestroyWindow(
    WindowPtr	pWin)
{
    DbeScreenPrivPtr	pDbeScreenPriv;
    DbeWindowPrivPtr	pDbeWindowPriv;
    ScreenPtr		pScreen;
    Bool		ret;


    /*
     **************************************************************************
     ** 1. Unwrap the member routine.
     **************************************************************************
     */

    pScreen                = pWin->drawable.pScreen;
    pDbeScreenPriv         = DBE_SCREEN_PRIV(pScreen);
    pScreen->DestroyWindow = pDbeScreenPriv->DestroyWindow;

    /*
     **************************************************************************
     ** 2. Do any work necessary before the member routine is called.
     **
     **    Call the window priv delete function for all buffer IDs associated
     **    with this window.
     **************************************************************************
     */

    if (pDbeWindowPriv = DBE_WINDOW_PRIV(pWin))
    {
        while (pDbeWindowPriv)
        {
            /* *DbeWinPrivDelete() will free the window private and set it to
             * NULL if there are no more buffer IDs associated with this
             * window.
             */
            FreeResource(pDbeWindowPriv->idList.id, RT_NONE);
            pDbeWindowPriv = DBE_WINDOW_PRIV(pWin);
        }
    }

    /*
     **************************************************************************
     ** 3. Call the member routine, saving its result if necessary.
     **************************************************************************
     */

    ret = (*pScreen->DestroyWindow)(pWin);

    /*
     **************************************************************************
     ** 4. Rewrap the member routine, restoring the wrapper value first in case
     **    the wrapper (or something that it wrapped) change this value.
     **************************************************************************
     */

    pDbeScreenPriv->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = DbeDestroyWindow;

    /*
     **************************************************************************
     ** 5. Do any work necessary after the member routine has been called.
     **
     **    In this case we do not need to do anything.
     **************************************************************************
     */

    return(ret);

} /* DbeDestroyWindow() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocWinPriv
 *
 * Description:
 *
 *     This function was cloned from AllocateWindow() in window.c.
 *     This function allocates a window priv structure to be associated 
 *     with a double-buffered window.
 *
 *****************************************************************************/
static DbeWindowPrivPtr
DbeAllocWinPriv(
    ScreenPtr	pScreen)
{
    DbeWindowPrivPtr		pDbeWindowPriv;
    DbeScreenPrivPtr		pDbeScreenPriv;
    register char		*ptr;
    register DevUnion		*ppriv;
    register unsigned int	*sizes;
    register unsigned int	size;
    register int		i;

    pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);
    pDbeWindowPriv = (DbeWindowPrivPtr)xalloc(pDbeScreenPriv->totalWinPrivSize);

    if (pDbeWindowPriv)
    {
        ppriv = (DevUnion *)(pDbeWindowPriv + 1);
        pDbeWindowPriv->devPrivates = ppriv;
        sizes = pDbeScreenPriv->winPrivPrivSizes;
        ptr = (char *)(ppriv + pDbeScreenPriv->winPrivPrivLen);
        for (i = pDbeScreenPriv->winPrivPrivLen; --i >= 0; ppriv++, sizes++)
        {
            if (size = *sizes)
            {
                ppriv->ptr = (pointer)ptr;
                ptr += size;
            }
            else
                ppriv->ptr = (pointer)NULL;
        }
    }

    return(pDbeWindowPriv);

} /* DbeAllocWinPriv() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeFallbackAllocWinPriv
 *
 * Description:
 *
 *     This is a fallback function for AllocWinPriv().
 *
 *****************************************************************************/

static DbeWindowPrivPtr
DbeFallbackAllocWinPriv(
    ScreenPtr	pScreen)
{
    return (NULL);

} /* DbeFallbackAllocWinPriv() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocWinPrivPrivIndex
 *
 * Description:
 *
 *     This function was cloned from AllocateWindowPrivateIndex() in window.c.
 *     This function allocates a new window priv priv index by simply returning
 *     an incremented private counter.
 *
 *****************************************************************************/

static int
DbeAllocWinPrivPrivIndex()
{
    return winPrivPrivCount++;

} /* DbeAllocWinPrivPrivIndex() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocWinPrivPriv
 *
 * Description:
 *
 *     This function was cloned from AllocateWindowPrivate() in privates.c.
 *     This function allocates a private structure to be hung off
 *     a window private.
 *
 *****************************************************************************/

static Bool
DbeAllocWinPrivPriv(
    register ScreenPtr	pScreen,
    int			index,
    unsigned int	amount)
{
    DbeScreenPrivPtr	pDbeScreenPriv;
    unsigned int	oldamount;


    pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

    if (index >= pDbeScreenPriv->winPrivPrivLen)
    {
	unsigned *nsizes;
	nsizes = (unsigned *)xrealloc(pDbeScreenPriv->winPrivPrivSizes,
				      (index + 1) * sizeof(unsigned));
	if (!nsizes)
        {
	    return(FALSE);
        }

	while (pDbeScreenPriv->winPrivPrivLen <= index)
	{
	    nsizes[pDbeScreenPriv->winPrivPrivLen++] = 0;
	    pDbeScreenPriv->totalWinPrivSize += sizeof(DevUnion);
	}

	pDbeScreenPriv->winPrivPrivSizes = nsizes;
    }

    oldamount = pDbeScreenPriv->winPrivPrivSizes[index];

    if (amount > oldamount)
    {
	pDbeScreenPriv->winPrivPrivSizes[index] = amount;
	pDbeScreenPriv->totalWinPrivSize += (amount - oldamount);
    }
    return(TRUE);

} /* DbeAllocWinPrivPriv() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeFallbackAllocScrnPriv
 *
 * Description:
 *
 *     This is a fallback function for AllocScrnPriv().
 *
 *****************************************************************************/

static DbeScreenPrivPtr
DbeFallbackAllocScrnPriv(
    DbeScreenPrivPtr	pDbeScreenPriv)
{
    return (NULL);

} /* DbeFallbackAllocWinPriv() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocScrnPriv
 *
 * Description:
 *
 *     This function re-allocates a DBE screen priv structure so that
 *     privates can be hung off it.
 *
 *****************************************************************************/
static DbeScreenPrivPtr
DbeAllocScrnPriv(
    DbeScreenPrivPtr	pOldDbeScreenPriv)
{
    DbeScreenPrivPtr		pNewDbeScreenPriv;
    register char		*ptr;
    register DevUnion		*ppriv;
    register unsigned int	*sizes;
    register unsigned int	size;
    register int		i;


    pNewDbeScreenPriv = (DbeScreenPrivPtr)xrealloc(pOldDbeScreenPriv,
                        pOldDbeScreenPriv->totalScrnPrivSize);

    if (pNewDbeScreenPriv)
    {
        ppriv = (DevUnion *)(pNewDbeScreenPriv + 1);
        pNewDbeScreenPriv->devPrivates = ppriv;
        sizes = pOldDbeScreenPriv->scrnPrivPrivSizes;
        ptr = (char *)(ppriv + pOldDbeScreenPriv->scrnPrivPrivLen);
        for (i = pOldDbeScreenPriv->scrnPrivPrivLen; --i >= 0; ppriv++, sizes++)
        {
            if (size = *sizes)
            {
                ppriv->ptr = (pointer)ptr;
                ptr += size;
            }
            else
                ppriv->ptr = (pointer)NULL;
        }
    }

    return(pNewDbeScreenPriv);

} /* DbeAllocScrnPriv() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocScrnPrivPrivIndex
 *
 * Description:
 *
 *     This function was cloned from AllocateWindowPrivateIndex() in window.c.
 *     This function allocates a new screen priv priv index by simply returning
 *     an incremented private counter.
 *
 *****************************************************************************/

static int
DbeAllocScrnPrivPrivIndex()
{
    return scrnPrivPrivCount++;

} /* DbeAllocScrnPrivPrivIndex() */


/******************************************************************************
 *
 * DBE DIX Procedure: DbeAllocScrnPrivPriv
 *
 * Description:
 *
 *     This function was cloned from AllocateWindowPrivate() in privates.c.
 *     This function allocates a private structure to be hung off
 *     a screen private.
 *
 *****************************************************************************/

static Bool
DbeAllocScrnPrivPriv(
    register ScreenPtr	pScreen,
    int			index,
    unsigned int	amount)
{
    DbeScreenPrivPtr	pDbeScreenPriv;
    unsigned int	oldamount;


    pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

    if (index >= pDbeScreenPriv->scrnPrivPrivLen)
    {
	unsigned *nsizes;
	nsizes = (unsigned *)xrealloc(pDbeScreenPriv->scrnPrivPrivSizes,
				      (index + 1) * sizeof(unsigned));
	if (!nsizes)
        {
	    return(FALSE);
        }

	while (pDbeScreenPriv->scrnPrivPrivLen <= index)
	{
	    nsizes[pDbeScreenPriv->scrnPrivPrivLen++] = 0;
	    pDbeScreenPriv->totalScrnPrivSize += sizeof(DevUnion);
	}

	pDbeScreenPriv->scrnPrivPrivSizes = nsizes;
    }

    oldamount = pDbeScreenPriv->scrnPrivPrivSizes[index];

    if (amount > oldamount)
    {
	pDbeScreenPriv->scrnPrivPrivSizes[index] = amount;
	pDbeScreenPriv->totalScrnPrivSize += (amount - oldamount);
    }
    return(TRUE);

} /* DbeAllocScrnPrivPriv() */


