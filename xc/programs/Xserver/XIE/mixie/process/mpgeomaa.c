/* $XConsortium: mpgeomaa.c,v 1.1 93/07/19 10:19:51 rws Exp $ */
/**** module mpgeomaa.c ****/
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
  
	mpgeomaa.c -- DDXIE element for handling antialias
			geometry technique
  
	Ben Fahy && Larry Hare -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/


#define _XIEC_MPGEOM
#define _XIEC_PGEOM

/*
 *  Include files
 */
#include <stdio.h>

#ifndef XoftWare
#define XoftWare
#endif

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
int	miAnalyzeGeomAA();

/*
 *  routines used internal to this module, technique dependent
 */

/* antialias by lowpass using boxcar filter*/
static int CreateGeomAA();
static int InitializeGeomAA();
static int ActivateGeomAA();
static int ResetGeomAA();
static int DestroyGeomAA();

static void DoFakeLine();
static void DoScaleLine();
static void DoCropLine();

/*
 * DDXIE Geometry entry points
 */
static ddElemVecRec AntiAliasVec = {
  CreateGeomAA,
  InitializeGeomAA,
  ActivateGeomAA,
  (xieIntProc)NULL,
  ResetGeomAA,
  DestroyGeomAA
  };

static void SL_R(), SL_b(), SL_B(), SL_P(), SL_Q();
static void FL_R(), FL_b(), FL_B(), FL_P(), FL_Q();
static void GL_R(), GL_b(), GL_B(), GL_P(), GL_Q();
static void (*scale_lines[5])() = { SL_R, SL_b, SL_B, SL_P, SL_Q, };
static void (*fill_lines[5])()  = { FL_R, FL_b, FL_B, FL_P, FL_Q, };
static void (*ggen_lines[5])()  = { GL_R, GL_b, GL_B, GL_P, GL_Q, };

typedef struct _bounding_rect {
	double 	xmin,xmax,ymin,ymax;
} brect;

typedef struct _mpaabanddef {
  	int 	finished;	/* done with this band */
	int	yOut;		/* what output line we are on */
	int	out_of_bounds;	/* if entire input image is missed */

	double	first_mlow,	/* lowest  input line mapped by first output */
		first_mhigh;	/* highest input line mapped by first output */

	int	first_ilow,	/* rounded first_mlow   */
		first_ihigh;	/* rounded first_mhigh  */

	brect 	left_br;	/* bounding rectangle, left  side */

	/* useful data precalculated for scaling */
	int	*ixmin;
	int	*ixmax;

/*** XXX - nuke these  after bit routines rewritten ***/
	int	*x_locs;
	int	x_start;
	int	x_end;

	int	int_constant;	/* precalculated for Constrained data fill */
	RealPixel flt_constant;	/* precalculated for UnConstrained data fill */

	/* required by general line filling routine */
	int	in_width;
	int	in_height;

	/* keep track of what input lines we've come across */
	int	lo_src_available;
	int	hi_src_available;

	/* need these to do any work! */
	void    (*linefunc) ();
	void    (*fillfunc) ();
  }
  mpAntiAliasBandRec, *mpAntiAliasBandPtr;

typedef struct _mpaadef {
  	int	input_line_increases_as_output_line_increases;
  	int	a_not_zero;
  	int	b_not_zero;
  	int	c_not_zero;
  	int	d_not_zero;
  	mpAntiAliasBandPtr bandInfo[xieValMaxBands];
  }
  mpAntiAliasDefRec, *mpAntiAliasDefPtr;
