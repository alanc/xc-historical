/* $XConsortium$ */

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

/* pcflag is initialized in  ddpexInit() */
ddBOOL       pcflag;
ddPCAttr     defaultPCAttr;
#define	MI_GET_DEFAULT_PC(pPC)	\
	if (!pcflag) {		\
		DefaultPC(pPC);	\
		pcflag = MI_TRUE;  }

ddFLOAT		ident4x4[] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

/* Level III Rendering Procedures */

/*++
 |
 |  Function Name:	InitRenderer
 |
 |  Function Description:
 |	 Initializes the dd stuff in the renderer for the
 |	 PEXCreateRenderer request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
InitRenderer(pRend)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
{

    extern GCPtr	CreateScratchGC();
    extern ddpex3rtn	CreateDDContext();

    ddpex3rtn		err = Success;


#ifdef DDTEST
    ErrorF( " InitRenderer\n");
#endif

    /* set renderer dynamics */
    /* for now, set them all to be dynamic regardless of drawable type 
     * bit value of 0 means dynamic modifications
     * bit value of 1 means no dynamic modifications
     * todo: change this to be easy to specify individual dynamics
     * OR with PEXDynxxx for static; OR with ~PEXDynxxx for dynamics
     */
    /* since we don't actually make copies of namesets
     * or luts so that their values are bound at BeginRendering,
     * any change made in those tables/ns will be noticed anytime
     * info is read from an lut/ns, therefore, the tables and namesets
     * are dynamics and anytime a change is made, anything affected
     * by that change must be updated. The update is done in
     * ValidateRenderer, which is called at appropriate times to
     * make sure everything is updated.
     */
    pRend->tablesMask = 0;
    pRend->namesetsMask = ~0;
    pRend->attrsMask = ~0;

    /*
     * Create a DDContext and associate it with the Renderer
     */
    if (err = CreateDDContext(pRend)) return(err);

    /* copy the initial oc functions to the OC table */
    pRend->render_mode = MI_REND_DRAWING;
    bcopy(  (char *)InitExecuteOCTable, (char *)pRend->executeOCs,
	    sizeof(ocTableType)*OCTABLE_LENGTH); 

    MI_SET_ALL_CHANGES(pRend);
    ValidateRenderer(pRend);

    return (Success);
}

/*++
 |
 |  Function Name:	DefaultPC
 |
 |  Function Description:
 |	 Initializes a global static copy of the PC to the default values.
 |	 This copy is, in turn, used to initialize PC's the the initial values.
 |
 |  Note(s):
 |
 --*/

