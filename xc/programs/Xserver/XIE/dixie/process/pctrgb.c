/* $XConsortium: pctrgb.c,v 1.1 93/10/26 10:02:06 rws Exp $ */
/**** module pctrgb.c ****/
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
  
	pctrgb.c -- DIXIE routines for managing the ConvertToRGB element
  
	Dean Verheiden -- AGE Logic, Inc. August 1993
  
*****************************************************************************/

#define _XIEC_PCTRGB

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
#include <extnsionst.h>
#include <dixstruct.h>
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <element.h>
#include <technq.h>
#include <difloat.h>


/*
 *  routines referenced by other modules
 */
peDefPtr	MakeConvertToRGB();
Bool		CopyPConvertToRGBCIE();
Bool		CopyPConvertToRGBYCC();
Bool		CopyPConvertToRGBYCbCr();
Bool		PrepPConvertToRGBCIE();
Bool		PrepPConvertToRGBYCC();
Bool		PrepPConvertToRGBYCbCr();

/*
 *  routines for White Adjust techniques
 */
Bool		CopyPWhiteAdjustNone();
Bool		CopyPWhiteAdjustCIELabShift();
Bool		PrepPWhiteAdjustNone();
Bool		PrepPWhiteAdjustCIELabShift();
/*
 *  routines for Gamut techniques
 */
Bool		CopyPGamut();
Bool		PrepPGamut();

/*
 *  other utility routines
 */

void		copy_floats();
void		swap_floats();

/*
 *  routines internal to this module
 */
static Bool	PrepPConvertToRGB();

/*
 * dixie element entry points
 */
static diElemVecRec pConvertToRGBVec = {
  PrepPConvertToRGB		/* prepare for analysis and execution	*/
  };

/*------------------------------------------------------------------------
------------------ routine: make a ConvertToRBG element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeConvertToRGB(flo,tag,pe)
floDefPtr      flo;
xieTypPhototag tag;
xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloConvertToRGB);
  ELEMENT_AT_LEAST_SIZE(xieFloConvertToRGB);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo,tag,xieElemConvertToRGB, return(NULL)) ;

  ped->diVec	     = &pConvertToRGBVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloConvertToRGB *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src,        raw->src);
    cpswaps(stuff->colorspace, raw->colorspace);
    cpswaps(stuff->lenParams,  raw->lenParams);
  } else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloConvertToRGB));
  /*
   * copy technique data (if any)
   */
  if(!(ped->techVec = FindTechnique(xieValConvertToRGB, raw->colorspace)) ||
     !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,xieValConvertToRGB,raw->colorspace,raw->lenParams,
		   return(ped));

 /*
   * assign phototag to inFlo
   */
  ped->inFloLst[SRCtag].srcTag = raw->src;

  return(ped);
}                               /* end MakePConvertToRGB */

/*------------------------------------------------------------------------
------ routine: copy routine for CIELab and CIEXYZ techniques  -----------
------------------------------------------------------------------------*/
Bool CopyPConvertToRGBCIE(flo, ped, sparms, rparms, tsize) 
floDefPtr  flo;
peDefPtr   ped;
xieTecCIELabToRGB *sparms, *rparms;     /* same as xieTecCIEXYZToRGB */
CARD16	tsize;
{
   pTecCIEToRGBDefPtr pvt;

   VALIDATE_TECHNIQUE_SIZE(ped->techVec, tsize, FALSE);

   if (!(ped->techPvt = (pointer )XieMalloc(sizeof(pTecCIEToRGBDefRec))))
	FloAllocError(flo, ped->phototag,xieElemConvertToRGB, return(TRUE));

   pvt = (pTecCIEToRGBDefPtr)ped->techPvt;

   if( flo->reqClient->swapped ) {
	swap_floats(&pvt->matrix[0], &sparms->matrix00, 9);
	cpswaps(sparms->whiteAdjusted,  pvt->whiteAdjusted);
	cpswaps(sparms->numWhiteParams, pvt->numWhiteParams);
	cpswaps(sparms->gamutTechnique, pvt->gamutTechnique);
	cpswaps(sparms->numGamutParams, pvt->numGamutParams);
   } else {
	copy_floats(&pvt->matrix[0], &sparms->matrix00, 9);
        pvt->whiteAdjusted  = sparms->whiteAdjusted;
        pvt->numWhiteParams = sparms->numWhiteParams;
        pvt->gamutTechnique = sparms->gamutTechnique;
        pvt->numGamutParams = sparms->numGamutParams;
   }

   if(!(pvt->whiteTec = FindTechnique(xieValWhiteAdjust, pvt->whiteAdjusted)) ||
      !(pvt->whiteTec->copyfnc(flo, ped, &sparms[1], pvt->whitePoint, 
                                  pvt->whiteTec, pvt->numWhiteParams, 
				  pvt->whiteAdjusted == xieValDefault)))
       TechniqueError(flo,ped,xieValWhiteAdjust,
		      pvt->whiteAdjusted,pvt->numWhiteParams, return(TRUE));

   if(!(pvt->gamutTec = FindTechnique(xieValGamut, pvt->gamutTechnique)) ||
      !(pvt->gamutTec->copyfnc(pvt->numGamutParams)))
       TechniqueError(flo,ped,xieValGamut,
		      pvt->gamutTechnique,pvt->numGamutParams, return(TRUE));

   return (TRUE);
}

