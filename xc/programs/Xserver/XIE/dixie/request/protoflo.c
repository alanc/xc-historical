/* $XConsortium: protoflo.c,v 1.3 93/07/19 20:13:49 rws Exp $ */
/**** module protoflo.c ****/
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
*****************************************************************************

	protoflo.c: photospace and photoflo request/reply procedures

	Robert NC Shelley, Dean Verheiden -- AGE Logic, Inc., May 1993

****************************************************************************/

#define _XIEC_PROTOFLO
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
 *  immediate-specific includes
 */
#include <corex.h>
#include <macro.h>
#include <memory.h>
#include <flostr.h>
#include <photospc.h>


/*
 *  Xie protocol proceedures called from the dispatcher
 */
int  ProcCreatePhotospace();
int  ProcDestroyPhotospace();
int  ProcExecuteImmediate();
int  ProcCreatePhotoflo();
int  ProcDestroyPhotoflo();
int  ProcExecutePhotoflo();
int  ProcModifyPhotoflo();
int  ProcRedefinePhotoflo();
int  ProcAbort();
int  ProcAwait();
int  ProcGetClientData();
int  ProcPutClientData();
int  ProcQueryPhotoflo();

int  SProcCreatePhotospace();
int  SProcDestroyPhotospace();
int  SProcExecuteImmediate();
int  SProcCreatePhotoflo();
int  SProcDestroyPhotoflo();
int  SProcExecutePhotoflo();
int  SProcModifyPhotoflo();
int  SProcRedefinePhotoflo();
int  SProcAbort();
int  SProcAwait();
int  SProcGetClientData();
int  SProcPutClientData();
int  SProcQueryPhotoflo();

/*
 *  routines referenced by other modules
 */
int  DeletePhotospace();
int  DeletePhotoflo();

/*
 *  routines used internal to this module
 */
static floDefPtr LookupExecutable();
static floDefPtr LookupImmediate();
static int	 RunFlo();
static int	 FloDone();
static void	 DeleteImmediate();



/*------------------------------------------------------------------------
--------------------------- CreatePhotospace Procedure -------------------
------------------------------------------------------------------------*/
int ProcCreatePhotospace(client)
 ClientPtr client;
{
  photospacePtr space;
  REQUEST(xieCreatePhotospaceReq);
  REQUEST_SIZE_MATCH(xieCreatePhotospaceReq);
  LEGAL_NEW_RESOURCE(stuff->nameSpace, client);

  /*
   * create a new lookup table
   */
  if(!(space = (photospacePtr) XieMalloc(sizeof(photospaceRec)))) 
    return(client->errorValue = stuff->nameSpace, BadAlloc);
  
  space->spaceID = stuff->nameSpace;
  space->floCnt  = 0;
  ListInit(&space->floLst);

  return( AddResource(space->spaceID, RT_PHOTOSPACE, space)
	? Success : (client->errorValue = stuff->nameSpace, BadAlloc) );
}                               /* end ProcCreatePhotospace */


/*------------------------------------------------------------------------
------------------------ DestroyPhotospace Procedure ---------------------
------------------------------------------------------------------------*/
int ProcDestroyPhotospace(client)
     ClientPtr client;
{
  photospacePtr space;
  REQUEST( xieDestroyPhotospaceReq );
  REQUEST_SIZE_MATCH( xieDestroyPhotospaceReq );
  
  if(!(space = (photospacePtr)LookupIDByType(stuff->nameSpace,RT_PHOTOSPACE)))
    return( SendResourceError(client, xieErrNoPhotospace, stuff->nameSpace) );
  
  /*
   * Disassociate the Photospace from core X -- it calls DeletePhotospace()
   */
  FreeResourceByType(stuff->nameSpace, RT_PHOTOSPACE, RT_NONE);
  
  return(Success);
}                               /* end ProcDestroyPhotospace */


