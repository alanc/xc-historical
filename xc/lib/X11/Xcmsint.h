/* $XConsortium: Xcmsint.h,v 1.1 91/02/11 11:50:46 dave Exp $ */

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
 *		TekCMSP.h
 *
 *	DESCRIPTION
 *		Private include file for TekCMS color extension to X.
 *		(i.e., for API internal use only)
 *              This file used to be: TekCMSP.h
 *                                    XcmsSD.h
 *                                    XcmsGRAYSD.h
 *                                    XcmsLRGBSD.h
 *                                    XcmsMath.h
 *                                    XcmsStdIll.h
 *
 */
#ifndef Xcmsint_H
#define Xcmsint_H

#include <sys/types.h>
#include "Xcms.h"

/*
 *	DEFINES
 */
#ifndef NULL
#define NULL 0
#endif
	/*
	 * Private Status Value
	 */
#define	_XCMS_NEWNAME	-1
	/*
	 * Device Color Name Database related defines
	 */
#define	XCMS_DBFILEPATH_ENV_VAR		"XCMS_DBFILEPATH"
#define	XCMS_COLORNAMEDB_TYPE		"colordb"
#define	XCMS_COLORNAMEDB_FILENAME	"Xcms"
#define	XCMS_COLORNAMEDB_SUFFIX		".txt"

	/*
	 * Color Space ID's are of XcmsSpecFmt type, which is an
	 *	unsigned short (16 bits).  
	 *
	 *	bit 15 (most significant bit):
	 *	    0 == Device-Independent
	 *	    1 == Device-Dependent
	 *
	 *	bit 14:
         *          0 == Registered with X Consortium
         *          1 == Unregistered
         */
#define       XCMS_DD_ID(id)          ((id) & (XcmsSpecFmt)0x80000000)
#define       XCMS_DI_ID(id)          (!((id) & (XcmsSpecFmt)0x80000000))
#define       XCMS_REG_ID(id)         ((id) & (XcmsSpecFmt)0x40000000)
#define       XCMS_UNREG_ID(id)       (!((id) & (XcmsSpecFmt)0x40000000))
#define       XCMS_FIRST_REG_DI_ID    (XcmsSpecFmt)0x00000001
#define       XCMS_FIRST_UNREG_DI_ID  (XcmsSpecFmt)0x40000000
#define       XCMS_FIRST_REG_DD_ID    (XcmsSpecFmt)0x80000000
#define       XCMS_FIRST_UNREG_DD_ID  (XcmsSpecFmt)0xc0000000

/*
 *	TYPEDEFS
 */

    /*
     * Structure for caching Colormap info.
     *    This is provided for the Xlib modifications to:
     *		XAllocNamedColor()
     *		XLookupColor()
     *		XParseColor()
     *		XStoreNamedColor()
     */
typedef struct _XcmsCmapRec {
    Colormap cmapID;
    Display *dpy;
    Window windowID;
    Visual *visual;
    struct _XcmsCCC *pCCC;
    struct _XcmsCmapRec *pNext;
} XcmsCmapRec;


    /*
     * Structure for storing "registered" color space prefix/ID
     */
typedef struct _XcmsRegColorSpaceEntry {
    char *prefix;	/* Color Space prefix (e.g., "CIEXYZ:") */
    XcmsSpecFmt id;	/* Color Space ID (e.g., XCMS_CIEXYZ_FORMAT) */
} XcmsRegColorSpaceEntry;


    /*
     * Xcms Per Display (i.e. connection) related data
     */
typedef struct _XcmsPerDpyInfo {

    XcmsCCC *paDefaultCCC; /* based on default visual of screen */
	    /*
	     * Pointer to an array of XcmsCCC structures, one for
	     * each screen.
	     */
    XcmsCmapRec *pClientCmaps;	/* Pointer to linked list of XcmsCmapRec's */

} XcmsPerDpyInfo, *XcmsPerDpyInfoPtr;