/*------------------------------------------------------------------------
---------- routine: copy routine for YCbCr and YCC techniques ------------
------------------------------------------------------------------------*/
Bool CopyPConvertToRGBYCbCr(flo, ped, sparms, rparms, tsize) 
floDefPtr  flo;
peDefPtr   ped;
xieTecYCbCrToRGB *sparms, *rparms;
CARD16	tsize;
{
   pTecYCbCrToRGBDefPtr pvt;

   VALIDATE_TECHNIQUE_SIZE(ped->techVec, tsize, FALSE);

   if (!(ped->techPvt = (pointer )XieMalloc(sizeof(pTecYCbCrToRGBDefRec))))
	FloAllocError(flo, ped->phototag,xieElemConvertToRGB, return(TRUE));

   pvt = (pTecYCbCrToRGBDefPtr)ped->techPvt;

   if( flo->reqClient->swapped ) {
	cpswapl(sparms->levels0, pvt->levels0);
	cpswapl(sparms->levels1, pvt->levels1);
	cpswapl(sparms->levels2, pvt->levels2);
	swap_floats(&pvt->red, &sparms->lumaRed, 3);
	swap_floats(&pvt->bias0, &sparms->bias0, 3);
	cpswaps(sparms->gamutTechnique, pvt->gamutTechnique);
	cpswaps(sparms->numGamutParams, pvt->numGamutParams);
   } else {
	pvt->levels0 = sparms->levels0;
	pvt->levels1 = sparms->levels1;
	pvt->levels2 = sparms->levels2;
	copy_floats(&pvt->red, &sparms->lumaRed, 3);
	copy_floats(&pvt->bias0, &sparms->bias0, 3);
        pvt->gamutTechnique = sparms->gamutTechnique;
        pvt->numGamutParams = sparms->numGamutParams;
   }

   if(!(pvt->gamutTec = FindTechnique(xieValGamut, pvt->gamutTechnique)) ||
      !(pvt->gamutTec->copyfnc(pvt->numGamutParams)))
       TechniqueError(flo,ped,xieValGamut,
		      pvt->gamutTechnique,pvt->numGamutParams, return(TRUE));

   return (TRUE);
}