/*------------------------------------------------------------------------
------------------------ ExecuteImmediate Procedure ----------------------
------------------------------------------------------------------------*/
int ProcExecuteImmediate(client)
     ClientPtr client;
{
  floDefPtr flo;
  photospacePtr space;
  REQUEST(xieExecuteImmediateReq);
  REQUEST_AT_LEAST_SIZE(xieExecuteImmediateReq);

  /* verify that the new flo-id is unique */
  flo = LookupImmediate(stuff->nameSpace, stuff->floID, &space);
  if( !space || flo )
    return(SendFloIDError(client,stuff->nameSpace,stuff->floID));
  
  /* create the flo structures and verify the DAG's topology */
  if(!(flo = MakeFlo(client, stuff->numElements, (xieFlo *)&stuff[1])))
    return(client->errorValue = stuff->floID, BadAlloc);
  
  /* append the new flo to the photospace */
  flo->space        = space;
  flo->spaceID      = stuff->nameSpace;
  flo->ID           = stuff->floID;
  flo->flags.notify = stuff->notify;
  space->floCnt++;
  InsertMember(flo,space->floLst.blink);

  /* try to execute it */
  return(RunFlo(client,flo));
}                               /* end ProcExecuteImmediate */


/*------------------------------------------------------------------------
------------------------- CreatePhotoflo Procedure -----------------------
------------------------------------------------------------------------*/
int ProcCreatePhotoflo(client)
     ClientPtr client;
{
  floDefPtr flo;
  REQUEST(xieCreatePhotofloReq);
  REQUEST_AT_LEAST_SIZE(xieCreatePhotofloReq);
  LEGAL_NEW_RESOURCE(stuff->floID, client);

  /* create a new Photoflo
   */
  if( !(flo = MakeFlo(client, stuff->numElements, (xieFlo *)&stuff[1])) )
    return(client->errorValue = stuff->floID, BadAlloc);
  flo->ID = stuff->floID;
  
  if( ferrCode(flo) ) {
    SendFloError(client,flo);
    DeletePhotoflo(flo, stuff->floID);
    return(Success);
  }
  /* All is well, try to register the new flo
   */
  return( AddResource(stuff->floID, RT_PHOTOFLO, (floDefPtr)flo)
	 ? Success : (client->errorValue = stuff->floID, BadAlloc) );
}                               /* end ProcCreatePhotoflo */


/*------------------------------------------------------------------------
------------------------ DestroyPhotoflo Procedure -----------------------
------------------------------------------------------------------------*/
int ProcDestroyPhotoflo(client)
     ClientPtr client;
{
  floDefPtr flo;
  REQUEST( xieDestroyPhotofloReq );
  REQUEST_SIZE_MATCH( xieDestroyPhotofloReq );
  
  if( !(flo = (floDefPtr) LookupIDByType(stuff->floID, RT_PHOTOFLO)) )
    return( SendResourceError(client, xieErrNoPhotoflo, stuff->floID) );
  
  /* Disassociate the Photoflo from core X -- it calls DeletePhotoflo()
   */
  FreeResourceByType(stuff->floID, RT_PHOTOFLO, RT_NONE);
  
  return(Success);
}                               /* end ProcDestroyPhotoflo */


/*------------------------------------------------------------------------
------------------------ ExecutePhotoflo Procedure -----------------------
------------------------------------------------------------------------*/
int ProcExecutePhotoflo(client)
     ClientPtr client;
{
  floDefPtr flo;
  REQUEST( xieExecutePhotofloReq );
  REQUEST_SIZE_MATCH( xieExecutePhotofloReq );
  
  if( !(flo = (floDefPtr) LookupIDByType(stuff->floID, RT_PHOTOFLO)) )
    return( SendResourceError(client, xieErrNoPhotoflo, stuff->floID) );
  
  if( flo->flags.active ) 
    FloAccessError(flo,0,0, return(SendFloError(client,flo)));
  flo->flags.notify = stuff->notify;

  /* try to execute it */
  return(RunFlo(client,flo));
}                               /* end ProcExecutePhotoflo */


/*------------------------------------------------------------------------
------------------------- ModifyPhotoflo Procedure -----------------------
------------------------------------------------------------------------*/
int ProcModifyPhotoflo(client)
     ClientPtr client;
{
  floDefPtr flo;
  xieTypPhototag end;
  REQUEST( xieModifyPhotofloReq );
  REQUEST_AT_LEAST_SIZE(xieModifyPhotofloReq);

  if( !(flo = (floDefPtr) LookupIDByType(stuff->floID, RT_PHOTOFLO)) )
    return( SendResourceError(client, xieErrNoPhotoflo, stuff->floID) );
  
  if( flo->flags.active )
    FloAccessError(flo,0,0, goto egress);

  if(!stuff->start || stuff->start > flo->peCnt)
    FloSourceError(flo,stuff->start,0, goto egress);
  
  if((end = stuff->start + stuff->numElements - 1) > flo->peCnt)
    FloElementError(flo,flo->peCnt,0, goto egress);
  
  /* edit existing elements according to the list of elements we were given
   */
  EditFlo(flo, stuff->start, end, (xieFlo *)&stuff[1]);

 egress:
  return(ferrCode(flo) ? SendFloError(client,flo) : Success);
}                               /* end ProcModifyPhotoflo */


