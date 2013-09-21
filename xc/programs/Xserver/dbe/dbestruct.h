/* $XConsortium: dbestruct.h,v 1.2 95/06/07 20:19:57 dpw Exp $ */
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
 * $Header: /x/programs/Xserver/dbe/RCS/dbestruct.h,v 1.2 95/06/07 20:19:57 dpw Exp $
 *
 *     Header file for DIX-related DBE
 *
 *****************************************************************************/


#ifndef DBE_STRUCT_H
#define DBE_STRUCT_H


/* INCLUDES */

#define NEED_DBE_PROTOCOL
#include "Xdbeproto.h"
#include "windowstr.h"


/* DEFINES */

#define DBE_SCREEN_PRIV(pScreen) \
    ((dbeScreenPrivIndex < 0) ? \
     NULL : \
     ((DbeScreenPrivPtr)((pScreen)->devPrivates[dbeScreenPrivIndex].ptr)))

#define DBE_SCREEN_PRIV_FROM_DRAWABLE(pDrawable) \
    DBE_SCREEN_PRIV((pDrawable)->pScreen)

#define DBE_SCREEN_PRIV_FROM_WINDOW_PRIV(pDbeWindowPriv) \
    DBE_SCREEN_PRIV((pDbeWindowPriv)->pWindow->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_WINDOW(pWindow) \
    DBE_SCREEN_PRIV((pWindow)->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_PIXMAP(pPixmap) \
    DBE_SCREEN_PRIV((pPixmap)->drawable.pScreen)

#define DBE_SCREEN_PRIV_FROM_GC(pGC)\
    DBE_SCREEN_PRIV((pGC)->pScreen)

#define DBE_WINDOW_PRIV(pWindow)\
    ((dbeWindowPrivIndex < 0) ? \
     NULL : \
     ((DbeWindowPrivPtr)(pWindow->devPrivates[dbeWindowPrivIndex].ptr)))


/* TYPEDEFS */

/*
 ******************************************************************************
 **
 ** Per-window data
 **
 ******************************************************************************
 */

typedef struct _DbeBufferIdRec
{
    XID				id;
    struct _DbeBufferIdRec	*prev; 
    struct _DbeBufferIdRec	*next; 

} DbeBufferIdRec, *DbeBufferIdPtr;

typedef struct _DbeWindowPrivRec
{
    /* A pointer to the window with which the DBE window private (buffer) is
     * associated.
     */
    WindowPtr		pWindow;

    /* Last known swap action for this buffer.  Legal values for this field
     * are XdbeUndefined, XdbeBackground, XdbeUntouched, and XdbeCopied.
     */
    unsigned char	swapAction;

    /* Last known buffer size.
     */
    unsigned short	width, height;

    /* Coordinates used for static gravity when the window is positioned.
     */
    short		x, y;

    /* List of XIDs associated with this buffer.  We are defining the XID list
     * as a record and not a pointer here to optimize for data locality.  In
     * most cases, only one buffer will be associated with a window.  Having a
     * record declared here can prevent us accessing the data in another memory
     * page, possibly resulting in a page swap and loss of performance.  For
     * situations where there is more than one buffer, we will possibly have to
     * access those XIDs on a different data page.  This situation could be
     * optimized by performing chunk allocations of memory specifically to be
     * used for XID list nodes.
     */
    DbeBufferIdRec	idList;

    /* Number of XIDs associated with this buffer.
     */
    int			nBufferIDs;

    /* Pointer to a pixmap that contains the contents of the back buffer.
     * This pointer is used for software buffering only.
     */
    PixmapPtr		pPixmapBack;

    /* Pointer to a pixmap that contains the contents of the front buffer.
     * This pointer is used for software buffering only.  Specifically, the
     * front pixmap is used for the XdbeUntouched swap action.  For that swap
     * action, we need to copy the front buffer (window) contents into this
     * pixmap, copy the contents of current back buffer pixmap (the back
     * buffer) into the window, swap the front and back pixmap pointers, and
     * then swap the pixmap/resource associations in the resource database.
     */
    PixmapPtr		pPixmapFront;

    /* Device-specific private information.
     */
    DevUnion		*devPrivates;

} DbeWindowPrivRec, *DbeWindowPrivPtr;


/*
 ******************************************************************************
 **
 ** Per-screen data
 **
 ******************************************************************************
 */

typedef struct _DbeScreenPrivRec
{
    /* Info for creating window privs */
    int          winPrivPrivLen;    /* Length of privs in DbeWindowPrivRec   */
    unsigned int *winPrivPrivSizes; /* Array of private record sizes         */
    unsigned int totalWinPrivSize;  /* PrivRec + size of all priv priv ptrs  */

    /* Info for creating screen privs */
    int          scrnPrivPrivLen;    /* Length of privs in DbeScreenPrivRec  */
    unsigned int *scrnPrivPrivSizes; /* Array of private record sizes        */
    unsigned int totalScrnPrivSize;  /* PrivRec + size of all priv priv ptrs */

    /* Resources created by DIX to be used by DDX */
    RESTYPE	dbeDrawableResType;
    RESTYPE	dbeWindowPrivResType;

    /* Private indices created by DIX to be used by DDX */
    int		dbeScreenPrivIndex;
    int		dbeWindowPrivIndex;

    /* Wrapped functions
     * It is the responsibilty of the DDX layer to wrap PositionWindow().
     * DbeExtensionInit wraps DestroyWindow().
     */
    Bool	(*PositionWindow)();	/* pWin, x, y */
    Bool	(*DestroyWindow)();	/* pWin */

    /* Per-screen DIX routines */
    Bool	     (*SetupBackgroundPainter)();  /* pWin, pGC              */
    DbeWindowPrivPtr (*AllocWinPriv)();            /* pScreen                */
    int              (*AllocWinPrivPrivIndex)();   /*                        */
    Bool             (*AllocWinPrivPriv)();        /* pScreen, index, amount */
    struct _DbeScreenPrivRec *(*AllocScrnPriv)();  /* pDbeScreenPriv         */
    int              (*AllocScrnPrivPrivIndex)();  /*                        */
    Bool             (*AllocScrnPrivPriv)();       /* pScreen, index, amount */

    /* Per-screen DDX routines */
    Bool	(*GetVisualInfo)();		  /* pScreen, pVisInfo       */
    int		(*AllocBackBufferName)();	  /* pWin, bufId, swapAction */
    int		(*SwapBuffers)();		  /* client, nWindows, pWins,*/
                                                  /* actions                 */
    void	(*BeginIdiom)();		  /* client                  */
    void	(*EndIdiom)();		          /* client                  */
    void	(*WinPrivDelete)();		  /* pDbeWindowPriv, bufID   */
    void	(*ResetProc)();		          /* pScreen                 */

    /* Device-specific private information.
     */
    DevUnion	*devPrivates;

} DbeScreenPrivRec, *DbeScreenPrivPtr;


/* EXTERNS */


#endif /* DBE_STRUCT_H */