void
DefaultPC(pPC)
    ddPCAttr	   *pPC;
{
    pPC->markerType = PEXMarkerAsterisk;
    pPC->markerScale = 1.0;
    pPC->markerColour.colourType = PEXIndexedColour;
    pPC->markerColour.colour.indexed.index = 1;
    pPC->markerIndex = 1;
    pPC->textFont = 1;
    pPC->textPrecision = PEXStringPrecision;
    pPC->charExpansion = 1.0;
    pPC->charSpacing = 0.0;
    pPC->textColour.colourType = PEXIndexedColour;
    pPC->textColour.colour.indexed.index = 1;
    pPC->charHeight = 0.01;
    pPC->charUp.x = 0.0;
    pPC->charUp.y = 1.0;
    pPC->textPath = PEXPathRight;
    pPC->textAlignment.vertical = PEXValignNormal;
    pPC->textAlignment.horizontal = PEXHalignNormal;
    pPC->atextHeight = 0.01;
    pPC->atextUp.x = 0.0;
    pPC->atextUp.y = 1.0;
    pPC->atextPath = PEXPathRight;
    pPC->atextAlignment.vertical = PEXValignNormal;
    pPC->atextAlignment.horizontal = PEXHalignNormal;
    pPC->atextStyle = PEXATextNotConnected;
    pPC->textIndex = 1;
    pPC->lineType = PEXLineTypeSolid;
    pPC->lineWidth = 1.0;
    pPC->lineColour.colourType = PEXIndexedColour;
    pPC->lineColour.colour.indexed.index = 1;
    pPC->curveApprox.approxMethod = PEXApproxConstantBetweenKnots;
    pPC->curveApprox.tolerance = 1.0;
    pPC->lineInterp = PEXPolylineInterpNone;
    pPC->lineIndex = 1;
    pPC->intStyle = PEXInteriorStyleHollow;
    pPC->intStyleIndex = 1;
    pPC->surfaceColour.colourType = PEXIndexedColour;
    pPC->surfaceColour.colour.indexed.index = 1;
    pPC->reflAttr.ambient = 1.0;
    pPC->reflAttr.diffuse = 1.0;
    pPC->reflAttr.specular = 1.0;
    pPC->reflAttr.specularConc = 0.0;
    pPC->reflAttr.transmission = 0.0;
    pPC->reflAttr.specularColour.colourType = PEXIndexedColour;
    pPC->reflAttr.specularColour.colour.indexed.index = 1;
    pPC->reflModel = PEXReflectionNoShading;
    pPC->surfInterp = PEXSurfaceInterpNone;
    pPC->bfIntStyle = PEXInteriorStyleHollow;
    pPC->bfIntStyleIndex = 1;
    pPC->bfSurfColour.colourType = PEXIndexedColour;
    pPC->bfSurfColour.colour.indexed.index = 1;
    pPC->bfReflAttr.ambient = 1.0;
    pPC->bfReflAttr.diffuse = 1.0;
    pPC->bfReflAttr.specular = 1.0;
    pPC->bfReflAttr.specularConc = 0.0;
    pPC->bfReflAttr.transmission = 0.0;
    pPC->bfReflAttr.specularColour.colourType = PEXIndexedColour;
    pPC->bfReflAttr.specularColour.colour.indexed.index = 1;
    pPC->bfReflModel = PEXReflectionNoShading;
    pPC->bfSurfInterp = PEXSurfaceInterpNone;

    pPC->surfApprox.approxMethod = PEXApproxConstantBetweenKnots;
    pPC->surfApprox.uTolerance = 1.0;
    pPC->surfApprox.vTolerance = 1.0;

    pPC->cullMode = 0;
    pPC->distFlag = FALSE;
    pPC->patternSize.x = 1.0;
    pPC->patternSize.y = 1.0;
    pPC->patternRefPt.x = 0.0;
    pPC->patternRefPt.y = 0.0;
    pPC->patternRefPt.z = 0.0;
    pPC->patternRefV1.x = 1.0;
    pPC->patternRefV1.y = 0.0;
    pPC->patternRefV1.z = 0.0;
    pPC->patternRefV2.x = 0.0;
    pPC->patternRefV2.y = 1.0;
    pPC->patternRefV2.z = 0.0;
    pPC->intIndex = 1;
    pPC->edges = PEXOff;
    pPC->edgeType = PEXSurfaceEdgeSolid;
    pPC->edgeWidth = 1.0;
    pPC->edgeColour.colourType = PEXIndexedColour;
    pPC->edgeColour.colour.indexed.index = 1;
    pPC->edgeIndex = 1;
    bcopy((char *) ident4x4, (char *) pPC->localMat, 16 * sizeof(ddFLOAT));
    bcopy((char *) ident4x4, (char *) pPC->globalMat, 16 * sizeof(ddFLOAT));
    pPC->modelClip = PEXOff;
    pPC->modelClipVolume = puCreateList(DD_HALF_SPACE);
    pPC->viewIndex = 0;
    pPC->lightState = puCreateList(DD_INDEX);
    pPC->depthCueIndex = 0;
    pPC->colourApproxIndex = 0;
    pPC->rdrColourModel = PEXRdrColourModelRGB;
    pPC->psc.type = PEXPSCNone;
    pPC->psc.data.none = '0';
    pPC->asfs = ~0L;
    pPC->pickId = 0;
    pPC->hlhsrType = 0;
    pPC->pCurrentNS = NULL;

    return;
}

/*++
 |
 |  Function Name:	InquireRendererDynamics
 |
 |  Function Description:
 |	 Supports the PEXGetRendererDynamics request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
InquireRendererDynamics(pRend, pTablesMask, pNSMask, pAttrMask)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
    ddBitmask	  *pTablesMask;	/* dynamics mask for luts */
    ddBitmask	  *pNSMask;  /* dynamics mask for name sets */
    ddBitmask	  *pAttrMask;/* dynamics mask for renderer attributes */
{

#ifdef DDTEST
    ErrorF( " InquireRendererDynamics\n");
#endif

    *pTablesMask = pRend->tablesMask;
    *pNSMask = pRend->namesetsMask;
    *pAttrMask = pRend->attrsMask;

    return (Success);
}