/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeGeomAA(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloGeometry *raw = (xieFloGeometry *)ped->elemRaw;
  inFloPtr  insrc= &ped->inFloLst[SRCtag];
  
   switch(raw->sample) {
   case xieValGeomAntialias:
	ped->ddVec = AntiAliasVec;
	break;

   default:
    	return(FALSE);
   }
  return(TRUE);
}                               /* end miAnalyzeGeomAA */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateGeomAA(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* allocate space for private data */
  return(MakePETex(flo, ped, sizeof(mpAntiAliasDefRec), NO_SYNC, NO_SYNC));
}                               /* end CreateGeomAA */
/*------------------------------------------------------------------------
---------------------------- free private data . . . ---------------------
------------------------------------------------------------------------*/
static int FreeBandData(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  mpAntiAliasDefPtr pvt = (mpAntiAliasDefPtr) (ped->peTex->private);
  mpAntiAliasBandPtr pvtband;
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
      if (pvtband->ixmin != NULL)
	XieFree(pvtband->ixmin);
      if (pvtband->ixmax != NULL)
	XieFree(pvtband->ixmax);

      pvt->bandInfo[band] = (mpAntiAliasBandPtr) XieFree(pvt->bandInfo[band]);
    }
  }
}
/*------------------------------------------------------------------------
/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeGeomAA(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  mpAntiAliasDefPtr pvt = (mpAntiAliasDefPtr) (ped->peTex->private);
  peTexPtr pet = ped->peTex;
  mpAntiAliasBandPtr pvtband;
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
  double upper_left_x,upper_left_y;
  double upper_rite_x,upper_rite_y;
  double lower_left_x,lower_left_y;
  double lower_rite_x,lower_rite_y;
  double left_xmin,left_xmax,left_ymin,left_ymax;
  double rite_xmin,rite_xmax,rite_ymin,rite_ymax;
  double xmin,ymin,xmax,ymax;
  
/*
 *  Initialize parameters for tracking input lines, etc.
 */
  pvt->input_line_increases_as_output_line_increases =
	(pedpvt->coeffs[3] > 0.0);

  pvt->a_not_zero = (a != 0.0);
  pvt->b_not_zero = (b != 0.0);
  pvt->c_not_zero = (c != 0.0);
  pvt->d_not_zero = (d != 0.0);



  for (band = 0 ; band < inf->bands ; band++) { 
    if (pedpvt->do_band[band]) {

        pvt->bandInfo[band] = 
	  (mpAntiAliasBandPtr) XieMalloc(sizeof(mpAntiAliasBandRec));
	if (!pvt->bandInfo[band]) {
	   FreeBandData(flo,ped);
  	   AllocError(flo, ped, return(FALSE));
	}
        pvtband = pvt->bandInfo[band];
	bzero (pvtband, sizeof(mpAntiAliasBandRec));
	pvtband->int_constant = pedpvt->constant[band] + .5;	
	pvtband->flt_constant = (RealPixel) pedpvt->constant[band];	


	pvtband->fillfunc = 
		fill_lines[IndexClass(pet->emitter[band].format->class)]; 
	pvtband->linefunc =
		ggen_lines[IndexClass(pet->emitter[band].format->class)]; 

	pvtband->in_width = inf->format[band].width;
	pvtband->in_height = inf->format[band].height;

 /*
  * THE BASIC IDEA - 
  *
  * We consider each output pixel as describing an *area* of width 1
  * and height 1.  Pixel (xo,yo) thus refers to the locus of points:
  *
  * LocusOut(xo,yo) = { (x,y) | xo <= x < xo+1, yo <= y < yo + 1 }
  *
  * When we map this area back to the input image, each corner maps
  * according to:
  *
  *	x_in = a * x_out + b * y_out + tx,
  *	y_in = c * x_out + b * y_out + ty.
  *
  * Now let P(xo,yo) be described as the point at relative position
  * (p,q) from (xo,yo) in LocusOut(xo,yo).  In other words, 
  *
  *	P(xo,yo) = (xo+p, yo+q)
  *
  * Let M(p,q;xo,yo) be the mapping in input space of P(xo,yo). Then
  * the x coordinate of M(p,q;xo+1,yo) is:
  *
  *	XCoord[ M(p,q:xo+1,yo) ] = a * (x_o+1+p) + b*yo + tx
  *				 = a * (x_o+p) + b*yo + tx + a*p
  * so
  *
  *	XCoord[ M(p,q:xo+1,yo) ] = XCoord[ M(p,q;xo,yo) ] + a*p
  *
  * similarly, 
  *
  *	YCoord[ M(p,q:xo,yo+1) ] = YCoord[ M(p,q;xo,yo) ] + c*q
  *
  * also,
  *
  *	XCoord[ M(p+1,q:xo,yo) ] = XCoord[ M(p,q;xo,yo) ] + a*p
  *	YCoord[ M(p+1,q:xo,yo) ] = XCoord[ M(p,q;xo,yo) ] + c*p
  *	XCoord[ M(p,q+1:xo,yo) ] = YCoord[ M(p,q;xo,yo) ] + b*q
  *	YCoord[ M(p,q+1:xo,yo) ] = YCoord[ M(p,q;xo,yo) ] + d*q
  *
  * We will use these results to derive a computationally simple
  * antialiasing algorithm.
  *
  * Suppose we are scaling an image down by a factor of four in
  * the X direction:
  *
  *  |  |  |  |**|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
  *  |  |  |  |**|  |  |  |  |**|**|**|**|**|**|**|**|**|**|  |
  *  |  |  |  |**|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
  *
  *       |           |          | ******** | ******** | *****
  *
  *  If we nearest neighbor sample the input image where the 
  *  output pixels map to input locations,  the horizontal line
  *  at the right will show up in the output image, but the vertical
  *  line at left will be completely missed.
  *
  *  The idea of this algorithm is to take the average of all pixels
  *  which are included in the *region* mapped by the output *area*,
  *  as opposed to looking at discrete pixels only.  So we consider
  *  a pixel as representing an output locus
  *
  * 	LocusOut(xo,yo) = { (x,y) | xo <= x < xo+1, yo <= y < yo + 1 }
  *
  *  and the input locus will have corner points:
  *
  *	( XCoord[ M(0,0:xo,yo) ],YCoord[ M(0,0:xo,yo) ] )
  *	( XCoord[ M(1,0:xo,yo) ],YCoord[ M(1,0:xo,yo) ] )
  *	( XCoord[ M(0,1:xo,yo) ],YCoord[ M(0,1:xo,yo) ] )
  *	( XCoord[ M(1,1:xo,yo) ],YCoord[ M(1,1:xo,yo) ] )
  *
  *  If  xi(xo) = XCoord[ M(0,0;xo,yo) ]
  *  and yi(yo) = YCoord[ M(0,0;xo,yo) ],
  *
  *  Then we can rewrite these using the relations above as:
  *
  *  	( xi(xo),     yi(yo) ),  
  *	( xi(xo)+a,   yi(yo)+c ), 
  *  	( xi(xo)+b,   yi(yo)+d),  
  *	( xi(xo)+a+b, yi(yo)+c+d ). 
  * 
  *  We have very little guarantee on what shape this collection
  *  of points will assume,  without agreeing to look for special
  *  cases of (a,b,c,d).  However, we do know that whatever the
  *  locus looks like, there is a "bounding rectangle":
  *
  *	BRect: { (xmin,ymin; xmax,ymax) } 
  *
  *  such that xmin is the greatest lower bound of all x in the input locus,
  *  such that ymin is the greatest lower bound of all y in the input locus,
  *  such that xmax is the least upper bound of all x in the input locus, &
  *  such that ymax is the least upper bound of all y in the input locus.
  *
  *  Furthermore, this bounding rectangle has the very nice property 
  *  of "shift invariance," ie, 
  *
  *  If   { (xmin,ymin; xmax,ymax) } 	is the bounding rectangle for (xo,yo),
  *
  *  Then { (xmin+a*m+b*n,ymin+c*m+d*n; xmax+a*m+b*n,ymax+c*m+d*n) } 
  *					is the bounding rect for (xo+m,yo+n).
  *
  *  Proof.  Let Yo be the ymax coordinate of the bounding rectangle for 
  *  (xo,yo). Let ILo be the input locus of (xo,yo).  Let Yo' be the ymax 
  *  coordinate of (xo+m,yo+n),  and ILo' be the input locus of (xo+m,yo+n).  
  *  We claim Yo' = Yo + c*m+d*n. 
  *
  *  By definition, Yo >= y for all y in ILo, and Yo' >= y for all y in ILo'.  
  *  Suppose Yo' < Yo + c*m+d*n.  Then Yo~ = (Yo' - c*m - d*n) is less than
  *  Yo.  If Yo~ >= y for all y in ILo,  then this contradicts Yo being a
  *  least upper bound.  Therefore there must be some offset (r,s) such 
  *  that Yrs = YCoord[ M(r,s;xo,yo) ] is > Yo~.  But by the translation
  *  rules above, YCoord[ M(r,s; xo+m,yo+n) ] = Yrs + c*m + d*n. Call 
  *  this point Yrs'.  Then:
  *
  *     Yrs' = Yrs + c*m + d*n > Yo~ + c*m + d*n = Yo'
  *
  *  which violates the assumption that Yo' is the least upper bound for
  *  ILo'.   Thus, Yo' >= Yo + c*m + d*n.  It is easy to show that Yo'
  *  greater than Yo + c*m + d*n also leads to a contradiction.  Therefore
  *  we have Yo' identically equal to Yo + c*m + d*n,  and the other 
  *  coordinates (xmin,xmax,ymin) follow by similar reasoning.
  *
  * ----------------------------------------------------------------------
  *
  *  WHY THIS IS USEFUL:
  *
  *  We compute the bounding rectangle for the first pixel on the first
  *  output line. Then it is easy to compute the bounding rectangle for
  *  all other points in the output image.  In particular, we can compute
  *  ymin,ymax values for any line in the output image. We don't bother
  *  decoding any output line until all of the required input image lines
  *  are available.
  *
  *  Once we have the data,  we call a line function that does the real
  *  work.  The line function simply marches through the bounding rect
  *  of each pixel in the output line,  adds up the input image numbers,
  *  and divides by the number of discrete pixels in the bounding rect.
  *  The next bounding rect in the line is easily calculated by the
  *  shift-invariance relation.  Clipping of the bounding rect is used
  *  to avoid integrating nonsense values.  An output pixel is filled
  *  with the constant value only if the *entire* bounding rectangle
  *  is off-image.
  *
  */

/***  Calculate bounding rectangle of first pixel, first output line ***/

       /*
        * The zero's are for clarity, not to slow code down.  :)
	* If your compiler can't optimize away a*0, get a better one!
        */
        upper_left_x  = a * 0  + b * 0  + tx;
        upper_left_y  = c * 0  + d * 0  + ty;
        upper_rite_x  = a * 1  + b * 0  + tx;
        upper_rite_y  = c * 1  + d * 0  + ty;

        lower_left_x  = a * 0  + b * 1  + tx;
        lower_left_y  = c * 0  + d * 1  + ty;
        lower_rite_x  = a * 1  + b * 1  + tx;
        lower_rite_y  = c * 1  + d * 1  + ty;

/***	Bounding rectangle for this pixel is (xmin,ymin; xmax,ymax)  ***/

	xmin = upper_left_x;
	xmin = (upper_rite_x < xmin)? upper_rite_x : xmin;
	xmin = (lower_left_x < xmin)? lower_left_x : xmin;
	xmin = (lower_rite_x < xmin)? lower_rite_x : xmin;

	ymin = upper_left_y;
	ymin = (upper_rite_y < ymin)? upper_rite_y : ymin;
	ymin = (lower_left_y < ymin)? lower_left_y : ymin;
	ymin = (lower_rite_y < ymin)? lower_rite_y : ymin;

	xmax = upper_left_x;
	xmax = (upper_rite_x > xmax)? upper_rite_x : xmax;
	xmax = (lower_left_x > xmax)? lower_left_x : xmax;
	xmax = (lower_rite_x > xmax)? lower_rite_x : xmax;

	ymax = upper_left_y;
	ymax = (upper_rite_y > ymax)? upper_rite_y : ymax;
	ymax = (lower_left_y > ymax)? lower_left_y : ymax;
	ymax = (lower_rite_y > ymax)? lower_rite_y : ymax;

/***	remember  this as left brect for first line  ***/
	pvtband->left_br.xmin = left_xmin = xmin;
	pvtband->left_br.ymin = left_ymin = ymin;
	pvtband->left_br.xmax = left_xmax = xmax;
	pvtband->left_br.ymax = left_ymax = ymax;

/***	compute right brect using translation rules   ***/
	rite_xmin = left_xmin + a*(width-1);
	rite_xmax = left_xmax + a*(width-1);
	rite_ymin = left_ymin + c*(width-1);
	rite_ymax = left_ymax + c*(width-1);

/***	Now compute ymin and ymax for the whole line	***/
	ymin = (left_ymin < rite_ymin) ? left_ymin : rite_ymin;
	ymax = (left_ymax > rite_ymax) ? left_ymax : rite_ymax;

/***	Save them away for using to track input image availability  ***/
        pvtband->first_mlow  = ymin;
        pvtband->first_mhigh = ymax;
        pvtband->first_ilow  = pvtband->first_mlow;
        pvtband->first_ihigh = pvtband->first_mhigh;

/***	Check for some special cases 	***/
	if (c == 0 && b == 0 ) {
	   int i,maxpixl  = pvtband->in_width - 1;

	   if (a == 1 && d == 1) {
	       /* just Cropping, no real resampling to be done */
	   }
	   pvtband->linefunc =
		scale_lines[IndexClass(pet->emitter[band].format->class)]; 

	   pvtband->ixmin = (int *) XieMalloc(width * sizeof(int));
	   if (!pvtband->ixmin) {
	      FreeBandData(flo,ped);
  	      AllocError(flo, ped, return(FALSE));
	   }
	   pvtband->ixmax = (int *) XieMalloc(width * sizeof(int));
	   if (!pvtband->ixmax) {
	      FreeBandData(flo,ped);
  	      AllocError(flo, ped, return(FALSE));
	   }
	   /* precalculate the range of x coordinates to look at   */
	   /* for each output pixel location.  This doesn't change */
	   /* from line to line because b=0			   */
	   for  ( i=0; i<width; ++i) { 
		register int ixmin,ixmax;
	   	ixmin = xmin; 
	   	if (ixmin < 0)	  ixmin = 0;
	   	pvtband->ixmin[i] = ixmin;

	   	ixmax = xmax; 
	   	if (ixmax >= maxpixl) ixmax = maxpixl;
	   	if (ixmax > ixmin)	  ixmax--;
	  	pvtband->ixmax[i] = ixmax;

	   	xmin += a; 
	   	xmax += a; 
	   }
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
}                               /* end InitializeGeomAA */
/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateGeomAA(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  peTexPtr pet = ped->peTex;
  pGeomDefPtr pedpvt = (pGeomDefPtr)ped->elemPvt; 
  mpAntiAliasDefPtr pvt = (mpAntiAliasDefPtr) ped->peTex->private;
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
      mpAntiAliasBandPtr pvtband = pvt->bandInfo[band];

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
	     pvtband->lo_src_available = 0;
	     pvtband->hi_src_available = iband->maxGlobal-1;

	      if (sline != iband->current) {
      		   ImplementationError(flo,ped, return(FALSE));
	       }

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
	   }
       /* want to make sure we GetSrc at least once before Freeing */
       if (iband->current)
          FreeData(void, flo, pet, iband, iband->current);
       }
breakout:
	;
    }	/* end of band loop */
  return(TRUE);
}                               /* end ActivateGeometry */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetGeomAA(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  FreeBandData(flo,ped);
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetGeomAA */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyGeomAA(flo,ped)
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
}                               /* end DestroyGeomAA */

