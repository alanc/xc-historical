/* $XConsortium: copyright.h,v 1.14 95/04/13 16:08:25 dpw Exp $ */

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
 * $Header: midbe.c,v 55.11 95/04/27 02:06:46 yip Exp $
 *
 *     Machine-independent DBE code
 *
 *****************************************************************************/


/* INCLUDES */

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include "dbestruct.h"
#include "midbestruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "inputstr.h"


/* DEFINES */


/* TYPEDEFS */


/* GLOBALS */

static int	miDbeWindowPrivPrivGeneration =  0;
static int	miDbeWindowPrivPrivIndex      = -1;
RESTYPE		dbeDrawableResType;
RESTYPE		dbeWindowPrivResType;
int		dbeScreenPrivIndex = -1;
int		dbeWindowPrivIndex = -1;


/* EXTERNS */


/* FORWARDS */

static void miDbeAliasBuffers(
    DbeWindowPrivPtr	pDbeWindowPriv);

static int miDbeAllocBackBufferName(
    WindowPtr		pWin,
    XID			bufId,
    xDbeSwapAction	swapAction);

static void miDbeBeginIdiom(
    ClientPtr	client);

static void miDbeEndIdiom(
    ClientPtr	client);

static Bool miDbeGetVisualInfo(
    ScreenPtr			pScreen,
    XdbeScreenVisualInfo	*pScrVisInfo);

Bool miDbeInit(
    ScreenPtr		pScreen,
    DbeScreenPrivPtr	pDbeScreenPriv);

static Bool miDbePositionWindow(
    WindowPtr	pWin,
    int		x,
    int		y);

static void miDbeResetProc(
    ScreenPtr	pScreen);

static int miDbeSwapBuffers(
    ClientPtr		client,
    int			nWindows,
    WindowPtr		*pWins,
    xDbeSwapAction	*actions);

static void miDbeWinPrivDelete(
    DbeWindowPrivPtr	pDbeWindowPriv,
    XID			bufId);


/******************************************************************************
 *
 * DBE MI Procedure: miDbeInit
 *
 * Description:
 *
 *     This is the MI initialization function called by DbeExtensionInit().
 *
 *****************************************************************************/

Bool
miDbeInit(
    ScreenPtr		pScreen,
    DbeScreenPrivPtr	pDbeScreenPriv)
{
    /* Copy resource types created by DIX */
    dbeDrawableResType   = pDbeScreenPriv->dbeDrawableResType;
    dbeWindowPrivResType = pDbeScreenPriv->dbeWindowPrivResType;

    /* Copy private indices created by DIX */
    dbeScreenPrivIndex = pDbeScreenPriv->dbeScreenPrivIndex;
    dbeWindowPrivIndex = pDbeScreenPriv->dbeWindowPrivIndex;

    /* Reset the window private if generations do not match. */
    if (miDbeWindowPrivPrivGeneration != serverGeneration)
    {
	miDbeWindowPrivPrivIndex = (*pDbeScreenPriv->AllocWinPrivPrivIndex)(
            pScreen);
	miDbeWindowPrivPrivGeneration = serverGeneration;

        /* Allocate the window priv priv. */
        if (!(*pDbeScreenPriv->AllocWinPrivPriv)(pScreen,
            miDbeWindowPrivPrivIndex, sizeof(MiDbeWindowPrivPrivRec)))
        {
            return(FALSE);
        }
    }

    /* Wrap functions. */
    pDbeScreenPriv->PositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow        = miDbePositionWindow;

    /* Initialize the per-screen DBE function pointers. */
    pDbeScreenPriv->GetVisualInfo         = miDbeGetVisualInfo;
    pDbeScreenPriv->AllocBackBufferName   = miDbeAllocBackBufferName;
    pDbeScreenPriv->SwapBuffers           = miDbeSwapBuffers;
    pDbeScreenPriv->BeginIdiom            = miDbeBeginIdiom;
    pDbeScreenPriv->EndIdiom              = miDbeEndIdiom;
    pDbeScreenPriv->ResetProc             = miDbeResetProc;
    pDbeScreenPriv->WinPrivDelete         = miDbeWinPrivDelete;

    return(TRUE);

} /* miDbeInit() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeGetVisualInfo
 *
 * Description:
 *
 *     This is the MI function for the DbeGetVisualInfo request.  This function
 *     is called through pDbeScreenPriv->GetVisualInfo.  This function is also
 *     called for the DbeAllocateBackBufferName request at the extension level;
 *     it is called by ProcDbeAllocateBackBufferName() in dbe.c.
 *
 *     If memory allocation fails or we can not get the visual info, this
 *     function returns FALSE.  Otherwise, it returns TRUE for success.
 *
 *****************************************************************************/

