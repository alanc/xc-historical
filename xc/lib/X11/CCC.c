/* $XConsortium: XcmsCCC.c,v 1.6 91/02/12 16:12:26 dave Exp $" */

/*
 * Code and supporting documentation (c) Copyright 1990 1991 Tektronix, Inc.
 * 	All Rights Reserved
 * 
 * This file is a component of an X Window System-specific implementation
 * of Xcms based on the TekColor Color Management System.  Permission is
 * hereby granted to use, copy, modify, sell, and otherwise distribute this
 * software and its documentation for any purpose and without fee, provided
 * that this copyright, permission, and disclaimer notice is reproduced in
 * all copies of this software and in supporting documentation.  TekColor
 * is a trademark of Tektronix, Inc.
 * 
 * Tektronix makes no representation about the suitability of this software
 * for any purpose.  It is provided "as is" and with all faults.
 * 
 * TEKTRONIX DISCLAIMS ALL WARRANTIES APPLICABLE TO THIS SOFTWARE,
 * INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL TEKTRONIX BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA, OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR THE PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *	NAME
 *		XcmsCCC.c - Color Conversion Context Routines
 *
 *	DESCRIPTION
 *		Routines to create, access, and free Color Conversion
 *		Context structures.
 *
 *
 */

#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"



/************************************************************************
 *									*
 *			PUBLIC INTERFACES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCreateCCC
 *
 *	SYNOPSIS
 */

XcmsCCC 
XcmsCreateCCC(dpy, screenNumber, visual, clientWhitePt, gamutCompProc,
	gamutCompClientData, whitePtAdjProc, whitePtAdjClientData)
    Display *dpy;
    int	screenNumber;
    Visual *visual;
    XcmsColor *clientWhitePt;
    XcmsFuncPtr gamutCompProc;
    XPointer gamutCompClientData;
    XcmsFuncPtr whitePtAdjProc;
    XPointer whitePtAdjClientData;
/*
 *	DESCRIPTION
 *		Given a Display, Screen, Visual, etc., this routine creates
 *		an appropriate Color Conversion Context.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise address of the newly
 *		created XcmsCCC.
 *
 */
{
    XcmsCCC pDefaultCCC = XcmsDefaultCCC(dpy, screenNumber);
    XcmsCCC newccc;

    if (pDefaultCCC == NULL ||
	    !(newccc = (XcmsCCC) Xcalloc(1, (unsigned) sizeof(XcmsCCCRec)))) {
	return(NULL);
    } 

    /*
     * Should inherit the following as result of a bcopy():
     *		dpy
     *		screenNumber
     *		pPerScrnInfo
     */
    bcopy((char *)pDefaultCCC, (char *)newccc, sizeof(XcmsCCCRec));
    if (clientWhitePt) {
	bcopy((char *)clientWhitePt, (char *)&newccc->clientWhitePt,
		sizeof(XcmsColor));
    }
    if (gamutCompProc) {
	newccc->gamutCompProc = gamutCompProc;
    }
    if (gamutCompClientData) {
	newccc->gamutCompClientData = gamutCompClientData;
    }
    if (whitePtAdjProc) {
	newccc->whitePtAdjProc = whitePtAdjProc;
    }
    if (whitePtAdjClientData) {
	newccc->whitePtAdjClientData = whitePtAdjClientData;
    }

    /*
     * Set visual component
     */
    newccc->visual = visual;

    return(newccc);
}


/*
 *	NAME
 *		XcmsDefaultCCC
 *
 *	SYNOPSIS
 */
XcmsCCC 
XcmsDefaultCCC(dpy, screenNumber)
    Display *dpy;
    int screenNumber;
/*
 *	DESCRIPTION
 *		Given a Display and Screen, this routine creates
 *		returns the Screen's default Color Conversion Context.
 *		Note that a Screen's default CCC is built with the
 *		screen default visual.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise address of the
 *		XcmsCCC for the Screen's default CCC.
 *
 */
{
    XcmsCCC ccc;


    if ((screenNumber < 0) || (screenNumber >= ScreenCount(dpy))) {
	return((XcmsCCC)NULL);
    }

    /*
     * Check if the XcmsCCC's for each screen has been created
     */
    if ((XcmsCCC)dpy->cms.defaultCCCs == NULL) {
	if (!_XcmsInitDefaultCCCs(dpy)) {
	    return((XcmsCCC)NULL);
	}
    }

    ccc = (XcmsCCC)dpy->cms.defaultCCCs + screenNumber;

    if (!ccc->pPerScrnInfo) {
	/*
	 * Need to create the XcmsPerScrnInfo structure.  The
	 * _XcmsInitScrnInfo routine will create the XcmsPerScrnInfo
	 * structure as well as initialize its functionSet and pScreenData
	 * components.
	 */
	if (!_XcmsInitScrnInfo(dpy, screenNumber)) {
	    return((XcmsCCC)NULL);
	}
	return(ccc);
    } else {
	/*
	 * If ccc->pPerScrnInfo->state == XcmsInitSuccess,
	 *    then the pPerScrnInfo component has already been initialized
	 *    therefore, just return ccc.
	 * If ccc->pPerScrnInfo->state == XcmsInitDefault,
	 *    then this means that we already attempted to initialize
	 *    the pPerScrnInfo component but failed therefore stuffing
	 *    the pPerScrnInfo component with defaults.  Just return ccc.
	 * If ccc->pPerScrnInfo->state == XcmsInitNone,
	 *    then attempt to initialize the pPerScrnInfo component.
	 */
	switch (ccc->pPerScrnInfo->state) {
	   case XcmsInitDefault :
	    /* fall through */
	   case XcmsInitSuccess :
	    return(ccc);
	   case XcmsInitNone :
	    /* XcmsPerScreenInfo has not been initialized */
	    if (!_XcmsInitScrnInfo(dpy, screenNumber)) {
		return((XcmsCCC)NULL);
	    }
	    return(ccc);
	   default :
	    return((XcmsCCC)NULL);
	}
    }
}


/*
 *	NAME
 *		XcmsFreeCCC
 *
 *	SYNOPSIS
 */
void
XcmsFreeCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Frees memory associated with a Color Conversion Context
 *		that was created with XcmsCreateCCC().
 *
 *	RETURNS
 *		void
 *
 */
{
    if (ccc == XcmsDefaultCCC(ccc->dpy, ccc->screenNumber)) {
	/* do not allow clients to free DefaultCCC's */
	return;
    }

    Xfree(ccc);

    /*
     * Note that XcmsPerScrnInfo sub-structures are not freed here.
     * There is only one allocated per Screen and it just so happens
     * that we place its initial reference is placed in the default CCC.
     * The routine _XcmsFreeDefaultCCCs frees them.
     */
}
