/* $XConsortium: XcmsAddSF.c,v 1.2 91/02/11 18:17:19 dave Exp $" */

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
 *		XcmsAddSF.c
 *
 *	DESCRIPTION
 *		Source for XcmsAddSCCFuncSet
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *      DEFINES
 */
#define NextUnregDdCsID(lastid) \
	    (XCMS_UNREG_ID(lastid) ? ++lastid : XCMS_FIRST_UNREG_DD_ID)
#define MIN(x,y) ((x) > (y) ? (y) : (x))


/*
 *      EXTERNS
 */
extern caddr_t *_XcmsPushPointerArray();
extern XcmsSpecFmt _XcmsRegIdOfPrefix();
extern XcmsSCCFuncSet **_XcmsSCCFuncSets;
extern XcmsColorSpace **_XcmsDDColorSpaces;



/*
 *	NAME
 *		XcmsAddSCCFuncSet - Add an Screen Color Characterization
 *					Function Set
 *
 *	SYNOPSIS
 */
Status
XcmsAddSCCFuncSet(pNewFS)
    XcmsSCCFuncSet *pNewFS;
/*
 *	DESCRIPTION
 *		Additional Screen Color Characterization Function Sets are
 *		managed on a global basis.  This means that with exception
 *		of the provided DD color spaces:
 *			    RGB and RGBi
 *		DD color spaces may have different XcmsSpecFmt IDs between
 *		clients.  So, you must be careful when using XcmsSpecFmt
 *		across clients!  Use the routines XcmsIDofPrefix()
 *		and XcmsPrefixOfID() appropriately.
 *
 *	RETURNS
 *		XCMS_SUCCESS if succeeded, otherwise XCMS_FAILURE
 *
 *	CAVEATS
 *		Additional Screen Color Characterization Function Sets
 *		should be added prior to any use of the routine
 *		XcmsCreateCCC().  If not, XcmsCCC structures created
 *		prior to the call of this routines will not have had
 *		a chance to initialize using the added Screen Color
 *		Characterization Function Set.
 */
{
    XcmsSCCFuncSet **papSCCFuncSets = _XcmsSCCFuncSets;
    XcmsColorSpace **papNewCSs;
    XcmsColorSpace *pNewCS, **paptmpCS;
    XcmsSpecFmt lastID = 0;


    if (papSCCFuncSets != NULL) {
	if ((papNewCSs = pNewFS->papDDColorSpaces) == NULL) {
	    /*
	     * Error, new Screen Color Characterization Function Set
	     *	missing color spaces
	     */
	    return(XCMS_FAILURE);
	}
	while ((pNewCS = *papNewCSs++) != NULL) {
	    if ((pNewCS->id = _XcmsRegIdOfPrefix(pNewCS->prefix)) != 0) {
		if (XCMS_DI_ID(pNewCS->id)) {
		    /* This is a Device-Independent Color Space */
		    return(XCMS_FAILURE);
		}
		/*
		 * REGISTERED DD Color Space
		 *    therefore use the registered ID.
		 */
	    } else {
		/*
		 * UNREGISTERED DD Color Space
		 *    then see if the color space is already in
		 *    _XcmsDDColorSpaces.
		 *	    a. If same prefix, then use the same ID.
		 *	    b. Otherwise, use a new ID.
		 */
		for (paptmpCS = _XcmsDDColorSpaces; *paptmpCS != NULL;
			paptmpCS++){
		    lastID = MIN(lastID, (*paptmpCS)->id);
		    if (strcmp(pNewCS->prefix, (*paptmpCS)->prefix) == 0) {
			pNewCS->id = (*paptmpCS)->id;
			break;
		    }
		}
		if (pNewCS->id == 0) {
		    /* still haven't found one */
		    pNewCS->id = NextUnregDdCsID(lastID);
		    if ((paptmpCS = (XcmsColorSpace **)_XcmsPushPointerArray(
		   	    (caddr_t *) _XcmsDDColorSpaces,
			    (caddr_t) pNewCS)) == NULL) {
			return(XCMS_FAILURE);
		    }
		    _XcmsDDColorSpaces = paptmpCS;
		}
	    }
	}
    }
    if ((papSCCFuncSets = (XcmsSCCFuncSet **)
	    _XcmsPushPointerArray((caddr_t *) _XcmsSCCFuncSets,
	    (caddr_t) pNewFS)) == NULL){
	return(XCMS_FAILURE);
    }
    _XcmsSCCFuncSets = papSCCFuncSets;

    return(XCMS_SUCCESS);
}
