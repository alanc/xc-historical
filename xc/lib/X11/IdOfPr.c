/* $XConsortium: XcmsIdOfPr.c,v 1.3 91/01/22 22:16:46 alt Exp $" */

/*
 * (c) Copyright 1989 1990 1991 Tektronix Inc.
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
 *		XcmsIdOfPr.c
 *
 *	DESCRIPTION
 *		Source for XcmsIDofPrefix()
 *
 *
 */

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
#include "Xlib.h"
#include "TekCMS.h"


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#include "TekCMSext.h"

/*
 *      EXTERNS
 */
extern XcmsColorSpace **_XcmsDIColorSpaces;
extern XcmsColorSpace **_XcmsDDColorSpaces;



/*
 *	NAME
 *		XcmsIDofPrefix
 *
 *	SYNOPSIS
 */
XcmsSpecFmt
XcmsIDofPrefix(prefix)
    char *prefix;
/*
 *	DESCRIPTION
 *		Returns the Color Space ID for the specified prefix
 *		if the color space is found in the Color Conversion
 *		Context.
 *
 *	RETURNS
 *		Color Space ID if found; zero otherwise.
 */
{
    XcmsColorSpace	**papColorSpaces;

    /*
     * First try Device-Independent color spaces
     */
    papColorSpaces = _XcmsDIColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strncmp((*papColorSpaces)->prefix, prefix,
		    strlen((*papColorSpaces)->prefix)) == 0) {
		return((*papColorSpaces)->id);
	    }
	    papColorSpaces++;
	}
    }

    /*
     * Next try Device-Dependent color spaces
     */
    papColorSpaces = _XcmsDDColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strncmp((*papColorSpaces)->prefix, prefix,
		    strlen((*papColorSpaces)->prefix)) == 0) {
		return((*papColorSpaces)->id);
	    }
	    papColorSpaces++;
	}
    }

    return(XCMS_UNDEFINED_FORMAT);
}
