/* $XConsortium: colorlst.c,v 1.1 93/07/19 10:09:42 rws Exp $ */
/**** colorlst.c ****/
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
******************************************************************************
  
	colorlst.c -- DIXIE ColorList management
  
	Robert NC Shelley -- AGE Logic, Inc. March, 1993
  
*****************************************************************************/

#define _XIEC_COLORLST

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
#include <colormapst.h>
/*
 *  Module Specific Includes
 */
#include <corex.h>
#include <macro.h>
#include <memory.h>
#include <colorlst.h>

/*
 *  Xie protocol proceedures called from the dispatcher
 */
int  ProcCreateColorList();
int  ProcDestroyColorList();
int  ProcPurgeColorList();
int  ProcQueryColorList();
int  SProcCreateColorList();
int  SProcDestroyColorList();
int  SProcPurgeColorList();
int  SProcQueryColorList();

/*
 *  routines referenced by other modules.
 */
int  DeleteColorList();
colorListPtr  LookupColorList();
void ResetColorList();


/*------------------------------------------------------------------------
------------------------ CreateColorList Procedures ----------------------
------------------------------------------------------------------------*/
int ProcCreateColorList(client)
 ClientPtr client;
{
  colorListPtr clst;
  REQUEST(xieCreateColorListReq);
  REQUEST_SIZE_MATCH(xieCreateColorListReq);
  LEGAL_NEW_RESOURCE(stuff->colorList, client);

  /*
   * create a new ColorList
   */
  if( !(clst = (colorListPtr) XieMalloc(sizeof(colorListRec))) )
    return(client->errorValue = stuff->colorList, BadAlloc);
  
  clst->ID      = stuff->colorList;
  clst->refCnt  = 1;
  clst->clindex = client->index;
  clst->cellPtr = NULL;

  ResetColorList(clst, NULL);

  return( AddResource(clst->ID, RT_COLORLIST, (colorListPtr)clst)
	? Success : (client->errorValue = stuff->colorList, BadAlloc) );
}                               /* end ProcCreateColorList */


/*------------------------------------------------------------------------
------------------------ DestroyColorList Procedures ---------------------
------------------------------------------------------------------------*/
int ProcDestroyColorList(client)
     ClientPtr client;
{
  colorListPtr clst;
  REQUEST( xieDestroyColorListReq );
  REQUEST_SIZE_MATCH( xieDestroyColorListReq );
  
  if( !(clst = LookupColorList(stuff->colorList)) )
    return( SendResourceError(client, xieErrNoColorList, stuff->colorList) );
  
  /*
   * Disassociate the ColorList from core X -- it calls DeleteColorList()
   */
  FreeResourceByType(stuff->colorList, RT_COLORLIST, RT_NONE);
  
  return(Success);
}                               /* end ProcDestroyColorList */


/*------------------------------------------------------------------------
-------------------------- PurgeColorList Procedures ---------------------
------------------------------------------------------------------------*/
int ProcPurgeColorList(client)
 ClientPtr client;
{
  colorListPtr clst;
  REQUEST( xiePurgeColorListReq );
  REQUEST_SIZE_MATCH( xiePurgeColorListReq );
  
  if( !(clst = LookupColorList(stuff->colorList)) )
    return( SendResourceError(client, xieErrNoColorList, stuff->colorList) );

  /*
   * Free the current list of colors
   */
  ResetColorList(clst, clst->mapPtr);
  
  return(Success);
}                               /* end ProcPurgeColorList */


