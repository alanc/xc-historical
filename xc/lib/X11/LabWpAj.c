/* $XConsortium: CIELabWpAj.c,v 1.2 91/01/30 17:12:25 dave Exp $" */

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
 *
 *	NAME
 *		CIELabWpAj.c
 *
 *	DESCRIPTION
 *		This file contains routine(s) that support white point
 *		adjustment of color specifications in the CIE L*a*b* color
 *		space.
 */

#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *	EXTERNS
 */

extern Status _XcmsConvertColorsWithWhitePt();


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCIELab_WhiteAdjColors
 *
 *	SYNOPSIS
 */
Status
XcmsCIELab_WhiteAdjColors(pCCC, pWhitePtFrom, pWhitePtTo, destSpecFmt,
	pColors_in_out, nColors, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pWhitePtFrom;
    XcmsColor *pWhitePtTo;
    XcmsSpecFmt destSpecFmt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Adjust color specifications in XcmsColor structures for
 *		differences in white points.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression,
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with gamut
 *			compression.
 */
{
    if (pWhitePtFrom == NULL || pWhitePtTo == NULL || pColors_in_out == NULL) {
	return(0);
    }

    /*
     * Convert to CIELab using pWhitePtFrom
     */
    if (_XcmsConvertColorsWithWhitePt(pCCC, pColors_in_out, pWhitePtFrom,
	    nColors, XCMS_CIELab_FORMAT, pCompressed) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /*
     * Convert from CIELab to destSpecFmt using pWhitePtTo
     */
    return(_XcmsConvertColorsWithWhitePt(pCCC, pColors_in_out,
	pWhitePtTo, nColors, destSpecFmt, pCompressed));
}
