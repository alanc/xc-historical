/* $XConsortium: strip.c,v 1.1 93/07/19 10:13:27 rws Exp $ */
/**** module strip.c ****/
/*****************************************************************************
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
  
	strip.c -- DDXIE machine independent data flo manager
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_STRIP

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
#include <flostr.h>
#include <element.h>
#include <texstr.h>


/* routines exported to the photoflo manager
 */
int	InitStripManager();

/* routines exported directly to DIXIE
 */
int	DebriefStrips();
void	FreeStrips();

/* routines exported to DIXIE via the photoflo management vector
 */
static int import_data();
static int export_data();
static int query_data();

/* routines exported to elements via the data manager's vector
 */
static CARD8	*make_bytes();
static CARD8	*make_lines();
static Bool	 map_data();
static CARD8	*get_data();
static Bool	 put_data();
static CARD8	*free_data();
static Bool	 pass_strip();
static void	 bypass_src();
static void	 disable_src();

/* routines used internal to this module
 */
static void	 disable_dst();
static stripPtr	 alter_data();
static stripPtr	 contig_data();
static stripPtr	 make_strip();
static stripPtr	 clone_strip();
static bandMsk	 put_strip();
static stripPtr	 free_strip();

/* DDXIE client data manager entry points
 */
static dataVecRec dataManagerVec = {
  import_data,
  export_data,
  query_data
  };

/* DDXIE photoflo manager entry points
 */
static stripVecRec stripManagerVec = {
  make_bytes,
  make_lines,
  map_data,
  get_data,
  put_data,
  free_data,
  pass_strip,
  bypass_src,
  disable_src
  };

INT32  STRIPS = 0; /* DEBUG */
INT32  BYTES  = 0; /* DEBUG */

/*------------------------------------------------------------------------
-------------------------- Initialize Data Manager -----------------------
------------------------------------------------------------------------*/
int InitStripManager(flo)
     floDefPtr flo;
{
  /* plug in the DDXIE client data management vector */
  flo->dataVec = &dataManagerVec;

  /* plug in the strip manager vector */
  flo->stripVec = &stripManagerVec;

  /* init the strip cache */
  ListInit(&flo->floTex->stripHead);

  /* choose the best strip size for this flo (a constant value for now) */
  flo->floTex->stripSize = STANDARD_STRIP_SIZE;

  /* clear the count of strips passed */
  flo->floTex->putCnt = 0;

  return(TRUE);
}                               /* end InitStripManager */


/*------------------------------------------------------------------------
----------- discard parent headers from a whole list of strips -----------
-----------   and copy data if multiple references are found   -----------
----------- then transfer them all to their final destination  -----------
------------------------------------------------------------------------*/
int DebriefStrips(i_head,o_head)
     stripLstPtr i_head;
     stripLstPtr o_head;
{
  stripPtr child, parent;

  /* NOTE: we might want to consider (re)allocing strip buffers to
   *	   strip->length instead of leaving them an strip->bufSiz.
   */
  for(child = i_head->flink; !ListEnd(child,i_head); child = child->flink) {
    while(parent = child->parent)
      if(parent->refCnt == 1) {		/* discard a cloned header	  */
	child->parent = parent->parent;
	XieFree(parent);
	--STRIPS; /*DEBUG*/
      } else {				/* copy multiply referenced data  */
	if(!(child->data = (CARD8*)XieMalloc(child->bufSiz)))
	  return(FALSE);
	bcopy(parent->data, child->data, child->bufSiz);
	child->parent = NULL;		/* de-reference child from parent */
	--parent->refCnt;
	BYTES += child->bufSiz; /*DEBUG*/
      }
    child->format = NULL;		/* kill per-strip format pointer  */
  }
  /* transfer the entire list of input strips to the output
   */
  if(ListEmpty(i_head))
    ListInit(o_head);
  else {
    i_head->flink->blink = (stripPtr)o_head;
    i_head->blink->flink = (stripPtr)o_head;
    *o_head = *i_head;
    ListInit(i_head);
  }
  return(TRUE);
}                               /* end DebriefStrips */


