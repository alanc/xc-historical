/* $XConsortium: mpgeomnn.c,v 1.1 93/07/19 10:20:06 rws Exp $ */
/**** module mpgeomnn.c ****/
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
  
	mpgeomnn.c -- DDXIE geometry element for handling nearest
			neighbor technique
  
	Ben Fahy && Larry Hare -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/


#define _XIEC_MPGEOM
#define _XIEC_PGEOM

/*
 *  Include files
 */
#include <stdio.h>

/*
 *  Core X Includes
 */
#include <X.h>
#include <Xproto.h>
/*
 *  XIE Includes
 */
#include <XIE.h>
#include <XIEproto.h>
/*
 *  more X server includes.
 */
#include <misc.h>
#include <dixstruct.h>
#include <extnsionst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <element.h>
#include <texstr.h>
#include <xiemd.h>
#include <mpgeom.h>

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeGeomNN();

/*
 *  routines used internal to this module, technique dependent
 */

/* antialias by lowpass using boxcar filter*/
static int CreateGeomNN();
static int InitializeGeomNN();
static int ActivateGeomNN();
static int ResetGeomNN();
static int DestroyGeomNN();

/*
 * DDXIE Geometry entry points
 */
static ddElemVecRec NearestNeighborVec = {
  CreateGeomNN,
  InitializeGeomNN,
  ActivateGeomNN,
  (xieIntProc)NULL,
  ResetGeomNN,
  DestroyGeomNN
  };

static void SL_R(), SL_b(), SL_B(), SL_P(), SL_Q();
static void FL_R(), FL_b(), FL_B(), FL_P(), FL_Q();
static void GL_R(), GL_b(), GL_B(), GL_P(), GL_Q();
static void (*scale_lines[5])() = { SL_R, SL_b, SL_B, SL_P, SL_Q, };
static void (*fill_lines[5])()  = { FL_R, FL_b, FL_B, FL_P, FL_Q, };
static void (*ggen_lines[5])()  = { GL_R, GL_b, GL_B, GL_P, GL_Q, };

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloGeometry *raw = (xieFloGeometry *)ped->elemRaw;
  inFloPtr  insrc= &ped->inFloLst[SRCtag];
  
   switch(raw->sample) {
   case xieValGeomDefault:
   case xieValGeomNearestNeighbor:
	ped->ddVec = NearestNeighborVec;
	break;

   default:
    	return(FALSE);
   }
  return(TRUE);
}                               /* end miAnalyzeGeomNN */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* allocate space for private data */
  return(MakePETex(flo, ped, sizeof(mpGeometryDefRec), NO_SYNC, NO_SYNC));
}                               /* end CreateGeomNN */
/*------------------------------------------------------------------------
---------------------------- free private data . . . ---------------------
------------------------------------------------------------------------*/
static int FreeBandData(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  mpGeometryDefPtr pvt = (mpGeometryDefPtr) (ped->peTex->private);
  mpGeometryBandPtr pvtband;
  inFloPtr  inf = &ped->inFloLst[SRCtag];
  int band;
  
/*
 *  Look for private data to free
 */
  for (band = 0 ; band < inf->bands ; band++) { 
    pvtband = pvt->bandInfo[band];
    if (pvtband != NULL) {

      if (pvtband->x_locs != NULL)
	XieFree(pvtband->x_locs);

      pvt->bandInfo[band] = (mpGeometryBandPtr) XieFree(pvt->bandInfo[band]);
    }
  }
}
/*------------------------------------------------------------------------
/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  mpGeometryDefPtr pvt = (mpGeometryDefPtr) (ped->peTex->private);
  peTexPtr pet = ped->peTex;
  mpGeometryBandPtr pvtband;
  xieFloGeometry *raw = (xieFloGeometry *)ped->elemRaw;
  pGeomDefPtr pedpvt = (pGeomDefPtr)ped->elemPvt; 
  inFloPtr  inf = &ped->inFloLst[SRCtag];
  int band;
  int in_height;
 /*
  * access coordinates for y_in = c * x_out + d * y_out + ty
  */
  double a  = pedpvt->coeffs[0];
  double b  = pedpvt->coeffs[1];
  double c  = pedpvt->coeffs[2];
  double d  = pedpvt->coeffs[3];
  double tx = pedpvt->coeffs[4];
  double ty = pedpvt->coeffs[5];
  int	width =raw->width;
  int	height=raw->height;
  int threshold;
  double left_map,right_map;
  
