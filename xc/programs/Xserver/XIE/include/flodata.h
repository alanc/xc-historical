/* $XConsortium$ */
/**** module flodata.h ****/
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

	flodata.h - data formats 

	Dean Verheiden, Robert NC Shelley -- AGE Logic, Inc. April 1993

******************************************************************************/

#ifndef _XIE_FLODATA
#define _XIE_FLODATA

#include <flo.h>

/*
 * Supported data format classes
 */
#define	BIT_PIXEL	  1	/* bitonal data				*/
#define BYTE_PIXEL	  2	/* up to 2^8  levels			*/
#define PAIR_PIXEL	  3	/* up to 2^16 levels			*/
#define	QUAD_PIXEL	  4	/* up to 2^32 levels			*/
#define UNCONSTRAINED	 16	/* levels are undefined 		*/
#define LUT_ARRAY	 32	/* non-canonic lut array		*/
#define RUN_LENGTH	 64	/* non-canonic run_length		*/
#define STREAM		128	/* non-canonic generic stream		*/
#define ENCODED		129	/* non-canonic compressed stream	*/

#define IsCanonic(df)     !(df & (LUT_ARRAY | RUN_LENGTH | STREAM))
#define IsConstrained(df) \
		!(df & (LUT_ARRAY | RUN_LENGTH | STREAM | UNCONSTRAINED))
#define IndexClass(df)    (df == UNCONSTRAINED ? 0 : df )

/*
 * Data types and sizes for supported format classes
 */
typedef	CARD8	BitPixel;
typedef	CARD8	BytePixel;
typedef	CARD16	PairPixel;
typedef	CARD32	QuadPixel;
typedef	float	RealPixel;		/* type of Unconstrained data	*/

#define sz_BitPixel		 1	
#define sz_BytePixel		 8	
#define sz_PairPixel		16	
#define sz_QuadPixel		32
#define sz_RealPixel		32	/* size of Unconstrained data	*/

typedef struct _format {
  CARD8   class;	/* format class {e.g. BIT_PIXEL, STREAM, ...}	*/
  CARD8	  band;		/* band number {0,1,2}				*/
  BOOL    interleaved;	/* true if pixels contain multiple bands	*/
  CARD8   depth;	/* minimum bits needed to contain levels	*/
  CARD32  width;	/* width in pixels				*/
  CARD32  height;	/* height in pixels				*/
  CARD32  levels;	/* quantization levels				*/
  CARD32  stride;	/* distance between adjacent pixels in bits	*/
  CARD32  pitch;	/* distance between adjacent scanlines in bits	*/
  void   *params;	/* pointer to technique-specific parameters	*/
} formatRec, *formatPtr;


typedef struct _strip {
  struct _strip *flink;	/* link to next strip				  */
  struct _strip *blink;	/* link to previous strip			  */
  struct _strip *parent;/* link to strip from which this one was cloned   */
  formatRec  *format;	/* pointer to format record that describes data	  */
  CARD32      refCnt;	/* reference count				  */
  BOOL	      Xowner;	/* if true, core X "owns" the data buffer	  */
  BOOL        canonic;	/* if true, units are scanlines, otherwise bytes  */
  BOOL        final;	/* if true, this is the last strip for this band  */
  BOOL        cache;	/* if true, buffer can be cached (standard size)  */
  CARD32      start;	/* first line/byte of overall data in this strip  */
  CARD32      end;	/* last  line/byte of overall data in this strip  */
  CARD32      length;	/* lines/bytes of useable data in buffer	  */
  CARD32      bitOff;	/* bit offset to first data (usually zero)	  */
  CARD32      bufSiz;	/* size of the data buffer in bytes		  */
  CARD8      *data;	/* pointer to the data buffer			  */
} stripRec, *stripPtr;

/* generic header for managing circular doubly linked lists
 */
typedef struct _lst {
    struct _lst *flink;
    struct _lst *blink; 
} lstRec, *lstPtr;

/* link-pair for managing a circular doubly linked list of strips
 */
typedef struct _striplst {
  stripRec  *flink;
  stripRec  *blink; 
} stripLstRec, *stripLstPtr;

#endif /* end _XIE_FLODATA */