/**********************************************************************/
/* fill routines */
static void FL_b (OUTP,srcimg,width,pvt,pvtband)
	register void *OUTP, **srcimg;
	register int width;
	mpAntiAliasDefPtr pvt;
	mpAntiAliasBandPtr pvtband;
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
mpAntiAliasDefPtr pvt;							\
mpAntiAliasBandPtr pvtband;						\
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
	mpAntiAliasDefPtr pvt;
	pGeomDefPtr pedpvt; 
	mpAntiAliasBandPtr pvtband;
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


	for (w = xbeg >> LOGSHIFT; w > 0; w--, i+=32)  *outp++ = fill;

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
	for ( ; i < width; i += 32) *outp++ = fill;
}

/* NOTE: too bad paper is white is 255, otherwise you can often avoid the
** divide by "if (value) value /= nfound;"
*/
#define nnlinsep_func(funcname, iotype, valtype, CONST)			\
									\
	/* round bounding rectangle limits to ints */			\
	iymin = pvtband->left_br.ymin;					\
	iymax = pvtband->left_br.ymax;					\
									\
	/* clip to available src data */				\
	if (iymin < minline) 	iymin = 0;				\
	if (iymax > maxline) 	iymax = maxline;			\
	if (iymax > iymin)	iymax--;				\
									\
	/* loop through all the output pixels on this line */		\
	for ( i=0; i<width; ++i) { 					\
	    /* loop variables */					\
	    register int ix,iy;						\
									\
	    nfound = 0;     						\
	    value = 0;     						\
	    /* ixmin = pvtband->ixmin[i]; */				\
	    /* ixmax = pvtband->ixmax[i]; */				\
	    ixmin = *ixminptr++;					\
	    ixmax = *ixmaxptr++;					\
									\
	    for (iy=iymin; iy<=iymax; ++iy) {				\
		ptrIn = ixmin + (iotype *) srcimg[iy];  		\
		for (ix=ixmin; ix<=ixmax; ++ix) {			\
		  value += *ptrIn++;		  			\
		  ++nfound;			  			\
		}				  			\
	    } /* end of iy loop */					\
									\
	    if (nfound) { 						\
		value /= nfound;					\
	    } else							\
	        value = constant;					\
									\
	    *outp++ = value;						\
									\
	}   /* end of for loop */					\
									\
	/* before leaving, update bounding rect for next line */	\
	if (pvt->d_not_zero) {						\
	   pvtband->left_br.ymin += d;					\
	   pvtband->left_br.ymax += d;					\
	}								

