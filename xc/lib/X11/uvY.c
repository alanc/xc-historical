/* $XConsortium: CIEuvY.c,v 1.2 91/02/05 13:40:49 dave Exp $" */

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
 *		CIEuvy.c
 *
 *	DESCRIPTION
 *		This file contains routines that support the CIE u'v'Y
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
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include "TekCMSext.h"


/*
 *	EXTERNS
 */
extern char	XcmsCIEuvY_prefix[];


/*
 *	FORWARD DECLARATIONS
 */

static int CIEuvY_ParseString();
Status XcmsCIEuvY_ValidSpec();
Status XcmsCIEuvY_to_CIEXYZ();
Status XcmsCIEXYZ_to_CIEuvY();
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
     * NULL terminated list of functions applied to get from CIEuvY to CIEXYZ
     */
static XcmsFuncPtr Fl_CIEuvY_to_CIEXYZ[] = {
    XcmsCIEuvY_to_CIEXYZ,
    NULL
};

    /*
     * NULL terminated list of functions applied to get from CIEXYZ to CIEuvY
     */
static XcmsFuncPtr Fl_CIEXYZ_to_CIEuvY[] = {
    XcmsCIEXYZ_to_CIEuvY,
    NULL
};


/*
 *	GLOBALS
 */

    /*
     * CIE uvY Color Space
     */
XcmsColorSpace	XcmsCIEuvY_ColorSpace =
    {
	XcmsCIEuvY_prefix,		/* prefix */
	XCMS_CIEuvY_FORMAT,		/* id */
	CIEuvY_ParseString,	/* parseString */
	Fl_CIEuvY_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_CIEuvY	/* from_CIEXYZ */
    };



/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIEuvY_ParseString
 *
 *	SYNOPSIS
 */
static int
CIEuvY_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_CIEuvY_FORMAT.
 *		The assumed CIEuvY string syntax is:
 *		    CIEuvY:<u>/<v>/<Y>
 *		Where u, v, and Y are in string input format for floats
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
    if (strncmp(spec, XcmsCIEuvY_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.CIEuvY.u,
	    &pColor->spec.CIEuvY.v,
	    &pColor->spec.CIEuvY.Y) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_CIEuvY_FORMAT;
    pColor->pixel = 0;
    return(XcmsCIEuvY_ValidSpec(pColor));
}


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCIEuvY_ValidSpec
 *
 *	SYNOPSIS
 */
