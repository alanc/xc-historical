/* $XConsortium: xiemd.h,v 1.2 93/10/31 09:46:20 dpw Exp $ */
/**** module xiemd.h ****/
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
  
	xiemd.h -- DDXIE MI prototype Machine Dependent definitions
  
	Larry Hare -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#ifndef _XIEH_MD
#define _XIEH_MD

#include <X.h>
#include <servermd.h>
#include "misc.h" /* for pointer */

#ifndef _XIEC_MPBITFUN
extern unsigned char _ByteReverseTable[];
#endif

/*
** Bit operations:
**
**		DATATYPE DEPENDENT:
**  LogInt -	Should be chosen as the largest efficient unsigned type which
**		supports C based logical operations, as well as loads and
**		stores.  This will usually be 32 bit unsigned long, but 
**		might conceivably be an 64 bit type on some newer machines.
**		If you change this, update LOGSHIFT, LOGSIZE, LOGMASK.
**  LOGSHIFT -	Log2 of LOGSIZE
**  LOGSIZE -	Size of LogInt in bits.  You should make sure your compiler
**		knows that this is really 32 and not (4<<3).
**  LOGMASK -	Make sure your compiler thinks this is 31 and not ((4<<3)-1)
**
**		ENDIAN DEPENDENT:
**  LOGLEFT -	LeftMost bit (from screen perspective) in a LogInt.
**  LOGRIGHT -	In place shift of a LogInt right 1 bit (in screen perspective)>
**
**  LOGINDX,LOGBIT - Used by LOG_xxxbit macros for individual access to bits.
**  LOG_tstbit - test if a bit from memory is set.
**  LOG_chgbit - test if a bit in memory differs from a constant.
**  LOG_setbit,LOG_clrbit,LOG_xorbit  - set, clr, or xor a bit in memory.
**  
**  Some pieces of bit related code have been written to use word loops
**  using LogInt Bits at a time.  Some algorithms use LOG_xxxbit macros
**  to access individual bits.  
*/

typedef unsigned long	LogInt;
#define LOGSHIFT	(5)
#define LOGSIZE		(sizeof(LogInt)<<3)
#define LOGMASK		(LOGSIZE-1)

#if (IMAGE_BYTE_ORDER == MSBFirst)
#define BitLeft(V,N)	((V) << (N))
#define BitRight(V,N)	((V) >> (N))
#else
#define BitLeft(V,N)	((V) >> (N))
#define BitRight(V,N)	((V) << (N))
#endif /* endian type */

#define LOGLEFT		BitLeft(((LogInt)~0),LOGMASK)
#define LOGRIGHT(M)	(M=BitRight(M,1))
#define LOGONES		~((LogInt)0)
#define LOGZERO		((LogInt)0)

#define LOGINDX(X)	((X)>>LOGSHIFT)
#define LOGBIT(X)	BitRight(LOGLEFT,(X) & LOGMASK)
#define LOG_chgbit(P,N,C)	(((P)[LOGINDX(N)] ^ C) & LOGBIT(N))
#define LOG_tstbit(P,N)		((P)[LOGINDX(N)] & LOGBIT(N))
#define LOG_clrbit(P,N)		((P)[LOGINDX(N)] &= ~LOGBIT(N))
#define LOG_setbit(P,N)		((P)[LOGINDX(N)] |= LOGBIT(N))
#define LOG_xorbit(P,N)		((P)[LOGINDX(N)] ^= LOGBIT(N))

/* Special version for unsigned bytes used for byte streams that might not be
   LogInt aligned
*/
typedef unsigned char		LogByte;
#define LOGBYTESHIFT		(3)
#define LOGBYTESIZE		(sizeof(LogByte)<<3)
#define LOGBYTEMASK		(LOGBYTESIZE-1)

#define LOGBYTELEFT		BitLeft(((LogByte)~0),LOGBYTEMASK)
#define LOGBYTERIGHT(M)		(M=BitRight(M,1))
#define LOGBYTEONES		~((LogByte)0)
#define LOGBYTEZERO		((LogByte)0)

#define LOGBYTEINDX(X)		((X)>>LOGBYTESHIFT)
#define LOGBYTEBIT(X)		BitRight(LOGBYTELEFT,(X) & LOGBYTEMASK)
#define LOGBYTE_chgbit(P,N,C)	(((P)[LOGBYTEINDX(N)] ^ C) & LOGBYTEBIT(N))
#define LOGBYTE_tstbit(P,N)	((P)[LOGBYTEINDX(N)] & LOGBYTEBIT(N))
#define LOGBYTE_clrbit(P,N)	((P)[LOGBYTEINDX(N)] &= ~LOGBYTEBIT(N))
#define LOGBYTE_setbit(P,N)	((P)[LOGBYTEINDX(N)] |= LOGBYTEBIT(N))
#define LOGBYTE_xorbit(P,N)	((P)[LOGBYTEINDX(N)] ^= LOGBYTEBIT(N))
/*
**  Bit Fun.  Fun with Bits.  Functions for bits:
**
**	bitexpand() - expand a bunch of bits to fg,bg bytes.
*/

pointer bitexpand();
void  bitshrink();
void  action_clear();
void  action_set();
void  action_invert();

/*
**  We envision a triangular matrix of conversion routines for 
**  indexing into smaller data quantums, such as to return a byte
**  from an array as a short.
*/

/*
**  Floating Point Considerations.
**
** The following definitions are used in individual modules. We might
** want to regularaize all these in this one file, or even make these
** into one tyepdef like ProcessFloat.  The goal is so that the server
** can consistently use floats instead of doubles on machines which 
** can do floats quicker than doubles:
**
**	typedef float BlendFloat;	(see mpblend.c)
**	typedef float ConstrainFloat;	(see mpcnst.c)
**	typedef float DitherFloat;	(see mpdither.c)
**	typedef float ConvolveFloat;	(see element.h)
**	typedef float PasteUpFloat;	(see element.h)
**
** Some functions are not yet written to this standard.  For instance
** geometry is susceptible to roundoff errors keeping track of scanline
** threshold data maps.
**
**	typedef RealPixel GeomFloat;
*/

/* 
** USE_FLOATS is used in mpcnst.c to enable the originaly easy to
** program floating point ClipScale routines.  The current integer
** based routine no longer does a divide and is hence probably faster
** on all rational machines.  We may come up with other choices for
** integer versus floating algorithms in the future.  We would perhaps
** like to select between them here.
**
**		#define USE_FLOATS
*/



#endif /* module _XIEH_MD */
