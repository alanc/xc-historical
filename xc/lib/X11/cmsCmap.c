/* $XConsortium: XcmsCmap.c,v 1.13 91/01/27 00:36:30 alt Exp $" */

/*
 * (c) Copyright 1990 1991 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		XcmsCmap.c - Client Colormap Management Routines
 *
 *	DESCRIPTION
 *		Routines that store additional information about
 *		colormaps being used by the X Client.
 *
 *
 */



/*
 *      EXTERNAL INCLUDES
 */
#include "Xlib.h"
#include "TekCMS.h"
#include "TekCMSP.h"

/*
 *      INTERNAL INCLUDES
 */
#include "Xlibos.h"
#include "Xutil.h"
#include "Xproto.h"

/*
 *      EXTERNS
 */
extern XcmsCCC *XcmsCreateCCC();

/*
 *      FORWARD DECLARATIONS
 */
XcmsCmapRec *_XcmsAddCmapRec();

/*
 *      LOCAL VARIABLES
 */
static unsigned char CreateWindowStatus;



/************************************************************************
 *									*
 *			PRIVATE INTERFACES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CreateWindowErrorHandler
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
static int
CreateWindowErrorHandler(dpy, errorp)
    Display *dpy;
    XErrorEvent *errorp;
/*
 *	DESCRIPTION
 *		Error Hander used in CmapRecForColormap() to catch
 *		errors occuring when creating a window with a foreign
 *		colormap.
 *
 *	RETURNS
 *		1
 *
 */
{
    if (errorp->request_code == X_CreateWindow) {
	CreateWindowStatus = errorp->error_code;
    }
    return(1);
}	


/*
 *	NAME
 *		CmapRecForColormap
 *
 *	SYNOPSIS
 */
static XcmsCmapRec *
CmapRecForColormap(dpy, cmap)
    Display *dpy;
    Colormap cmap;
/*
 *	DESCRIPTION
 *		Find the corresponding XcmsCmapRec for cmap.  In not found
 *		this routines attempts to create one.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the corresponding XcmsCmapRec.
 *
 */
{
    XcmsCmapRec *pRec;
    int nScrn;
    int i, j;
    XVisualInfo visualTemplate;	/* Template of the visual we want */
    XVisualInfo *visualList;	/* List for visuals that match */
    int nVisualsMatched;	/* Number of visuals that match */
    XSetWindowAttributes windowAttr;
    Window tmpWindow;
    int (*oldErrorHandler)();

    for (pRec = (XcmsCmapRec *)dpy->cms.clientCmaps; pRec != NULL;
	    pRec = pRec->pNext) {
	if (pRec->cmapID == cmap) {
	    return(pRec);
	}
    }

    /*
     * Can't find an XcmsCmapRec associated with cmap in our records.
     * Let's try to see if its a default colormap
     */
    nScrn = ScreenCount(dpy);
    for (i = 0; i < nScrn; i++) {
	if (cmap == DefaultColormap(dpy, i)) {
	    /* It is ... lets go ahead and store that info */
	    if ((pRec = _XcmsAddCmapRec(dpy, cmap, RootWindow(dpy, i),
		    DefaultVisual(dpy, i))) == NULL) {
		return((XcmsCmapRec *)NULL);
	    }
	    pRec->pCCC = XcmsCreateCCC(
		    dpy,
		    i,			/* screen_number */
		    DefaultVisual(dpy, i),
		    (XcmsColor *)NULL,	/* clientWhitePt */
		    (XcmsFuncPtr *)NULL,/* gamutCompFunc */
		    (caddr_t)NULL,	/* gamutCompClientData */
		    (XcmsFuncPtr *)NULL,/* whitePtAdjFunc */
		    (caddr_t)NULL	/* whitePtAdjClientData */
		    );
	    return(pRec);
	}
    }

    /*
     * Nope, its not a default colormap, so it's probably a foreign color map
     * of which we have no specific details.  Let's go through the
     * rigorous process of finding this colormap:
     *        for each screen
     *            for each screen's visual types
     *                create a window
     *                attempt to set the window's colormap to cmap
     *                if successful
     *                    Add a CmapRec
     *                    Create an XcmsCCC
     *                    return the CmapRec
     *                else
     *                    continue
     */

    /*
     * Before we start into this ugly process, let's sync up.
     */
    XSync(dpy, False);

    /*
     * Setup Temporary Error Handler
     */
    oldErrorHandler = XSetErrorHandler(CreateWindowErrorHandler);

    for (i = 0; i < nScrn; i++) {
	visualTemplate.screen = i;
	visualList = XGetVisualInfo(dpy, VisualScreenMask, &visualTemplate,
	    &nVisualsMatched);
	if (nVisualsMatched == 0) {
	    continue;
	}

	j = 0;
	windowAttr.colormap = cmap;

	/*
	 * Call routine that attempts to create a window with cmap
	 */
	do {
	    CreateWindowStatus = Success;
	    tmpWindow = XCreateWindow(dpy, RootWindow(dpy, i),
		    0, 0, 5, 5, 1,
		    (visualList+j)->depth, 
		    CopyFromParent,
		    (visualList+j)->visual, 
		    CWColormap,
		    &windowAttr);
	    XSync(dpy, False);
	} while (CreateWindowStatus != Success && ++j < nVisualsMatched);

	/*
	 * if successful
	 */
	if (CreateWindowStatus == Success) {
	    if ((pRec = _XcmsAddCmapRec(dpy, cmap, tmpWindow,
		    (visualList+j)->visual)) == NULL) {
		return((XcmsCmapRec *)NULL);
	    }
	    pRec->pCCC = XcmsCreateCCC(
		    dpy,
		    i,			/* screen_number */
		    (visualList+j)->visual,
		    (XcmsColor *)NULL,	/* clientWhitePt */
		    (XcmsFuncPtr *)NULL,/* gamutCompFunc */
		    (caddr_t)NULL,	/* gamutCompClientData */
		    (XcmsFuncPtr *)NULL,/* whitePtAdjFunc */
		    (caddr_t)NULL	/* whitePtAdjClientData */
		    );
	    XSetErrorHandler(oldErrorHandler);
	    XDestroyWindow(dpy, tmpWindow);
	    XFree((caddr_t)visualList);
	    return(pRec);
	}

	/*
	 * Otherwise continue ....
	 */
	XFree((caddr_t)visualList);
    }

    /*
     * Unsetup Temporary Error Handler
     */
    XSetErrorHandler(oldErrorHandler);

    return(NULL);
}