/*
 *  Initialize parameters for tracking input lines, etc.
 */
  pvt->input_line_increases_as_output_line_increases =
	(pedpvt->coeffs[3] > 0.0);



  for (band = 0 ; band < inf->bands ; band++) { 
    if (pedpvt->do_band[band]) {

        pvt->bandInfo[band] = 
	  (mpGeometryBandPtr) XieMalloc(sizeof(mpGeometryBandRec));
	if (!pvt->bandInfo[band]) {
	   FreeBandData(flo,ped);
  	   AllocError(flo, ped, return(FALSE));
	}
        pvtband = pvt->bandInfo[band];
	bzero (pvtband, sizeof(mpGeometryBandRec));
	pvtband->int_constant = pedpvt->constant[band] + .5;	
	pvtband->flt_constant = (RealPixel) pedpvt->constant[band];	


	pvtband->fillfunc = 
		fill_lines[IndexClass(pet->emitter[band].format->class)]; 
	pvtband->linefunc =
		ggen_lines[IndexClass(pet->emitter[band].format->class)]; 

	pvtband->in_width = inf->format[band].width;
	pvtband->in_height = inf->format[band].height;

	if (c == 0 && b == 0 ) {
	   int x;
	   int in_width = pvtband->in_width;
	   double in_x;
	   int    in_x_coord;
	   /*    For Scaling, can precalculate a lot 	*/

	   if (a == 1 && d == 1) {
	       /* just Cropping, no real resampling to be done */
	   }
	   pvtband->linefunc =
		scale_lines[IndexClass(pet->emitter[band].format->class)]; 

	   pvtband->x_locs = (int *) XieMalloc(width * sizeof(int));
	   if (!pvtband->x_locs) {
	      FreeBandData(flo,ped);
  	      AllocError(flo, ped, return(FALSE));
	   }

	   /*  coordinate of line is   x_in = a * x_out + tx 	*/
	   /*  however, we will map pixel centers to centers,   */
	   /*  so we plug in output pixel location x_out+0.5	*/
	   /*  for output_pixel x_out. Happily, finding the     */
	   /*  nearest pixel centered on the computed input	*/
	   /*  location is then found simply by truncating	*/

	   /* initialize to nonsense values */
	   pvtband->x_start = in_width;
	   pvtband->x_end   = -1;
	   pvtband->out_of_bounds = 1;

	   in_x = 0.5*a + tx;	/* location of center  */
	   in_x_coord = in_x;	/* closest input pixel */
	   if (in_x_coord >= 0 && in_x_coord < in_width) {
	   	pvtband->x_start = 0;
	   	pvtband->x_end   = 0;
		pvtband->x_locs[0] =  in_x_coord;
	   }

	   for (x=1; x<width; ++x) {
		in_x += a;		/* next center location */
	   	in_x_coord = in_x;	/* closest input pixel */
	        if (in_x_coord >= 0 && in_x_coord < in_width) {
		   /* this pixel is useful */
		   if (pvtband->x_start == in_width)
			pvtband->x_start = x;
		   pvtband->x_end = x;
		   pvtband->x_locs[x] = in_x_coord;
		}
	   }
	}

 /*
  * we need to compute the initial input line
  * number ranges for the entire output image and the first output line.
  * Just for fun, we will also compute the range for the last output line.
  * How these limits are computed, of course, may depend on techniqu;
  */

 /*
  * For nearest neighbor, we consider lines to have both width and height.
  * Thus, if an image is of area w x h, we picture the image as being:
  *
  *         x=0                         w-1  w
  *   y=0    ________________________________
  *          |   |   |   |   |  ...  |   |   |
  *   y=1    ---------------------------------
  *
  *
  *   y=h-1  ________________________________
  *          |   |   |   |   |  ...  |   |   |
  *   y=h    ---------------------------------
  *
  * The first pixel on the first output line is therefore (.5,.5) and
  * the last  pixel on the  last output line is (w-.5,w-.5), etc.  We 
  * compute the input line ranges by seeing where the four corners map,
  * and selecting the coordinates of the pixel whose *center* maps most
  * closely.
  *
  */

       /*
        * first line of output image
        */
        left_map  = c * 0.5         + d * 0.5 + ty;
        right_map = c * (width-0.5) + d * 0.5 + ty;
        pvtband->first_mlow  = (left_map <= right_map)? left_map : right_map;
        pvtband->first_mhigh = (left_map >= right_map)? left_map : right_map;

       /*
        * last line of output image
        */
        left_map  = c * 0.5         + d * (height-0.5) + ty;
        right_map = c * (width-0.5) + d * (height-0.5) + ty;
        pvtband->last_mlow  = (left_map <= right_map)? left_map : right_map;
        pvtband->last_mhigh = (left_map >= right_map)? left_map : right_map;

        pvtband->global_mlow  =  (pvtband->first_mlow  <= pvtband->last_mlow)? 
		pvtband->first_mlow  :  pvtband->last_mlow;

        pvtband->global_mhigh =  (pvtband->first_mhigh >= pvtband->last_mhigh)? 
		pvtband->first_mhigh :  pvtband->last_mhigh;


       /* 
        *  coordinates with center closest are just truncated doubles
        */
        pvtband->last_ilow  = pvtband->last_mlow;
        pvtband->last_ihigh = pvtband->last_mhigh;
        pvtband->first_ilow  = pvtband->first_mlow;
        pvtband->first_ihigh = pvtband->first_mhigh;
        pvtband->global_ilow  = pvtband->global_mlow;
        pvtband->global_ihigh = pvtband->global_mhigh;
	{
	int first_map_size = pvtband->first_ihigh - pvtband->first_ilow + 1;
	if (first_map_size > pvt->input_map_size)
		pvt->input_map_size = first_map_size;
	}

    }   /* end of if do_band[] */
  }	/* end of for loop */