/*
 *	DESCRIPTION
 *		Include file for TekCMS Color Extension when using the
 *		X Device Color Characterization Convention (XDCCC).
 *
 */

/*
 *	DEFINES
 */

#define XDCCC_NUMBER	0x8000000	/* 2**27 per XDCCC */

typedef struct _IntensityRec {
    unsigned short value;
    XcmsFloat intensity;
} IntensityRec;

typedef struct _IntensityTbl {
    IntensityRec *pBase;
    unsigned int nEntries;
} IntensityTbl;

/*
 *	DESCRIPTION
 *		Include file for TekCMS Color Extension when using the
 *		X Device Color Characterization Convention (XDCCC).
 */

/*
 *	DEFINES
 */

#define XDCCC_SCREENWHITEPT_ATOM_NAME	"XDCCC_GRAY_SCREENWHITEPOINT"
#define XDCCC_GRAY_CORRECT_ATOM_NAME	"XDCCC_GRAY_CORRECTION"

#ifndef _ConversionValues
typedef struct _ConversionValues {
    IntensityTbl IntensityTbl;
} ConversionValues;
#endif

typedef struct {
    IntensityTbl *IntensityTbl;
} GRAY_SCCData;

/*
 *	DESCRIPTION
 *		Include file for TekCMS Color Extension when using the
 *		X Device Color Characterization Convention (XDCCC).
 *
 */

/*
 *	DEFINES
 */

#define XDCCC_NUMBER	0x8000000	/* 2**27 per XDCCC */

#define XDCCC_MATRIX_ATOM_NAME	"XDCCC_LINEAR_RGB_MATRICES"
#define XDCCC_CORRECT_ATOM_NAME "XDCCC_LINEAR_RGB_CORRECTION"

typedef struct {
    XcmsFloat XYZtoRGBmatrix[3][3];
    XcmsFloat RGBtoXYZmatrix[3][3];
    IntensityTbl *pRedTbl;
    IntensityTbl *pGreenTbl;
    IntensityTbl *pBlueTbl;
} LINEAR_RGB_SCCData;

/*
 *	DESCRIPTION
 *		Include file for defining the math macros used in the
 *		TekCMS source.  Instead of using math library routines
 *		directly, TekCMS uses macros so that based on the
 *		definitions here, vendors and sites can specify exactly
 *		what routine will be called (those from libm.a or their
 *		custom routines).  If not defined to math library routines
 *		(e.g., sqrt in libm.a), then the client is not forced to
 *		be linked with -lm.
 */

#define XCMS_CUBEROOT(x)	XcmsCubeRoot(x)

#ifdef __GNUC__
const double XcmsCubeRoot(double a);
const double XcmsSquareRoot(double a);
#else
#ifdef __STDC__
double XcmsCubeRoot(double a);
double XcmsSquareRoot(double a);
#else
double XcmsCubeRoot();
double XcmsSquareRoot();
#endif
#endif

/*
 *
 *	DESCRIPTION
 *		This file contains a series of standard white point values.
 */
#define CIE_A_u		0.2560
#define CIE_A_v		0.5243
#define CIE_A_Y		1.0000

#define CIE_B_u		0.2137
#define CIE_B_v		0.4852
#define CIE_B_Y		1.0000

#define CIE_C_u		0.2009
#define CIE_C_v		0.4609
#define CIE_C_Y		1.0000

#define CIE_D55_u	0.2044
#define CIE_D55_v	0.4808
#define CIE_D55_Y	1.0000

#define CIE_D65_u	0.1978
#define CIE_D65_v	0.4684
#define CIE_D65_Y	1.0000

#define CIE_D75_u	0.1935
#define CIE_D75_v	0.4586
#define CIE_D75_Y	1.0000

#define ASTM_D50_u	0.2092
#define ASTM_D50_v	0.4881
#define ASTM_D50_Y	1.0000

#define WP_9300K_u	0.1884
#define WP_9300K_v	0.4463
#define WP_9300K_Y	1.0000

#endif /* Xcmsint_H */
