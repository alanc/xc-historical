/* $XConsortium: Xcms.h,v 1.11 91/01/27 00:31:39 alt Exp  */

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
 *		Public include file for X Color Management System
 */
#ifndef _XCMS_H_
#define _XCMS_H_

    /*
     * XCMS Status Values
     */
#define XcmsFailure			0
#define XcmsSuccess			1
#define XcmsSuccessWithCompression	2

    /*
     * Color Space Format ID's
     *    Color Space ID's are of XcmsColorFormat type, which is an
     *    unsigned short (16 bits).  
     *
     *    bit 15 (most significant bit):
     *	    0 == Device-Independent
     *	    1 == Device-Dependent
     *
     *    bit 14:
     *	    0 == Registered with X Consortium
     *	    1 == Unregistered
     */
#define XcmsUndefinedFormat	(XcmsColorFormat)0x00000000
#define XcmsCIEXYZFormat	(XcmsColorFormat)0x00000001
#define XcmsCIEuvYFormat	(XcmsColorFormat)0x00000002
#define XcmsCIExyYFormat	(XcmsColorFormat)0x00000003
#define XcmsCIELabFormat	(XcmsColorFormat)0x00000004
#define XcmsCIELuvFormat	(XcmsColorFormat)0x00000005
#define XcmsTekHVCFormat	(XcmsColorFormat)0x00000006
#define XcmsRGBFormat		(XcmsColorFormat)0x80000000
#define XcmsRGBiFormat		(XcmsColorFormat)0x80000001

    /*
     * State of XcmsPerScrnInfo
     */
#define XcmsInitNone		0x00	/* no initialization attempted */
#define XcmsInitSuccess		0x01	/* initialization successful */
#define XcmsInitDefault		0xff	/* initialization failed */

#define DisplayOfCCC(ccc)		((ccc)->dpy)
#define ScreenNumberOfCCC(ccc)		((ccc)->screenNumber)
#define VisualOfCCC(ccc)		((ccc)->visual)
#define ClientWhitePointOfCCC(ccc)	((ccc)->clientWhitePt)
#define ScreenWhitePointOfCCC(ccc)	((ccc)->pPerScrnInfo->screenWhitePt)

typedef unsigned int XcmsColorFormat;	/* Color Space Format ID */

typedef double XcmsFloat;

typedef Status (*XcmsFuncPtr)(); /* Pointer to a function returning Status */

    /*
     * Device RGB
     */
typedef struct {
    unsigned short red;		/* scaled from 0x0000 to 0xffff */
    unsigned short green;	/* scaled from 0x0000 to 0xffff */
    unsigned short blue;	/* scaled from 0x0000 to 0xffff */
} XcmsRGB;

    /*
     * RGB Intensity
     */
typedef struct {
    XcmsFloat red;	/* 0.0 - 1.0 */
    XcmsFloat green;	/* 0.0 - 1.0 */
    XcmsFloat blue;	/* 0.0 - 1.0 */
} XcmsRGBi;

    /*
     * CIE XYZ
     */
typedef struct {
    XcmsFloat X;
    XcmsFloat Y;
    XcmsFloat Z;
} XcmsCIEXYZ;

    /*
     * CIE u'v'Y
     */
typedef struct {
    XcmsFloat u_prime;		/* 0.0 - 1.0 */
    XcmsFloat v_prime;		/* 0.0 - 1.0 */
    XcmsFloat Y;		/* 0.0 - 1.0 */
} XcmsCIEuvY;

    /*
     * CIE xyY
     */
typedef struct {
    XcmsFloat x;		/* 0.0 - 1.0 */
    XcmsFloat y;		/* 0.0 - 1.0 */
    XcmsFloat Y;		/* 0.0 - 1.0 */
} XcmsCIExyY;

    /*
     * CIE L*a*b*
     */
typedef struct {
    XcmsFloat L_star;		/* 0.0 - 100.0 */
    XcmsFloat a_star;
    XcmsFloat b_star;
} XcmsCIELab;

    /*
     * CIE L*u*v*
     */