/* 
 * Now adjust thresholds so we won't be called until all the data
 * for the first input line is ready
 */
  for (band = 0 ; band < inf->bands ; band++) { 
    bandPtr iband = &(pet->receptor[SRCtag].band[band]);
    if (pedpvt->do_band[band]) {

        pvtband = pvt->bandInfo[band];

	/* set threshold so we get all needed src lines */
 	threshold = pvtband->first_ihigh + 1;
		/* if we need line 256, must ask for 257! */

	/* make sure we get something */
	if (threshold < 1)
	    threshold = 1;

	/* but don't ask for stuff we can't ever get! */
	if (threshold > inf->format[band].height)
	    threshold = inf->format[band].height;

	if(!InitBand(flo, ped, iband,
		     inf->format[band].height, threshold, NO_INPLACE))
	    return(FALSE);
    } else {
      /* we're suppose to pass this band thru unscathed */
      BypassSrc(flo,pet,iband);
    }
  }
  return(raw->bandMask ? InitEmitter(flo,ped,NO_DATAMAP,NO_INPLACE) : TRUE);
}                               /* end InitializeGeomNN */
/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  peTexPtr pet = ped->peTex;
  pGeomDefPtr pedpvt = (pGeomDefPtr)ped->elemPvt; 
  mpGeometryDefPtr pvt = (mpGeometryDefPtr) ped->peTex->private;
  bandPtr oband = &(pet->emitter[0]);
  bandPtr iband = &(pet->receptor[SRCtag].band[0]);
  int band, nbands = pet->receptor[SRCtag].inFlo->bands;
  inFloPtr  insrc = &ped->inFloLst[SRCtag];
  xieFloGeometry *raw = (xieFloGeometry *)ped->elemRaw;
  int	width =raw->width;
  int	height=raw->height;
  register void *outp;

  for(band = 0; band < nbands; band++, iband++, oband++) {
  int sline = iband->current;
  mpGeometryBandPtr pvtband = pvt->bandInfo[band];

      if (! pvt->input_line_increases_as_output_line_increases) {

	  /* we're going backwards, which is actually *simpler*, 
	   * because we don't get ANY data until we have ALL data.
	   * Thus, first time through, just map everything we have.
	   */
	   if (!pvtband->yOut)  {
	     if (!MapData(flo,pet,iband,0,0,iband->maxGlobal,KEEP)) { 
      		   ImplementationError(flo,ped, return(FALSE));
	     }
	     pvtband->lo_src_available = 0;
	     pvtband->hi_src_available = iband->maxGlobal-1;
	  }
	   
	   outp = GetCurrentDst(void,flo,pet,oband);
	   while (outp) {
		int lo_in,hi_in;

		/* get range of src lines for this output line */
		lo_in = pvtband->first_ilow;
		hi_in = pvtband->first_ihigh;
			
		if (hi_in < 0) {
		  /* rest of output image is off input image */
	     	  (*pvtband->fillfunc)(outp,iband->dataMap,
			width,pvt,pvtband);
		}
		else if (lo_in > pvtband->in_height) {
		  /* haven't reach input image yet */
	     	  (*pvtband->fillfunc)(outp,iband->dataMap,
			width,pvt,pvtband);
		}
		else {
		   /* Compute output pixels for this line */
		   (*pvtband->linefunc)(outp,iband->dataMap,
		   	width,hi_in,pedpvt,pvt,pvtband);

		   /* now compute highest input line for next oline */
		}

		pvtband->first_mlow  += pedpvt->coeffs[3];
		pvtband->first_mhigh += pedpvt->coeffs[3];
		pvtband->first_ilow  = (int) pvtband->first_mlow ;
		pvtband->first_ihigh = (int) pvtband->first_mhigh;
		pvtband->yOut++;
		outp = GetNextDst(void,flo,pet,oband,TRUE);
	   }
	  /* 2 possible reasons for no more dst: done, or scheduler 
	   * wants us to be nice and give up control to somebody else
	   */
	   if (oband->final)
		DisableSrc(flo,pet,iband,FLUSH);

	   continue;	/* go to the next band*/
       }
      /* 
       * nice normal image progression.  This means that I know when
       * I am done with an input line for the current output line,  I 
       * can purge it,  because it won't be needed for subsequent 
       * output lines.
       */
       else {

       while (!ferrCode(flo)) {		
	  int map_lo;		/* lowest  line mapped by output line */
	  int map_hi;		/* highest line mapped by output line */
          int last_src_line = insrc->format[band].height - 1;
	  int len=2;
	  int threshold;
	  register int i;
   	  Bool ok;
	   /* access current output line */
	   outp = GetDst(void,flo,pet,oband,pvtband->yOut,FLUSH);
	   if (!outp) {
		if (oband->final) {
       		  DisableSrc(flo, pet, iband, FLUSH);
		}
		else  {
		  /* Since we still have to produce more output lines */
		  /* we must have gotten NULL because the strip manager */
		  /* wants us to exit, to give the scheduler a chance  */
		  /* to let some element run 			 */

		  if (iband->current != 0)  {
		    /* we shouldn't free data if we still need the first line
		     */
       		      FreeData(void, flo, pet, iband, iband->current);
		  }
		}
		goto breakout;
	   }

	  sline = pvtband->first_ilow;
	  if (sline < 0)
		sline = 0;

	  map_hi = pvtband->first_ihigh;
	  if (map_hi > last_src_line)
		map_hi = last_src_line;

	  if (map_hi < 0 || sline > last_src_line) {
	     /* whole line is off the input image */
	     /* XXX could reduce arguments to fill func */
	     (*pvtband->fillfunc)(outp,iband->dataMap,width,pvt,pvtband);
	  }
	  else {
	     len = map_hi - sline + 1;
	     

	     if(!(ok  = MapData(flo,pet,iband,sline,sline,len,KEEP)))
		break;

	     if (sline != iband->current) 
      		   ImplementationError(flo,ped, return(FALSE));

	     pvtband->lo_src_available = 0;
	     pvtband->hi_src_available = iband->maxGlobal-1;

	      /***	Compute output pixels for this line ***/
	      (*pvtband->linefunc)(outp,iband->dataMap,
		   width,sline,pedpvt,pvt,pvtband);

	    }

	     /* Now we need to compute the input line range
		for the next destination line. We use the fact
		that since y_in = c * x_out + d*y_out + ty, 
		
		if the range of values for y_in on line y_out=N 
		was (old_low,old_high),   the range of values for
		y_in on line y_out=N+1 must be (old_low+d,old_high+d)
	     */

    	     pvtband->first_mlow  += pedpvt->coeffs[3]; 
    	     pvtband->first_mhigh += pedpvt->coeffs[3];
					
   	     /* have to be careful about -0.5 rounding to 0, not -1 */
	     if (pvtband->first_ilow < 0) {

		if (pvtband->first_mlow < 0)
		   pvtband->first_ilow = -1;
		else
    	           pvtband->first_ilow  = (int)pvtband->first_mlow;	
	     
		if (pvtband->first_mhigh < 0)
		   pvtband->first_ihigh = -1;
		else
    	           pvtband->first_ihigh = (int)pvtband->first_mhigh;
	     } 
	     else {
		/* if ilow was positive before, needn't check for negative */
    	        pvtband->first_ilow  = (int)pvtband->first_mlow;	
    	        pvtband->first_ihigh = (int)pvtband->first_mhigh;
	     }

	     ++pvtband->yOut;						

		if (pvtband->first_ilow > last_src_line) {
	          /* rest of output image is off the input image */
		  /* we will exit after filling output strip */
	          while(outp=GetDst(void,flo,pet,oband,pvtband->yOut,FLUSH)) {

		   /* XXX could reduce arguments to fill func */
		   (*pvtband->fillfunc)(outp,iband->dataMap,width,pvt,pvtband);
		   pvtband->yOut++;
		 }
		   if (oband->final) {
		      /* out of destination lines */
		      DisableSrc(flo, pet, iband, FLUSH);
		    }
		    else  
			goto breakout;
		  /* Be nice and let downstream element eat our data */
		  /* notice we don't free input data, because then the */
		  /* silly scheduler would turn us off */
		}

	     map_hi = pvtband->first_ihigh;
	     if (map_hi > last_src_line)
		map_hi = last_src_line;

	     threshold = map_hi - iband->current + 1;

	     /* make sure we get something */
	     if (threshold <= 1)
	         threshold = 1;

	     /* but don't ask for stuff we can't ever get! */
	     if (threshold > insrc->format[band].height)
	         threshold = insrc->format[band].height;

       	     SetBandThreshold(iband, threshold);
	     if (map_hi >= (int) iband->maxGlobal) {
		/* we need to let someone else generate more data */
		break;
	     }
	   }  /* end of while no flo err */
       /* want to make sure we GetSrc at least once before Freeing */
       if (iband->current)
          FreeData(void, flo, pet, iband, iband->current);
       }  /* end of else on normal order */
breakout:
	;
    }	/* end of band loop */
  return(TRUE);
}                               /* end ActivateGeometry */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  FreeBandData(flo,ped);
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetGeomNN */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyGeomNN(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create = (xieIntProc)NULL;
  ped->ddVec.initialize = (xieIntProc)NULL;
  ped->ddVec.activate = (xieIntProc)NULL;
  ped->ddVec.flush = (xieIntProc)NULL;
  ped->ddVec.reset = (xieIntProc)NULL;
  ped->ddVec.destroy = (xieIntProc)NULL;

  return(TRUE);
}                               /* end DestroyGeomNN */

