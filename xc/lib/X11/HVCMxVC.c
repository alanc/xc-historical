/* $XConsortium: TekHVCMxVC.c,v 1.1 91/01/30 18:05:32 dave Exp $" */

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
 *		TekHVCMxVC.c
 *
 *	DESCRIPTION
 *		Source for the XcmsTekHVC_MaxValueChroma() gamut boundary
 *		querying routine.
 *
 */

/*
 *	EXTERNAL INCLUDES
 *		Include files that must be exported to any package or
 *		program using this package.
 */
#include "TekCMS.h"

/*
 *	INTERNAL INCLUDES
 *		Include files that need NOT be exported to any package or
 *		program using this package.
 */
#include "TekCMSext.h"

/*
 *	DEFINES
 */
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define MIN3(x,y,z) (MIN((x), MIN((y), (z))))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MAX3(x,y,z) (MAX((x), MAX((y), (z))))
#define START_V	    40.0
#define START_C	    120.0

/*
 *	EXTERNS
 */
extern Status _XcmsTekHVC_CheckModify();
extern Status _XcmsConvertColorsWithWhitePt();
extern XcmsColorSpace XcmsTekHVC_ColorSpace;

/*
 *	FORWARD DECLARATIONS
 */
Status _XcmsTekHVC_MaxValueChromaRGB();


/************************************************************************
 *									*
 *			 API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsTekHVC_MaxValueChromaRGB - Compute maximum value/chroma.
 *
 *	SYNOPSIS
 */
Status
_XcmsTekHVC_MaxValueChromaRGB(pCCC, pColor_in_out, pRGB_return)
    XcmsCCC	*pCCC;
    XcmsColor   *pColor_in_out;
    XcmsRGBi    *pRGB_return;

/*
 *	DESCRIPTION
 *		Return the maximum chroma for a specific hue, and the
 *		corresponding value.  This is computed by a binary search of
 *		all possible chromas.  An assumption is made that there are
 *		no local maxima.  Use the unrounded Max Chroma because
 *		the difference check can be small.
 *
 *		NOTE:  No local CCC is used because this is a private
 *		       routine and all routines that call it are expected
 *		       to behave properly, i.e. send a local CCC with
 *		       no white adjust function and no gamut compression
 *		       function.
 *
 *		This routine only accepts HVC's as input and outputs
 *		HVC's and RGBi's.
 *
 *		Since this routine works with the value within
 *		pColor_in_out intermediate results may be returned
 *		even though it may be invalid.
 *
 *	RETURNS
 *		XCMS_FAILURE - Failure
 *		XCMS_SUCCUSS - Succeeded
 *
 */ 
{
    XcmsFloat nSmall, nLarge, Hue;

    if (pColor_in_out->format != XCMS_TekHVC_FORMAT) {
	return (XCMS_FAILURE);
    }

    /*  Use some unreachable color on the given hue */
    Hue = pColor_in_out->spec.TekHVC.H;
    pColor_in_out->spec.TekHVC.V = START_V;
    pColor_in_out->spec.TekHVC.C = START_C;

    /*
     * Convert from HVC to RGB
     *
     * Note that the CIEXYZ to RGBi conversion routine must stuff the
     * out of bounds RGBi values in pColor_in_out when the pCCC->gamutCompFunc
     * is NULL.
     */
    if ((_XcmsConvertColorsWithWhitePt(pCCC, pColor_in_out,
	    &pCCC->pPerScrnInfo->screenWhitePt, 1, XCMS_RGBi_FORMAT, (Bool *) NULL) 
	    == XCMS_FAILURE) && pColor_in_out->format != XCMS_RGBi_FORMAT) {
	return (XCMS_FAILURE);
    }

    /* Now pick the smallest RGB */
    nSmall = MIN3(pColor_in_out->spec.RGBi.red, 
		  pColor_in_out->spec.RGBi.green, 
		  pColor_in_out->spec.RGBi.blue);
    /* Make the smallest RGB equal to zero */
    pColor_in_out->spec.RGBi.red   -= nSmall;
    pColor_in_out->spec.RGBi.green -= nSmall;
    pColor_in_out->spec.RGBi.blue  -= nSmall;

    /* Now pick the largest RGB */
    nLarge = MAX3(pColor_in_out->spec.RGBi.red, 
		  pColor_in_out->spec.RGBi.green, 
		  pColor_in_out->spec.RGBi.blue);
    /* Scale the RGB values based on the largest one */
    pColor_in_out->spec.RGBi.red   /= nLarge;
    pColor_in_out->spec.RGBi.green /= nLarge;
    pColor_in_out->spec.RGBi.blue  /= nLarge;
    pColor_in_out->format = XCMS_RGBi_FORMAT;

    /* If the calling routine wants RGB value give them the ones used. */
    if (pRGB_return) {
	pRGB_return->red   = pColor_in_out->spec.RGBi.red;
	pRGB_return->green = pColor_in_out->spec.RGBi.green;
	pRGB_return->blue  = pColor_in_out->spec.RGBi.blue;
    }

    /* Convert from RGBi to HVC */
    if (_XcmsConvertColorsWithWhitePt(pCCC, pColor_in_out,
	    &pCCC->pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT, (Bool *) NULL) 
	    == XCMS_FAILURE) {
	return (XCMS_FAILURE);
    }

    /* make sure to return the input hue */
    pColor_in_out->spec.TekHVC.H = Hue;
    return (XCMS_SUCCESS);    
}


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_MaxValueChroma - Compute maximum value and chroma.
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_MaxValueChroma (pCCC, pColor_in_out)
    XcmsCCC *pCCC;
    XcmsColor *pColor_in_out;

/*
 *	DESCRIPTION
 *		Return the maximum chroma for a specific hue, and the
 *		corresponding value.  This routine takes any color as input
 *		and outputs a CIE XYZ color.
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
    XcmsCCC  myCCC;

    /*
     * Check Arguments
     */
    if (pCCC == NULL || pColor_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Insure TekHVC installed
     */
    if (XcmsAddDIColorSpace(&XcmsTekHVC_ColorSpace) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }
    
    /* Use my own CCC */
    bcopy ((char *)pCCC, (char *)&myCCC, sizeof (XcmsCCC));
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;
    myCCC.gamutCompFunc = (XcmsFuncPtr)NULL;
    /* myCCC.whitePtAdjFunc = (XcmsFuncPtr)NULL; */

    /* Convert the color to HVC format if it is not */
    if (pColor_in_out->format != XCMS_TekHVC_FORMAT) {
	/* convert using Screen White Point */
	if (_XcmsConvertColorsWithWhitePt(&myCCC, pColor_in_out,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }
    
    /* check input for a valid TekHVC number */
    if (!_XcmsTekHVC_CheckModify (pColor_in_out)) {
	return (XCMS_FAILURE);
    }

    return(_XcmsTekHVC_MaxValueChromaRGB (&myCCC, pColor_in_out,
	    (XcmsRGBi *)NULL));
}
