/* $Xconsortium: CIELuv.c,v 1.12 91/01/28 14:31:03 alt Exp $" */

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
 *		CIELuv.c
 *
 *	DESCRIPTION
 *		This file contains routines that support the CIE L*u*v*
 *		color space to include conversions to and from the CIE
 *		XYZ space.
 *
 *	DOCUMENTATION
 *		"TekColor Color Management System, System Implementor's Manual"
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
#include <string.h>
#include "TekCMSext.h"
#include "XcmsMath.h"

/*
 *	EXTERNS
 */

extern char XcmsCIELuv_prefix[];
extern int XcmsCIEuvY_to_CIEXYZ();
extern int XcmsCIEXYZ_to_CIEuvY();


/*
 *	FORWARD DECLARATIONS
 */

static int CIELuv_ParseString();
Status XcmsCIELuv_ValidSpec();
Status XcmsCIELuv_to_CIEuvY();
Status XcmsCIEuvY_to_CIELuv();

/*
 *	DEFINES
 *		Internal definitions that need NOT be exported to any package
 *		or program using this package.
 */
#ifdef DBL_EPSILON
#  define XMY_DBL_EPSILON DBL_EPSILON
#else
#  define XMY_DBL_EPSILON 0.00001
#endif


/*
 *	LOCAL VARIABLES
 */

    /*
     * NULL terminated list of functions applied to get from CIELuv to CIEXYZ
     */
static XcmsFuncPtr Fl_CIELuv_to_CIEXYZ[] = {
    XcmsCIELuv_to_CIEuvY,
    XcmsCIEuvY_to_CIEXYZ,
    NULL
};

    /*
     * NULL terminated list of functions applied to get from CIEXYZ to CIELuv
     */
static XcmsFuncPtr Fl_CIEXYZ_to_CIELuv[] = {
    XcmsCIEXYZ_to_CIEuvY,
    XcmsCIEuvY_to_CIELuv,
    NULL
};

/*
 *	GLOBALS
 */

    /*
     * CIE Luv Color Space
     */
XcmsColorSpace	XcmsCIELuv_ColorSpace =
    {
	XcmsCIELuv_prefix,	/* prefix */
	XCMS_CIELuv_FORMAT,		/* id */
	CIELuv_ParseString,	/* parseString */
	Fl_CIELuv_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_CIELuv	/* from_CIEXYZ */
    };


/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIELuv_ParseString
 *
 *	SYNOPSIS
 */
