/* $Xconsortium: CIEXYZ.c,v 1.3 91/01/22 22:13:51 alt Exp $" */

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
 *		CIEXYZ.c
 *
 *	DESCRIPTION
 *		CIE XYZ Color Space
 *
 *
 */

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#include <string.h>
#include "TekCMS.h"
#include "TekCMSext.h"


/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern char XcmsCIEXYZ_prefix[];

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
 *      FORWARD DECLARATIONS
 */
static int CIEXYZ_ParseString();
Status XcmsCIEXYZ_ValidSpec();


/*
 *      LOCALS VARIABLES
 */

static XcmsFuncPtr Fl_CIEXYZ_to_CIEXYZ[] = {
    NULL
};



/*
 *      GLOBALS
 *              Variables declared in this package that are allowed
 *		to be used globally.
 */
    /*
     * CIE XYZ Color Space
     */
XcmsColorSpace	XcmsCIEXYZ_ColorSpace =
    {
	XcmsCIEXYZ_prefix,		/* prefix */
	XCMS_CIEXYZ_FORMAT,		/* id */
	CIEXYZ_ParseString,	/* parseString */
	Fl_CIEXYZ_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_CIEXYZ	/* from_CIEXYZ */
    };


/************************************************************************
 *									*
 *			PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		CIEXYZ_ParseString
 *
 *	SYNOPSIS
 */
static int
CIEXYZ_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_CIEXYZ_FORMAT.
 *		The assumed CIEXYZ string syntax is:
 *		    CIEXYZ:<X>/<Y>/<Z>
 *		Where X, Y, and Z are in string input format for floats
 *		consisting of:
 *		    a. an optional sign
 *		    b. a string of numbers possibly containing a decimal point,
 *		    c. an optional exponent field containing an 'E' or 'e'
 *			followed by a possibly signed integer string.
 *
 *	RETURNS
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
    if (strncmp(spec, XcmsCIEXYZ_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.CIEXYZ.X,
	    &pColor->spec.CIEXYZ.Y,
	    &pColor->spec.CIEXYZ.Z) != 3) {
	return(XCMS_FAILURE);
    }
    pColor->format = XCMS_CIEXYZ_FORMAT;
    pColor->pixel = 0;
    return(XcmsCIEXYZ_ValidSpec(pColor));
}


/************************************************************************
 *									*
 *			PUBLIC ROUTINES 				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsCIELab_ValidSpec
 *
 *	SYNOPSIS
 */
Status
XcmsCIEXYZ_ValidSpec(pColor)
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Checks if color specification valid for CIE XYZ
 *
 *	RETURNS
 *		XCMS_FAILURE if invalid,
 *		XCMS_SUCCESS if valid.
 *
 */
{
    if (pColor->format != XCMS_CIEXYZ_FORMAT
	    ||
	    (pColor->spec.CIEXYZ.Y < 0.0 - XMY_DBL_EPSILON)
	    ||
	    (pColor->spec.CIEXYZ.Y > 1.0 + XMY_DBL_EPSILON)) {
	return(XCMS_FAILURE);
    }
    return(XCMS_SUCCESS);
}
