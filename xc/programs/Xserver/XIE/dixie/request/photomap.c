/* $XConsortium: photomap.c,v 1.1 93/10/26 09:58:27 rws Exp $ */
/**** module photomap.c ****/
/****************************************************************************
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
****************************************************************************

 	photomap.c: Routines to handle Photomap protocol requests

	Dean Verheiden, Robert NC Shelley  AGE Logic, Inc.  April 1993

****************************************************************************/

#define _XIEC_PHOTOMAP

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
 *  Module Specific Includes
 */
#include <corex.h>
#include <macro.h>
#include <memory.h>
#include <photomap.h>


/*
 *  Xie protocol proceedures called from the dispatcher
 */
int  ProcCreatePhotomap();
int  ProcDestroyPhotomap();
int  ProcQueryPhotomap();
int  SProcCreatePhotomap();
int  SProcDestroyPhotomap();
int  SProcQueryPhotomap();

/*
 *  routines referenced by other modules.
 */
int  DeletePhotomap();


/*------------------------------------------------------------------------
------------------------ CreatePhotomap Procedures ----------------------
------------------------------------------------------------------------*/
int ProcCreatePhotomap(client)
     ClientPtr client;
{
  int b;
  photomapPtr map;
  REQUEST(xieCreatePhotomapReq);
  REQUEST_SIZE_MATCH(xieCreatePhotomapReq);
  LEGAL_NEW_RESOURCE(stuff->photomap, client);

  /* create a new Photomap
   */
  if( !(map = (photomapPtr) XieCalloc(sizeof(photomapRec))) )
    return(client->errorValue = stuff->photomap, BadAlloc);
  
  map->ID      = stuff->photomap;
  map->refCnt  = 1;
  map->bands   = 0;
  for(b = 0; b < xieValMaxBands; b++)
    ListInit(&map->strips[b]);

  return( AddResource(map->ID, RT_PHOTOMAP, (photomapPtr)map)
	? Success : (client->errorValue = stuff->photomap, BadAlloc) );
}                               /* end ProcCreatePhotomap */


/*------------------------------------------------------------------------
------------------------ DestroyPhotomap Procedures ---------------------
------------------------------------------------------------------------*/
int ProcDestroyPhotomap(client)
     ClientPtr client;
{
  photomapPtr map;
  REQUEST( xieDestroyPhotomapReq );
  REQUEST_SIZE_MATCH( xieDestroyPhotomapReq );
  
  if( !(map = (photomapPtr)LookupIDByType(stuff->photomap, RT_PHOTOMAP)) ) 
    return( SendResourceError(client, xieErrNoPhotomap, stuff->photomap) );
  
  /* Disassociate the Photomap from core X -- it calls DeletePhotomap()
   */
  FreeResourceByType(stuff->photomap, RT_PHOTOMAP, RT_NONE);

  return(Success);
}                               /* end ProcDestroyPhotomap */


/*------------------------------------------------------------------------
------------------------ QueryPhotomap Procedures -----------------------
------------------------------------------------------------------------*/
int ProcQueryPhotomap(client)
     ClientPtr client;
{
  xieQueryPhotomapReply rep;
  photomapPtr map;
  REQUEST( xieQueryPhotomapReq );
  REQUEST_SIZE_MATCH( xieQueryPhotomapReq );
  
  if( !(map = (photomapPtr)LookupIDByType(stuff->photomap, RT_PHOTOMAP)) ) 
    return( SendResourceError(client, xieErrNoPhotomap, stuff->photomap) );
  
  /* Fill in the reply header
   */
  bzero((char *)&rep, sz_xieQueryPhotomapReply);
  rep.type        = X_Reply;
  rep.sequenceNum = client->sequence;
  rep.length 	  = sz_xieQueryPhotomapReply-32>>2;

  if( map->bands ) {
    rep.populated       = TRUE;
    rep.dataType        = map->dataType;
    rep.data            = map->dataClass;
    rep.width0          = map->format[0].width;
    rep.height0         = map->format[0].height;
    rep.levels0         = map->format[0].levels;
    rep.decodeTechnique = map->technique;
    if( map->bands == xieValMaxBands ) {
      rep.width1  = map->format[1].width;
      rep.height1 = map->format[1].height;
      rep.levels1 = map->format[1].levels;
      rep.width2  = map->format[2].width;
      rep.height2 = map->format[2].height;
      rep.levels2 = map->format[2].levels;
    }
  } else {
    rep.populated = FALSE;
  }
  
  if( client->swapped ) {      
    /*
     * Swap the reply header fields
     */
    register int n;
    
    swaps(&rep.sequenceNum,n);
    swapl(&rep.length,n);
    if (rep.populated) {
      swaps(&rep.decodeTechnique,n);
      swapl(&rep.width0,n);
      swapl(&rep.height0,n);
      swapl(&rep.levels0,n);
      if (rep.data == xieValTripleBand) {
	swapl(&rep.width1,n);
	swapl(&rep.height1,n);
	swapl(&rep.levels1,n);
	swapl(&rep.width2,n);
	swapl(&rep.height2,n);
	swapl(&rep.levels2,n);
      }
    }
  }
  WriteToClient(client, sz_xieQueryPhotomapReply, (char *)&rep);
  
  return(Success);
}                               /* end ProcQueryPhotomap */


/*------------------------------------------------------------------------
----------------------- deleteFunc: DeletePhotomap ----------------------
------------------------------------------------------------------------*/
int DeletePhotomap(map, id)
     photomapPtr   map;
     xieTypPhotomap id;
{
  int i;
  
  if( --map->refCnt )
    return(Success);
  
  /* free compression parameters and image data
   */
  if (map->tecParms)
      map->tecParms = (void *)XieFree(map->tecParms);
  if (map->pvtParms)
      map->pvtParms = (void *)XieFree(map->pvtParms);
  for(i = 0; i < map->bands; i++) 
    FreeStrips(&map->strips[i]);
  /* 
	Free the Photomap structure.
   */
  XieFree(map);
  
  return(Success);
}                               /* end DeletePhotomap */

int SProcCreatePhotomap(client)
     ClientPtr client;
{
  register int n;
  REQUEST(xieCreatePhotomapReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieCreatePhotomapReq);
  swapl(&stuff->photomap, n); 
  return (ProcCreatePhotomap(client));
}                               /* end SProcCreatePhotomap */

int SProcDestroyPhotomap(client)
     ClientPtr client;
{
  register int n;
  REQUEST( xieDestroyPhotomapReq );
  swaps(&stuff->length, n); 
  REQUEST_SIZE_MATCH( xieDestroyPhotomapReq );
  swapl(&stuff->photomap, n); 
  return (ProcDestroyPhotomap(client));
}                               /* end SProcDestroyPhotomap */

int SProcQueryPhotomap(client)
     ClientPtr client;
{
  register int n;
  REQUEST( xieQueryPhotomapReq );
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH( xieQueryPhotomapReq );
  swapl(&stuff->photomap, n); 
  return (ProcQueryPhotomap(client));
}                               /* end SProcQueryPhotomap */

/* end module Photomap.c */
