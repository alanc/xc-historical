/* $XConsortium: pexRndr.c,v 5.1 91/02/16 09:56:54 rws Exp $ */

/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*++
 *	PEXCreateRenderer
 *	PEXFreeRenderer
 *	PEXChangeRenderer
 *	PEXGetRendererAttributes
 *	PEXBeginRendering
 *	PEXEndRendering
 *	PEXBeginStructure
 *	PEXEndStructure
 *	PEXRenderNetwork
 *	PEXRenderOutputCommands
 --*/

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


#include "X.h"
#include "pexError.h"
#include "ddpex3.h"
#include "pexLookup.h"
#include "pexExtract.h"
#include "pexUtils.h"
#include "pex_site.h"
#include "scrnintstr.h"


#ifndef PEX_BUFFER_CHUNK
#define PEX_BUFFER_CHUNK 1024
#endif

#define CHANGELUT(LUT_INDEX, REND_DYN_CHANGE_BIT) { \
	diLUTHandle plut = 0; \
	LU_TABLE((*((CARD32 *)ptr)), plut); \
	SKIP_PADDING(ptr,sizeof(CARD32)); \
	if (prend->lut[LUT_INDEX]) { \
	    err = UpdateLUTRefs(    prend->lut[LUT_INDEX], \
				    (diResourceHandle)(prend),\
				    (ddResourceType)RENDERER_RESOURCE, \
				    (ddAction)REMOVE); \
	    if (err) PEX_ERR_EXIT(err,0,cntxtPtr); } \
	prend->lut[LUT_INDEX] = plut; \
	err = UpdateLUTRefs(    prend->lut[LUT_INDEX], \
				(diResourceHandle)(prend), \
				(ddResourceType)RENDERER_RESOURCE, \
				(ddAction)ADD); \
	if (err) PEX_ERR_EXIT(err,0,cntxtPtr); \
	prend->tablesChanges |= REND_DYN_CHANGE_BIT; \
    }

#define CHANGENS(NS_INDEX, REND_DYN_CHANGE_BIT) { \
	diNSHandle pns = 0; \
	LU_NAMESET(((pexNameSet)(*((CARD32 *)ptr))),pns); \
	SKIP_PADDING(ptr,sizeof(CARD32)); \
	if (prend->ns[(unsigned)(NS_INDEX)]) { \
	    err = UpdateNSRefs(	prend->ns[(unsigned)(NS_INDEX)], \
				(diResourceHandle)(prend), \
				(ddResourceType)RENDERER_RESOURCE, \
				(ddAction)REMOVE); \
	    if (err) PEX_ERR_EXIT(err,0,cntxtPtr); } \
	prend->ns[(unsigned)(NS_INDEX)] = pns; \
	err = UpdateNSRefs( pns, (diResourceHandle)(prend), \
			    (ddResourceType)RENDERER_RESOURCE, \
			    (ddAction)ADD); \
	if (err) PEX_ERR_EXIT(err,0,cntxtPtr); \
	prend->namesetsChanges |= REND_DYN_CHANGE_BIT; \
    }


#define CHK_PEX_BUF(SIZE,INCR,REPLY,TYPE,PTR) { \
	SIZE+=INCR; \
	if (pPEXBuffer->dataSize < SIZE) { \
	    ErrorCode err = Success; \
	    int offset = (int)ptr - (int)(pPEXBuffer->pHead); \
	    err = puBuffRealloc(pPEXBuffer,(ddULONG)(SIZE)); \
	    if (err) PEX_ERR_EXIT(err,0,cntxtPtr); \
	    REPLY = (TYPE *)(pPEXBuffer->pHead); \
	    PTR = (unsigned char *)(pPEXBuffer->pHead + offset); } \
    }



/*
    From mail from Sally C. Barry
*/
#define CO_(mask)   ((mask) \
		    - (((mask)>>1)&0x77777777) \
		    - (((mask)>>1)&0x33333333) \
		    - (((mask)>>1)&0x11111111))

#define CountOnes(mask, countReturn) \
    countReturn = (((CO_(mask)+(CO_(mask)>>4)) & 0x0F0F0F0F) % 255)


extern ErrorCode UpdatePCRefs();