#define DO_SL(funcname, iotype, valtype, CONST)				\
static void funcname (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)	\
register void *OUTP;							\
register void **srcimg;							\
register int width;							\
int sline;								\
pGeomDefPtr pedpvt; 							\
mpAntiAliasDefPtr pvt;							\
mpAntiAliasBandPtr pvtband;						\
{									\
/* Mapping coefficients */						\
double d  = 	pedpvt->coeffs[3];					\
									\
/* These variables describe the limits of the bounding rectangle */	\
register int *ixminptr=pvtband->ixmin;					\
register int *ixmaxptr=pvtband->ixmax;					\
register int ixmin,iymin,ixmax,iymax;					\
									\
/* some variables which describe available input data (for clipping) */	\
register int 	maxpixl  = pvtband->in_width - 1;			\
register int 	minline  = pvtband->lo_src_available;			\
register int 	maxline  = pvtband->hi_src_available;			\
									\
/* cast of constant, output and input pointers to correct type */	\
register iotype constant = (iotype) pvtband->CONST;			\
register iotype *outp	= (iotype *) OUTP;				\
register iotype *ptrIn;							\
									\
register valtype value; 						\
register int	i,nfound;						\
									\
	nnlinsep_func(funcname, iotype, valtype, CONST)			\
}

DO_SL	(SL_R, RealPixel, RealPixel, flt_constant)
DO_SL	(SL_B, BytePixel, QuadPixel, int_constant)
DO_SL	(SL_P, PairPixel, QuadPixel, int_constant)
DO_SL	(SL_Q, QuadPixel, QuadPixel, int_constant)

