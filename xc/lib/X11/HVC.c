/* $XConsortium: TekHVC.c,v 1.1 91/01/30 17:55:03 dave Exp $" */

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
 *		TekHVC.c
 *
 *	DESCRIPTION
 *		This file contains routines that support the TekHVC
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
#include "TekCMSext.h"
#include "XcmsMath.h"

/*
 *	DEFINES
 */
#define u_BR    0.7127          /* u' Best Red */
#define v_BR    0.4931          /* v' Best Red */
#define EPS     0.001
#define CHROMA_SCALE_FACTOR   7.50725
#ifndef PI
#  ifdef M_PI
#    define PI	M_PI
#  else
#    define PI       3.14159265358979323846264338327950
#  endif
#endif
#ifndef degrees
#  define degrees(r) ((XcmsFloat)(r) * 180.0 / PI)
#endif /* degrees */
#ifndef radians
#  define radians(d) ((XcmsFloat)(d) * PI / 180.0)
#endif /* radians */

/*************************************************************************
 * Note: The DBL_EPSILON for ANSI is 1e-5 so my checks need to take
 *       this into account.  If your DBL_EPSILON is different then
 *       adjust this define. 
 *
 *       Also note that EPS is the error factor in the calculations
 *       This may need to be the same as XMY_DBL_EPSILON in
 *       some implementations.
 **************************************************************************/
#ifdef DBL_EPSILON
#  define XMY_DBL_EPSILON DBL_EPSILON
#else
#  define XMY_DBL_EPSILON 0.00001
#endif

/*
 *	EXTERNS
 */

extern char XcmsTekHVC_prefix[];
extern Status XcmsCIEuvY_to_CIEXYZ();
extern Status XcmsCIEXYZ_to_CIEuvY();

/*
 *	FORWARD DECLARATIONS
 */

static int TekHVC_ParseString();
Status XcmsTekHVC_ValidSpec();
Status XcmsTekHVC_to_CIEuvY();
Status XcmsCIEuvY_to_TekHVC();



/*
 *	LOCAL VARIABLES
 */

    /*
     * NULL terminated list of functions applied to get from TekHVC to CIEXYZ
     */
static XcmsFuncPtr Fl_TekHVC_to_CIEXYZ[] = {
    XcmsTekHVC_to_CIEuvY,
    XcmsCIEuvY_to_CIEXYZ,
    NULL
};

    /*
     * NULL terminated list of functions applied to get from CIEXYZ to TekHVC
     */
static XcmsFuncPtr Fl_CIEXYZ_to_TekHVC[] = {
    XcmsCIEXYZ_to_CIEuvY,
    XcmsCIEuvY_to_TekHVC,
    NULL
};

/*
 *	GLOBALS
 */

    /*
     * TekHVC Color Space
     */
XcmsColorSpace	XcmsTekHVC_ColorSpace =
    {
	XcmsTekHVC_prefix,	/* prefix */
	XCMS_TekHVC_FORMAT,		/* id */
	TekHVC_ParseString,	/* parseString */
	Fl_TekHVC_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_TekHVC	/* from_CIEXYZ */
    };




/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		TekHVC_ParseString
 *
 *	SYNOPSIS
 */
static int
TekHVC_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_TekHVC_FORMAT.
 *		The assumed TekHVC string syntax is:
 *		    TekHVC:<H>/<V>/<C>
 *		Where H, V, and C are in string input format for floats
 *		consisting of:
 *		    a. an optional sign
 *		    b. a string of numbers possibly containing a decimal point,
 *		    c. an optional exponent field containing an 'E' or 'e'
 *			followed by a possibly signed integer string.
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid;
 *		XCMS_SUCCESS if valid.
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
    if (strncmp(spec, XcmsTekHVC_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.TekHVC.H,
	    &pColor->spec.TekHVC.V,
	    &pColor->spec.TekHVC.C) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_TekHVC_FORMAT;
    pColor->pixel = 0;
    return(XcmsTekHVC_ValidSpec(pColor));
}