/*++
 |
 |  Function Name:	RenderOCs
 |
 |  Function Description:
 |	 Supports the PEXRenderOutputCommands request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
RenderOCs(pRend, numOCs, pOCs)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
    ddULONG		numOCs;
    ddElementInfo	*pOCs;
/* out */
{
	register ddElementInfo	*poc;
	miGenericElementPtr	pexoc;
	ddpex2rtn	err;

#ifdef DDTEST
    ErrorF( " RenderOCs\n");
#endif

    ValidateRenderer(pRend);

    for ( poc=pOCs; numOCs>0; numOCs-- )
    {
	switch( poc->elementType ) {
         /* drawing primitives */
          case PEXOCMarker:
          case PEXOCMarker2D:
          case PEXOCText:
          case PEXOCText2D:
          case PEXOCAnnotationText:
          case PEXOCAnnotationText2D:
          case PEXOCPolyline:
          case PEXOCPolyline2D:
          case PEXOCPolylineSet:
          case PEXOCNurbCurve:
          case PEXOCFillArea:
          case PEXOCFillArea2D:
          case PEXOCExtFillArea:
          case PEXOCFillAreaSet:
          case PEXOCFillAreaSet2D:
          case PEXOCExtFillAreaSet:
          case PEXOCTriangleStrip:
          case PEXOCQuadrilateralMesh:
          case PEXOCSOFAS:
          case PEXOCNurbSurface:
          case PEXOCCellArray:
          case PEXOCCellArray2D:
          case PEXOCExtCellArray:
          case PEXOCGdp:
          case PEXOCGdp2D:

		/* drop out if not doing primitives
                 * otherwise fall through */
                if (!MI_DDC_DO_PRIMS(pRend)) 
                   break; 

          default:
	    pexoc = 0;
	    err = ParseOCTable[ (int)poc->elementType ]( (ddPointer)poc, &pexoc );
	    if (err != Success)
	    	return( PEXERR(PEXOutputCommandError) );
	    
	    /* add one to the current_path's element offset if a begin structure
	     * has been done
	     */
	    if (pRend->curPath->numObj)
	        ((ddElementRef *)pRend->curPath->pList)[pRend->curPath->numObj - 1].offset++;
	    pRend->executeOCs[ (int)poc->elementType ]( pRend, &pexoc->element );

	    DestroyOCTable[ (int)poc->elementType ](  pexoc );
	}

	poc += poc->length;	/* length is in four byte units & sizeof(poc) is 4 */
    }

    return (Success);
}

/*++
 |
 |  Function Name:	init_def_matrix
 |
 |  Note(s):
 |
 --*/

void
init_def_matrix (matrix)
ddFLOAT matrix[4][4];
{
  bcopy((char *) ident4x4, (char *) matrix, 16 * sizeof(ddFLOAT));
}


/*++
 |
 |  Function Name:	init_pipeline
 |
 |  Function Description:
 |	 does stuff common to BeginRendering, BeginPicking, BeginSearching
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
init_pipeline(pRend, pDrawable)
/* in */
    ddRendererPtr	pRend;	  /* renderer handle */
    DrawablePtr		pDrawable;/* pointer to drawable */
/* out */
{
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;
    ddPCAttr		*ppca;
    listofObj		*pMC, *pLS;

#ifdef DDTEST
    ErrorF( " init_pipeline\n");
#endif

    /*
     * check that drawable is OK for renderer this means it's ok for the
     * luts, ns, and GC since they are ok for the example drawable
     */

    /* empty current path */
    PU_EMPTY_LIST(pRend->curPath);

    /* 
     * Initialize ddcontext. 
     */
    ppca = pddc->Dynamic->pPCAttr;
    pMC = ppca->modelClipVolume;
    pLS = ppca->lightState;
    if (pRend->pPC != NULL) {
	*ppca = *pRend->pPC->pPCAttr;
	/* 
	 * don't forget the model clip half planes and list of
	 * light sources, which are only pointed to 
	 */
	if (puCopyList(pRend->pPC->pPCAttr->modelClipVolume, pMC))
		return(BadAlloc);
	if (puCopyList(pRend->pPC->pPCAttr->lightState, pLS))
		return(BadAlloc);
    } else {				  /* use default PC values */
	MI_GET_DEFAULT_PC(&defaultPCAttr);
	*ppca = defaultPCAttr;
	/* 
	 * don't forget the model clip half planes and list of
	 * light sources, which are only pointed to 
	 */
	if (puCopyList(defaultPCAttr.modelClipVolume, pMC))
		return(BadAlloc);
	if (puCopyList(defaultPCAttr.lightState, pLS))
		return(BadAlloc);
    }
    ppca->modelClipVolume = pMC;
    ppca->lightState = pLS;

    /* copy the current name set from the ns resource to the renderer */
    MINS_EMPTY_NAMESET(pddc->Dynamic->currentNames);
    if (ppca->pCurrentNS)
    {
	miNSHeader	*pns = (miNSHeader *)ppca->pCurrentNS->deviceData;

	MINS_COPY_NAMESET(pns->names, pddc->Dynamic->currentNames);
    }

    /* set the filter_flags in pddc for high, invis, pick, search */
    mi_set_filters(pRend, pddc);

    MI_DDC_SET_DO_PRIMS(pRend, pddc);

    /* this must be called before the rendering state is set 
     * and after the filters are set */

    ValidateRenderer(pRend);

    /* 
     * Compute composite 4x4s for use in the rendering pipeline 
     * Computed composites: 1. [GM][LM] = [CMM] or mc_to_wc_xform
     *		      2. [VO][VM] = [VOM] or wc_to_npc_xform
     *		      3. [CMM][VOM] = [VCM] or mc_to_npc_xform
     * Reminder: [GM] and  [LM] are in pipeline context, [VO]
     *	   and [VM] are in the View LUT indexed by the
     *	   view index set in the pipeline context.
     */

    /* Compute the composite [CMM] next */
    miMatMult (pddc->Dynamic->mc_to_wc_xform, 
	       pddc->Dynamic->pPCAttr->localMat,
	       pddc->Dynamic->pPCAttr->globalMat); 

    return (Success);
}

