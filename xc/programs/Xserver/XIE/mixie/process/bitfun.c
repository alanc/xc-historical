/* $XConsortium$ */
/**** module bitfun.c ****/
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
  
	bitfun.c -- DDXIE utilities for bits and other fun things.
  
	Larry Hare -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#include "xiemd.h"

/*---------------------------------------------------------------------------
--------------------------------  bitexpand  --------------------------------
---------------------------------------------------------------------------*/

/*
**  Following routine is used to expand a length of bit pixels to byte
**  pixels.  Similar routines could be written for other pixels sizes
**  but currently we don't really support them (eg 4 bits) or doesnt
**  really speed them up (16 bits).  
**
**  You could also do this with any other bit expansion tricks you 
**  have up your sleeve.  For instance, you could look up 8 bits
**  at a time, and have a table of 256 long longs; or use your 
**  favorite graphics accelerator, or ...  bitfun.
*/


unsigned long xie8StippleMasks[16] = {
0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff,
};


#if (BITMAP_BIT_ORDER == MSBFirst)
#define g4	(c = (inval>>26) & 0x3c, inval <<= 4,			\
		*((unsigned long *) ((int) &xie8StippleMasks[0] + c)) & val)
#define g4r	(c = (inval>>26) & 0x3c, inval <<= 4,			\
		~(*((unsigned long *) ((int) &xie8StippleMasks[0] + c))) & val)
#define g4b	(c = (inval>>26) & 0x3c, inval <<= 4,			\
		 c = *((unsigned long *) ((int) &xie8StippleMasks[0] + c)), \
						(loval & ~c) | (hival & c))
#else
#define g4	(c = inval & 0xf, inval >>= 4, xie8StippleMasks[c] & val)
#define g4r	(c = inval & 0xf, inval >>= 4, ~xie8StippleMasks[c] & val)
#define g4b	(c = inval & 0xf, inval >>= 4, c = xie8StippleMasks[c], \
						(loval & ~c) | (hival & c))
#endif

#define p4 *outp++		

void
bitexpand(inp,outp,bw,olow,ohigh)
	LogInt *inp;			/* these are actualy bits... */
	unsigned long *outp;		/* its actually bytes but ... */
	int bw;				/* number of bits to extract */
	unsigned char olow;		/* zero bit maps to this */
	unsigned char ohigh;		/* one bit maps to this */
{
	int nw;
	unsigned long inval, c;
	if (olow == 0) {
	    unsigned long val = ohigh;
	    val = (val << 8) | val; val = (val << 16) | val;
	    for ( nw = bw >> 5; nw > 0; nw--) {
		inval = *inp++;
		p4 = g4; p4 = g4; p4 = g4; p4 = g4;
		p4 = g4; p4 = g4; p4 = g4; p4 = g4;
	    }
	    if ((bw &= 31) > 0) 
		for (inval = *inp; bw > 0; bw -= 4)
		    p4 = g4;
	} else if (ohigh == 0) {
	    unsigned long val = olow;
	    val = (val << 8) | val; val = (val << 16) | val;
	    for ( nw = bw >> 5; nw > 0; nw--) {
		inval = *inp++;
		p4 = g4r; p4 = g4r; p4 = g4r; p4 = g4r;
		p4 = g4r; p4 = g4r; p4 = g4r; p4 = g4r;
	    }
	    if ((bw &= 31) > 0) 
		for (inval = *inp; bw > 0; bw -= 4)
		    p4 = g4r;
	} else {
	    unsigned long loval = olow, hival = ohigh;
	    loval = (loval << 8) | loval; loval = (loval << 16) | loval;
	    hival = (hival << 8) | hival; hival = (hival << 16) | hival;
	    for ( nw = bw >> 5; nw > 0; nw--) {
		inval = *inp++;
		p4 = g4b; p4 = g4b; p4 = g4b; p4 = g4b;
		p4 = g4b; p4 = g4b; p4 = g4b; p4 = g4b;
	    }
	    if ((bw &= 31) > 0) 
		for (inval = *inp; bw > 0; bw -= 4)
		    p4 = g4b;
	}
	/********
	if (bw > 0)
	    for (M=LOGLEFT, inval = *inp++; bw; bw--, LOGRIGHT(M)) 
		*outp++ =  (inval & M) ? ohigh : olow ; 
	*********/
}

/**** module bitfun.c ****/
