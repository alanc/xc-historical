/* $XConsortium$ */
/**** module tex.h ****/
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
******************************************************************************
  
	tex.h -- generic device dependent photoflo definitions
  
	Dean Verheiden -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#ifndef _XIEH_TEX
#define _XIEH_TEX

#define ALL_BANDS	((bandMsk)~0)
#define NO_BANDS	((bandMsk)0)
#define NO_INPLACE	((INT32)-1)
#define NO_DATAMAP	((CARD32)0)
#define NO_PRIVATE	((CARD32)0)
#define NO_SYNC		((Bool)FALSE)
#define SYNC		((Bool)TRUE)

#define FLUSH		((Bool)TRUE)
#define KEEP		((Bool)FALSE)



#ifndef _XIEC_FLOMAN
extern	int	InitFloManager();
extern	int	MakePETex();
extern	Bool	InitReceptors();
extern	Bool	InitReceptor();
extern	Bool	InitEmitter();
extern	Bool	InitBand();
extern	void	ResetReceptors();
extern	void	ResetEmitter();
#endif

#ifndef _XIEC_SCHED
extern	int	InitScheduler();
#endif

#ifndef _XIEC_STRIP
extern	int	InitStripManager();
extern  int	DebriefStrips();
extern  void	FreeStrips();
#endif

#endif /* end _XIEH_TEX */
