/* $XConsortium: sched.h,v 1.1 93/10/26 09:50:40 rws Exp $ */
/**** module sched.h ****/
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

	sched.h -- DDXIE machine independent scheduler definitions

	Robert NC Shelley -- AGE Logic, Inc.  May 1993

******************************************************************************/

#ifndef _XIEH_SCHED
#define _XIEH_SCHED

/* scheduler interface
 */
typedef bandMsk	(*xieBandProc)();
typedef struct _schedvec {
  xieBoolProc	execute;
  xieBandProc	runnable;
} schedVecRec;

/* call scheduler to execute until out of data;
 * returns Bool: true if flo still active
 */
#define Execute(flo,import_pet) \
		(*flo->schedVec->execute)(flo,import_pet)

/* detect execution suspension/resumption
 * returns Bool: true if scheduler has exited to core X since last checked
 * (useful for determining if fresh lookups are needed for core X resources)
 */
#define Resumed(flo,pet) \
		(pet->schedCnt == flo->floTex->exitCnt ? FALSE \
                 : ((pet->schedCnt = flo->floTex->exitCnt), TRUE))

/* toggle the band's ready bit if "available >= threshold" status has changed
 * and return the updated ready mask;
 * if the ready bit doesn't toggle, return NO_BANDS
 */
#define CheckSrcReady(bnd,bmsk) \
		(bnd->receptor->ready & (bmsk) \
		 ? (bnd->available < bnd->threshold \
		    ? (bnd->receptor->ready &= ~(bmsk)) : NO_BANDS) \
		 : (bnd->available >= bnd->threshold \
		    ? (bnd->receptor->ready |=  (bmsk)) : NO_BANDS))

/* try to schedule the specified element based on new data arriving for:
 * 			receptorPtr rcp, bandPtr bnd, and bandMsk bmsk
 * nothing is returned, but pet->scheduled indicates which bands are runnable
 */
#define Schedule(flo,pet,rcp,bnd,bmsk) \
		{ bandMsk r = CheckSrcReady(bnd,bmsk) & rcp->attend; \
		  if(r && !pet->scheduled) { \
		    if(!pet->inSync) { \
		      if(!pet->bandSync || r == (rcp->active & rcp->attend)) {\
		        pet->scheduled |= r; \
			InsertMember(pet, &flo->floTex->schedHead); \
		      } \
		    } else if(r = (*flo->schedVec->runnable)(flo,pet)) { \
		      pet->scheduled = r; \
		      InsertMember(pet,&flo->floTex->schedHead); \
		} } }
#endif /* end _XIEH_SCHED */