/*++
 |
 |  Function Name:	BeginRendering
 |
 |  Function Description:
 |	 Supports the PEXBeginRendering request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
BeginRendering(pRend, pDrawable)
/* in */
    ddRendererPtr	pRend;	  /* renderer handle */
    DrawablePtr		pDrawable;/* pointer to drawable */
/* out */
{
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;

#ifdef DDTEST
    ErrorF( " BeginRendering\n");
#endif

    pRend->render_mode = MI_REND_DRAWING;

    init_pipeline(pRend, pDrawable);

    /*
     * Determine the npc -> dc viewport transform
     */
     miBldViewport_xform(pRend, pDrawable, &pddc->Static.misc.viewport_xform[0][0], pddc );

    /*
     * compute cc xform, concatenate to appropriate dd context matrices.
     */
     miBldCC_xform(pRend, pddc);

    /* do double buffering stuff */
    /* do hlhsr stuff */

    pRend->state = PEXRendering;
    return (Success);
}

/*++
 |
 |  Function Name:	EndRendering
 |
 |  Function Description:
 |	 Supports the PEXEndRendering request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
EndRendering(pRend)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
{

#ifdef DDTEST
    ErrorF( " EndRendering\n");
#endif

    pRend->state = PEXIdle;
    /* switch display buffers if doing multi-buffers */

    return (Success);
}

/*++
 |
 |  Function Name:	BeginStructure
 |
 |  Function Description:
 |	 Supports the PEXBeginStructure request.
 |
 |  Note(s):
 |      This procedure creates a new ddcontext which looks like the
 |      old (current) context.  
 |
 |      Since some of these elements contain and/or are pointers to
 |      objects, the info cannot be copied directly, but new objects
 |      must be made to be pointed to and their contents copied.
 |      So, the sequence that this procedure goes through is this:
 |	create a new dd context data structure
 |	copy old context to the new context, but remember
 |		that pointers to objects will be replaced.
 |	create a new PCAttr structure and copy old to new
 |	update the current path and transform matrices
 |	push the new context onto the stack
 |
 --*/

ddpex3rtn
BeginStructure(pRend, sId)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
    ddResourceId	sId;	  /* structure id */
/* out */
{
    ddpex3rtn		PushddContext();
    ddElementRef	newRef;
    ddpex3rtn		status;

#ifdef DDTEST
    ErrorF( " BeginStructure %d\n", sId);
#endif

    /* 
     * Push the current ddContext attributes onto the stack and create
     * a new instance of these attributes.
     */
    if (status = PushddContext(pRend)) return(status);

    /* update offset of existing path to count execute structure element */
    if (pRend->curPath->numObj)
     ((ddElementRef *)pRend->curPath->pList)[pRend->curPath->numObj-1].offset++;

    /** Add a new element to the cur_path for the new structure **/
    /* sid is really a handle not an id */
    newRef.structure = (diStructHandle)sId;
    newRef.offset = 0;

    /* puAddToList returns 0 if it's successful, other there's an error */
    if (puAddToList((ddPointer)&newRef, (ddULONG)1, pRend->curPath))
    {
	return (BadAlloc);
    }

    return (Success);
}	/* BeginStructure */

/*++
 |
 |  Function Name:	EndStructure
 |
 |  Function Description:
 |	 Supports the PEXEndStructure request.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
EndStructure(pRend)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
{
    ddpex3rtn		PopddContext();

    ddpex3rtn		status;

#ifdef DDTEST
    ErrorF( " EndStructure\n");
#endif

    /*
     * Pop ddContext off stack - retrieve attributes for current structure */
    if (status = PopddContext(pRend)) return (status);

    /* 
     * could put more intelligence here, 
     * but for now assume everything changes 
     */
    MI_SET_ALL_CHANGES(pRend);
    ValidateRenderer(pRend);

    /** Remove the last currentPath element from the renderer **/
    PU_REMOVE_LAST_OBJ(pRend->curPath);

    return (Success);

}	/* EndStructure */

