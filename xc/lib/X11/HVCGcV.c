/* $XConsortium: TekHVCGcV.c,v 1.13 91/01/28 14:33:09 alt Exp $" */

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
 *		TekHVCGcV.c
 *
 *	DESCRIPTION
 *		Source for XcmsTekHVC_AdjustValue() gamut compression routine.
 *
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
 *		XcmsTekHVC_AdjustValue - Return the closest value
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status
XcmsTekHVC_AdjustValue (pCCC, pColors_in_out, nColors, i, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    unsigned int i;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Return the closest value for a specific hue and chroma.
 *		This routine takes any color as input and outputs 
 *		a CIE XYZ color.
 *
 *		Since this routine works with the value within
 *		pColor_in_out intermediate results may be returned
 *		even though it may be invalid.
 *
 *	RETURNS
 *		XCMS_FAILURE - Failure
 *              XCMS_SUCCESS - Succeeded
 *
 */
{
    XcmsColor  *pColor;
    XcmsColor  hvc_max;
    XcmsCCC    myCCC;
    Status retval;

    /*
     * Insure TekHVC installed
     */
    if (XcmsAddDIColorSpace(&XcmsTekHVC_ColorSpace) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /* Use my own CCC */
    bcopy ((char *)pCCC, (char *)&myCCC, sizeof (XcmsCCC));
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;/* Inherit Screen WP */
    myCCC.gamutCompFunc = (XcmsFuncPtr)NULL;	/* no gamut compression */

    /*
     * Color specification passed as input can be assumed to:
     *	1. Be in XCMS_CIEXYZ_FORMAT
     *	2. Already be white point adjusted for the Screen White Point.
     *	    This means that the white point now associated with this
     *	    color spec is the Screen White Point (even if the
     *	    pCCC->clientWhitePt differs).
     */

    pColor = pColors_in_out + i;
    
    if (pCCC->visual->class < PseudoColor) {
	/*
	 * GRAY !
	 */
	return(XCMS_FAILURE);
    } else {
	/* Convert from CIEXYZ to TekHVC format */
	if (_XcmsDIConvertColors(&myCCC, pColor,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT)
		== XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}

	/* check to make sure we have a valid TekHVC number */
	if (!_XcmsTekHVC_CheckModify (pColor)) {
	    return (XCMS_FAILURE);
	}

	/* Step 1: compute the maximum value and chroma for this hue. */
	/*         This copy may be overkill but it preserves the pixel etc. */
	bcopy((char *)pColor, (char *)&hvc_max, sizeof(XcmsColor));
	if (_XcmsTekHVC_MaxValueChromaRGB (&myCCC, &hvc_max, (XcmsRGBi *)NULL)
		== XCMS_FAILURE) {
	    return (XCMS_FAILURE);
	}

	/* Now check and return the appropriate value */
	if (pColor->spec.TekHVC.C == hvc_max.spec.TekHVC.C) {
	    /* When the chroma input is equal to the maximum chroma */
	    /* merely return the value for that chroma. */
	    pColor->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	    if (!_XcmsTekHVC_CheckModify (pColor)) {
		return (XCMS_FAILURE);
	    }
	    retval = _XcmsDIConvertColors(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	} else if (pColor->spec.TekHVC.C > hvc_max.spec.TekHVC.C) {
	    /* When the chroma input is greater than the maximum chroma */
	    /* merely return the value and chroma for the given hue. */
	    pColor->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	    pColor->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	    return (XCMS_FAILURE);
	} else if (pColor->spec.TekHVC.V < hvc_max.spec.TekHVC.V) {
	    /* When the value input is less than the maximum value point */
	    /* compute the intersection of the line from 0,0 to max_V, max_C */
	    /* using the chroma input. */
	    pColor->spec.TekHVC.V = pColor->spec.TekHVC.C * 
				    hvc_max.spec.TekHVC.V / hvc_max.spec.TekHVC.C;
	    if (pColor->spec.TekHVC.V >= hvc_max.spec.TekHVC.V) {
		pColor->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
		pColor->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	    }
	    if (!_XcmsTekHVC_CheckModify (pColor)) {
		return (XCMS_FAILURE);
	    }
	    retval = _XcmsDIConvertColors(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	} else {
	    /* When the value input is greater than the maximum value point */
	    /* use HvcMaxValue to find the maximum value for the given chroma. */
	    if (XcmsTekHVC_MaxValue(&myCCC, pColor) == XCMS_FAILURE) {
		return (XCMS_FAILURE);
	    }
	    retval = _XcmsDIConvertColors(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	}
	if (retval != XCMS_FAILURE && pCompressed != NULL) {
	    *(pCompressed + i) = True;
	}
	return(retval);
    }
}