/*------------------------------------------------------------------------
------------------------ RedefinePhotoflo Procedure ----------------------
------------------------------------------------------------------------*/
int ProcRedefinePhotoflo(client)
     ClientPtr client;
{
  floDefPtr old, new;
  REQUEST( xieRedefinePhotofloReq );
  REQUEST_AT_LEAST_SIZE(xieRedefinePhotofloReq);
  
  if( !(old = (floDefPtr) LookupIDByType(stuff->floID, RT_PHOTOFLO)) )
    return( SendResourceError(client, xieErrNoPhotoflo, stuff->floID) );
  
  if( old->flags.active ) 
    FloAccessError(old,0,0, return(SendFloError(client,old)));
  
  /* create a new Photoflo
   */
  if( !(new = MakeFlo(client, stuff->numElements, (xieFlo *)&stuff[1])) )
    return(client->errorValue = stuff->floID, BadAlloc);
  new->ID = stuff->floID;
  
  if( ferrCode(new) ) {
    SendFloError(client,new);
    DeletePhotoflo(new, stuff->floID);
    return(Success);
  }
  /* Disassociate the old flo from core X -- it calls DeletePhotoflo()
   */
  FreeResourceByType(stuff->floID, RT_PHOTOFLO, RT_NONE);
  
  /* Then (re)register the new flo using the old flo's ID
   */
  return( AddResource(stuff->floID, RT_PHOTOFLO, (floDefPtr)new)
	 ? Success : (client->errorValue = stuff->floID, BadAlloc) );
}                               /* end ProcRedefinePhotoflo */


/*------------------------------------------------------------------------
------------------------------ Abort Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcAbort(client)
     ClientPtr client;
{
  floDefPtr flo;
  REQUEST( xieAbortReq );
  REQUEST_SIZE_MATCH( xieAbortReq );
  
  if( flo = LookupExecutable(stuff->nameSpace, stuff->floID) )
    if( flo->flags.active ) {
      flo->flags.aborted = TRUE;
      ddShutdown(flo);
      FloDone(client,flo);
    }
  return(Success);
}                               /* end ProcAbort */


/*------------------------------------------------------------------------
------------------------------ Await Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcAwait(client)
     ClientPtr client;
{
  floDefPtr flo;
  REQUEST( xieAwaitReq );
  REQUEST_SIZE_MATCH( xieAwaitReq );
  
  if( (flo = LookupExecutable(stuff->nameSpace, stuff->floID))
     && flo->flags.active ) {
    /*
     * tell core X to ignore this client, we'll awaken it when the flo is done
     */
    IgnoreClient(client);
    flo->flags.awaken = TRUE;
  }
  
  return(Success);
}                               /* end ProcAwait */


/*------------------------------------------------------------------------
-------------------------- Get Client Data Procedure ---------------------
------------------------------------------------------------------------*/
int ProcGetClientData(client)
     ClientPtr client;
{
  CARD8   *data;
  CARD32  bytes;
  floDefPtr flo;
  peDefPtr  ped;
  xieGetClientDataReply rep;
  REQUEST( xieGetClientDataReq );
  REQUEST_SIZE_MATCH( xieGetClientDataReq );
  
  if( !(flo = LookupExecutable(stuff->nameSpace, stuff->floID)) )
    return( SendFloIDError(client, stuff->nameSpace, stuff->floID) );
  
  if( !flo->flags.active )
    FloAccessError(flo,stuff->element,0, return(SendFloError(client,flo)));
  
  /* verify that the specified element and band are OK
   */
  ped = stuff->element && stuff->element <= flo->peCnt
    ? flo->peArray[stuff->element] : NULL;
  if( !ped || !ped->flags.getData )
    FloElementError(flo, stuff->element, ped ? ped->elemRaw->elemType : 0,
                    goto egress);
  if( stuff->bandNumber >= ped->inFloLst[0].bands )
    ValueError(flo,ped,stuff->bandNumber, goto egress);
  
  /* get some data and fill in the reply header
   */
  memset(&rep, 0, sz_xieGetClientDataReply);
  rep.newState    = ddOutput(flo, ped, stuff->bandNumber, &data, &bytes,
			     stuff->maxBytes, stuff->terminate);
  rep.type        = X_Reply;
  rep.sequenceNum = client->sequence;
  rep.length      = bytes+3>>2;
  rep.byteCount   = bytes;
  
  if( client->swapped ) {      
    register int n;
    swaps(&rep.sequenceNum, n);
    swapl(&rep.length, n);
    swapl(&rep.byteCount, n);
  }
  WriteToClient(client, sz_xieGetClientDataReply, (char *)&rep);
  
  if( bytes )
    WriteToClient(client, bytes, (char *)data);

 egress:  
  return(ferrCode(flo) || !flo->flags.active ? FloDone(client,flo) : Success);
}                               /* end ProcGetClientData */