static void
set_highlight_colours(pRend, pddc)
    ddRendererPtr    pRend;
	miDDContext    *pddc;
{
    pddc->Static.attrs->lineColour = pddc->Static.misc.highlight_colour;
    pddc->Static.attrs->edgeColour = pddc->Static.misc.highlight_colour;
    pddc->Static.attrs->markerColour = pddc->Static.misc.highlight_colour;
    pddc->Static.attrs->surfaceColour = pddc->Static.misc.highlight_colour;
    pddc->Static.attrs->textColour = pddc->Static.misc.highlight_colour;

    pddc->Static.misc.flags |= POLYLINEGCFLAG;
    pddc->Static.misc.flags |= EDGEGCFLAG;
    pddc->Static.misc.flags |= MARKERGCFLAG;
    pddc->Static.misc.flags |= FILLAREAGCFLAG;
    pddc->Static.misc.flags |= TEXTGCFLAG;
    return;
}

static void
unset_highlight_colours(pRend, pddc)
    ddRendererPtr    pRend;
    miDDContext    *pddc;
{
    ddBitmask    tables, namesets, attrs;

    /* not too efficient: ValidateDDContextAttrs does more than
     * just colours
     */
    tables  = PEXDynMarkerBundle | PEXDynTextBundle | PEXDynLineBundle
	    | PEXDynInteriorBundle | PEXDynEdgeBundle;
    namesets = 0;
    attrs = 0;
    ValidateDDContextAttrs(pRend, pddc, tables, namesets, attrs);
    return;
}

/*++
 |
 |  Function Name:	ValidateFilters
 |
 |  Function Description:
 |	 updates filters flags 
 |
 |  Note(s):
 |
 --*/

void
ValidateFilters(pRend, pddc, namesets)
ddRendererPtr     pRend;        /* renderer handle */
miDDContext      *pddc;         /* ddPEX attribute structure */
ddBitmask         namesets;
{
    ddUSHORT      save_flags;

    if ((namesets & PEXDynHighlightNameset) ||
        (namesets & PEXDynInvisibilityNameset) ||
        (namesets & PEXDynHighlightNamesetContents) ||
        (namesets & PEXDynInvisibilityNamesetContents)) {

	    save_flags = pddc->Dynamic->filter_flags;

	    mi_set_filters(pRend, pddc, namesets);

	    if ( (MI_DDC_IS_HIGHLIGHT(pddc)) &&
		 !(save_flags & MI_DDC_HIGHLIGHT_FLAG) )
		/* just turned on highlighting */
		set_highlight_colours(pRend, pddc);
	    else if ( (!(MI_DDC_IS_HIGHLIGHT(pddc))) &&
		      (save_flags & MI_DDC_HIGHLIGHT_FLAG) )
		/* just turned off highlighting */
		unset_highlight_colours(pRend, pddc);

	    MI_DDC_SET_DO_PRIMS(pRend, pddc);
    }
}

/*++
 |
 |  Function Name:	ValidateRenderer
 |
 |  Function Description:
 |	 loads executeOC table in renderer correctly and calls
 |	 to validate the ddcontext
 |
 |  Note(s):
 |
 --*/

ValidateRenderer(pRend)
    ddRendererPtr	pRend;	  /* renderer handle */
{
    ddpex3rtn		ValidateDDContextAttrs();

    miDDContext		*pddc = (miDDContext *)pRend->pDDContext;
    ddBitmask		tables, namesets, attrs;
    extern void inq_last_colour_entry();

    /* load in different executeOCs if needed
     * can do this here or in ValidateDDContextAttrs as needed 
     * eg, if there are multiple procs in a set for an oc (different
     * ones for hollow fill areas and solid ones), then load 
     * them here or in ValidateDDContextAttrs when the attribute
     * controlling it changes
     */

    /* set highlight colour if necessary  */
    if (pRend->tablesChanges & (PEXDynColourTable | PEXDynColourTableContents))
	inq_last_colour_entry(pRend->lut[PEXColourLUT], &pddc->Static.misc.highlight_colour);

    /* validate the attributes */
    if (pRend->state == PEXRendering)
    {
	/* validate only dynamic attrs */
	tables = pRend->tablesChanges & ~pRend->tablesMask;
	namesets = pRend->namesetsChanges & ~pRend->namesetsMask;
	attrs = pRend->attrsChanges & ~pRend->attrsMask;
	ValidateFilters(pRend, pddc, tables, namesets, attrs);
	ValidateDDContextAttrs(pRend, pddc, tables, namesets, attrs);
	pRend->tablesChanges &= (~tables);
	pRend->namesetsChanges &= (~namesets);
	pRend->attrsChanges &= (~attrs);
    } else
    {
	/* validate all attrs */
	tables = pRend->tablesChanges;
	namesets = pRend->namesetsChanges;
	attrs = pRend->attrsChanges;
	ValidateDDContextAttrs(pRend, pddc, tables, namesets, attrs);
	ValidateFilters(pRend, pddc, tables, namesets, attrs);
	/* reset change masks */
	MI_ZERO_ALL_CHANGES(pRend);
    }
}