/*++	PEXCreateRenderer
 --*/
ErrorCode
PEXCreateRenderer (cntxtPtr, strmPtr)
pexContext              *cntxtPtr;
pexCreateRendererReq    *strmPtr;
{
    ErrorCode err = Success;
    ErrorCode freeRenderer();
    ddRendererStr *prend = 0;
    CARD8 *ptr = (CARD8 *)(strmPtr+1);
	
    if (prend = (ddRendererStr *) LookupIDByType (strmPtr->rdr, PEXRendType))
	PEX_ERR_EXIT(BadIDChoice,strmPtr->rdr,cntxtPtr);

    prend = (ddRendererStr *) Xalloc ((unsigned long)(sizeof(ddRendererStr)));
    if (!prend) PEX_ERR_EXIT(BadAlloc,0,cntxtPtr);

    prend->rendId = strmPtr->rdr;

    LU_DRAWABLE(strmPtr->drawable, prend->pDrawable);
    prend->drawableId = strmPtr->drawable;
    prend->drawExample.type = prend->pDrawable->type;
    prend->drawExample.class = prend->pDrawable->class;
    prend->drawExample.depth = prend->pDrawable->depth;
    prend->drawExample.rootDepth = prend->pDrawable->pScreen->rootDepth;
    prend->drawExample.rootVisual = prend->pDrawable->pScreen->rootVisual;

    prend->curPath = puCreateList(DD_ELEMENT_REF);	
    if (!(prend->curPath)) {
        Xfree((pointer)prend);
        PEX_ERR_EXIT(BadAlloc,0,cntxtPtr);
    }

    prend->state = PEXIdle;
    /* renderer dynamics masks are set by ddPEX */
    prend->tablesMask = 0;
    prend->namesetsMask = 0;
    prend->attrsMask = 0;
    /* flags for ddPEX */
    prend->tablesChanges = 0;
    prend->namesetsChanges = 0;
    prend->attrsChanges = 0;
    /* executeOCs table is set by ddPEX */

    /* Defaults for Lookup Tables and Name Sets */
    prend->lut[PEXMarkerBundleLUT] = 0;	prend->lut[PEXTextBundleLUT] = 0;
    prend->lut[PEXLineBundleLUT] = 0;	prend->lut[PEXInteriorBundleLUT] = 0;
    prend->lut[PEXEdgeBundleLUT] = 0;	prend->lut[PEXViewLUT] = 0;
    prend->lut[PEXColourLUT] = 0;	prend->lut[PEXDepthCueLUT] = 0;
    prend->lut[PEXLightLUT] = 0;	prend->lut[PEXColourApproxLUT] = 0;
    prend->lut[PEXPatternLUT] = 0;	prend->lut[PEXTextFontLUT] = 0;
    prend->ns[(unsigned)DD_HIGH_INCL_NS] = 0;
    prend->ns[(unsigned)DD_HIGH_EXCL_NS] = 0;
    prend->ns[(unsigned)DD_INVIS_INCL_NS] = 0;
    prend->ns[(unsigned)DD_INVIS_EXCL_NS] = 0;

    if (strmPtr->itemMask & PEXRDPipelineContext) {
	ddPCStr *ppc = 0;
	LU_PIPELINECONTEXT((*((CARD32 *)ptr)), ppc);
	prend->pPC = ppc;
	SKIP_PADDING(ptr,sizeof(CARD32));
	err = UpdatePCRefs (ppc, prend, (unsigned long)ADD);
	if (err != Success) {
	    Xfree((pointer)prend);
	    PEX_ERR_EXIT(err,0,cntxtPtr); }
    } else prend->pPC = 0;

    if (strmPtr->itemMask & PEXRDCurrentPath)  {
	puDeleteList(prend->curPath);
	if (prend->pPC)
	    (void)UpdatePCRefs (prend->pPC, prend, (unsigned long)(REMOVE));
	Xfree((pointer)prend);
	PEX_ERR_EXIT(BadValue,0,cntxtPtr);
    }

    if (strmPtr->itemMask & PEXRDMarkerBundle)
	CHANGELUT(PEXMarkerBundleLUT, PEXDynMarkerBundle);

    if (strmPtr->itemMask & PEXRDTextBundle)
	CHANGELUT(PEXTextBundleLUT, PEXDynTextBundle);

    if (strmPtr->itemMask & PEXRDLineBundle)
	CHANGELUT(PEXLineBundleLUT, PEXDynLineBundle);

    if (strmPtr->itemMask & PEXRDInteriorBundle)
	CHANGELUT(PEXInteriorBundleLUT, PEXDynInteriorBundle);

    if (strmPtr->itemMask & PEXRDEdgeBundle)
	CHANGELUT(PEXEdgeBundleLUT, PEXDynEdgeBundle);

    if (strmPtr->itemMask & PEXRDViewTable)
	CHANGELUT(PEXViewLUT, PEXDynViewTable);

    if (strmPtr->itemMask & PEXRDColourTable)
	CHANGELUT(PEXColourLUT, PEXDynColourTable);

    if (strmPtr->itemMask & PEXRDDepthCueTable)
	CHANGELUT(PEXDepthCueLUT, PEXDynDepthCueTable);

    if (strmPtr->itemMask & PEXRDLightTable)
	CHANGELUT(PEXLightLUT, PEXDynLightTable);

    if (strmPtr->itemMask & PEXRDColourApproxTable)
	CHANGELUT(PEXColourApproxLUT, PEXDynColourApproxTable);

    if (strmPtr->itemMask & PEXRDPatternTable)
	CHANGELUT(PEXPatternLUT, PEXDynPatternTable);

    if (strmPtr->itemMask & PEXRDTextFontTable)
	CHANGELUT(PEXTextFontLUT, PEXDynTextFontTable);

    if (strmPtr->itemMask & PEXRDHighlightIncl)
	CHANGENS(DD_HIGH_INCL_NS, PEXDynHighlightNameset);

    if (strmPtr->itemMask & PEXRDHighlightExcl)
	CHANGENS(DD_HIGH_EXCL_NS, PEXDynHighlightNameset);

    if (strmPtr->itemMask & PEXRDInvisibilityIncl)
	CHANGENS(DD_INVIS_INCL_NS, PEXDynInvisibilityNameset);

    if (strmPtr->itemMask & PEXRDInvisibilityExcl)
	CHANGENS(DD_INVIS_EXCL_NS, PEXDynInvisibilityNameset);

    if (strmPtr->itemMask & PEXRDHlhsrMode) {
	EXTRACT_INT16(prend->hlhsrMode,ptr);
	SKIP_PADDING(ptr,sizeof(CARD16));
    } else prend->hlhsrMode = PEXHlhsrOff;		/* default */

    if (strmPtr->itemMask & PEXRDNpcSubvolume) { 
	EXTRACT_COORD3D(&(prend->npcSubvolume.minval),ptr);
	EXTRACT_COORD3D(&(prend->npcSubvolume.maxval),ptr);
    } else {						/* defaults */
	prend->npcSubvolume.minval.x = 0.0;
	prend->npcSubvolume.minval.y = 0.0;
	prend->npcSubvolume.minval.z = 0.0;
	prend->npcSubvolume.maxval.x = 1.0;
	prend->npcSubvolume.maxval.y = 1.0;
	prend->npcSubvolume.maxval.z = 1.0;
    };


    if (strmPtr->itemMask & PEXRDViewport) {
	EXTRACT_CARD16(prend->viewport.minval.x,ptr);
	EXTRACT_CARD16(prend->viewport.minval.y,ptr);
	EXTRACT_FLOAT(prend->viewport.minval.z,ptr);
	EXTRACT_CARD16(prend->viewport.maxval.x,ptr);
	EXTRACT_CARD16(prend->viewport.maxval.y,ptr);
	EXTRACT_FLOAT(prend->viewport.maxval.z,ptr);
	EXTRACT_CARD8(prend->viewport.useDrawable,ptr);
	SKIP_PADDING(ptr,(sizeof(CARD8)+sizeof(CARD16)));
    } else prend->viewport.useDrawable = 1;		/* default */


    prend->clipList = puCreateList(DD_DEVICE_RECT);	
    if (!(prend->clipList)) {
	    puDeleteList(prend->curPath);
	    if (prend->pPC)
		(void)UpdatePCRefs (prend->pPC, prend, (unsigned long)(REMOVE));
	    Xfree((pointer)prend);
	    PEX_ERR_EXIT(BadAlloc,0,cntxtPtr);
    }
    if (strmPtr->itemMask & PEXRDClipList) {
	unsigned long nrects;
	EXTRACT_CARD32(nrects,ptr);
	puAddToList((ddPointer)ptr, nrects, prend->clipList);
	SKIP_STRUCT(ptr, nrects, pexDeviceRect);
    }; /* else prend->clipList = 0;*/				/* default */

    err = InitRenderer(prend);
    if (err) {
	puDeleteList(prend->clipList);
	puDeleteList(prend->curPath);
	if (prend->pPC)
	    (void)UpdatePCRefs (prend->pPC, prend, (unsigned long)(REMOVE));
	Xfree((pointer)prend);
	PEX_ERR_EXIT(err,0,cntxtPtr);
    };

    ADDRESOURCE(strmPtr->rdr, PEXRendType, prend);
    return( err );

} /* end-PEXCreateRenderer() */