/*------------------------------------------------------------------------
------------------------- Free a whole list of strips --------------------
------------------------------------------------------------------------*/
void  FreeStrips(head)
     stripLstPtr head;
{
  stripPtr strip;

  while( !ListEmpty(head) )
    free_strip(NULL, RemoveMember(strip, head->flink));

}                               /* end FreeStrips */


/*------------------------------------------------------------------------
------------------------ Input from PutClientData ------------------------
------------------------------------------------------------------------*/
static int import_data(flo,ped,band,data,len,final)
     floDefPtr flo;
     peDefPtr  ped;
     CARD8    band;
     CARD8   *data;
     CARD32    len;
     BOOL    final;
{
  peTexPtr    pet =  ped->peTex;
  receptorPtr rcp = &pet->receptor[IMPORT];
  bandPtr     bnd = &rcp->band[band];
  bandMsk     msk = 1<<band;
  stripPtr  strip;

  if(!((rcp->admit | rcp->bypass) & msk))
    return(TRUE);				/* drop unwanted data */

  /* make a strip and fill it in the info from the client
   * (the format info was supplied with the element and technique parameters)
   */
  if(!(strip = make_strip(flo,bnd->format,bnd->maxGlobal,len,len,FALSE)))
    AllocError(flo,ped, return(FALSE));
  strip->final  = final;
  strip->data   = data;
  strip->bufSiz = len;

  if(rcp->bypass & msk) {
    put_strip(flo,pet,strip);			/* pass it downstream */
    if(!strip->flink)
      free_strip(flo,strip);			/* nobody wanted it   */
  } else {
    bnd->maxGlobal  = strip->end + 1;
    bnd->available += len;
    rcp->ready     |= msk;
    InsertMember(strip,bnd->stripLst.blink);
    if(bnd->final = final) {
      if(!(rcp->admit &= ~msk))
	--pet->admissionCnt;
      --pet->floTex->imports;
    }
  }
  /* fire up the scheduler -- then we're outa here */
  return( Execute(flo,pet) );
}                               /* end import_data */


/*------------------------------------------------------------------------
-------------------------- Output for GetClientData ----------------------
------------------------------------------------------------------------*/
static int export_data(flo,ped,band,data,bytes,maxLen,term)
     floDefPtr flo;
     peDefPtr  ped;
     CARD8    band;
     CARD8  **data;
     CARD32 *bytes;
     CARD32 maxLen;
     BOOL     term;
{
  BOOL release = FALSE, final = FALSE;
  stripLstPtr lst = &ped->outFlo.export[band];
  stripPtr  strip;
  peTexPtr  pet;

  if(*bytes = ListEmpty(lst) ? 0 : min(lst->flink->length,maxLen)) {
    strip = lst->flink;
    *data = strip->data + (strip->bitOff>>3);
    if(strip->length -= *bytes) {
      strip->start   += *bytes;
      strip->bitOff  += *bytes<<3;  
    } else {
      final   = strip->final;
      release = TRUE;
      RemoveMember(strip,strip);
      if(ListEmpty(lst))
	ped->outFlo.ready &= ~(1<<band);
    }
  }
  if(term || final) {
    /* shut down this output band */
    ped->outFlo.active &= ~(1<<band);
    ped->outFlo.ready  &= ~(1<<band);
    pet = ped->peTex;
    pet->floTex->exports--;
    if(!final)
      disable_dst(flo,pet,&pet->emitter[band]);
  }
  /* if we took any data, give the scheduler a poke before we leave
   */
  if(*bytes)
    Execute(flo,NULL);

  if(release)
    free_strip(flo,strip);

  return(   ferrCode(flo)      ? xieValExportError
	 : !ped->outFlo.active ? xieValExportDone
	 :  ListEmpty(lst)     ? xieValExportEmpty : xieValExportMore);
}                               /* end export_data */