/*++
 |
 |  Function Name:	BeginPicking
 |
 |  Function Description:
 |	 Sets up the pipeline to do Picking.
 |
 |  Note(s): This is a copy of BeginRendering with extraneous rendering
 |	   stuff removed. Wherever, the code has been removed, comments
 |	   have been placed to identify the removals.
 |
 --*/

ddpex3rtn
BeginPicking(pRend, pPM)
/* in */
    ddRendererPtr	pRend;	  /* renderer handle */
    diPMHandle		pPM;	/* pick measure */
/* out */
{
    miPickMeasureStr *ppm = (miPickMeasureStr *) pPM->deviceData;
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;
    DrawablePtr		pDrawable = pRend->pDrawable;
    ddPCAttr		*ppca;
    ddFLOAT             inv_xform[4][4];

#ifdef DDTEST
    ErrorF( " BeginPicking\n");
#endif

    /* set device info needed for picking */
    pddc->Static.pick.type = ppm->type; 
    pddc->Static.pick.status = ppm->status; 
    switch (ppm->type)
    {
	case PEXPickDeviceDC_HitBox:
		pddc->Static.pick.data_rec.dc_data_rec = 
			ppm->data_rec.dc_data_rec; 
		pddc->Static.pick.input_rec.dc_hit_box = 
			ppm->input_rec.dc_hit_box; 
		break;

	case PEXPickDeviceNPC_HitVolume:
		pddc->Static.pick.data_rec.npc_data_rec = 
			ppm->data_rec.npc_data_rec; 
		pddc->Static.pick.input_rec.npc_hit_volume = 
			ppm->input_rec.npc_hit_volume; 
		break;
    }

    MINS_EMPTY_NAMESET(pddc->Static.pick.inclusion);
    MINS_EMPTY_NAMESET(pddc->Static.pick.exclusion);
    if (ppm->incl_handle) {
	miNSHeader	*pns = (miNSHeader *)ppm->incl_handle->deviceData;
	MINS_COPY_NAMESET(pns->names, pddc->Static.pick.inclusion);
    }
    if (ppm->excl_handle) {
	miNSHeader	*pns = (miNSHeader *)ppm->excl_handle->deviceData;
	MINS_COPY_NAMESET(pns->names, pddc->Static.pick.exclusion);
    }

/* load picking procs into executeOCs */

    bcopy((char *)PickExecuteOCTable, 
	(char *)pRend->executeOCs, 
	sizeof(ocTableType)*OCTABLE_LENGTH); 

    pRend->render_mode = MI_REND_PICKING;

    /*
     * Reinitialize level 1 procedure jump table for PICKING !
     */
    bcopy((char *)PickPrimitiveTable, 
	  (char *)pddc->Static.RenderProcs, 
	  sizeof(RendTableType) * RENDER_TABLE_LENGTH); 

    init_pipeline(pRend, pDrawable);

    /*
     * Determine the npc -> dc viewport transform
     */
    miBldViewport_xform( pRend, pDrawable, &pddc->Static.misc.viewport_xform[0][0], pddc );

    /* Compute the inverse of the viewport transform to be used to */
    /* convert DC_HitBoxes to NPC_HitVolumes.                      */

    bcopy ((char *)pddc->Static.misc.viewport_xform, 
	   (char *)inv_xform, 16*sizeof(ddFLOAT));
    miMatInverse (inv_xform);
    bcopy ((char *) inv_xform, (char *)pddc->Static.misc.inv_vpt_xform, 
	    16*sizeof(ddFLOAT));

    /* Now, clear out the viewport transform computed above to identity */
    /* since we are PICKING, and we do not need this to go to final ddx */
    /* space. THIS IS IMPORTANT SINCE WE WILL BE USING THE LEVEL 2 REND-*/
    /* ERING ROUTINES TO DO TRANSFORMATIONS AND CLIPPING. ONLY LEVEL 1  */
    /* PICKING ROUTINES WILL ACTUALLY DO THE PICK HIT TEST IN CC. THUS  */
    /* BY MAKING THE viewport transform IDENTITY WE WILL STAY IN CC.    */

    bcopy((char *) ident4x4, 
	  (char *) pddc->Static.misc.viewport_xform, 16 * sizeof(ddFLOAT));

    /* Clear out the cc_to_dc_xform also, since we will not be going to DC */

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->cc_to_dc_xform, 16 * sizeof(ddFLOAT));

    /* Mark as invalid appropriate inverse transforms in dd context */
    pddc->Static.misc.flags |= (INVTRMCTOWCXFRMFLAG | INVTRWCTOCCXFRMFLAG |
				INVTRMCTOCCXFRMFLAG | INVTRCCTODCXFRMFLAG |
				INVVIEWXFRMFLAG);

    /* Mark as invalid any transform dependant fields in ddContext */
    pddc->Static.misc.flags |= (MCVOLUMEFLAG | CC_DCUEVERSION);

    /* 
     * Computation of the composite mc -> dc transform has been REMOVED.
     */

    /* do double buffering stuff */
    /* do hlhsr stuff */

    pRend->state = PEXRendering;

    return (Success);
}