/*++
	freeRenderer
 --*/
ErrorCode
FreeRenderer (prend, id)
ddRendererStr *prend;
pexRenderer id;
{
    if (prend) {
	DeleteDDContext(prend->pDDContext);

	puDeleteList(prend->clipList);
	puDeleteList(prend->curPath);
	Xfree((pointer)prend);
    }

    return( Success );
}

/*++	PEXFreeRenderer
 --*/

ErrorCode
PEXFreeRenderer (cntxtPtr, strmPtr)
pexContext              *cntxtPtr;
pexFreeRendererReq      *strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;

    if ((strmPtr == NULL) || (strmPtr->id == 0)) {
	err = PEX_ERROR_CODE(PEXRendererError);
	PEX_ERR_EXIT(err,0,cntxtPtr);
    }

    LU_RENDERER(strmPtr->id, prend);

    FreeResource(strmPtr->id, RT_NONE);

    return( err );

} /* end-PEXFreeRenderer() */

/*++	PEXChangeRenderer
 --*/

ErrorCode
PEXChangeRenderer( cntxtPtr, strmPtr )
pexContext  	 	*cntxtPtr;
pexChangeRendererReq 	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;
    CARD8 *ptr = (CARD8 *)(strmPtr+1);

    LU_RENDERER(strmPtr->rdr, prend);

    if (strmPtr->itemMask & PEXRDPipelineContext) {
	ddPCStr *ppc = 0, *old_ppc = 0;
	old_ppc = prend->pPC;
	if (old_ppc) (void)UpdatePCRefs (old_ppc, prend, (unsigned long)REMOVE);
	LU_PIPELINECONTEXT((*((CARD32 *)ptr)), ppc);
	SKIP_PADDING(ptr,sizeof(CARD32));
	err = UpdatePCRefs (ppc, prend, (unsigned long)ADD);
	if (err != Success) {
	    Xfree((pointer)prend);
	    PEX_ERR_EXIT(err,0,cntxtPtr); }
	prend->pPC = ppc;
    };

    if (strmPtr->itemMask & PEXRDCurrentPath)
	PEX_ERR_EXIT(BadValue,0,cntxtPtr);

    if (strmPtr->itemMask & PEXRDMarkerBundle)
	CHANGELUT(PEXMarkerBundleLUT, PEXDynMarkerBundle);

    if (strmPtr->itemMask & PEXRDTextBundle)
	CHANGELUT(PEXTextBundleLUT, PEXDynTextBundle);

    if (strmPtr->itemMask & PEXRDLineBundle)
	CHANGELUT(PEXLineBundleLUT, PEXDynLineBundle);

    if (strmPtr->itemMask & PEXRDInteriorBundle)
	CHANGELUT(PEXInteriorBundleLUT, PEXDynInteriorBundle);

    if (strmPtr->itemMask & PEXRDEdgeBundle)
	CHANGELUT(PEXEdgeBundleLUT, PEXDynEdgeBundle);

    if (strmPtr->itemMask & PEXRDViewTable)
	CHANGELUT(PEXViewLUT, PEXDynViewTable);

    if (strmPtr->itemMask & PEXRDColourTable)
	CHANGELUT(PEXColourLUT, PEXDynColourTable);

    if (strmPtr->itemMask & PEXRDDepthCueTable)
	CHANGELUT(PEXDepthCueLUT, PEXDynDepthCueTable);

    if (strmPtr->itemMask & PEXRDLightTable)
	CHANGELUT(PEXLightLUT, PEXDynLightTable);

    if (strmPtr->itemMask & PEXRDColourApproxTable)
	CHANGELUT(PEXColourApproxLUT, PEXDynColourApproxTable);

    if (strmPtr->itemMask & PEXRDPatternTable)
	CHANGELUT(PEXPatternLUT, PEXDynPatternTable);

    if (strmPtr->itemMask & PEXRDTextFontTable)
	CHANGELUT(PEXTextFontLUT, PEXDynTextFontTable);

    if (strmPtr->itemMask & PEXRDHighlightIncl)
	CHANGENS(DD_HIGH_INCL_NS, PEXDynHighlightNameset);

    if (strmPtr->itemMask & PEXRDHighlightExcl)
	CHANGENS(DD_HIGH_EXCL_NS, PEXDynHighlightNameset);

    if (strmPtr->itemMask & PEXRDInvisibilityIncl)
	CHANGENS(DD_INVIS_INCL_NS, PEXDynInvisibilityNameset);

    if (strmPtr->itemMask & PEXRDInvisibilityExcl)
	CHANGENS(DD_INVIS_EXCL_NS, PEXDynInvisibilityNameset);

    if (strmPtr->itemMask & PEXRDHlhsrMode) {
	EXTRACT_INT16(prend->hlhsrMode,ptr);
	SKIP_PADDING(ptr,sizeof(CARD16));
	prend->attrsChanges |= PEXDynHlhsrMode;
    }

    if (strmPtr->itemMask & PEXRDNpcSubvolume) { 
	EXTRACT_COORD3D(&(prend->npcSubvolume.minval),ptr);
	EXTRACT_COORD3D(&(prend->npcSubvolume.maxval),ptr);
	prend->attrsChanges |= PEXDynNpcSubvolume;
    }

    if (strmPtr->itemMask & PEXRDViewport) {
	EXTRACT_CARD16(prend->viewport.minval.x,ptr);
	EXTRACT_CARD16(prend->viewport.minval.y,ptr);
	EXTRACT_FLOAT(prend->viewport.minval.z,ptr);
	EXTRACT_CARD16(prend->viewport.maxval.x,ptr);
	EXTRACT_CARD16(prend->viewport.maxval.y,ptr);
	EXTRACT_FLOAT(prend->viewport.maxval.z,ptr);
	EXTRACT_CARD8(prend->viewport.useDrawable,ptr);
	SKIP_PADDING(ptr,(sizeof(CARD8)+sizeof(CARD16)));
	prend->attrsChanges |= PEXDynViewport;
    };

    if (strmPtr->itemMask & PEXRDClipList) {
	unsigned long nrects;
	EXTRACT_CARD32(nrects,ptr);
	PU_EMPTY_LIST(prend->clipList);
	puAddToList((ddPointer)ptr, nrects, prend->clipList);
	prend->attrsChanges |= PEXDynClipList;
	SKIP_STRUCT(ptr, nrects, pexDeviceRect);
    }

    return( err );

} /* end-PEXChangeRenderer() */

