/* $XConsortium: XcmsCvCols.c,v 1.2 91/02/11 18:17:33 dave Exp $" */

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
 *	NAME
 *		XcmsCvCols.c
 *
 *	DESCRIPTION
 *		TekCMS API routine that converts between the
 *		device-independent color spaces.
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"
#include "Xlibos.h"

/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern XcmsRegColorSpaceEntry _XcmsRegColorSpaces[];
extern XcmsColorSpace **_XcmsDIColorSpaces;
extern XcmsColorSpace **_XcmsDDColorSpaces;

/*
 *      LOCAL DEFINES
 */
#define	DD_FORMAT	0x01
#define	DI_FORMAT	0x02
#define	MIX_FORMAT	0x04
#ifndef MAX
#  define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif


/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		EqualCIEXYZ
 *
 *	SYNOPSIS
 */
static int
EqualCIEXYZ(p1, p2)
    XcmsColor *p1, *p2;
/*
 *	DESCRIPTION
 *		Compares two XcmsColor structures that are in XCMS_CIEXYZ_FORMAT
 *
 *	RETURNS
 *		Returns 1 if equal; 0 otherwise.
 *
 */
{
    if (p1->format != XCMS_CIEXYZ_FORMAT || p2->format != XCMS_CIEXYZ_FORMAT) {
	return(0);
    }
    if ((p1->spec.CIEXYZ.X != p2->spec.CIEXYZ.X)
	    || (p1->spec.CIEXYZ.Y != p2->spec.CIEXYZ.Y)
	    || (p1->spec.CIEXYZ.Z != p2->spec.CIEXYZ.Z)) {
	return(0);
    }
    return(1);
}


/*
 *	NAME
 *		XcmsColorSpace
 *
 *	SYNOPSIS
 */
static XcmsColorSpace *
ColorSpaceOfID(pCCC, id)
    XcmsCCC *pCCC;
    XcmsSpecFmt	id;
/*
 *	DESCRIPTION
 *		Returns a pointer to the color space structure
 *		(XcmsColorSpace) associated with the specified color space
 *		ID.
 *
 *	RETURNS
 *		Pointer to matching XcmsColorSpace structure if found;
 *		otherwise NULL.
 */
{
    XcmsColorSpace	**papColorSpaces;

    if (pCCC == NULL) {
	return(NULL);
    }

    /*
     * First try Device-Independent color spaces
     */
    papColorSpaces = _XcmsDIColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if ((*papColorSpaces)->id == id) {
		return(*papColorSpaces);
	    }
	    papColorSpaces++;
	}
    }

    /*
     * Next try Device-Dependent color spaces
     */
    papColorSpaces = ((XcmsSCCFuncSet *)pCCC->pPerScrnInfo->pSCCFuncSet)->papDDColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if ((*papColorSpaces)->id == id) {
		return(*papColorSpaces);
	    }
	    papColorSpaces++;
	}
    }

    return(NULL);
}


/*
 *	NAME
 *		ValidDIColorSpaceID
 *
 *	SYNOPSIS
 */
static int
ValidDIColorSpaceID(id)
    XcmsSpecFmt id;
/*
 *	DESCRIPTION
 *		Determines if the specified color space ID is a valid
 *		Device-Independent color space in the specified Color
 *		Conversion Context.
 *
 *	RETURNS
 *		Returns zero if not valid; otherwise non-zero.
 */
{
    XcmsColorSpace **papRec;
    papRec = _XcmsDIColorSpaces;
    if (papRec != NULL) {
	while (*papRec != NULL) {
	    if ((*papRec)->id == id) {
		return(1);
	    }
	    papRec++;
	}
    }
    return(0);
}


/*
 *	NAME
 *		ValidDDColorSpaceID
 *
 *	SYNOPSIS
 */
static int
ValidDDColorSpaceID(pCCC, id)
    XcmsCCC *pCCC;
    XcmsSpecFmt id;