Status
XcmsCIEuvY_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if color specification valid for CIE u'v'Y.
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid,
 *		XCMS_SUCCESS if valid.
 *
 */
{
    if (pColor->format != XCMS_CIEuvY_FORMAT
	    ||
	    (pColor->spec.CIEuvY.Y < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIEuvY.Y > 1.0 + XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIEuvY_to_CIEXYZ - convert CIEuvY to CIEXYZ
 *
 *	SYNOPSIS
 */
Status
XcmsCIEuvY_to_CIEXYZ(pCCC, puvY_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *puvY_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIEuvY format to CIEXYZ format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsCIEXYZ XYZ_return;
    XcmsColor whitePt;
    int i;
    XcmsColor *pColor = pColors_in_out;
    XcmsFloat div, x, y, z, Y;

    /*
     * Check arguments
     *	Postpone checking puvY_WhitePt until it is actually needed
     *	otherwise converting between XYZ and uvY will fail.
     */
    if (pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }


    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is CIEuvY */
	if (!XcmsCIEuvY_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/*
	 * Convert to CIEXYZ
	 */

	Y = pColor->spec.CIEuvY.Y;

	/* Convert color u'v' to xyz space */
	div = (6.0 * pColor->spec.CIEuvY.u) - (16.0 * pColor->spec.CIEuvY.v) + 12.0;
	if (div == 0.0) {
	    /* use white point since div == 0 */
	    if (puvY_WhitePt == NULL ) {
		return(XCMS_FAILURE);
	    }
	    /*
	     * Make sure white point is in CIEuvY form
	     */
	    if (puvY_WhitePt->format != XCMS_CIEuvY_FORMAT) {
		/* Make copy of the white point because we're going to modify it */
		bcopy((char *)puvY_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
		if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL, 1,
			XCMS_CIEuvY_FORMAT)) {
		    return(XCMS_FAILURE);
		}
		puvY_WhitePt = &whitePt;
	    }
	    /* Make sure it is a white point, i.e., Y == 1.0 */
	    if (puvY_WhitePt->spec.CIEuvY.Y != 1.0) {
		return(XCMS_FAILURE);
	    }
	    div = (6.0 * puvY_WhitePt->spec.CIEuvY.u) -
		    (16.0 * puvY_WhitePt->spec.CIEuvY.v) + 12.0;
	    if (div == 0) {
		/* internal error */
		return(XCMS_FAILURE);
	    }
	    x = 9.0 * puvY_WhitePt->spec.CIEuvY.u / div;
	    y = 4.0 * puvY_WhitePt->spec.CIEuvY.v / div;
	} else {
	    x = 9.0 * pColor->spec.CIEuvY.u / div;
	    y = 4.0 * pColor->spec.CIEuvY.v / div;
	}
	z = 1.0 - x - y;

	/* Convert from xyz to XYZ */
	/* Conversion uses color normalized lightness based on Y */
	if (y != 0.0) {
	    XYZ_return.X = x * Y / y;
	} else {
	    XYZ_return.X = x;
	}
	XYZ_return.Y = Y;
	if (y != 0.0) {
	    XYZ_return.Z = z * Y / y;
	} else {
	    XYZ_return.Z = z;
	}

	bcopy((char *)&XYZ_return, (char *)&pColor->spec.CIEXYZ, sizeof(XcmsCIEXYZ));
	/* Identify that format is now CIEXYZ */
	pColor->format = XCMS_CIEXYZ_FORMAT;
    }

    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIEXYZ_to_CIEuvY - convert CIEXYZ to CIEuvY
 *
 *	SYNOPSIS
 */
Status
XcmsCIEXYZ_to_CIEuvY(pCCC, puvY_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *puvY_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIEXYZ format to CIEuvY format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsCIEuvY uvY_return;
    XcmsColor whitePt;
    int i;
    XcmsColor *pColor = pColors_in_out;
    XcmsFloat div;

    /*
     * Check arguments
     *	Postpone checking puvY_WhitePt until it is actually needed
     *	otherwise converting between XYZ and uvY will fail.
     */
    if (pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEuvY form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is CIEXYZ */
	if (!XcmsCIEXYZ_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/* Convert to CIEuvY */
	div = pColor->spec.CIEXYZ.X + (15.0 * pColor->spec.CIEXYZ.Y) +
		(3.0 * pColor->spec.CIEXYZ.Z);
	if (div == 0.0) {
	    /* Use white point since div == 0.0 */
	    if (puvY_WhitePt == NULL ) {
		return(XCMS_FAILURE);
	    }
	    /*
	     * Make sure white point is in CIEuvY form
	     */
	    if (puvY_WhitePt->format != XCMS_CIEuvY_FORMAT) {
		/* Make copy of the white point because we're going to modify it */
		bcopy((char *)puvY_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
		if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL, 1,
			XCMS_CIEuvY_FORMAT)) {
		    return(XCMS_FAILURE);
		}
		puvY_WhitePt = &whitePt;
	    }
	    /* Make sure it is a white point, i.e., Y == 1.0 */
	    if (puvY_WhitePt->spec.CIEuvY.Y != 1.0) {
		return(XCMS_FAILURE);
	    }
	    uvY_return.Y = pColor->spec.CIEXYZ.Y;
	    uvY_return.u = puvY_WhitePt->spec.CIEuvY.u;
	    uvY_return.v = puvY_WhitePt->spec.CIEuvY.v;
	} else {
	    uvY_return.u = 4.0 * pColor->spec.CIEXYZ.X / div;
	    uvY_return.v = 9.0 * pColor->spec.CIEXYZ.Y / div;
	    uvY_return.Y = pColor->spec.CIEXYZ.Y;
	}

	bcopy((char *)&uvY_return, (char *)&pColor->spec.CIEuvY, sizeof(XcmsCIEuvY));
	/* Identify that format is now CIEuvY */
	pColor->format = XCMS_CIEuvY_FORMAT;
    }

    return(XCMS_SUCCESS);
}
