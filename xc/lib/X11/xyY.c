/* $XConsortium: CIExyY.c,v 1.2 91/02/05 13:40:51 dave Exp $" */

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
 *		CIExyY.c
 *
 *	DESCRIPTION
 *		This file contains routines that support the CIE xyY
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
#include <stdio.h>
#include <X11/Xfuncs.h>
#include <X11/Xos.h>
#include "TekCMSext.h"

/*
 *	DEFINES
 */
#define EPS 0.00001	/* some extremely small number */
#ifdef DBL_EPSILON
#  define XMY_DBL_EPSILON DBL_EPSILON
#else
#  define XMY_DBL_EPSILON 0.00001
#endif

/*
 *	EXTERNS
 */

extern char XcmsCIExyY_prefix[];

/*
 *	FORWARD DECLARATIONS
 */

static int CIExyY_ParseString();
Status XcmsCIExyY_ValidSpec();
Status XcmsCIExyY_to_CIEXYZ();
Status XcmsCIEXYZ_to_CIExyY();


/*
 *	LOCAL VARIABLES
 */

    /*
     * NULL terminated list of functions applied to get from CIExyY to CIEXYZ
     */
static XcmsFuncPtr Fl_CIExyY_to_CIEXYZ[] = {
    XcmsCIExyY_to_CIEXYZ,
    NULL
};

    /*
     * NULL terminated list of functions applied to get from CIEXYZ to CIExyY
     */
static XcmsFuncPtr Fl_CIEXYZ_to_CIExyY[] = {
    XcmsCIEXYZ_to_CIExyY,
    NULL
};


/*
 *	GLOBALS
 */

    /*
     * CIE xyY Color Space
     */
XcmsColorSpace	XcmsCIExyY_ColorSpace =
    {
	XcmsCIExyY_prefix,	/* prefix */
	XCMS_CIExyY_FORMAT,		/* id */
	CIExyY_ParseString,	/* parseString */
	Fl_CIExyY_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_CIExyY	/* from_CIEXYZ */
    };



/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIExyY_ParseString
 *
 *	SYNOPSIS
 */
static int
CIExyY_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_CIExyY_FORMAT.
 *		The assumed CIExyY string syntax is:
 *		    CIExyY:<x>/<y>/<Y>
 *		Where x, y, and Y are in string input format for floats
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
    if (strncmp(spec, XcmsCIExyY_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.CIExyY.x,
	    &pColor->spec.CIExyY.y,
	    &pColor->spec.CIExyY.Y) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_CIExyY_FORMAT;
    pColor->pixel = 0;
    return(XcmsCIExyY_ValidSpec(pColor));
}



/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIExyY_ValidSpec()
 *
 *	SYNOPSIS
 */
Status
XcmsCIExyY_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks a valid CIExyY color specification.
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid.
 *		XCMS_SUCCESS if valid.
 *
 */
{
    if (pColor->format != XCMS_CIExyY_FORMAT
	    ||
	    (pColor->spec.CIExyY.x < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIExyY.x > 1.0 + XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIExyY.y < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIExyY.y > 1.0 + XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIExyY.Y < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIExyY.Y > 1.0 + XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIExyY_to_CIEXYZ - convert CIExyY to CIEXYZ
 *
 *	SYNOPSIS
 */
Status
XcmsCIExyY_to_CIEXYZ(pCCC, pxyY_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pxyY_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIExyY format to CIEXYZ format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 */
{
    XcmsColor	*pColor = pColors_in_out;
    XcmsColor	whitePt;
    XcmsCIEXYZ	XYZ_return;
    XcmsFloat	div;		/* temporary storage in case divisor is zero */
    XcmsFloat	u, v, x, y, z;	/* temporary storage */
    register int i;

    /*
     * Check arguments
     */
    if (pxyY_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }


    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {
	/* Make sure original format is CIExyY and valid */
	if (!XcmsCIExyY_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	if ((div = (-2 * pColor->spec.CIExyY.x) + (12 * pColor->spec.CIExyY.y) + 3) == 0.0) {
	    /* Note that the divisor is zero */
	    /* This return is abitrary. */
	    XYZ_return.X = 0;
	    XYZ_return.Y = 0;
	    XYZ_return.Z = 0;
	} else {
	    /*
	     * Make sure white point is in CIEXYZ form
	     */
	    if (pxyY_WhitePt->format != XCMS_CIEXYZ_FORMAT) {
		/* Make copy of the white point because we're going to modify it */
		bcopy((char *)pxyY_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
		if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL, 1,
			XCMS_CIEXYZ_FORMAT)) {
		    return(XCMS_FAILURE);
		}
		pxyY_WhitePt = &whitePt;
	    }

	    /* Make sure it is a white point, i.e., Y == 1.0 */
	    if (pxyY_WhitePt->spec.CIEXYZ.Y != 1.0) {
		return(XCMS_FAILURE);
	    }

	    /* Convert from xyY to uvY to XYZ */
	    u = (4 * pColor->spec.CIExyY.x) / div;
	    v = (9 * pColor->spec.CIExyY.y) / div;
	    div = (6.0 * u) - (16.0 * v) + 12.0;
	    if (div == 0.0) {
		/* Note that the divisor is zero */
		/* This return is abitrary. */
		if ((div = (6.0 * whitePt.spec.CIEuvY.u) -
		           (16.0 * whitePt.spec.CIEuvY.v) + 12.0) == 0.0) {
		    div = EPS;
		}
		x = 9.0 * whitePt.spec.CIEuvY.u / div;
		y = 4.0 * whitePt.spec.CIEuvY.u / div;
	    } else {
		/* convert u, v to small xyz */
		x = 9.0 * u / div;
		y = 4.0 * v / div;
	    }
	    z = 1.0 - x - y;
	    if (y == 0.0) y = EPS;	/* Have to worry about divide by 0 */
	    XYZ_return.Y = pColor->spec.CIExyY.Y;
	    XYZ_return.X = x * XYZ_return.Y / y;
	    XYZ_return.Z = z * XYZ_return.Y / y;
	}

	/* Copy result to pColor */
	bcopy ((char *)&XYZ_return, (char *)&pColor->spec, sizeof(XcmsCIEXYZ));

	/* Identify that the format is now CIEXYZ */
	pColor->format = XCMS_CIEXYZ_FORMAT;
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIEXYZ_to_CIExyY - convert CIEXYZ to CIExyY
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status
XcmsCIEXYZ_to_CIExyY(pCCC, pxyY_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pxyY_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIEXYZ format to CIExyY format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsColor	*pColor = pColors_in_out;
    XcmsCIExyY	xyY_return;
    XcmsFloat	div;		/* temporary storage in case divisor is zero */
    register int i;

    /*
     * Check arguments
     * 		pxyY_WhitePt ignored
     */
    if (pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	if (!XcmsCIEXYZ_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}
	/* Now convert for XYZ to xyY */
	if ((div = pColor->spec.CIEXYZ.X + pColor->spec.CIEXYZ.Y + pColor->spec.CIEXYZ.Z) == 0.0) {
	    div = EPS;
	} 
	xyY_return.x = pColor->spec.CIEXYZ.X / div;
	xyY_return.y = pColor->spec.CIEXYZ.Y / div;
	xyY_return.Y = pColor->spec.CIEXYZ.Y;

	/* Copy result to pColor */
	bcopy ((char *)&xyY_return, (char *)&pColor->spec, sizeof(XcmsCIExyY));

	/* Identify that the format is now CIEXYZ */
	pColor->format = XCMS_CIExyY_FORMAT;
    }
    return(XCMS_SUCCESS);
}