/*------------------------------------------------------------------------
----------- Query flo elements involved in client data transport ---------
------------------------------------------------------------------------*/
static int query_data(flo,list,pending,available)
     floDefPtr         flo;
     xieTypPhototag **list;
     CARD32       *pending;
     CARD32     *available;
{
  peDefPtr ped;
  pedLstPtr lst = ListEmpty(&flo->optDAG) ? &flo->defDAG : &flo->optDAG;
  CARD32 exdex;
  
  *pending = *available = 0;
  if(!(*list = (xieTypPhototag *)XieMalloc(flo->peCnt * sz_xieTypPhototag)))
    FloAllocError(flo,0,0, return(FALSE));

  /* find all the import elements that need client data */
  for(ped = lst->flink; ped; ped = ped->clink)
    if(ped->flags.putData && ped->peTex->admissionCnt)
      *list[(*pending)++] = ped->phototag;

  /* find all the export elements that have client data */
  exdex = *pending + (*pending & 1);
  for(ped = lst->blink; ped; ped = ped->clink)
    if(ped->flags.getData && ped->outFlo.ready)
      *list[exdex + (*available)++] = ped->phototag;

  return(TRUE);
}                               /* end query_data */


/*------------------------------------------------------------------------
---- make a strip containing the specified number of contiguous bytes ----
------------------------------------------------------------------------*/
static CARD8* make_bytes(flo,pet,bnd,contig,purge)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	bnd;
     CARD32  contig;
     Bool     purge;
{
  stripPtr strip = bnd->stripLst.blink;
  CARD32 limit, size, units;
  Bool avail = (!ListEmpty(&bnd->stripLst) && bnd->current >= strip->start &&
		bnd->current + contig <= strip->start + strip->bufSiz);

  if(purge && !avail && put_data(flo,pet,bnd))
    return(bnd->data = NULL);	    /* force element to suspend processing */
  
  if(_is_global(bnd))
    return(get_data(flo,pet,bnd,contig,FALSE));	 /* "current" is available */

  if(avail) {
    /* extend the available space in our current strip
     */
    limit = bnd->current + contig;
    bnd->available += limit - bnd->maxGlobal;
    bnd->maxGlobal  = limit;
    strip->end      = limit - 1;
    strip->length   = limit - strip->start;
  } else {
    /* time to make a fresh strip
     */
    units = bnd->current + contig - bnd->maxGlobal;
    size  = units + Align(units,flo->floTex->stripSize);
    if(!(strip = make_strip(flo,bnd->format,bnd->maxGlobal,units,size,TRUE)))
      AllocError(flo,pet->peDef, return(NULL));

    bnd->available += strip->length;
    bnd->maxGlobal  = strip->end + 1;
    InsertMember(strip,bnd->stripLst.blink);
  }  
  /* update our bandRec with the results */
  bnd->strip    = strip;
  bnd->minLocal = max(bnd->minGlobal,strip->start);
  bnd->maxLocal = strip->end + 1;
  return(bnd->data = _byte_ptr(bnd));
}                               /* end make_bytes */


/*------------------------------------------------------------------------
--- Find or make a strip containing the unit specified by bnd->current ---
------------------------------------------------------------------------*/
static CARD8* make_lines(flo,pet,bnd,purge)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	bnd;
     Bool     purge;
{
  stripPtr strip;
  formatPtr  fmt;
  CARD32 size, units;
  
  if(purge && _release_ok(bnd) && put_data(flo,pet,bnd))
    return(bnd->data = NULL);	/* force element to suspend processing */
  
  if(_is_global(bnd))	  	/* we already have it, just go find it */
    return(get_data(flo,pet,bnd,1,FALSE));
  
  fmt = bnd->format;
  if(bnd->current >= fmt->height)
    return(NULL);			/* trying to go beyond end of image */
  
  while(bnd->current >= bnd->maxGlobal) {
    /*
     * re-use src if we're allowed to alter the data
     */
    if(!bnd->inPlace || !(strip = alter_data(flo,pet,bnd))) {
      size  = flo->floTex->stripSize;
      units = size / bnd->pitch;
      if(units == 0) {			/* image bigger than standard strip */
	units = 1;
	size  = bnd->pitch;
      } else if(bnd->current + units > fmt->height)	/* at end of image  */
	units = fmt->height - bnd->current;
      strip = make_strip(flo,fmt,bnd->maxGlobal,units,size,TRUE);
    }
    if(!strip)
      AllocError(flo,pet->peDef, return(NULL));
    bnd->available += strip->length;
    bnd->maxGlobal  = strip->end + 1;
    if(bnd->maxGlobal == fmt->height)
      bnd->final = strip->final = TRUE;
    InsertMember(strip,bnd->stripLst.blink);
  }
  /* update our bandRec with the results */
  bnd->strip    = strip;
  bnd->minLocal = max(bnd->minGlobal,strip->start);
  bnd->maxLocal = strip->end + 1;
  return(bnd->data = _line_ptr(bnd));
}                               /* end make_lines */