/*------------------------------------------------------------------------
-------------------------- Put Client Data Procedure ---------------------
------------------------------------------------------------------------*/
int ProcPutClientData(client)
     ClientPtr client;
{
  floDefPtr flo;
  peDefPtr  ped;
  REQUEST( xiePutClientDataReq );
  REQUEST_AT_LEAST_SIZE(xiePutClientDataReq);
  
  if( !(flo = LookupExecutable(stuff->nameSpace, stuff->floID)) )
    return( SendFloIDError(client, stuff->nameSpace, stuff->floID) );
  
  if( !flo->flags.active )
    FloAccessError(flo,stuff->element,0, return(SendFloError(client,flo)));
  
  /* verify that the target element and band are OK
   */
  ped = stuff->element && stuff->element <= flo->peCnt
    ? flo->peArray[stuff->element] : NULL;
  if( !ped || !ped->flags.putData )
    FloElementError(flo, stuff->element, ped ? ped->elemRaw->elemType : 0,
                    goto egress);
  if( stuff->bandNumber >= ped->inFloLst[0].bands )
    ValueError(flo,ped,stuff->bandNumber, goto egress);
  
  /* pass the byte-stream to the target element
   */
  ddInput(flo, ped, stuff->bandNumber, (CARD8 *)&stuff[1],
          stuff->byteCount, stuff->final);
 egress:  
  return(ferrCode(flo) || !flo->flags.active ? FloDone(client,flo) : Success);
}                               /* end ProcPutClientData */


/*------------------------------------------------------------------------
------------------------ QueryPhotoflo Procedure -------------------------
------------------------------------------------------------------------*/
int ProcQueryPhotoflo(client)
     ClientPtr client;
{
  CARD16 imCnt, exCnt;
  CARD32 shorts;
  floDefPtr flo;
  xieTypPhototag *list;
  xieQueryPhotofloReply rep;
  REQUEST( xieQueryPhotofloReq );
  REQUEST_SIZE_MATCH( xieQueryPhotofloReq );
  
  memset(&rep, 0, sz_xieQueryPhotofloReply);
  rep.state = ((flo = LookupExecutable(stuff->nameSpace, stuff->floID))
	       ? (flo->flags.active ? xieValActive : xieValInactive)
	       : xieValNonexistent);
  
  /* Ask ddxie about the status of client transport
   */
  if(!flo || !flo->flags.active)
    imCnt = exCnt = 0;
  else if(!ddQuery(flo,&list,&imCnt,&exCnt))
    return(SendFloError(client,flo));
  
  /* Fill in the reply header
   */
  shorts = (imCnt + 1 & ~1) + (exCnt + 1 & ~1) << 1;
  rep.type           = X_Reply;
  rep.sequenceNum    = client->sequence;
  rep.length         = shorts >> 1;
  rep.expectedCount  = imCnt;
  rep.availableCount = exCnt;
  
  if( client->swapped ) {      
    register int n;
    swaps(&rep.sequenceNum,n);
    swapl(&rep.length,n);
    swaps(&rep.expectedCount,n);
    swaps(&rep.availableCount,n);
  }
  WriteToClient(client, sz_xieQueryPhotofloReply, (char *)&rep);
  
  if(shorts) {
    /* Send the list of pending import/export(s) (swapped as necessary)
     */
    if( client->swapped )
      SwapShorts((short *)list, shorts);
    WriteToClient(client, shorts<<1, (char *)list);
    XieFree(list);
  }
  return(Success);
}                               /* end ProcQueryPhotoflo */


