/* $XConsortium: XcmsCvColW.c,v 1.1 91/01/30 18:35:24 dave Exp $" */

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
 *
 *	NAME
 *		XcmsCvColW.c
 *
 *	DESCRIPTION
 *		<overall description of what the package does>
 *
 *
 */

#include "Xcmsint.h"


/************************************************************************
 *									*
 *			 API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/


/*
 *	NAME
 *		_XcmsConvertColorsWithWhitePt - Convert XcmsColor structures
 *
 *	SYNOPSIS
 */
Status
_XcmsConvertColorsWithWhitePt(pCCC, pColors_in_out, pWhitePt, nColors,
	newFormat, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    XcmsColor *pWhitePt;
    unsigned int nColors;
    XcmsSpecFmt newFormat;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Convert XcmsColor structures between device-independent
 *		and/or device-dependent formats but allowing the calling
 *		routine to specify the white point to be associated
 *		with the color specifications (overriding
 *		pCCC->clientWhitePt).
 *
 *		This routine has been provided for use in white point
 *		adjustment routines.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression,
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with gamut
 *			compression.
 *
 */
{
    if (pCCC == NULL || pColors_in_out == NULL ||
	    pColors_in_out->format == XCMS_UNDEFINED_FORMAT) {
	return(XCMS_FAILURE);
    }

    if (nColors == 0 || pColors_in_out->format == newFormat) {
	/* do nothing */
	return(XCMS_SUCCESS);
    }

    if (XCMS_DI_ID(pColors_in_out->format) && XCMS_DI_ID(newFormat)) {
	/*
	 * Device-Independent to Device-Independent Conversion
	 */
	return(_XcmsDIConvertColors(pCCC, pColors_in_out, pWhitePt, nColors,
		newFormat));
    }
    if (XCMS_DD_ID(pColors_in_out->format) && XCMS_DD_ID(newFormat)) {
	/*
	 * Device-Dependent to Device-Dependent Conversion
	 */
	return(_XcmsDDConvertColors(pCCC, pColors_in_out, nColors, newFormat,
		pCompressed));
    }

    /*
     * Otherwise we have:
     *    1. Device-Independent to Device-Dependent Conversion
     *		OR
     *    2. Device-Dependent to Device-Independent Conversion
     */

    if (XCMS_DI_ID(pColors_in_out->format)) {
	/*
	 *    1. Device-Independent to Device-Dependent Conversion
	 */
	/* First convert to CIEXYZ */
	if (_XcmsDIConvertColors(pCCC, pColors_in_out, pWhitePt, nColors,
		XCMS_CIEXYZ_FORMAT) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	/* Then convert to DD Format */
	return(_XcmsDDConvertColors(pCCC, pColors_in_out, nColors, newFormat,
		pCompressed));
    } else {
	/*
	 *    2. Device-Dependent to Device-Independent Conversion
	 */
	/* First convert to CIEXYZ */
	if (_XcmsDDConvertColors(pCCC, pColors_in_out, nColors,
		XCMS_CIEXYZ_FORMAT, pCompressed) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	/* Then convert to DI Format */
	return(_XcmsDIConvertColors(pCCC, pColors_in_out, pWhitePt, nColors,
		newFormat));
    }
}
