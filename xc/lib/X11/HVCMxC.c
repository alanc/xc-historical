/* $XConsortium: TekHVCMxC.c,v 1.2 91/02/05 13:41:12 dave Exp $" */

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
 *		TekHVCMxC.c
 *
 *	DESCRIPTION
 *		Source for the XcmsTekHVC_MaxChroma() gamut boudary
 *		querying routine.
 *
 */

#include <math.h>
#include "Xcmsint.h"

/*
 *	DEFINES
 */
#define MAXBISECTCOUNT	100
#define EPS	        0.001

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
 *		XcmsTekHVC_MaxChroma - Compute the maximum chroma for a hue and value
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_MaxChroma(pCCC, pColor_in_out)
    XcmsCCC *pCCC;
    XcmsColor *pColor_in_out;
/*
 *	DESCRIPTION
 *		Return the maximum chroma for a specific hue and value.
 *		The input color specification may be in any format,
 *		however, the returned format is in XCMS_TekHVC_FORMAT.
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
 *              XCMS_SUCCESS - Succeeded
 *
 */
{
    XcmsCCC    myCCC;
    XcmsColor  hvc_max;
    XcmsRGBi   rgb_saved;
    int nCount, nMaxCount;
    XcmsFloat nValue, savedValue, lastValue, lastChroma, prevValue;
    XcmsFloat maxDist, nT, rFactor, Hue;

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
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT; /* inherit screen white Pt */
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

    /* check to make sure we have a valid TekHVC number */
    if (!_XcmsTekHVC_CheckModify(pColor_in_out)) {
	return(XCMS_FAILURE);
    }

    /* Step 1: compute the maximum value and chroma for this hue. */
    /*	       This copy may be overkill but it preserves the pixel etc. */
    Hue = pColor_in_out->spec.TekHVC.H;
    bcopy((char *)pColor_in_out, (char *)&hvc_max, sizeof(XcmsColor));
    if (_XcmsTekHVC_MaxValueChromaRGB(&myCCC, &hvc_max, &rgb_saved)
	    == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /* Step 2: If the value is less than the value for the maximum */
    /*         value, chroma point then the chroma is on the line  */
    /*         from hvc_max to 0,0. */
    if (pColor_in_out->spec.TekHVC.V <= hvc_max.spec.TekHVC.V) {
	pColor_in_out->spec.TekHVC.C = pColor_in_out->spec.TekHVC.V 
			      * hvc_max.spec.TekHVC.C / hvc_max.spec.TekHVC.V;
	if (_XcmsTekHVC_CheckModify (pColor_in_out)) {
	    return(XCMS_SUCCESS);
	} else {
	    return(XCMS_FAILURE);
	}
    } else {
	/* must do a bisection here to compute the maximum chroma */
	/* save the structure input so that any elements that */
	/* are not touched are recopied later in the routine. */
	nValue = savedValue = pColor_in_out->spec.TekHVC.V;
	lastValue = -1.0;
	nMaxCount = MAXBISECTCOUNT;
	maxDist = 100.0 - hvc_max.spec.TekHVC.V;
	rFactor = 1.0;

	for (nCount = 0; nCount < nMaxCount; nCount++) {
	    prevValue =  lastValue;
	    lastValue =  pColor_in_out->spec.TekHVC.V;
	    lastChroma = pColor_in_out->spec.TekHVC.C;
	    nT = (nValue - hvc_max.spec.TekHVC.V) / maxDist * rFactor;
	    pColor_in_out->spec.RGBi.red = rgb_saved.red * (1.0 - nT) + nT;
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
	    if (pColor_in_out->spec.TekHVC.V <= savedValue + EPS &&
		pColor_in_out->spec.TekHVC.V >= savedValue - EPS) {
		/* make sure to return the input hue */
		pColor_in_out->spec.TekHVC.H = Hue;
		if (_XcmsTekHVC_CheckModify (pColor_in_out)) {
		    return(XCMS_SUCCESS);
		} else {
		    return(XCMS_FAILURE);
		}
	    }
	    nValue += savedValue - pColor_in_out->spec.TekHVC.V;
	    if (nValue < hvc_max.spec.TekHVC.V) {
		nValue = hvc_max.spec.TekHVC.V;
		rFactor *= 0.5;  /* selective relaxation employed */
	    } else if (nValue > 100.0) {
		/* make sure to return the input hue */
		pColor_in_out->spec.TekHVC.H = Hue;
		if (fabs ((double) (lastValue - savedValue)) < 
		    fabs ((double) (pColor_in_out->spec.TekHVC.V - savedValue))) {
		    pColor_in_out->spec.TekHVC.V = lastValue;
		    pColor_in_out->spec.TekHVC.C = lastChroma;
		} 
		if (_XcmsTekHVC_CheckModify (pColor_in_out)) {
		    return(XCMS_SUCCESS);
		} else {
		    return(XCMS_FAILURE);
		}
	    } else if (pColor_in_out->spec.TekHVC.V <= prevValue + EPS &&
		       pColor_in_out->spec.TekHVC.V >= prevValue - EPS) {
		rFactor *= 0.5;  /* selective relaxation employed */
	    }
	}
	if (nCount >= nMaxCount) {
	    if (fabs((double) (lastValue - savedValue)) < 
		fabs((double) (pColor_in_out->spec.TekHVC.V - savedValue))) {
		pColor_in_out->spec.TekHVC.V = lastValue;
		pColor_in_out->spec.TekHVC.C = lastChroma;
	    }
	}
    } 
    /* make sure to return the input hue */
    pColor_in_out->spec.TekHVC.H = Hue;
    return(XCMS_SUCCESS);
}