/**********************************************************************/
/* general routines (should be able to handle any valid map) */

/* XXX - this routine isn't real, it's just a copy of scale so far */
static void GL_b (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)
	register void *OUTP, **srcimg;
	register int width,sline;
	pGeomDefPtr pedpvt; 
	mpAntiAliasDefPtr pvt;
	mpAntiAliasBandPtr pvtband;
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


	for (w = xbeg >> LOGSHIFT; w > 0; w--, i+=32)  *outp++ = fill;

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
	for ( ; i < width; i += 32) *outp++ = fill;
}

/* NOTE: too bad paper is white is 255, otherwise you can often avoid the
** divide by "if (value) value /= nfound;"
*/
#define some_compilers_are_asinine(funcname, iotype, valtype, CONST)	\
	/* round bounding rectangle limits to ints */			\
	ixmin = xmin;		     					\
	iymin = ymin;     						\
	ixmax = xmax;     						\
	iymax = ymax;     						\
									\
	/* clip to available src data */				\
	if (ixmin < 0) 	 	ixmin = 0;				\
	if (iymin < minline) 	iymin = 0;				\
	if (ixmax > maxpixl) 	ixmax = maxpixl;			\
	if (iymax > maxline) 	iymax = maxline;			\
									\
	/* loop through all the output pixels on this line */		\
	while ( width > 0 ) { 						\
									\
	    xmin += a; 							\
	    xmax += a; 							\
	    nfound = 0;     						\
	    value = 0;     						\
	    for (iy=iymin; iy<=iymax; ++iy) {				\
		ptrIn = (iotype *) srcimg[iy];  			\
		for (ix=ixmin; ix<=ixmax; ++ix) {			\
		  value += ptrIn[ix];		  			\
		  ++nfound;			  			\
		}				  			\
	    } /* end of iy loop */					\
									\
	    if (nfound) { 						\
		value /= nfound;					\
	    } else							\
	        value = constant;					\
									\
	    /* prepare for next loop */					\
	    width--; 							\
	    ixmin = xmin;		     				\
	    ixmax = xmax;     						\
	    if (c_not_zero) {						\
		ymin += c; 						\
		ymax += c; 						\
		iymin = ymin;     					\
		iymax = ymax;     					\
		if (iymin < minline)	iymin = minline;		\
	       	if (iymax >= maxline)	iymax = maxline;		\
		if (iymax > iymin)	iymax--;			\
	    } 								\
		if (ixmin < 0)		ixmin = 0;			\
		if (ixmax >= maxpixl)	ixmax = maxpixl;		\
		if (ixmax > ixmin)	ixmax--;			\
	    *outp++ = value;						\
	}/* end of line loop */						\
									\
	/* before leaving, update bounding rect for next line */	\
	if (pvt->b_not_zero) {						\
	   pvtband->left_br.xmin += b;					\
	   pvtband->left_br.xmax += b;					\
	}								\
	if (pvt->d_not_zero) {						\
	   pvtband->left_br.ymin += d;					\
	   pvtband->left_br.ymax += d;					\
	}								