/*------------------------------------------------------------------------
-------------- load data map with pointers to specified data -------------
------------------------------------------------------------------------*/
static Bool map_data(flo,pet,bnd,map,unit,len,purge)
     floDefPtr  flo;
     peTexPtr   pet;
     bandPtr    bnd;
     CARD32     map,unit,len;
     Bool	purge;
{
  CARD32 line, pitch;
  CARD8 *next, *last, **ptr = bnd->dataMap + map;
  stripPtr strip;

  /* first map the last unit and then the first unit -- if we have to make
   * strips, or if input strips aren't there, we may as well handle it now
   */
  if(len && map + len <= bnd->mapSize)
    if(bnd->isInput) {
      last = GetSrc(CARD8,flo,pet,bnd,unit+len-1,KEEP);
      next = GetSrc(CARD8,flo,pet,bnd,unit,purge);
    } else {
      last = GetDst(CARD8,flo,pet,bnd,unit+len-1,KEEP);
      next = GetDst(CARD8,flo,pet,bnd,unit,purge);
    }
  if(!next || !last)
    return(FALSE);	/* map too small or can't map first and last unit */

  /* now walk through the strips and map all the lines (or bytes!)
   */
  strip = bnd->strip;
  pitch = bnd->pitch;
  line  = unit;
  while((*ptr++ = next) != last)
    if(++line <= strip->end)
      next += pitch;
    else {
      strip = strip->flink;
      next  = strip->data;
    }
  return(TRUE);
}                               /* end map_data */


/*------------------------------------------------------------------------
------ Find the strip containing the unit specified by bnd->current ------
------------------------------------------------------------------------*/
static CARD8* get_data(flo,pet,bnd,contig,purge)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	bnd;
     CARD32  contig;
     Bool     purge;
{
  /* NOTE: get_data assumes that the caller has already verified that the
   *	   beginning of the requested data is available in bnd->stripLst.
   */
  stripPtr strip = bnd->strip ? bnd->strip : bnd->stripLst.flink;
  
  /* first get rid of extra baggage if we can
   */
  if(purge && _release_ok(bnd))
    free_data(flo,pet,bnd);

  strip = bnd->strip ? bnd->strip : bnd->stripLst.flink;
  while(!ListEnd(strip,&bnd->stripLst))
    if(bnd->current > strip->end)
      strip = strip->flink;			/* try the next strip        */
    else if(bnd->current < strip->start)
      strip = strip->blink;			/* try the previous strip    */
    else if(bnd->current+contig-1 <= strip->end ||
	    (strip = contig_data(flo,pet,bnd,strip,contig)))
      break;					/* we found or assembled it  */
    else
      return(NULL);				/* couldn't get enough bytes */

  /* update our bandRec with the results
   */
  bnd->strip    = strip;
  bnd->minLocal = max(bnd->minGlobal,strip->start);
  bnd->maxLocal = strip->end + 1;
  bnd->data     = _is_local_contig(bnd,contig) ? _line_ptr(bnd) : NULL;
  return(bnd->data);
}                               /* end get_data */