/*++	PEXGetRendererAttributes
 --*/
ErrorCode
PEXGetRendererAttributes( cntxtPtr, strmPtr )
pexContext 	 		*cntxtPtr;
pexGetRendererAttributesReq 	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;
    extern ddBuffer *pPEXBuffer;
    pexGetRendererAttributesReply *reply
			= (pexGetRendererAttributesReply *)(pPEXBuffer->pHead);
    CARD8 *ptr = 0;
    int size = 0;
    CARD32 lwords_mask, num_lwords = 0;

    LU_RENDERER(strmPtr->rdr, prend);

    SETUP_INQ(pexGetRendererAttributesReply);
    ptr = (CARD8 *) (pPEXBuffer->pBuf);

    lwords_mask = strmPtr->itemMask
			& ~(PEXRDNpcSubvolume | PEXRDViewport | PEXRDClipList);
    CountOnes(lwords_mask, num_lwords);
    num_lwords += ((strmPtr->itemMask & PEXRDCurrentPath)
		    ? (prend->curPath->numObj * sizeof(pexElementRef) / 4) + 1
		    : 0);
    CHK_PEX_BUF(size, sizeof(pexGetRendererAttributesReply)
			+ num_lwords * sizeof(CARD32),
		reply, pexGetRendererAttributesReply, ptr);

    if (strmPtr->itemMask & PEXRDPipelineContext) 
	PACK_CARD32(((prend->pPC) ? prend->pPC->PCid : 0), ptr);

    if (strmPtr->itemMask & PEXRDCurrentPath) {
	pexStructure sid = 0;
	unsigned long i;
	ddElementRef *per = (ddElementRef *)(prend->curPath->pList);
	PACK_CARD32( prend->curPath->numObj, ptr);
	for (i=0, prend->curPath->pList; i<prend->curPath->numObj; i++, per++) {
	    sid = GetId(per->structure);
	    PACK_CARD32(sid, ptr);
	    PACK_CARD32(per->offset, ptr);
	}
    }

    if (strmPtr->itemMask & PEXRDMarkerBundle)
	PACK_CARD32( GetId(prend->lut[PEXMarkerBundleLUT]), ptr);

    if (strmPtr->itemMask & PEXRDTextBundle)
	PACK_CARD32( GetId(prend->lut[PEXTextBundleLUT]), ptr);

    if (strmPtr->itemMask & PEXRDLineBundle)
	PACK_CARD32( GetId(prend->lut[PEXLineBundleLUT]), ptr);

    if (strmPtr->itemMask & PEXRDInteriorBundle)
	PACK_CARD32( GetId(prend->lut[PEXInteriorBundleLUT]), ptr);

    if (strmPtr->itemMask & PEXRDEdgeBundle)
	PACK_CARD32( GetId(prend->lut[PEXEdgeBundleLUT]), ptr);

    if (strmPtr->itemMask & PEXRDViewTable)
	PACK_CARD32( GetId(prend->lut[PEXViewLUT]), ptr);

    if (strmPtr->itemMask & PEXRDColourTable)
	PACK_CARD32( GetId(prend->lut[PEXColourLUT]), ptr);

    if (strmPtr->itemMask & PEXRDDepthCueTable)
	PACK_CARD32( GetId(prend->lut[PEXDepthCueLUT]), ptr);

    if (strmPtr->itemMask & PEXRDLightTable)
	PACK_CARD32( GetId(prend->lut[PEXLightLUT]), ptr);

    if (strmPtr->itemMask & PEXRDColourApproxTable)
	PACK_CARD32( GetId(prend->lut[PEXColourApproxLUT]), ptr);

    if (strmPtr->itemMask & PEXRDPatternTable)
	PACK_CARD32( GetId(prend->lut[PEXPatternLUT]), ptr);

    if (strmPtr->itemMask & PEXRDTextFontTable)
	PACK_CARD32( GetId(prend->lut[PEXTextFontLUT]), ptr);

    if (strmPtr->itemMask & PEXRDHighlightIncl)
	PACK_CARD32( GetId(prend->ns[(unsigned)DD_HIGH_INCL_NS]), ptr);

    if (strmPtr->itemMask & PEXRDHighlightExcl)
	PACK_CARD32( GetId(prend->ns[(unsigned)DD_HIGH_EXCL_NS]), ptr);

    if (strmPtr->itemMask & PEXRDInvisibilityIncl)
	PACK_CARD32( GetId(prend->ns[(unsigned)DD_INVIS_INCL_NS]), ptr);

    if (strmPtr->itemMask & PEXRDInvisibilityExcl)
	PACK_CARD32( GetId(prend->ns[(unsigned)DD_INVIS_EXCL_NS]), ptr);

    if (strmPtr->itemMask & PEXRDHlhsrMode) PACK_CARD32( prend->hlhsrMode, ptr);

    if (strmPtr->itemMask & PEXRDNpcSubvolume) { 
	CHK_PEX_BUF(size, sizeof(pexNpcSubvolume),
		    reply, pexGetRendererAttributesReply, ptr);
	PACK_COORD3D(&(prend->npcSubvolume.minval), ptr);
	PACK_COORD3D(&(prend->npcSubvolume.maxval), ptr);
    }

    if (strmPtr->itemMask & PEXRDViewport) {
	CHK_PEX_BUF(size, sizeof(pexViewport),
		    reply, pexGetRendererAttributesReply, ptr);
	PACK_CARD32( prend->viewport.minval.x, ptr);
	PACK_CARD32( prend->viewport.minval.y, ptr);
	PACK_CARD32( prend->viewport.minval.z, ptr);
	PACK_CARD32( prend->viewport.maxval.x, ptr);
	PACK_CARD32( prend->viewport.maxval.y, ptr);
	PACK_CARD32( prend->viewport.maxval.z, ptr);
	PACK_CARD32( prend->viewport.useDrawable, ptr);
    }

    if (strmPtr->itemMask & PEXRDClipList) {
	int num_bytes = prend->clipList->numObj * sizeof(pexDeviceRect);
	CHK_PEX_BUF(size, sizeof(CARD32) + num_bytes,
		    reply, pexGetRendererAttributesReply, ptr);
	PACK_CARD32(prend->clipList->numObj, ptr);
	bcopy((char *)(prend->clipList->pList), (char *)ptr, num_bytes);
	ptr += num_bytes + 4;
    }

    reply->length = (unsigned long)(ptr) - (unsigned long)(pPEXBuffer->pBuf) +1;
    reply->length = LWORDS(reply->length);
    WritePEXReplyToClient(	cntxtPtr, strmPtr,
				sizeof(pexGetRendererAttributesReply)
				 + sizeof(CARD32) * reply->length,
				reply);

    return( err );

} /* end-PEXGetRendererAttributes() */

