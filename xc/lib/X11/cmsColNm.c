/* $XConsortium: XcmsColNm.c,v 1.15 91/05/14 13:56:34 rws Exp $" */

/*
 * Code and supporting documentation (c) Copyright 1990 1991 Tektronix, Inc.
 * 	All Rights Reserved
 * 
 * This file is a component of an X Window System-specific implementation
 * of Xcms based on the TekColor Color Management System.  Permission is
 * hereby granted to use, copy, modify, sell, and otherwise distribute this
 * software and its documentation for any purpose and without fee, provided
 * that this copyright, permission, and disclaimer notice is reproduced in
 * all copies of this software and in supporting documentation.  TekColor
 * is a trademark of Tektronix, Inc.
 * 
 * Tektronix makes no representation about the suitability of this software
 * for any purpose.  It is provided "as is" and with all faults.
 * 
 * TEKTRONIX DISCLAIMS ALL WARRANTIES APPLICABLE TO THIS SOFTWARE,
 * INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL TEKTRONIX BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA, OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR THE PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *	NAME
 *		XcmsColNm.c
 *
 *	DESCRIPTION
 *		Source for _XcmsLookupColorName().
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"
#include <X11/Xos.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#define XK_LATIN1
#include <X11/keysymdef.h>


/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
#ifdef X_NOT_STDC_ENV
extern char *getenv();
extern void qsort();
extern char *bsearch();
#endif
extern XcmsColorSpace **_XcmsDIColorSpaces;
static Status LoadColornameDB();

/*
 *      LOCAL DEFINES
 *		#define declarations local to this package.
 */
#ifndef XCMS_DEFAULT_DBPATH
#define XCMS_DEFAULT_DBPATH  "/usr/lib/X11/Xcms.txt"
#endif

#ifndef isgraph
#  define isgraph(c)	(isprint((c)) && !isspace((c)))
#endif

#define FORMAT_VERSION	"0.1"
#define START_TOKEN	"XCMS_COLORDB_START"
#define END_TOKEN	"XCMS_COLORDB_END"
#define DELIM_CHAR	'\t'

#define	NOT_VISITED	0x0
#define	VISITED		0x1
#define CYCLE		0xFFFF
#define XcmsDbInitNone		-1
#define XcmsDbInitFailure	0
#define XcmsDbInitSuccess	1

/*
 *      LOCAL TYPEDEFS
 */
typedef struct _XcmsPair {
    char *first;
    char *second;
    int flag;
} XcmsPair;

/*
 *      LOCAL VARIABLES
 */
static int XcmsColorDbState = XcmsDbInitNone;
static int nEntries;
static char *strings;
static XcmsPair *pairs;


/************************************************************************
 *									*
 *			PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsColorSpaceOfString
 *
 *	SYNOPSIS
 */
static XcmsColorSpace *
_XcmsColorSpaceOfString(ccc, color_string)
    XcmsCCC ccc;
    char *color_string;
/*
 *	DESCRIPTION
 *		Returns a pointer to the color space structure
 *		(XcmsColorSpace) associated with the specified color string.
 *
 *	RETURNS
 *		Pointer to matching XcmsColorSpace structure if found;
 *		otherwise NULL.
 *
 *	CAVEATS
 *
 */
{
    XcmsColorSpace	**papColorSpaces;
    int n;
    char *pchar;

    if ((pchar = strchr(color_string, ':')) == NULL) {
	return(XcmsFailure);
    }
    n = (int)(pchar - color_string);

    if (ccc == NULL) {
	return(NULL);
    }

    /*
     * First try Device-Independent color spaces
     */
    papColorSpaces = _XcmsDIColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strncmp((*papColorSpaces)->prefix, color_string, n) == 0) {
		return(*papColorSpaces);
	    }
	    papColorSpaces++;
	}
    }

    /*
     * Next try Device-Dependent color spaces
     */
    papColorSpaces = ((XcmsSCCFuncSet *)ccc->pPerScrnInfo->functionSet)->papDDColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strncmp((*papColorSpaces)->prefix, color_string, n) == 0) {
		return(*papColorSpaces);
	    }
	    papColorSpaces++;
	}
    }

    return(NULL);
}


