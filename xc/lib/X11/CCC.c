/* $XConsortium: XcmsCCC.c,v 1.2 91/02/05 13:41:23 dave Exp $" */

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
 *		XcmsCCC.c - Color Conversion Context Routines
 *
 *	DESCRIPTION
 *		Routines to create, access, and free Color Conversion
 *		Context structures.
 *
 *
 */



/*
 *      EXTERNAL INCLUDES
 */
#include "TekCMS.h"

/*
 *      INTERNAL INCLUDES
 */
#include <stdio.h>
#include "Xlibos.h"
#include "TekCMSP.h"



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

XcmsCCC *
XcmsCreateCCC(dpy, screen_number, visual, clientWhitePt, gamutCompFunc,
	gamutCompClientData, whitePtAdjFunc, whitePtAdjClientData)
    Display *dpy;
    int	screen_number;
    Visual *visual;
    XcmsColor *clientWhitePt;
    XcmsFuncPtr gamutCompFunc;
    caddr_t gamutCompClientData;
    XcmsFuncPtr whitePtAdjFunc;
    caddr_t whitePtAdjClientData;
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
    XcmsCCC *pDefaultCCC = XcmsDefaultCCC(dpy, screen_number);
    XcmsCCC *pNewCCC;

    if (pDefaultCCC == NULL ||
	    !(pNewCCC = (XcmsCCC *) Xcalloc(1, (unsigned) sizeof(XcmsCCC)))) {
	return(NULL);
    } 

    /*
     * Should inherit the following as result of a bcopy():
     *		dpy
     *		screen_number
     *		pPerScrnInfo
     */
    bcopy((char *)pDefaultCCC, (char *)pNewCCC, sizeof(XcmsCCC));
    if (clientWhitePt) {
	bcopy((char *)clientWhitePt, (char *)&pNewCCC->clientWhitePt,
		sizeof(XcmsColor));
    }
    if (gamutCompFunc) {
	pNewCCC->gamutCompFunc = gamutCompFunc;
    }
    if (gamutCompClientData) {
	pNewCCC->gamutCompClientData = gamutCompClientData;
    }
    if (whitePtAdjFunc) {
	pNewCCC->whitePtAdjFunc = whitePtAdjFunc;
    }
    if (whitePtAdjClientData) {
	pNewCCC->whitePtAdjClientData = whitePtAdjClientData;
    }

    /*
     * Set visual component
     */
    pNewCCC->visual = visual;

    return(pNewCCC);
}


/*
 *	NAME
 *		XcmsDefaultCCC
 *
 *	SYNOPSIS
 */
XcmsCCC *
XcmsDefaultCCC(dpy, screen_number)
    Display *dpy;
    int screen_number;
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
    XcmsCCC *pCCC;


    if ((screen_number < 0) || (screen_number >= ScreenCount(dpy))) {
	return((XcmsCCC *)NULL);
    }

    /*
     * Check if the XcmsCCC's for each screen has been created
     */
    if ((XcmsCCC *)dpy->cms.defaultCCCs == NULL) {
	if (!_XcmsInitDefaultCCCs(dpy)) {
	    return((XcmsCCC *)NULL);
	}
    }

    pCCC = (XcmsCCC *)dpy->cms.defaultCCCs + screen_number;

    if (!pCCC->pPerScrnInfo) {
	/*
	 * Need to create the XcmsPerScrnInfo structure.  The
	 * _XcmsInitScrnInfo routine will create the XcmsPerScrnInfo
	 * structure as well as initialize its pSCCFuncSet and pSCCData
	 * components.
	 */
	if (!_XcmsInitScrnInfo(dpy, screen_number)) {
	    return((XcmsCCC *)NULL);
	}
	return(pCCC);
    } else {
	/*
	 * If pCCC->pPerScrnInfo->state == XCMS_INIT_SUCCESS,
	 *    then the pPerScrnInfo component has already been initialized
	 *    therefore, just return pCCC.
	 * If pCCC->pPerScrnInfo->state == XCMS_INIT_DEFAULT,
	 *    then this means that we already attempted to initialize
	 *    the pPerScrnInfo component but failed therefore stuffing
	 *    the pPerScrnInfo component with defaults.  Just return pCCC.
	 * If pCCC->pPerScrnInfo->state == XCMS_INIT_NONE,
	 *    then attempt to initialize the pPerScrnInfo component.
	 */
	switch (pCCC->pPerScrnInfo->state) {
	   case XCMS_INIT_DEFAULT :
	    /* fall through */
	   case XCMS_INIT_SUCCESS :
	    return(pCCC);
	   case XCMS_INIT_NONE :
	    /* XcmsPerScreenInfo has not been initialized */
	    if (!_XcmsInitScrnInfo(dpy, screen_number)) {
		return((XcmsCCC *)NULL);
	    }
	    return(pCCC);
	   default :
	    return((XcmsCCC *)NULL);
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
XcmsFreeCCC(pCCC)
    XcmsCCC *pCCC;
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
    if (pCCC == XcmsDefaultCCC(pCCC->dpy, pCCC->screen_number)) {
	/* do not allow clients to free DefaultCCC's */
	return;
    }

    Xfree(pCCC);

    /*
     * Note that XcmsPerScrnInfo sub-structures are not freed here.
     * There is only one allocated per Screen and it just so happens
     * that we place its initial reference is placed in the default CCC.
     * The routine _XcmsFreeDefaultCCCs frees them.
     */
}