static Bool
miDbeGetVisualInfo(
    ScreenPtr			pScreen,
    XdbeScreenVisualInfo	*pScrVisInfo)
{
    register int	i, j, k;
    register int	count;
    DepthPtr		pDepth;
    XdbeVisualInfo	*visInfo;


    /* Determine number of visuals for this screen. */
    for (i = 0, count = 0; i < pScreen->numDepths; i++)
    {
        count += pScreen->allowedDepths[i].numVids;
    }

    /* Allocate an array of XdbeVisualInfo items. */
    if (!(visInfo = (XdbeVisualInfo *)xalloc(count * sizeof(XdbeVisualInfo))))
    {
        return(FALSE); /* memory alloc failure */
    }

    for (i = 0, k = 0; i < pScreen->numDepths; i++)
    {
        /* For each depth of this screen, get visual information. */

        pDepth = &pScreen->allowedDepths[i];

        for (j = 0; j < pDepth->numVids; j++)
        {
            /* For each visual for this depth of this screen, get visual ID
             * and visual depth.  Since this is MI code, we will always return
             * the same performance level for all visuals (0).  A higher
             * performance level value indicates higher performance.
             */
            visInfo[k].visual    = pDepth->vids[j];
            visInfo[k].depth     = pDepth->depth;
            visInfo[k].perflevel = 0;
            k++;
        }
    }

    /* Record the number of visuals and point visual_depth to
     * the array of visual info.
     */
    pScrVisInfo->count   = count;
    pScrVisInfo->visinfo = visInfo;

    return(TRUE); /* success */

} /* miDbeGetVisualInfo() */


/******************************************************************************
 *
 * DBE MI Procedure: miAllocBackBufferName
 *
 * Description:
 *
 *     This is the MI function for the DbeAllocateBackBufferName request.
 *
 *****************************************************************************/