/*
 *	NAME
 *		_XcmsCopyISOLatin1Lowered
 *
 *	SYNOPSIS
 */
static void
_XcmsCopyISOLatin1Lowered(dst, src)
    char *dst, *src;
/*
 *	DESCRIPTION
 *		ISO Latin-1 case conversion routine
 *		Identical to XmuCopyISOLatin1Lowered() but provided here
 *		to eliminate need to link with libXmu.a.
 *
 *		IMPLEMENTORS NOTE:
 *		    This routine is currently used by _XcmsParseColorString
 *		    and _XcmsLookupColorName.  When _XcmsLookupColorName is
 *		    replaced by the appropriate i18n database support
 *		    routine, the contents of this routine can be placed
 *		    directly into _XcmsParseColorString.
 *
 *	RETURNS
 *		Void
 *
 */
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source;
	 source++, dest++)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}


/*
 *	NAME
 *		_XcmsParseColorString
 *
 *	SYNOPSIS
 */
static int
_XcmsParseColorString(ccc, color_string, pColor)
    XcmsCCC ccc;
    char *color_string;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Assuming color_string contains a numerical string color
 *		specification, attempts to parse a string into an
 *		XcmsColor structure.		
 *
 *	RETURNS
 *		0 if failed; otherwise non-zero.
 *
 *	CAVEATS
 *		A color string containing a numerical color specification
 *		must be in ISO Latin-1 encoding!
 */
{
    XcmsColorSpace	*pColorSpace;
    char		string_lowered[BUFSIZ];

    if (ccc == NULL) {
	return(0);
    }

    /*
     * While copying color_string to string_lowered, convert to lowercase
     */
    if ((int)(strlen(color_string)) > BUFSIZ -1) {
	return(0);
    }

    _XcmsCopyISOLatin1Lowered((char *)string_lowered, (char *)color_string);

    if (*string_lowered == '#') {
	if ((pColorSpace = _XcmsColorSpaceOfString(ccc, "rgb:")) != NULL) {
	    return((*pColorSpace->parseString)(string_lowered, pColor));
	}
    }

    if ((pColorSpace = _XcmsColorSpaceOfString(ccc, string_lowered)) != NULL) {
	return((*pColorSpace->parseString)(string_lowered, pColor));
    }

    return(0);
}


/*
 *	NAME
 *		FirstCmp - Compare color names of pair recs
 *
 *	SYNOPSIS
 */
static int
FirstCmp(p1, p2)
#if __STDC__
    const void *p1, *p2;
#else
    XcmsPair *p1, *p2;
#endif
/*
 *	DESCRIPTION
 *		Compares the color names of XcmsColorTuples.
 *		This routine is public to allow access from qsort???.
 *
 *	RETURNS
 *		0 if equal;
 *		< 0 if first precedes second,
 *		> 0 if first succeeds second.
 *
 */
{
    return(strcmp(((XcmsPair *)p1)->first, ((XcmsPair *)p2)->first));
}



/*
 *	NAME
 *		stringSectionSize - determine memory needed for strings
 *
 *	SYNOPSIS
 */
static void
SetNoVisit()
/*
 *	DESCRIPTION
 *
 *	RETURNS
 *		void
 *
 */
{
    int i;
    XcmsPair *pair = pairs;

    for (i = 0; i < nEntries; i++, pair++) {
	if (pair->flag != CYCLE) {
	    pair->flag = NOT_VISITED;
	}
    }
}




/*
 *	NAME
 *		field2 - extract two fields
 *
 *	SYNOPSIS
 */
static int
field2(pBuf, delim, p1, p2)
    char *pBuf;
    char delim;	/* in:  field delimiter */
    char **p1;	/* in/out: pointer to pointer to field 1 */
    char **p2;	/* in/out: pointer to pointer to field 2 */
