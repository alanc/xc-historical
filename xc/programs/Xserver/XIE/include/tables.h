/* $XConsortium$ */
/**** module tables.h ****/
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

	tables.h: entry points etc.

	Dean Verheiden, Robert NC Shelley -- AGE Logic, Inc. April 1993

******************************************************************************/

#ifndef _XIEH_TABLES
#define _XIEH_TABLES

#include <flostr.h>

#ifndef _XIEC_TABLES
extern	void		init_proc_tables();     
extern	peDefPtr	(*MakeTable[])();

#else  /* _XIEC_TABLES */
	/* colorlist.c */
extern	int		ProcCreateColorList();
extern	int		ProcDestroyColorList();
extern	int		ProcPurgeColorList();
extern	int		ProcQueryColorList();
extern	int		SProcCreateColorList();
extern	int		SProcDestroyColorList();
extern	int		SProcPurgeColorList();
extern	int		SProcQueryColorList();
	/* error.c */
extern	int		ProcNotImplemented();
	/* flo.c */
extern  int		ProcAbort();
extern  int		ProcAwait();
extern  int		ProcGetClientData();
extern  int		ProcPutClientData();
extern  int		ProcQueryPhotoflo();
extern  int		SProcAbort();
extern  int		SProcAwait();
extern  int		SProcGetClientData();
extern  int		SProcPutClientData();
extern  int		SProcQueryPhotoflo();
	/* immediate.c */
extern	int		ProcCreatePhotospace();
extern	int		ProcDestroyPhotospace();
extern  int		ProcExecuteImmediate();
extern	int		SProcCreatePhotospace();
extern	int		SProcDestroyPhotospace();
extern  int		SProcExecuteImmediate();
/* lut.c */
extern	int		ProcCreateLUT();
extern	int		ProcDestroyLUT();
extern	int		SProcCreateLUT();
extern	int		SProcDestroyLUT();
	/* photomap.c */
extern	int		ProcCreatePhotomap();
extern	int		ProcDestroyPhotomap();
extern	int		ProcQueryPhotomap();
extern	int		SProcCreatePhotomap();
extern	int		SProcDestroyPhotomap();
extern	int		SProcQueryPhotomap();
	/* roi.c */
extern	int		ProcCreateROI();
extern	int		ProcDestroyROI();
extern	int		SProcCreateROI();
extern	int		SProcDestroyROI();
	/* stored.c */
extern	int		ProcCreatePhotoflo();
extern	int		ProcDestroyPhotoflo();
extern	int		ProcExecutePhotoflo();
extern	int		ProcModifyPhotoflo();
extern	int		ProcRedefinePhotoflo();
extern	int		SProcCreatePhotoflo();
extern	int		SProcDestroyPhotoflo();
extern	int		SProcExecutePhotoflo();
extern	int		SProcModifyPhotoflo();
extern	int		SProcRedefinePhotoflo();
	/* technq.c */
extern	int		ProcQueryTechniques();
extern	int		SProcQueryTechniques();

/* elements */
extern	peDefPtr	MakeIDraw();
extern	peDefPtr	MakeIDrawP();
extern	peDefPtr	MakeICLUT();
extern	peDefPtr	MakeICPhoto();
extern	peDefPtr	MakeICROI();
extern	peDefPtr	MakeILUT();
extern	peDefPtr	MakeIPhoto();
extern	peDefPtr	MakeIROI();
extern	peDefPtr	MakeArith();
extern	peDefPtr	MakeBandCom();
extern	peDefPtr	MakeBandExt();
extern	peDefPtr	MakeBlend();
extern	peDefPtr	MakeConstrain();
extern	peDefPtr	MakeConvertFromIndex();
extern	peDefPtr	MakeConvertToIndex();
extern	peDefPtr	MakeConvolve();
extern	peDefPtr	MakeDither();
extern	peDefPtr	MakeGeometry();
extern	peDefPtr	MakeLogic();
extern	peDefPtr	MakePasteUp();
extern	peDefPtr	MakePoint();
extern	peDefPtr	MakeUnconstrain();
extern	peDefPtr	MakeECLUT();
extern	peDefPtr	MakeECPhoto();
extern	peDefPtr	MakeEDraw();
extern	peDefPtr	MakeEDrawPlane();
extern	peDefPtr	MakeELUT();
extern	peDefPtr	MakeEPhoto();
extern	peDefPtr	MakeEROI();
extern	peDefPtr	MakeECROI();
#endif /* _XIEC_TABLES */

#endif /* _XIEH_TABLES */
