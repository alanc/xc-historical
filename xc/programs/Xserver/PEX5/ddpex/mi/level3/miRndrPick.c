/* $XConsortium: miRndrPick.c,v 1.4 92/10/05 17:04:55 hersh Exp $ */

/************************************************************
Copyright 1992 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include "miLUT.h"
#include "ddpex3.h"
#include "PEXproto.h"
#include "PEXprotost.h"
#include "pexExtract.h"
#include "PEXErr.h"
#include "pexUtils.h"
#include "pixmap.h"
#include "windowstr.h"
#include "regionstr.h"
#include "miscstruct.h"
#include "miRender.h"
#include "miStruct.h"
#include "miStrMacro.h"
#include "miWks.h"
#include "ddpex4.h"

/* External variables used */

extern  void		mi_set_filters();
extern	void		miMatMult();
extern	ddpex3rtn	miBldViewport_xform();
extern	ddpex3rtn	miBldCC_xform();
extern	ocTableType	ParseOCTable[];
extern  void		(*DestroyOCTable[])();
extern  ocTableType	InitExecuteOCTable[];
extern  ocTableType	PickExecuteOCTable[];
extern  ocTableType	SearchExecuteOCTable[];
extern  RendTableType	RenderPrimitiveTable[];
extern  RendTableType	PickPrimitiveTable[];


/* Level III Renderer Pick Procedures */

/*++
|
|  Function Name:      CreatePseudoPickMeasure
|
|  Function Description:
|     Create a Pick Measure for Renderer Picking use 
|
|  Note(s):
|
--*/

ddpex3rtn
CreatePseudoPickMeasure( pRend)
ddRendererPtr       pRend;    /* renderer handle */
{
    register miPickMeasureStr *ppm;

    ppm = (miPickMeasureStr *) xalloc(sizeof(miPickMeasureStr));
    if (!ppm) return (BadAlloc);

    ppm->path = puCreateList(DD_PICK_PATH);
    if (!ppm->path) {
	xfree(ppm);
	return (BadAlloc);
    }

    /* initialize pointers to NULL values */
    ppm->pWks = 0;
    /* initialize type to an out of range value */
    ppm->type = -1;
    ppm->status = PEXNoPick;
    ppm->pathOrder = PEXTopFirst;
    ppm->incl_handle = 0;
    ppm->excl_handle = 0;
    ppm->devPriv = (ddPointer) NULL;

    (pRend->pickstr.pseudoPM)->deviceData = (ddPointer) ppm;
    return(Success);
}


/*++
|
|  Function Name:      ChangePseudoPickMeasure
|
|  Function Description:
|     Change a Pick Measure for Renderer Picking use 
|
|  Note(s):
|
--*/

ddpex3rtn
ChangePseudoPickMeasure( pRend, pRec)
ddRendererPtr       pRend;    /* renderer handle */
ddPickRecord       *pRec;     /* PickRecord */
{
    register miPickMeasureStr *ppm;
    
    ppm = (miPickMeasureStr *) (pRend->pickstr.pseudoPM)->deviceData;

    if (!ppm->path) {
	ppm->path = puCreateList(DD_PICK_PATH);
	if (!ppm->path) {
	    xfree(ppm);
	    return (BadAlloc);
	}
    } else {
	if (puCopyList(pRend->pickStartPath, ppm->path)) {
	    puDeleteList(ppm->path);
	    xfree(ppm);
	    return (BadAlloc);
	}
    }
    ppm->incl_handle = pRend->ns[DD_PICK_INCL_NS];
    ppm->excl_handle = pRend->ns[DD_PICK_EXCL_NS];

    if (ppm->incl_handle)
	UpdateNSRefs(   ppm->incl_handle, (diResourceHandle) NULL,
			PICK_RESOURCE, ADD);

    if (ppm->excl_handle)
	UpdateNSRefs(   ppm->excl_handle, (diResourceHandle) NULL,
			PICK_RESOURCE, ADD);

    /* now store the pick record */
    ppm->type = pRec->pickType;
    switch (ppm->type) {
	case PEXPickDeviceDC_HitBox:
	    bcopy((char *)&(pRec->hit_box.DC_HitBox), 
	       	  (char *)&(ppm->input_rec.dc_hit_box), 
		   sizeof(pexPD_DC_HitBox));
            break;

	case PEXPickDeviceNPC_HitVolume:
	    bcopy((char *)&(pRec->hit_box.NPC_HitVolume), 
	       	  (char *)&(ppm->input_rec.npc_hit_volume), 
		   sizeof(pexPD_NPC_HitVolume));
            break;
    }


    ppm->status = PEXNoPick;

    return(Success);
}