static int
miDbeAllocBackBufferName(
    WindowPtr		pWin,
    XID			bufId,
    xDbeSwapAction	swapAction)
{
    ScreenPtr			pScreen;
    DbeWindowPrivPtr		pDbeWindowPriv;
    MiDbeWindowPrivPrivPtr	pDbeWindowPrivPriv; 
    DbeScreenPrivPtr		pDbeScreenPriv;
    GCPtr			pGC;
    xRectangle			clearRect;


    pScreen = pWin->drawable.pScreen;

    if ((pDbeWindowPriv = DBE_WINDOW_PRIV(pWin)) == NULL)
    {
        /* There is no buffer associated with the window.
         * We have to create everything.
         */

        pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

        /* Allocate a window priv. */
        if (!(pDbeWindowPriv = (*pDbeScreenPriv->AllocWinPriv)(pScreen)))
        {
            return(BadAlloc);
        }

        /* Setup the window priv priv. */
        pDbeWindowPrivPriv = MI_DBE_WINDOW_PRIV_PRIV(pDbeWindowPriv);
        pDbeWindowPrivPriv->pDbeWindowPriv = pDbeWindowPriv;

        /* Fill out the window priv information. */
	pDbeWindowPriv->pWindow      = pWin;
	pDbeWindowPriv->width        = pWin->drawable.width;
	pDbeWindowPriv->height       = pWin->drawable.height;
	pDbeWindowPriv->x            = pWin->drawable.x;
	pDbeWindowPriv->y            = pWin->drawable.y;
        pDbeWindowPriv->idList.id    = bufId;
        pDbeWindowPriv->idList.prev  = NULL;
        pDbeWindowPriv->idList.next  = NULL;
        pDbeWindowPriv->nBufferIDs   = 0;
        pDbeWindowPriv->pPixmapFront = 0;
        pDbeWindowPriv->pPixmapBack  = 0;

        /* Get a front pixmap. */
        if (!(pDbeWindowPriv->pPixmapFront = (*pScreen->CreatePixmap)(pScreen,
            pDbeWindowPriv->width, pDbeWindowPriv->height,
            pWin->drawable.depth)))
        {
            xfree(pDbeWindowPriv);
            return(BadAlloc);
        }

        /* Get a back pixmap. */
        if (!(pDbeWindowPriv->pPixmapBack = (*pScreen->CreatePixmap)(pScreen,
            pDbeWindowPriv->width, pDbeWindowPriv->height,
            pWin->drawable.depth)))
        {
            (*pScreen->DestroyPixmap)(pDbeWindowPriv->pPixmapFront); 
            xfree(pDbeWindowPriv);
            return(BadAlloc);
        }

        /* Make the window priv a DBE window priv resource. */
        if (!AddResource(bufId, dbeWindowPrivResType, (pointer)pDbeWindowPriv))
        {
            (*pScreen->DestroyPixmap)(pDbeWindowPriv->pPixmapFront); 
            (*pScreen->DestroyPixmap)(pDbeWindowPriv->pPixmapBack); 
            xfree(pDbeWindowPriv);
            return(BadAlloc);
        }

        /* Make the back pixmap a DBE drawable resource. */
        if (!AddResource(bufId, dbeDrawableResType,
            (pointer)pDbeWindowPriv->pPixmapBack))
        {
            /* free the buffer and the drawable resource */
            FreeResource(bufId, 0);
            return(BadAlloc);
        }

	pWin->devPrivates[dbeWindowPrivIndex].ptr = (pointer)pDbeWindowPriv;

	pDbeWindowPriv->devPrivates[miDbeWindowPrivPrivIndex].ptr =
            (pointer)pDbeWindowPrivPriv;


        /* Clear the back buffer. */
        pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);
        if ((*pDbeScreenPriv->SetupBackgroundPainter)(pWin, pGC))
        {
            ValidateGC(pDbeWindowPriv->pPixmapBack, pGC);
            clearRect.x = clearRect.y = 0;
            clearRect.width  = pDbeWindowPriv->pPixmapBack->drawable.width;
            clearRect.height = pDbeWindowPriv->pPixmapBack->drawable.height;
            (*pGC->ops->PolyFillRect)(pDbeWindowPriv->pPixmapBack, pGC, 1,
                                      &clearRect);
        }
        FreeScratchGC(pGC);

    } /* if ((pDbeWindowPriv = DBE_WINDOW_PRIV(pWin)) == NULL) */

    else
    {
        /* A buffer is already associated with the window.
         * Place the new buffer ID information at the head of the ID list.
         */

        DbeBufferIdPtr	pDbeBufferId;


        /* Allocate a new ID node. */
	if (!(pDbeBufferId = (DbeBufferIdPtr)xalloc(sizeof(DbeBufferIdRec))))
        {
            return(BadAlloc);
        }

        /* Associate the new ID with an existing pixmap. */
        if (!AddResource(bufId, dbeDrawableResType,
                         (pointer)pDbeWindowPriv->pPixmapBack))
        {
            xfree(pDbeBufferId);
            return(BadAlloc);
        }

        /* Associate the new ID with an existing window priv. */
        if (!AddResource(bufId, dbeWindowPrivResType, (pointer)pDbeWindowPriv))
        {
            xfree(pDbeBufferId);
            return(BadAlloc);
        }

        /* Copy the first ID node's data.  Fix the prev pointer.
         * Remember, the head of the list is a record in the window priv.
         */
        pDbeBufferId->id   = pDbeWindowPriv->idList.id;
        pDbeBufferId->prev = &pDbeWindowPriv->idList;
        pDbeBufferId->next = pDbeWindowPriv->idList.next;

        /* Fill in our new first node data. */
        pDbeWindowPriv->idList.id   = bufId;
        pDbeWindowPriv->idList.prev = NULL;
        pDbeWindowPriv->idList.next = pDbeBufferId;

        /* Fix the third node's prev pointer, if that node exists. */
        if (pDbeBufferId->next)
        {
            pDbeBufferId->next->prev = pDbeBufferId;
        }
    }

    /* Increment the number of buffers (XIDs) associated with this window. */
    pDbeWindowPriv->nBufferIDs++;

    /* Set swap action on all calls. */
    pDbeWindowPriv->swapAction = swapAction;

    return(Success);

} /* miDbeAllocBackBufferName() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeSwapBuffers
 *
 * Description:
 *
 *     This is the MI function for the DbeSwapBuffers request.
 *
 *****************************************************************************/

