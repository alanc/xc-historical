/* $XConsortium: TekHVCMxV.c,v 1.3 91/02/11 18:17:08 dave Exp $" */

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
 *		TekHVCMxV.c
 *
 *	DESCRIPTION
 *		Source for the XcmsTekHVC_MaxValue() gamut boundary
 *		querying routine.
 *
 */

#include <math.h>
#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *	DEFINES
 */
#define MAXBISECTCOUNT	100
#define EPS	    0.001

/*
 *	EXTERNS
 */
extern Status _XcmsTekHVC_MaxValueChromaRGB();
extern int _XcmsTekHVC_CheckModify();
extern XcmsColorSpace XcmsTekHVC_ColorSpace;


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_MaxValue - Compute maximum value for a hue and chroma
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_MaxValue(pCCC, pColor_in_out)
    XcmsCCC *pCCC;
    XcmsColor *pColor_in_out;
/*
 *	DESCRIPTION
 *		Return the maximum value for a specific hue and chroma.
 *		This routine will take any input color and return
 *		a CIE XYZ color.
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
 *              XCMS_SUCCESS - Succeeded with no modifications
 *
 */
{
    XcmsCCC     myCCC;
    XcmsColor   hvc_max;
    XcmsRGBi    rgb_saved;
    int         nCount, nMaxCount;
    XcmsFloat   nT, nChroma, savedChroma, lastValue, lastChroma, prevChroma;
    XcmsFloat   rFactor, Hue;

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

    /* setup the CCC to use for the conversions. */
    bcopy ((char *) pCCC, (char *) &myCCC, sizeof (XcmsCCC));
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;
    myCCC.gamutCompFunc = (XcmsFuncPtr) NULL;

    /* Convert the color to HVC format if it is not */
    if (pColor_in_out->format != XCMS_TekHVC_FORMAT) {
	/* convert using Screen White Point */
	if (_XcmsConvertColorsWithWhitePt(&myCCC, pColor_in_out,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }

    if (!_XcmsTekHVC_CheckModify (pColor_in_out)) {
	return(XCMS_FAILURE);
    }

    /* Step 1: compute the maximum value and chroma for this hue. */
    /*         This copy may be overkill but it preserves the pixel etc. */
    bcopy((char *)pColor_in_out, (char *)&hvc_max, sizeof(XcmsColor));
    Hue = hvc_max.spec.TekHVC.H;
    if (_XcmsTekHVC_MaxValueChromaRGB(&myCCC, &hvc_max, &rgb_saved)
	    == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    if (hvc_max.spec.TekHVC.C < pColor_in_out->spec.TekHVC.C) {
	/*
	 *  If the chroma is greater than the chroma for the 
	 *  maximum value/chroma point then the value is the
	 *  the value for the maximum value, chroma point.
	 *  This is an error but it I return the best approximation I can.
         *  Thus the inconsistency.
	 */
	pColor_in_out->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	pColor_in_out->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	return(XCMS_SUCCESS);
    } else if (hvc_max.spec.TekHVC.C == pColor_in_out->spec.TekHVC.C) {
	/*
	 *  If the chroma is equal to the chroma for the 
	 *  maximum value/chroma point then the value is the
	 *  the value for the maximum value, chroma point.
	 */
	pColor_in_out->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	pColor_in_out->format = XCMS_TekHVC_FORMAT;
	return(XCMS_SUCCESS);
    } else {
	/* must do a bisection here to compute the maximum value */
	/* save the structure input so that any elements that */
	/* are not touched are recopied later in the routine. */
	nChroma = savedChroma = pColor_in_out->spec.TekHVC.C;
	pColor_in_out->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	pColor_in_out->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	lastChroma = -1.0;
	nMaxCount = MAXBISECTCOUNT;
	rFactor = 1.0;

	for (nCount = 0; nCount < nMaxCount; nCount++) {
	    prevChroma = lastChroma;
	    lastValue =  pColor_in_out->spec.TekHVC.V;
	    lastChroma = pColor_in_out->spec.TekHVC.C;
	    nT = (1.0 - (nChroma / hvc_max.spec.TekHVC.C)) * rFactor;
	    pColor_in_out->spec.RGBi.red   = rgb_saved.red * (1.0 - nT) + nT;
	    pColor_in_out->spec.RGBi.green = rgb_saved.green * (1.0 - nT) + nT;
	    pColor_in_out->spec.RGBi.blue  = rgb_saved.blue * (1.0 - nT) + nT;
	    pColor_in_out->format = XCMS_RGBi_FORMAT;

	    /* convert from RGB to HVC */
	    if (_XcmsConvertColorsWithWhitePt(&myCCC, pColor_in_out,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		    (Bool *) NULL) == XCMS_FAILURE) {
		return(XCMS_FAILURE);
	    }

	    /* Now check the return against what is expected */
	    if (pColor_in_out->spec.TekHVC.C <= savedChroma + EPS &&
		pColor_in_out->spec.TekHVC.C >= savedChroma - EPS) {
		pColor_in_out->spec.TekHVC.H = Hue;  /* use the saved Hue */
		return(XCMS_SUCCESS);
	    } 
	    nChroma += savedChroma - pColor_in_out->spec.TekHVC.C;
	    if (nChroma > hvc_max.spec.TekHVC.C) {
		nChroma = hvc_max.spec.TekHVC.C;
		rFactor *= 0.5;  /* selective relaxation employed */
	    } else if (nChroma < 0.0) {
		if (fabs ((double) (lastChroma - savedChroma)) < 
		    fabs ((double) (pColor_in_out->spec.TekHVC.C - savedChroma))) {
		    pColor_in_out->spec.TekHVC.V = lastValue;
		    pColor_in_out->spec.TekHVC.C = lastChroma;
		}
		/* make sure to return the input hue */
		pColor_in_out->spec.TekHVC.H = Hue;
		if (!_XcmsTekHVC_CheckModify(pColor_in_out)) {
		    return(XCMS_FAILURE);
		}
		return(XCMS_SUCCESS);
	    } else if (pColor_in_out->spec.TekHVC.C <= prevChroma + EPS &&
		       pColor_in_out->spec.TekHVC.C >= prevChroma - EPS) {
		rFactor *= 0.5;  /* selective relaxation employed */
	    }
	}
	if (nCount >= nMaxCount) {
	    if (fabs((double) (lastChroma - savedChroma)) < 
		fabs((double) (pColor_in_out->spec.TekHVC.C - savedChroma))) {
		    pColor_in_out->spec.TekHVC.V = lastValue;
		    pColor_in_out->spec.TekHVC.C = lastChroma;
	    }
	}
    }

    /* make sure to return the input hue */
    pColor_in_out->spec.TekHVC.H = Hue;
    return(XCMS_SUCCESS);
}
