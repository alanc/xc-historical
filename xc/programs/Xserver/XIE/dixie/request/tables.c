/* $XConsortium$ */
/**** tables.c ****/
/****************************************************************************
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

	tables.c: XIE vector tables

	Dean Verheiden, AGE Logic, Inc	March 1993

****************************************************************************/

#define _XIEC_TABLES

#define NEED_REPLIES
#include "X.h"			/* Needed for just about anything	*/
#include "Xproto.h"		/* defines protocol-related stuff	*/
#include "misc.h"		/* includes os.h, which type FatalError	*/
#include "dixstruct.h" 		/* this picks up ClientPtr definition	*/
#include "extnsionst.h"		/* defines things like ExtensionEntry	*/
#include <stdio.h>		/* needed if we do any printf's		*/

#include "XIE.h"		
#include "XIEproto.h"		/* Xie v4 protocol specification	*/
#include <corex.h>		/* interface to core X definitions	*/
#include <error.h>
#include <tables.h>

static peDefPtr ElementNotImplemented();

static int (*ProcTable413[])() = {
/* 00 */  ProcNotImplemented,	/* Illegal protocol request */	
/* 01 */  ProcNotImplemented,	/* QueryImageExtension doesn't use the table */
/* 02 */  ProcQueryTechniques,
/* 03 */  ProcCreateColorList,
/* 04 */  ProcDestroyColorList,
/* 05 */  ProcPurgeColorList,
/* 06 */  ProcQueryColorList,
/* 07 */  ProcCreateLUT,
/* 08 */  ProcDestroyLUT,
/* 09 */  ProcCreatePhotomap,
/* 10 */  ProcDestroyPhotomap,
/* 11 */  ProcQueryPhotomap,
/* 12 */  ProcCreateROI,
/* 13 */  ProcDestroyROI,
/* 14 */  ProcCreatePhotospace,
/* 15 */  ProcDestroyPhotospace,
/* 16 */  ProcExecuteImmediate,
/* 17 */  ProcCreatePhotoflo,
/* 18 */  ProcDestroyPhotoflo,
/* 19 */  ProcExecutePhotoflo,
/* 20 */  ProcModifyPhotoflo,
/* 21 */  ProcRedefinePhotoflo,
/* 22 */  ProcPutClientData,
/* 23 */  ProcGetClientData,
/* 24 */  ProcQueryPhotoflo,
/* 25 */  ProcAwait,
/* 26 */  ProcAbort
  };
     

static int (*SProcTable413[])() = {
/* 00 */  ProcNotImplemented,	/* Illegal protocol request */	
/* 01 */  ProcNotImplemented,	/* QueryImageExtension doesn't use the table */
/* 02 */  SProcQueryTechniques,
/* 03 */  SProcCreateColorList,
/* 04 */  SProcDestroyColorList,
/* 05 */  SProcPurgeColorList,
/* 06 */  SProcQueryColorList,
/* 07 */  SProcCreateLUT,
/* 08 */  SProcDestroyLUT,
/* 09 */  SProcCreatePhotomap,
/* 10 */  SProcDestroyPhotomap,
/* 11 */  SProcQueryPhotomap,
/* 12 */  SProcCreateROI,
/* 13 */  SProcDestroyROI,
/* 14 */  SProcCreatePhotospace,
/* 15 */  SProcDestroyPhotospace,
/* 16 */  SProcExecuteImmediate,
/* 17 */  SProcCreatePhotoflo,
/* 18 */  SProcDestroyPhotoflo,
/* 19 */  SProcExecutePhotoflo,
/* 20 */  SProcModifyPhotoflo,
/* 21 */  SProcRedefinePhotoflo,
/* 22 */  SProcPutClientData,
/* 23 */  SProcGetClientData,
/* 24 */  SProcQueryPhotoflo,
/* 25 */  SProcAwait,
/* 26 */  SProcAbort
  };
     
peDefPtr (*MakeTable[])() = {
/* 00 */  ElementNotImplemented,
/* 01 */  MakeICLUT,
/* 02 */  MakeICPhoto,
/* 03 */  MakeICROI,
/* 04 */  MakeIDraw,
/* 05 */  MakeIDrawP,
/* 06 */  MakeILUT,
/* 07 */  MakeIPhoto,
/* 08 */  MakeIROI,
/* 09 */  MakeArith,
/* 10 */  MakeBandCom,
/* 11 */  MakeBandExt,
/* 12 */  MakeBlend,
/* 13 */  ElementNotImplemented,
/* 14 */  MakeConstrain,
/* 15 */  MakeConvertFromIndex,
/* 16 */  ElementNotImplemented,
/* 17 */  MakeConvertToIndex,
/* 18 */  ElementNotImplemented,
/* 19 */  MakeConvolve,
/* 20 */  MakeDither,
/* 21 */  MakeGeometry,
/* 22 */  MakeLogic,
/* 23 */  ElementNotImplemented,
/* 24 */  ElementNotImplemented,
/* 25 */  MakePasteUp,
/* 26 */  MakePoint,
/* 27 */  MakeUnconstrain,
/* 28 */  ElementNotImplemented,
/* 29 */  MakeECLUT,
/* 30 */  MakeECPhoto,
/* 31 */  MakeECROI,
/* 32 */  MakeEDraw,
/* 33 */  MakeEDrawPlane,
/* 34 */  MakeELUT,
/* 35 */  MakeEPhoto,
/* 36 */  MakeEROI
  };

   
/************************************************************************
     Fill in the version specific tables for the selected minor protocol 
     version
************************************************************************/
void init_proc_tables(minorVersion, ptable, sptable)
     CARD16 minorVersion;
     int (**ptable[])(), (**sptable[])();
{
  /* Kind of boring with only one version to work with */
  switch (minorVersion) {
  case 13:
    *ptable = ProcTable413;
    *sptable = SProcTable413;
    break;
  default:
    *ptable = ProcTable413;
    *sptable = SProcTable413;
  }
}

/*------------------------------------------------------------------------
---------------- Error stub for unsupported element types ----------------
------------------------------------------------------------------------*/
static peDefPtr ElementNotImplemented(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  FloImplementationError(flo,tag,pe->elemType, return(NULL));
}

/* end module tables.c */


