/* $XConsortium: flo.h,v 1.2 93/11/06 15:44:15 rws Exp $ */
/**** module flo.h ****/
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc. and the Massachusetts
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
  
	flo.h -- high level photoflo-specific definitions
  
	Robert NC Shelley -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#ifndef _XIEH_FLO
#define _XIEH_FLO

/* various structure pointers
 */
typedef struct _flotex          *ddFloTexPtr;
typedef struct _petex           *ddPETexPtr;
typedef struct _schedvec        *ddSchedVecPtr;
typedef struct _stripvec        *ddStripVecPtr;

typedef struct _photospace	*photospacePtr;
typedef struct _techvec		*techVecPtr;

/* various function type pointers
 */
typedef Bool		(*xieBoolProc)();
typedef CARD8 *		(*xieDataProc)();
typedef int		(*xieIntProc)();
typedef void		(*xieVoidProc)();


#ifndef _XIEC_FLO
extern	struct _flodef *MakeFlo();
extern	Bool            EditFlo();
extern  void		PrepFlo();
extern	struct _flodef *FreeFlo();
extern	struct _pedef  *MakePEDef();
extern	struct _pedef  *FreePEDef();
extern  void		SendClientData();
extern  Bool		UpdateFormatfromLevels();
#endif

#ifndef _XIEC_EVENT
extern	void		SendFloEvent();
#endif

#endif /* end _XIEH_FLO */
