/* $XConsortium: UNDEFINED.c,v 1.3 91/01/22 22:14:57 alt Exp $" */

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
 *		UNDEFINED.c
 *
 *	DESCRIPTION
 *		UNDEFINED Color Space
 *
 *
 */

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
#include "TekCMS.h"


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#include "TekCMSext.h"

/*
 *      FORWARD DECLARATIONS
 */
static int ReturnZero();


/*
 *      LOCALS VARIABLES
 */

static XcmsFuncPtr Fl_ReturnZero[] = {
    ReturnZero,
    NULL
};




/*
 *      GLOBALS
 *              Variables declared in this package that are allowed
 *		to be used globally.
 */
    /*
     * UNDEFINED Color Space
     */
XcmsColorSpace	XcmsUNDEFINED_ColorSpace =
    {
	"undefined",		/* prefix */
	XCMS_UNDEFINED_FORMAT,	/* id */
	ReturnZero,		/* parseString */
	Fl_ReturnZero,		/* to_CIEXYZ */
	Fl_ReturnZero		/* from_CIEXYZ */
    };



/************************************************************************
 *									*
 *			PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		ReturnZero
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
static int
ReturnZero(pLab_WhitePt, pColors_in_out, nColors)
    XcmsColor *pLab_WhitePt;
    XcmsColor pColors_in_out;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Does nothing.
 *
 *	RETURNS
 *		0
 *
 */
{
    return(0);
}