/*
 *	Thexe requests provide support for client-side traversal.
 *	PEX currently provides only rendering support for client-side
 *	traversal: no picking and searching in the server for
 *	client-side or mixed mode structures.
 */

/*++	PEXBeginRendering
 --*/
ErrorCode
PEXBeginRendering( cntxtPtr, strmPtr )
pexContext   	 	*cntxtPtr;
pexBeginRenderingReq 	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;

    LU_RENDERER(strmPtr->rdr, prend);

    /* set drawableId = 0 : this helps protect us if we error-return
       out of the lookup id, and then later try to RenderOC's on this
       renderer with a bad drawable */
    prend->drawableId = 0;

    LU_DRAWABLE(strmPtr->drawable, prend->pDrawable);
    prend->drawableId = strmPtr->drawable;

    err = BeginRendering(prend, prend->pDrawable);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);
    return( err );

} /* end-PEXBeginRendering() */

/*++	PEXEndRendering
 --*/
ErrorCode
PEXEndRendering( cntxtPtr, strmPtr )
pexContext 		*cntxtPtr;
pexEndRenderingReq	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;

    LU_RENDERER(strmPtr->rdr, prend);
	
    err = EndRendering(prend);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);
    return( err );

} /* end-PEXEndRendering() */

/*++	PEXBeginStructure
 --*/
