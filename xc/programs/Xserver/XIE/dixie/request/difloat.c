/* $XConsortium: difloat.c,v 1.1 93/10/26 09:58:58 rws Exp $ */
/**** module difloat.c ****/
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

	difloat.c - float converters

	Ben Fahy          -- AGE Logic, Inc. December 1992
	Robert NC Shelley -- AGE Logic, Inc. April 1993

******************************************************************************/

#ifndef XIE_IEEE_IS_NATIVE
#define _XIEC_DIFLOAT

/*
 *  Include files
 */
#include <stdio.h>
#include <math.h>
  /*
   *  Core X Includes
   */
#include <Xproto.h>
  /*
   *  Server XIE Includes
   */
#include <difloat.h>

/*
 *  routines referenced by other modules.
 */
double ConvertIEEEtoNative();


double ConvertIEEEtoNative(ieee)
     CARD32 ieee;
{
  double sign;
  double exponent;
  double fraction;
  
  if(!ieee) return(0.0);
  
  sign     = (ieee & ieeeFloatSignMask) == ieeeFloatSignMask ? -1.0 : 1.0;
  exponent = (int)((ieee & ieeeFloatExpMask) >> ieeeFloatExpShift) - 127;
  fraction = (double)(ieee & ieeeFloatMantissaMask) / (1 << ieeeMantissaSize);
  
  return(sign * pow(2.0,exponent) * (1.0 + fraction));
}                               /* end ConvertIEEEtoNative */
#endif /* !XIE_IEEE_IS_NATIVE */

/* end module difloat.c */