/*
 *	DESCRIPTION
 *		Determines if the specified color space ID is a valid
 *		Device-Dependent color space in the specified Color
 *		Conversion Context.
 *
 *	RETURNS
 *		Returns zero if not valid; otherwise non-zero.
 */
{
    XcmsColorSpace **papRec;

    if (pCCC->pPerScrnInfo->state == XCMS_INIT_SUCCESS ||
	    pCCC->pPerScrnInfo->state == XCMS_INIT_DEFAULT) {
	papRec = ((XcmsSCCFuncSet *)pCCC->pPerScrnInfo->pSCCFuncSet)->papDDColorSpaces;
	while (*papRec != NULL) {
	    if ((*papRec)->id == id) {
		return(1);
	    }
	    papRec++;
	}
    }
    return(0);
}


/************************************************************************
 *									*
 *			 API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsEqualWhitePts
 *
 *	SYNOPSIS
 */
int
_XcmsEqualWhitePts(pCCC, pWhitePt1, pWhitePt2)
    XcmsCCC *pCCC;
    XcmsColor *pWhitePt1, *pWhitePt2;
/*
 *	DESCRIPTION
 *
 *	RETURNS
 *		Returns 0 if not equal; otherwise 1.
 *
 */
{
    XcmsColor tmp1, tmp2;

    bcopy((char *)pWhitePt1, (char *)&tmp1, sizeof(XcmsColor));
    bcopy((char *)pWhitePt2, (char *)&tmp2, sizeof(XcmsColor));

    if (tmp1.format != XCMS_CIEXYZ_FORMAT) {
	if (_XcmsDIConvertColors(pCCC, &tmp1, (XcmsColor *) NULL, 1,
		XCMS_CIEXYZ_FORMAT)==0) {
	    return(0);
	}
    }

    if (tmp2.format != XCMS_CIEXYZ_FORMAT) {
	if (_XcmsDIConvertColors(pCCC, &tmp2, (XcmsColor *) NULL, 1,
		XCMS_CIEXYZ_FORMAT)==0) {
	    return(0);
	}
    }

    return (EqualCIEXYZ(&tmp1, &tmp2));
}


/*
 *	NAME
 *		_XcmsDIConvertColors - Convert XcmsColor structures
 *
 *	SYNOPSIS
 */
Status
_XcmsDIConvertColors(pCCC, pColors_in_out, pWhitePt, nColors,
	newFormat)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    XcmsColor *pWhitePt;
    unsigned int nColors;
    XcmsSpecFmt newFormat;