ddpex3rtn
EndPickOne( pRend, pBuffer, numPickElRefs, pickStatus, betterPick)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
/* out */
ddBufferPtr     pBuffer;    /* list of pick element ref */
ddULONG         *numPickElRefs;
ddUSHORT        *pickStatus;
ddUSHORT        *betterPick;
{
    ddpex3rtn		err = Success;
    miPickMeasureStr    *ppm;
    int                 numbytes, i, j;
    ddPickPath          *per;
    pexPickElementRef   *dest;
    ddPickPath          *sIDpp;

    err = EndPicking(pRend, pRend->pickstr.pseudoPM);

    ppm = (miPickMeasureStr *) (pRend->pickstr.pseudoPM)->deviceData;
    *numPickElRefs = 0;
    *pickStatus = ppm->status;
    *betterPick = 0;

    if (ppm->status == PEXOk && ppm->path) {  /* we have a pick */

	/* send back the number of objects */
	*numPickElRefs = ppm->path->numObj;

	/* Now, tack on the list of Element Refs to the back of the reply 
	   Note that we do NOT include the length of the list. 
	   The length is found in the reply itself. 
	*/
	numbytes = sizeof(ddPickPath) * ppm->path->numObj;

	PU_CHECK_BUFFER_SIZE(pBuffer, numbytes); 
	/* Copy the Pick Path to the buffer */
	for (per = (ddPickPath*) ppm->path->pList,
	     dest = (pexPickElementRef*) pBuffer->pBuf, i=0;
	     i < ppm->path->numObj; per++, dest++, i++) {

	     /* if returned structure handle is in the sIDlist
	        then the pick was on a path below an immediate OC
	        so return the struct id the user sent over in the BeginPick
	        request, otherwise return the resource ID as normal
	     */
	     sIDpp = (ddPickPath *) (pRend->pickstr.sIDlist)->pList;
	     for (j = 0; j < (pRend->pickstr.sIDlist)->numObj; j++, sIDpp++) {
		if ((diStructHandle)(per->structure) == sIDpp->structure) {
		    /* this is CORRECT, pickid is used to store the client
		       provided structure id, yes it is a kludge...
		    */
		    dest->sid = sIDpp->pickid;
		    break;
		}
		else
		  dest->sid = ((ddStructResource*)(per->structure))->id;
	     }
	    dest->offset = per->offset;
	    dest->pickid = per->pickid;
	}

	pBuffer->dataSize = numbytes; /* tells dipex how long the reply is  */
    }

    if (ppm->path) {
	puDeleteList(ppm->path); 
	ppm->path = NULL; 
    }

  return(err);
}


ddpex3rtn
PickOne( pRend)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
{
    ddpex3rtn		err = Success;
    ddElementRange      range;
    miStructPtr         pstruct;

    /* JSH this one uses the structure handle in prend->pickstr
       and makes a fake ddElementRange so that it can call
       RenderElements to render all elements in the structure
   */

   pstruct = (miStructPtr) (pRend->pickstr.strHandle)->deviceData;
   range.position1.whence = PEXBeginning;
   range.position1.offset = 1;
   range.position2.whence = PEXBeginning;
   range.position2.offset = MISTR_NUM_EL(pstruct);

   err = RenderElements(pRend, pRend->pickstr.strHandle, &range);

  return(err);
}

ddpex3rtn
EndPickAll( pRend, pBuffer)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
/* out */
ddBufferPtr     pBuffer;    /* list of pick element ref */
{
    ddpex3rtn		err = Success;

    pexEndPickAllReply	*reply = (pexEndPickAllReply *)(pBuffer->pHead);
    int 		i, j, k, numbytes = 0, pbytes, numObj; 
    listofObj		*list;
    listofObj		**listofp;
    ddPickPath		*pp, *sIDpp;
    ddPointer		pplist;
    ddPickElementRef	ref;

    reply->numPicked = (pRend->pickstr.list)->numObj;
    reply->pickStatus = ((pRend->pickstr.list)->numObj) ?1:0;
    reply->morePicks = pRend->pickstr.more_hits;

    numObj = (pRend->pickstr.list)->numObj;
    listofp = (listofObj **)(pRend->pickstr.list)->pList;

    /* convert the pick path to a pick element ref for return */
    for (i = 0; i < numObj; i++) {
	list = listofp[0];
	pbytes = list->numObj * sizeof(ddPickElementRef);
	numbytes += pbytes + sizeof(CARD32);
	PU_CHECK_BUFFER_SIZE(pBuffer, numbytes);
	PACK_CARD32(list->numObj,  pBuffer->pBuf);
	pplist = list->pList;

	/* now convert each pick path to a pick element ref */
	/* and pack it into the reply buffer */
	for (j = 0; j < list->numObj; j++) {
	  pp = (ddPickPath *) pplist;
	  pplist = (ddPointer)(pp+1);
	  sIDpp = (ddPickPath *) (pRend->pickstr.sIDlist)->pList;
	  for (k = 0; k < (pRend->pickstr.sIDlist)->numObj; k++, sIDpp++) {
	    if ((diStructHandle)(pp->structure) == sIDpp->structure) {
		/* this is CORRECT, pickid is used to store the client
		   provided structure id, yes it is a kludge...
		*/
		ref.sid = sIDpp->pickid;
		break;
	    }
	    else
		ref.sid = ((ddStructResource *)(pp->structure))->id;
	  }
	  ref.offset = pp->offset;
	  ref.pickid = pp->pickid;
	  PACK_STRUCT(ddPickElementRef, &ref, pBuffer->pBuf);
	}

	/* remove the list from the list of list */
	puRemoveFromList( (ddPointer) &list, pRend->pickstr.list);

	/* if there are more hits save the last hit into the start path */
	if ((pRend->pickstr.more_hits == PEXMoreHits) && (i == numObj-1)) 
	    pRend->pickStartPath = list;
	else 
	    puDeleteList( list);
	
    }

    /* if there were no more hits empty the pickStartPath */
    if (pRend->pickstr.more_hits == PEXNoMoreHits) {
	puDeleteList( pRend->pickStartPath);
	pRend->pickStartPath = puCreateList( DD_PICK_PATH);
    }

    pRend->pickstr.more_hits = PEXNoMoreHits;
    pBuffer->dataSize = numbytes;


  return(err);
}


