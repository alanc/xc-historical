/* $XConsortium: TekHVCMxVs.c,v 1.13 91/01/27 00:34:22 alt Exp $" */

/*
 * (c) Copyright 1990 1991 Tektronix Inc.
 * 	All Rights Reserved
 *
 * This code, which implements the TekColor Human Interface and/or the TekHVC
 * Color Space algorithms, is proprietary to Tektronix, Inc., and permission
 * is granted for use only in the form supplied.  Revisions, modifications,
 * or * adaptations are not permitted without the prior written approval of
 * Tektronix, Inc., Beaverton, OR 97077.  Code and supporting documentation
 * copyright Tektronix, Inc. 1990 1991 All rights reserved.  TekColor and TekHVC
 * are trademarks of Tektronix, Inc.  U.S. and foreign patents pending.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *	NAME
 *		TekHVCMxVs.c
 *
 *	DESCRIPTION
 *		Source for the XcmsTekHVC_MaxValueSamples() gamut boundary
 *		querying routine.
 */

/*
 *	EXTERNAL INCLUDES
 *		Include files that must be exported to any package or
 *		program using this package.
 */
#include <Xlib.h>
#include "TekCMS.h"

/*
 *	INTERNAL INCLUDES
 *		Include files that need NOT be exported to any package or
 *		program using this package.
 */
#include "TekCMSext.h"

/*
 *	EXTERNS
 */
extern Status _XcmsTekHVC_MaxValueChromaRGB();
extern Status _XcmsTekHVC_CheckModify();
extern XcmsColorSpace XcmsTekHVC_ColorSpace;



/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_MaxValueSamples - Compute a set of value/chroma
 *						pairs.
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_MaxValueSamples(pCCC, hue, pColor_in_out, nSamples)
    XcmsCCC *pCCC;
    XcmsFloat hue;
    XcmsColor *pColor_in_out;
    unsigned short nSamples;

/*
 *	DESCRIPTION
 *		Return a set of values and chromas for the input Hue.
 *		This routine will take any color as input.  
 *		It returns TekHVC colors.
 *
 *		Since this routine works with the value within
 *		pColor_in_out intermediate results may be returned
 *		even though it may be invalid.
 *
 *	ASSUMPTIONS
 *		This routine assumes that the white point associated with
 *		the color specification is the Screen White Point.  The
 *		Screen White Point will also be associated with the
 *		returned color specification.
 *
 *	RETURNS
 *		XCMS_FAILURE - Failure
 *		XCMS_SUCCESS - Succeeded
 *
 */
{
    XcmsCCC	    myCCC;
    XcmsColor       *pHVC;
    XcmsRGBi        rgb_saved;
    unsigned short nI;
    XcmsFloat       nT;

    /*
     * Check Arguments
     */
    if (pCCC == NULL || pColor_in_out == NULL || nSamples == 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Insure TekHVC installed
     */
    if (XcmsAddDIColorSpace(&XcmsTekHVC_ColorSpace) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /* setup the CCC to use for the conversions. */
    bcopy ((char *) pCCC, (char *) &myCCC, sizeof(XcmsCCC));
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;
    myCCC.gamutCompFunc = (XcmsFuncPtr) NULL;

    /* Step 1: compute the maximum value and chroma for this hue. */


    /* save the Hue for use later. */
    while (hue < 0.0) {
	hue += 360.0;
    }
    while (hue > 360.0) {
	hue -= 360.0;
    }
    pColor_in_out->spec.TekHVC.H = hue;
    pColor_in_out->format = XCMS_TekHVC_FORMAT;

    /* Get the maximum value and chroma point for this hue */
    if (_XcmsTekHVC_MaxValueChromaRGB (&myCCC, pColor_in_out,
	    (XcmsRGBi *)&rgb_saved) == XCMS_FAILURE) {
	return (XCMS_FAILURE);
    }

    /* Step 2:  Convert each of the RGBi's to HVC's */
    pHVC = pColor_in_out;
    for (nI = 0; nI < nSamples; nI++, pHVC++) {
	nT = (XcmsFloat) nI / (XcmsFloat) nSamples;
	pHVC->spec.RGBi.red   = rgb_saved.red * (1.0 - nT) + nT;
	pHVC->spec.RGBi.green = rgb_saved.green * (1.0 - nT) + nT;
	pHVC->spec.RGBi.blue  = rgb_saved.blue * (1.0 - nT) + nT;
	pHVC->format          = XCMS_RGBi_FORMAT;
	pHVC->pixel           = pColor_in_out->pixel;
	/* convert from RGB to HVC */
	if (_XcmsConvertColorsWithWhitePt(&myCCC, pHVC,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}

	/* make sure to return the input hue */
	pHVC->spec.TekHVC.H = hue;
    }

    return(XCMS_SUCCESS);
}
