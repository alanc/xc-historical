/* $XConsortium: photomap.h,v 1.2 93/10/31 09:49:01 dpw Exp $ */
/**** module photomap.h ****/
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
******************************************************************************

    photomap.h -- contains photomap definitions

    Dean Verheiden, Robert NC Shelley -- AGE Logic, Inc. April 1993

******************************************************************************/

#ifndef _XIEH_PHOTOMAP
#define _XIEH_PHOTOMAP

#include <flodata.h>

/*
 *  Definitions
 */
typedef struct _photomap *photomapPtr;

typedef struct _photomap {
  xieTypPhotomap ID;
  CARD16	  refCnt;
  CARD16	  technique;	/* decode technique needed to decompress*/
  xieTypDataClass dataClass;	/* SingleBand or TripleBand. 		*/
  CARD8		  bands;	/* number of bands {0=unpopulated,1,3}	*/
				/* Tripleband, interleaved has bands=1, */
				/* so this parameter is *not* redundant */
  xieTypDataType  dataType;	/* {constrained,unconstrined}		*/
  CARD8		  pad;
  pointer	  tecParms;	/* contains technique specific decode   */
				/* parameters				*/
  pointer	  pvtParms;	/* decode hints				*/
  formatRec	  format[xieValMaxBands];   /* format of data in strips */
  stripLstRec	  strips[xieValMaxBands];   /* lists of image strips    */
} photomapRec;


#endif
/* end _XIEH_PHOTOMAP */