typedef struct {
    XcmsFloat L_star;		/* 0.0 - 100.0 */
    XcmsFloat u_star;
    XcmsFloat v_star;
} XcmsCIELuv;

    /*
     * TekHVC
     */
typedef struct {
    XcmsFloat H;		/* 0.0 - 360.0 */
    XcmsFloat V;		/* 0.0 - 100.0 */
    XcmsFloat C;		/* 0.0 - 100.0 */
} XcmsTekHVC;

    /*
     * PAD
     */
typedef struct {
    XcmsFloat pad0;
    XcmsFloat pad1;
    XcmsFloat pad2;
    XcmsFloat pad3;
} XcmsPad;


    /*
     * XCMS Color Structure
     */
typedef struct {
    union {
	XcmsRGB RGB;
	XcmsRGBi RGBi;
	XcmsCIEXYZ CIEXYZ;
	XcmsCIEuvY CIEuvY;
	XcmsCIExyY CIExyY;
	XcmsCIELab CIELab;
	XcmsCIELuv CIELuv;
	XcmsTekHVC TekHVC;
	XcmsPad Pad;
    } spec;			/* the color specification	*/
    unsigned long pixel;	/* pixel value (as needed)	*/
    XcmsColorFormat	format;		/* the specification format	*/
} XcmsColor;

    /*
     * XCMS Per Screen related data
     */

typedef struct _XcmsPerScrnInfo {
    XcmsColor	screenWhitePt;	/* Screen White point */
    XPointer	functionSet;	/* pointer to Screen Color Characterization */
				/*      Function Set structure		*/
    XPointer	screenData;	/* pointer to corresponding Screen Color*/
				/*	Characterization Data		*/
    unsigned char state;	/* XcmsInitNone, XcmsInitSuccess, XcmsInitDefault */
    char	pad[3];
} XcmsPerScrnInfo;

    /*
     * XCMS Color Conversion Context
     */
typedef struct _XcmsCCC {
    Display	*dpy;			/* X Display */
    int		screenNumber;		/* X screen number */
    Visual	*visual;		/* X Visual */
    XcmsColor	clientWhitePt;		/* Client White Point */
    XcmsFuncPtr	gamutCompProc;		/* Gamut Compression Function */
    XPointer	gamutCompClientData;	/* Gamut Comp Func Client Data */
    XcmsFuncPtr	whitePtAdjProc;		/* White Point Adjustment Function */
    XPointer	whitePtAdjClientData;	/* White Pt Adj Func Client Data */
    XcmsPerScrnInfo *pPerScrnInfo;	/* pointer to per screen information */
					/*  associated with the above display */
					/*  screenNumber */
} XcmsCCCRec, *XcmsCCC;

    /*
     * Callback typedefs
     */
typedef Status (*XcmsCompressionProc)();	/* Gamut Compression Proc */
typedef Status (*XcmsWhiteAdjustProc)();	/* White Point Adjustment Proc */
typedef int (*XcmsParseStringProc)();	/* Color String Parsing Proc */
typedef int (*XcmsConversionProc)();	/* Color Conversion Proc */
typedef Status (*XcmsScreenInitProc)();	/* Screen Initialization Proc */

    /*
     * Function List Pointer -- pointer to an array of function pointers.
     *    The end of list is indicated by a NULL pointer.
     */
typedef XcmsConversionProc *XcmsFuncListPtr;

    /*
     * Color Space -- per Color Space related data (Device-Independent
     *    or Device-Dependent)
     */
typedef struct _XcmsColorSpace {
    char *prefix;		/* Prefix of string format.		*/
    XcmsColorFormat id;		/* Format ID number.			*/
    XcmsParseStringProc parseString;
				/* String format parsing function	*/
    XcmsFuncListPtr to_CIEXYZ;	/* Pointer to an array of function 	*/
				/*   pointers such that when the	*/
				/*   functions are executed in sequence	*/
				/*   will convert a XcmsColor structure	*/
				/*   from this color space to CIEXYZ	*/
				/*   space.				*/
    XcmsFuncListPtr from_CIEXYZ;/* Pointer to an array of function 	*/
				/*   pointers such that when the	*/
				/*   functions are executed in sequence	*/
				/*   will convert a XcmsColor structure	*/
				/*   from CIEXYZ space to this color	*/
				/*   space.				*/
    int inverse_flag;		/* If 1, indicates that for 0 <= i < n	*/
				/*   where n is the number of function	*/
				/*   pointers in the lists to_CIEXYZ	*/
				/*   and from_CIEXYZ; for each function */
				/*   to_CIEXYZ[i] its inverse function	*/
				/*   is from_CIEXYZ[n - i].		*/

} XcmsColorSpace;

    /*
     * Screen Color Characterization Function Set -- per device class
     *    color space conversion functions.
     */
