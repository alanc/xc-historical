/* $XConsortium: XcmsAddDIC.c,v 1.2 91/02/11 18:17:17 dave Exp $" */

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
 *		XcmsAddDIC.c
 *
 *	DESCRIPTION
 *		Source for XcmsAddDIColorSpace
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"


/*
 *      DEFINES
 */
#define NextUnregDiCsID(lastid) \
	    (XCMS_UNREG_ID(lastid) ? ++lastid : XCMS_FIRST_UNREG_DI_ID)
#define MAX(x,y) ((x) < (y) ? (y) : (x))


/*
 *      EXTERNS
 */
extern XcmsPerDpyInfo *_XcmsFindDpyInfo();
extern caddr_t *_XcmsPushPointerArray();
extern XcmsSpecFmt _XcmsRegIdOfPrefix();
extern XcmsColorSpace **_XcmsDIColorSpaces;

/*
 *	NAME
 *		XcmsAddDIColorSpace - Add a Device-Independent Color Space
 *
 *	SYNOPSIS
 */
Status
XcmsAddDIColorSpace(pCS)
    XcmsColorSpace *pCS;
/*
 *	DESCRIPTION
 *		DI Color Spaces are managed on a global basis.
 *		This means that with exception of the provided DI color spaces:
 *			CIEXYZ, CIExyY, CIELab, CIEuvY, CIELuv, and TekHVC
 *		DI color spaces may have different XcmsSpecFmt IDs between
 *		clients.  So, you must be careful when using XcmsColor
 *		structures between clients!  Use the routines XcmsIDofPrefix()
 *		and XcmsPrefixOfID() appropriately.
 *
 *	RETURNS
 *		XCMS_SUCCESS if succeeded, otherwise XCMS_FAILURE
 */
{
    XcmsColorSpace **papColorSpaces;
    XcmsColorSpace *ptmpCS;
    XcmsSpecFmt lastID = 0;

    if ((pCS->id = _XcmsRegIdOfPrefix(pCS->prefix)) != 0) {
	if (XCMS_DD_ID(pCS->id)) {
	    /* This is a Device-Dependent Color Space */
	    return(XCMS_FAILURE);
	}
	/*
	 * REGISTERED DI Color Space
	 *    then see if the color space has already been added to the
	 *    system:
	 *	    a. If the same ID/prefix and same XcmsColorSpec is found,
	 *		then its a duplicate, so return success.
	 *	    b. If same ID/prefix but different XcmsColorSpec is
	 *		found, then add the color space to the front of the
	 *		list using the same ID.  This allows one to override
	 *		an existing DI Color Space.
	 *	    c. Otherwise none found so just add the color space.
	 */
	if ((papColorSpaces = _XcmsDIColorSpaces) != NULL) {
	    while ((ptmpCS = *papColorSpaces++) != NULL) {
		if (pCS->id == ptmpCS->id) {
		    if (pCS == ptmpCS) {
			/* a. duplicate*/
			return(XCMS_SUCCESS);
		    }
		    /* b. same ID/prefix but different XcmsColorSpace */
		    break;
		}
	    }
	}
	/* c. None found */
    } else {
	/*
	 * UNREGISTERED DI Color Space
	 *    then see if the color space has already been added to the
	 *    system:
	 *	    a. If same prefix and XcmsColorSpec, then
	 *		its a duplicate ... return success.
	 *	    b. If same prefix but different XcmsColorSpec, then
	 *		add the color space to the front of the list using
	 *		the same ID.  This allows one to override an existing
	 *		DI Color Space.
	 *	    c. Otherwise none found so, add the color space using the
	 *		next unregistered ID for the connection.
	 */
	if ((papColorSpaces = _XcmsDIColorSpaces) != NULL) {
	    while ((ptmpCS = *papColorSpaces++) != NULL) {
		lastID = MAX(lastID, ptmpCS->id);
		if (strcmp(pCS->prefix, ptmpCS->prefix) == 0) {
		    if (pCS == ptmpCS) {
			/* a. duplicate */
			return(XCMS_SUCCESS);
		    }
		    /* b. same prefix but different XcmsColorSpec */
		    pCS->id = ptmpCS->id;
		    goto AddColorSpace;
		}
	    }
	}
	/* c. None found */
	pCS->id = NextUnregDiCsID(lastID);
    }


AddColorSpace:
    if ((papColorSpaces = (XcmsColorSpace **)
	    _XcmsPushPointerArray((caddr_t *)_XcmsDIColorSpaces,
	    (caddr_t)pCS)) == NULL) {
	return(XCMS_FAILURE);
    }
    _XcmsDIColorSpaces = papColorSpaces;
    return(XCMS_SUCCESS);
}