/*------------------------------------------------------------------------
----------------------- deleteFunc: DeletePhotospace ---------------------
------------------------------------------------------------------------*/
int DeletePhotospace(space, id)
     photospacePtr space;
     xieTypPhotospace id;
{
  /* abort and destroy all flos in the photospace
   */
  while( space->floCnt )
    DeleteImmediate( space->floLst.flink );
  
  /* Free the Photospace structure.
   */
  XieFree(space);
  
  return(Success);
}                               /* end DeletePhotospace */


/*------------------------------------------------------------------------
----------------------- deleteFunc: DeletePhotoflo -----------------------
------------------------------------------------------------------------*/
int DeletePhotoflo(flo, id)
     floDefPtr     flo;
     xieTypPhotoflo id;
{
  /* destroy any lingering ddxie structures
   */
  if(flo->flags.active)
    ddShutdown(flo);
  ddDestroy(flo);
  
  /* free the dixie element structures
   */
  FreeFlo(flo);
  
  return(Success);
}                               /* end DeletePhotoflo */


/*------------------------------------------------------------------------
------------------------- routine: LookupExecutable ----------------------
------------------------------------------------------------------------*/
static floDefPtr LookupExecutable(spaceID, floID)
     CARD32 spaceID;
     CARD32 floID;
{
  floDefPtr flo;
  
  if( spaceID )
    flo = LookupImmediate(spaceID, floID, NULL);
  else
    flo = (floDefPtr) LookupIDByType(floID, RT_PHOTOFLO);
  
  return(flo);
}                               /* end LookupExecutable */


/*------------------------------------------------------------------------
------------------------ routine: LookupImmediate ------------------------
------------------------------------------------------------------------*/
static floDefPtr LookupImmediate(spaceID, floID, spacePtr)
     CARD32  spaceID;
     CARD32  floID;
     photospacePtr *spacePtr;
{
  floDefPtr flo;
  photospacePtr space = (photospacePtr) LookupIDByType(spaceID, RT_PHOTOSPACE);
  
  if(spacePtr)
    *spacePtr = space;
  if(!space)
    return(NULL);
  
  /* search the photospace for the specified flo
   */
  for(flo = space->floLst.flink;
      !ListEnd(flo,&space->floLst) && floID != flo->ID;
      flo = flo->flink);
  
  return( ListEnd(flo,&space->floLst) ? NULL : flo );
}                               /* end LookupImmediate */


/*------------------------------------------------------------------------
----------- initiate, and possibly complete, photoflo execution ----------
------------------------------------------------------------------------*/
static int RunFlo(client,flo)
     ClientPtr client;
     floDefPtr flo;
{
  /* validate parameters and propagate attributes between elements */
  if( !ferrCode(flo) )
    PrepFlo(flo);
  
  /* choose the "best" set of handlers for this DAG (this also
   * establishes all DDXIE entry points in the floDef and peDefs)
   */
  if( !ferrCode(flo) )
    DAGalyze(flo);
  
  /* create all the new handlers that were chosen by DAGalyze */
  if( !ferrCode(flo) && flo->flags.modified )
    ddLink(flo);
  
  /* begin (and maybe complete) execution */
  if( ferrCode(flo) || !ddStartup(flo) )
    FloDone(client,flo);
  
  return(Success);
}                               /* end RunFlo */


/*------------------------------------------------------------------------
-------- Handle Photoflo Done: send error and event, then clean up -------
------------------------------------------------------------------------*/
static int FloDone(client,flo)
     ClientPtr client;
     floDefPtr flo;
{
  peDefPtr ped;
  pedLstPtr lst = ListEmpty(&flo->optDAG) ? &flo->defDAG : &flo->optDAG;
  Bool       ok = !ferrCode(flo) && !flo->flags.aborted;

  /* debrief import elements */
  for(ped = lst->flink; ped && !ListEnd(ped,lst); ped = ped->clink)
    if(ped->diVec->debrief)
      (*ped->diVec->debrief)(flo,ped,ok);

  /* debrief all other elements (e.g. export and ConvertToIndex) */
  for(ped = lst->flink; ped && !ListEnd(ped,lst); ped = ped->flink)
    if(!ped->flags.import && ped->diVec->debrief)
      (*ped->diVec->debrief)(flo,ped,ok);

  /* handle errors */
  if(ferrCode(flo)) {
    ddShutdown(flo);
    SendFloError(client,flo);
  }
  /* handle events */
  if(flo->flags.notify) {
    flo->event.event = xieEvnNoPhotofloDone;
    
    if(ferrCode(flo))
      ((xiePhotofloDoneEvn *)&flo->event)->outcome = xieValFloError;
    else if(flo->flags.aborted)
      ((xiePhotofloDoneEvn *)&flo->event)->outcome = xieValFloAbort;
    else
      ((xiePhotofloDoneEvn *)&flo->event)->outcome = xieValFloSuccess;
    
    SendFloEvent(flo);
  }
  /* if this was an immediate flo, it's history */
  if(flo->spaceID)
    DeleteImmediate(flo);

  return(Success);
}                               /* end FloDone */


