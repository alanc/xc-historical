/* $XConsortium: faxint.h,v 1.1 93/10/26 09:52:13 rws Exp $ */
/**** module fax/faxint.h ****/
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
  
	fax/faxint.h -- DDXIE internal fax definitions
  
	Ben Fahy -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/

/* this should correspond to what is in pretab.h, except for code entry */
typedef struct _FaxTable {
short run_length;	/* run length for this code 	*/
char 	makeup;		/* 1 if this code is makeup 	*/
unsigned char n_bits;	/* how many bits this code used */
} WhiteTable,BlackTable;

typedef struct _TwoDTable {
	 char  mode;	/* what to do next		*/
unsigned char n_bits;	/* how many bits this code used */
} TwoDTable;

#ifdef FAX_TABLE_OWNER
#include "faxtabs.h"		/* initializes tables	*/
#else
extern unsigned char  	_ByteReverseTable[];
extern BlackTable 	_BlackFaxTable[];
extern WhiteTable 	_WhiteFaxTable[];
extern TwoDTable 	_TwoDFaxTable[];
#endif

/**** module fax/faxint.h ****/