Bool CopyPConvertToRGBYCC(flo, ped, sparms, rparms, tsize) 
floDefPtr  flo;
peDefPtr   ped;
xieTecYCCToRGB *sparms, *rparms;
CARD16	tsize;
{
   pTecYCCToRGBDefPtr pvt;

   VALIDATE_TECHNIQUE_SIZE(ped->techVec, tsize, FALSE);

   if (!(ped->techPvt = (pointer )XieMalloc(sizeof(pTecYCCToRGBDefRec))))
	FloAllocError(flo, ped->phototag,xieElemConvertToRGB, return(TRUE));

   pvt = (pTecYCCToRGBDefPtr)ped->techPvt;

   if( flo->reqClient->swapped ) {
	cpswapl(sparms->levels0, pvt->levels0);
	cpswapl(sparms->levels1, pvt->levels1);
	cpswapl(sparms->levels2, pvt->levels2);
	swap_floats(&pvt->red, &sparms->lumaRed, 3);
	pvt->scale = ConvertFromIEEE(lswapl(sparms->scale));
	cpswaps(sparms->gamutTechnique, pvt->gamutTechnique);
	cpswaps(sparms->numGamutParams, pvt->numGamutParams);
   } else {
	pvt->levels0 = sparms->levels0;
	pvt->levels1 = sparms->levels1;
	pvt->levels2 = sparms->levels2;
	copy_floats(&pvt->red, &sparms->lumaRed, 3);
	pvt->scale = ConvertFromIEEE(sparms->scale);
        pvt->gamutTechnique = sparms->gamutTechnique;
        pvt->numGamutParams = sparms->numGamutParams;
   }

   if(!(pvt->gamutTec = FindTechnique(xieValGamut, pvt->gamutTechnique)) ||
      !(pvt->gamutTec->copyfnc(pvt->numGamutParams)))
       TechniqueError(flo,ped,xieValGamut,
		      pvt->gamutTechnique,pvt->numGamutParams, return(TRUE));

   return (TRUE);
}
/*------------------------------------------------------------------------
-- routine: copy routine for White Adjust None technique ----------
------------------------------------------------------------------------*/
Bool CopyPWhiteAdjustNone(flo, ped, sparms, pvtf, tv, tsize, isDefault) 
floDefPtr  flo;
peDefPtr   ped;
pointer sparms;
double *pvtf;
techVecPtr tv;
CARD16	tsize;
Bool isDefault;
{
   return (tsize == 0);
}

/*------------------------------------------------------------------------
-- routine: copy routine for White Adjust CIELabShift technique ----------
------------------------------------------------------------------------*/
Bool CopyPWhiteAdjustCIELabShift(flo, ped, sparms, pvtf, tv, tsize, isDefault) 
floDefPtr  flo;
peDefPtr   ped;
xieTecWhiteAdjustCIELabShift *sparms;
double *pvtf;
techVecPtr tv;
CARD16	tsize;
Bool isDefault;
{
   VALIDATE_TECHNIQUE_SIZE(tv, tsize, isDefault);

   if( flo->reqClient->swapped ) {
	swap_floats(pvtf, &sparms->whitePoint0, 3);
   } else {
	copy_floats(pvtf, &sparms->whitePoint0, 3);
   }

   return (TRUE);
}

/*------------------------------------------------------------------------
----------- routine: copy routine for Gamut techniques -------------------
------------------------------------------------------------------------*/
Bool CopyPGamut(tsize) 
CARD16	tsize;
{
   return (tsize == 0);
}

/*------------------------------------------------------------------------
-- routine: prep routine for RGB to CIElab and CIEXYZ techniques ---------
------------------------------------------------------------------------*/
Bool PrepPConvertToRGBCIE(flo, ped, raw, tec) 
floDefPtr  flo;
peDefPtr   ped;
xieFloConvertToRGB *raw;
xieTecCIELabToRGB *tec;		/* same as xieTecCIEXYZToRGB */
{
  pTecCIEToRGBDefPtr pvt = (pTecCIEToRGBDefPtr)ped->techPvt;
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  int b;

  /* grab a copy of the src attributes and propagate them to our input */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; b++) {
	if (IsConstrained(src->format[0].class))
	    return FALSE;	/* must be floats */
	/* Since we know its floats, structure copy sufficient */
	dst->format[b] = inf->format[b] = src->format[b];
  }

  return(pvt->whiteTec->prepfnc(flo,ped,pvt->whitePoint));
}

/*------------------------------------------------------------------------
-- routine: prep routine for RGB to YCbCr and YCC techniques -------------
------------------------------------------------------------------------*/
Bool PrepPConvertToRGBYCbCr(flo, ped, raw, tec) 
floDefPtr  flo;
peDefPtr   ped;
xieFloConvertToRGB *raw;
xieTecYCbCrToRGB *tec;
{
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  pTecYCbCrToRGBDefPtr pvt = (pTecYCbCrToRGBDefPtr)ped->techPvt;
  CARD32 *levels = &(pvt->levels0);
  int b;

  /* grab a copy of the src attributes and propagate them to our input */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; b++) {
	dst->format[b] = inf->format[b] = src->format[b];
	if (IsConstrained(dst->format[b].class))
	    dst->format[b].levels = *(levels+b);
  }

  /* Set depth, class, stride, and pitch */
  if (IsConstrained(dst->format[0].class)) {
      if (UpdateFormatfromLevels(ped) == FALSE)
	MatchError(flo,ped, return(FALSE));
  } /* else the structure copy was sufficient */

  return(TRUE);
}

