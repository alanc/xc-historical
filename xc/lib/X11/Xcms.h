/* $XConsortium: Xcms.h,v 1.11 91/01/27 00:31:39 alt Exp  */

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
 *		TekCMS.h
 *
 *	DESCRIPTION
 *		Public include file for TekCMS Color Extension to X
 *              This file used to be TekCMS.h and TekCMSext.h - dms.
 */
#ifndef _XCMS_H_
#define _XCMS_H_

    /*
     * TekCMS Status Values
     */
#define XCMS_FAILURE			0
#define XCMS_SUCCESS			1
#define XCMS_SUCCESS_WITH_COMPRESSION	2

    /*
     * Color Space Format ID's
     *    Color Space ID's are of XcmsSpecFmt type, which is an
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
#define XCMS_UNDEFINED_FORMAT	(XcmsSpecFmt)0x00000000
#define XCMS_CIEXYZ_FORMAT	(XcmsSpecFmt)0x00000001
#define XCMS_CIEuvY_FORMAT	(XcmsSpecFmt)0x00000002
#define XCMS_CIExyY_FORMAT	(XcmsSpecFmt)0x00000003
#define XCMS_CIELab_FORMAT	(XcmsSpecFmt)0x00000004
#define XCMS_CIELuv_FORMAT	(XcmsSpecFmt)0x00000005
#define XCMS_TekHVC_FORMAT	(XcmsSpecFmt)0x00000006
#define XCMS_RGB_FORMAT		(XcmsSpecFmt)0x80000000
#define XCMS_RGBi_FORMAT	(XcmsSpecFmt)0x80000001

    /*
     * State of XcmsPerScrnInfo
     */
#define XCMS_INIT_NONE		0x00	/* no initialization attempted */
#define XCMS_INIT_SUCCESS		0x01	/* initialization successful */
#define XCMS_INIT_DEFAULT		0xff	/* initialization failed */

#define XcmsDisplay(ccc)		((ccc)->dpy)
#define XcmsScreenNumber(ccc)		((ccc)->screen_number)
#define XcmsVisual(ccc)			((ccc)->visual)
#define XcmsClientWhitePt(ccc)		((ccc)->clientWhitePt)
#define XcmsScreenWhitePt(ccc)		((ccc)->pPerScrnInfo->screenWhitePt)
#define XcmsGamutCompFunc(ccc)		((ccc)->gamutCompFunc)
#define XcmsGamutCompClientData(ccc)	((ccc)->gamutCompClientData)
#define XcmsWhitePtAdjFunc(ccc)		((ccc)->WhitePtAdjFunc)
#define XcmsWhitePtAdjClientData(ccc)	((ccc)->WhitePtAdjClientData)
#define XcmsSetClientWhitePt(ccc,wp)	((ccc)->clientWhitePt = wp)
#define XcmsSetGamutCompFunc(ccc,f)	((ccc)->gamutCompFunc = f)
#define XcmsSetGamutCompClientData(ccc,d)	((ccc)->gamutCompClientData = d)
#define XcmsSetWhitePtAdjFunc(ccc,f)	((ccc)->WhitePtAdjFunc = f)
#define XcmsSetWhitePtAdjClientData(ccc,d)	((ccc)->WhitePtAdjClientData = d)


typedef unsigned int XcmsSpecFmt;	/* Color Space Format ID */

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
    XcmsFloat u;		/* 0.0 - 1.0 */
    XcmsFloat v;		/* 0.0 - 1.0 */
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
    XcmsFloat L;		/* 0.0 - 1.0 */
    XcmsFloat a;
    XcmsFloat b;
} XcmsCIELab;

    /*
     * CIE L*u*v*
     */
typedef struct {
    XcmsFloat L;		/* 0.0 - 1.0 */
    XcmsFloat u;
    XcmsFloat v;
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
     * TekCMS Color Structure
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
    XcmsSpecFmt	format;		/* the specification format	*/
} XcmsColor;

    /*
     * TekCMS Per Screen related data
     */

typedef struct _XcmsPerScrnInfo {
    XcmsColor	screenWhitePt;	/* Screen White point */
    caddr_t	pSCCFuncSet;	/* pointer to Screen Color Characterization */
				/*      Function Set structure		*/
    caddr_t	pSCCData;	/* pointer to corresponding Screen Color*/
				/*	Characterization Data		*/
    unsigned char state;	/* XCMS_INIT_NONE, XCMS_INIT_SUCCESS, XCMS_INIT_DEFAULT */
    char	pad[3];
} XcmsPerScrnInfo;

    /*
     * TekCMS Color Conversion Context
     */
