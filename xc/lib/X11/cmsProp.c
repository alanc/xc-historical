/* $XConsortium: XcmsProp.c,v 1.5 91/02/11 18:17:56 dave Exp $" */

/*
 * (c) Copyright 1990 1991 Tektronix Inc.
 * 	All Rights Reserved
 *
 * This code, which implements the TekColor Human Interface and/or the TekHVC
 * Color Space algorithms, is proprietary to Tektronix, Inc., and permission
 * is granted for use only in the form supplied.  Revisions, modifications,
 * or * adaptations are not permitted without the prior written approval of
 * Tektronix, Inc., Beaverton, OR 97077.  Code and supporting documentation
 * copyright Tektronix, Inc. 1990 1991 All rights reserved. TekColor and TekHVC
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
 *		XcmsProp.c
 *
 *	DESCRIPTION
 *		This utility routines for manipulating properties.
 *
 */

#include <X11/Xatom.h>
#include "Xlibint.h"
#include "Xcmsint.h"


/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/


/*
 *	NAME
 *		_XcmsGetElement -- get an element value from the property passed
 *
 *	SYNOPSIS
 */
int
_XcmsGetElement (format, pValue) 
    int             format;
    char            **pValue;
/*
 *	DESCRIPTION
 *	    Get the next element from the property and return it.
 *	    Also increment the pointer the amount needed.
 *
 *	Returns
 *	    int
 */
{
    char      *pChar;
    int       *pInt;
    short int *pShort;

    switch (format) {
      case 32:
	pInt = (int *) *pValue;
	*pValue += 4;
	return((int) *pInt);
	break;
      case 16:
	pShort = (short int *) *pValue;
	*pValue += 2;
	return((int) *pShort);
	break;
      case 8:
	pChar = *pValue;
	*pValue += 1;
	return((int) *pChar);
      default:
	break;
    }
    return(0);
}


/*
 *	NAME
 *		_XcmsGetProperty -- Determine the existance of a property
 *
 *	SYNOPSIS
 */
int
_XcmsGetProperty (pDpy, w, property, pFormat, pNItems, pNBytes, pValue) 
    Display *pDpy;
    Window  w;
    Atom property;
    int             *pFormat;
    unsigned long   *pNItems;
    unsigned long   *pNBytes;
    char            **pValue;
/*
 *	DESCRIPTION
 *
 *	Returns
 *	    0 if property does not exist.
 *	    1 if property exists.
 */
{
    char *prop_ret;
    int format_ret;
    long len = 6516;
    unsigned long nitems_ret, after_ret;
    Atom atom_ret;
    
    while (XGetWindowProperty (pDpy, w, property, 0, len, False, 
			       XA_INTEGER, &atom_ret, &format_ret, 
			       &nitems_ret, &after_ret, 
			       (unsigned char **)&prop_ret)) {
	if (after_ret > 0) {
	    len += nitems_ret * (format_ret >> 3);
	    XFree (prop_ret);
	} else {
	    break;
	}
    }
    if (format_ret == 0 || nitems_ret == 0) { 
	/* the property does not exist or is of an unexpected type */
	return(XCMS_FAILURE);
    }

    *pFormat = format_ret;
    *pNItems = nitems_ret;
    *pNBytes = nitems_ret * (format_ret >> 3);
    *pValue = prop_ret;
    return(XCMS_SUCCESS);
}