/**********************************************************************/
/* fill routines */
static void FL_b (OUTP,srcimg,width,pvt,pvtband)
	register void *OUTP, **srcimg;
	register int width;
	mpGeometryDefPtr pvt;
	mpGeometryBandPtr pvtband;
{
	register LogInt constant = (LogInt) pvtband->int_constant;
	register LogInt *outp	= (LogInt *) OUTP;
	register int	i;

	if (constant) constant = ~0;
	/*
	** NOTE: Following code assume filling entire line. Which is
	** currently true.  In the future we may need to abide by
	** bit boundaries. Conversely code for bytes and pairs below
	** could be sped up by doing something similar 
	*/
	width = (width + 31) >> 5;
	for (i=0; i < width; ++i) *outp++ = constant;
}

#define DO_FL(funcname, iotype, CONST)					\
static void funcname (OUTP,srcimg,width,pvt,pvtband)			\
register void *OUTP;							\
register void **srcimg;							\
register int width;							\
mpGeometryDefPtr pvt;							\
mpGeometryBandPtr pvtband;						\
{									\
register iotype constant = (iotype) pvtband->CONST;			\
register iotype *outp	= (iotype *) OUTP;				\
register int	i;							\
	for (i=0; i < width; ++i) *outp++ = constant;			\
}