ddpex3rtn
PickAll( pRend)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
{
    ddpex3rtn			err = Success;
    miTraverserState      	trav_state;
    ddULONG            		offset1, offset2, numberOCs;
    diStructHandle 		pstruct = 0;
    miStructPtr 		pheader;
    ddPickPath			*pp;
    diPMHandle            	pPM = (diPMHandle) NULL;
    ddpex3rtn 			ValidatePickPath();

    if (!pRend->pickStartPath)  return (PEXERR(PEXPathError));
    err = ValidatePickPath(pRend->pickStartPath);
    if (err != Success) return(err);

    /* now call the traverser to traverse this structure */
    /* set exec_str_flag */
    trav_state.exec_str_flag = ES_FOLLOW_PICK;
    trav_state.p_curr_pick_el = (ddPickPath *) pRend->pickStartPath->pList ;
    trav_state.p_curr_sc_el = (ddElementRef *) NULL;
    trav_state.max_depth = 0;
    trav_state.pickId = 0;
    pPM = pRend->pickstr.pseudoPM;

    pp = (ddPickPath *) pRend->pickStartPath->pList ;
    pstruct = pp->structure;
    pheader = (miStructPtr) pstruct->deviceData;

    offset1 = 1;
    offset2 =  MISTR_NUM_EL(pheader);

    err = traverser(pRend, pstruct, offset1, offset2, pPM, NULL, &trav_state);

  return(err);
}

ddpex3rtn
AddPickPathToList( pRend, depth, path)
ddRendererPtr		pRend;		/* renderer handle */
int			depth;		/* pick path depth */
miPPLevel		*path;		/* the path 	   */
{
    listofObj		*list;
    int 		i, err;
    ddPickPath		*patharray;


    /* dont know what this is supposed to do */
    if ((pRend->pickstr.list)->numObj >= pRend->pickstr.max_hits) {
	pRend->pickstr.more_hits = PEXMoreHits;
	return;
    }
    else pRend->pickstr.more_hits = PEXNoMoreHits;

    /* allocate space to store path while reversing */
    patharray = (ddPickPath *)  xalloc(depth * sizeof(ddPickPath));

    /* create list to place the path into */
    list = puCreateList(DD_PICK_PATH);

    /* traverse the list from bottom up and copy into temp store */
    for (i = 0; i < depth; i++){
	patharray[i] = path->pp;	
	path = path->up;
    }

    /* now store the path from top down */
    for (i = depth-1; i >= 0; i--){
	err = puAddToList((ddPointer) &patharray[i], (ddULONG) 1, list);
	if (err != Success) return(err);
    }

    xfree(patharray);

    err = puAddToList( (ddPointer) &list, (ddULONG) 1, pRend->pickstr.list);
    if (err != Success) return(err);

    if ((pRend->pickstr.send_event) && 
	((pRend->pickstr.list)->numObj == pRend->pickstr.max_hits))
	err = PEXMaxHitsReachedNotify( pRend->pickstr.client, pRend->rendId);

    return(err);
}

ddpex3rtn
ValidatePickPath(pPath)
    listofObj      *pPath;
{
    miGenericElementPtr p_element;
    diStructHandle  pStruct, pNextStruct;
    miStructPtr     pstruct;
    ddULONG         offset;
    int             i;
    ddPickPath     *pPickPath;

    
    pPickPath = (ddPickPath *) pPath->pList;
    pNextStruct = pPickPath->structure;

    for (i = pPath->numObj; i > 0; i--, pPickPath++) {
	pStruct = pPickPath->structure;
	if (pNextStruct != pStruct) return (PEXERR(PEXPathError));

	pstruct = (miStructPtr) pStruct->deviceData;

	offset = pPickPath->offset;
	if (offset > MISTR_NUM_EL(pstruct)) return (PEXERR(PEXPathError));

	/* dont bother with the leaves */
	if (i == 1) break;

	MISTR_FIND_EL(pstruct, offset, p_element);

	if (MISTR_EL_TYPE(p_element) != PEXOCExecuteStructure)
	    return (PEXERR(PEXPathError));

	pNextStruct = (diStructHandle) MISTR_GET_EXSTR_STR(p_element);
    }
    return (Success);
}