Bool PrepPConvertToRGBYCC(flo, ped, raw, tec) 
floDefPtr  flo;
peDefPtr   ped;
xieFloConvertToRGB *raw;
xieTecYCCToRGB *tec;
{
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  pTecYCCToRGBDefPtr pvt = (pTecYCCToRGBDefPtr)ped->techPvt;
  CARD32 *levels = &(pvt->levels0);
  int b;

  /* grab a copy of the src attributes and propagate them to our input */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; b++) {
	dst->format[b] = inf->format[b] = src->format[b];
	if (IsConstrained(dst->format[b].class))
	    dst->format[b].levels = *(levels+b);
  }

  /* Set depth, class, stride, and pitch */
  if (IsConstrained(dst->format[0].class)) {
      if (UpdateFormatfromLevels(ped) == FALSE)
	MatchError(flo,ped, return(FALSE));
  } /* else the structure copy was sufficient */

  if (pvt->scale < .001) { /* should be about 1.35 or 1.4 */
	ValueError(flo, ped, tec->scale, return(FALSE));
  }

  return(TRUE);
}

/*------------------------------------------------------------------------
------ routine: prep routine for White Adjust none technique -------------
------------------------------------------------------------------------*/
Bool PrepPWhiteAdjustNone(flo, ped, pwp) 
floDefPtr  flo;
peDefPtr   ped;
double 	  *pwp;
{
  return(TRUE);
}

/*------------------------------------------------------------------------
-- routine: prep routine for White Adjust CIELabShift technique ----------
------------------------------------------------------------------------*/
Bool PrepPWhiteAdjustCIELabShift(flo, ped, pwp) 
floDefPtr  flo;
peDefPtr   ped;
double 	  *pwp;
{
  return(TRUE);
}

/*------------------------------------------------------------------------
------------ routine: prep routine for gamut techniques ------------------
------------------------------------------------------------------------*/
Bool PrepPGamut() 
{
  return(TRUE);
}

/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/

static Bool PrepPConvertToRGB(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  xieFloConvertToRGB *raw = (xieFloConvertToRGB *)ped->elemRaw;

  /* Input must be triple band and dimensions must match */
  if (IsntCanonic(src->format[0].class) ||
      src->bands != 3 ||
      src->format[0].width  != src->format[1].width ||
      src->format[1].width  != src->format[2].width ||
      src->format[0].height != src->format[1].height ||
      src->format[1].height != src->format[2].height)
      MatchError(flo,ped, return(FALSE));


  /*
  ** Technique Prep routine will complete the normal propagation of src
  ** attributes, and setup of destination attributes.
  */

  if (!(ped->techVec->prepfnc(flo, ped, raw, &raw[1])))
	TechniqueError(flo,ped,xieValConvertToRGB,
		       raw->colorspace,raw->lenParams, return(FALSE));


  return (TRUE);
}	

/*------------------------------------------------------------------------
---------------------- utility routines for parameters -------------------
------------------------------------------------------------------------*/

void
swap_floats(doubles_out, funny_floats_in, cnt)
   double	*doubles_out;
   xieTypFloat	*funny_floats_in;
   int		cnt;
{
   int m;
   for (m = 0; m < cnt; m++)		
	doubles_out[m] = ConvertFromIEEE(lswapl(funny_floats_in[m]));
}

void
copy_floats(doubles_out, funny_floats_in, cnt)
   double	*doubles_out;
   xieTypFloat	*funny_floats_in;
   int		cnt;
{
   int m;
   for (m = 0; m < cnt; m++)		
	doubles_out[m] = ConvertFromIEEE(funny_floats_in[m]);
}

/* end module pctrgb.c */