/*
 *	NAME
 *		ThetaOffset -- compute thetaOffset
 *
 *	SYNOPSIS
 */
static int
ThetaOffset(pWhitePt, pThetaOffset)
    XcmsColor *pWhitePt;
    XcmsFloat *pThetaOffset;
/*
 *	DESCRIPTION
 *		This routine computes the theta offset of a given
 *		white point, i.e. XcmsColor.  It is used in both this
 *		conversion and the printer conversions.
 *
 *	RETURNS
 *		0 if failed.
 *		1 if succeeded with no modifications.
 *
 *	ASSUMPTIONS
 *		Assumes:
 *			pWhitePt != NULL
 *			pWhitePt->format == XCMS_CIEuvY_FORMAT
 *
 */
{
    double div, slopeuv;

    if (pWhitePt == NULL || pWhitePt->format != XCMS_CIEuvY_FORMAT) {
	return(0);
    }

    if ((div = u_BR - pWhitePt->spec.CIEuvY.u) == 0.0) {
	return(0);
    }
    slopeuv = (v_BR - pWhitePt->spec.CIEuvY.v) / div;
    *pThetaOffset = degrees(atan(slopeuv));
    return(1);
}



/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsTekHVC_ValidSpec()
 *
 *	SYNOPSIS
 */