typedef struct _XcmsCCC {
    Display	*dpy;			/* X Display */
    int		screen_number;		/* X Screen Number */
    Visual	*visual;		/* X Visual */
    XcmsColor	clientWhitePt;		/* Client White Point */
    XcmsFuncPtr	gamutCompFunc;		/* Gamut Compression Function */
    caddr_t	gamutCompClientData;	/* Gamut Comp Func Client Data */
    XcmsFuncPtr	whitePtAdjFunc;		/* White Point Adjustment Function */
    caddr_t	whitePtAdjClientData;	/* White Pt Adj Func Client Data */
    XcmsPerScrnInfo *pPerScrnInfo;	/* pointer to per screen information */
					/*  associated with the above display */
					/*  screen_number */
} XcmsCCC;

    /*
     * Function List Pointer -- pointer to an array of function pointers.
     *    The end of list is indicated by a NULL pointer.
     */
typedef XcmsFuncPtr *XcmsFuncListPtr;

    /*
     * Color Space -- per Color Space related data (Device-Independent
     *    or Device-Dependent)
     */
typedef struct _XcmsColorSpace {
    char *prefix;		/* Prefix of string format.		*/
    XcmsSpecFmt id;		/* Format ID number.			*/
    XcmsFuncPtr parseString;	/* String format parsing function	*/
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
    XcmsSpecFmt		/* result_format */
#endif
);

extern Status XcmsAllocNamedColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    _Xconst char*	/* color_string */,
    XcmsColor*		/* color_scrn_return */,
    XcmsColor*		/* color_exact_return */,
    XcmsSpecFmt		/* result_format */
#endif
);

extern XcmsCCC *XcmsCCCofColormap (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */
#endif
);

extern Status XcmsConvertColors (
#if NeedFunctionPrototypes
    XcmsCCC*		/* pCCC */,
    XcmsColor []	/* colorArry_in_out */,
    unsigned int	/* nColors */,
    XcmsSpecFmt		/* targetFormat */,
    Bool*		/* compArry_return */
#endif
);

extern XcmsCCC *XcmsCreateCCC (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen_number */,
    Visual*		/* visual */,
    XcmsColor*		/* clientWhitePt */,
    XcmsFuncPtr	        /* gamutCompFunc */,
    caddr_t		/* gamutCompClientData */,
    XcmsFuncPtr 	/* whitePtAdjFunc */,
    caddr_t		/* whitePtAdjClientData */
#endif
);

extern XcmsCCC *XcmsDefaultCCC (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int			/* screen_number */
#endif
);

extern void XcmsFreeCCC (
#if NeedFunctionPrototypes
    XcmsCCC*		/* pCCC */
#endif
);

extern Status XcmsLookupColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    _Xconst char*	/* color_string */,
    XcmsColor*		/* pColor_exact_in_out */,
    XcmsColor*		/* pColor_scrn_in_out */,
    XcmsSpecFmt		/* result_format */
#endif
);

extern Status XcmsQueryColor (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor*		/* pColor_in_out */,
    XcmsSpecFmt		/* result_format */
#endif
);

extern Status XcmsQueryColors (
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Colormap		/* colormap */,
    XcmsColor []	/* colorArry_in_out */,
    unsigned int	/* nColors */,
    XcmsSpecFmt		/* result_format */
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
    XcmsColor []	/* colorArry_in */,
    unsigned int	/* nColors */,
    Bool*		/* compArry_return */
#endif
);

extern XcmsSpecFmt XcmsIDofPrefix (
#if NeedFunctionPrototypes
    char*		/* prefix */
#endif
);

extern char *XcmsPrefixOfID (
#if NeedFunctionPrototypes
    XcmsSpecFmt		/* id */
#endif
);

extern Status XcmsAddDIColorSpace (
#if NeedFunctionPrototypes
    XcmsColorSpace*	/* pColorSpace */
#endif
);

extern Status XcmsAddSCCFuncSet (
#if NeedFunctionPrototypes
    XcmsSCCFuncSet*	/* pSCCFuncSet */
#endif
);

_XFUNCPROTOEND

#endif /* _XCMS_H_ */