/************************************************************************
 *									*
 *			API PRIVATE INTERFACES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsAddCmapRec
 *
 *	SYNOPSIS
 */
XcmsCmapRec *
_XcmsAddCmapRec(dpy, cmap, windowID, visual)
    Display *dpy;
    Colormap cmap;
    Window windowID;
    Visual *visual;
/*
 *	DESCRIPTION
 *		Create an XcmsCmapRec for the specified cmap, windowID,
 *		and visual, then adds it to its list of CmapRec's.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the added XcmsCmapRec.
 *
 */
{
    XcmsCmapRec *pNew;

    if ((pNew = (XcmsCmapRec *) Xcalloc(1, (unsigned) sizeof(XcmsCmapRec)))
	    == NULL) {
	return((XcmsCmapRec *)NULL);
    }

    pNew->cmapID = cmap;
    pNew->dpy = dpy;
    pNew->windowID = windowID;
    pNew->visual = visual;
    pNew->pNext = (XcmsCmapRec *)dpy->cms.clientCmaps;
    dpy->cms.clientCmaps = (caddr_t)pNew;

    /*
     * Note, we don't create the XcmsCCC for pNew->pCCC here because
     * it may require the use of XGetWindowAttributes (a round trip request)
     * to determine the screen.
     */
    return(pNew);
}


/*
 *	NAME
 *		_XcmsCopyCmapRecAndFree
 *
 *	SYNOPSIS
 */
XcmsCmapRec *
_XcmsCopyCmapRecAndFree(dpy, src_cmap, copy_cmap)
    Display *dpy;
    Colormap src_cmap;
    Colormap copy_cmap;
/*
 *	DESCRIPTION
 *		Augments Xlib's XCopyColormapAndFree() to copy
 *		XcmsCmapRecs.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the copy XcmsCmapRec.
 *
 */
{
    XcmsCmapRec *pRec_src;
    XcmsCmapRec *pRec_copy;

    if ((pRec_src = CmapRecForColormap(dpy, src_cmap)) != NULL) {
	pRec_copy =_XcmsAddCmapRec(dpy, copy_cmap, pRec_src->windowID,
		pRec_src->visual);
	if (pRec_copy != NULL) {
	    pRec_copy->pCCC = (XcmsCCC *)Xcalloc(1, (unsigned) sizeof(XcmsCCC));
	    bcopy((char *)pRec_src->pCCC, (char *)pRec_copy->pCCC,
		    sizeof(XcmsCCC));
	}
	return(pRec_copy);
    }
    return((XcmsCmapRec *)NULL);
}


/*
 *	NAME
 *		_XcmsDeleteCmapRec
 *
 *	SYNOPSIS
 */
void
_XcmsDeleteCmapRec(dpy, cmap)
    Display *dpy;
    Colormap cmap;