/*
 *	DESCRIPTION
 *		Extracts two fields from a "record".
 *
 *	RETURNS
 *		XcmsSuccess if succeeded, otherwise XcmsFailure.
 *
 */
{
    *p1 = *p2 = NULL;

    /* Find Field 1 */
    while (!isgraph(*pBuf)) {
	if ((*pBuf != '\n') || (*pBuf != '\0')) {
	    return(XcmsFailure);
	}
	if (isspace(*pBuf) || (*pBuf == delim)) {
	    pBuf++;
	}
    }
    *p1 = pBuf;

    /* Find end of Field 2 */
    while (isprint(*pBuf) && (*pBuf != delim)) {
	pBuf++;
    }
    if ((*pBuf == '\n') || (*pBuf == '\0')) {
	return(XcmsFailure);
    }
    if ((*pBuf == ' ') || (*pBuf == delim)) {
	*pBuf++ = '\0';	/* stuff end of string character */
    } else {
	return(XcmsFailure);
    }

    /* Find Field 2 */
    while (!isgraph(*pBuf)) {
	if ((*pBuf == '\n') || (*pBuf == '\0')) {
	    return(XcmsFailure);
	}
	if (isspace(*pBuf) || (*pBuf == delim)) {
	    pBuf++;
	}
    }
    *p2 = pBuf;

    /* Find end of Field 2 */
    while (isprint(*pBuf) && (*pBuf != delim)) {
	pBuf++;
    }
    if (*pBuf != '\0') {
	*pBuf = '\0';	/* stuff end of string character */
    }

    return(XcmsSuccess);
}


/*
 *	NAME
 *		_XcmsLookupColorName - Lookup DB entry for a color name
 *
 *	SYNOPSIS
 */