/*------------------------------------------------------------------------
------- move strip(s) onto awaiting receptor(s) or an export outFlo ------
------------------------------------------------------------------------*/
static Bool put_data(flo,pet,bnd)
     floDefPtr  flo;
     peTexPtr   pet;
     bandPtr    bnd;
{
  bandMsk suspend = 0;
  stripPtr strip;
  
  /* transfer strips until we run out or reach the one we're working in
   */
  while(_release_ok(bnd)) {
    RemoveMember(strip, bnd->stripLst.flink);
    bnd->available -= strip->length;
    bnd->minGlobal  = strip->end + 1;
    
    if(!(pet->emitting & 1<<bnd->band))
      free_strip(flo,strip);			/* output disabled    */
    else {
      strip->flink = NULL;
      suspend |= put_strip(flo,pet,strip);	/* send it downstream */
      if(!strip->flink)
	free_strip(flo,strip);			/* nobody wanted it!  */
    }
  }
  if(ListEmpty(&bnd->stripLst)) {
    bnd->strip = NULL;
    bnd->data  = NULL;
    if(bnd->final)
      disable_dst(flo,pet,bnd);
  }
  return(suspend != 0);
}                               /* end put_data */


/*------------------------------------------------------------------------
---------- free strip(s) from a receptor band or an emitter band ---------
------------------------------------------------------------------------*/
static CARD8 *free_data(flo,pet,bnd)
     floDefPtr   flo;
     peTexPtr    pet;
     bandPtr     bnd;
{
  stripPtr strip;
  bandMsk  msk = 1<<bnd->band;
  
  /* free strips until we run out or reach the one we're working in
   */
  while(_release_ok(bnd)) {
    RemoveMember(strip,bnd->stripLst.flink);
    bnd->available -= strip->length - (bnd->minGlobal - strip->start);
    bnd->minGlobal  = strip->end + 1;
    free_strip(flo,strip);
  }
  /* a little bookkeeping to let the scheduler know where we're at
   */
  bnd->available -= bnd->current - bnd->minGlobal;
  bnd->minGlobal  = bnd->current;
  if(bnd->isInput) {
    CheckSrcReady(bnd->receptor,bnd,msk);
  }
  if(bnd->final && bnd->isInput && ListEmpty(&bnd->stripLst)) {
    bnd->receptor->active &= ~msk;
    bnd->receptor->attend &= ~msk;
  }
  if(!(bnd->data = _is_local(bnd) ? _line_ptr(bnd) : NULL))
    bnd->strip = NULL;

  return(NULL);
}                               /* end free_data */


/*------------------------------------------------------------------------
--------- Clone a strip and pass it on to the element's recipients -------
------------------------------------------------------------------------*/
static Bool pass_strip(flo,pet,bnd,strip)
     floDefPtr	flo;
     peTexPtr   pet;
     bandPtr    bnd;
     stripPtr strip;
{
  stripPtr clone;

  if(!(pet->emitting & 1<<bnd->band))
    return(TRUE);			  /* output disabled */

  if(!(clone = clone_strip(flo, strip)))
    AllocError(flo,pet->peDef, return(FALSE));

  clone->format = bnd->format;		  /* this had better be right!	  */

  put_strip(flo,pet,clone);		  /* give to downstream receptors */
  if(!clone->flink)
    free_strip(flo,clone);		  /* nobody wanted it */

  if(bnd->final = strip->final)
    disable_dst(flo,pet,bnd);

  return(TRUE);
}                               /* end pass_strip */


/*------------------------------------------------------------------------
--------- pass all remaining input for this band straight through --------
------------------------------------------------------------------------*/
static void bypass_src(flo,pet,sbnd)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr   sbnd;
{
  receptorPtr rcp;
  stripPtr  strip;
  bandPtr    dbnd = &pet->emitter[sbnd->band];
  CARD8      *src;
  CARD8      *dst;

  if(sbnd->receptor->active & 1<<sbnd->band) {
    /*
     * if there's lingering data, see that it gets to its destination
     */
    for(src = GetCurrentSrc(CARD8,flo,pet,sbnd),
	dst = GetCurrentDst(CARD8,flo,pet,dbnd);
	src && dst;
	src = GetNextSrc(CARD8,flo,pet,sbnd,KEEP),
	dst = GetNextDst(CARD8,flo,pet,dbnd,!src)) {
      if(src != dst)
	bcopy(src, dst, dbnd->pitch);
    }
    /* if there's a partial strip still here, adjust its length
     */
    if(!ListEmpty(&dbnd->stripLst)) {
      strip = dbnd->stripLst.blink;
      if(strip->start < dbnd->current) {
	strip->end    = dbnd->current - 1;
	strip->length = dbnd->current - strip->start;
	put_data(flo,pet,dbnd,dbnd->current);
      }
    }
    /* shut down the src band, or the dst band if we're all done
     */
    if(pet->emitting & ~(1<<dbnd->band))
      disable_src(flo,pet,sbnd,FLUSH);
    else
      disable_dst(flo,pet,dbnd);
  }    
  /* if we're still accepting input, the remainder will bypass this element
   */
  sbnd->receptor->bypass |= 1<<sbnd->band;
}                               /* end bypass_src */