ErrorCode
PEXBeginStructure( cntxtPtr, strmPtr )
pexContext  	 	*cntxtPtr;
pexBeginStructureReq 	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;

    LU_RENDERER(strmPtr->rdr, prend);

    err = BeginStructure (prend, strmPtr->sid);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);
    return( err );

} /* end-PEXBeginStructure() */

/*++	PEXEndStructure
 --*/
ErrorCode
PEXEndStructure( cntxtPtr, strmPtr )
pexContext      	*cntxtPtr;
pexEndStructureReq    	*strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;

    LU_RENDERER(strmPtr->id, prend);

    err = EndStructure (prend);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);
    return( err );

} /* end-PEXEndStructure() */

/*++	PEXRenderNetwork
 --*/
typedef struct {
    pexRenderOutputCommandsReq	    header;
    pexExecuteStructure		    data;
} fakeRenderNetwork;

static fakeRenderNetwork froc = {
    0,				    /* reqType	    */
    PEX_RenderNetwork,		    /* opcode	    */
    sizeof(fakeRenderNetwork),	    /* length	    */
    SERVER_NATIVE_FP,		    /* fpFormat	    */
    0,				    /* unused	    */
    0,				    /* rdr	    */
    1,				    /* numCommands  */
    PEXOCExecuteStructure,	    /* elementType  */
    sizeof(pexExecuteStructure),    /* length	    */
    0				    /* id	    */
};