/*++
 |
 |  Function Name:	EndPicking
 |
 |  Function Description:
 |	 Handles the stuff to be done after a Pick traversal.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
EndPicking(pRend)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
{
/* Locals */
      miDDContext      *pddc = (miDDContext *)(pRend->pDDContext);

#ifdef DDTEST
    ErrorF( " EndPicking\n");
#endif

    EndRendering(pRend);

    pRend->render_mode = MI_REND_DRAWING;

    /* copy the initial oc functions to the OC table */

    bcopy(  (char *)InitExecuteOCTable, 
	    (char *)pRend->executeOCs, 
	    sizeof(ocTableType)*OCTABLE_LENGTH); 

    /*
     * Reinitialize level 1 procedure jump table for Rendering !
     */
    bcopy((char *)RenderPrimitiveTable, 
	  (char *)pddc->Static.RenderProcs, 
	  sizeof(RendTableType) * RENDER_TABLE_LENGTH); 

    return (Success);
}

/*++
 |
 |  Function Name:	InquirePickStatus
 |
 |  Function Description:
 |	 returns current pick status
 |
 |  Note(s):

 |
 --*/

/* 
#define PEX_SI_FAKE_PICK 
*/
#ifdef PEX_SI_FAKE_PICK
/*dummy proc to use to fake hit:
  hit occurs when traversal depth = 3, structure offset = 3,
  current pick id = 4
  need p_trav_state for this, but p_trav_state will not be a 
  parameter when real picking is done
*/
InquirePickStatus(pRend, pStatus, p_trav_state)
	ddRendererPtr	pRend;
	ddUSHORT	*pStatus;	
	miTraverserState	*p_trav_state;	
{
	if ( (p_trav_state->max_depth == 3) &&
		(((ddElementRef *)pRend->curPath-> pList)[pRend->curPath->numObj - 1].offset == 3) &&
		( ((miDDContext *)pRend->pDDContext)->Dynamic->pPCAttr->pickId == 4) )
	  *pStatus = PEXOk;
	else
	  *pStatus = PEXNoPick;
	return;
}
#else

void
InquirePickStatus(pRend, pStatus, p_trav_state)
	ddRendererPtr	pRend;
	ddUSHORT	*pStatus;	
{
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;

	*pStatus = pddc->Static.pick.status;
	return;
}
#endif

/*++
 |
 |  Function Name:	BeginSearching
 |
 |  Function Description:
 |	 Sets up the pipeline to do spatial search
 |
 |  Note(s): This is a copy of BeginRendering with extraneous rendering
 |	   stuff removed. Wherever, the code has been removed, comments
 |	   have been placed to identify the removals.
 |
 --*/

ddpex3rtn
BeginSearching(pRend, pSC)
/* in */
    ddRendererPtr	pRend;	  /* renderer handle */
    ddSCStr		*pSC;	/* search context */
