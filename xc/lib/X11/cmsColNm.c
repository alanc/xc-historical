/* $XConsortium: XcmsColNm.c,v 1.4 91/02/07 17:36:03 dave Exp $" */

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
 *	NAME
 *		XcmsColNm.c
 *
 *	DESCRIPTION
 *		Source for _XcmsLookupColorName().
 *
 *
 */

#include "Xcmsint.h"
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <X11/Xos.h>
#include <ctype.h>
/*****
#if __STDC__ 
#include <stdlib.h>
#endif 
*****/
#include "Xlibos.h"
#define XK_LATIN1
#include <X11/keysymdef.h>


/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern XcmsColorSpace **_XcmsDIColorSpaces;


/*
 *      LOCAL DEFINES
 *		#define declarations local to this package.
 */
#define DEFAULT_ROOT "/usr/lib/X11"

#ifndef isgraph
#  define isgraph(c)	(isprint((c)) && !isspace((c)))
#endif

#define FORMAT_VERSION	"0.1"
#define START_TOKEN	"XCMS_COLORDB_START"
#define END_TOKEN	"XCMS_COLORDB_END"
#define DELIM_CHAR	'\t'

/*
 *      LOCAL TYPEDEFS
 *              typedefs local to this package (for use with local vars).
 *
 */



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
_XcmsColorSpaceOfString(pCCC, color_string)
    XcmsCCC *pCCC;
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
	return(XCMS_FAILURE);
    }
    n = (int)(pchar - color_string);

    if (pCCC == NULL) {
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
    papColorSpaces = ((XcmsSCCFuncSet *)pCCC->pPerScrnInfo->pSCCFuncSet)->papDDColorSpaces;
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
_XcmsParseColorString(pCCC, color_string, pColor)
    XcmsCCC *pCCC;
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

    if (pCCC == NULL) {
	return(0);
    }

    /*
     * While copying color_string to string_lowered, convert to lowercase
     */
    if ((int)(strlen(color_string)) > BUFSIZ -1) {
	return(0);
    }

    _XcmsCopyISOLatin1Lowered((unsigned char *)string_lowered,
	    (unsigned char *)color_string);

    if (*string_lowered == '#') {
	if ((pColorSpace = _XcmsColorSpaceOfString(pCCC, "rgb:")) != NULL) {
	    return((*pColorSpace->parseString)(string_lowered, pColor));
	}
    }

    if ((pColorSpace = _XcmsColorSpaceOfString(pCCC, string_lowered)) != NULL) {
	return((*pColorSpace->parseString)(string_lowered, pColor));
    }

    return(0);
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
 *		XCMS_SUCCESS if succeeded, otherwise XCMS_FAILURE.
 *
 */
{
    *p1 = *p2 = NULL;

    /* Find Field 1 */
    while (!isgraph(*pBuf)) {
	if ((*pBuf != '\n') || (*pBuf != '\0')) {
	    return(XCMS_FAILURE);
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
	return(XCMS_FAILURE);
    }
    if ((*pBuf == ' ') || (*pBuf == delim)) {
	*pBuf++ = '\0';	/* stuff end of string character */
    } else {
	return(XCMS_FAILURE);
    }

    /* Find Field 2 */
    while (!isgraph(*pBuf)) {
	if ((*pBuf == '\n') || (*pBuf == '\0')) {
	    return(XCMS_FAILURE);
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

    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		_XcmsLookupColorName - Lookup DB entry for a color name
 *
 *	SYNOPSIS
 */
static Status
_XcmsLookupColorName(pCCC, stream, name, pColor)
    XcmsCCC *pCCC;
    FILE *stream;
    char *name;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Searches for an entry in the Device-Independent Color Name
 *		Database for the specified string.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed to find a matching entry in
 *			the database.
 *		XCMS_SUCCESS if succeeded in converting color name to
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
    char		buf[BUFSIZ];
    register int	i, j;
    int			len;
    char		token[BUFSIZ];
    char		token2[BUFSIZ];
    char		*tmpName;
    char		*f1;
    char		*f2;
    char		tmp[BUFSIZ];
    char		*pBuf;

    /*
     * While copying name to name_lowered, convert to lowercase
     */

    tmpName = name;

Retry:
    if ((len = strlen(tmpName)) > BUFSIZ -1) {
	return(XCMS_FAILURE);
    }

    _XcmsCopyISOLatin1Lowered((unsigned char *)name_lowered, (unsigned char *)tmpName);

    /*
     * Now, remove spaces.
     */
    for (i = 0, j = 0; i < len; j++) {
	if (!isspace(name_lowered[j])) {
	    name_lowered[i++] = name_lowered[j];
	}
    }
    name_lowered[i] = '\0';

    /*
     * Advance to START_TOKEN
     *	 Anything before is just considered as comments.
     */

    while((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s %s", token, token2))
		&& (strcmp(token, START_TOKEN) == 0)) {
	    if (strcmp(token2, FORMAT_VERSION) != 0) {
		/* text file not in the right format */
		return(XCMS_FAILURE);
	    }
	    break;
	} /* else it was just a blank line or comment */
    }

    if (pBuf == NULL) {
	if (retval == 2) {
	    if (name != tmpName) {
		strncpy(name, tmpName, BUFSIZ - 1);
	    }
	    return(_XCMS_NEWNAME);
	}
	return(XCMS_FAILURE);
    }

    /*
     * Process lines between START_TOKEN to END_TOKEN
     */

    while ((pBuf = fgets(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s", token))
	    && (strcmp(token, END_TOKEN) == 0)) {
	    /*
	     * Found END_TOKEN
	     */
	    if (retval == 2) {
		if (name != tmpName) {
		    strncpy(name, tmpName, BUFSIZ - 1);
		}
		return(_XCMS_NEWNAME);
	    }
	    return(XCMS_FAILURE);
	}

	/*
	 * Get tuples
	 */
	if (field2(buf, DELIM_CHAR, &f1, &f2) != XCMS_SUCCESS) {
	    /* Invalid line */
	    continue;
	}

	/*
	 * Process string
	 */
	len = strlen(f1);

	/* REMOVE SPACES */
	for (i = 0, j = 0; i < len; j++) {
	    if (!isspace(f1[j])) {
		f1[i++] = f1[j];
	    }
	}
	f1[i] = '\0';

	/* CONVERT TO LOWERCASE */
	len = strlen(f1);
	_XcmsCopyISOLatin1Lowered((unsigned char *)tmp, (unsigned char *)f1);

	/*
	 * Compare
	 */
	if (strcmp(name_lowered, tmp) == 0) {
	    if (_XcmsParseColorString(pCCC, f2, pColor) == XCMS_SUCCESS) {
		/* f2 contains a numerical string specification */
		return(XCMS_SUCCESS);
	    } else {
		/* f2 does not contain a numerical string specification */
		name = f2;
		retval = 2;
		goto Retry;
	    }
	}
    }

    return(0);
}

#ifndef XRESOLVEPATHNAME

/*
 *	NAME
 *		XResolvePathname
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
static char *
XResolvePathname(dpy, type, filename, suffix, root, path, substitutions,
	num_substitutions, predicate, fp, mode)
    Display *dpy;
    char *type, *filename, *suffix, *root;
    char *path;
    caddr_t /* Substitution */ substitutions;
    int num_substitutions;
    caddr_t /* XFilePredicate */ predicate;
    FILE **fp;
    char *mode;
/*
 *	DESCRIPTION
 *		"Dummy" routine until the real XResolvePathname is
 *		available.  This is a hack!
 *
 *	RETURNS
 *		Returns the filename.
 *		Also opens the file and returns the file pointer via fp.
 *
 */
 {
    char pathname[BUFSIZ];
    char real_root[BUFSIZ];
    char real_locale[BUFSIZ];
    struct stat txt;
    char *pathname_ret;
    int i;

    *fp = NULL;

    /*
     * root
     */
    if (root == NULL) {
	strcpy(real_root, DEFAULT_ROOT);
    } else {
	strcpy(real_root, root);
    }

    /*
     * Fudge Locale to usascii
     */
    strcpy(real_locale, "usascii");

    pathname[0] = '\0';

    if (path) {
	if (*path == ':') {
	    /* ignore leading colon */
	    path++;
	}

	i = 0;
	while (*path != '\0') {
	    if (*path == '%') {
		path++;
		switch(*path++) {
		  case 'D' : /* directory separator */
		    pathname[i++] = '/';
		    break;
		  case 'F' : /* file separator */
		    pathname[i++] = '/';
		    break;
		  case 'R' : /* root */
		    strcpy(&pathname[i], real_root);
		    i += strlen(real_root);
		    break;
		  case 'L' : /* locale */
		    strcpy(&pathname[i], real_locale);
		    i += strlen(real_locale);
		    break;
		  case 'T' : /* type */
		    if (type != NULL) {
			strcpy(&pathname[i], type);
			i += strlen(type);
		    }
		    break;
		  case 'N' : /* filename */
		    if (filename != NULL) {
			strcpy(&pathname[i], filename);
			i += strlen(filename);
		    }
		    break;
		  case 'S' : /* suffix */
		    if (suffix != NULL) {
			strcpy(&pathname[i], suffix);
			i += strlen(suffix);
		    }
		    break;
		  case ':' : /* colon */
		    pathname[i++] = ':';
		    break;
		  case '%' : /* colon */
		    pathname[i++] = '%';
		    break;
		  default :
		   break;
		}
	    } else if (*path == ':') {
		path++;
		if (*path == ':') {
		    /* not path separator, really a colon */
		    pathname[i++] = *path++;
		} else {
		    /* path separator */
		    pathname[i] = '\0';
		    if ((*fp = fopen(pathname, "r")) != NULL) {
			pathname_ret = (char *) Xmalloc(strlen(pathname) + 1);
			strcpy(pathname_ret, pathname);
			return(pathname_ret);
		    } else {
			/* start all over */
			pathname[0] = '\0';
			i = 0;
		    }

		}
	    } else {
		pathname[i++] = *path++;
	    }
	}
    }

    if (pathname[0] == '\0') {
	/*
	 * Couldn't find file using path, so build our own
	 */
	strcpy(pathname, real_root);
	strcat(pathname, "/");
	strcat(pathname, real_locale);
	if (type != NULL) {
	    strcat(pathname, "/");
	    strcat(pathname, type);
	}
	if (filename != NULL) {
	    strcat(pathname, "/");
	    strcat(pathname, filename);
	}
	if (suffix != NULL) {
	    strcat(pathname, suffix);
	}

	if (stat(pathname, &txt)) {
	    /* can't stat file */
	    return((char *)NULL);
	}
    }

    pathname[i] = '\0';
    if ((*fp = fopen(pathname, "r")) == NULL) {
	return((char *)NULL);
    }
    pathname_ret = (char *) Xmalloc(strlen(pathname) + 1);
    strcpy(pathname_ret, pathname);
    return(pathname_ret);
}
#endif /* XRESOLVEPATHNAME */



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
    XcmsCCC *pCCC,
    _Xconst char *color_string,
    XcmsColor *pColor_exact_return,
    XcmsSpecFmt result_format)
#else
Status
_XcmsResolveColorString(pCCC, color_string, pColor_exact_return, result_format)
    XcmsCCC *pCCC;
    char *color_string;
    XcmsColor *pColor_exact_return;
    XcmsSpecFmt result_format;
#endif
/*
 *	DESCRIPTION
 *		The XcmsLookupColor function finds the color specification
 *		associated with a color name in the Device-Independent Color
 *		Name Database.
 *	RETURNS
 *		XCMS_FAILURE if failed to parse string or find any entry in
 *			the database.
 *		XCMS_SUCCESS if succeeded in converting color string to
 *			XcmsColor.
 *		_XCMS_NEWNAME if succeeded in converting color string (which
 *			is a color name to yet another color name.
 *
 *		This function returns both the color specification found in the
 *		database (db specification) and the color specification for the
 *		color displayable by the specified screen (screen
 *		specification).  The calling routine sets the format for these
 *		returned specifications in the XcmsColor format component.
 *		If XCMS_UNDEFINED_FORMAT, the specification is returned in the
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
    FILE *stream;
    char *filename_ret;
    int retval;

/*
 * 0. Check for invalid arguments.
 */
    if (pCCC == NULL || color_string[0] == '\0' || pColor_exact_return == NULL) {
	return(XCMS_FAILURE);
    }

/*
 * 1. First attempt to parse the string
 *    If successful, then convert the specification to the target format
 *    and return.
 */
    if (_XcmsParseColorString(pCCC, color_string, pColor_exact_return)
	    == 1) {
	if (result_format != XCMS_UNDEFINED_FORMAT
		&& pColor_exact_return->format != result_format) {
	    /* need to be converted to the target format */
	    return(XcmsConvertColors(pCCC, pColor_exact_return, 1, 
		    result_format, (Bool *) NULL));
	} else {
	    return(XCMS_SUCCESS);
	}
    }

/*
 * 2. Attempt to find it in the DI Color Name Database
 */
    /*
     * a. Build path to the proper national XCMS.txt.
     */
    filename_ret = XResolvePathname(
	    pCCC->dpy,	/* display */
	    XCMS_COLORNAMEDB_TYPE,	/* type */
	    XCMS_COLORNAMEDB_FILENAME,	/* filename */
	    XCMS_COLORNAMEDB_SUFFIX,	/* suffix */
	    NULL,	/* root */
	    getenv(XCMS_DBFILEPATH_ENV_VAR), /* file path */
	    NULL,	/* substitutions */
	    0,		/* num_substitutions */
	    NULL,	/* predicate */
	    &stream,	/* address of FILE pointer */
	    "r"		/* mode */
	    );

    if (stream == NULL) {
	if (filename_ret != NULL) {
	    Xfree(filename_ret);
	}
	return(XCMS_FAILURE);
    }

    /*
     * b. Convert String into a XcmsColor structure
     *       Attempt to extract the specification for color_string from the
     *       DI Database (pColor_exact_return).  If the DI Database does not
     *	     have this entry, then return failure.
     */
    retval = _XcmsLookupColorName(pCCC, stream, color_string,
	    pColor_exact_return);

    if (retval == _XCMS_NEWNAME) {
	/* color_string replaced with a color name */
	return(retval);
    }

    if ((retval == XCMS_FAILURE)
	   || (pColor_exact_return->format == XCMS_UNDEFINED_FORMAT)) {
	Xfree(filename_ret);
	fclose(stream);
	return(XCMS_FAILURE);
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
	    (_XcmsLookupColorName(pCCC, stream, "WhitePoint", &dbWhitePt)
	    != 1)) {
	inheritScrnWhitePt++;
	bcopy((char *)&pCCC->pPerScrnInfo->screenWhitePt, (char *)&dbWhitePt,
		sizeof(XcmsColor));
    }
    Xfree(filename_ret);
    fclose(stream);

    /*
     * d. White Point Adjustment  (ClientWhitePoint versus DBWhitePt)
     *
     */
    if (result_format == XCMS_UNDEFINED_FORMAT) {
	result_format = pColor_exact_return->format;
    }
    if ((pCCC->clientWhitePt.format == XCMS_UNDEFINED_FORMAT && inheritScrnWhitePt)
	    || _XcmsEqualWhitePts(pCCC, &pCCC->clientWhitePt, &dbWhitePt)) {
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
	return(_XcmsConvertColorsWithWhitePt(pCCC, pColor_exact_return,
		&dbWhitePt, 1, result_format, (Bool *) NULL));
    } else if (pCCC->whitePtAdjFunc) {
	/*
	 * Database White Point and Client White Point are not equal therefore
	 *	the pColor_exact_return must be White Point Adjusted from the
	 *	Database White Point to the Client White Point.
	 */
	return((*pCCC->whitePtAdjFunc)(pCCC, &dbWhitePt,
		&pCCC->clientWhitePt, result_format, pColor_exact_return, 1,
		(Bool *) NULL));
    } else {
	/*
	 * White Point Adjustment function unavailable in pCCC, therefore
	 * do not perform white point adjustment.  Just convert to target
	 * format
	 */
	if (result_format == pColor_exact_return->format) {
	    return(1);
	}
	/* Convert to the target format */
	return(XcmsConvertColors(pCCC, pColor_exact_return,
		1, result_format, (Bool *) NULL));
    }
}