/*------------------------------------------------------------------------
---------- disable src band and discard any remaining input data ---------
------------------------------------------------------------------------*/
static void disable_src(flo,pet,bnd,purge)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	bnd;
     Bool	purge;
{
  bandMsk msk = 1<<bnd->band;

  if(bnd->receptor->admit & msk && pet->peDef->flags.putData)
    --pet->floTex->imports;		/* one less import client band    */

  if(bnd->receptor->admit && !(bnd->receptor->admit &= ~msk))
    --pet->admissionCnt;		/* one less receptor needing data */

  bnd->final = TRUE;

  if(purge)
    FreeData(void,flo,pet,bnd,bnd->maxGlobal);
}                               /* end disable_src */


/*------------------------------------------------------------------------
--- disable dst band -- also disables all src's if no dst bands remain ---
------------------------------------------------------------------------*/
static void disable_dst(flo,pet,dbnd)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	dbnd;
{
  receptorPtr rcp, rend = &pet->receptor[pet->peDef->inCnt];
  bandMsk mask;
  bandPtr sbnd;

  /* if this is the last emitter band to turned off and this isn't an import 
   * client element, we'll step thru all the receptor bands and kill them too
   */
  if(!(pet->emitting &= ~(1<<dbnd->band)) && !pet->peDef->flags.putData)
    for(rcp = pet->receptor; rcp < rend; ++rcp)
      for(mask = 1, sbnd = rcp->band; rcp->active; mask <<= 1, ++sbnd)
	if(rcp->active & mask)
	  disable_src(flo,pet,sbnd,TRUE);
}                               /* end disable_dst */


/*------------------------------------------------------------------------
- Get permission for an emitter to write into an existing receptor strip -
------------------------------------------------------------------------*/
static stripPtr	alter_data(flo,pet,db)
     floDefPtr flo;
     peTexPtr  pet;
     bandPtr   db;
{
  bandPtr  sb = db->inPlace;
  stripPtr chk, strip = sb->strip ? sb->strip : sb->stripLst.flink;
  
  /* search through the source data for the corresponding line number
   */
  while(!ListEnd(strip,&sb->stripLst))
    if(db->current > strip->end)
      strip = strip->flink;
    else if(db->current < strip->start)
      strip = strip->blink;
    else if(!strip->data || strip->Xowner)
      break;
    else {
      /* make sure there are no other users of this strip's data
       */
      for(chk = strip; chk->parent && chk->refCnt == 1; chk = chk->parent);
      if(chk->refCnt > 1)
	break;
      return(clone_strip(flo,strip));    /* return a clone of the src strip */
    }
  return(NULL);
}                               /* end alter_data */


