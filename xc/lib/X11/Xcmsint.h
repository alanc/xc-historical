/* $XConsortium: Xcmsint.h,v 1.11 91/06/07 17:28:45 rws Exp $ */

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
 *	DESCRIPTION
 *		Private include file for Color Management System.
 *		(i.e., for API internal use only)
 *
 */
#ifndef _XCMSINT_H_
#define _XCMSINT_H_

#include <X11/Xcms.h>

/*
 *	DEFINES
 */

	/*
	 * Private Status Value
	 */
#define	_XCMS_NEWNAME	-1

	/*
	 * Color Space ID's are of XcmsColorFormat type, which is an
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
#define       XCMS_DD_ID(id)          ((id) & (XcmsColorFormat)0x80000000)
#define       XCMS_DI_ID(id)          (!((id) & (XcmsColorFormat)0x80000000))
#define       XCMS_REG_ID(id)         ((id) & (XcmsColorFormat)0x40000000)
#define       XCMS_UNREG_ID(id)       (!((id) & (XcmsColorFormat)0x40000000))
#define       XCMS_FIRST_REG_DI_ID    (XcmsColorFormat)0x00000001
#define       XCMS_FIRST_UNREG_DI_ID  (XcmsColorFormat)0x40000000
#define       XCMS_FIRST_REG_DD_ID    (XcmsColorFormat)0x80000000
#define       XCMS_FIRST_UNREG_DD_ID  (XcmsColorFormat)0xc0000000

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
    struct _XcmsCCC *ccc;
    struct _XcmsCmapRec *pNext;
} XcmsCmapRec;


    /*
     * Structure for storing "registered" color space prefix/ID
     */
typedef struct _XcmsRegColorSpaceEntry {
    char *prefix;	/* Color Space prefix (e.g., "CIEXYZ:") */
    XcmsColorFormat id;	/* Color Space ID (e.g., XcmsCIEXYZFormat) */
} XcmsRegColorSpaceEntry;


    /*
     * Xcms Per Display (i.e. connection) related data
     */
typedef struct _XcmsPerDpyInfo {

    XcmsCCC paDefaultCCC; /* based on default visual of screen */
	    /*
	     * Pointer to an array of XcmsCCC structures, one for
	     * each screen.
	     */
    XcmsCmapRec *pClientCmaps;	/* Pointer to linked list of XcmsCmapRec's */

} XcmsPerDpyInfo, *XcmsPerDpyInfoPtr;

/*
 *	DEFINES
 */

#define XDCCC_NUMBER	0x8000000L	/* 2**27 per XDCCC */

typedef struct _IntensityRec {
    unsigned short value;
    XcmsFloat intensity;
} IntensityRec;

typedef struct _IntensityTbl {
    IntensityRec *pBase;
    unsigned int nEntries;
} IntensityTbl;

/*
 *	DEFINES
 */

#ifdef GRAY
#define XDCCC_SCREENWHITEPT_ATOM_NAME	"XDCCC_GRAY_SCREENWHITEPOINT"
#define XDCCC_GRAY_CORRECT_ATOM_NAME	"XDCCC_GRAY_CORRECTION"
#endif /* GRAY */

#ifndef _ConversionValues
typedef struct _ConversionValues {
    IntensityTbl IntensityTbl;
} ConversionValues;
#endif

#ifdef GRAY
typedef struct {
    IntensityTbl *IntensityTbl;
} GRAY_SCCData;
#endif /* GRAY */

/*
 *	DEFINES
 */

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
 *		XCMS source.  Instead of using math library routines
 *		directly, XCMS uses macros so that based on the
 *		definitions here, vendors and sites can specify exactly
 *		what routine will be called (those from libm.a or their
 *		custom routines).  If not defined to math library routines
 *		(e.g., sqrt in libm.a), then the client is not forced to
 *		be linked with -lm.
 */

#define XCMS_ATAN(x)		_XcmsArcTangent(x)
#define XCMS_COS(x)		_XcmsCosine(x)
#define XCMS_CUBEROOT(x)	_XcmsCubeRoot(x)
#define XCMS_FABS(x)		((x) < 0.0 ? -(x) : (x))
#define XCMS_MOD(x)		_XcmsModulo(x)
#define XCMS_MODF(x)		_XcmsModuloF(x)
#define XCMS_POLY(x)		_XcmsPolynomial(x)
#define XCMS_SIN(x)		_XcmsSine(x)
#define XCMS_SQRT(x)		_XcmsSquareRoot(x)

#if __STDC__
double _XcmsArcTangent(double a);
double _XcmsCosine(double a);
double _XcmsCubeRoot(double a);
double _XcmsModulo(double a, double b);
double _XcmsModuloF(double a, double b);
double _XcmsPolynomial(int a, double b, double c);
double _XcmsSine(double a);
double _XcmsSquareRoot(double a);
#else
double _XcmsArcTangent();
double _XcmsCosine();
double _XcmsCubeRoot();
double _XcmsModulo();
double _XcmsModuloF();
double _XcmsPolynomial();
double _XcmsSine();
double _XcmsSquareRoot();
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

#endif /* _XCMSINT_H_ */
