/* $XConsortium: $ */

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
#include "PEXErr.h"
#include "pexUtils.h"
#include "pixmap.h"
#include "windowstr.h"
#include "regionstr.h"
#include "miscstruct.h"
#include "miRender.h"
#include "miStruct.h"
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

    /* initialize pointers to NULL values */
    ppm->path = 0;
    ppm->pWks = 0;
    ppm->incl_handle = 0;
    ppm->excl_handle = 0;

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

    return(Success);
}

ddpex3rtn
EndPickOne( pRend, pBuffer)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
/* out */
ddBufferPtr     pBuffer;    /* list of pick element ref */
{
    ddpex3rtn		err = Success;

   /* JSH - to be filled in later with stuff that counts and
      returns the path from the pick measure 
  */
  return(err);
}


ddpex3rtn
PickOne( pRend)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
{
    ddpex3rtn		err = Success;

    /* JSH this one uses the structure handle in prend->pickstr
       and makes a fake ddElementRange so that it can call
       RenderElements to render all elements in the structure
   */

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

   /* JSH - to be filled in later with stuff that counts and
      returns the path from the pick measure 
  */
  return(err);
}


ddpex3rtn
PickAll( pRend)
/* in */
ddRendererPtr       pRend;    /* renderer handle */
{
    ddpex3rtn		err = Success;

    /* JSH this one uses the structure handle in prend->pickstr
       and makes a fake ddElementRange so that it can call
       RenderElements to render all elements in the structure
   */

  return(err);
}
