/* $XConsortium: XcmsInt.c,v 1.11 91/01/28 14:33:58 alt Exp $" */

/*
 * (c) Copyright 1990 1991, Tektronix Inc.
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
 *		XcmsInt.c - TekCMS API utility routines
 *
 *	DESCRIPTION
 *		TekCMS Application Program Interface (API) utility
 *		routines for hanging information directly onto
 *		the Display structure.
 *
 *
 */



/*
 *      EXTERNAL INCLUDES
 */
/* #define NEED_EVENTS */
#include "TekCMS.h"

/*
 *      INTERNAL INCLUDES
 */
#include <stdio.h>
#include "Xlibos.h"
#include "TekCMSP.h"

/*
 *      EXTERNS
 */
extern XcmsColorSpace **_XcmsDIColorSpaces;
extern XcmsSCCFuncSet **_XcmsSCCFuncSets;

/*
 *      GLOBALS
 */



/************************************************************************
 *									*
 *			   API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsCopyPointerArray
 *
 *	SYNOPSIS
 */
caddr_t *
_XcmsCopyPointerArray(pap)
    caddr_t *pap;
/*
 *	DESCRIPTION
 *		Copies an array of NULL terminated pointers.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the copy.
 *
 */
{
    caddr_t *newArray;
    char **tmp;
    int n;

    for (tmp = pap, n = 0; *tmp != NULL; tmp++, n++);
    n++; /* add 1 to include the NULL pointer */

    if (newArray = (caddr_t *)Xmalloc(n * sizeof(caddr_t))) {
	bcopy((char *)pap, (char *)newArray, (unsigned)(n * sizeof(caddr_t)));
    }
    return((caddr_t *)newArray);
}

/*
 *	NAME
 *		_XcmsFreePointerArray
 *
 *	SYNOPSIS
 */
void
_XcmsFreePointerArray(pap)
    caddr_t *pap;
/*
 *	DESCRIPTION
 *		Frees an array of NULL terminated pointers.
 *
 *	RETURNS
 *		void
 *
 */
{
    Xfree(pap);
}

/*
 *	NAME
 *		_XcmsPushPointerArray
 *
 *	SYNOPSIS
 */
caddr_t *
_XcmsPushPointerArray(pap, p)
    caddr_t *pap;
    caddr_t p;
/*
 *	DESCRIPTION
 *		Places the specified pointer at the head of an array of NULL
 *		terminated pointers.
 *
 *	RETURNS
 *		Returns NULL if failed; otherwise the address to
 *		the head of the array.
 *
 */
{
    caddr_t *newArray;
    char **tmp;
    int n;

    for (tmp = pap, n = 0; *tmp != NULL; tmp++, n++);

    /* add 2: 1 for the new pointer and another for the NULL pointer */
    n += 2;

    if (newArray = (caddr_t *)Xmalloc(n * sizeof(caddr_t))) {
	bcopy((char *)pap, (char *)(newArray+1),
		(unsigned)(n * sizeof(caddr_t)));
	*newArray = p;
    }
    _XcmsFreePointerArray(pap);
    return((caddr_t *)newArray);
}

/*
 *	NAME
 *		_XcmsInitDefaultCCCs
 *
 *	SYNOPSIS
 */
int
_XcmsInitDefaultCCCs(dpy)
    Display *dpy;
/*
 *	DESCRIPTION
 *		Initializes the TekCMS per Display Info structure
 *		(XcmsPerDpyInfo).
 *
 *	RETURNS
 *		Returns 0 if failed; otherwise non-zero.
 *
 */
{
    int nScrn = ScreenCount(dpy);
    int i;
    XcmsCCC *pCCC;

    if (nScrn <= 0) {
	return(0);
    }

    /*
     * Create an array of XcmsCCC structures, one for each screen.
     * They serve as the screen's default CCC.
     */
    if (!(pCCC = (XcmsCCC *)
	    Xcalloc((unsigned)nScrn, (unsigned) sizeof(XcmsCCC)))) {
	return(0);
    } 
    dpy->cms.defaultCCCs = (caddr_t)pCCC;

    for (i = 0; i < nScrn; i++, pCCC++) {
	pCCC->dpy = dpy;
	pCCC->screen_number = i;
	pCCC->visual = DefaultVisual(dpy, i);
	/*
	 * Used calloc to allocate memory so:
	 *	pCCC->clientWhitePt->format == XCMS_UNDEFINED_FORMAT
	 *	pCCC->gamutCompFunc == NULL
	 *	pCCC->whitePtAdjFunc == NULL
	 *	pCCC->pPerScrnInfo = NULL
	 *
	 * Don't need to create XcmsPerScrnInfo and its pSCCFuncSet and
	 * pSCCData components until the default CCC is accessed.
	 * Note that the XcmsDefaultCCC routine calls _XcmsInitScrnInto
	 * to do this.
	 */
    }

    return(1);
}


