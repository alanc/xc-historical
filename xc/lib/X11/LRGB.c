/* $XConsortium: XcmsLRGB.c,v 1.1 91/01/30 18:43:27 dave Exp $" */

/*
 * (c) Copyright 1990 1991 Tektronix Inc.
 * 	All Rights Reserved
 *
 * This code, which implements the TekColor Human Interface and/or the TekHVC
 * Color Space algorithms, is proprietary to Tektronix, Inc., and permission
 * is granted for use only in the form supplied.  Revisions, modifications,
 * or * adaptations are not permitted without the prior written approval of
 * Tektronix, Inc., Beaverton, OR 97077.  Code and supporting documentation
 * copyright Tektronix, Inc. 1990 1991 All rights reserved.  TekColor and TekHVC
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
 *		XcmsLRGB.c
 *
 *	DESCRIPTION
 *		This file contains the conversion routines:
 *		    1. CIE XYZ to RGB intensity
 *		    2. RGB intensity to device RGB
 *		    3. device RGB to RGB intensity
 *		    4. RGB intensity to CIE XYZ
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
#include <stdio.h>
#include <X11/Xos.h>
#include "Xlibos.h"
#include "TekCMSP.h"
#include "TekCMSext.h"
#include <X11/Xatom.h>
#include "XcmsLRGBSD.h"

/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern char XcmsRGB_prefix[];
extern char XcmsRGBi_prefix[];
extern LINEAR_RGB_SCCData Default_RGB_SCCData;


/*
 *      LOCAL DEFINES
 *		#define declarations local to this package.
 */
#define EPS	0.001
#ifndef MIN
#define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif /* MIN */
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif /* MAX */
#ifndef MIN3
#define MIN3(x,y,z) (MIN(x, MIN(y, z)))
#endif /* MIN3 */
#ifndef MAX3
#define MAX3(x,y,z) (MAX(x, MAX(y, z)))
#endif /* MAX3 */

/*
 *      LOCAL TYPEDEFS
 *              typedefs local to this package (for use with local vars).
 *
 */

/*
 *      EXTERNS
 */
extern void LINEAR_RGB_FreeSCCData();

/*
 *      FORWARD DECLARATIONS
 */
Status LINEAR_RGB_RGB_to_RGBi();
Status LINEAR_RGB_RGBi_to_CIEXYZ();
Status LINEAR_RGB_CIEXYZ_to_RGBi();
Status LINEAR_RGB_RGBi_to_RGB();
int LINEAR_RGB_InitSCCData();
caddr_t LINEAR_RGB_CopySCCData();
int XcmsLRGB_RGB_ParseString();
int XcmsLRGB_RGBi_ParseString();

/*
 *      LOCALS VARIABLES
 *		Variables local to this package.
 *		    Usage example:
 *		        static int	ExampleLocalVar;
 */

static unsigned short HALF[17] = {
    0x0000,	/*  0 bitsPerRGB */
    0x3fff,	/*  1 bitsPerRGB */
    0x1fff,	/*  2 bitsPerRGB */
    0x0fff,	/*  3 bitsPerRGB */
    0x07ff,	/*  4 bitsPerRGB */
    0x03ff,	/*  5 bitsPerRGB */
    0x01ff,	/*  6 bitsPerRGB */
    0x00ff,	/*  7 bitsPerRGB */
    0x007f,	/*  8 bitsPerRGB */
    0x003f,	/*  9 bitsPerRGB */
    0x001f,	/* 10 bitsPerRGB */
    0x000f,	/* 11 bitsPerRGB */
    0x0007,	/* 12 bitsPerRGB */
    0x0003,	/* 13 bitsPerRGB */
    0x0001,	/* 14 bitsPerRGB */
    0x0000,	/* 15 bitsPerRGB */
    0x0000	/* 16 bitsPerRGB */
};

static unsigned short MASK[17] = {
    0x0000,	/*  0 bitsPerRGB */
    0x8000,	/*  1 bitsPerRGB */
    0xc000,	/*  2 bitsPerRGB */
    0xe000,	/*  3 bitsPerRGB */
    0xf000,	/*  4 bitsPerRGB */
    0xf800,	/*  5 bitsPerRGB */
    0xfc00,	/*  6 bitsPerRGB */
    0xfe00,	/*  7 bitsPerRGB */
    0xff00,	/*  8 bitsPerRGB */
    0xff80,	/*  9 bitsPerRGB */
    0xffc0,	/* 10 bitsPerRGB */
    0xffe0,	/* 11 bitsPerRGB */
    0xfff0,	/* 12 bitsPerRGB */
    0xfff8,	/* 13 bitsPerRGB */
    0xfffc,	/* 14 bitsPerRGB */
    0xfffe,	/* 15 bitsPerRGB */
    0xffff	/* 16 bitsPerRGB */
};


    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XCMS_RGB_FORMAT
     * to XCMS_CIEXYZ_FORMAT.
     */