#define DO_GL(funcname, iotype, valtype, CONST)				\
static void funcname (OUTP,srcimg,width,sline,pedpvt,pvt,pvtband)	\
register void *OUTP;							\
register void **srcimg;							\
register int width,sline;						\
pGeomDefPtr pedpvt; 							\
mpAntiAliasDefPtr pvt;							\
mpAntiAliasBandPtr pvtband;						\
{									\
/* Mapping coefficients */						\
double a  = 	pedpvt->coeffs[0]; 					\
double b  = 	pedpvt->coeffs[1];					\
double c  = 	pedpvt->coeffs[2]; 					\
double d  = 	pedpvt->coeffs[3];					\
int a_not_zero = pvt->a_not_zero;					\
int c_not_zero = pvt->c_not_zero;					\
									\
/* These variables describe the limits of the bounding rectangle */	\
double xmin = 	pvtband->left_br.xmin;					\
double ymin = 	pvtband->left_br.ymin;					\
double xmax = 	pvtband->left_br.xmax;					\
double ymax = 	pvtband->left_br.ymax;					\
register int ixmin,iymin,ixmax,iymax;					\
									\
/* loop variables for roaming through the bounding rectangle */		\
register int 	ix,iy;							\
									\
/* some variables which describe available input data (for clipping) */	\
register int 	maxpixl  = pvtband->in_width - 1;			\
register int 	minline  = pvtband->lo_src_available;			\
register int 	maxline  = pvtband->hi_src_available;			\
									\
/* cast of constant, output and input pointers to correct type */	\
register iotype constant = (iotype) pvtband->CONST;			\
register iotype *outp	= (iotype *) OUTP;				\
register iotype *ptrIn;							\
									\
register valtype value;							\
register int	nfound;							\
									\
	some_compilers_are_asinine(funcname, iotype, valtype, CONST)	\
}