/*------------------------------------------------------------------------
------------------------ QueryColorList Procedures -----------------------
------------------------------------------------------------------------*/
int ProcQueryColorList(client)
 ClientPtr client;
{
  xieQueryColorListReply rep;
  colorListPtr clst;
  REQUEST( xieQueryColorListReq );
  REQUEST_SIZE_MATCH( xieQueryColorListReq );
  
  if( !(clst = LookupColorList(stuff->colorList)) )
    return( SendResourceError(client, xieErrNoColorList, stuff->colorList) );

  /*
   * Fill in the reply header
   */
  memset(&rep, 0, sz_xieQueryColorListReply);
  rep.type        = X_Reply;
  rep.sequenceNum = client->sequence;
  rep.colormap    = clst->mapID;
  rep.length      = clst->cellCnt;
  
  if( client->swapped ) {      
    /*
     * Swap the reply header fields
     */
    register int n;
    swaps(&rep.sequenceNum,n);
    swapl(&rep.colormap,n);
    swapl(&rep.length,n);
  }
  WriteToClient(client, sz_xieQueryColorListReply, (char *)&rep);
  
  if( clst->cellCnt )
    /*
     * Send the list of colors (swapped as necessary)
     * Note: cellPtr is type Pixel, which unfortunately is type unsigned long
     *       and that means more work needed here if longs are 64-bits...
     *       (anyone care to donate an Alpha?)
     */
    if( client->swapped )
      CopySwap32Write(client, clst->cellCnt, clst->cellPtr);
    else
      WriteToClient(client, clst->cellCnt, (char *)clst->cellPtr);
  
  return(Success);
}                               /* end ProcQueryColorList */


/*------------------------------------------------------------------------
----------------------- deleteFunc: DeleteColorList ----------------------
------------------------------------------------------------------------*/
int DeleteColorList(clst, id)
 colorListPtr  clst;
 xieTypColorList id;
{
  if( --clst->refCnt )
    return(Success);

  /* free any colors we're holding
   */
  ResetColorList(clst, !clst->mapID ? NULL
		 : (ColormapPtr) LookupIDByType(clst->mapID, RT_COLORMAP));

  /* free the ColorList structure.
   */
  XieFree(clst);
  
  return(Success);
}                               /* end DeleteColorList */


/*------------------------------------------------------------------------
------------------------ routine: LookupColorList ------------------------
------------------------------------------------------------------------*/
colorListPtr LookupColorList(id)
 xieTypColorList id;
{
  colorListPtr clst;
  ColormapPtr  cmap;
  
  clst = (colorListPtr) LookupIDByType(id, RT_COLORLIST);
  
  if( clst && clst->mapID ) {
    /*
     *  Lookup the associated Colormap.
     */
    cmap = (ColormapPtr) LookupIDByType(clst->mapID, RT_COLORMAP);

    if( cmap != clst->mapPtr )
      /*
       *  Forget about this Colormap and the list of colors
       */
      ResetColorList(clst, cmap);
  }
  
  return(clst);
}                               /* end LookupColorList */


/*------------------------------------------------------------------------
-------------------------- routine: ResetColorList -----------------------
------------------------------------------------------------------------*/
void ResetColorList(clst, cmap)
 colorListPtr clst;
 ColormapPtr  cmap;
{
  if( clst->cellPtr ) {
    if( cmap )
      /*
       * Free our colors from the colormap
       */
      FreeColors(cmap, clst->clindex, clst->cellCnt, clst->cellPtr, 0);

    /*
     * Free our list
     */
    XieFree(clst->cellPtr);
  }

  /*
   * Reset the ColorList to its create-time state
   */
  clst->mapID   = 0;
  clst->mapPtr  = NULL;
  clst->cellCnt = 0;
  clst->cellPtr = NULL;
}                               /* end ResetColorList */


int SProcCreateColorList(client)
 ClientPtr client;
{
  register int n;
  REQUEST(xieCreateColorListReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieCreateColorListReq);
  swapl(&stuff->colorList, n);
  return (ProcCreateColorList(client));
}                               /* end SProcCreateColorList */

int SProcDestroyColorList(client)
 ClientPtr client;
{
  register int n;
  REQUEST( xieDestroyColorListReq );
  swaps(&stuff->length, n); 
  REQUEST_SIZE_MATCH( xieDestroyColorListReq );
  swapl(&stuff->colorList, n);
  return (ProcDestroyColorList(client));
}                               /* end SProcDestroyColorList */

int SProcPurgeColorList(client)
 ClientPtr client;
{
  register int n; 
  REQUEST( xiePurgeColorListReq );
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH( xiePurgeColorListReq );
  swapl(&stuff->colorList, n);
  return (ProcPurgeColorList(client));
}                               /* end SProcPurgeColorList */

int SProcQueryColorList(client)
 ClientPtr client;
{
  register int n;
  REQUEST( xieQueryColorListReq );
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH( xieQueryColorListReq );
  swapl(&stuff->colorList, n);
  return (ProcQueryColorList(client));
}                               /* end SProcQueryColorList */
  
/* end module colorlst.c */