/*------------------------------------------------------------------------
------------------------- routine: DeleteImmediate -----------------------
------------------------------------------------------------------------*/
static void DeleteImmediate(flo)
     floDefPtr flo;
{
  floDefPtr tmp;

  /* destroy any lingering DDXIE structures
   */
  ddDestroy(flo);

  /* remove the photoflo from the photospace and destroy it
   */
  flo->space->floCnt--;
  FreeFlo( RemoveMember(tmp,flo) );
}                               /* end DeleteImmediate */


/*------------------------------------------------------------------------
----------------------------- Swap procedures ----------------------------
------------------------------------------------------------------------*/
int SProcCreatePhotospace(client)
     ClientPtr client;
{
  register long n;
  REQUEST(xieCreatePhotospaceReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieCreatePhotospaceReq);
  swapl(&stuff->nameSpace, n);
  return (ProcCreatePhotospace(client));
}                               /* end SProcCreatePhotospace */

int SProcDestroyPhotospace(client)
     ClientPtr client;
{
  register long n;
  REQUEST( xieDestroyPhotospaceReq );
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH( xieDestroyPhotospaceReq );
  swapl(&stuff->nameSpace, n);
  return (ProcDestroyPhotospace(client));
}                               /* end SProcDestroyPhotospace */

int SProcExecuteImmediate(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieExecuteImmediateReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xieExecuteImmediateReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  swaps(&stuff->numElements, n);
  return( ProcExecuteImmediate(client) );
}

int SProcCreatePhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieCreatePhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xieCreatePhotofloReq);
  swapl(&stuff->floID, n);
  swaps(&stuff->numElements, n);
  return( ProcCreatePhotoflo(client) );
}

int SProcDestroyPhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieDestroyPhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieDestroyPhotofloReq);
  swapl(&stuff->floID, n);
  return( ProcDestroyPhotoflo(client) );
}

int SProcExecutePhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieExecutePhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieExecutePhotofloReq);
  swapl(&stuff->floID, n);
  return( ProcExecutePhotoflo(client) );
}

int SProcModifyPhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieModifyPhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xieModifyPhotofloReq);
  swapl(&stuff->floID, n);
  swaps(&stuff->start, n);
  swaps(&stuff->numElements, n);
  return( ProcModifyPhotoflo(client) );
}

int SProcRedefinePhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieRedefinePhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xieRedefinePhotofloReq);
  swapl(&stuff->floID, n);
  swaps(&stuff->numElements, n);
  return( ProcRedefinePhotoflo(client) );
}

int SProcAbort(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieAbortReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieAbortReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  return( ProcAbort(client) );
}

int SProcAwait(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieAwaitReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieAwaitReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  return( ProcAwait(client) );
}

int SProcGetClientData(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieGetClientDataReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieGetClientDataReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  swapl(&stuff->maxBytes, n);
  swaps(&stuff->element, n);
  return( ProcGetClientData(client) );
}

int SProcPutClientData(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xiePutClientDataReq);
  swaps(&stuff->length, n);
  REQUEST_AT_LEAST_SIZE(xiePutClientDataReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  swaps(&stuff->element, n);
  swapl(&stuff->byteCount, n);
  return( ProcPutClientData(client) );
}

int SProcQueryPhotoflo(client)
     register ClientPtr client;
{
  register int n;
  REQUEST(xieQueryPhotofloReq);
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH(xieQueryPhotofloReq);
  swapl(&stuff->nameSpace, n);
  swapl(&stuff->floID, n);
  return( ProcQueryPhotoflo(client) );
}

/* end module protoflo.c */