DO_FL	(FL_R, RealPixel, flt_constant)
DO_FL	(FL_B, BytePixel, int_constant)
DO_FL	(FL_P, PairPixel, int_constant)
DO_FL	(FL_Q, QuadPixel, int_constant)

/**********************************************************************/
/* (x,y) separable routines (eg, scale, mirror_x, mirror_y)  */

static void SL_b (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)
	register void *OUTP, **srcimg;
	register int width,sline;
	mpGeometryDefPtr pvt;
	pGeomDefPtr pedpvt; 
	mpGeometryBandPtr pvtband;
{
	register int	xbeg	= pvtband->x_start;
	register int	xend	= pvtband->x_end;
	register int	*x_locs	= pvtband->x_locs;
	register LogInt constant = (LogInt) pvtband->int_constant;
	register LogInt *src	= (LogInt *) (srcimg[sline]);
	register LogInt *outp	= (LogInt *) OUTP;
	register LogInt outval, M, fill;
	register int	i= 0, w;

	fill = (constant ? ~(LogInt)0 : 0);


	for (w = xbeg >> LOGSHIFT; w > 0; w--, i+=LOGSIZE)  *outp++ = fill;

	if (xbeg & LOGMASK)  {
	    /* XXX Do funny word */
	    outval = BitLeft(fill,LOGSIZE-i);
	    for (i = xbeg, M=LOGBIT(i) ; M && i <= xend ; LOGRIGHT(M), i++)
		if (LOG_tstbit(src,x_locs[i]))
		    outval |= M;
	    if (i > xend) {
	        if (fill) outval |= ~BitLeft(fill,LOGSIZE-i);
		i = (i+LOGMASK) & ~LOGMASK;
	    }
	    *outp++ = outval;
	}

	if ( i <= xend) {
	    w = (xend - i + 1) >> LOGSHIFT;
	    for ( ; w > 0; w--, *outp++ = outval)
		for (outval = 0, M=LOGLEFT ; M ; LOGRIGHT(M), i++)
		    if (LOG_tstbit(src,x_locs[i]))
			    outval |= M;

	    for (outval = 0, M=LOGLEFT; i <= xend; LOGRIGHT(M), i++)
	        if (LOG_tstbit(src,x_locs[i]))
		    outval |= M;

	    if (i & LOGMASK) {
	        if (fill) outval |= ~BitLeft(fill,LOGSIZE-i);
		i = (i+LOGMASK) & ~LOGMASK;
		*outp++ = outval;
	    }
	}
	for ( ; i < width; i += LOGSIZE) *outp++ = fill;
}