static Status
_XcmsLookupColorName(ccc, name, pColor)
    XcmsCCC ccc;
    char *name;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Searches for an entry in the Device-Independent Color Name
 *		Database for the specified string.
 *
 *	RETURNS
 *		XcmsFailure if failed to find a matching entry in
 *			the database.
 *		XcmsSuccess if succeeded in converting color name to
 *			XcmsColor.
 *		_XCMS_NEWNAME if succeeded in converting color string (which
 *			is a color name to yet another color name.
 *
 *	CAVEATS
 *		Assumes name is an array of BUFSIZ characters so we can
 *		overwrite!
 *
 */
 {
    Status		retval = 0;
    char		name_lowered[BUFSIZ];
    register int	i, j, left, right;
    int			len;
    char		*tmpName;
    XcmsPair		*pair;

    /*
     * Check state of Database:
     *		XcmsDbInitNone
     *		XcmsDbInitSuccess
     *		XcmsDbInitFailure
     */
    if (XcmsColorDbState == XcmsDbInitFailure) {
	return(XcmsFailure);
    }
    if (XcmsColorDbState == XcmsDbInitNone) {
	if (!LoadColornameDB()) {
	    return(XcmsFailure);
	}
    }

    SetNoVisit();

    /*
     * While copying name to name_lowered, convert to lowercase
     */

    tmpName = name;

Retry:
    if ((len = strlen(tmpName)) > BUFSIZ -1) {
	return(XcmsFailure);
    }

    _XcmsCopyISOLatin1Lowered((char *)name_lowered, (char *)tmpName);

    /*
     * Now, remove spaces.
     */
    for (i = 0, j = 0; i < len; j++) {
	if (!isspace(name_lowered[j])) {
	    name_lowered[i++] = name_lowered[j];
	}
    }
    name_lowered[i] = '\0';

    left = 0;
    right = nEntries - 1;
    while (left <= right) {
	i = (left + right) >> 1;
	pair = &pairs[i];
	j = strcmp(name_lowered, pair->first);
	if (j < 0)
	    right = i - 1;
	else if (j > 0)
	    left = i + 1;
	else {
	    break;
	}
    }

    if (left > right) {
	if (retval == 2) {
	    if (name != tmpName) {
		strncpy(name, tmpName, BUFSIZ - 1);
	    }
	    return(_XCMS_NEWNAME);
	}
	return(XcmsFailure);
    }

    if (pair->flag == CYCLE) {
	return(XcmsFailure);
    }
    if (pair->flag == VISITED) {
	pair->flag = CYCLE;
	return(XcmsFailure);
    }
	    
    if (_XcmsParseColorString(ccc, pair->second, pColor) == XcmsSuccess) {
	/* f2 contains a numerical string specification */
	return(XcmsSuccess);
    } else {
	/* f2 does not contain a numerical string specification */
	tmpName = pair->second;
	pair->flag = VISITED;
	retval = 2;
	goto Retry;
    }
}


/*
 *	NAME
 *		RemoveSpaces
 *
 *	SYNOPSIS
 */
static int
RemoveSpaces(pString)
    char *pString;
/*
 *	DESCRIPTION
 *		Removes spaces from string.
 *
 *	RETURNS
 *		Void
 *
 */
{
    int i, count = 0;
    char *cptr;

    /* REMOVE SPACES */
    cptr = pString;
    for (i = strlen(pString); i; i--, cptr++) {
	if (!isspace(*cptr)) {
	    *pString++ = *cptr;
	    count++;
	}
    }
    *pString = '\0';
    return(count);
}


/*
 *	NAME
 *		stringSectionSize - determine memory needed for strings
 *
 *	SYNOPSIS
 */
static int
stringSectionSize(stream, pNumEntries, pSectionSize)
    FILE *stream;
    int	*pNumEntries;
    int	*pSectionSize;
/*
 *	DESCRIPTION
 *		Determines the amount of memory required to store the
 *		color name strings and also the number of strings.
 *
 *	RETURNS
 *		XcmsSuccess if succeeded, otherwise XcmsFailure.
 *
 */
{
    char buf[BUFSIZ];
    char token[BUFSIZ];
    char token2[BUFSIZ];
    char *pBuf;
    char *f1;
    char *f2;
    int i;

    *pNumEntries = 0;
    *pSectionSize = 0;

    /*
     * Advance to START_TOKEN
     *	 Anything before is just considered as comments.
     */

    while((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s %s", token, token2))
		&& (strcmp(token, START_TOKEN) == 0)) {
	    if (strcmp(token2, FORMAT_VERSION) != 0) {
		/* text file not in the right format */
		return(XcmsFailure);
	    }
	    break;
	} /* else it was just a blank line or comment */
    }

    if (pBuf == NULL) {
	return(XcmsFailure);
    }

    while((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s", token)) && (strcmp(token, END_TOKEN) == 0)) {
	    break;
	}

	if (field2(buf, DELIM_CHAR, &f1, &f2) != XcmsSuccess) {
	    return(XcmsFailure);
	}

	(*pNumEntries)++;

	(*pSectionSize) += (i = strlen(f1)) + 1;
	for (; i; i--, f1++) {
	    /* REMOVE SPACES FROM COUNT */
	    if (isspace(*f1)) {
		(*pSectionSize)--;
	    }
	}

	(*pSectionSize) += (i = strlen(f2)) + 1;
	for (; i; i--, f2++) {
	    /* REMOVE SPACES FROM COUNT */
	    if (isspace(*f2)) {
		(*pSectionSize)--;
	    }
	}

    }

    return(XcmsSuccess);
}


/*
 *	NAME
 *		ReadColornameDB - Read the Color Name Database
 *
 *	SYNOPSIS
 */
static Status
ReadColornameDB(stream, pRec, pString)
    FILE *stream;
    XcmsPair *pRec;
    char *pString;
/*
 *	DESCRIPTION
 *		Loads the Color Name Database from a text file.
 *
 *	RETURNS
 *		XcmsSuccess if succeeded, otherwise XcmsFailure.
 *
 */
{
    char buf[BUFSIZ];
    char token[BUFSIZ];
    char token2[BUFSIZ];
    char *f1;
    char *f2;
    char *pBuf;

    /*
     * Advance to START_TOKEN
     *	 Anything before is just considered as comments.
     */

    while((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s %s", token, token2))
		&& (strcmp(token, START_TOKEN) == 0)) {
	    if (strcmp(token2, FORMAT_VERSION) != 0) {
		/* text file not in the right format */
		return(XcmsFailure);
	    }
	    break;
	} /* else it was just a blank line or comment */
    }

    if (pBuf == NULL) {
	return(XcmsFailure);
    }

    /*
     * Process lines between START_TOKEN to END_TOKEN
     */

    while ((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s", token)) && (strcmp(token, END_TOKEN) == 0)) {
	    /*
	     * Found END_TOKEN so break out of for loop
	     */
	    break;
	}

	/*
	 * Get pairs
	 */
	if (field2(buf, DELIM_CHAR, &f1, &f2) != XcmsSuccess) {
	    /* Invalid line */
	    continue;
	}

	/*
	 * Add strings
	 */

	/* Left String */
	pRec->first = pString;
	_XcmsCopyISOLatin1Lowered((char *)pString, (char *)f1);
	pString += (1 + RemoveSpaces(pString));
	pRec->second = pString;
	/* Right String */
	_XcmsCopyISOLatin1Lowered((char *)pString, (char *)f2);
	pString += RemoveSpaces(pString) + 1;
	pRec++;

    }

    return(XcmsSuccess);
}


/*
 *	NAME
 *		LoadColornameDB - Load the Color Name Database
 *
 *	SYNOPSIS
 */
static Status
LoadColornameDB()
/*
 *	DESCRIPTION
 *		Loads the Color Name Database from a text file.
 *
 *	RETURNS
 *		XcmsSuccess if succeeded, otherwise XcmsFailure.
 *
 */
{
    int size;
    FILE *stream;
    char *pathname;
    struct stat txt;
    int length;

    if ((pathname = (char *)getenv("XCMSDB")) == NULL) {
	pathname = XCMS_DEFAULT_DBPATH;
    }

    length = strlen(pathname);
    if ((length == 0) || (length >= (BUFSIZ - 5))){
	XcmsColorDbState = XcmsDbInitFailure;
	return(XcmsFailure);
    }

    if (stat(pathname, &txt)) {
	/* can't stat file */
	XcmsColorDbState = XcmsDbInitFailure;
	return(XcmsFailure);
    }

    if ((stream = fopen(pathname, "r")) == NULL) {
	return(XcmsFailure);
    }

    stringSectionSize(stream, &nEntries, &size);
    rewind(stream);

    strings = (char *) Xmalloc(size);
    pairs = (XcmsPair *)Xcalloc(nEntries, sizeof(XcmsPair));

    ReadColornameDB(stream, pairs, strings);

    /*
     * sort the pair recs
     */
    qsort((char *)pairs, nEntries, sizeof(XcmsPair), FirstCmp);

    XcmsColorDbState = XcmsDbInitSuccess;
    return(XcmsSuccess);
}


/*
 *	NAME
 *		XcmsFreeColorDB - Free Color Name Database
 *
 *	SYNOPSIS
 */
void
XcmsFreeColorDB()
/*
 *	DESCRIPTION
 *		Creates
 *
 *	RETURNS
 *		XcmsSuccess if succeeded, otherwise XcmsFailure.
 *
 */
{
    /*
     * Check if XcmsColorDB has been intialized
     */
    if (XcmsColorDbState != XcmsDbInitSuccess) {
	return;
    }

    /*
     * Free memory
     */
    free(strings);
    free(pairs);
}



/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsResolveColorString - 
 *
 *	SYNOPSIS
 */
#if NeedFunctionPrototypes
Status
_XcmsResolveColorString (
    XcmsCCC ccc,
    _Xconst char *color_string,
    XcmsColor *pColor_exact_return,
    XcmsColorFormat result_format)
#else
Status
_XcmsResolveColorString(ccc, color_string, pColor_exact_return, result_format)
    XcmsCCC ccc;
    char *color_string;
    XcmsColor *pColor_exact_return;
    XcmsColorFormat result_format;
#endif
/*
 *	DESCRIPTION
 *		The XcmsLookupColor function finds the color specification
 *		associated with a color name in the Device-Independent Color
 *		Name Database.
 *	RETURNS
 *		XcmsFailure if failed to parse string or find any entry in
 *			the database.
 *		XcmsSuccess if succeeded in converting color string to
 *			XcmsColor.
 *		_XCMS_NEWNAME if succeeded in converting color string (which
 *			is a color name to yet another color name.
 *
 *		This function returns both the color specification found in the
 *		database (db specification) and the color specification for the
 *		color displayable by the specified screen (screen
 *		specification).  The calling routine sets the format for these
 *		returned specifications in the XcmsColor format component.
 *		If XcmsUndefinedFormat, the specification is returned in the
 *		format used to store the color in the database.
 *
 *	CAVEATS
 *		Assumes name is an array of BUFSIZ characters so we can
 *		overwrite!
 */
{
    XcmsColor dbWhitePt;	/* whitePt associated with pColor_exact_return*/
    int inheritScrnWhitePt = 0;	/* Indicates if pColor_exact_return inherits */
				/*    the screen's white point */
    int retval;

/*
 * 0. Check for invalid arguments.
 */
    if (ccc == NULL || color_string[0] == '\0' || pColor_exact_return == NULL) {
	return(XcmsFailure);
    }

/*
 * 1. First attempt to parse the string
 *    If successful, then convert the specification to the target format
 *    and return.
 */
    if (_XcmsParseColorString(ccc, color_string, pColor_exact_return)
	    == 1) {
	if (result_format != XcmsUndefinedFormat
		&& pColor_exact_return->format != result_format) {
	    /* need to be converted to the target format */
	    return(XcmsConvertColors(ccc, pColor_exact_return, 1, 
		    result_format, (Bool *) NULL));
	} else {
	    return(XcmsSuccess);
	}
    }

/*
 * 2. Attempt to find it in the DI Color Name Database
 */

    /*
     * a. Convert String into a XcmsColor structure
     *       Attempt to extract the specification for color_string from the
     *       DI Database (pColor_exact_return).  If the DI Database does not
     *	     have this entry, then return failure.
     */
    retval = _XcmsLookupColorName(ccc, color_string, pColor_exact_return);

    if (retval == _XCMS_NEWNAME) {
	/* color_string replaced with a color name */
	return(retval);
    }

    if ((retval == XcmsFailure)
	   || (pColor_exact_return->format == XcmsUndefinedFormat)) {
	return(XcmsFailure);
    }

    /*
     * c. Attempt to extract the specification for the "WhitePoint" from
     *    the DI Database (dbWhitePt).
     *
     *    If
     *	     (1) the database specification is device-dependent (e.g., RGB
     *	  	or RGBi), or
     *	     (2) the DI Database does not have a white point,
     *	  then assume the white point is the same as the Screen White Point.
     */
    if (XCMS_DD_ID(pColor_exact_return->format) ||
	    (_XcmsLookupColorName(ccc, "WhitePoint", &dbWhitePt)
	    != 1)) {
	inheritScrnWhitePt++;
	bcopy((char *)&ccc->pPerScrnInfo->screenWhitePt, (char *)&dbWhitePt,
		sizeof(XcmsColor));
    }

    /*
     * d. White Point Adjustment  (ClientWhitePoint versus DBWhitePt)
     *
     */
    if (result_format == XcmsUndefinedFormat) {
	result_format = pColor_exact_return->format;
    }
    if ((ccc->clientWhitePt.format == XcmsUndefinedFormat && inheritScrnWhitePt)
	    || _XcmsEqualWhitePts(ccc, &ccc->clientWhitePt, &dbWhitePt)) {
	/*
	 * 1. Client White Point is equal to the Screen White Point AND
	 *	a. pColor_exact_return->format is Device-Dependent (e.g. RGB,
	 *		RGBi),  OR
	 *	b. No white point was found in the database.
	 *     OR
	 * 2. Database White Point and Client White Point are equal.
	 *
	 * Therefore, we can just use dbWhitePt in our conversion and
	 * convert to the target format.
	 */
	return(_XcmsConvertColorsWithWhitePt(ccc, pColor_exact_return,
		&dbWhitePt, 1, result_format, (Bool *) NULL));
    } else if (ccc->whitePtAdjProc) {
	/*
	 * Database White Point and Client White Point are not equal therefore
	 *	the pColor_exact_return must be White Point Adjusted from the
	 *	Database White Point to the Client White Point.
	 */
	return((*ccc->whitePtAdjProc)(ccc, &dbWhitePt,
		&ccc->clientWhitePt, result_format, pColor_exact_return, 1,
		(Bool *) NULL));
    } else {
	/*
	 * White Point Adjustment function unavailable in ccc, therefore
	 * do not perform white point adjustment.  Just convert to target
	 * format
	 */
	if (result_format == pColor_exact_return->format) {
	    return(1);
	}
	/* Convert to the target format */
	return(XcmsConvertColors(ccc, pColor_exact_return,
		1, result_format, (Bool *) NULL));
    }
}
