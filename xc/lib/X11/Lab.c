/* $XConsortium: CIELab.c,v 1.1 91/01/30 16:52:38 dave Exp $ */

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
 *	    CIELab.c
 *
 *	DESCRIPTION
 *		This file contains routines that support the CIE L*a*b*
 *		color space to include conversions to and from the CIE
 *		XYZ space.  These conversions are from Principles of
 *		Color Technology Second Edition, Fred W. Billmeyer, Jr.
 *		and Max Saltzman, John Wiley & Sons, Inc., 1981.
 *
 *		Note that the range for L* is 0 to 1.
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
#include "TekCMSext.h"
#include "XcmsMath.h"

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
#define DIV16BY116	0.137931

/*
 *	EXTERNS
 */
extern char	XcmsCIELab_prefix[];


/*
 *	FORWARD DECLARATIONS
 */

static int CIELab_ParseString();
Status XcmsCIELab_ValidSpec();
Status XcmsCIELab_to_CIEXYZ();
Status XcmsCIEXYZ_to_CIELab();


/*
 *	LOCAL VARIABLES
 */


    /*
     * NULL terminated list of functions applied to get from CIELab to CIEXYZ
     */
static XcmsFuncPtr Fl_CIELab_to_CIEXYZ[] = {
    XcmsCIELab_to_CIEXYZ,
    NULL
};

    /*
     * NULL terminated list of functions applied to get from CIEXYZ to CIELab
     */
static XcmsFuncPtr Fl_CIEXYZ_to_CIELab[] = {
    XcmsCIEXYZ_to_CIELab,
    NULL
};


/*
 *	GLOBALS
 */
    /*
     * CIE Lab Color Space
     */
XcmsColorSpace	XcmsCIELab_ColorSpace =
    {
	XcmsCIELab_prefix,	/* prefix */
	XCMS_CIELab_FORMAT,		/* id */
	CIELab_ParseString,	/* parseString */
	Fl_CIELab_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_CIELab	/* from_CIEXYZ */
    };


/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIELab_ParseString
 *
 *	SYNOPSIS
 */
static int
CIELab_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_CIELab_FORMAT.
 *		The assumed CIELab string syntax is:
 *		    CIELab:<L>/<a>/<b>
 *		Where L, a, and b are in string input format for floats
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
    if (strncmp(spec, XcmsCIELab_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.CIELab.L,
	    &pColor->spec.CIELab.a,
	    &pColor->spec.CIELab.b) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_CIELab_FORMAT;
    pColor->pixel = 0;

    return(XcmsCIELab_ValidSpec(pColor));
}



/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCIELab_ValidSpec
 *
 *	SYNOPSIS
 */