#define DO_SL(funcname, iotype, CONST)					\
static void funcname (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)	\
register void *OUTP;							\
register void **srcimg;							\
register int width,sline;						\
pGeomDefPtr pedpvt; 							\
mpGeometryDefPtr pvt;							\
mpGeometryBandPtr pvtband;						\
{									\
register int	xbeg	= pvtband->x_start;				\
register int	xend	= pvtband->x_end;				\
register int	*x_locs	= pvtband->x_locs;				\
register iotype constant = (iotype) pvtband->CONST;			\
register iotype *src	= (iotype *) (srcimg[sline]);			\
register iotype *outp	= (iotype *) OUTP;				\
register int	i;							\
	for (i=0; i <  xbeg; ++i) *outp++ = constant;			\
        for (   ; i <= xend; ++i) *outp++ = src[x_locs[i]];		\
        for (   ; i < width; ++i) *outp++ = constant;			\
}

DO_SL	(SL_R, RealPixel, flt_constant)
DO_SL	(SL_B, BytePixel, int_constant)
DO_SL	(SL_P, PairPixel, int_constant)
DO_SL	(SL_Q, QuadPixel, int_constant)

/**********************************************************************/
/* general routines (should be able to handle any valid map) */

static void GL_b (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)
register void *OUTP;
register void **srcimg;
register int width,sline;
pGeomDefPtr pedpvt; 
mpGeometryDefPtr pvt;
mpGeometryBandPtr pvtband;
{
double 	 b  = 	pedpvt->coeffs[1];
double 	 d  = 	pedpvt->coeffs[3];
double 	 tx = 	pedpvt->coeffs[4];
double 	 ty = 	pedpvt->coeffs[5];
register double a  = pedpvt->coeffs[0];
register double c  = pedpvt->coeffs[2];
register double srcline = c*0.5 + d * pvtband->yOut + ty;
register double srcpix  = a*0.5 + b * pvtband->yOut + tx;
register int 	isrcline,isrcpix;
register LogInt constant, val, M, *ptrIn;
register LogInt *outp	= (LogInt *) OUTP;
register int 	srcwidth  = pvtband->in_width;
register int 	srcheight = pvtband->in_height;
register int 	minline  = pvtband->lo_src_available;
register int 	maxline  = pvtband->hi_src_available;


    	constant = pvtband->int_constant ? ~(LogInt) 0 : 0;
	/* could pull out inner if (constant) */
	M=LOGLEFT; val = constant;
	while ( width > 0 ) { 
	    /* in our coordinate system, truncate does a round */
	    isrcline = srcline;
	    isrcpix  = srcpix;
	    /* prepare for next loop */
	    width--;
	    srcline += c;
	    srcpix  += a;
	    /* if (isrcline,isrcpix) not in src image, fill w/val*/
	    if ( (isrcline >= minline) && (isrcline <= maxline) ) { 
		ptrIn = (LogInt *) srcimg[isrcline];
		if ( (isrcpix >= 0) && (isrcpix < srcwidth) && ptrIn )
		    if (LOG_chgbit(ptrIn,isrcpix,constant))
			val ^= M;
	    }
	    LOGRIGHT(M); if (!M) { *outp++ = val; M=LOGLEFT; val = constant; }
	}
	if (M != LOGLEFT) *outp = val;
}