static int
CIELuv_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_CIELuv_FORMAT.
 *		The assumed CIELuv string syntax is:
 *		    CIELuv:<L>/<u>/<v>
 *		Where L, u, and v are in string input format for floats
 *		consisting of:
 *		    a. an optional sign
 *		    b. a string of numbers possibly containing a decimal point,
 *		    c. an optional exponent field containing an 'E' or 'e'
 *			followed by a possibly signed integer string.
 *
 *	RETURNS
 *		0 if failed, non-zero otherwise.
 */
{
    int n;
    char *pchar;

    if ((pchar = strchr(spec, ':')) == NULL) {
	return(XCMS_FAILURE);
    }
    n = (int)(pchar - spec);

    /*
     * Check for proper prefix.
     */
    if (strncmp(spec, XcmsCIELuv_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.CIELuv.L,
	    &pColor->spec.CIELuv.u,
	    &pColor->spec.CIELuv.v) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_CIELuv_FORMAT;
    pColor->pixel = 0;
    return(XcmsCIELuv_ValidSpec(pColor));
}


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCIELuv_ValidSpec
 *
 *	SYNOPSIS
 */
Status
XcmsCIELuv_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if color specification valid for CIE L*u*v*.
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid,
 *		XCMS_SUCCESS if valid.
 *
 */
{
    if (pColor->format != XCMS_CIELuv_FORMAT
	    ||
	    (pColor->spec.CIELuv.L < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIELuv.L > 1.0 + XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIELuv_to_CIEuvY - convert CIELuv to CIEuvY
 *
 *	SYNOPSIS
 */
Status
XcmsCIELuv_to_CIEuvY(pCCC, pLuv_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pLuv_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIELuv format to CIEuvY format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsColor	*pColor = pColors_in_out;
    XcmsColor	whitePt;
    XcmsCIEuvY	uvY_return;
    XcmsFloat	tmpVal;
    register int i;

    /*
     * Check arguments
     */
    if (pLuv_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEuvY form
     */
    if (pLuv_WhitePt->format != XCMS_CIEuvY_FORMAT) {
	/* Make copy of the white point because we're going to modify it */
	bcopy((char *)pLuv_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL,
		1, XCMS_CIEuvY_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pLuv_WhitePt = &whitePt;
    }
    /* Make sure it is a white point, i.e., Y == 1.0 */
    if (pLuv_WhitePt->spec.CIEuvY.Y != 1.0) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is CIELuv and is valid */
	if (!XcmsCIELuv_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	if (pColor->spec.CIELuv.L < 0.0799953624) {
	    uvY_return.Y = pColor->spec.CIELuv.L / 9.0329;
	} else {
	    tmpVal = (pColor->spec.CIELuv.L + 0.16) / 1.16;
	    uvY_return.Y = tmpVal * tmpVal * tmpVal; /* tmpVal ** 3 */
	}



	if (pColor->spec.CIELuv.L == 0.0) {
	    uvY_return.u = pLuv_WhitePt->spec.CIEuvY.u;
	    uvY_return.v = pLuv_WhitePt->spec.CIEuvY.v;
	} else {
	    uvY_return.u = (pColor->spec.CIELuv.u + 
			    (13.0 * pColor->spec.CIELuv.L * 
			     pLuv_WhitePt->spec.CIEuvY.u)) / 
			   (13.0 * pColor->spec.CIELuv.L);
	    uvY_return.v = (pColor->spec.CIELuv.v +
			    (13.0 * pColor->spec.CIELuv.L *
			     pLuv_WhitePt->spec.CIEuvY.v)) /
			    (13.0 * pColor->spec.CIELuv.L);
	}
	/* Copy result to pColor */
	bcopy ((char *)&uvY_return, (char *)&pColor->spec, sizeof(XcmsCIEuvY));

	/* Identify that the format is now CIEuvY */
	pColor->format = XCMS_CIEuvY_FORMAT;
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIEuvY_to_CIELuv - convert CIEuvY to CIELuv
 *
 *	SYNOPSIS
 */
Status
XcmsCIEuvY_to_CIELuv(pCCC, pLuv_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pLuv_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIEuvY format to CIELab format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsColor	*pColor = pColors_in_out;
    XcmsColor	whitePt;
    XcmsCIELuv	Luv_return;
    register int i;

    /*
     * Check arguments
     */
    if (pLuv_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEuvY form
     */
    if (pLuv_WhitePt->format != XCMS_CIEuvY_FORMAT) {
	/* Make copy of the white point because we're going to modify it */
	bcopy((char *)pLuv_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt,
		(XcmsColor *)NULL, 1, XCMS_CIEuvY_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pLuv_WhitePt = &whitePt;
    }
    /* Make sure it is a white point, i.e., Y == 1.0 */
    if (pLuv_WhitePt->spec.CIEuvY.Y != 1.0) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	if (!XcmsCIEuvY_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/* Now convert the uvY to Luv */
	Luv_return.L = 
	    (pColor->spec.CIEuvY.Y < 0.008856)
	    ?
	    (pColor->spec.CIEuvY.Y * 9.0329)
	    :
	    ((XcmsFloat)(XCMS_CUBEROOT(pColor->spec.CIEuvY.Y) * 1.16) - 0.16);
	Luv_return.u = 13.0 * Luv_return.L * 
		       (pColor->spec.CIEuvY.u - pLuv_WhitePt->spec.CIEuvY.u);
	Luv_return.v = 13.0 * Luv_return.L *
		       (pColor->spec.CIEuvY.v - pLuv_WhitePt->spec.CIEuvY.v);

	/* Copy result to pColor */
	bcopy ((char *)&Luv_return, (char *)&pColor->spec, sizeof(XcmsCIELuv));

	/* Identify that the format is now CIEuvY */
	pColor->format = XCMS_CIELuv_FORMAT;
    }
    return(XCMS_SUCCESS);
}