/*
 *	DESCRIPTION
 *		Removes and frees the specified XcmsCmapRec structure
 *		from the linked list of structures.
 *
 *	RETURNS
 *		void
 *
 */
{
    XcmsCmapRec *pPrev;
    XcmsCmapRec *pNext;

    if (((XcmsCmapRec *)dpy->cms.clientCmaps)->cmapID == cmap) {
	/*
	 * Target CmapRec is at the head of the list.
	 */
	pNext = ((XcmsCmapRec *)dpy->cms.clientCmaps)->pNext;
	if (((XcmsCmapRec *)dpy->cms.clientCmaps)->pCCC) {
	    XcmsFreeCCC(((XcmsCmapRec *)dpy->cms.clientCmaps)->pCCC);
	}
	Xfree(dpy->cms.clientCmaps);
	dpy->cms.clientCmaps = (caddr_t)pNext;
	return;
    }

    /*
     * Target CmapRec is not at the head of the list.
     * Traverse the linked list until found (or end).
     */
    pPrev = (XcmsCmapRec *)dpy->cms.clientCmaps;
    while ((pPrev != NULL) && (pPrev->pNext->cmapID != cmap)) {
	    pPrev = pPrev->pNext;
    }
    if (pPrev != NULL) {
	/*
	 * We found it!
	 */
	pNext = pPrev->pNext->pNext;
	Xfree(pPrev->pNext);
	pPrev->pNext = pNext;
    }
}


/*
 *	NAME
 *		_XcmsFreeClientCmaps
 *
 *	SYNOPSIS
 */
void
_XcmsFreeClientCmaps(dpy)
    Display *dpy;
/*
 *	DESCRIPTION
 *		Frees all XcmsCmapRec structures in the linked list
 *		and sets dpy->cms.clientCmaps to NULL.
 *
 *	RETURNS
 *		void
 *
 */
{
    XcmsCmapRec *pRecNext, *pRecFree;

    pRecNext = (XcmsCmapRec *)dpy->cms.clientCmaps;
    while (pRecNext != NULL) {
	pRecFree = pRecNext;
	pRecNext = pRecNext->pNext;
	if (pRecFree->pCCC) {
	    /* Free the XcmsCCC structure */
	    XcmsFreeCCC(pRecFree->pCCC);
	}
	/* Now free the XcmsCmapRec structure */
	Xfree(pRecFree);
    }
    dpy->cms.clientCmaps = (caddr_t)NULL;
}



/************************************************************************
 *									*
 *			PUBLIC INTERFACES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCCCofColormap
 *
 *	SYNOPSIS
 */
XcmsCCC *
XcmsCCCofColormap(dpy, cmap)
    Display *dpy;
    Colormap cmap;
/*
 *	DESCRIPTION
 *		Finds the XcmsCCC associated with the specified colormap.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the associated XcmsCCC structure.
 *
 */
{
    XWindowAttributes windowAttr;
    XcmsCmapRec *pRec;
    int nScrn = ScreenCount(dpy);
    int i;

    if ((pRec = CmapRecForColormap(dpy, cmap)) != NULL) {
	if (pRec->pCCC) {
	    /* XcmsCmapRec already has a XcmsCCC */
	    return(pRec->pCCC);
	}

	/*
	 * The XcmsCmapRec does not have a XcmsCCC yet, so let's create
	 * one.  But first, we need to know the screen associated with
	 * cmap, so use XGetWindowAttributes() to extract that
	 * information.  Unless, of course there is only one screen!!
	 */
	if (nScrn == 1) {
	    /* Assume screen_number == 0 */
	    return(pRec->pCCC = XcmsCreateCCC(
		    dpy,
		    0,			/* screen_number */
		    pRec->visual,
		    (XcmsColor *)NULL,	/* clientWhitePt */
		    (XcmsFuncPtr *)NULL,/* gamutCompFunc */
		    (caddr_t)NULL,	/* gamutCompClientData */
		    (XcmsFuncPtr *)NULL,/* whitePtAdjFunc */
		    (caddr_t)NULL	/* whitePtAdjClientData */
		    ));
	} else {
	    if (XGetWindowAttributes(dpy, pRec->windowID, &windowAttr)) {
		for (i = 0; i < nScrn; i++) {
		    if (ScreenOfDisplay(dpy, i) == windowAttr.screen) {
			return(pRec->pCCC = XcmsCreateCCC(
				dpy,
				i,			/* screen_number */
				pRec->visual,
				(XcmsColor *)NULL,	/* clientWhitePt */
				(XcmsFuncPtr *)NULL,/* gamutCompFunc */
				(caddr_t)NULL,	/* gamutCompClientData */
				(XcmsFuncPtr *)NULL,/* whitePtAdjFunc */
				(caddr_t)NULL	/* whitePtAdjClientData */
				));
		    }
		}
	    }
	}
    }

    /*
     * No such cmap
     */
    return(NULL);
}