/*------------------------------------------------------------------------
---------- enter with a strip containing at least one byte of data  ------
----------    {if a new strip must be created (to hold at least     ------
----------     contig bytes), available data will be copied to it}  ------
---------- return with a strip containing "contig" contiguous bytes ------
---------- of space (even if the actual data aren't available)      ------
------------------------------------------------------------------------*/
static stripPtr contig_data(flo,pet,bnd,i_strip,contig)
     floDefPtr	flo;
     peTexPtr	pet;
     bandPtr	bnd;
     stripPtr i_strip;
     CARD32  contig;
{
  stripPtr o_strip, n_strip;
  CARD32   limit, start, skip, avail = i_strip->end - bnd->current + 1;
  
  if(contig <= i_strip->bufSiz - i_strip->length + avail)
    o_strip = i_strip;
  else {
    /* i_strip too small, make a new one and copy available data into it
     */
    if(!(o_strip = make_strip(flo, bnd->format, bnd->current, avail,
			  contig+Align(contig,flo->floTex->stripSize),TRUE)))
      AllocError(flo,pet->peDef, return(NULL));
    InsertMember(o_strip,i_strip);
    bcopy(&i_strip->data[bnd->current-i_strip->start], o_strip->data, avail);
    if(i_strip->length -= avail)
       i_strip->end -= avail;
    else
      free_strip(flo, RemoveMember(n_strip,i_strip));
  }
  /* determine how far we can extend our o_strip
   */
  if(bnd->current + contig <= bnd->maxGlobal)
    limit = bnd->current + contig;		/* limit to data needed     */
  else
    limit = bnd->maxGlobal;			/* limit to data available  */

  /* if there are more strips beyond "o_strip", transfer the needed amount of
   * data into our mega-strip (free any strips that we completely consume)
   */
  for(start = o_strip->end+1; start < limit; start += avail) {
    n_strip = o_strip->flink;
    skip    = start - n_strip->start;
    avail   = min(n_strip->length - skip, limit - start);
    bcopy(&n_strip->data[skip], &o_strip->data[o_strip->length], avail);
    o_strip->end    += avail;
    o_strip->length += avail;
    if(avail+skip == n_strip->length)
      free_strip(flo, RemoveMember(n_strip,n_strip));
  }
 if(!bnd->isInput) {
    limit = bnd->current + contig;
    bnd->available += limit - bnd->maxGlobal;
    bnd->maxGlobal  = limit;
    o_strip->end    = limit - 1;
    o_strip->length = limit - o_strip->start;
  }
  return(o_strip);
}                               /* end contig_data */


/*------------------------------------------------------------------------
---------------------------- Make a new strip ----------------------------
------------------------------------------------------------------------*/
static stripPtr	make_strip(flo,fmt,start,units,bytes,allocData)
     floDefPtr flo;
     formatPtr fmt;
     CARD32    start, units, bytes;
     Bool      allocData;
{
  stripPtr    strip;
  stripLstPtr cache = &flo->floTex->stripHead;
  Bool     cachable = allocData && bytes == flo->floTex->stripSize;

  if(!ListEmpty(cache) && (cachable || !cache->blink->data))
    RemoveMember(strip, cachable ? cache->flink : cache->blink);
  else if(strip = (stripPtr) XieMalloc(sizeof(stripRec))) {
      strip->data  = NULL;
      ++STRIPS; /*DEBUG*/
  }
  if(strip) {
    strip->flink   = NULL;
    strip->parent  = NULL;
    strip->format  = fmt;
    strip->refCnt  = 1;
    strip->Xowner  = !allocData;
    strip->canonic = IsCanonic(fmt->class);
    strip->final   = FALSE;
    strip->cache   = cachable;
    strip->start   = start;
    strip->end     = start + units - 1;
    strip->length  = units;
    strip->bitOff  = 0;
    strip->bufSiz  = bytes;

    if(allocData && bytes && !strip->data)
      if(strip->data = (CARD8 *) XieMalloc(bytes))
	BYTES += bytes; /*DEBUG*/
      else
	strip  = free_strip(NULL,strip);
  }
  return(strip);
}                               /* end make_strip */


/*------------------------------------------------------------------------
---- Clone a new modifiable strip wrapper for existing read-only data ----
------------------------------------------------------------------------*/
static stripPtr	clone_strip(flo,in_strip)
     floDefPtr flo;
     stripPtr  in_strip;
{
  stripLstPtr  cache = &flo->floTex->stripHead;
  stripPtr out_strip;

  if(ListEmpty(cache) || cache->blink->data) {
    out_strip = (stripPtr) XieMalloc(sizeof(stripRec));
    ++STRIPS; /*DEBUG*/
  } else {
    RemoveMember(out_strip, cache->blink);
  }
  if(out_strip) {
    *out_strip         = *in_strip;
     out_strip->flink  =  NULL;
     out_strip->parent =  in_strip;
     out_strip->refCnt =  1;
    ++in_strip->refCnt;
  }
  return(out_strip);
}                               /* end clone_strip */


