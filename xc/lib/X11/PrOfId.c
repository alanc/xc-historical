/* $XConsortium: XcmsPrOfId.c,v 1.2 91/02/11 18:17:54 dave Exp $" */

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
 *		XcmsPrOfId.c
 *
 *	DESCRIPTION
 *		Source for XcmsPrefixOfID()
 *
 *
 */

#include "Xlibos.h"
#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *      EXTERNS
 */
extern XcmsColorSpace **_XcmsDIColorSpaces;
extern XcmsColorSpace **_XcmsDDColorSpaces;


/*
 *	NAME
 *		XcmsPrefixOfId
 *
 *	SYNOPSIS
 */
char *
XcmsPrefixOfID(id)
    XcmsSpecFmt	id;
/*
 *	DESCRIPTION
 *		Returns the color space prefix for the specified color
 *		space ID if the color space is found in the Color
 *		Conversion Context.
 *
 *	RETURNS
 *		Returns a color space prefix.
 *
 *	CAVEATS
 *		Space is allocated for the returned string, therefore,
 *		the application is responsible for freeing (using XFree)
 *		the space.
 *
 */
{
    XcmsColorSpace	**papColorSpaces;
    char *prefix;

    /*
     * First try Device-Independent color spaces
     */
    papColorSpaces = _XcmsDIColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if ((*papColorSpaces)->id == id) {
		prefix = (char *)Xmalloc((strlen((*papColorSpaces)->prefix) +
		1) * sizeof(char));
		strcpy(prefix, (*papColorSpaces)->prefix);
		return(prefix);
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
	    if ((*papColorSpaces)->id == id) {
		prefix = (char *)Xmalloc((strlen((*papColorSpaces)->prefix) +
		1) * sizeof(char));
		strcpy(prefix, (*papColorSpaces)->prefix);
		return(prefix);
	    }
	    papColorSpaces++;
	}
    }

    return(NULL);
}
