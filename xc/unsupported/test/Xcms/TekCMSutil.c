/*
 * (c) Copyright 1990, Tektronix Inc.
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
 *	NAME
 *		TekCMSutil.c
 *
 *	DESCRIPTION
 *		Utility routines for the TekCMS testing interface.
 */

#ifndef LINT
static char *copy_notice = "(c) Copyright 1990 Tektronix, Inc., All Rights Reserved.";
#  ifdef RCS_ID
static char *rcsid=  "$Header: TekCMSutil.c,v 1.10 91/01/31 13:25:31 alt Exp $";
#  endif /* RCS_ID */
#endif LINT

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
#include <stdio.h>

/*
 *      INTERNAL INCLUDES
 *              Include files that need NOT be exported to any package or
 *              program using this package.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "TekCMS.h"
#include "LibTest.h"

#ifdef AUTOHEADER
#include "TekCMSglob.ah"
#else
#include "TekCMSglob.h"
#endif

/*
 *	DEFINES
 *		Declarations that are local to this module.
 *		(ignored by 'autohdr').
 */

/*
 *	EXTERNALS
 *		Declarations that are needed by calling modules.
 *		When using 'autohdr', these declaration will be placed
 *		in the resulting header file.
 */


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		fPrintXcmsColorSpec - Print a XcmsColorSpec
 *
 *	SYNOPSIS
 */
void
fPrintXcmsColorSpec(fp, pColor)
    FILE *fp;
    XcmsColor *pColor;
/*
 *	DESCRIPTION
 *		Prints one XcmsColor to the specified file.
 *
 *	RETURNS
 *		void
 *
 */
{
    switch (pColor->format) {
      case XCMS_RGBi_FORMAT:
	fprintf(fp, "\t    format = XCMS_RGBi_FORMAT\n");
	fprintf(fp, "\t    spec.RGBi.red   = %f\n", pColor->spec.RGBi.red);
	fprintf(fp, "\t    spec.RGBi.green = %f\n", pColor->spec.RGBi.green);
	fprintf(fp, "\t    spec.RGBi.blue  = %f\n", pColor->spec.RGBi.blue);
	break;
      case XCMS_RGB_FORMAT:
	fprintf(fp, "\t    format = XCMS_RGB_FORMAT\n");
	fprintf(fp, "\t    spec.RGB.red   = 0x%x\n", pColor->spec.RGB.red);
	fprintf(fp, "\t    spec.RGB.green = 0x%x\n", pColor->spec.RGB.green);
	fprintf(fp, "\t    spec.RGB.blue  = 0x%x\n", pColor->spec.RGB.blue);
	break;
      case XCMS_UNDEFINED_FORMAT:
	fprintf(fp, "\t    format = XCMS_UNDEFINED_FORMAT\n");
	fprintf(fp, "\t    spec.UND.U = %f\n", pColor->spec.CIEXYZ.X);
	fprintf(fp, "\t    spec.UND.N = %f\n", pColor->spec.CIEXYZ.Y);
	fprintf(fp, "\t    spec.UND.D = %f\n", pColor->spec.CIEXYZ.Z);
	break;
      case XCMS_CIEXYZ_FORMAT:
	fprintf(fp, "\t    format = XCMS_CIEXYZ_FORMAT\n");
	fprintf(fp, "\t    spec.CIEXYZ.X = %f\n", pColor->spec.CIEXYZ.X);
	fprintf(fp, "\t    spec.CIEXYZ.Y = %f\n", pColor->spec.CIEXYZ.Y);
	fprintf(fp, "\t    spec.CIEXYZ.Z = %f\n", pColor->spec.CIEXYZ.Z);
	break;
      case XCMS_CIExyY_FORMAT:
	fprintf(fp, "\t    format = XCMS_CIExyY_FORMAT\n");
	fprintf(fp, "\t    spec.CIExyY.x = %f\n", pColor->spec.CIExyY.x);
	fprintf(fp, "\t    spec.CIExyY.y = %f\n", pColor->spec.CIExyY.y);
	fprintf(fp, "\t    spec.CIExyY.Y = %f\n", pColor->spec.CIExyY.Y);
	break;
      case XCMS_CIEuvY_FORMAT:
	fprintf(fp, "\t    format = XCMS_CIEuvY_FORMAT\n");
	fprintf(fp, "\t    spec.CIEuvY.u = %f\n", pColor->spec.CIEuvY.u);
	fprintf(fp, "\t    spec.CIEuvY.v = %f\n", pColor->spec.CIEuvY.v);
	fprintf(fp, "\t    spec.CIEuvY.Y = %f\n", pColor->spec.CIEuvY.Y);
	break;
      case XCMS_CIELab_FORMAT:
	fprintf(fp, "\t    format = XCMS_CIELab_FORMAT\n");
	fprintf(fp, "\t    spec.CIELab.L = %f\n", pColor->spec.CIELab.L);
	fprintf(fp, "\t    spec.CIELab.a = %f\n", pColor->spec.CIELab.a);
	fprintf(fp, "\t    spec.CIELab.b = %f\n", pColor->spec.CIELab.b);
	break;
    case XCMS_CIELuv_FORMAT:
	fprintf(fp, "\t    format = XCMS_CIELuv_FORMAT\n");
	fprintf(fp, "\t    spec.CIELuv.L = %f\n", pColor->spec.CIELuv.L);
	fprintf(fp, "\t    spec.CIELuv.u = %f\n", pColor->spec.CIELuv.u);
	fprintf(fp, "\t    spec.CIELuv.v = %f\n", pColor->spec.CIELuv.v);
	break;
      case XCMS_TekHVC_FORMAT:
	fprintf(fp, "\t    format = XCMS_TekHVC_FORMAT\n");
	fprintf(fp, "\t    spec.TekHVC.H = %f\n", pColor->spec.TekHVC.H);
	fprintf(fp, "\t    spec.TekHVC.V = %f\n", pColor->spec.TekHVC.V);
	fprintf(fp, "\t    spec.TekHVC.C = %f\n", pColor->spec.TekHVC.C);
	break;
      default:
	fprintf(fp, "\t    format = Invalid format\n");
    }
}