/*------------------------------------------------------------------------
---------------- Put strip on each receptor fed by an element ------------
------------------------------------------------------------------------*/
static bandMsk put_strip(flo,pet,strip)
     floDefPtr  flo;
     peTexPtr   pet;
     stripPtr strip;
{
  peTexPtr    dst;
  inFloPtr    inf;
  bandPtr     bnd;
  stripPtr  clone;
  receptorPtr rcp;
  CARD32     band = strip->format->band;
  bandMsk  repmsk, mask = 1<<band, suspend = 0;

  if(pet->peDef->flags.export) {
    pet->outFlo->ready |= mask;
    /*
     * give to DIXIE via our outFlo; if we're not first in line, make a clone
     */
    if(!(clone = strip->flink ? clone_strip(flo,strip) : strip))
      AllocError(flo,pet->peDef, return(suspend));
    InsertMember(clone,pet->outFlo->export[band].blink);
    ++flo->floTex->putCnt;
    return(suspend);
  }
  /* hang this strip on the receptor of each of our interested recipients
   */
  for(inf = pet->outFlo->outChain; inf; inf = inf->outChain) {
    dst =  inf->ownDef->peTex;
    rcp = &dst->receptor[inf->index];
    bnd = &rcp->band[band];
    if(rcp->bypass & mask) {
      suspend |= put_strip(flo,dst,strip);	/* just passin' through */
      continue;
    } else if(!(rcp->admit & mask)) {
      continue;					/* data not wanted here */
    }
    if(strip->final && !(rcp->admit &= ~mask))
      --dst->admissionCnt;			/* all bands complete   */
    /*
     * give the data to the intended receptor band
     */
    for(repmsk = mask; repmsk <= rcp->active; ++bnd, repmsk <<= 1) {
      if(rcp->active & repmsk) {
	bnd->final      = strip->final;
	bnd->maxGlobal  = strip->end + 1;
	bnd->available += strip->length;
	Schedule(flo,dst,rcp,bnd,repmsk);	/* schedule if runnable */
	suspend |= dst->scheduled;
	/*
	 * first recipient gets the original, all others get clones
	 */
	if(!(clone = strip->flink ? clone_strip(flo,strip) : strip))
	  AllocError(flo,dst->peDef, break);
	InsertMember(clone,bnd->stripLst.blink);
	++flo->floTex->putCnt;
      }
      /* see if we should replicate the data through the other bands
       */
      if(!rcp->band[0].replicate) break;
    }
  }
  return(suspend);
}                               /* end put_strip */


/*------------------------------------------------------------------------
----------------------------- Get rid of a strip -------------------------
------------------------------------------------------------------------*/
static stripPtr	free_strip(flo, strip)
     floDefPtr  flo;
     stripPtr strip;
{
  lstPtr cache;

  if(strip && !--strip->refCnt) {
    /* since this was the last reference we're going to nuke this strip,
     * if this was a clone, free the parent and forget where the data was
     */
    if(strip->parent) {
      free_strip(flo, strip->parent);
      strip->data = NULL;
    } else if(strip->data) {
      /* if the data buffer belongs to coreX or is uncachable, nuke it */
      if(strip->Xowner)
	strip->data = NULL;
      else if(!flo || !strip->cache) {
	strip->data = (CARD8 *) XieFree(strip->data);
	BYTES -= strip->bufSiz; /*DEBUG*/
      }
    }
    if(flo) {
      /* strips with standard data buffers are cached at the front
       * strips without data buffers go at the back
       */
      strip->refCnt = 1;
      strip->parent = NULL;
      cache = (strip->data
	       ? (lstPtr)&flo->floTex->stripHead
	       : (lstPtr) flo->floTex->stripHead.blink);
      InsertMember(strip,cache);
    } else {
      XieFree(strip);
      --STRIPS; /*DEBUG*/
    }
  }
  return(NULL);
}                               /* end free_strip */

/* end module strip.c */