/*
 *	NAME
 *		_XcmsFreeDefaultCCCs - Free Default CCCs and its PerScrnInfo
 *
 *	SYNOPSIS
 */
void
_XcmsFreeDefaultCCCs(dpy)
    Display *dpy;
/*
 *	DESCRIPTION
 *		This routine frees the default XcmsCCC's associated with
 *		each screen and its associated substructures as neccessary.
 *
 *	RETURNS
 *		void
 *
 *
 */
{
    int nScrn = ScreenCount(dpy);
    XcmsCCC *pCCC;
    int i;

    /*
     * Free Screen data in each DefaultCCC
     *		Do not use XcmsFreeCCC here because it will not free
     *		DefaultCCC's.
     */
    pCCC = (XcmsCCC *)dpy->cms.defaultCCCs;
    for (i = nScrn; i--; pCCC++) {
	/*
	 * Check if XcmsPerScrnInfo exists.
	 *
	 * This is the only place where XcmsPerScrnInfo structures
	 * are freed since there is only one allocated per Screen.
	 * It just so happens that we place its reference in the
	 * default CCC.
	 */
	if (pCCC->pPerScrnInfo) {
	    /* Check if SCCData exists */
	    if ((pCCC->pPerScrnInfo->state == XCMS_INIT_SUCCESS ||
		    pCCC->pPerScrnInfo->state == XCMS_INIT_DEFAULT)
		    && pCCC->pPerScrnInfo->pSCCData) {
		(*((XcmsSCCFuncSet *)pCCC->pPerScrnInfo->pSCCFuncSet)->pFreeSCCData)
			(pCCC->pPerScrnInfo->pSCCData);
	    }
	}
	Xfree(pCCC->pPerScrnInfo);
    }

    /*
     * Free the array of XcmsCCC structures
     */
    Xfree(dpy->cms.defaultCCCs);
    dpy->cms.defaultCCCs = (caddr_t)NULL;
}



/*
 *	NAME
 *		_XcmsInitScrnInfo
 *
 *	SYNOPSIS
 */
int
_XcmsInitScrnInfo(dpy, screen_number)
    register Display *dpy;
    int screen_number;
/*
 *	DESCRIPTION
 *		Given a display and screen number, this routine attempts
 *		to initialize the TekCMS per Screen Info structure
 *		(XcmsPerScrnInfo).
 *
 *	RETURNS
 *		Returns zero if initialization failed; non-zero otherwise.
 */
{
    XcmsSCCFuncSet **papSCCFuncSet = _XcmsSCCFuncSets;
    XcmsCCC	*pDefaultCCC;

    /*
     * Check if the XcmsCCC's for each screen has been created.
     * Really dont need to be created until some routine uses the TekCMS
     * API routines.
     */
    if ((XcmsCCC *)dpy->cms.defaultCCCs == NULL) {
	if (!_XcmsInitDefaultCCCs(dpy)) {
	    return(0);
	}
    }

    pDefaultCCC = (XcmsCCC *)dpy->cms.defaultCCCs + screen_number;

    /*
     * For each SCCFuncSet, try its pInitScrnFunc.
     *	If the function succeeds, then we got it!
     */

    if (!pDefaultCCC->pPerScrnInfo) {
	/*
	 * This is the only place where XcmsPerScrnInfo structures
	 * are allocated since there is only one allocated per Screen.
	 * It just so happens that we place its reference in the
	 * default CCC.
	 */
	if (!(pDefaultCCC->pPerScrnInfo = (XcmsPerScrnInfo *)
		Xcalloc(1, (unsigned) sizeof(XcmsPerScrnInfo)))) {
	    return(0);
	} 
	pDefaultCCC->pPerScrnInfo->state = XCMS_INIT_NONE;
    }

    while (*papSCCFuncSet != NULL) {
	if ((*(*papSCCFuncSet)->pInitScrnFunc)(dpy, screen_number,
		pDefaultCCC->pPerScrnInfo)) {
	    pDefaultCCC->pPerScrnInfo->state = XCMS_INIT_SUCCESS;
	    return(1);
	}
	papSCCFuncSet++;
    }

    /*
     * Use Default SCCData
     */
    return(_XcmsLRGB_InitScrnDefault(dpy, screen_number, pDefaultCCC->pPerScrnInfo));
}