static int
miDbeSwapBuffers(
    ClientPtr		client,
    int			nWindows,
    WindowPtr		*pWins,
    xDbeSwapAction	*actions)
{
    DbeScreenPrivPtr    pDbeScreenPriv;
    GCPtr	    	pGC;
    WindowPtr	    	pWin;
    register int	i;
    DbeWindowPrivPtr	pDbeWindowPriv; 
    PixmapPtr           pTempPixmap;
    xRectangle          clearRect;


    for (i = 0; i < nWindows; i++)
    {
        pWin = pWins[i];
        pDbeScreenPriv = DBE_SCREEN_PRIV_FROM_WINDOW(pWin);
        pDbeWindowPriv = DBE_WINDOW_PRIV(pWin);
        pGC = GetScratchGC(pWin->drawable.depth, pWin->drawable.pScreen);

        /*
         **********************************************************************
         **
         ** Setup before swap.
         **
         **********************************************************************
         */

        switch(actions[i])
        {
            case XdbeUndefined:
                break;

            case XdbeBackground:
                break;

            case XdbeUntouched:
                ValidateGC(pDbeWindowPriv->pPixmapFront, pGC);
                (*pGC->ops->CopyArea)(pWin, pDbeWindowPriv->pPixmapFront, pGC,
                    0, 0, pWin->drawable.width, pWin->drawable.height, 0, 0);
                break;

            case XdbeCopied:
                break;

        } /* switch(actions[i]) */

        /*
         **********************************************************************
         **
         ** Swap.
         **
         **********************************************************************
         */

        ValidateGC(pWin, pGC);
        (*pGC->ops->CopyArea)(pDbeWindowPriv->pPixmapBack, pWin, pGC, 0, 0,
            pWin->drawable.width, pWin->drawable.height, 0, 0);

        /*
         **********************************************************************
         **
         ** Setup after swap.
         **
         **********************************************************************
         */

        switch(actions[i])
        {
            case XdbeUndefined:
                break;

            case XdbeBackground:
                if ((*pDbeScreenPriv->SetupBackgroundPainter)(pWin, pGC))
                {
                    ValidateGC(pDbeWindowPriv->pPixmapBack, pGC);
                    clearRect.x = 0;
                    clearRect.y = 0;
                    clearRect.width =
                        pDbeWindowPriv->pPixmapBack->drawable.width;
                    clearRect.height =
                        pDbeWindowPriv->pPixmapBack->drawable.height;
                    (*pGC->ops->PolyFillRect)(pDbeWindowPriv->pPixmapBack, pGC,
                        1, &clearRect);
                }
                break;

            case XdbeUntouched:
                /* Swap pixmap pointers. */
                pTempPixmap                  = pDbeWindowPriv->pPixmapBack;
                pDbeWindowPriv->pPixmapBack  = pDbeWindowPriv->pPixmapFront;
                pDbeWindowPriv->pPixmapFront = pTempPixmap;

                miDbeAliasBuffers(pDbeWindowPriv);

                break;

            case XdbeCopied:
                break;

        } /* switch(actions[i]) */

        FreeScratchGC(pGC);

    } /* for (i = 0; i < nWindows; i++) */

    return(Success);

} /* miSwapBuffers() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeBeginIdiom
 *
 * Description:
 *
 *     This is the MI function for the DbeBeginIdiom request.
 *     This function is a NOOP since we can not take advantage of hardware.
 *
 *****************************************************************************/

static void
miDbeBeginIdiom(
    ClientPtr	client)
{

} /* miDbeBeginIdiom() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeEndIdiom
 *
 * Description:
 *
 *     This is the MI function for the DbeEndIdiom request.
 *     This function is a NOOP since we can not take advantage of hardware.
 *
 *****************************************************************************/