static XcmsFuncPtr Fl_RGB_to_CIEXYZ[] = {
    LINEAR_RGB_RGB_to_RGBi,
    LINEAR_RGB_RGBi_to_CIEXYZ,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XCMS_CIEXYZ_FORMAT
     * to XCMS_RGB_FORMAT.
     */
static XcmsFuncPtr Fl_CIEXYZ_to_RGB[] = {
    LINEAR_RGB_CIEXYZ_to_RGBi,
    LINEAR_RGB_RGBi_to_RGB,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XCMS_RGBi_FORMAT
     * to XCMS_CIEXYZ_FORMAT.
     */
static XcmsFuncPtr Fl_RGBi_to_CIEXYZ[] = {
    LINEAR_RGB_RGBi_to_CIEXYZ,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XCMS_CIEXYZ_FORMAT
     * to XCMS_RGBi_FORMAT.
     */
static XcmsFuncPtr Fl_CIEXYZ_to_RGBi[] = {
    LINEAR_RGB_CIEXYZ_to_RGBi,
    NULL
};

    /*
     * RGBi Color Spaces
     */
XcmsColorSpace	XcmsLRGB_RGBi_ColorSpace =
    {
	XcmsRGBi_prefix,	/* prefix */
	XCMS_RGBi_FORMAT,		/* id */
	XcmsLRGB_RGBi_ParseString,	/* parseString */
	Fl_RGBi_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_RGBi	/* from_CIEXYZ */
    };

    /*
     * RGB Color Spaces
     */
XcmsColorSpace	XcmsLRGB_RGB_ColorSpace =
    {
	XcmsRGB_prefix,		/* prefix */
	XCMS_RGB_FORMAT,		/* id */
	XcmsLRGB_RGB_ParseString,	/* parseString */
	Fl_RGB_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_RGB	/* from_CIEXYZ */
    };

    /*
     * Device-Independent Color Spaces known to the 
     * LINEAR_RGB Screen Color Characteristics Function Set.
     */
static XcmsColorSpace	*DDColorSpaces[] = {
    &XcmsLRGB_RGB_ColorSpace,
    &XcmsLRGB_RGBi_ColorSpace,
    NULL
};


/*
 *      GLOBALS
 *              Variables declared in this package that are allowed
 *		to be used globally.
 */

    /*
     * LINEAR_RGB Screen Color Characteristics Function Set.
     */
XcmsSCCFuncSet	LINEAR_RGB_SCCFuncSet =
    {
	&DDColorSpaces[0],	/* pDDColorSpaces */
	LINEAR_RGB_InitSCCData,	/* pInitScrnFunc */
	LINEAR_RGB_FreeSCCData	/* pFreeSCCData */
    };


/************************************************************************
 *									*
 *			PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		ValueCmp
 *
 *	SYNOPSIS
 */
static int
ValueCmp (p1, p2)
    IntensityRec *p1, *p2;
/*
 *	DESCRIPTION
 *		Compares the value component of two IntensityRec
 *		structures.
 *
 *	RETURNS
 *		0 if p1->value is equal to p2->value
 *		< 0 if p1->value is less than p2->value
 *		> 0 if p1->value is greater than p2->value
 *
 */
{
    return (p1->value - p2->value);
}


/*
 *	NAME
 *		IntensityCmp
 *
 *	SYNOPSIS
 */
static int
IntensityCmp (p1, p2)
    IntensityRec *p1, *p2;
/*
 *	DESCRIPTION
 *		Compares the intensity component of two IntensityRec
 *		structures.
 *
 *	RETURNS
 *		0 if equal;
 *		< 0 if first precedes second
 *		> 0 if first succeeds second
 *
 */
{
    if (p1->intensity < p2->intensity) {
	return (-1);
    }
    if (p1->intensity > p2->intensity) {
	return (XCMS_SUCCESS);
    }
    return (XCMS_FAILURE);
}

/*
 *	NAME
 *		ValueInterpolation
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
static int
ValueInterpolation (key, lo, hi, answer, bitsPerRGB)
    IntensityRec *key, *lo, *hi, *answer;
    int bitsPerRGB;
/*
 *	DESCRIPTION
 *		Based on a given value, performs a linear interpolation
 *		on the intensities between two IntensityRec structures.
 *		Note that the bitsPerRGB parameter is ignored.
 *
 *	RETURNS
 *		Returns 0 if failed; otherwise non-zero.
 */
{
    XcmsFloat ratio;

    ratio = (XcmsFloat)(key->value - lo->value) / (XcmsFloat)(hi->value - lo->value);
    answer->value = key->value;
    answer->intensity = (hi->intensity - lo->intensity) * ratio;
    answer->intensity += lo->intensity;
    return (XCMS_SUCCESS);
}

/*
 *	NAME
 *		IntensityInterpolation
 *
 *	SYNOPSIS
 */
static int
IntensityInterpolation (key, lo, hi, answer, bitsPerRGB)
    IntensityRec *key, *lo, *hi, *answer;
    int bitsPerRGB;
/*
 *	DESCRIPTION
 *		Based on a given intensity, performs a linear interpolation
 *		on the values between two IntensityRec structures.
 *		The bitsPerRGB parameter is necessary to perform rounding
 *		to the correct number of significant bits.
 *
 *	RETURNS
 *		Returns 0 if failed; otherwise non-zero.
 */
{
    XcmsFloat ratio;
    int tmp;
    ratio = (key->intensity - lo->intensity) / (hi->intensity - lo->intensity);
    answer->intensity = key->intensity;
    tmp = ((XcmsFloat)(hi->value - lo->value) * ratio) + 0.5;
    answer->value = (lo->value + tmp + HALF[bitsPerRGB]) & MASK[bitsPerRGB];
    return (XCMS_SUCCESS);
}



/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsTableSearch
 *
 *	SYNOPSIS
 */
int
_XcmsTableSearch (key, bitsPerRGB, base, nel, nKeyPtrSize, compar, interpol, answer)
    char *key;
    int bitsPerRGB;
    char *base;
    unsigned nel;
    unsigned nKeyPtrSize;
    int (*compar)();
    int (*interpol)();
    char *answer;

/*
 *	DESCRIPTION
 *		A binary search through the specificied table.
 *
 *	RETURNS
 *		Returns 0 if failed; otherwise non-zero.
 *
 */
{
    char *hi, *lo, *mid, *last;
    int result;

    /* for value, use only the significant high order bits */
    ((IntensityRec *)key)->value &= MASK[bitsPerRGB];

    last = hi = base + (nel * nKeyPtrSize);
    mid = lo = base;

    while (mid != last) {
	last = mid;
	mid = lo + ((int)((((int)hi - (int)lo) / nKeyPtrSize) / 2) *
		nKeyPtrSize);
	result = compar (key, mid);
	if (result == 0) {

	    bcopy(mid, answer, nKeyPtrSize);
	    return (XCMS_SUCCESS);
	} else if (result < 0) {
	    hi = mid;
	} else {
	    lo = mid;
	}
    }

    /*
     * If we got to here, we didn't find a solution, so we
     * need to apply interpolation.
     */
    return ((*interpol)(key, lo, hi, answer, bitsPerRGB));
}


/*
 *      NAME
 *		_XcmsMatVec - multiply a 3 x 3 by a 3 x 1 vector
 *
 *	SYNOPSIS
 */
void _XcmsMatVec(pMat, pIn, pOut)
    XcmsFloat *pMat, *pIn, *pOut;
/*
 *      DESCRIPTION
 *		Multiply the passed vector by the passed matrix to return a 
 *		vector. Matrix is 3x3, vectors are of length 3.
 *
 *	RETURNS
 *		void
 */
{
    int i, j;

    for (i = 0; i < 3; i++) {
	pOut[i] = 0.0;
	for (j = 0; j < 3; j++)
	    pOut[i] += *(pMat+(i*3)+j) * pIn[j];
    }
}


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/


/*
 *	NAME
 *		XcmsLRGB_RGB_ParseString
 *
 *	SYNOPSIS
 */
int
XcmsLRGB_RGB_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_RGB_FORMAT.
 *
 *	RETURNS
 *		0 if failed, non-zero otherwise.
 */
{
    register int n, i;
    int r, g, b;
    char c;
    char *pchar;
    unsigned short *pShort;

    /*
     * Check for old # format
     */
    if (*spec == '#') {
	/*
	 * Attempt to parse the value portion.
	 */
	spec++;
	n = strlen(spec);
	if (n != 3 && n != 6 && n != 9 && n != 12) {
	    return(XCMS_FAILURE);
	}

	n /= 3;
	g = b = 0;
	do {
	    r = g;
	    g = b;
	    b = 0;
	    for (i = n; --i >= 0; ) {
		c = *spec++;
		b <<= 4;
		if (c >= '0' && c <= '9')
		    b |= c - '0';
		/* assume string in lowercase
		else if (c >= 'A' && c <= 'F')
		    b |= c - ('A' - 10);
		*/
		else if (c >= 'a' && c <= 'f')
		    b |= c - ('a' - 10);
		else return (XCMS_FAILURE);
	    }
	} while (*spec != '\0');

	/*
	 * Succeeded !
	 */
	n <<= 2;
	n = 16 - n;
	pColor->spec.RGB.red =
		((unsigned long)r * 0xFFFF) / ((1 << n) - 1);
	pColor->spec.RGB.green =
		((unsigned long)g * 0xFFFF) / ((1 << n) - 1);
	pColor->spec.RGB.blue = 
		((unsigned long)b * 0xFFFF) / ((1 << n) - 1);
    } else {
	if ((pchar = strchr(spec, ':')) == NULL) {
	    return(XCMS_FAILURE);
	}
	n = (int)(pchar - spec);

	/*
	 * Check for proper prefix.
	 */
	if (strncmp(spec, XcmsRGB_prefix, n) != 0) {
	    return(XCMS_FAILURE);
	}

	/*
	 * Attempt to parse the value portion.
	 */
	spec += (n + 1);
	pShort = &pColor->spec.RGB.red;
	for (i = 0; i < 3; i++, pShort++, spec++) {
	    n = 0;
	    *pShort = 0;
	    while (*spec != '/' && *spec != '\0') {
		if (++n > 4) {
		    return(XCMS_FAILURE);
		}
		c = *spec++;
		*pShort <<= 4;
		if (c >= '0' && c <= '9')
		    *pShort |= c - '0';
		/* assume string in lowercase
		else if (c >= 'A' && c <= 'F')
		    *pShort |= c - ('A' - 10);
		*/
		else if (c >= 'a' && c <= 'f')
		    *pShort |= c - ('a' - 10);
		else return (XCMS_FAILURE);
	    }
	    if (n < 4) {
		*pShort = ((unsigned long)*pShort * 0xFFFF) / ((1 << n*4) - 1);
	    }
	}
    }
    pColor->format = XCMS_RGB_FORMAT;
    pColor->pixel = 0;
    return (XCMS_SUCCESS);
}


/*
 *	NAME
 *		XcmsLRGB_RGBi_ParseString
 *
 *	SYNOPSIS
 */
int
XcmsLRGB_RGBi_ParseString(spec, pColor)
    register char *spec;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		This routines takes a string and attempts to convert
 *		it into a XcmsColor structure with XCMS_RGBi_FORMAT.
 *		The assumed RGBi string syntax is:
 *		    RGBi:<r>/<g>/<b>
 *		Where r, g, and b are in string input format for floats
 *		consisting of:
 *		    a. an optional sign
 *		    b. a string of numbers possibly containing a decimal point,
 *		    c. an optional exponent field containing an 'E' or 'e'
 *			followed by a possibly signed integer string.
 *
 *	RETURNS
 *		0 if failed, non-zero otherwise.
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
    if (strncmp(spec, XcmsRGBi_prefix, n) != 0) {
	return(XCMS_FAILURE);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.RGBi.red,
	    &pColor->spec.RGBi.green,
	    &pColor->spec.RGBi.blue) != 3) {
	return(XCMS_FAILURE);
    }

    /*
     * Succeeded !
     */
    pColor->format = XCMS_RGBi_FORMAT;
    pColor->pixel = 0;
    return (XCMS_SUCCESS);
}


/*
 *	NAME
 *		LINEAR_RGB_CIEXYZ_to_RGBi - convert CIE XYZ to RGB
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
LINEAR_RGB_CIEXYZ_to_RGBi(pCCC, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    Bool *pCompressed;		/* pointer to an array of Bool		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGB format to RGBi format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression.
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with gamut
 *			compression.
 */
{
    LINEAR_RGB_SCCData *pSCCData;
    XcmsFloat tmp[3];
    int hasCompressed = 0;
    unsigned int i;
    XcmsColor *pColor = pXcmsColors_in_out;

    if (pCCC == NULL) {
	return(XCMS_FAILURE);
    }

    pSCCData = (LINEAR_RGB_SCCData *)pCCC->pPerScrnInfo->pSCCData;

    /*
     * XcmsColors should be White Point Adjusted, if necessary, by now!
     */

    /*
     * NEW!!! for extended gamut compression
     *
     * 1. Need to zero out pCompressed
     *
     * 2. Need to save initial address of pColor
     *
     * 3. Need to save initial address of pCompressed
     */

    for (i = 0; i < nColors; i++) {

	/* Make sure format is XCMS_CIEXYZ_FORMAT */
	if (pColor->format != XCMS_CIEXYZ_FORMAT) {
	    return(XCMS_FAILURE);
	}

	/* Multiply [A]-1 * [XYZ] to get RGB intensity */
	_XcmsMatVec((XcmsFloat *) pSCCData->XYZtoRGBmatrix,
		(XcmsFloat *) &pColor->spec, tmp);

	if ((MIN3 (tmp[0], tmp[1], tmp[2]) < -EPS) ||
	    (MAX3 (tmp[0], tmp[1], tmp[2]) > (1.0 + EPS))) {

	    /*
	     * RGBi out of screen's gamut
	     */

	    if (pCCC->gamutCompFunc == NULL) {
		/*
		 * Aha!! Here's that little trick that will allow
		 * gamut compression routines to get the out of bound
		 * RGBi.  
		 */
		bcopy((char *)tmp, (char *)&pColor->spec, sizeof(tmp));
		pColor->format = XCMS_RGBi_FORMAT;
		return(XCMS_FAILURE);
	    } else if ((*pCCC->gamutCompFunc)(pCCC, pXcmsColors_in_out, nColors,
		    i, pCompressed) == 0) {
		return(XCMS_FAILURE);
	    }

	    /*
	     * The gamut compression function should return colors in CIEXYZ
	     *	Also check again to if the new color is within gamut.
	     */
	    if (pColor->format != XCMS_CIEXYZ_FORMAT) {
		return(XCMS_FAILURE);
	    }
	    _XcmsMatVec((XcmsFloat *) pSCCData->XYZtoRGBmatrix,
		    (XcmsFloat *) &pColor->spec, tmp);
	    if ((MIN3 (tmp[0], tmp[1], tmp[2]) < -EPS) ||
		(MAX3 (tmp[0], tmp[1], tmp[2]) > (1.0 + EPS))) {
		return(XCMS_FAILURE);
	    }
	    hasCompressed++;
	}
	bcopy((char *)tmp, (char *)&pColor->spec, sizeof(tmp));
	/* These if statements are done to ensure the fudge factor is */
	/* is taken into account. */
	if (pColor->spec.RGBi.red < 0.0) {
		pColor->spec.RGBi.red = 0.0;
	} else if (pColor->spec.RGBi.red > 1.0) {
		pColor->spec.RGBi.red = 1.0;
	}
	if (pColor->spec.RGBi.green < 0.0) {
		pColor->spec.RGBi.green = 0.0;
	} else if (pColor->spec.RGBi.green > 1.0) {
		pColor->spec.RGBi.green = 1.0;
	}
	if (pColor->spec.RGBi.blue < 0.0) {
		pColor->spec.RGBi.blue = 0.0;
	} else if (pColor->spec.RGBi.blue > 1.0) {
		pColor->spec.RGBi.blue = 1.0;
	}
	(pColor++)->format = XCMS_RGBi_FORMAT;
    }
    return (hasCompressed ? XCMS_SUCCESS_WITH_COMPRESSION : XCMS_SUCCESS);
}


/*
 *	NAME
 *		LINEAR_RGBi_to_CIEXYZ - convert RGBi to CIEXYZ
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
LINEAR_RGB_RGBi_to_CIEXYZ(pCCC, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGBi format to CIEXYZ format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 */
{
    LINEAR_RGB_SCCData *pSCCData;
    XcmsFloat tmp[3];

    /*
     * pCompressed ignored in this function.
     */

    if (pCCC == NULL) {
	return(XCMS_FAILURE);
    }

    pSCCData = (LINEAR_RGB_SCCData *)pCCC->pPerScrnInfo->pSCCData;

    /*
     * XcmsColors should be White Point Adjusted, if necessary, by now!
     */

    while (nColors--) {

	/* Multiply [A]-1 * [XYZ] to get RGB intensity */
	_XcmsMatVec((XcmsFloat *) pSCCData->RGBtoXYZmatrix,
		(XcmsFloat *) &pXcmsColors_in_out->spec, tmp);

	bcopy((char *)tmp, (char *)&pXcmsColors_in_out->spec, sizeof(tmp));
	(pXcmsColors_in_out++)->format = XCMS_CIEXYZ_FORMAT;
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		LINEAR_RGB_RGBi_to_RGB
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
LINEAR_RGB_RGBi_to_RGB(pCCC, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGBi format to RGB format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded without gamut compression.
 *		XCMS_SUCCESS_WITH_COMPRESSION if succeeded with gamut
 *			compression.
 */
{
    LINEAR_RGB_SCCData *pSCCData;
    XcmsRGB tmpRGB;
    IntensityRec keyIRec, answerIRec;

    /*
     * pCompressed ignored in this function.
     */

    if (pCCC == NULL) {
	return(XCMS_FAILURE);
    }

    pSCCData = (LINEAR_RGB_SCCData *)pCCC->pPerScrnInfo->pSCCData;

    while (nColors--) {

	/* Make sure format is XCMS_RGBi_FORMAT */
	if (pXcmsColors_in_out->format != XCMS_RGBi_FORMAT) {
	    return(XCMS_FAILURE);
	}

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.red;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pRedTbl->pBase,
		(unsigned)pSCCData->pRedTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		IntensityCmp, IntensityInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGB.red = answerIRec.value;

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.green;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pGreenTbl->pBase,
		(unsigned)pSCCData->pGreenTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		IntensityCmp, IntensityInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGB.green = answerIRec.value;

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.blue;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pBlueTbl->pBase,
		(unsigned)pSCCData->pBlueTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		IntensityCmp, IntensityInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGB.blue = answerIRec.value;

	bcopy((char *)&tmpRGB, (char *)&pXcmsColors_in_out->spec, sizeof(XcmsRGB));
	(pXcmsColors_in_out++)->format = XCMS_RGB_FORMAT;
    }
    return(XCMS_SUCCESS);
}


/*
 *	NAME
 *		LINEAR_RGB_RGB_to_RGBi
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
LINEAR_RGB_RGB_to_RGBi(pCCC, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC *pCCC;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGB format to RGBi format.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed,
 *		XCMS_SUCCESS if succeeded.
 */
{
    LINEAR_RGB_SCCData *pSCCData;
    XcmsRGBi tmpRGBi;
    IntensityRec keyIRec, answerIRec;

    /*
     * pCompressed ignored in this function.
     */

    if (pCCC == NULL) {
	return(XCMS_FAILURE);
    }

    pSCCData = (LINEAR_RGB_SCCData *)pCCC->pPerScrnInfo->pSCCData;

    while (nColors--) {

	/* Make sure format is XCMS_RGB_FORMAT */
	if (pXcmsColors_in_out->format != XCMS_RGB_FORMAT) {
	    return(XCMS_FAILURE);
	}

	keyIRec.value = pXcmsColors_in_out->spec.RGB.red;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pRedTbl->pBase,
		(unsigned)pSCCData->pRedTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		ValueCmp, ValueInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGBi.red = answerIRec.intensity;

	keyIRec.value = pXcmsColors_in_out->spec.RGB.green;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pGreenTbl->pBase,
		(unsigned)pSCCData->pGreenTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		ValueCmp, ValueInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGBi.green = answerIRec.intensity;

	keyIRec.value = pXcmsColors_in_out->spec.RGB.blue;
	if (!_XcmsTableSearch((char *)&keyIRec, pCCC->visual->bits_per_rgb,
		(char *)pSCCData->pBlueTbl->pBase,
		(unsigned)pSCCData->pBlueTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		ValueCmp, ValueInterpolation, (char *)&answerIRec)) {
	    return(XCMS_FAILURE);
	}
	tmpRGBi.blue = answerIRec.intensity;

	bcopy((char *)&tmpRGBi, (char *)&pXcmsColors_in_out->spec, sizeof(XcmsRGBi));
	(pXcmsColors_in_out++)->format = XCMS_RGBi_FORMAT;
    }
    return(XCMS_SUCCESS);
}

/*
 *	NAME
 *		LINEAR_RGB_InitSCCData()
 *
 *	SYNOPSIS
 */
int
LINEAR_RGB_InitSCCData(dpy, screen_number, pPerScrnInfo)
    Display *dpy;
    int screen_number;
    XcmsPerScrnInfo *pPerScrnInfo;
/*
 *	DESCRIPTION
 *
 *	RETURNS
 *		0 if failed.
 *		1 if succeeded with no modifications.
 *
 */
{
    Atom  CorrectAtom = XInternAtom (dpy, XDCCC_CORRECT_ATOM_NAME, True);
    Atom  MatrixAtom  = XInternAtom (dpy, XDCCC_MATRIX_ATOM_NAME, True);
    int	  format_return, count, cType, nTables, nElements;
    unsigned long nitems_return, nbytes_return;
    char *property_return, *pChar;
    XcmsFloat *pValue;
    IntensityRec *pIRec;

    LINEAR_RGB_SCCData *pSCCData;

    /*
     * Allocate memory for pSCCData
     */
    if (!(pSCCData = (LINEAR_RGB_SCCData *) 
		      Xcalloc (1, sizeof(LINEAR_RGB_SCCData)))) {
	return(XCMS_FAILURE);
    }

    /*
     *	1. Get the XYZ to RGB Matrix
     *	2. Get the RGB to XYZ Matrix
     *	3. Compute the white point of the matrix
     */

    /* 
     *  First try to see if the XDXcmsCCC.conversion matrices property is
     *  loaded on the root window.  If it is then read it and set the
     *  screen info using this property.  If it is not use the default.
     */
    if (MatrixAtom != None &&
	_XcmsGetProperty (dpy, RootWindow(dpy, screen_number), MatrixAtom, 
	   &format_return, &nitems_return, &nbytes_return, &property_return) &&
	   nitems_return == 18) {

	/* Get the RGBtoXYZ and XYZtoRGB matrices */
	pValue = (XcmsFloat *) pSCCData;
	pChar = property_return;
	for (count = 0; count < 18; count++) {
	    *pValue++ = (XcmsFloat) _XcmsGetElement (format_return, &pChar) / 
			(XcmsFloat) XDCCC_NUMBER;
	}
	XFree (property_return);
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.X = pSCCData->RGBtoXYZmatrix[0][0] +
					      pSCCData->RGBtoXYZmatrix[0][1] +
					      pSCCData->RGBtoXYZmatrix[0][2];
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = pSCCData->RGBtoXYZmatrix[1][0] +
					      pSCCData->RGBtoXYZmatrix[1][1] +
					      pSCCData->RGBtoXYZmatrix[1][2];
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Z = pSCCData->RGBtoXYZmatrix[2][0] +
					      pSCCData->RGBtoXYZmatrix[2][1] +
					      pSCCData->RGBtoXYZmatrix[2][2];
	if ((pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y < (1.0 - EPS) )
		|| (pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y > (1.0 + EPS))) {
	    goto FreeSCCData;
	} else {
	    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = 1.0;
	}
	pPerScrnInfo->screenWhitePt.format = XCMS_CIEXYZ_FORMAT;
	pPerScrnInfo->screenWhitePt.pixel = 0;
#ifdef PDEBUG
	printf ("A Matrix values:\n");
	printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n",
		pSCCData->RGBtoXYZmatrix[0][0],
		pSCCData->RGBtoXYZmatrix[0][1],
		pSCCData->RGBtoXYZmatrix[0][2],
		pSCCData->RGBtoXYZmatrix[1][0],
		pSCCData->RGBtoXYZmatrix[1][1],
		pSCCData->RGBtoXYZmatrix[1][2],
		pSCCData->RGBtoXYZmatrix[2][0],
		pSCCData->RGBtoXYZmatrix[2][1],
		pSCCData->RGBtoXYZmatrix[2][2]);
	printf ("A- Matrix values:\n");
	printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n",
		pSCCData->XYZtoRGBmatrix[0][0],
		pSCCData->XYZtoRGBmatrix[0][1],
		pSCCData->XYZtoRGBmatrix[0][2],
		pSCCData->XYZtoRGBmatrix[1][0],
		pSCCData->XYZtoRGBmatrix[1][1],
		pSCCData->XYZtoRGBmatrix[1][2],
		pSCCData->XYZtoRGBmatrix[2][0],
		pSCCData->XYZtoRGBmatrix[2][1],
		pSCCData->XYZtoRGBmatrix[2][2]);
	printf ("Screen White Pt value: %f %f %f\n",
		pPerScrnInfo->screenWhitePt.spec.CIEXYZ.X,
		pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y,
		pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Z);
#endif /* PDEBUG */
    } else {
	/* when debug is on then use the statics otherwise just return. */
	goto FreeSCCData;
    }

    /*
     *	4. Get the Intensity Profile if it exists
     */
    if (CorrectAtom != None &&
	_XcmsGetProperty (dpy, RootWindow(dpy, screen_number), CorrectAtom,
	   &format_return, &nitems_return, &nbytes_return, &property_return) &&
	    nitems_return > 3) {

	pChar = property_return;
	cType = (int) _XcmsGetElement (format_return, &pChar);
	nTables = (int) _XcmsGetElement (format_return, &pChar);
	nElements = (int) _XcmsGetElement (format_return, &pChar);

	if (cType == 0) {
	    /* Red Intensity Table */
	    if (!(pSCCData->pRedTbl = (IntensityTbl *)
		    Xcalloc (1, sizeof(IntensityTbl)))) {
		goto FreeSCCData;
	    }
	    if (!(pSCCData->pRedTbl->pBase = (IntensityRec *)
		  Xcalloc (nElements, sizeof(IntensityRec)))) {
		goto FreeRedTbl;
	    }
	    pSCCData->pRedTbl->nEntries = nElements;
	    pIRec = (IntensityRec *) pSCCData->pRedTbl->pBase;
	    for (count = 0; count < nElements; count++, pIRec++) {
		pIRec->value = _XcmsGetElement (format_return, &pChar);
		pIRec->intensity = (XcmsFloat) 
		 _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
	    }
	    if (nTables == 1) {
		/* Green Intensity Table */
		pSCCData->pGreenTbl =pSCCData->pRedTbl;
		/* Blue Intensity Table */
		pSCCData->pBlueTbl = pSCCData->pRedTbl;
	    } else {
		/* Green Intensity Table */
		if (!(pSCCData->pGreenTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeRedTblElements;
		}
		if (!(pSCCData->pGreenTbl->pBase = (IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeGreenTbl;
		}
		pSCCData->pGreenTbl->nEntries = nElements;
		pIRec = (IntensityRec *) pSCCData->pGreenTbl->pBase;
		for (count = 0; count < nElements; count++, pIRec++) {
		    pIRec->value = _XcmsGetElement (format_return, &pChar);
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
		/* Blue Intensity Table */
		if (!(pSCCData->pBlueTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeGreenTblElements;
		}
		if (!(pSCCData->pBlueTbl->pBase = (IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeBlueTbl;
		}
		pSCCData->pBlueTbl->nEntries = nElements;
		pIRec = (IntensityRec *) pSCCData->pBlueTbl->pBase;
		for (count = 0; count < nElements; count++, pIRec++) {
		    pIRec->value = _XcmsGetElement (format_return, &pChar);
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
	    }	    
	} else {
	    /* Red Intensity Table */
	    if (!(pSCCData->pRedTbl = (IntensityTbl *)
		    Xcalloc (1, sizeof(IntensityTbl)))) {
		goto FreeSCCData;
	    }
	    if (!(pSCCData->pRedTbl->pBase = (IntensityRec *)
		  Xcalloc (nElements, sizeof(IntensityRec)))) {
		goto FreeRedTbl;
	    }
	    pSCCData->pRedTbl->nEntries = nElements;
	    pIRec = (IntensityRec *) pSCCData->pRedTbl->pBase;
	    for (count = 0; count < nElements; count++, pIRec++) {
		pIRec->value = count;
		pIRec->intensity = (XcmsFloat) 
		  _XcmsGetElement (format_return,&pChar)/(XcmsFloat)XDCCC_NUMBER;
	    }
	    if (nTables == 1) {
		/* Green Intensity Table */
		pSCCData->pGreenTbl = pSCCData->pRedTbl;
		/* Blue Intensity Table */
		pSCCData->pBlueTbl = pSCCData->pRedTbl;
	    } else {
		/* Green Intensity Table */
		if (!(pSCCData->pGreenTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeRedTblElements;
		}
		if (!(pSCCData->pGreenTbl->pBase =(IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeGreenTbl;
		}
		pSCCData->pGreenTbl->nEntries = nElements;
		pIRec = (IntensityRec *) pSCCData->pGreenTbl->pBase;
		for (count = 0; count < nElements; count++, pIRec++) {
		    pIRec->value = count;
		    pIRec->intensity = (XcmsFloat) 
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
		/* Blue Intensity Table */
		if (!(pSCCData->pBlueTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeGreenTblElements;
		}
		if (!(pSCCData->pBlueTbl->pBase =(IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeBlueTbl;
		}
		pSCCData->pBlueTbl->nEntries = nElements;
		pIRec = (IntensityRec *) pSCCData->pBlueTbl->pBase;
		for (count = 0; count < nElements; count++, pIRec++) {
		    pIRec->value = count;
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
	    }
	}
	XFree (property_return);
#ifdef ALLDEBUG
	printf ("Intensity Table  RED    %d\n", nElements);
	pIRec = (IntensityRec *) pSCCData->pRedTbl->pBase;
	for (count = 0; count < nElements; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
	if (pSCCData->pGreenTbl->pBase != pSCCData->pRedTbl->pBase) {
	    printf ("Intensity Table  GREEN  %d\n", nElements);
	    pIRec = (IntensityRec *)pSCCData->pGreenTbl->pBase;
	    for (count = 0; count < nElements; count++, pIRec++) {
		printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	    }
	}
	if (pSCCData->pBlueTbl->pBase != pSCCData->pRedTbl->pBase) {
	    printf ("Intensity Table  BLUE   %d\n", nElements);
	    pIRec = (IntensityRec *) pSCCData->pBlueTbl->pBase;
	    for (count = 0; count < nElements; count++, pIRec++) {
		printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	    }
	}
#endif /* ALLDEBUG */
    } else {
	XFree (property_return);
	goto FreeSCCData;
    }
    /* Free the old memory and use the new structure created. */
    LINEAR_RGB_FreeSCCData((LINEAR_RGB_SCCData *) pPerScrnInfo->pSCCData);

    pPerScrnInfo->pSCCFuncSet = (caddr_t) &LINEAR_RGB_SCCFuncSet;

    pPerScrnInfo->pSCCData = (caddr_t) pSCCData;

    pPerScrnInfo->state = XCMS_INIT_SUCCESS;

    return(XCMS_SUCCESS);

FreeBlueTblElements:
    free(pSCCData->pBlueTbl->pBase);

FreeBlueTbl:
    free(pSCCData->pBlueTbl);

FreeGreenTblElements:
    free(pSCCData->pBlueTbl->pBase);

FreeGreenTbl:
    free(pSCCData->pGreenTbl);

FreeRedTblElements:
    free(pSCCData->pRedTbl->pBase);

FreeRedTbl:
    free(pSCCData->pRedTbl);

FreeSCCData:
    free(pSCCData);
    pPerScrnInfo->state = XCMS_INIT_NONE;
    return(XCMS_FAILURE);
}

/*
 *	NAME
 *		_XcmsInitScrnDefaultInfo
 *
 *	SYNOPSIS
 */
int
_XcmsLRGB_InitScrnDefault(dpy, screen_number, pPerScrnInfo)
    Display *dpy;
    int screen_number;
    XcmsPerScrnInfo *pPerScrnInfo;
/*
 *	DESCRIPTION
 *		Given a display and screen number, this routine attempts
 *		to initialize the TekCMS per Screen Info structure
 *		(XcmsPerScrnInfo) with defaults.
 *
 *	RETURNS
 *		Returns zero if initialization failed; non-zero otherwise.
 */
{
    pPerScrnInfo->pSCCData = (caddr_t)&Default_RGB_SCCData;
    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.X =
		Default_RGB_SCCData.RGBtoXYZmatrix[0][0] +
		Default_RGB_SCCData.RGBtoXYZmatrix[0][1] +
		Default_RGB_SCCData.RGBtoXYZmatrix[0][2];
    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y =
		Default_RGB_SCCData.RGBtoXYZmatrix[1][0] +
		Default_RGB_SCCData.RGBtoXYZmatrix[1][1] +
		Default_RGB_SCCData.RGBtoXYZmatrix[1][2];
    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Z =
		Default_RGB_SCCData.RGBtoXYZmatrix[2][0] +
		Default_RGB_SCCData.RGBtoXYZmatrix[2][1] +
		Default_RGB_SCCData.RGBtoXYZmatrix[2][2];
    if ((pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y < (1.0 - EPS) )
	    || (pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y > (1.0 + EPS))) {
	pPerScrnInfo->pSCCData = (caddr_t)NULL;
	pPerScrnInfo->state = XCMS_INIT_NONE;
	return(0);
    }
    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = 1.0;
    pPerScrnInfo->screenWhitePt.format = XCMS_CIEXYZ_FORMAT;
    pPerScrnInfo->screenWhitePt.pixel = 0;
    pPerScrnInfo->pSCCFuncSet = (caddr_t)&LINEAR_RGB_SCCFuncSet;
    pPerScrnInfo->state = XCMS_INIT_DEFAULT;
    fprintf(stderr, "TekCMS:  WARNING -- Unable to acquire Screen Color Characterization Data \n\t required for accurate color conversion to device space for: \n\t\tDisplay  -- %s \n\t\tScreen # -- %d \n\t Using default data.\n\n",
	    dpy->display_name, screen_number);	
    return(1);
}
