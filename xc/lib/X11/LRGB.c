/* $XConsortium: XcmsLRGB.c,v 1.10 91/05/14 14:32:07 rws Exp $" */

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

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include "Xlibint.h"
#include "Xcmsint.h"

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern char XcmsRGB_prefix[];
extern char XcmsRGBi_prefix[];


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
 *      FORWARD DECLARATIONS
 */
static void LINEAR_RGB_FreeSCCData();
Status XcmsRGBToRGBi();
Status XcmsRGBiToCIEXYZ();
Status XcmsCIEXYZToRGBi();
Status XcmsRGBiToRGB();
static int LINEAR_RGB_InitSCCData();
int XcmsLRGB_RGB_ParseString();
int XcmsLRGB_RGBi_ParseString();

/*
 *      LOCALS VARIABLES
 *		Variables local to this package.
 *		    Usage example:
 *		        static int	ExampleLocalVar;
 */

static unsigned short Const HALF[17] = {
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

static unsigned short Const MASK[17] = {
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
     * in series will convert an XcmsColor structure from XcmsRGBFormat
     * to XcmsCIEXYZFormat.
     */
static XcmsFuncPtr Fl_RGB_to_CIEXYZ[] = {
    XcmsRGBToRGBi,
    XcmsRGBiToCIEXYZ,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XcmsCIEXYZFormat
     * to XcmsRGBFormat.
     */
static XcmsFuncPtr Fl_CIEXYZ_to_RGB[] = {
    XcmsCIEXYZToRGBi,
    XcmsRGBiToRGB,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XcmsRGBiFormat
     * to XcmsCIEXYZFormat.
     */
static XcmsFuncPtr Fl_RGBi_to_CIEXYZ[] = {
    XcmsRGBiToCIEXYZ,
    NULL
};

    /*
     * A NULL terminated array of function pointers that when applied
     * in series will convert an XcmsColor structure from XcmsCIEXYZFormat
     * to XcmsRGBiFormat.
     */
static XcmsFuncPtr Fl_CIEXYZ_to_RGBi[] = {
    XcmsCIEXYZToRGBi,
    NULL
};

    /*
     * RGBi Color Spaces
     */
XcmsColorSpace	XcmsRGBiColorSpace =
    {
	XcmsRGBi_prefix,	/* prefix */
	XcmsRGBiFormat,		/* id */
	XcmsLRGB_RGBi_ParseString,	/* parseString */
	Fl_RGBi_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_RGBi,	/* from_CIEXYZ */
	1
    };

    /*
     * RGB Color Spaces
     */
XcmsColorSpace	XcmsRGBColorSpace =
    {
	XcmsRGB_prefix,		/* prefix */
	XcmsRGBFormat,		/* id */
	XcmsLRGB_RGB_ParseString,	/* parseString */
	Fl_RGB_to_CIEXYZ,	/* to_CIEXYZ */
	Fl_CIEXYZ_to_RGB,	/* from_CIEXYZ */
	1
    };

    /*
     * Device-Independent Color Spaces known to the 
     * LINEAR_RGB Screen Color Characteristics Function Set.
     */
static XcmsColorSpace	*DDColorSpaces[] = {
    &XcmsRGBColorSpace,
    &XcmsRGBiColorSpace,
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
XcmsSCCFuncSet	XcmsLinearRGBFunctionSet =
    {
	&DDColorSpaces[0],	/* pDDColorSpaces */
	LINEAR_RGB_InitSCCData,	/* pInitScrnFunc */
	LINEAR_RGB_FreeSCCData	/* pFreeSCCData */
    };

/*
 *	DESCRIPTION
 *		Contents of Default SCCData should be replaced if other
 *		data should be used as default.
 *
 *
 */

/*
 * NAME		Tektronix 19" (Sony) CRT
 * PART_NUMBER		119-2451-00
 * MODEL		Tek4300, Tek4800
 */

static IntensityRec Const Default_RGB_RedTuples[] = {
    /* {unsigned short value, XcmsFloat intensity} */
            0x0000,    0.000000,
            0x0900,    0.000000,
            0x0a00,    0.000936,
            0x0f00,    0.001481,
            0x1400,    0.002329,
            0x1900,    0.003529,
            0x1e00,    0.005127,
            0x2300,    0.007169,
            0x2800,    0.009699,
            0x2d00,    0.012759,
            0x3200,    0.016392,
            0x3700,    0.020637,
            0x3c00,    0.025533,
            0x4100,    0.031119,
            0x4600,    0.037431,
            0x4b00,    0.044504,
            0x5000,    0.052373,
            0x5500,    0.061069,
            0x5a00,    0.070624,
            0x5f00,    0.081070,
            0x6400,    0.092433,
            0x6900,    0.104744,
            0x6e00,    0.118026,
            0x7300,    0.132307,
            0x7800,    0.147610,
            0x7d00,    0.163958,
            0x8200,    0.181371,
            0x8700,    0.199871,
            0x8c00,    0.219475,
            0x9100,    0.240202,
            0x9600,    0.262069,
            0x9b00,    0.285089,
            0xa000,    0.309278,
            0xa500,    0.334647,
            0xaa00,    0.361208,
            0xaf00,    0.388971,
            0xb400,    0.417945,
            0xb900,    0.448138,
            0xbe00,    0.479555,
            0xc300,    0.512202,
            0xc800,    0.546082,
            0xcd00,    0.581199,
            0xd200,    0.617552,
            0xd700,    0.655144,
            0xdc00,    0.693971,
            0xe100,    0.734031,
            0xe600,    0.775322,
            0xeb00,    0.817837,
            0xf000,    0.861571,
            0xf500,    0.906515,
            0xfa00,    0.952662,
            0xff00,    1.000000
};

static IntensityRec Const Default_RGB_GreenTuples[] = {
    /* {unsigned short value, XcmsFloat intensity} */
            0x0000,      0.000000,
            0x1300,    0.000000,
            0x1400,    0.000832,
            0x1900,    0.001998,
            0x1e00,    0.003612,
            0x2300,    0.005736,
            0x2800,    0.008428,
            0x2d00,    0.011745,
            0x3200,    0.015740,
            0x3700,    0.020463,
            0x3c00,    0.025960,
            0x4100,    0.032275,
            0x4600,    0.039449,
            0x4b00,    0.047519,
            0x5000,    0.056520,
            0x5500,    0.066484,
            0x5a00,    0.077439,
            0x5f00,    0.089409,
            0x6400,    0.102418,
            0x6900,    0.116485,
            0x6e00,    0.131625,
            0x7300,    0.147853,
            0x7800,    0.165176,
            0x7d00,    0.183604,
            0x8200,    0.203140,
            0x8700,    0.223783,
            0x8c00,    0.245533,
            0x9100,    0.268384,
            0x9600,    0.292327,
            0x9b00,    0.317351,
            0xa000,    0.343441,
            0xa500,    0.370580,
            0xaa00,    0.398747,
            0xaf00,    0.427919,
            0xb400,    0.458068,
            0xb900,    0.489165,
            0xbe00,    0.521176,
            0xc300,    0.554067,
            0xc800,    0.587797,
            0xcd00,    0.622324,
            0xd200,    0.657604,
            0xd700,    0.693588,
            0xdc00,    0.730225,
            0xe100,    0.767459,
            0xe600,    0.805235,
            0xeb00,    0.843491,
            0xf000,    0.882164,
            0xf500,    0.921187,
            0xfa00,    0.960490,
            0xff00,    1.000000
};

static IntensityRec Const Default_RGB_BlueTuples[] = {
    /* {unsigned short value, XcmsFloat intensity} */
            0x0000,    0.000000,
            0x0e00,    0.000000,
            0x0f00,    0.001341,
            0x1400,    0.002080,
            0x1900,    0.003188,
            0x1e00,    0.004729,
            0x2300,    0.006766,
            0x2800,    0.009357,
            0x2d00,    0.012559,
            0x3200,    0.016424,
            0x3700,    0.021004,
            0x3c00,    0.026344,
            0x4100,    0.032489,
            0x4600,    0.039481,
            0x4b00,    0.047357,
            0x5000,    0.056154,
            0x5500,    0.065903,
            0x5a00,    0.076634,
            0x5f00,    0.088373,
            0x6400,    0.101145,
            0x6900,    0.114968,
            0x6e00,    0.129862,
            0x7300,    0.145841,
            0x7800,    0.162915,
            0x7d00,    0.181095,
            0x8200,    0.200386,
            0x8700,    0.220791,
            0x8c00,    0.242309,
            0x9100,    0.264937,
            0x9600,    0.288670,
            0x9b00,    0.313499,
            0xa000,    0.339410,
            0xa500,    0.366390,
            0xaa00,    0.394421,
            0xaf00,    0.423481,
            0xb400,    0.453547,
            0xb900,    0.484592,
            0xbe00,    0.516587,
            0xc300,    0.549498,
            0xc800,    0.583291,
            0xcd00,    0.617925,
            0xd200,    0.653361,
            0xd700,    0.689553,
            0xdc00,    0.726454,
            0xe100,    0.764013,
            0xe600,    0.802178,
            0xeb00,    0.840891,
            0xf000,    0.880093,
            0xf500,    0.919723,
            0xfa00,    0.959715,
            0xff00,    1.00000
};

static IntensityTbl Default_RGB_RedTbl = {
    /* IntensityRec *pBase */
	(IntensityRec *) Default_RGB_RedTuples,
    /* unsigned int nEntries */
	52
};

static IntensityTbl Default_RGB_GreenTbl = {
    /* IntensityRec *pBase */
	(IntensityRec *)Default_RGB_GreenTuples,
    /* unsigned int nEntries */
	50
};

static IntensityTbl Default_RGB_BlueTbl = {
    /* IntensityRec *pBase */
	(IntensityRec *)Default_RGB_BlueTuples,
    /* unsigned int nEntries */
	51
};

static LINEAR_RGB_SCCData Default_RGB_SCCData = {

    /* XcmsFloat XYZtoRGBmatrix[3][3] */
       3.48340481253539000, -1.52176374927285200, -0.55923133354049780,
      -1.07152751306193600,  1.96593795204372400,  0.03673691339553462,
       0.06351179790497788, -0.20020501000496480,  0.81070942031648220,

    /* XcmsFloat RGBtoXYZmatrix[3][3] */
       0.38106149108714790, 0.32025712365352110, 0.24834578525933100,
       0.20729745115140850, 0.68054638776373240, 0.11215616108485920,
       0.02133944350088028, 0.14297193020246480, 1.24172892629665500,

    /* IntensityTbl *pRedTbl */
	&Default_RGB_RedTbl,

    /* IntensityTbl *pGreenTbl */
	&Default_RGB_GreenTbl,

    /* IntensityTbl *pBlueTbl */
	&Default_RGB_BlueTbl
};

/************************************************************************
 *									*
 *			PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		LINEAR_RGB_InitSCCData()
 *
 *	SYNOPSIS
 */
static Status
LINEAR_RGB_InitSCCData(dpy, screenNumber, pPerScrnInfo)
    Display *dpy;
    int screenNumber;
    XcmsPerScrnInfo *pPerScrnInfo;
/*
 *	DESCRIPTION
 *
 *	RETURNS
 *		XcmsFailure if failed.
 *		XcmsSuccess if succeeded.
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

    LINEAR_RGB_SCCData *pScreenData;

    /*
     * Allocate memory for pScreenData
     */
    if (!(pScreenData = (LINEAR_RGB_SCCData *) 
		      Xcalloc (1, sizeof(LINEAR_RGB_SCCData)))) {
	return(XcmsFailure);
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
	_XcmsGetProperty (dpy, RootWindow(dpy, screenNumber), MatrixAtom, 
	   &format_return, &nitems_return, &nbytes_return, &property_return) &&
	   nitems_return == 18) {

	/* Get the RGBtoXYZ and XYZtoRGB matrices */
	pValue = (XcmsFloat *) pScreenData;
	pChar = property_return;
	for (count = 0; count < 18; count++) {
	    *pValue++ = (XcmsFloat)_XcmsGetElement(format_return, &pChar)
		    / (XcmsFloat)XDCCC_NUMBER;
	}
	XFree (property_return);
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.X = pScreenData->RGBtoXYZmatrix[0][0] +
					      pScreenData->RGBtoXYZmatrix[0][1] +
					      pScreenData->RGBtoXYZmatrix[0][2];
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = pScreenData->RGBtoXYZmatrix[1][0] +
					      pScreenData->RGBtoXYZmatrix[1][1] +
					      pScreenData->RGBtoXYZmatrix[1][2];
	pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Z = pScreenData->RGBtoXYZmatrix[2][0] +
					      pScreenData->RGBtoXYZmatrix[2][1] +
					      pScreenData->RGBtoXYZmatrix[2][2];
	if ((pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y < (1.0 - EPS) )
		|| (pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y > (1.0 + EPS))) {
	    goto FreeSCCData;
	} else {
	    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = 1.0;
	}
	pPerScrnInfo->screenWhitePt.format = XcmsCIEXYZFormat;
	pPerScrnInfo->screenWhitePt.pixel = 0;
#ifdef PDEBUG
	printf ("A Matrix values:\n");
	printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n",
		pScreenData->RGBtoXYZmatrix[0][0],
		pScreenData->RGBtoXYZmatrix[0][1],
		pScreenData->RGBtoXYZmatrix[0][2],
		pScreenData->RGBtoXYZmatrix[1][0],
		pScreenData->RGBtoXYZmatrix[1][1],
		pScreenData->RGBtoXYZmatrix[1][2],
		pScreenData->RGBtoXYZmatrix[2][0],
		pScreenData->RGBtoXYZmatrix[2][1],
		pScreenData->RGBtoXYZmatrix[2][2]);
	printf ("A- Matrix values:\n");
	printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n",
		pScreenData->XYZtoRGBmatrix[0][0],
		pScreenData->XYZtoRGBmatrix[0][1],
		pScreenData->XYZtoRGBmatrix[0][2],
		pScreenData->XYZtoRGBmatrix[1][0],
		pScreenData->XYZtoRGBmatrix[1][1],
		pScreenData->XYZtoRGBmatrix[1][2],
		pScreenData->XYZtoRGBmatrix[2][0],
		pScreenData->XYZtoRGBmatrix[2][1],
		pScreenData->XYZtoRGBmatrix[2][2]);
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
	_XcmsGetProperty (dpy, RootWindow(dpy, screenNumber), CorrectAtom,
	   &format_return, &nitems_return, &nbytes_return, &property_return) &&
	    nitems_return > 3) {

	pChar = property_return;
	cType = _XcmsGetElement(format_return, &pChar);
	nTables = _XcmsGetElement(format_return, &pChar);

	if (cType == 0) {
	    /* Red Intensity Table */
	    if (!(pScreenData->pRedTbl = (IntensityTbl *)
		    Xcalloc (1, sizeof(IntensityTbl)))) {
		goto FreeSCCData;
	    }
	    nElements = pScreenData->pRedTbl->nEntries = 
		    _XcmsGetElement(format_return, &pChar);
	    if (!(pScreenData->pRedTbl->pBase = (IntensityRec *)
		  Xcalloc (nElements, sizeof(IntensityRec)))) {
		goto FreeRedTbl;
	    }
	    pIRec = (IntensityRec *) pScreenData->pRedTbl->pBase;
	    for (; nElements--; pIRec++) {
		pIRec->value = _XcmsGetElement (format_return, &pChar);
		pIRec->intensity = (XcmsFloat) 
		 _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
	    }
	    if (nTables == 1) {
		/* Green Intensity Table */
		pScreenData->pGreenTbl = pScreenData->pRedTbl;
		/* Blue Intensity Table */
		pScreenData->pBlueTbl = pScreenData->pRedTbl;
	    } else {
		/* Green Intensity Table */
		if (!(pScreenData->pGreenTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeRedTblElements;
		}
		nElements = pScreenData->pGreenTbl->nEntries =
			(int) _XcmsGetElement (format_return, &pChar);
		if (!(pScreenData->pGreenTbl->pBase = (IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeGreenTbl;
		}
		pIRec = (IntensityRec *) pScreenData->pGreenTbl->pBase;
		for (; nElements--; pIRec++) {
		    pIRec->value = _XcmsGetElement (format_return, &pChar);
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
		/* Blue Intensity Table */
		if (!(pScreenData->pBlueTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeGreenTblElements;
		}
		nElements = pScreenData->pBlueTbl->nEntries =
			(int) _XcmsGetElement (format_return, &pChar);
		if (!(pScreenData->pBlueTbl->pBase = (IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeBlueTbl;
		}
		pIRec = (IntensityRec *) pScreenData->pBlueTbl->pBase;
		for (; nElements--; pIRec++) {
		    pIRec->value = _XcmsGetElement (format_return, &pChar);
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
	    }	    
	} else {
	    /* Red Intensity Table */
	    if (!(pScreenData->pRedTbl = (IntensityTbl *)
		    Xcalloc (1, sizeof(IntensityTbl)))) {
		goto FreeSCCData;
	    }
	    nElements = pScreenData->pRedTbl->nEntries =
		    (int) _XcmsGetElement (format_return, &pChar);
	    if (!(pScreenData->pRedTbl->pBase = (IntensityRec *)
		  Xcalloc (nElements, sizeof(IntensityRec)))) {
		goto FreeRedTbl;
	    }
	    pIRec = (IntensityRec *) pScreenData->pRedTbl->pBase;
	    for (; nElements--; pIRec++) {
		pIRec->value = count;
		pIRec->intensity = (XcmsFloat) 
		  _XcmsGetElement (format_return,&pChar)/(XcmsFloat)XDCCC_NUMBER;
	    }
	    if (nTables == 1) {
		/* Green Intensity Table */
		pScreenData->pGreenTbl = pScreenData->pRedTbl;
		/* Blue Intensity Table */
		pScreenData->pBlueTbl = pScreenData->pRedTbl;
	    } else {
		/* Green Intensity Table */
		if (!(pScreenData->pGreenTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeRedTblElements;
		}
		nElements = pScreenData->pGreenTbl->nEntries =
			(int) _XcmsGetElement (format_return, &pChar);
		if (!(pScreenData->pGreenTbl->pBase =(IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeGreenTbl;
		}
		pIRec = (IntensityRec *) pScreenData->pGreenTbl->pBase;
		for (; nElements; pIRec++) {
		    pIRec->value = count;
		    pIRec->intensity = (XcmsFloat) 
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
		/* Blue Intensity Table */
		if (!(pScreenData->pBlueTbl = (IntensityTbl *)
			Xcalloc (1, sizeof(IntensityTbl)))) {
		    goto FreeGreenTblElements;
		}
		nElements = pScreenData->pBlueTbl->nEntries =
			(int) _XcmsGetElement (format_return, &pChar);
		if (!(pScreenData->pBlueTbl->pBase =(IntensityRec *)
		      Xcalloc (nElements, sizeof(IntensityRec)))) {
		    goto FreeBlueTbl;
		}
		pIRec = (IntensityRec *) pScreenData->pBlueTbl->pBase;
		for (; nElements--; pIRec++) {
		    pIRec->value = count;
		    pIRec->intensity = (XcmsFloat)
		     _XcmsGetElement (format_return, &pChar)/(XcmsFloat)XDCCC_NUMBER;
		}
	    }
	}
	XFree (property_return);
#ifdef ALLDEBUG
	printf ("Intensity Table  RED    %d\n", pScreenData->pRedTbl->nEntries);
	pIRec = (IntensityRec *) pScreenData->pRedTbl->pBase;
	for (count = 0; count < pScreenData->pRedTbl->nEntries; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
	if (pScreenData->pGreenTbl->pBase != pScreenData->pRedTbl->pBase) {
	    printf ("Intensity Table  GREEN  %d\n", pScreenData->pGreenTbl->nEntries);
	    pIRec = (IntensityRec *)pScreenData->pGreenTbl->pBase;
	    for (count = 0; count < pScreenData->pGreenTbl->nEntries; count++, pIRec++) {
		printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	    }
	}
	if (pScreenData->pBlueTbl->pBase != pScreenData->pRedTbl->pBase) {
	    printf ("Intensity Table  BLUE   %d\n", pScreenData->pBlueTbl->nEntries);
	    pIRec = (IntensityRec *) pScreenData->pBlueTbl->pBase;
	    for (count = 0; count < pScreenData->pBlueTbl->nEntries; count++, pIRec++) {
		printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	    }
	}
#endif /* ALLDEBUG */
    } else {
	XFree (property_return);
	goto FreeSCCData;
    }
    /* Free the old memory and use the new structure created. */
    LINEAR_RGB_FreeSCCData((LINEAR_RGB_SCCData *) pPerScrnInfo->screenData);

    pPerScrnInfo->functionSet = (XPointer) &XcmsLinearRGBFunctionSet;

    pPerScrnInfo->screenData = (XPointer) pScreenData;

    pPerScrnInfo->state = XcmsInitSuccess;

    return(XcmsSuccess);

FreeBlueTblElements:
    free(pScreenData->pBlueTbl->pBase);

FreeBlueTbl:
    free(pScreenData->pBlueTbl);

FreeGreenTblElements:
    free(pScreenData->pBlueTbl->pBase);

FreeGreenTbl:
    free(pScreenData->pGreenTbl);

FreeRedTblElements:
    free(pScreenData->pRedTbl->pBase);

FreeRedTbl:
    free(pScreenData->pRedTbl);

FreeSCCData:
    free(pScreenData);
    pPerScrnInfo->state = XcmsInitNone;
    return(XcmsFailure);
}


/*
 *	NAME
 *		LINEAR_RGB_FreeSCCData()
 *
 *	SYNOPSIS
 */
static void
LINEAR_RGB_FreeSCCData(pScreenData)
    LINEAR_RGB_SCCData *pScreenData;
/*
 *	DESCRIPTION
 *
 *	RETURNS
 *		0 if failed.
 *		1 if succeeded with no modifications.
 *
 */
{
    if (pScreenData) {
	if (pScreenData->pRedTbl) {
	    if (pScreenData->pGreenTbl) {
		if (pScreenData->pRedTbl->pBase != 
		    pScreenData->pGreenTbl->pBase) {
		    if (pScreenData->pGreenTbl->pBase) {
			free (pScreenData->pGreenTbl->pBase);
		    }
		}
		if (pScreenData->pGreenTbl != pScreenData->pRedTbl) {
		    free (pScreenData->pGreenTbl);
		}
	    }
	    if (pScreenData->pBlueTbl) {
		if (pScreenData->pRedTbl->pBase != 
		    pScreenData->pBlueTbl->pBase) {
		    if (pScreenData->pBlueTbl->pBase) {
			free (pScreenData->pBlueTbl->pBase);
		    }
		}
		if (pScreenData->pBlueTbl != pScreenData->pRedTbl) {
		    free (pScreenData->pBlueTbl);
		}
	    }
	    if (pScreenData->pRedTbl->pBase) {
		free (pScreenData->pRedTbl->pBase);
	    }
	    free (pScreenData->pRedTbl);
	}
	free (pScreenData);
    }
}



/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/
/*
 *	NAME
 *		ValueCmp
 *
 *	SYNOPSIS
 */
int
_XcmsValueCmp (p1, p2)
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
int
_XcmsIntensityCmp (p1, p2)
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
	return (XcmsSuccess);
    }
    return (XcmsFailure);
}

/*
 *	NAME
 *		ValueInterpolation
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
int
_XcmsValueInterpolation (key, lo, hi, answer, bitsPerRGB)
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

    ratio = ((XcmsFloat)key->value - (XcmsFloat)lo->value) / 
	((XcmsFloat)hi->value - (XcmsFloat)lo->value);
    answer->value = key->value;
    answer->intensity = (hi->intensity - lo->intensity) * ratio;
    answer->intensity += lo->intensity;
    return (XcmsSuccess);
}

/*
 *	NAME
 *		IntensityInterpolation
 *
 *	SYNOPSIS
 */
int
_XcmsIntensityInterpolation (key, lo, hi, answer, bitsPerRGB)
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
    tmp = (((XcmsFloat)hi->value - (XcmsFloat)lo->value) * ratio);
    answer->value = (lo->value + tmp + HALF[bitsPerRGB]) & MASK[bitsPerRGB];
    return (XcmsSuccess);
}


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

    last = hi = base + ((nel - 1) * nKeyPtrSize);
    mid = lo = base;

    while (mid != last) {
	last = mid;
	mid = lo + ((int)((((int)hi - (int)lo) / nKeyPtrSize) / 2) *
		nKeyPtrSize);
	result = compar (key, mid);
	if (result == 0) {

	    bcopy(mid, answer, nKeyPtrSize);
	    return (XcmsSuccess);
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
 *		it into a XcmsColor structure with XcmsRGBFormat.
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
	    return(XcmsFailure);
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
		else return (XcmsFailure);
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
	    return(XcmsFailure);
	}
	n = (int)(pchar - spec);

	/*
	 * Check for proper prefix.
	 */
	if (strncmp(spec, XcmsRGB_prefix, n) != 0) {
	    return(XcmsFailure);
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
		    return(XcmsFailure);
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
		else return (XcmsFailure);
	    }
	    if (n < 4) {
		*pShort = ((unsigned long)*pShort * 0xFFFF) / ((1 << n*4) - 1);
	    }
	}
    }
    pColor->format = XcmsRGBFormat;
    pColor->pixel = 0;
    return (XcmsSuccess);
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
 *		it into a XcmsColor structure with XcmsRGBiFormat.
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
	return(XcmsFailure);
    }
    n = (int)(pchar - spec);

    /*
     * Check for proper prefix.
     */
    if (strncmp(spec, XcmsRGBi_prefix, n) != 0) {
	return(XcmsFailure);
    }

    /*
     * Attempt to parse the value portion.
     */
    if (sscanf(spec + n + 1, "%lf/%lf/%lf",
	    &pColor->spec.RGBi.red,
	    &pColor->spec.RGBi.green,
	    &pColor->spec.RGBi.blue) != 3) {
	return(XcmsFailure);
    }

    /*
     * Succeeded !
     */
    pColor->format = XcmsRGBiFormat;
    pColor->pixel = 0;
    return (XcmsSuccess);
}


/*
 *	NAME
 *		XcmsCIEXYZToRGBi - convert CIE XYZ to RGB
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
XcmsCIEXYZToRGBi(ccc, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC ccc;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    Bool *pCompressed;		/* pointer to an array of Bool		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGB format to RGBi format.
 *
 *	RETURNS
 *		XcmsFailure if failed,
 *		XcmsSuccess if succeeded without gamut compression.
 *		XcmsSuccessWithCompression if succeeded with gamut
 *			compression.
 */
{
    LINEAR_RGB_SCCData *pScreenData;
    XcmsFloat tmp[3];
    int hasCompressed = 0;
    unsigned int i;
    XcmsColor *pColor = pXcmsColors_in_out;

    if (ccc == NULL) {
	return(XcmsFailure);
    }

    pScreenData = (LINEAR_RGB_SCCData *)ccc->pPerScrnInfo->screenData;

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

	/* Make sure format is XcmsCIEXYZFormat */
	if (pColor->format != XcmsCIEXYZFormat) {
	    return(XcmsFailure);
	}

	/* Multiply [A]-1 * [XYZ] to get RGB intensity */
	_XcmsMatVec((XcmsFloat *) pScreenData->XYZtoRGBmatrix,
		(XcmsFloat *) &pColor->spec, tmp);

	if ((MIN3 (tmp[0], tmp[1], tmp[2]) < -EPS) ||
	    (MAX3 (tmp[0], tmp[1], tmp[2]) > (1.0 + EPS))) {

	    /*
	     * RGBi out of screen's gamut
	     */

	    if (ccc->gamutCompProc == NULL) {
		/*
		 * Aha!! Here's that little trick that will allow
		 * gamut compression routines to get the out of bound
		 * RGBi.  
		 */
		bcopy((char *)tmp, (char *)&pColor->spec, sizeof(tmp));
		pColor->format = XcmsRGBiFormat;
		return(XcmsFailure);
	    } else if ((*ccc->gamutCompProc)(ccc, pXcmsColors_in_out, nColors,
		    i, pCompressed) == 0) {
		return(XcmsFailure);
	    }

	    /*
	     * The gamut compression function should return colors in CIEXYZ
	     *	Also check again to if the new color is within gamut.
	     */
	    if (pColor->format != XcmsCIEXYZFormat) {
		return(XcmsFailure);
	    }
	    _XcmsMatVec((XcmsFloat *) pScreenData->XYZtoRGBmatrix,
		    (XcmsFloat *) &pColor->spec, tmp);
	    if ((MIN3 (tmp[0], tmp[1], tmp[2]) < -EPS) ||
		(MAX3 (tmp[0], tmp[1], tmp[2]) > (1.0 + EPS))) {
		return(XcmsFailure);
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
	(pColor++)->format = XcmsRGBiFormat;
    }
    return (hasCompressed ? XcmsSuccessWithCompression : XcmsSuccess);
}


/*
 *	NAME
 *		LINEAR_RGBi_to_CIEXYZ - convert RGBi to CIEXYZ
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
XcmsRGBiToCIEXYZ(ccc, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC ccc;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGBi format to CIEXYZ format.
 *
 *	RETURNS
 *		XcmsFailure if failed,
 *		XcmsSuccess if succeeded.
 */
{
    LINEAR_RGB_SCCData *pScreenData;
    XcmsFloat tmp[3];

    /*
     * pCompressed ignored in this function.
     */

    if (ccc == NULL) {
	return(XcmsFailure);
    }

    pScreenData = (LINEAR_RGB_SCCData *)ccc->pPerScrnInfo->screenData;

    /*
     * XcmsColors should be White Point Adjusted, if necessary, by now!
     */

    while (nColors--) {

	/* Multiply [A]-1 * [XYZ] to get RGB intensity */
	_XcmsMatVec((XcmsFloat *) pScreenData->RGBtoXYZmatrix,
		(XcmsFloat *) &pXcmsColors_in_out->spec, tmp);

	bcopy((char *)tmp, (char *)&pXcmsColors_in_out->spec, sizeof(tmp));
	(pXcmsColors_in_out++)->format = XcmsCIEXYZFormat;
    }
    return(XcmsSuccess);
}


/*
 *	NAME
 *		XcmsRGBiToRGB
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
XcmsRGBiToRGB(ccc, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC ccc;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGBi format to RGB format.
 *
 *	RETURNS
 *		XcmsFailure if failed,
 *		XcmsSuccess if succeeded without gamut compression.
 *		XcmsSuccessWithCompression if succeeded with gamut
 *			compression.
 */
{
    LINEAR_RGB_SCCData *pScreenData;
    XcmsRGB tmpRGB;
    IntensityRec keyIRec, answerIRec;

    /*
     * pCompressed ignored in this function.
     */

    if (ccc == NULL) {
	return(XcmsFailure);
    }

    pScreenData = (LINEAR_RGB_SCCData *)ccc->pPerScrnInfo->screenData;

    while (nColors--) {

	/* Make sure format is XcmsRGBiFormat */
	if (pXcmsColors_in_out->format != XcmsRGBiFormat) {
	    return(XcmsFailure);
	}

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.red;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pRedTbl->pBase,
		(unsigned)pScreenData->pRedTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsIntensityCmp, _XcmsIntensityInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGB.red = answerIRec.value;

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.green;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pGreenTbl->pBase,
		(unsigned)pScreenData->pGreenTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsIntensityCmp, _XcmsIntensityInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGB.green = answerIRec.value;

	keyIRec.intensity = pXcmsColors_in_out->spec.RGBi.blue;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pBlueTbl->pBase,
		(unsigned)pScreenData->pBlueTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsIntensityCmp, _XcmsIntensityInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGB.blue = answerIRec.value;

	bcopy((char *)&tmpRGB, (char *)&pXcmsColors_in_out->spec, sizeof(XcmsRGB));
	(pXcmsColors_in_out++)->format = XcmsRGBFormat;
    }
    return(XcmsSuccess);
}


/*
 *	NAME
 *		XcmsRGBToRGBi
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
Status 
XcmsRGBToRGBi(ccc, pXcmsColors_in_out, nColors, pCompressed)
    XcmsCCC ccc;
    XcmsColor *pXcmsColors_in_out;/* pointer to XcmsColors to convert 	*/
    unsigned int nColors;	/* Number of colors			*/
    char *pCompressed;		/* pointer to a bit array		*/
/*
 *	DESCRIPTION
 *		Converts color specifications in an array of XcmsColor
 *		structures from RGB format to RGBi format.
 *
 *	RETURNS
 *		XcmsFailure if failed,
 *		XcmsSuccess if succeeded.
 */
{
    LINEAR_RGB_SCCData *pScreenData;
    XcmsRGBi tmpRGBi;
    IntensityRec keyIRec, answerIRec;

    /*
     * pCompressed ignored in this function.
     */

    if (ccc == NULL) {
	return(XcmsFailure);
    }

    pScreenData = (LINEAR_RGB_SCCData *)ccc->pPerScrnInfo->screenData;

    while (nColors--) {

	/* Make sure format is XcmsRGBFormat */
	if (pXcmsColors_in_out->format != XcmsRGBFormat) {
	    return(XcmsFailure);
	}

	keyIRec.value = pXcmsColors_in_out->spec.RGB.red;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pRedTbl->pBase,
		(unsigned)pScreenData->pRedTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsValueCmp, _XcmsValueInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGBi.red = answerIRec.intensity;

	keyIRec.value = pXcmsColors_in_out->spec.RGB.green;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pGreenTbl->pBase,
		(unsigned)pScreenData->pGreenTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsValueCmp, _XcmsValueInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGBi.green = answerIRec.intensity;

	keyIRec.value = pXcmsColors_in_out->spec.RGB.blue;
	if (!_XcmsTableSearch((char *)&keyIRec, ccc->visual->bits_per_rgb,
		(char *)pScreenData->pBlueTbl->pBase,
		(unsigned)pScreenData->pBlueTbl->nEntries,
		(unsigned)sizeof(IntensityRec),
		_XcmsValueCmp, _XcmsValueInterpolation, (char *)&answerIRec)) {
	    return(XcmsFailure);
	}
	tmpRGBi.blue = answerIRec.intensity;

	bcopy((char *)&tmpRGBi, (char *)&pXcmsColors_in_out->spec, sizeof(XcmsRGBi));
	(pXcmsColors_in_out++)->format = XcmsRGBiFormat;
    }
    return(XcmsSuccess);
}

/*
 *	NAME
 *		_XcmsInitScrnDefaultInfo
 *
 *	SYNOPSIS
 */
/* ARGSUSED */
int
_XcmsLRGB_InitScrnDefault(dpy, screenNumber, pPerScrnInfo)
    Display *dpy;
    int screenNumber;
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
    pPerScrnInfo->screenData = (XPointer)&Default_RGB_SCCData;
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
	pPerScrnInfo->screenData = (XPointer)NULL;
	pPerScrnInfo->state = XcmsInitNone;
	return(0);
    }
    pPerScrnInfo->screenWhitePt.spec.CIEXYZ.Y = 1.0;
    pPerScrnInfo->screenWhitePt.format = XcmsCIEXYZFormat;
    pPerScrnInfo->screenWhitePt.pixel = 0;
    pPerScrnInfo->functionSet = (XPointer)&XcmsLinearRGBFunctionSet;
    pPerScrnInfo->state = XcmsInitDefault;
    return(1);
}