typedef struct _XcmsSCCFuncSet {
    XcmsColorSpace **papDDColorSpaces;
				/* Pointer to an array of pointers to	*/
				/*   Device-DEPENDENT color spaces	*/
				/*   understood by this SCCFuncSet.	*/
    int (*pInitScrnFunc)();	/* Screen initialization function that	*/
				/*   reads Screen Color Characterization*/
				/*   Data off properties on the screen's*/
				/*   root window.			*/
    void (*pFreeSCCData)();	/* Function that frees the SCCData	*/
				/*   structures.			*/
} XcmsSCCFuncSet;

_XFUNCPROTOBEGIN

extern Status XcmsAllocColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* color_in_out */,
    XcmsColorFormat		/* result_format */
#endif
);

extern Status XcmsAllocNamedColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    _Xconst char*	/* color_string */,
    XcmsColor*		/* color_scrn_return */,
    XcmsColor*		/* color_exact_return */,
    XcmsColorFormat		/* result_format */
#endif
);

extern XcmsCCC XcmsCCCOfColormap (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */
#endif
);

extern Status XcmsConvertColors (
#if NeedFunctionPrototypes
    XcmsCCC		/* ccc */,
    XcmsColor*		/* colorArry_in_out */,
    unsigned int	/* nColors */,
    XcmsColorFormat		/* targetFormat */,
    Bool*		/* compArry_return */
#endif
);

extern XcmsCCC XcmsCreateCCC (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screenNumber */,
    Visual*		/* visual */,
    XcmsColor*		/* clientWhitePt */,
    XcmsFuncPtr	        /* gamutCompProc */,
    XPointer		/* gamutCompClientData */,
    XcmsFuncPtr 	/* whitePtAdjProc */,
    XPointer		/* whitePtAdjClientData */
#endif
);

extern XcmsCCC XcmsDefaultCCC (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screenNumber */
#endif
);

extern void XcmsFreeCCC (
#if NeedFunctionPrototypes
    XcmsCCC		/* ccc */
#endif
);

extern Status XcmsLookupColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    _Xconst char*	/* color_string */,
    XcmsColor*		/* pColor_exact_in_out */,
    XcmsColor*		/* pColor_scrn_in_out */,
    XcmsColorFormat		/* result_format */
#endif
);

extern Status XcmsQueryColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* pColor_in_out */,
    XcmsColorFormat		/* result_format */
#endif
);

extern Status XcmsQueryColors (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* colorArry_in_out */,
    unsigned int	/* nColors */,
    XcmsColorFormat		/* result_format */
#endif
);

extern Status XcmsStoreColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* pColor_in */
#endif
);

extern Status XcmsStoreColors (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* colorArry_in */,
    unsigned int	/* nColors */,
    Bool*		/* compArry_return */
#endif
);

extern XcmsColorFormat XcmsFormatOfPrefix (
#if NeedFunctionPrototypes
    char*		/* prefix */
#endif
);

extern char *XcmsPrefixOfFormat (
#if NeedFunctionPrototypes
    XcmsColorFormat		/* id */
#endif
);

extern Status XcmsAddColorSpace (
#if NeedFunctionPrototypes
    XcmsColorSpace*	/* pColorSpace */
#endif
);

extern Status XcmsAddFunctionSett (
#if NeedFunctionPrototypes
    XcmsSCCFuncSet*	/* functionSet */
#endif
);

_XFUNCPROTOEND

#endif /* _XCMS_H_ */
