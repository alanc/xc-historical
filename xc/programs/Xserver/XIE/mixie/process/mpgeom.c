/* $XConsortium$ */
/**** module mpgeom.c ****/
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
*****************************************************************************
  
	mpgeom.c -- DDXIE geometry element
  
	Ben Fahy  -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/


#define _XIEC_MPGEOM
#define _XIEC_PGEOM

/*
 *  Include files
 */
#include <stdio.h>

/*
 *  Core X Includes
 */
#include <X.h>
#include <Xproto.h>
/*
 *  XIE Includes
 */
#include <XIE.h>
#include <XIEproto.h>
/*
 *  more X server includes.
 */
#include <misc.h>
#include <dixstruct.h>
#include <extnsionst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <element.h>
#include <texstr.h>
#include <mpgeom.h>


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeGeometry();

/*
 *  routines which we reference from other DDXIE modules
 */
extern int	miAnalyzeGeomNN();
extern int	miAnalyzeGeomAA();
extern int	miAnalyzeGeomBi();

static xieIntProc miAnalyzeGeomVecs[] = {
	miAnalyzeGeomAA,
	miAnalyzeGeomBi,
	miAnalyzeGeomNN,
	(xieIntProc) NULL
};
#define n_vecs (sizeof(miAnalyzeGeomVecs)/sizeof(xieIntProc) - 1)

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeGeometry(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
int i;

	for (i=0; i<n_vecs; ++i) {
		if (miAnalyzeGeomVecs[i](flo,ped) == TRUE)
			return(TRUE);
	}
	return(FALSE);
}                               /* end miAnalyzeGeometry */
/* end module mpgeom.c */