static void
miDbeEndIdiom(
    ClientPtr	client)
{

} /* miDbeEndIdiom() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeWinPrivDelete
 *
 * Description:
 *
 *     This is the MI function for deleting the dbeWindowPrivResType resource.
 *     This function is invoked indirectly by calling FreeResource() to free
 *     the resources associated with a DBE buffer ID.  There are 5 ways that
 *     miDbeWinPrivDelete() can be called by FreeResource().  They are:
 *
 *     - A DBE window is destroyed, in which case the DbeDestroyWindow()
 *       wrapper is invoked.  The wrapper calls FreeResource() for all DBE
 *       buffer IDs.
 *
 *     - miDbeAllocBackBufferName() calls FreeResource() to clean up resources
 *       after a buffer allocation failure.
 *
 *     - The PositionWindow wrapper, miDbePositionWindow(), calls
 *       FreeResource() when it fails to create buffers of the new size.
 *       FreeResource() is called for all DBE buffer IDs.
 *
 *     - FreeClientResources() calls FreeResource() when a client dies or the
 *       the server resets.
 *
 *     When FreeResource() is called for a DBE buffer ID, the delete function
 *     for the only other type of DBE resource, dbeDrawableResType, is also
 *     invoked.  This delete function (DbeDrawableDelete) is a NOOP to make
 *     resource deletion easier.  It is not guaranteed which delete function is
 *     called first.  Hence, we will let miDbeWinPrivDelete() free all DBE
 *     resources.
 *     
 *     This function deletes/frees the following stuff associated with
 *     the window private:
 *
 *     - the ID node in the ID list representing the passed in ID.
 *
 *     In addition, pDbeWindowPriv->nBufferIDs is decremented.
 *
 *     If this function is called for the last/only buffer ID for a window,
 *     these are additionally deleted/freed:
 *
 *     - the front and back pixmaps
 *     - the window priv itself
 *
 *****************************************************************************/

static void
miDbeWinPrivDelete(
    DbeWindowPrivPtr	pDbeWindowPriv,
    XID			bufId)
{
    DbeBufferIdPtr		pID, pDbeBufferId = NULL;
    MiDbeWindowPrivPrivPtr	pDbeWindowPrivPriv;


    /*
     **************************************************************************
     ** Find the ID on the ID list.
     **************************************************************************
     */

    pID = &pDbeWindowPriv->idList;

    while (pID)
    {
        if (pID->id == bufId)
        {
            pDbeBufferId = pID;
            break;
        }
        pID = pID->next;
    }

    if (pDbeBufferId == NULL)
    {
        /* This should never happen. */
        return;
    } 

    /*
     **************************************************************************
     ** Remove the buffer name from the ID list.
     **************************************************************************
     */

    if (pDbeBufferId->prev == NULL)
    {
        /* Node to delete is head of list.  Remember, this is a record in the
         * DBE window private, not a pointer.  It can not be freed.
         */

        if (pDbeWindowPriv->idList.next == NULL)
        {
            /* This is the only node in the list. */
            pDbeWindowPriv->idList.id = (XID)0;
        }
        else
        {
            /* There are at least two nodes in the list. */

            /* Remember the second node, which we will free below. */
            pID = pDbeWindowPriv->idList.next;

            if (pDbeWindowPriv->idList.next->next)
            {
                /* Fix the third node's prev pointer, if there is one. */
                pDbeWindowPriv->idList.next->next->prev =
                     &pDbeWindowPriv->idList;
            }

            /* Copy the second node to the ID record in the window private. */
            pDbeWindowPriv->idList.id   = pDbeWindowPriv->idList.next->id;
            pDbeWindowPriv->idList.next = pDbeWindowPriv->idList.next->next;

            /* Free the second node since we copied it to the head of the
             * list.
             */
            xfree(pID);
        }
    }
    else
    {
        /* Node to delete is somewhere after the beginning of the list. */
        pDbeBufferId->prev->next = pDbeBufferId->next;

        if (pDbeBufferId->next)
        {
            /* Correct next node's pointers if there is a next node. */
            pDbeBufferId->next->prev = pDbeBufferId->prev;
        }

        xfree(pDbeBufferId);
    }

    /*
     **************************************************************************
     ** Decrement the number of buffer IDs for this window.
     **************************************************************************
     */

    pDbeWindowPriv->nBufferIDs--;

    if (pDbeWindowPriv->nBufferIDs != 0)
    {
        /* We still have at least one more buffer ID associated with this
         * window.
         */
        return;
    }

    /*
     **************************************************************************
     ** We have no more buffer IDs associated with this window.  We need to
     ** free some more stuff.
     **************************************************************************
     */

    /* Destroy the front and back pixmaps. */
    if (pDbeWindowPriv->pPixmapFront)
    {
        (*pDbeWindowPriv->pWindow->drawable.pScreen->DestroyPixmap)(
            pDbeWindowPriv->pPixmapFront);
    }
    if (pDbeWindowPriv->pPixmapBack)
    {
        (*pDbeWindowPriv->pWindow->drawable.pScreen->DestroyPixmap)(
            pDbeWindowPriv->pPixmapBack);
    }

    /* Reset the DBE window priv pointer. */
    pDbeWindowPriv->pWindow->devPrivates[dbeWindowPrivIndex].ptr =
        (pointer)NULL;

    /* Free the window priv. */
    xfree(pDbeWindowPriv);

} /* miDbeWinPrivDelete() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbePositionWindow
 *
 * Description:
 *
 *     This function was cloned from miMbxPositionWindow() in mimultibuf.c. 
 *     This function resizes the buffer when the window is resized.
 *
 *****************************************************************************/