#define DO_GL(funcname, iotype, CONST)					\
static void funcname (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)	\
register void *OUTP;							\
register void **srcimg;							\
register int width,sline;						\
pGeomDefPtr pedpvt; 							\
mpGeometryDefPtr pvt;							\
mpGeometryBandPtr pvtband;						\
{									\
double 	 b  = 	pedpvt->coeffs[1];					\
double 	 d  = 	pedpvt->coeffs[3];					\
double 	 tx = 	pedpvt->coeffs[4];					\
double 	 ty = 	pedpvt->coeffs[5];					\
register double a  = pedpvt->coeffs[0];					\
register double c  = pedpvt->coeffs[2];					\
register double srcline = c*0.5 + d * pvtband->yOut + ty;		\
register double srcpix  = a*0.5 + b * pvtband->yOut + tx;		\
register int 	isrcline,isrcpix;					\
register iotype constant = (iotype) pvtband->CONST;			\
register iotype *outp	= (iotype *) OUTP;				\
register iotype *ptrIn;							\
register iotype val;							\
/* some variables which describe available input data (for clipping) */	\
register int 	srcwidth  = pvtband->in_width;				\
register int 	minline  = pvtband->lo_src_available;			\
register int 	maxline  = pvtband->hi_src_available;			\
									\
	/* in our coordinate system, truncate does a round */		\
	while ( width > 0 ) { 						\
		isrcline = srcline; 					\
		isrcpix  = srcpix; /* no fpu?, move down in 'if' */	\
		/* prepare for next loop */				\
		width--; 						\
		srcline += c; 						\
		srcpix  += a; 						\
		/* if (isrcline,isrcpix) not in src image, fill w/val*/	\
		val = constant; 					\
		if ( (isrcline >= minline) && (isrcline <= maxline) ) { \
		     ptrIn = (iotype *) srcimg[isrcline];  		\
		     if ( (isrcpix >= 0) &&				\
			  (isrcpix < srcwidth) &&			\
			  ptrIn )					\
			val = ptrIn[isrcpix]; 				\
		}							\
		*outp++ = val; 						\
	}								\
}