ErrorCode
PEXRenderNetwork( cntxtPtr, strmPtr )
pexContext      	*cntxtPtr;
pexRenderNetworkReq    	*strmPtr;
{
    ErrorCode err = PEXNYI;
    ddRendererStr *prend = 0;
    diStructHandle ps = 0;

    LU_RENDERER(strmPtr->rdr, prend);
    LU_STRUCTURE(strmPtr->sid, ps);

    err = BeginRendering(prend, prend->pDrawable);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);

    froc.header.reqType = strmPtr->reqType;
    froc.header.rdr = strmPtr->rdr;
    froc.data.id = strmPtr->sid;
    err = PEXRenderOutputCommands(cntxtPtr, &(froc.header));
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);

    err = EndRendering(prend);
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);

    return( err );

} /* end-PEXRenderNetwork() */

/*++	PEXRenderNetwork
 --*/
ErrorCode
PEXGetRendererDynamics( cntxtPtr, strmPtr )
pexContext		    *cntxtPtr;
pexGetRendererDynamicsReq   *strmPtr;
{
    ErrorCode err = Success;
    ddRendererStr *prend = 0;
    extern ddBuffer *pPEXBuffer;
    pexGetRendererDynamicsReply *reply
			      = (pexGetRendererDynamicsReply *)pPEXBuffer->pHead;

    LU_RENDERER(strmPtr->id, prend);

    err = InquireRendererDynamics(  prend, &(reply->tables),
				    &(reply->namesets), &(reply->attributes));
    if (err) PEX_ERR_EXIT(err,0,cntxtPtr);

    reply->length = 0;
    WritePEXReplyToClient(  cntxtPtr, strmPtr,
			    sizeof(pexGetRendererDynamicsReply), reply);
    return( err );

} /* end-PEXRenderNetwork() */

