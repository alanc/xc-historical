/* $XConsortium: XcmsXRGB.c,v 1.1 91/01/30 19:33:57 dave Exp $" */

/*
 * (c) Copyright 1989 1990 1991 Tektronix Inc.
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
 *		XcmsRtoX.c
 *
 *	DESCRIPTION
 *		Convert color specifications in XcmsRGB format in one array of
 *		XcmsColor structures to RGB in an array of XColor structures.
 *
 *
 */

#include "Xcmsint.h"

/*
 *      LOCAL VARIABLES
 */

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



/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		_XcmsRGB_to_XColor - 
 *
 *	SYNOPSIS
 */
void
_XcmsRGB_to_XColor(pColors, pXColors, nColors)
    XcmsColor *pColors;
    XColor *pXColors;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *	    Translates a color specification in XCMS_RGB_FORMAT in a XcmsColor
 * 	    structure to an XColor structure.
 *
 *	RETURNS
 *		void.
 */
{
    for (; nColors--; pXColors++, pColors++) {
	pXColors->pixel = pColors->pixel;
	pXColors->red = pColors->spec.RGB.red;
	pXColors->green = pColors->spec.RGB.green;
	pXColors->blue  = pColors->spec.RGB.blue;
	pXColors->flags = (DoRed | DoGreen | DoBlue);
    }
}


/*
 *	NAME
 *		_XColor_to_XcmsRGB
 *
 *	SYNOPSIS
 */
void
_XColor_to_XcmsRGB(pCCC, pXColors, pColors, nColors)
    XcmsCCC *pCCC;
    XColor *pXColors;
    XcmsColor *pColors;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Translates an RGB color specification in an XColor
 *		structure to an XcmsRGB structure.
 *
 *		IMPORTANT NOTE:  Bit replication that may have been caused
 *		with ResolveColor() routine in the X Server is undone
 *		here if requested!  For example, if red = 0xcaca and the
 *		bits_per_rgb is 8, then spec.RGB.red will be 0xca00.
 *
 *	RETURNS
 *		void
 */
{
    int bits_per_rgb = pCCC->visual->bits_per_rgb;

    for (; nColors--; pXColors++, pColors++) {
	pColors->spec.RGB.red = (pXColors->red & MASK[bits_per_rgb]);
	pColors->spec.RGB.green = (pXColors->green & MASK[bits_per_rgb]);
	pColors->spec.RGB.blue = (pXColors->blue & MASK[bits_per_rgb]);
	pColors->format = XCMS_RGB_FORMAT;
	pColors->pixel = pXColors->pixel;
    }
}


/*
 *	NAME
 *		_XcmsResolveColor
 *
 *	SYNOPSIS
 */
void
_XcmsResolveColor(pCCC, pXColors, nColors)
    XcmsCCC *pCCC;
    XColor *pXColors;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *	    Uses the X Server ResolveColor() algorithm to
 *	    modify values to closest values supported by hardware.
 *	    Old algorithm simply masked low-order bits.  The new algorithm
 *	    has the effect of replicating significant bits into lower order
 *	    bits in order to stretch the hardware value into all 16 bits.
 *
 *	    On a display with N-bit DACs, the "hardware" color is computed as:
 *
 *	    ((unsignedlong)(ClientValue >> (16-N)) * 0xFFFF) / ((1 << N) - 1)
 *		
 *
 *	RETURNS
 *		void.
 */
{
    int shift;
    int max_color;

    shift = 16 - pCCC->visual->bits_per_rgb;
    max_color = (1 << pCCC->visual->bits_per_rgb) - 1;


    for (; nColors--; pXColors++) {
	pXColors->red =
		((unsigned long)(pXColors->red >> shift) * 0xFFFF)
		/ max_color;
	pXColors->green =
		((unsigned long)(pXColors->green >> shift) * 0xFFFF)
		/ max_color;
	pXColors->blue  =
		((unsigned long)(pXColors->blue  >> shift) * 0xFFFF)
		/ max_color;
    }
}


/*
 *	NAME
 *		_XcmsUnresolveColor
 *
 *	SYNOPSIS
 */
void
_XcmsUnresolveColor(pCCC, pColors, nColors)
    XcmsCCC *pCCC;
    XcmsColor *pColors;
    unsigned int nColors;
/*
 *	DESCRIPTION
 *		Masks out insignificant bits.
 *
 *	RETURNS
 *		void.
 *
 *	ASSUMPTIONS
 *		format == XCMS_RGB_FORMAT
 */
{
    int bits_per_rgb = pCCC->visual->bits_per_rgb;

    for (; nColors--; pColors++) {
	pColors->spec.RGB.red &= MASK[bits_per_rgb];
	pColors->spec.RGB.green &= MASK[bits_per_rgb];
	pColors->spec.RGB.blue &= MASK[bits_per_rgb];
    }
}