/*
 *	DESCRIPTION
 *		Convert XcmsColor structures to another Device-Independent
 *		form.
 *
 *		Here are some assumptions that this routine makes:
 *		1. The calling routine has already checked if
 *		    pColors_in_out->format == newFormat, therefore
 *		    there is no need to check again here.
 *		2. The calling routine has already checked nColors,
 *		    therefore this routine assumes nColors > 0.
 *		3. The calling routine may want to convert only between
 *			CIExyY <-> CIEXYZ <-> CIEuvY
 *		    therefore, this routine allows pWhitePt to equal NULL.
 *		
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression,
 *
 */
{
    XcmsColorSpace *pFrom, *pTo;
    XcmsFuncPtr *to_CIEXYZ, *from_CIEXYZ;
    XcmsFuncPtr *to_CIEXYZ_stop, *from_CIEXYZ_start;

    /*
     * Allow pWhitePt to equal NULL.  This appropriate when converting
     *    anywhere between:
     *		CIExyY <-> CIEXYZ <-> CIEuvY
     */

    if (pColors_in_out == NULL ||
	    !ValidDIColorSpaceID(pColors_in_out->format) ||
	    !ValidDIColorSpaceID(newFormat)) {
	return(XCMS_FAILURE);
    }

    /*
     * Get a handle on the function list for the current specification format
     */
    if ((pFrom = ColorSpaceOfID(pCCC, pColors_in_out->format))
	    == NULL) {
	return(XCMS_FAILURE);
    }
    to_CIEXYZ = pFrom->to_CIEXYZ;

    /*
     * Get a handle on the function list for the new specification format
     */
    if ((pTo = ColorSpaceOfID(pCCC, newFormat)) == NULL) {
	return(XCMS_FAILURE);
    }
    from_CIEXYZ = pTo->from_CIEXYZ;

    /*
     * Find common function pointers
     */
    for (to_CIEXYZ_stop = to_CIEXYZ; *to_CIEXYZ_stop; to_CIEXYZ_stop++) {
	if (*to_CIEXYZ_stop == *from_CIEXYZ) {
	    break;
	}
    }
    from_CIEXYZ_start = from_CIEXYZ;
    if (*from_CIEXYZ_start && (to_CIEXYZ_stop != to_CIEXYZ)) {
	while (*(to_CIEXYZ_stop - 1) == *(from_CIEXYZ_start + 1)) {
	    to_CIEXYZ_stop--;
	    from_CIEXYZ_start++;
	}
    }

    /*
     * Execute the functions
     */
    while (to_CIEXYZ != to_CIEXYZ_stop) {
	if (!(*to_CIEXYZ++)(pCCC, pWhitePt, pColors_in_out,
		nColors)) {
	    return(XCMS_FAILURE);
	}
    }
    while (*from_CIEXYZ_start) {
	if (!(*from_CIEXYZ_start++)(pCCC, pWhitePt, pColors_in_out,
		nColors)) {
	    return(XCMS_FAILURE);
	}
    }

    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		_XcmsDDConvertColors - Convert XcmsColor structures
 *
 *	SYNOPSIS
 */
Status
_XcmsDDConvertColors(pCCC, pColors_in_out, nColors, newFormat,
	pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    XcmsSpecFmt newFormat;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Convert XcmsColor structures:
 *
 *		1. From CIEXYZ to Device-Dependent formats (typically RGB and
 *			RGBi),
 *		    or
 *		2. Between Device-Dependent formats (typically RGB and RGBi).
 *
 *		Assumes that these specifications have already been white point
 *		adjusted if necessary from Client White Point to Screen
 *		White Point.  Therefore, the white point now associated
 *		with the specifications is the Screen White Point.
 *
 *		pCompressed may be NULL.  If so this indicates that the
 *		calling routine is not interested in knowing exactly which
 *		color was compressed, if any.
 *
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression,
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with
 *				gamut compression.
 *
 */
{
    XcmsColorSpace *pFromRec, *pToRec;
    XcmsFuncPtr *from_CIEXYZ, *to_CIEXYZ;
    XcmsFuncPtr *from_CIEXYZ_start, *to_CIEXYZ_stop;
    int	retval;
    int hasCompressed = 0;

    if (pCCC == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    if (nColors == 0 || pColors_in_out->format == newFormat) {
	/* do nothing */
	return(XCMS_SUCCESS);
    }

    if (((XcmsSCCFuncSet *)pCCC->pPerScrnInfo->pSCCFuncSet) == NULL) {
	return(XCMS_FAILURE);	/* hmm, an internal error? */
    }

    /*
     * Its ok if pColors_in_out->format == XCMS_CIEXYZ_FORMAT
     *	or 
     * if newFormat == XCMS_CIEXYZ_FORMAT
     */
    if ( !( ValidDDColorSpaceID(pCCC, pColors_in_out->format)
	    ||
	    (pColors_in_out->format == XCMS_CIEXYZ_FORMAT))
	 ||
	 !(ValidDDColorSpaceID(pCCC, newFormat)
	    ||
	    newFormat == XCMS_CIEXYZ_FORMAT)) {
	return(XCMS_FAILURE);
    }

    if ((pFromRec = ColorSpaceOfID(pCCC, pColors_in_out->format)) == NULL){
	return(XCMS_FAILURE);
    }

    if ((pToRec = ColorSpaceOfID(pCCC, newFormat)) == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Find common function pointers
     */
    to_CIEXYZ = pFromRec->to_CIEXYZ;
    from_CIEXYZ = pToRec->from_CIEXYZ;
    for (to_CIEXYZ_stop = pFromRec->to_CIEXYZ; *to_CIEXYZ_stop; to_CIEXYZ_stop++) {
	if (*to_CIEXYZ_stop == *from_CIEXYZ) {
	    break;
	}
    }
    from_CIEXYZ_start = pToRec->from_CIEXYZ;
    if (*from_CIEXYZ_start && (to_CIEXYZ_stop != to_CIEXYZ)) {
	while (*(to_CIEXYZ_stop - 1) == *(from_CIEXYZ_start + 1)) {
	    to_CIEXYZ_stop--;
	    from_CIEXYZ_start++;
	}
    }

    /*
     * Execute the functions
     */
    while (to_CIEXYZ != to_CIEXYZ_stop) {
	retval = (*to_CIEXYZ++)(pCCC, pColors_in_out, nColors, pCompressed);
	if (retval== XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	hasCompressed |= (retval == XCMS_SUCCESS_WITH_COMPRESSION);
    }
    while (*from_CIEXYZ_start) {
	retval = (*from_CIEXYZ_start++)(pCCC, pColors_in_out, nColors,
		pCompressed);
	if (retval== XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	hasCompressed |= (retval == XCMS_SUCCESS_WITH_COMPRESSION);
    }

    return(hasCompressed ? XCMS_SUCCESS_WITH_COMPRESSION : XCMS_SUCCESS);
}


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsConvertColors - Convert XcmsColor structures
 *
 *	SYNOPSIS
 */
Status
XcmsConvertColors(pCCC, pColors_in_out, nColors, targetFormat, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    XcmsSpecFmt targetFormat;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Convert XcmsColor structures to another format
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression,
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with gamut
 *			compression.
 *
 */
{
    XcmsColor clientWhitePt, tmpWhitePt;
    XcmsColor Color1;
    XcmsColor *pColors_tmp;
    XcmsColor *pColors_start;
    XcmsColor *pColor;
    int whiteAdj = 0;
    XcmsSpecFmt format;
    Status retval_tmp;
    Status retval;
    unsigned char contents_flag = 0x00;
    unsigned int count;

    if (pCCC == NULL || pColors_in_out == NULL ||
		!(ValidDIColorSpaceID(targetFormat) ||
		ValidDDColorSpaceID(pCCC, targetFormat))) {
	return(XCMS_FAILURE);
    }

    /*
     * Check formats in color specification array
     */
    format = pColors_in_out->format;
    for (pColors_tmp = pColors_in_out, count = nColors; count; pColors_tmp++, count--) {
	if (!(ValidDIColorSpaceID(pColors_tmp->format) ||
		ValidDDColorSpaceID(pCCC, pColors_tmp->format))) {
	    return(XCMS_FAILURE);
	}
	if (XCMS_DI_ID(pColors_tmp->format)) {
	    contents_flag |= DI_FORMAT;
	} else {
	    contents_flag |= DD_FORMAT;
	}
	if (pColors_tmp->format != format) {
	    contents_flag |= MIX_FORMAT;
	}
    }

    /*
     * Check if we need the Client White Point.
     */
    if ((contents_flag & DI_FORMAT) || XCMS_DI_ID(targetFormat)) {
	/* To proceed, we need to get the Client White Point */
	bcopy((char *)&pCCC->clientWhitePt, (char *)&clientWhitePt,
		sizeof(XcmsColor));
	if (clientWhitePt.format == XCMS_UNDEFINED_FORMAT) {
	    /*
	     * If the Client White Point is undefined, convert to the Screen
	     *   White Point
	     */
	    bcopy((char *)&pCCC->pPerScrnInfo->screenWhitePt, (char *)&clientWhitePt,
		    sizeof(XcmsColor));
	} else if (pCCC->whitePtAdjFunc != NULL) {
	    if (clientWhitePt.format != XCMS_CIEXYZ_FORMAT) {
		bcopy((char *)&clientWhitePt, (char *)&tmpWhitePt,
			sizeof(XcmsColor));
		if (_XcmsDIConvertColors(pCCC, &tmpWhitePt, (XcmsColor *) NULL,
			1, XCMS_CIEXYZ_FORMAT)==0){
		    return(XCMS_FAILURE);
		}
		if (!EqualCIEXYZ(&tmpWhitePt, &pCCC->pPerScrnInfo->screenWhitePt)) {
		    whiteAdj = 1;
		}
	    } else if (!EqualCIEXYZ(&clientWhitePt, &pCCC->pPerScrnInfo->screenWhitePt)) {
		whiteAdj = 1;
	    }
	}
    }

    /*
     * Make copy of array of color specifications
     */
    if (nColors > 1) {
	pColors_tmp = (XcmsColor *) Xmalloc(nColors * sizeof(XcmsColor));
    } else {
	pColors_tmp = &Color1;
    }
    bcopy((char *)pColors_in_out, (char *)pColors_tmp,
	    nColors * sizeof(XcmsColor));

    /*
     * zero out pCompressed
     */
    if (pCompressed) {
	bzero((char *)pCompressed, nColors * sizeof(Bool));
    }

    if (contents_flag == DD_FORMAT || contents_flag == DI_FORMAT) {
	/*
	 * ENTIRE ARRAY IS IN ONE FORMAT.
	 */
	if (XCMS_DI_ID(format) && XCMS_DI_ID(targetFormat)) {
	    /*
	     * DI-to-DI only conversion
	     */
	    retval = _XcmsDIConvertColors(pCCC, pColors_tmp,
		    &clientWhitePt, nColors, targetFormat);
	} else if (XCMS_DD_ID(format) && XCMS_DD_ID(targetFormat)) {
	    /*
	     * DD-to-DD only conversion
	     *   Since DD->DD there will be no compressed thus we can
	     *   pass NULL instead of pCompressed.
	     */
	    retval = _XcmsDDConvertColors(pCCC, pColors_tmp, nColors,
		    targetFormat, (Bool *)NULL);
	} else {
	    /*
	     * Otherwise we have:
	     *    1. Device-Independent to Device-Dependent Conversion
	     *		OR
	     *    2. Device-Dependent to Device-Independent Conversion
	     *
	     *  We need to go from oldFormat -> CIEXYZ -> targetFormat
	     *	adjusting for white points as necessary.
	     */

	    if (XCMS_DI_ID(format)) {
		/*
		 *    1. Device-Independent to Device-Dependent Conversion
		 */
		if (whiteAdj) {
		    /*
		     * White Point Adjustment
		     *		Client White Point to Screen White Point
		     */
		    retval = (*pCCC->whitePtAdjFunc)(pCCC, &clientWhitePt,
			    &pCCC->pPerScrnInfo->screenWhitePt,
			    targetFormat, pColors_tmp, nColors, pCompressed);
		} else {
		    if (_XcmsDIConvertColors(pCCC, pColors_tmp,
			    &clientWhitePt, nColors, XCMS_CIEXYZ_FORMAT)
			    == XCMS_FAILURE) {
			goto Failure;
		    }
		    retval = _XcmsDDConvertColors(pCCC, pColors_tmp, nColors,
			    targetFormat, pCompressed);
		}
	    } else {
		/*
		 *    2. Device-Dependent to Device-Independent Conversion
		 */
		if (whiteAdj) {
		    /*
		     * White Point Adjustment
		     *		Screen White Point to Client White Point
		     */
		    retval = (*pCCC->whitePtAdjFunc)(pCCC,
			    &pCCC->pPerScrnInfo->screenWhitePt, &clientWhitePt,
			    targetFormat, pColors_tmp, nColors, pCompressed);
		} else {
		    /*
		     * Since DD->CIEXYZ, no compression takes place therefore
		     * we can pass NULL instead of pCompressed.
		     */
		    if (_XcmsDDConvertColors(pCCC, pColors_tmp, nColors,
			    XCMS_CIEXYZ_FORMAT, (Bool *)NULL) == XCMS_FAILURE) {
			goto Failure;
		    }
		    retval = _XcmsDIConvertColors(pCCC, pColors_tmp,
			    &clientWhitePt, nColors, targetFormat);
		}
	    }
	}
    } else {
	/*
	 * ARRAY HAS MIXED FORMATS.
	 */
	if ((contents_flag == (DI_FORMAT | MIX_FORMAT)) &&
		XCMS_DI_ID(targetFormat)) {
	    /*
	     * Convert from DI to DI in batches of contiguous formats
	     */
	    count = 0;
	    while (count < nColors) {
		/*
		 * Find contiguous array of color specifications with the same
		 * format
		 */
		pColor = pColors_start = pColors_tmp + count;
		format = pColors_start->format;
		while (count < nColors && pColor->format == format) {
			pColor++;
			count++;
		}
		if ((retval_tmp = _XcmsDIConvertColors(pCCC, pColors_start,
			&clientWhitePt, count, targetFormat)) ==
			XCMS_FAILURE) {
		    goto Failure;
		}
		retval = MAX(retval, retval_tmp);
	    }
	} else if ((contents_flag == (DD_FORMAT | MIX_FORMAT)) &&
		XCMS_DD_ID(targetFormat)) {
	    /*
	     * Convert from DD to DD in batches of contiguous formats
	     */
	    count = 0;
	    while (count < nColors) {
		/*
		 * Find contiguous array of color specifications with the same
		 * format
		 */
		pColor = pColors_start = pColors_tmp + count;
		format = pColors_start->format;
		while (count < nColors && pColor->format == format) {
			pColor++;
			count++;
		}
		/*
		 * Since DD->DD, no compression takes place therefore
		 * we can pass NULL instead of pCompressed.
		 */
		if ((retval_tmp = _XcmsDDConvertColors(pCCC, pColors_start,
			count, targetFormat, (Bool *)NULL)) == XCMS_FAILURE) {
		    goto Failure;
		}
		retval = MAX(retval, retval_tmp);
	    }
	} else {
	    /*
	     * We need to convert from DI-to-DD or DD-to-DI, therefore
	     *   1. convert in batches to CIEXYZ, then
	     *   2. convert all to the target format.
	     *
	     *   This allows white point adjustment and gamut compression
	     *	 to be applied to all the color specifications in one
	     *   swoop if those functions do in fact modify the entire
	     *   group of color specifications.
	     */

	    /*
	     * Convert in batches to CIEXYZ
	     */
	    count = 0;
	    while (count < nColors) {
		/*
		 * Find contiguous array of color specifications with the same
		 * format
		 */
		pColor = pColors_start = pColors_tmp + count;
		format = pColors_start->format;
		while (count < nColors && pColor->format == format) {
			pColor++;
			count++;
		}
		if (XCMS_DI_ID(format)) {
		    if ((retval_tmp = _XcmsDIConvertColors(pCCC, pColors_start,
			    &clientWhitePt, count, XCMS_CIEXYZ_FORMAT)) ==
			    XCMS_FAILURE) {
			goto Failure;
		    }
		} else {
		    /*
		     * Since DD->CIEXYZ, no compression takes place therefore
		     * we can pass NULL instead of pCompressed.
		     */
		    if ((retval_tmp = _XcmsDDConvertColors(pCCC, pColors_start,
			    count, XCMS_CIEXYZ_FORMAT, (Bool *)NULL)) == XCMS_FAILURE) {
			goto Failure;
		    }
		}
		retval = MAX(retval, retval_tmp);
	    }

	    /*
	     * Convert all to target format
	     */
	    if (XCMS_DI_ID(targetFormat)) {
		if ((retval_tmp = _XcmsDIConvertColors(pCCC, pColors_tmp,
			&clientWhitePt, nColors, targetFormat)) ==
			XCMS_FAILURE) {
		    goto Failure;
		}
	    } else {
		if ((retval_tmp = _XcmsDDConvertColors(pCCC, pColors_tmp,
			nColors, targetFormat, (Bool *)NULL))
			== XCMS_FAILURE) {
		    goto Failure;
		}
	    }
	    retval = MAX(retval, retval_tmp);
	}
    }

    if (retval != XCMS_FAILURE) {
	bcopy((char *)pColors_tmp, (char *)pColors_in_out,
		nColors * sizeof(XcmsColor));
    }
    if (nColors > 1) {
	Xfree((char *)pColors_tmp);
    }
    return(retval);

Failure:
    if (nColors > 1) {
	Xfree((char *)pColors_tmp);
    }
    return(XCMS_FAILURE);
}


/*
 *	NAME
 *		XcmsRegIdOfPrefix
 *
 *	SYNOPSIS
 */
XcmsSpecFmt
_XcmsRegIdOfPrefix(prefix)
    char *prefix;
/*
 *	DESCRIPTION
 *		Returns a color space ID associated with the specified
 *		X Consortium registered color space prefix.
 *
 *	RETURNS
 *		The color space ID if found;
 *		otherwise NULL.
 */
{
    XcmsRegColorSpaceEntry *pEntry = _XcmsRegColorSpaces;

    while (pEntry->prefix != NULL) {
	if (strcmp(prefix, pEntry->prefix) == 0) {
	    return(pEntry->id);
	}
	pEntry++;
    }
    return(XCMS_UNDEFINED_FORMAT);
}

