/* $XConsortium: mpgeom.h,v 1.1 93/07/19 10:17:45 rws Exp $ */
/**** module mpgeom.h ****/
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
  
	mpgeom.h -- DDXIE MI prototype Geometry definitions
  
	Ben Fahy -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#ifndef _XIEH_MPGEOM
#define _XIEH_MPGEOM

/*
 * peTex extension for the Geometry element
 */

#ifndef old_stuff
typedef struct _mpgeombanddef {
  	int 	finished;	/* done with this band */
	int	yOut;		/* what output line we are on */
	int	out_of_bounds;	/* if entire input image is missed */

	double	first_mlow,	/* lowest  input line mapped by first output */
		first_mhigh;	/* highest input line mapped by first output */
	double	last_mlow,	/* lowest  input line mapped by last  output */
		last_mhigh;	/* highest input line mapped by last  output */
	double	global_mlow,	/* lowest  input line mapped, overall 	     */
		global_mhigh;	/* highest input line mapped, overall 	     */

	int	first_ilow,	/* rounded first_mlow   */
		first_ihigh;	/* rounded first_mhigh  */
	int	last_ilow,	/* rounded last_mlow    */
		last_ihigh;	/* rounded last_mlow    */
	int	global_ilow,	/* rounded global_mlow  */
		global_ihigh;	/* rounded global_mlow  */

	/* useful data precalculated for scaling */
	int	*x_locs;
	int	x_start;
	int	x_end;
	int	int_constant;	/* precalculated for Constrained data fill */
	RealPixel flt_constant;	/* precalculated for UnConstrained data fill */

	/* required by general line filling routine */
	int	in_width;
	int	in_height;

	/* keep track of what input lines we've come across */
	int     lo_src_available;
	int     hi_src_available;

	void    (*linefunc) ();
	void    (*fillfunc) ();
  }
  mpGeometryBandRec, *mpGeometryBandPtr;

typedef struct _mpgeometrydef {
  	int	input_line_increases_as_output_line_increases;
	int	input_map_size;
  	mpGeometryBandPtr bandInfo[xieValMaxBands];
  }
  mpGeometryDefRec, *mpGeometryDefPtr;
#else
typedef struct _mpgeometrydef {
  int 	started[xieValMaxBands];
  int 	lastY[xieValMaxBands];
  int   n_instrips[xieValMaxBands];
  int   first_valid[xieValMaxBands];
  int   last_valid[xieValMaxBands];
  stripPtr 
	*instrip_array[xieValMaxBands], 
	out_strip[xieValMaxBands];
  int	
	pending_strip_low[ xieValMaxBands],
  	pending_strip_high[xieValMaxBands],
  	current_strip_low[ xieValMaxBands],
  	current_strip_high[xieValMaxBands],
  	saved_strip_low[   xieValMaxBands],
  	saved_strip_high[  xieValMaxBands];
  unsigned char	**iline_data[xieValMaxBands]; 
	/* for a given line of input and a given band, point to start
	   of line data */
} 
  mpGeometryDefRec, *mpGeometryDefPtr;

/*** nasty macros for mpgeom.c ***/
#define CreateOrAccessOutputStrip() { 				\
	  if (!pvt->started[b]) {				\
	    if (!(out_strip = MakeStrip(flo, 			\
		&ped->outFlo.format[b],				\
		pvt->lastY[b],					\
		in_strip->unitCnt,				\
 		TRUE)))						\
  		    AllocError(flo, ped, return(FALSE));	\
	    pvt->out_strip[b] = out_strip;			\
	    pvt->started[b] = 1;				\
	  } else						\
		out_strip = pvt->out_strip[b];			\
	}

#define UpdatePendingInputStrips(pvt,b) 			\
        if (in_strip->final || src_img_height <= in_start + in_size) 	\
		last_instrip = 1;				\
		/* if this is last strip, stop tracking */	\
	else if (in_start == pvt->pending_strip_low[b]) {	\
	    pvt->pending_strip_low[b] =in_start + in_size;	\
	    pvt->current_strip_low[b] =in_start;		\
	    pvt->current_strip_high[b]=in_end;			\
	}							\
	else {							\
		pvt->finished[b] = 1;				\
      		ImplementationError(flo,ped, return(FALSE));	\
	}

/***	This macro returns TRUE when a line has all the strips	***/
/***	it needs,  meaning that there are no more strips coming ***/
/***	in that will be useful. (the ones available may not be  ***/
/***	useful either,  in which case the line will be filled   ***/
/***	in with a constant value				***/

#define CurrentLineIsReady()     					\
   (last_instrip	|| 	/* there ain't no more 	   */ 	     	\
    pedpvt->first_ilow > (pvt->pending_strip_high[b]+fudge)  ||	        \
			/* current line out of bounds high */ 	    	\
    pedpvt->first_ihigh< (pvt->pending_strip_low[b]-fudge)	        \
			/* current line lower than pending strips */ 	\
    )

#define NextOutputLine() \
  ( ! ((++(pvt->lastY[b]) >= out_strip->format[b].height)))

#define OutputStripFull() 					\
  (pvt->lastY[b]>= out_strip[b].start +out_strip[b].unitCnt)

#define ThisStripIsUseless()					\
    (in_start > pedpvt->global_ihigh+fudge ||			\
    (in_end)  < pedpvt->global_ilow-fudge   ) 
      
#define SaveStrip() {						\
       /* save this strip in our strip reserve */		\
       if ( pvt->first_valid[b] < 0) {				\
       	  pvt->first_valid[b] = 0;				\
       	  pvt->last_valid[b] = 0;				\
	  strip_array[0] = in_strip;				\
       } 							\
       else {							\
          int i = ++(pvt->last_valid[b]);			\
	  strip_array[i] = in_strip;				\
       }							\
    }

#define UpdateRanges() {						\
    /* we are looking to see what range of lines are required */	\
    /* for the next input line. y_in = c*x_out + d*y_out + ty */	\
    /* if the range for y_out=N was (old_low,old_high), then  */ 	\
    /* the range for y_out=N+1 must be (old_low+d,old_high+d) */	\
    /* thus: */								\
    pedpvt->first_mlow  += pedpvt->coeffs[3]; 				\
    pedpvt->first_mhigh += pedpvt->coeffs[3]; 				\
									\
    /* take floor of low value, ceiling of high value */		\
    pedpvt->first_ilow  = (int)pedpvt->first_mlow;			\
    pedpvt->first_ihigh = 1+(int)pedpvt->first_mhigh;			\
									\
    /* now for global limits, if d>0, lower bound is creeping up */	\
    /* otherwise, upper limit is creeping down                   */	\
    if (normal_order) 							\
	pedpvt->global_mlow = pedpvt->first_ilow; 			\
    else								\
	pedpvt->global_mhigh = pedpvt->first_ihigh; 			\
    }

#endif /* old stuff */
#endif /* module _XIEH_MPGEOM */