/*
 *	NAME
 *		fPrintXcmsColorSpecs -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
fPrintXcmsColorSpecs(fp,pColors, nColors)
    FILE *fp;
    XcmsColor *pColors;
    int nColors;
/*
 *	DESCRIPTION
 *		Prints the spec component of the XcmsColor structures
 *		to the specified file.
 *
 *	RETURNS
 *		void
 *
 */
{
    register int i;
    for (i = 0; i < nColors; i++, pColors++) {
	if (nColors > 1) {
	    fprintf(fp, "\tcolor[%3d]:\n", i);
	}
	fPrintXcmsColorSpec(fp, pColors);
    }
}


/*
 *	NAME
 *		fPrintXcmsColors -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
fPrintXcmsColors(fp, pColors, nColors)
    FILE *fp;
    XcmsColor *pColors;
    int nColors;
/*
 *	DESCRIPTION
 *		Prints a number of  XcmsColor's to the specified file.
 *
 *	RETURNS
 *		void
 *
 */
{
    register int i;
    for (i = 0; i < nColors; i++, pColors++) {
	if (nColors > 1) {
	    fprintf(fp, "\tcolor[%3d]:\n", i);
	}
	fprintf(fp, "\t    pixel = %lu\n", pColors->pixel);
	fPrintXcmsColorSpec(fp, pColors);
    }
}



/*
 *	NAME
 *		fPrintXcmsColorsWithComp -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
fPrintXcmsColorsWithComp(fp, pColors, nColors, pCompressed)
    FILE *fp;
    XcmsColor *pColors;
    int nColors;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Prints an number of XcmsColor's to the specified file, and
 *		also the compression flags.
 *
 *	RETURNS
 *		void
 *
 */
{
    register int i;
    for (i = 0; i < nColors; i++, pColors++) {
	if (nColors > 1) {
	    fprintf(fp, "\tcolor[%3d]:\n", i);
	}
	fprintf(fp, "\t    pixel = %lu\n", pColors->pixel);
	fPrintXcmsColorSpec(fp, pColors);
	fprintf(fp, "\t    Compressed = %s\n",
	    *pCompressed == True ? "YES" : "no");
    }
}


/*
 *	NAME
 *		PrintXcmsColors -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
PrintXcmsColors(pColors, nColors)
    XcmsColor *pColors;
    int nColors;
/*
 *	DESCRIPTION
 *		Prints a number of  XcmsColor's to the standard out.
 *		
 *
 *	RETURNS
 *		void
 *
 */
{
    fPrintXcmsColors(stdout, pColors, nColors);
}


/*
 *	NAME
 *		PrintXcmsColorsWithComp -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
PrintXcmsColorsWithComp(pColors, nColors, pCompressed)
    XcmsColor *pColors;
    int nColors;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Prints an number of XcmsColor's to standard out, and
 *		also the compression flags.
 *
 *	RETURNS
 *		void
 *
 */
{
    fPrintXcmsColorsWithComp(stdout, pColors, nColors, pCompressed);
}


/*
 *	NAME
 *		PrintXcmsColor -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
PrintXcmsColor(pColors)
    XcmsColor *pColors;
/*
 *	DESCRIPTION
 *		Prints one XcmsColor to standard out.
 *		
 *
 *	RETURNS
 *		void
 *
 */
{
    fPrintXcmsColors(stdout, pColors, 1);
}


/*
 *	NAME
 *		PrintXcmsColorSpecs -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
PrintXcmsColorSpecs(pColors, nColors)
    XcmsColor *pColors;
    int nColors;
/*
 *	DESCRIPTION
 *		Prints the spec component of the XcmsColor structures
 *		to standard out.
 *		
 *
 *	RETURNS
 *		void
 *
 */
{
    fPrintXcmsColorSpecs(stdout, pColors, nColors);
}


/*
 *	NAME
 *		PrintXcmsColorSpec -- Print a XcmsColor
 *
 *	SYNOPSIS
 */
void
PrintXcmsColorSpec(pColors)
    XcmsColor *pColors;
/*
 *	DESCRIPTION
 *		Prints the spec component of one XcmsColor structure
 *		to standard out.
 *		
 *
 *	RETURNS
 *		void
 *
 */
{
    fPrintXcmsColorSpecs(stdout, pColors, 1);
}