/*++	PEXRenderNetwork
 --*/
ErrorCode
PEXRenderOutputCommands( cntxtPtr, strmPtr )
pexContext		    *cntxtPtr;
pexRenderOutputCommandsReq  *strmPtr;
{
    CARD32		*curOC;
    pexElementInfo	*pe;
    CARD32		i;
    ErrorCode		err = Success;
    pexOutputCommandError *pErr;
    ddRendererStr	*prend = 0;
    pexStructure *ps;
    diStructHandle ph;

    LU_RENDERER(strmPtr->rdr, prend);
    CHECK_FP_FORMAT (strmPtr->fpFormat);

    /*
	Just in case, check the drawableId.  It may have been freed
	due to some race condition with client cleanup.
	Since unlike phigs workstations resources, renderers don't
	do implicit regeneration, we can just check here and we don't
	have to do the complicated resource tracking like in pexPhigs.c
     */
    LU_DRAWABLE(prend->drawableId, prend->pDrawable);

    curOC = (CARD32 *)(strmPtr + 1);

    for (i = 0, curOC = (CARD32 *)(strmPtr + 1);
	 i < strmPtr->numCommands;
	 i++, curOC += pe->length  ) {
	pe = (pexElementInfo *)curOC;
    	if ((PEXOCAll < pe->elementType ) && (pe->elementType <= PEXMaxOC)) { 
	    if (pe->elementType == PEXOCExecuteStructure) {
		ps = &(((pexExecuteStructure *)(pe))->id);
		LU_STRUCTURE(*ps, ph);
		*ps = (pexStructure)(ph);
	    }
	}

    }

    err = RenderOCs(prend, strmPtr->numCommands, (strmPtr+1), &pErr);

    if (err) PEX_OC_ERROR(pErr, cntxtPtr);

    return( err );

} /* end-PEXRenderOutputCommands() */

ErrorCode
UpdateRendRefs ( pr, pc, type, flag)
ddRendererStr *pr;
pexPC pc;
unsigned long type;
unsigned long flag;
{

}
/*++
 *
 *	End of File
 *
 --*/
