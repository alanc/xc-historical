/* $XConsortium: TekHVCGcC.c,v 1.2 91/02/05 13:41:03 dave Exp $" */

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
 *		TekHVCGcC.c
 *
 *	DESCRIPTION
 *		Source for XcmsTekHVC_ReduceChroma() gamut compression routine.
 *
 */

#include "Xcmsint.h"

/*
 *	EXTERNS
 */
extern Status XcmsTekHVC_MaxChroma();
extern XcmsColorSpace XcmsTekHVC_ColorSpace;



/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_ReduceChroma - Reduce the chroma for a hue and value
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status
XcmsTekHVC_ReduceChroma (pCCC, pColors_in_out, nColors, i, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    unsigned int i;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Reduce the Chroma for a specific hue and value to
 *		to bring the given color into the gamut of the 
 *		specified device.  As required of gamut compression
 *		functions in TekCMS, this routine returns pColor_in_out
 *		in XCMS_CIEXYZ_FORMAT on successful completion.
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
    XcmsColor *pColor;

    /*
     * Color specification passed as input can be assumed to:
     *	1. Be in XCMS_CIEXYZ_FORMAT
     *	2. Already be white point adjusted for the Screen White Point.
     *	    This means that the white point now associated with this
     *	    color spec is the Screen White Point (even if the
     *	    pCCC->clientWhitePt differs).
     */

    /*
     * Insure TekHVC installed
     */
    if (XcmsAddDIColorSpace(&XcmsTekHVC_ColorSpace) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

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
	if (XcmsTekHVC_MaxChroma(pCCC, pColor) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	retval = _XcmsDIConvertColors(pCCC, pColor,
		&pCCC->pPerScrnInfo->screenWhitePt, 1, XCMS_CIEXYZ_FORMAT);
	if (retval != XCMS_FAILURE && pCompressed != NULL) {
	    *(pCompressed + i) = True;
	}
	return(retval);
    }
}