static Bool
miDbePositionWindow(
    WindowPtr	pWin,
    int		x,
    int		y)
{
    ScreenPtr			pScreen;
    DbeScreenPrivPtr		pDbeScreenPriv;
    DbeWindowPrivPtr		pDbeWindowPriv;
    int				width, height;
    int				dx, dy, dw, dh;
    int				sourcex, sourcey;
    int				destx, desty;
    int				savewidth, saveheight;
    PixmapPtr			pPixmapFront;
    PixmapPtr			pPixmapBack;
    Bool			clear;
    GCPtr			pGC;
    xRectangle			clearRect;
    Bool			ret;


    /*
     **************************************************************************
     **
     ** 1. Unwrap the member routine.
     **
     **************************************************************************
     */
     
    pScreen                 = pWin->drawable.pScreen;
    pDbeScreenPriv          = DBE_SCREEN_PRIV(pScreen);
    pScreen->PositionWindow = pDbeScreenPriv->PositionWindow;

    /*
     **************************************************************************
     **
     ** 2. Do any work necessary before the member routine is called.
     **
     **    In this case we do not need to do anything.
     **
     **************************************************************************
     */
     
    /*
     **************************************************************************
     **
     ** 3. Call the member routine, saving its result if necessary.
     **
     **************************************************************************
     */
     
    ret = (*pScreen->PositionWindow)(pWin, x, y);

    /*
     **************************************************************************
     **
     ** 4. Do any work necessary after the member routine has been called.
     **
     **************************************************************************
     */
     
    if (!(pDbeWindowPriv = DBE_WINDOW_PRIV(pWin)))
    {
        /* Rewrap the member routine. */
        pScreen->PositionWindow = miDbePositionWindow;

	return(ret);
    }

    if (pDbeWindowPriv->width  == pWin->drawable.width &&
        pDbeWindowPriv->height == pWin->drawable.height)
    {
        /* Rewrap the member routine. */
        pScreen->PositionWindow = miDbePositionWindow;

	return(ret);
    }

    width  = pWin->drawable.width;
    height = pWin->drawable.height;

    dx = pWin->drawable.x - pDbeWindowPriv->x;
    dy = pWin->drawable.y - pDbeWindowPriv->y;
    dw = width  - pDbeWindowPriv->width;
    dh = height - pDbeWindowPriv->height;

    GravityTranslate (0, 0, -dx, -dy, dw, dh, pWin->bitGravity, &destx, &desty);

    clear = pDbeWindowPriv->width < width || pDbeWindowPriv->height < height ||
	    pWin->bitGravity == ForgetGravity;

    sourcex = 0;
    sourcey = 0;
    savewidth  = pDbeWindowPriv->width;
    saveheight = pDbeWindowPriv->height;

    /* Clip rectangle to source and destination. */
    if (destx < 0)
    {
	savewidth += destx;
	sourcex   -= destx;
	destx      = 0;
    }

    if (destx + savewidth > width)
    {
	savewidth = width - destx;
    }

    if (desty < 0)
    {
	saveheight += desty;
	sourcey    -= desty;
	desty       = 0;
    }

    if (desty + saveheight > height)
    {
	saveheight = height - desty;
    }

    pDbeWindowPriv->width  = width;
    pDbeWindowPriv->height = height;
    pDbeWindowPriv->x = pWin->drawable.x;
    pDbeWindowPriv->y = pWin->drawable.y;

    pGC = GetScratchGC (pWin->drawable.depth, pScreen);

    if (clear)
    {
	if ((*pDbeScreenPriv->SetupBackgroundPainter)(pWin, pGC))
	{
	    clearRect.x = 0;
	    clearRect.y = 0;
	    clearRect.width  = width;
	    clearRect.height = height;
	}
	else
	{ 
	    clear = FALSE;
	}
    }

    /* Create DBE buffer pixmaps equal to size of resized window. */
    pPixmapFront = (*pScreen->CreatePixmap)(pScreen, width, height,
        pWin->drawable.depth);

    pPixmapBack = (*pScreen->CreatePixmap)(pScreen, width, height,
        pWin->drawable.depth);

    if (!pPixmapFront || !pPixmapBack)
    {
        /* We failed at creating 1 or 2 of the pixmaps. */

        if (pPixmapFront)
        {
	    (*pScreen->DestroyPixmap)(pPixmapFront);
        }

        if (pPixmapBack)
        {
	    (*pScreen->DestroyPixmap)(pPixmapBack);
        }

        /* Destroy all buffers for this window. */
        while (pDbeWindowPriv)
        {
            /* miDbeWinPrivDelete() will free the window private if there no
             * more buffer IDs associated with this window.
             */
            FreeResource(pDbeWindowPriv->idList.id, 0);
            pDbeWindowPriv = DBE_WINDOW_PRIV(pWin);
        }

        FreeScratchGC(pGC);

        /* Rewrap the member routine. */
        pScreen->PositionWindow = miDbePositionWindow;

        return(FALSE);
    }

    else
    {
        /* Clear out the new DBE buffer pixmaps. */

        ValidateGC(pPixmapFront, pGC);

	/* I suppose this could avoid quite a bit of work if
	 * it computed the minimal area required.
	 */
	if (clear)
        {
	    (*pGC->ops->PolyFillRect)(pPixmapFront, pGC, 1, &clearRect);
	    (*pGC->ops->PolyFillRect)(pPixmapBack , pGC, 1, &clearRect);
        }

        /* Copy the contents of the old DBE pixmaps to the new pixmaps. */
	if (pWin->bitGravity != ForgetGravity)
	{
	    (*pGC->ops->CopyArea)(pDbeWindowPriv->pPixmapFront, pPixmapFront,
                pGC, sourcex, sourcey, savewidth, saveheight, destx, desty);
	    (*pGC->ops->CopyArea)(pDbeWindowPriv->pPixmapBack, pPixmapBack,
                pGC, sourcex, sourcey, savewidth, saveheight, destx, desty);
	}

        /* Destroy the old pixmaps, and point the DBE window priv to the new
         * pixmaps.
         */

	(*pScreen->DestroyPixmap)(pDbeWindowPriv->pPixmapFront);
	(*pScreen->DestroyPixmap)(pDbeWindowPriv->pPixmapBack);

        pDbeWindowPriv->pPixmapFront = pPixmapFront;
        pDbeWindowPriv->pPixmapBack  = pPixmapBack;

	/* Make sure all XID are associated with the new back pixmap. */
        miDbeAliasBuffers(pDbeWindowPriv);

        FreeScratchGC(pGC);
    }

    /*
     **************************************************************************
     **
     ** 5. Rewrap the member routine.
     **
     **************************************************************************
     */
     
    pScreen->PositionWindow = miDbePositionWindow;
    
    return(ret);

} /* miDbePositionWindow() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeAliasBuffers
 *
 * Description:
 *
 *     This function associates all XIDs of a buffer with the back pixmap
 *     stored in the window priv.
 *
 *****************************************************************************/

static void
miDbeAliasBuffers(
    DbeWindowPrivPtr	pDbeWindowPriv)
{
    DbeBufferIdPtr	pDbeBufferId = &pDbeWindowPriv->idList;


    while (pDbeBufferId)
    {
        ChangeResourceValue(pDbeBufferId->id, dbeDrawableResType,
                            (pointer)pDbeWindowPriv->pPixmapBack);
        pDbeBufferId = pDbeBufferId->next;
    }

} /* miDbeAliasBuffers() */


/******************************************************************************
 *
 * DBE MI Procedure: miDbeResetProc
 *
 * Description:
 *
 *     This function is called from DbeResetProc(), which is called at the end
 *     of every server generation.  This function peforms any MI-specific
 *     shutdown tasks.
 *
 *****************************************************************************/

static void
miDbeResetProc(
    ScreenPtr	pScreen)
{
    DbeScreenPrivPtr    pDbeScreenPriv;


    pDbeScreenPriv = DBE_SCREEN_PRIV(pScreen);

    /* Unwrap wrappers */
    pScreen->PositionWindow = pDbeScreenPriv->PositionWindow;

} /* miDbeResetProc() */

