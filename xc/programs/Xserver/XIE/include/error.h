/* $XConsortium$ */
/**** module error.h ****/
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

	error.h -- error specific definitions

	Robert NC Shelley -- AGE Logic, Inc. April, 1993

******************************************************************************/

#ifndef _XIEH_ERROR
#define _XIEH_ERROR

#ifndef _XIEC_ERROR
#include <flo.h>
extern	int	 SendResourceError();
extern	int	 SendFloIDError();
extern	int	 SendFloError();
extern	void	 ErrGeneric();
extern	void	 ErrResource();
extern	void	 ErrDomain();
extern	void	 ErrOperator();
extern	void	 ErrTechnique();
extern	void	 ErrValue();
#endif

/*
 *  Convenience macros for dealing with the floDef generic error packet
 */
#define ferrTag(flo)          ((flo)->error.phototag)
#define ferrType(flo)         ((flo)->error.type)
#define ferrCode(flo)         ((flo)->error.floErrorCode)
#define	ferrError(flo,tag,type,code) \
		(ferrTag(flo)=(tag),ferrType(flo)=(type),ferrCode(flo)=(code))

/*
 * convenience macros for general flo errors
 */
#define	FloAccessError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloAccess); xfer;}
#define	FloAllocError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloAlloc); xfer;}
#define	FloElementError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloElement); xfer;}
#define	FloImplementationError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloImplementation); xfer;}
#define	FloLengthError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloLength); xfer;}
#define	FloSourceError(flo,tag,type,xfer) \
		{FloError(flo,tag,type,xieErrNoFloSource); xfer;}

/*
 * convenience macros for element-specific errors
 */
#define	AccessError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloAccess); xfer;}
#define	AllocError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloAlloc); xfer;}
#define	ColorListError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloColorList,id); xfer;}
#define	ColormapError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloColormap,id); xfer;}
#define	DomainError(flo,ped,dom,xfer) \
		{ErrDomain(flo,ped,dom); xfer;}
#define	DrawableError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloDrawable,id); xfer;}
#define	ElementError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloElement); xfer;}
#define	GCError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloGC,id); xfer;}
#define	ImplementationError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloImplementation); xfer;}
#define	LUTError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloLUT,id); xfer;}
#define	MatchError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloMatch); xfer;}
#define	OperatorError(flo,ped,op,xfer) \
		{ErrOperator(flo,ped,op); xfer;}
#define	PhotomapError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloPhotomap,id); xfer;}
#define	ROIError(flo,ped,id,xfer) \
		{ErrResource(flo,ped,xieErrNoFloROI,id); xfer;}
#define	SourceError(flo,ped,xfer) \
		{ErrGeneric(flo,ped,xieErrNoFloSource); xfer;}
#define	TechniqueError(flo,ped,tech,len,xfer) \
		{ErrTechnique(flo,ped,tech,len); xfer;}
#define	ValueError(flo,ped,value,xfer) \
		{ErrValue(flo,ped,value); xfer;}

#endif /* end _XIEH_ERROR */