Status
XcmsCIELab_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if color specification valid for CIE L*a*b*.
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid,
 *		XCMS_SUCCESS if valid.
 *
 */
{
    if (pColor->format != XCMS_CIELab_FORMAT
	    ||
	    (pColor->spec.CIELab.L < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIELab.L > 1.0 + XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsCIELab_to_CIEXYZ - convert CIELab to CIEXYZ
 *
 *	SYNOPSIS
 */
Status
XcmsCIELab_to_CIEXYZ(pCCC, pLab_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pLab_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIELab format to CIEXYZ format.
 *
 *		WARNING: This routine assumes that Yn = 1.0;
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsCIEXYZ XYZ_return;
    XcmsFloat tmpFloat, tmpL;
    XcmsColor whitePt;
    int i;
    XcmsColor *pColor = pColors_in_out;

    /*
     * Check arguments
     */
    if (pLab_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEXYZ form, if not, convert it.
     */
    if (pLab_WhitePt->format != XCMS_CIEXYZ_FORMAT) {
	/* Make a copy of the white point because we're going to modify it */
	bcopy((char *)pLab_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt,
		(XcmsColor *)NULL, 1, XCMS_CIEXYZ_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pLab_WhitePt = &whitePt;
    }

    /*
     * Make sure it is a white point, i.e., Y == 1.0
     */
    if (pLab_WhitePt->spec.CIEXYZ.Y != 1.0) {
	return (0);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is CIELab */
	if (!XcmsCIELab_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/* Calculate Y: assume that Yn = 1.0 */
	tmpL = (pColor->spec.CIELab.L + 0.16) / 1.16;
	XYZ_return.Y = tmpL * tmpL * tmpL;

	if (XYZ_return.Y < 0.008856) {
	    /* Calculate Y: assume that Yn = 1.0 */
	    tmpL = pColor->spec.CIELab.L / 0.0903292;

	    /* Calculate X */
	    XYZ_return.X = pLab_WhitePt->spec.CIEXYZ.X *
		    ((pColor->spec.CIELab.a / 3893.5) + tmpL);
	    /* Calculate Y */
	    XYZ_return.Y = tmpL;
	    /* Calculate Z */
	    XYZ_return.Z = pLab_WhitePt->spec.CIEXYZ.Z *
		    (tmpL - (pColor->spec.CIELab.b / 1557.4));
	} else {
	    /* Calculate X */
	    tmpFloat = tmpL + (pColor->spec.CIELab.a / 5.0);
	    XYZ_return.X = pLab_WhitePt->spec.CIEXYZ.X * tmpFloat * tmpFloat * tmpFloat;

	    /* Calculate Z */
	    tmpFloat = tmpL - (pColor->spec.CIELab.b / 2.0);
	    XYZ_return.Z = pLab_WhitePt->spec.CIEXYZ.Z * tmpFloat * tmpFloat * tmpFloat;
	}

	bcopy((char *)&XYZ_return, (char *)&pColor->spec.CIEXYZ,
		sizeof(XcmsCIEXYZ));
	pColor->format = XCMS_CIEXYZ_FORMAT;
    }

    return (1);
}


/*
 *	NAME
 *		XcmsCIEXYZ_to_CIELab - convert CIEXYZ to CIELab
 *
 *	SYNOPSIS
 */
Status
XcmsCIEXYZ_to_CIELab(pCCC, pLab_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pLab_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from CIEXYZ format to CIELab format.
 *
 *		WARNING: This routine assumes that Yn = 1.0;
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 *
 */
{
    XcmsCIELab Lab_return;
    XcmsFloat fX_Xn, fY_Yn, fZ_Zn;
    XcmsColor whitePt;
    int i;
    XcmsColor *pColor = pColors_in_out;

    /*
     * Check arguments
     */
    if (pLab_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEXYZ form, if not, convert it.
     */
    if (pLab_WhitePt->format != XCMS_CIEXYZ_FORMAT) {
	/* Make a copy of the white point because we're going to modify it */
	bcopy((char *)pLab_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL,
		1, XCMS_CIEXYZ_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pLab_WhitePt = &whitePt;
    }

    /*
     * Make sure it is a white point, i.e., Y == 1.0
     */
    if (pLab_WhitePt->spec.CIEXYZ.Y != 1.0) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is CIELab */
	if (!XcmsCIEXYZ_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/* Calculate L*:  assume Yn = 1.0 */
	if (pColor->spec.CIEXYZ.Y < 0.008856) {
	    fY_Yn = (0.07787 * pColor->spec.CIEXYZ.Y) + DIV16BY116;
	    /* note fY_Yn used to compute Lab_return.a below */
	    Lab_return.L = 1.16 * (fY_Yn - DIV16BY116);
	} else {
	    fY_Yn = (XcmsFloat)XCMS_CUBEROOT(pColor->spec.CIEXYZ.Y);
	    /* note fY_Yn used to compute Lab_return.a below */
	    Lab_return.L = (1.16 * fY_Yn) - 0.16;
	}

	/* Calculate f(X/Xn) */
	if ((fX_Xn = pColor->spec.CIEXYZ.X / pLab_WhitePt->spec.CIEXYZ.X) < 0.008856) {
	    fX_Xn = (0.07787 * fX_Xn) + DIV16BY116;
	} else {
	    fX_Xn = (XcmsFloat) XCMS_CUBEROOT(fX_Xn);
	}

	/* Calculate f(Z/Zn) */
	if ((fZ_Zn = pColor->spec.CIEXYZ.Z / pLab_WhitePt->spec.CIEXYZ.Z) < 0.008856) {
	    fZ_Zn = (0.07787 * fZ_Zn) + DIV16BY116;
	} else {
	    fZ_Zn = (XcmsFloat) XCMS_CUBEROOT(fZ_Zn);
	}

	Lab_return.a = 5.0 * (fX_Xn - fY_Yn);
	Lab_return.b = 2.0 * (fY_Yn - fZ_Zn);

	bcopy((char *)&Lab_return, (char *)&pColor->spec.CIELab,
		sizeof(XcmsCIELab));
	pColor->format = XCMS_CIELab_FORMAT;
    }

    return(XCMS_SUCCESS);
}
