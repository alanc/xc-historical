/* $XConsortium: TekHVCMnV.c,v 1.2 91/02/05 13:41:10 dave Exp $" */

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
 *		TekHVCMnV.c
 *
 *	DESCRIPTION
 *		Source for XcmsTekHVC_MinValue gamut boundary querying routine.
 *
 */

#include "Xcmsint.h"

/*
 *	DEFINES
 */
#define EPS	    0.001

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
 *		XcmsTekHVC_MinValue - Compute minimum value for hue and chroma
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_MinValue (pCCC, pColor_in_out)
    XcmsCCC *pCCC;
    XcmsColor *pColor_in_out;

/*
 *	DESCRIPTION
 *		Return the minimum value for a specific hue, and the
 *		corresponding chroma.  The input color specification
 *		may be in any format, however output is in XCMS_TekHVC_FORMAT.
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
 *		XCMS_SUCCESS - Succeeded with no modifications
 *
 */ 
{
    XcmsCCC	myCCC;
    XcmsColor	hvc_max;

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
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;/* inherit screen white pt */
    myCCC.gamutCompFunc = (XcmsFuncPtr)NULL;	/* no gamut comp func */

    /* Convert the color to HVC format if it is not */
    if (pColor_in_out->format != XCMS_TekHVC_FORMAT) {
	/* convert using Screen White Point */
	if (_XcmsConvertColorsWithWhitePt(&myCCC, pColor_in_out,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }

    /* Check for a valid HVC */
    if (!_XcmsTekHVC_CheckModify (pColor_in_out)) {
	return(XCMS_FAILURE);
    }

    /* Step 1: compute the maximum value and chroma for this hue. */
    /*         This copy may be overkill but it preserves the pixel etc. */
    bcopy((char *)pColor_in_out, (char *)&hvc_max, sizeof(XcmsColor));
    if (_XcmsTekHVC_MaxValueChromaRGB (&myCCC, &hvc_max, (XcmsRGBi *)NULL)
	    == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /* Step 2: find the intersection with the maximum hvc and chroma line. */
    if (pColor_in_out->spec.TekHVC.C > hvc_max.spec.TekHVC.C + EPS) {
	/* If the chroma is to large then return maximum hvc. */
	pColor_in_out->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	pColor_in_out->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
    } else {
	pColor_in_out->spec.TekHVC.V = pColor_in_out->spec.TekHVC.C *
		hvc_max.spec.TekHVC.V / hvc_max.spec.TekHVC.C;
	if (pColor_in_out->spec.TekHVC.V > hvc_max.spec.TekHVC.V) {
	    pColor_in_out->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	} else if (pColor_in_out->spec.TekHVC.V < 0.0) {
	    pColor_in_out->spec.TekHVC.V = pColor_in_out->spec.TekHVC.C = 0.0;
	}
    }
    if (_XcmsTekHVC_CheckModify (pColor_in_out)) {
	return(XCMS_SUCCESS);
    } else {
	return(XCMS_FAILURE);
    }
}
