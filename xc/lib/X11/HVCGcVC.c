/* $XConsortium: TekHVCGcVC.c,v 1.2 91/02/05 13:41:07 dave Exp $" */

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
 *		TekHVCGcVC.c
 *
 *	DESCRIPTION
 *		Source for XcmsTekHVC_ShortestValueChroma() gamut
 *		compression function.
 */

#include <math.h>
#include "Xcmsint.h"

/*
 *	INTERNALS
 *		Internal defines that need NOT be exported to any package or
 *		program using this package.
 */
#define MAXBISECTCOUNT	100

/*
 *	EXTERNS
 */
extern int _XcmsTekHVC_CheckModify();
extern Status _XcmsTekHVC_MaxValueChromaRGB();
extern XcmsColorSpace XcmsTekHVC_ColorSpace;


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_ShortestValueChroma - Return the closest value and chroma
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status
XcmsTekHVC_ShortestValueChroma (pCCC, pColors_in_out, nColors, i, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    unsigned int i;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		This routine will find the closest value and chroma 
 *		for a specific hue.  The color input is converted to
 *		HVC format and returned as CIE XYZ format.
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
    Status retval;
    XcmsCCC    myCCC;
    XcmsColor  *pColor;
    XcmsColor  hvc_max;
    XcmsRGBi   rgb_max;
    int	      nCount, nMaxCount, nI, nILast;
    XcmsFloat  Chroma, Value, bestChroma, bestValue, nT, saveDist, tmpDist;

    /*
     * Insure TekHVC installed
     */
    if (XcmsAddDIColorSpace(&XcmsTekHVC_ColorSpace) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /* Use my own CCC */
    bcopy ((char *)pCCC, (char *)&myCCC, sizeof (XcmsCCC));
    myCCC.clientWhitePt.format = XCMS_UNDEFINED_FORMAT;/* inherit screen white pt */
    myCCC.gamutCompFunc = (XcmsFuncPtr)NULL;	/* no gamut compression func */

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
	_XcmsDIConvertColors(pCCC, pColor, &pCCC->pPerScrnInfo->screenWhitePt,
		1, XCMS_TekHVC_FORMAT);
	pColor->spec.TekHVC.H = pColor->spec.TekHVC.C = 0.0;
	_XcmsDIConvertColors(pCCC, pColor, &pCCC->pPerScrnInfo->screenWhitePt,
		1, XCMS_CIEXYZ_FORMAT);
	if (pCompressed) {
	    *(pCompressed + i) = True;
	}
	return(XCMS_SUCCESS);
    } else {
	/* Convert from CIEXYZ to TekHVC format */
	if (_XcmsDIConvertColors(&myCCC, pColor,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT)
		== XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}

	if (!_XcmsTekHVC_CheckModify(pColor)) {
	    return (XCMS_FAILURE);
	}

	/* Step 1: compute the maximum value and chroma for this hue. */
	/*         This copy may be overkill but it preserves the pixel etc. */
	bcopy((char *)pColor, (char *)&hvc_max, sizeof(XcmsColor));
	if (_XcmsTekHVC_MaxValueChromaRGB (&myCCC, &hvc_max, &rgb_max)
		== XCMS_FAILURE) {
	    return (XCMS_FAILURE);
	}

	/* Now check and return the appropriate value */
	if (pColor->spec.TekHVC.V == hvc_max.spec.TekHVC.V) {
	    /* When the value input is equal to the maximum value */
	    /* merely return the chroma for that value. */
	    pColor->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	    retval = _XcmsDIConvertColors(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	}

	if (pColor->spec.TekHVC.V < hvc_max.spec.TekHVC.V) {
	    /* return the intersection of the perpindicular line through     */
	    /* the value and chroma given and the line from 0,0 and hvc_max. */
	    Chroma = pColor->spec.TekHVC.C;
	    Value = pColor->spec.TekHVC.V;
	    pColor->spec.TekHVC.C = 
	       (Value + (hvc_max.spec.TekHVC.C / hvc_max.spec.TekHVC.V * Chroma)) /
	       ((hvc_max.spec.TekHVC.V / hvc_max.spec.TekHVC.C) +
		(hvc_max.spec.TekHVC.C / hvc_max.spec.TekHVC.V));
	    if (pColor->spec.TekHVC.C >= hvc_max.spec.TekHVC.C) {
		pColor->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
		pColor->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	    } else {
		pColor->spec.TekHVC.V = pColor->spec.TekHVC.C *
				    hvc_max.spec.TekHVC.V / hvc_max.spec.TekHVC.C;
	    }
	    retval = _XcmsDIConvertColors(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	}

	/* return the closest point on the upper part of the hue leaf. */
	/* must do a bisection here to compute the delta e. */
	nMaxCount = MAXBISECTCOUNT;
	nI =     nMaxCount / 2;
	bestValue = Value =  pColor->spec.TekHVC.V;
	bestChroma = Chroma = pColor->spec.TekHVC.C;
	saveDist = (XcmsFloat) sqrt ((double) (((Chroma - hvc_max.spec.TekHVC.C) *
					       (Chroma - hvc_max.spec.TekHVC.C)) +
					      ((Value - hvc_max.spec.TekHVC.V) *
					       (Value - hvc_max.spec.TekHVC.V))));
	for (nCount = 0; nCount < nMaxCount; nCount++) {
	    nT = (XcmsFloat) nI / (XcmsFloat) nMaxCount;
	    pColor->spec.RGBi.red   = rgb_max.red * (1.0 - nT) + nT;
	    pColor->spec.RGBi.green = rgb_max.green * (1.0 - nT) + nT;
	    pColor->spec.RGBi.blue  = rgb_max.blue * (1.0 - nT) + nT;
	    pColor->format = XCMS_RGBi_FORMAT;

	    /* Convert from RGBi to HVC */
	    if (_XcmsConvertColorsWithWhitePt(&myCCC, pColor,
		    &myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_TekHVC_FORMAT,
		    (Bool *) NULL)
		    == XCMS_FAILURE) {
		return (XCMS_FAILURE);
	    }
	    if (!_XcmsTekHVC_CheckModify(pColor)) {
		return (XCMS_FAILURE);
	    }
	    tmpDist = (XcmsFloat) sqrt ((double) 
			(((Chroma - pColor->spec.TekHVC.C) *
			  (Chroma - pColor->spec.TekHVC.C)) +
			 ((Value - pColor->spec.TekHVC.V) *
			  (Value - pColor->spec.TekHVC.V))));
	    nILast = nI;
	    if (tmpDist > saveDist) {
		nI /= 2;
	    } else {
		nI = (nMaxCount + nI) / 2;
		saveDist = tmpDist;
		bestValue = pColor->spec.TekHVC.V;
		bestChroma = pColor->spec.TekHVC.C;	    
	    }
	    if (nI == nILast || nI == 0) {
		break;
	    }

	}

	if (bestChroma >= hvc_max.spec.TekHVC.C) {
	    pColor->spec.TekHVC.C = hvc_max.spec.TekHVC.C;
	    pColor->spec.TekHVC.V = hvc_max.spec.TekHVC.V;
	} else {
	    pColor->spec.TekHVC.C = bestChroma;
	    pColor->spec.TekHVC.V = bestValue;
	}
	if (!_XcmsTekHVC_CheckModify(pColor)) {
	    return (XCMS_FAILURE);
	}
	retval = _XcmsDIConvertColors(&myCCC, pColor,
		&myCCC.pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);

	if (retval != XCMS_FAILURE && pCompressed != NULL) {
	    *(pCompressed + i) = True;
	}
	return(retval);
    }
}