int
XcmsTekHVC_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if values in the color specification are valid.
 *		Also brings hue into the range 0.0 <= Hue < 360.0
 *
 *	RETURNS
 *		0 if not valid.
 *		1 if valid.
 *
 */
{
    if (pColor->format != XCMS_TekHVC_FORMAT) {
	return(XCMS_FAILURE);
    }
    if (pColor->spec.TekHVC.V < (0.0 - XMY_DBL_EPSILON)
	    || pColor->spec.TekHVC.V > (100.0 + XMY_DBL_EPSILON)
	    || (pColor->spec.TekHVC.C < 0.0 - XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }

    if (pColor->spec.TekHVC.V < 0.0) {
	    pColor->spec.TekHVC.V = 0.0 + XMY_DBL_EPSILON;
    } else if (pColor->spec.TekHVC.V > 100.0) {
	pColor->spec.TekHVC.V = 100.0 - XMY_DBL_EPSILON;
    }

    if (pColor->spec.TekHVC.C < 0.0) {
	pColor->spec.TekHVC.C = 0.0 - XMY_DBL_EPSILON;
    }

    while (pColor->spec.TekHVC.H < 0.0) {
	pColor->spec.TekHVC.H += 360.0;
    }
    while (pColor->spec.TekHVC.H >= 360.0) {
	pColor->spec.TekHVC.H -= 360.0;
    } 
    return(XCMS_SUCCESS);
}

/*
 *	NAME
 *		XcmsTekHVC_to_CIEuvY - convert TekHVC to CIEuvY
 *
 *	SYNOPSIS
 */
Status
XcmsTekHVC_to_CIEuvY(pCCC, pHVC_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pHVC_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Transforms an array of TekHVC color specifications, given
 *		their associated white point, to CIECIEuvY.color
 *		specifications.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed, XCMS_SUCCESS otherwise.
 *
 */
{
    XcmsFloat	thetaOffset;
    XcmsColor	*pColor = pColors_in_out;
    XcmsColor	whitePt;
    XcmsCIEuvY	uvY_return;
    XcmsFloat	tempHue, u, v;
    XcmsFloat	tmpVal;
    register int i;

    /*
     * Check arguments
     */
    if (pHVC_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEuvY form
     */
    if (pHVC_WhitePt->format != XCMS_CIEuvY_FORMAT) {
	/* Make copy of the white point because we're going to modify it */
	bcopy((char *)pHVC_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL, 1,
		XCMS_CIEuvY_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pHVC_WhitePt = &whitePt;
    }
    /* Make sure it is a white point, i.e., Y == 1.0 */
    if (pHVC_WhitePt->spec.CIEuvY.Y != 1.0) {
	return(XCMS_FAILURE);
    }

    /* Get the thetaOffset */
    if (!ThetaOffset(pHVC_WhitePt, &thetaOffset)) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {

	/* Make sure original format is TekHVC and is valid */
	if (!XcmsTekHVC_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	if (pColor->spec.TekHVC.V == 0.0 || pColor->spec.TekHVC.V == 100.0) {
	    if (pColor->spec.TekHVC.V == 100.0) {
		uvY_return.Y = 1.0;
	    } else { /* pColor->spec.TekHVC.V == 0.0 */
		uvY_return.Y = 0.0;
	    }
	    uvY_return.u = pHVC_WhitePt->spec.CIEuvY.u;
	    uvY_return.v = pHVC_WhitePt->spec.CIEuvY.v;
	} else {

	    /* Find the hue based on the white point offset */
	    tempHue = pColor->spec.TekHVC.H + thetaOffset;

	    while (tempHue < 0.0) {
		tempHue += 360.0;
	    }
	    while (tempHue >= 360.0) {
		tempHue -= 360.0;
	    }

	    tempHue = radians(tempHue);

	    /* Calculate u'v' for the obtained hue */
	    u = (XcmsFloat) ((cos(tempHue) * pColor->spec.TekHVC.C) / 
		    (pColor->spec.TekHVC.V * (double)CHROMA_SCALE_FACTOR));
	    v = (XcmsFloat) ((sin(tempHue) * pColor->spec.TekHVC.C) / 
		    (pColor->spec.TekHVC.V * (double)CHROMA_SCALE_FACTOR));

	    /* Based on the white point get the offset from best red */
	    uvY_return.u = u + pHVC_WhitePt->spec.CIEuvY.u;
	    uvY_return.v = v + pHVC_WhitePt->spec.CIEuvY.v;

	    /* Calculate the Y value based on the L* = V. */
	    if (pColor->spec.TekHVC.V < 7.99953624) {
		uvY_return.Y = pColor->spec.TekHVC.V / 903.29;
	    } else {
		tmpVal = (pColor->spec.TekHVC.V + 16.0) / 116.0;
		uvY_return.Y = tmpVal * tmpVal * tmpVal; /* tmpVal ** 3 */
	    }
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
 *		XcmsCIEuvY_to_TekHVC - convert CIEuvY to TekHVC
 *
 *	SYNOPSIS
 */
Status
XcmsCIEuvY_to_TekHVC(pCCC, pHVC_WhitePt, pColors_in_out, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pHVC_WhitePt;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Transforms an array of CIECIEuvY.color specifications, given
 *		their assiciated white point, to TekHVC specifications.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed, XCMS_SUCCESS otherwise.
 *
 */
{
    XcmsFloat	theta, L2, u, v, nThetaLow, nThetaHigh;
    XcmsFloat	thetaOffset;
    XcmsColor	*pColor = pColors_in_out;
    XcmsColor	whitePt;
    XcmsTekHVC	HVC_return;
    register int i;

    /*
     * Check arguments
     */
    if (pHVC_WhitePt == NULL || pColors_in_out == NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * Make sure white point is in CIEuvY form
     */
    if (pHVC_WhitePt->format != XCMS_CIEuvY_FORMAT) {
	/* Make copy of the white point because we're going to modify it */
	bcopy((char *)pHVC_WhitePt, (char *)&whitePt, sizeof(XcmsColor));
	if (!_XcmsDIConvertColors(pCCC, &whitePt, (XcmsColor *)NULL, 1,
		XCMS_CIEuvY_FORMAT)) {
	    return(XCMS_FAILURE);
	}
	pHVC_WhitePt = &whitePt;
    }
    /* Make sure it is a white point, i.e., Y == 1.0 */
    if (pHVC_WhitePt->spec.CIEuvY.Y != 1.0) {
	return(XCMS_FAILURE);
    }
    if (!ThetaOffset(pHVC_WhitePt, &thetaOffset)) {
	return(XCMS_FAILURE);
    }

    /*
     * Now convert each XcmsColor structure to CIEXYZ form
     */
    for (i = 0; i < nColors; i++, pColor++) {
	if (!XcmsCIEuvY_ValidSpec(pColor)) {
	    return(XCMS_FAILURE);
	}

	/* Use the white point offset to determine HVC */
	u = pColor->spec.CIEuvY.u - pHVC_WhitePt->spec.CIEuvY.u;
	v = pColor->spec.CIEuvY.v - pHVC_WhitePt->spec.CIEuvY.v;

	/* Calculate the offset */
	if (u == 0.0) {
	    theta = 0.0;
	} else {
	    theta = v / u;
	    theta = (XcmsFloat) atan((double)theta);
	    theta = degrees(theta);
	}

	nThetaLow = 0.0;
	nThetaHigh = 360.0;
	if (u > 0.0 && v > 0.0) {
	    nThetaLow = 0.0;
	    nThetaHigh = 90.0;
	} else if (u < 0.0 && v > 0.0) {
	    nThetaLow = 90.0;
	    nThetaHigh = 180.0;
	} else if (u < 0.0 && v < 0.0) {
	    nThetaLow = 180.0;
	    nThetaHigh = 270.0;
	} else if (u > 0.0 && v < 0.0) {
	    nThetaLow = 270.0;
	    nThetaHigh = 360.0;
	}
	while (theta < nThetaLow) {
		theta += 90.0;
	}
	while (theta >= nThetaHigh) {
	    theta -= 90.0;
	}

	/* calculate the L value from the given Y */
	L2 = (pColor->spec.CIEuvY.Y < 0.008856)
	    ?
	    (pColor->spec.CIEuvY.Y * 903.29)
	    :
	    ((XcmsFloat)(XCMS_CUBEROOT(pColor->spec.CIEuvY.Y) * 116.0) - 16.0);
	HVC_return.C = L2 * CHROMA_SCALE_FACTOR * sqrt((double) ((u * u) + (v * v)));
	if (HVC_return.C < 0.0) {
	    theta = 0.0;
	}
	HVC_return.V = L2;
	HVC_return.H = theta - thetaOffset;

	/*
	 * If this is within the error margin let some other routine later
	 * in the chain worry about the slop in the calculations.
	 */
	while (HVC_return.H < -EPS) {
	    HVC_return.H += 360.0;
	}
	while (HVC_return.H >= 360.0 + EPS) {
	    HVC_return.H -= 360.0;
	}

	/* Copy result to pColor */
	bcopy ((char *)&HVC_return, (char *)&pColor->spec, sizeof(XcmsTekHVC));

	/* Identify that the format is now CIEuvY */
	pColor->format = XCMS_TekHVC_FORMAT;
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		_XcmsTekHVC_CheckModify
 *
 *	SYNOPSIS
 */
int
_XcmsTekHVC_CheckModify(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if values in the color specification are valid.
 *		If they are not it modifies the values.
 *		Also brings hue into the range 0.0 <= Hue < 360.0
 *
 *	RETURNS
 *		0 if not valid.
 *		1 if valid.
 *
 */
{
    /* For now only use the TekHVC numbers as inputs */
    if (pColor->format != XCMS_TekHVC_FORMAT) {
	return(0);
    }

    if (pColor->spec.TekHVC.V < 0.0) {
	pColor->spec.TekHVC.V = 0.0 + XMY_DBL_EPSILON;
    } else if (pColor->spec.TekHVC.V > 100.0) {
	pColor->spec.TekHVC.V = 100.0 - XMY_DBL_EPSILON;
    }

    if (pColor->spec.TekHVC.C < 0.0) {
	pColor->spec.TekHVC.C = 0.0 - XMY_DBL_EPSILON;
    }

    while (pColor->spec.TekHVC.H < 0.0) {
	pColor->spec.TekHVC.H += 360.0;
    }
    while (pColor->spec.TekHVC.H >= 360.0) {
	pColor->spec.TekHVC.H -= 360.0;
    } 
    return(1);
}
