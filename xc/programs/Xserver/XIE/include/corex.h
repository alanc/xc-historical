/* $XConsortium: corex.h,v 1.1 93/10/26 10:03:35 rws Exp $ */
/**** module corex.h ****/
/******************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*******************************************************************************

	corex.h - Xie server interface (for hooking to Core X)

		Dean Verheiden -- AGE Logic, Inc. April, 1993

******************************************************************************/

#ifndef _XIEH_COREX
#define _XIEH_COREX

#include <extnsionst.h>

/* temporary hack:
 *
 * XIE is suppose to support single-band images up to the depth
 * of the deepest drawable supported by the server -- but not less than 16.
 * Triple-band data should never be more than 16 bits.
 *
 * Later releases will compute the correct value, but it will not exceed 24
 * in the sample implementation.  (eeeck, sounds like a global variable..)
 */

#define MAX_SINGLE 24
#define MAX_TRIPLE 16
#define MAX_DEPTH(nbands)  (nbands > 1 ? MAX_TRIPLE : MAX_SINGLE)
#define MAX_LEVELS(nbands) (nbands > 1 ? (1<<MAX_TRIPLE) : (1<<MAX_SINGLE))

#define ALIGNMENT  xieValArbitrary	

#define PITCH_MOD 32			/* default scanline pitch	   */


#if defined(_XIEC_SESSION)
/*
 * Define constants relating to unconstrained computation
 * See QueryImageExtension for description
 * Return values for Single Precision IEEE floats for now
 */
#define UNCONSTRAINED_MANTISSA		  24
#define UNCONSTRAINED_MAX_EXPONENT	 128	
#define UNCONSTRAINED_MIN_EXPONENT	-126	

CARD32       Preferred_levels[] = 
			{ 
			1<<1, 		/* Bit pixels */
			1<<8,		/* Byte pixels */
			1<<16,		/* Pair pixels */	
			1<<24		/* Quad pixels */
			};
#endif

/*
 * Interface to Core X 
 */
extern Bool         AddResource();
extern void         FreeResource();

#ifndef _XIEC_SESSION
extern ExtensionEntry 	*extEntry;

extern RESTYPE	RC_XIE;			/* XIE Resource Class		*/
#if XIE_FULL
extern RESTYPE	RT_COLORLIST;		/* ColorList resource type	*/
#endif
extern RESTYPE	RT_LUT;			/* Lookup table resource type	*/
extern RESTYPE	RT_PHOTOFLO;		/* Photoflo   resource type	*/
extern RESTYPE	RT_PHOTOMAP;		/* Photomap   resource type	*/
extern RESTYPE	RT_PHOTOSPACE;		/* Photospace resource type	*/
#if XIE_FULL
extern RESTYPE	RT_ROI;			/* Region Of Interest type	*/
#endif
extern RESTYPE	RT_XIE_CLIENT;		/* XIE Type for Shutdown Notice */
#endif

#if !defined(_XIEC_COLORLST) && (XIE_FULL)
extern	int		DeleteColorList();
#endif
#ifndef _XIEC_LUT
extern	int		DeleteLUT();
#endif
#ifndef _XIEC_PHOTOMAP
extern	int		DeletePhotomap();
#endif
#ifndef _XIEC_PROTOFLO
extern	int		DeletePhotoflo();
extern	int		DeletePhotospace();
#endif
#if !defined(_XIEC_ROI) && (XIE_FULL)
extern	int		DeleteROI();
#endif
#ifndef _XIEC_SESSION
extern	int		DeleteXieClient();
#endif

#endif /* end _XIEH_COREX */