/* out */
{
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;
    DrawablePtr		pDrawable = pRend->pDrawable;
    ddNSPair		*pPairs;
    miNSHeader		*pns; 
    register int	i;

#ifdef DDTEST
    ErrorF( " BeginSearching\n");
#endif

    /* set device info needed for searching */
    pddc->Static.search.status = PEXNotFound;	
    pddc->Static.search.position = pSC->position; 
    pddc->Static.search.distance = pSC->distance; 
    pddc->Static.search.modelClipFlag = pSC->modelClipFlag; 

    MINS_EMPTY_NAMESET(pddc->Static.search.norm_inclusion);
    MINS_EMPTY_NAMESET(pddc->Static.search.norm_exclusion);
    MINS_EMPTY_NAMESET(pddc->Static.search.invert_inclusion);
    MINS_EMPTY_NAMESET(pddc->Static.search.invert_exclusion);

    if (pSC->normal.numPairs) {
	pPairs = pSC->normal.pPairs;
	for (i=0; i<pSC->normal.numPairs; i++, pPairs++ ) {

	    if (pPairs->incl) {
	        pns = (miNSHeader *)pPairs->incl->deviceData;
	        MINS_OR_NAMESETS(pns->names, pddc->Static.search.norm_inclusion);
	    }
	    if (pPairs->excl) {
	        pns = (miNSHeader *)pPairs->excl->deviceData;
	        MINS_OR_NAMESETS(pns->names, pddc->Static.search.norm_exclusion);
	    }
	}
    }
    if (pSC->inverted.numPairs) {
	pPairs = pSC->inverted.pPairs;
	for (i=0; i<pSC->inverted.numPairs; i++, pPairs++ ) {

	    if (pPairs->incl) {
	        pns = (miNSHeader *)pPairs->incl->deviceData;
	        MINS_OR_NAMESETS(pns->names, pddc->Static.search.invert_inclusion);
	    }
	    if (pPairs->excl) {
	        pns = (miNSHeader *)pPairs->excl->deviceData;
	        MINS_OR_NAMESETS(pns->names, pddc->Static.search.invert_exclusion);
	    }
	}
    }

 /* load searching procs into executeOCs  */

    bcopy((char *)SearchExecuteOCTable, 
	(char *)pRend->executeOCs, 
	sizeof(ocTableType)*OCTABLE_LENGTH); 

    /*
     * Reinitialize level 1 procedure jump table for Searching.
     * Note that we use the same table as Picking.
     */
    bcopy((char *)PickPrimitiveTable,
          (char *)pddc->Static.RenderProcs,
          sizeof(RendTableType) * RENDER_TABLE_LENGTH);
 
    /* Set the model clipping flag to value in search context
       resource */
    pddc->Dynamic->pPCAttr->modelClip = pSC->modelClipFlag;
 
    pRend->render_mode = MI_REND_SEARCHING;

    init_pipeline(pRend, pDrawable);

    /*
     * Since searching is done in world coordinate space, we need NOT
     * compute any of the rest of the matrices. They must all be set
     * to identity.
     */

    bcopy((char *) ident4x4, 
	  (char *) pddc->Static.misc.viewport_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->wc_to_npc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->mc_to_npc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->wc_to_cc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->cc_to_dc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->mc_to_cc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->mc_to_dc_xform, 16 * sizeof(ddFLOAT));

    bcopy((char *) ident4x4, 
	  (char *) pddc->Dynamic->npc_to_cc_xform, 16 * sizeof(ddFLOAT));

    /* Mark as invalid appropriate inverse transforms in dd context */
    pddc->Static.misc.flags |= (INVTRMCTOWCXFRMFLAG | INVTRWCTOCCXFRMFLAG |
				INVTRMCTOCCXFRMFLAG | INVTRCCTODCXFRMFLAG |
				INVVIEWXFRMFLAG);

    /* Mark as invalid any transform dependant fields in ddContext */
    pddc->Static.misc.flags |= (MCVOLUMEFLAG | CC_DCUEVERSION);

    /* 
     * Computation of the composite mc -> dc transform has been REMOVED.
     */

    /* do double buffering stuff */
    /* do hlhsr stuff */

    pRend->state = PEXRendering;

    return (Success);
}

/*++
 |
 |  Function Name:	EndSearching
 |
 |  Function Description:
 |	 Handles the stuff to be done after a search traversal.
 |
 |  Note(s):
 |
 --*/

ddpex3rtn
EndSearching(pRend)
/* in */
    ddRendererPtr       pRend;	  /* renderer handle */
/* out */
{

#ifdef DDTEST
    ErrorF( " EndSearching\n");
#endif

    EndRendering(pRend);

    pRend->render_mode = MI_REND_DRAWING;
    /* copy the initial oc functions to the OC table */
    bcopy((char *)InitExecuteOCTable, 
	(char *)pRend->executeOCs, 
	sizeof(ocTableType)*OCTABLE_LENGTH); 

    return (Success);
}

/*++
 |
 |  Function Name:	InquireSearchStatus
 |
 |  Function Description:
 |	 returns current spatial search status
 |
 |  Note(s):
 |
 --*/

void
InquireSearchStatus(pRend, pStatus)
	ddRendererPtr	pRend;
	ddUSHORT	*pStatus;	/* PEXFound or PEXNotFound */
{
    miDDContext		*pddc = (miDDContext *) pRend->pDDContext;

	*pStatus = pddc->Static.search.status;
	return;
}