DO_GL	(GL_R, RealPixel, flt_constant)
#if 1
DO_GL	(GL_B, BytePixel, int_constant)
#else
/* note: due to the glory of cdefs, there's no reason to take this out! :) */
static void GL_B (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)	
register void *OUTP;						
register void **srcimg;					
register int width,sline;				
pGeomDefPtr pedpvt; 
mpGeometryDefPtr pvt;	
mpGeometryBandPtr pvtband;
{				
double 	 b  = 	pedpvt->coeffs[1];
double 	 d  = 	pedpvt->coeffs[3];
double 	 tx = 	pedpvt->coeffs[4];
double 	 ty = 	pedpvt->coeffs[5];
register double a  = pedpvt->coeffs[0];
register double c  = pedpvt->coeffs[2];
register double srcline = c*0.5 + d * pvtband->yOut + ty;
register double srcpix  = a*0.5 + b * pvtband->yOut + tx;
register int 	isrcline,isrcpix;				
register BytePixel constant = (BytePixel) pvtband->int_constant;		
register BytePixel *outp	= (BytePixel *) OUTP;		
register BytePixel *ptrIn;					
register BytePixel val;					
register int 	srcwidth  = pvtband->in_width;	
register int 	srcheight = pvtband->in_height;
register int 	minline  = pvtband->lo_src_available;
register int 	maxline  = pvtband->hi_src_available;
						
	/* in our coordinate system, truncate does a round */
	while ( width > 0 ) { 					
		isrcline = srcline; 			
		isrcpix  = srcpix; /* no fpu?, move down in 'if' */
		/* prepare for next loop */			
		width--; 				
		srcline += c; 			
		srcpix  += a; 		
		/* if (isrcline,isrcpix) not in src image, fill w/val*/
		val = constant; 				
		if ( (isrcline >= minline) && (isrcline <= maxline) ) { 
		     ptrIn = (BytePixel *) srcimg[isrcline];  	
		     if ( (isrcpix >= 0) &&		
			  (isrcpix < srcwidth) &&
			  ptrIn )		
			val = ptrIn[isrcpix]; 
		}			
		*outp++ = val; 	
	}		
}
#endif

DO_GL	(GL_P, PairPixel, int_constant)
DO_GL	(GL_Q, QuadPixel, int_constant)

/**********************************************************************/
/* end module mpgeomnn.c */