DO_GL	(GL_R, RealPixel, RealPixel, flt_constant)
DO_GL	(GL_B, BytePixel, QuadPixel, int_constant)
DO_GL	(GL_P, PairPixel, QuadPixel, int_constant)
DO_GL	(GL_Q, QuadPixel, QuadPixel, int_constant)

/**********************************************************************/
static void DoCropLine(outp,srcimg,width,sline,fconstant,pvt,pvtband)
register BytePixel *outp;
register BytePixel **srcimg;
register int width,sline;
double fconstant;
mpAntiAliasDefPtr pvt;
mpAntiAliasBandPtr pvtband;
{
	/* let's fake it until we have time to clean up */
	DoFakeLine(outp,srcimg,width,sline,fconstant,pvt,pvtband);
}
/**********************************************************************/
static void DoFakeLine(outp,srcimg,width,sline,fconstant,pvt,pvtband)
register BytePixel *outp;
register BytePixel **srcimg;
register int width,sline;
double fconstant;
mpAntiAliasDefPtr pvt;
mpAntiAliasBandPtr pvtband;
{
register int i;
register BytePixel constant = (BytePixel) pvtband->int_constant;
register BytePixel *src = srcimg[sline];

        for (i=0; i < width; ++i) *outp++ = i%256;
}
/**********************************************************************/
/* end module mpgeomnn.c */
