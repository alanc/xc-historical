/* $XConsortium: uconvRep.c,v 5.1 91/02/16 09:57:19 rws Exp $ */

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


/*
	Swapping and float conversion routines.

	Variations on a theme by CITI.
 */

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "PEX.h"
#include "PEXproto.h"
#include "PEXprotost.h"
#include "dipex.h"
#include "pexSwap.h"
#include "pex_site.h"
#include "convertStr.h"

#undef LOCAL_FLAG
#define LOCAL_FLAG extern
#include "convUtil.h"
#include "UconvName.h"
#include "OCprim.h"
#include "OCcolour.h"
#include "OCattr.h"

#undef LOCAL_FLAG
#define LOCAL_FLAG
#include "uconvRep.h"

/*************************************************************
	Replies

 *************************************************************/

void
SWAP_FUNC_PREFIX(ConvertGetExtensionInfoReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetExtensionInfoReq	    *strmPtr;
pexGetExtensionInfoReply    *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->majorVersion);
    SWAP_CARD16 (reply->minorVersion);
    SWAP_CARD32 (reply->release);
    SWAP_CARD32 (reply->lengthName);
    SWAP_CARD32 (reply->subsetInfo);
}

void
SWAP_FUNC_PREFIX(ConvertGetEnumeratedTypeInfoReply) (cntxtPtr, strmPtr, reply)
pexContext			*cntxtPtr;
pexGetEnumeratedTypeInfoReq	*strmPtr;
pexGetEnumeratedTypeInfoReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i, j, pad;
    CARD16 *ptr, *optr;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* NOT 0 */

    switch (strmPtr->itemMask) {

	case PEXETIndex : {
		/* PEXETIndex =1 per define in PEX.h */
	    for (i=0, ptr = (CARD16 *)(reply+1); i < reply->numLists; i++) {
		j = *((CARD32 *)ptr);
		pad = j & 1;
		SWAP_CARD32(*((CARD32 *)ptr));
		ptr += 2;
		while (--j >= 0) {
		    SWAP_CARD16((*ptr));
		    ptr++;
		}
		ptr += pad;
	    }

	  break;
	}

	case PEXETMnemonic : {
		/* PEXETMnemonic =2 per define in PEX.h */
	    for (i=0, ptr = (CARD16 *)(reply+1); i < reply->numLists; i++) {
		j = *((CARD32 *)ptr);
		SWAP_CARD32(*((CARD32 *)ptr));
		ptr += 2;
		optr = ptr;
		while (--j >= 0) {
		    pad = *ptr;
		    SWAP_CARD16((*ptr));
		    ptr += 1 + ((pad + 1) >> 1);;
		}
		ptr += (ptr - optr) & 1;
	    }
	  break;
	}

	case PEXETBoth : {
		/* PEXETBOTH =3 per define in PEX.h */
	    for (i=0, ptr = (CARD16 *)(reply+1); i < reply->numLists; i++) {
		j = *((CARD32 *)ptr);
		SWAP_CARD32(*((CARD32 *)ptr));
		ptr += 2;
		optr = ptr;
		while (--j >= 0) {
		    SWAP_CARD16((*ptr));
		    ptr++;
		    pad = *ptr;
		    SWAP_CARD16((*ptr));
		    ptr += 1 + ((pad + 1) >> 1);;
		}
		ptr += (ptr - optr) & 1;
	    }
	  break;
        }

	default: { /* counts */

		/* swap the counts */
	    for (i=0, ptr = (CARD16 *)(reply+1); i < reply->numLists; i++) {
		SWAP_CARD32(*((CARD32 *)ptr));
		ptr += 2;
	    }
	    break; 
	}

    }

    SWAP_CARD32 (reply->numLists);

}

void
SWAP_FUNC_PREFIX(ConvertGetTableInfoReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetTableInfoReq	*strmPtr;
pexGetTableInfoReply *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->definableEntries);
    SWAP_CARD16 (reply->numPredefined);
    SWAP_INT16 (reply->predefinedMin);
    SWAP_INT16 (reply->predefinedMax);
}


void
SWAP_FUNC_PREFIX(ConvertGetPredefinedEntriesReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetPredefinedEntriesReq  *strmPtr;
pexGetPredefinedEntriesReply *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapTable) (   swapPtr, strmPtr->tableType,
				    reply->numEntries,
				    (unsigned char *)(reply+1));

    SWAP_CARD32 (reply->numEntries);
}

void
SWAP_FUNC_PREFIX(ConvertGetDefinedIndicesReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetDefinedIndicesReq	    *strmPtr;
pexGetDefinedIndicesReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    CARD16 *ind;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, ind=(CARD16 *)(reply+1); i < reply->numIndices; i++, ind++)
	SWAP_CARD16((*ind));

    SWAP_CARD32 (reply->numIndices);

}

void
SWAP_FUNC_PREFIX(ConvertGetTableEntryReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetTableEntryReq	*strmPtr;
pexGetTableEntryReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->status);
    SWAP_FUNC_PREFIX(SwapTable) (   swapPtr, reply->tableType, (CARD32)1,
				    (unsigned char *)(reply+1));

    SWAP_CARD16 (reply->tableType);
}

void
SWAP_FUNC_PREFIX(ConvertGetTableEntriesReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetTableEntriesReq	*strmPtr;
pexGetTableEntriesReply *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_FUNC_PREFIX(SwapTable) (   swapPtr, reply->tableType,
				    reply->numEntries,
				    (unsigned char *)(reply+1));
    SWAP_CARD32 (reply->numEntries);
    SWAP_CARD16 (reply->tableType);
}

void
SWAP_FUNC_PREFIX(ConvertGetPipelineContextReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetPipelineContextReq    *strmPtr;
pexGetPipelineContextReply  *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapPipelineContextAttr) (	swapPtr, (strmPtr->itemMask),
						(CARD8 *)(reply+1));
}

void
SWAP_FUNC_PREFIX(ConvertGetRendererAttributesReply) (cntxtPtr, strmPtr, reply)
pexContext			*cntxtPtr;
pexGetRendererAttributesReq	*strmPtr;
pexGetRendererAttributesReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapRendererAttributes) (	swapPtr, strmPtr->itemMask,
						(CARD8 *)(reply+1));
}

void
SWAP_FUNC_PREFIX(ConvertGetRendererDynamicsReply) (cntxtPtr, strmPtr, reply)
pexContext			*cntxtPtr;
pexGetRendererDynamicsReq	*strmPtr;
pexGetRendererDynamicsReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    CARD32 *ptr = (CARD32 *)(reply+1);
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD32 ((*ptr)); ptr++;	/* tables */
    SWAP_CARD32 ((*ptr)); ptr++;	/* namesets */
    SWAP_CARD32 ((*ptr));		/* attributes */
}

void
SWAP_FUNC_PREFIX(ConvertGetStructureInfoReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetStructureInfoReq	    *strmPtr;
pexGetStructureInfoReply    *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->editMode);
    SWAP_CARD32 (reply->elementPtr);
    SWAP_CARD32 (reply->numElements);
    SWAP_CARD32 (reply->lengthStructure);
    SWAP_CARD16 (reply->hasRefs);
}

void
SWAP_FUNC_PREFIX(ConvertGetElementInfoReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetElementInfoReq	*strmPtr;
pexGetElementInfoReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexElementInfo *pe;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, pe=(pexElementInfo *)(reply+1); i < reply->numInfo; i++, pe++)
	SWAP_ELEMENT_INFO((*pe));

    SWAP_CARD32 (reply->numInfo);

}

void
SWAP_FUNC_PREFIX(ConvertGetStructuresInNetworkReply) (cntxtPtr, strmPtr, reply)
pexContext			*cntxtPtr;
pexGetStructuresInNetworkReq	*strmPtr;
pexGetStructuresInNetworkReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexStructure *ps;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, ps=(pexStructure *)(reply+1); i<reply->numStructures; i++, ps++)
	SWAP_STRUCTURE ((*ps));

    SWAP_CARD32 (reply->numStructures);

}


void
SWAP_FUNC_PREFIX(ConvertGetAncestorsReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetAncestorsReq	*strmPtr;
pexGetAncestorsReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    CARD32 i, j, num, *buf;
    pexElementRef *pe;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, buf=(CARD32 *)(reply+1); i<reply->numPaths; i++) {
	num = *buf;
	SWAP_CARD32((*buf));
	buf++;
	for ( j=0, pe=(pexElementRef *)(buf); j<num; j++, pe++)
	    SWAP_ELEMENT_REF((*pe));
	buf = (CARD32 *)pe;
	}

    SWAP_CARD32 (reply->numPaths);
}

/* typedef pexGetAncestorsReply pexGetDescendantsReply; */

void
SWAP_FUNC_PREFIX(ConvertElementSearchReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexElementSearchReq	*strmPtr;
pexElementSearchReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->status);
    SWAP_CARD32 (reply->foundOffset);

}

void
SWAP_FUNC_PREFIX(ConvertFetchElementsReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexFetchElementsReq	*strmPtr;
pexFetchElementsReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexElementInfo *pe;
    CARD32 *curCmd;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    if (cntxtPtr->pexSwapReplyOC) {
	for (i=0, curCmd = (CARD32 *)(reply+1);
	     i<reply->numElements;
	     i++, curCmd += pe->length){
	  pe = (pexElementInfo *)curCmd;
	  if (cntxtPtr->pexSwapReplyOC)
		cntxtPtr->pexSwapReplyOC[ reply->type ]( swapPtr, pe );
	};
    }

    SWAP_CARD32 (reply->numElements);
}

void
SWAP_FUNC_PREFIX(ConvertGetNameSetReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexGetNameSetReq    *strmPtr;
pexGetNameSetReply  *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexName *pn;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, pn=(pexName *)(reply+1); i<reply->numNames; i++, pn++)
	SWAP_NAME((*pn));

    SWAP_CARD32 (reply->numNames);

}

void
SWAP_FUNC_PREFIX(ConvertGetSearchContextReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetSearchContextReq	    *strmPtr;
pexGetSearchContextReply    *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapSearchContext)(swapPtr, strmPtr->itemMask,
					(unsigned char *)(reply+1));

}

void
SWAP_FUNC_PREFIX(ConvertSearchNetworkReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexSearchNetworkReq	*strmPtr;
pexSearchNetworkReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexElementRef *pe;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, pe=(pexElementRef *)(reply+1); i<reply->numItems; i++, pe++)
	SWAP_ELEMENT_REF((*pe));

    SWAP_CARD32 (reply->numItems);

}

void
SWAP_FUNC_PREFIX(ConvertGetWksInfoReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexGetWksInfoReq    *strmPtr;
pexGetWksInfoReply  *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    unsigned char *ptr;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    ptr = (unsigned char *)(reply+1);

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWDisplayUpdate) {
	SWAP_ENUM_TYPE_INDEX ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWVisualState) {
	ptr += sizeof(CARD32);
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWDisplaySurface) {
	ptr += sizeof(CARD32);
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWViewUpdate) {
	ptr += sizeof(CARD32);
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWDefinedViews) {
	int len, i;
	len = (int)(*ptr)*2;			    /* ? */
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr+=sizeof(CARD32);
	for (i=0; i<len; i++, ptr += sizeof(CARD32)) {
	    SWAP_TABLE_INDEX ((*((CARD16 *)ptr)));
	}
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWWksUpdate){
	ptr += sizeof(CARD32);
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWReqNpcSubvolume) {
	SwapNpcSubvolume(swapPtr, (pexNpcSubvolume *)ptr);
	ptr += sizeof(pexNpcSubvolume);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWCurNpcSubvolume) {
	SwapNpcSubvolume(swapPtr, (pexNpcSubvolume *)ptr);
	ptr += sizeof(pexNpcSubvolume);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWReqWksViewport) {
	SwapViewport(swapPtr, (pexViewport *)ptr);
	ptr += sizeof(pexViewport);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWCurWksViewport) {
	SwapViewport(swapPtr, (pexViewport *)ptr);
	ptr += sizeof(pexViewport);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWHlhsrUpdate) {
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWReqHlhsrMode) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWCurHlhsrMode) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWDrawable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWMarkerBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWTextBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWLineBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWInteriorBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWEdgeBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWColourTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWDepthCueTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWLightTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWColourApproxTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWPatternTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWTextFontTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWHighlightIncl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWHighlightExcl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWInvisibilityIncl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWInvisibilityExcl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWPostedStructures) {
	CARD32 len, i;
	pexStructureInfo *ps;

	len = *((CARD32 *)ptr);
	SWAP_CARD32((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	ps = (pexStructureInfo *)ptr;
	for (i=0; i<len; i++, ps++)
	    SWAP_STRUCT_INFO ((*ps));
	ptr = (CARD8 *)ps;
    };

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWNumPriorities) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWBufferUpdate) {
	ptr += sizeof(CARD32);	
    }	

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWReqBufferMode) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);	
    }

    CHECK_BITMASK_ARRAY(strmPtr->itemMask, PEXPWCurBufferMode) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);	
    }


}

void
SWAP_FUNC_PREFIX(ConvertGetDynamicsReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetDynamicsReq	*strmPtr;
pexGetDynamicsReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* 0 */
}

void
SWAP_FUNC_PREFIX(ConvertGetViewRepReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexGetViewRepReq    *strmPtr;
pexGetViewRepReply  *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    pexViewRep *pv;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->viewUpdate);

    pv = (pexViewRep *)(reply+1);

    SwapViewRep(swapPtr, pv);		/* requested */
    pv++;
    SwapViewRep(swapPtr, pv);		/* current   */

}

void
SWAP_FUNC_PREFIX(ConvertMapDCtoWCReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexMapDCtoWCReq	    *strmPtr;
pexMapDCtoWCReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexCoord3D *pc;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD16 (reply->viewIndex);

    for ( i=0, pc=(pexCoord3D *)(reply+1); i<reply->numCoords; i++, pc++)
	SWAP_COORD3D((*pc));

    SWAP_CARD32 (reply->numCoords);

}

void
SWAP_FUNC_PREFIX(ConvertMapWCtoDCReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexMapWCtoDCReq	    *strmPtr;
pexMapWCtoDCReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i;
    pexDeviceCoord *pc;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for (i=0, pc=(pexDeviceCoord *)(reply+1); i<reply->numCoords; i++,pc++)
	SWAP_DEVICE_COORD((*pc));

    SWAP_CARD32 (reply->numCoords);

}

void
SWAP_FUNC_PREFIX(ConvertGetWksPostingsReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetWksPostingsReq	*strmPtr;
pexGetWksPostingsReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    int i, num;
    pexPhigsWks *pi;

    SWAP_CARD16 (reply->sequenceNumber);

    num = (int)((reply->length)/sizeof(pexPhigsWks));
    SWAP_CARD32 (reply->length);	/* not 0 */

    for ( i=0, pi=(pexPhigsWks *)(reply+1); i<num; i++, pi++)
	SWAP_PHIGS_WKS((*pi));

}

void
SWAP_FUNC_PREFIX(ConvertGetPickDeviceReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetPickDeviceReq	*strmPtr;
pexGetPickDeviceReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapPickDevAttr) (	swapPtr, strmPtr->itemMask,
					(unsigned char *)(reply+1));


}

void
SWAP_FUNC_PREFIX(ConvertGetPickMeasureReply) (cntxtPtr, strmPtr, reply)
pexContext		*cntxtPtr;
pexGetPickMeasureReq	*strmPtr;
pexGetPickMeasureReply	*reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    SWAP_FUNC_PREFIX(SwapPickMeasAttr) (    swapPtr, strmPtr->itemMask,
					    (unsigned char *)(reply + 1));

}

void
SWAP_FUNC_PREFIX(ConvertQueryFontReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexQueryFontReq	    *strmPtr;
pexQueryFontReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_CARD32 (reply->lengthFontInfo);

    SWAP_FUNC_PREFIX(SwapFontInfo) (swapPtr, (pexFontInfo *)(reply+1));
}

void
SWAP_FUNC_PREFIX(ConvertListFontsReply) (cntxtPtr, strmPtr, reply)
pexContext	    *cntxtPtr;
pexListFontsReq	    *strmPtr;
pexListFontsReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    SWAP_FUNC_PREFIX(SwapStringList) (	swapPtr, reply->numStrings,
					(pexString *)(reply+1));

    SWAP_CARD32 (reply->numStrings);
}

void
SWAP_FUNC_PREFIX(ConvertListFontsWithInfoReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexListFontsWithInfoReq	    *strmPtr;
pexListFontsWithInfoReply   *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    CARD32 numInfo;
    CARD32 i;
    CARD8 *ptr;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */
    ptr = SWAP_FUNC_PREFIX(SwapStringList) (	swapPtr, reply->numStrings,
						(pexString *)(reply+1));

    SWAP_CARD32 (reply->numStrings);

    numInfo = *((CARD32 *)ptr);
    SWAP_CARD32((*((CARD32 *)ptr)));
    ptr += sizeof(CARD32);

    for (i=0; i<numInfo; i++)
	ptr = SWAP_FUNC_PREFIX(SwapFontInfo) (swapPtr, (pexFontInfo *)ptr);	
}

void
SWAP_FUNC_PREFIX(ConvertQueryTextExtentsReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexQueryTextExtentsReq	    *strmPtr;
pexQueryTextExtentsReply    *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    CARD32 numInfo, *ptr = (CARD32 *)(reply+1);

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    numInfo = *ptr++ / 6;
    SwapExtentInfo (swapPtr, numInfo, (pexExtentInfo *)ptr);
}

void
SWAP_FUNC_PREFIX(ConvertGetImpDepConstantsReply) (cntxtPtr, strmPtr, reply)
pexContext		    *cntxtPtr;
pexGetImpDepConstantsReq    *strmPtr;
pexGetImpDepConstantsReply  *reply;
{
    pexSwap *swapPtr = cntxtPtr->swap;
    CARD16 *pnames = (CARD16 *)(reply+1);
    CARD32 i;
    CARD32 *ptr = (CARD32 *)(reply+1);
    FLOAT *pf;

    SWAP_CARD16 (reply->sequenceNumber);
    SWAP_CARD32 (reply->length);	/* not 0 */

    for (i=0; i<strmPtr->numNames; i++, pnames++, ptr++) {
	switch (*pnames) {
	    case PEXIDDitheringSupported:
	    case PEXIDMaxEdgeWidth:
	    case PEXIDMaxLineWidth:
	    case PEXIDMaxMarkerSize:
	    case PEXIDMaxModelClipPlanes:
	    case PEXIDMaxNameSetNames:
	    case PEXIDMaxNonAmbientLights:
	    case PEXIDMaxNURBOrder:
	    case PEXIDMaxTrimCurveOrder:
	    case PEXIDMinEdgeWidth:
	    case PEXIDMinLineWidth:
	    case PEXIDMinMarkerSize:
	    case PEXIDNominalEdgeWidth:
	    case PEXIDNominalLineWidth:
	    case PEXIDNominalMarkerSize:
	    case PEXIDNumSupportedEdgeWidths:
	    case PEXIDNumSupportedLineWidths:
	    case PEXIDNumSupportedMarkerSizes:
	    case PEXIDBestColourApproximation:
	    case PEXIDTransparencySupported:
						SWAP_CARD32((*ptr));
						break;
	    case PEXIDChromaticityRedU:
	    case PEXIDChromaticityRedV:
	    case PEXIDLuminanceRed:
	    case PEXIDChromaticityGreenU:
	    case PEXIDChromaticityGreenV:
	    case PEXIDLuminanceGreen:
	    case PEXIDChromaticityBlueU:
	    case PEXIDChromaticityBlueV:
	    case PEXIDLuminanceBlue:
	    case PEXIDChromaticityWhiteU:
	    case PEXIDChromaticityWhiteV:
	    case PEXIDLuminanceWhite:
						pf = (FLOAT *)ptr;
						SWAP_FLOAT((*pf));
						break;
	}
    }
}

void
SWAP_FUNC_PREFIX(NoReply)()
{

 }


/****************************************************************
 *  		utilities					*
 ****************************************************************/

unsigned char *
SWAP_FUNC_PREFIX(SwapStringList) (swapPtr, numStrings, stringPtr)
pexSwap		*swapPtr;
CARD32	    	numStrings;
pexString   	*stringPtr;
{
    CARD32 i;
    CARD16 j;
    CARD8 *ptr = (CARD8 *)stringPtr;

    for (i=0; i<numStrings; i++) {
	SWAP_STRING ((*((pexString *)ptr)));
	j = ((pexString *)ptr)->length;
	ptr += j + j%2;
    }
    return (ptr);
}


void
SWAP_FUNC_PREFIX(SwapTable)(swapPtr, TType, num, where)
pexSwap		*swapPtr;
pexTableType	TType;
CARD32		num;
unsigned char	*where;
{
    int i;
    unsigned char *ptr = where;

    switch (TType) { 
	case PEXLineBundleLUT:	{
	    for (i=0; i<num; i++) 
		ptr = SWAP_FUNC_PREFIX(SwapLineBundleEntry) (swapPtr,
						(pexLineBundleEntry *)ptr);
	    break; } 

	case PEXMarkerBundleLUT:	{ 
	    for (i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapMarkerBundleEntry) (swapPtr,
						(pexMarkerBundleEntry *)ptr);
	    break; }

	case PEXTextBundleLUT:	{ 
	    for (i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapTextBundleEntry) (swapPtr,
						(pexTextBundleEntry *)ptr);
	    break; }

	case PEXInteriorBundleLUT:	{ 
	    for (i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapInteriorBundleEntry) (swapPtr,
						(pexInteriorBundleEntry *)(ptr));
	    break; }

	case PEXEdgeBundleLUT:	{ 
	    for (i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapEdgeBundleEntry) (swapPtr,
						(pexEdgeBundleEntry *)(ptr));
	    break; }

	case PEXPatternLUT:	{ 
	    pexPatternEntry *pe;
	    for (i=0; i<num; i++) {
		pe = (pexPatternEntry *)ptr;
		ptr = SWAP_FUNC_PREFIX(SwapPatternEntry) (  swapPtr, pe,
							    pe->numx, pe->numy);
		SWAP_CARD16 (pe->numx);
		SWAP_CARD16 (pe->numy);
	    };
	    break; }

	case PEXTextFontLUT:	{
	    for ( i=0; i<num; i++, ptr += sizeof(pexFont)) {
		SWAP_FONT ((*(pexFont *)ptr));
	    }
	    break; }

	case PEXColourLUT:	{ 
	    for ( i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						(pexColourSpecifier *)ptr);
	    break; }

	case PEXViewLUT:	{ 
	    pexViewEntry *pv = (pexViewEntry *)ptr;
	    for ( i=0; i<num; i++, pv++)
		SwapViewEntry (swapPtr, pv);
	    break; }

	case PEXLightLUT:	{ 
	    for ( i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapLightEntry) (swapPtr,
						    (pexLightEntry *)(ptr));
	    break; }

	case PEXDepthCueLUT:	{ 
	    for ( i=0; i<num; i++)
		ptr = SWAP_FUNC_PREFIX(SwapDepthCueEntry) (swapPtr,
						(pexDepthCueEntry *)(ptr));
	    break; }

	case PEXColourApproxLUT:	{ 
	    extern void SwapColourApproxEntry();
	    pexColourApproxEntry *pa = (pexColourApproxEntry *)ptr;
	    for ( i=0; i<num; i++, pa++)
		SwapColourApproxEntry( swapPtr, pa);
	    break; }

	}
}


void
SWAP_FUNC_PREFIX(SwapSearchContext)(swapPtr, im, ptr) 
pexSwap		*swapPtr;
CARD32		im;
unsigned char	*ptr;
{
    unsigned char *sc_data = ptr;

    if (im & PEXSCPosition) {
	SWAP_COORD3D ((*((pexCoord3D *)sc_data)));
	sc_data += sizeof(pexCoord3D);
    };

    if (im & PEXSCDistance) {
	SWAP_FLOAT ((*((FLOAT *)sc_data)));
	sc_data += sizeof(FLOAT);
    };

    if (im & PEXSCCeiling) {
	SWAP_CARD16 ((*((CARD16 *)sc_data)));
	sc_data += sizeof(CARD32);	    /* include pad */
    }

    if (im & PEXSCStartPath) {
	int len, i;
	len = (int)(*sc_data);
	SWAP_CARD32 ((*((CARD32 *)sc_data)));
	sc_data += sizeof(CARD32);
	for (i=0; i<len; i++, sc_data += sizeof(pexElementRef)) {
	    SWAP_ELEMENT_REF ((*((pexElementRef *)sc_data)));
	}
    };

    if (im & PEXSCNormalList) {
	int len, i;
	len = (int)(*sc_data);
	SWAP_CARD32 ((*((CARD32 *)sc_data)));
	for (i=0; i<len; i++, sc_data += sizeof(CARD32)) {
	    SWAP_NAME ((*((CARD32 *)sc_data)));
	}

    };

    if (im & PEXSCInvertedList) {
	int len, i;
	len = (int)(*sc_data);
	SWAP_CARD32 ((*((CARD32 *)sc_data)));
	for (i=0; i<len; i++, sc_data += sizeof(CARD32)) {
	    SWAP_NAME ((*((CARD32 *)sc_data)));
	}

    };

}


CARD8 *
SWAP_FUNC_PREFIX(SwapFontInfo) (swapPtr, pfi)
pexSwap		*swapPtr;
pexFontInfo	*pfi;
{
    CARD8 *ptr;
    CARD32 i;
    pexFontProp *pfp;

    SWAP_CARD32 (pfi->firstGlyph);
    SWAP_CARD32 (pfi->lastGlyph);
    SWAP_CARD32 (pfi->defaultGlyph);

    pfp = (pexFontProp *)(pfi+1);
    for (i=0; i<pfi->numProps; i++)
	pfp = (pexFontProp *)(SwapFontProp(swapPtr, pfp));

    SWAP_CARD32 (pfi->numProps);

    ptr = (CARD8 *)pfp;
    return (ptr);
}


void
SWAP_FUNC_PREFIX(SwapPickMeasAttr) (swapPtr, im, p_data)
pexSwap		*swapPtr;
CARD32		im;
unsigned char	*p_data;
{
    unsigned char *ptr = p_data;

    if (im & PEXPMStatus) {
	SWAP_CARD32((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXPMPath) {
	CARD32 i;
	CARD32 numRefs = *((CARD32 *)ptr);
	pexPickPath *pp;
	ptr += sizeof(CARD32);
	SWAP_CARD32((*((CARD32 *)ptr)));
	for (i=0, pp = (pexPickPath *)ptr; i<numRefs; i++, pp++) {
	    SWAP_PICK_PATH((*pp));
	}
    }
}


unsigned char *
SWAP_FUNC_PREFIX(SwapLightEntry) (swapPtr, p_data) 
pexSwap		    *swapPtr;
pexLightEntry	    *p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_ENUM_TYPE_INDEX (p_data->lightType);
    SWAP_VECTOR3D (p_data->direction);
    SWAP_COORD3D (p_data->point);
    SWAP_FLOAT (p_data->concentration);
    SWAP_FLOAT (p_data->spreadAngle);
    SWAP_FLOAT (p_data->attenuation1);
    SWAP_FLOAT (p_data->attenuation2);

    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (   swapPtr,
						    &(p_data->lightColour));

    return (ptr);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapLineBundleEntry) (swapPtr, p_data) 
pexSwap		    *swapPtr;
pexLineBundleEntry  *p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_ENUM_TYPE_INDEX (p_data->lineType); 
    SWAP_ENUM_TYPE_INDEX (p_data->polylineInterp); 
    SWAP_FLOAT (p_data->lineWidth); 
    SWAP_CURVE_APPROX (p_data->curveApprox);
    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier)(swapPtr, &(p_data->lineColour));

    return (ptr);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapMarkerBundleEntry) (swapPtr, p_data) 
pexSwap			*swapPtr;
pexMarkerBundleEntry	*p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_ENUM_TYPE_INDEX (p_data->markerType); 
    SWAP_FLOAT (p_data->markerScale); 
    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (   swapPtr,
						    &(p_data->markerColour));

    return (ptr);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapTextBundleEntry) (swapPtr, p_data) 
pexSwap		    *swapPtr;
pexTextBundleEntry  *p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_CARD16 (p_data->textFontIndex); 
    SWAP_CARD16 (p_data->textPrecision); 
    SWAP_FLOAT (p_data->charExpansion); 
    SWAP_FLOAT (p_data->charSpacing); 
    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier)(swapPtr, &(p_data->textColour));

    return (ptr);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapInteriorBundleEntry) (swapPtr, p_data) 
pexSwap			*swapPtr;
pexInteriorBundleEntry	*p_data;
{
    unsigned char *po;
    SWAP_ENUM_TYPE_INDEX (p_data->interiorStyle); 
    SWAP_INT16 (p_data->interiorStyleIndex);
    SWAP_ENUM_TYPE_INDEX (p_data->reflectionModel); 
    SWAP_ENUM_TYPE_INDEX (p_data->surfaceInterp); 
    SWAP_ENUM_TYPE_INDEX (p_data->bfInteriorStyle); 
    SWAP_INT16 (p_data->bfInteriorStyleIndex);
    SWAP_ENUM_TYPE_INDEX (p_data->bfReflectionModel); 
    SWAP_ENUM_TYPE_INDEX (p_data->bfSurfaceInterp); 

    SwapSurfaceApprox (swapPtr, &(p_data->surfaceApprox));
    po = (unsigned char *)(p_data+1);
    po = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						(pexColourSpecifier *)po);
    po = SWAP_FUNC_PREFIX(SwapReflectionAttr) (	swapPtr,
						(pexReflectionAttr *)po);
    po = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						(pexColourSpecifier *)po);
    po = SWAP_FUNC_PREFIX(SwapReflectionAttr) (	swapPtr,
						(pexReflectionAttr *)po); 

    return (po);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapEdgeBundleEntry) (swapPtr, p_data) 
pexSwap		    *swapPtr;
pexEdgeBundleEntry  *p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_CARD16 (p_data->edges);
    SWAP_ENUM_TYPE_INDEX (p_data->edgeType); 
    SWAP_FLOAT (p_data->edgeWidth);
    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier)(swapPtr, &(p_data->edgeColour));

    return (ptr);
}


unsigned char *
SWAP_FUNC_PREFIX(SwapDepthCueEntry) (swapPtr, p_data) 
pexSwap		    *swapPtr;
pexDepthCueEntry    *p_data;
{
    unsigned char *ptr = (unsigned char *)p_data;
    SWAP_FLOAT (p_data->frontPlane);
    SWAP_FLOAT (p_data->backPlane);
    SWAP_FLOAT (p_data->frontScaling);
    SWAP_FLOAT (p_data->backScaling);
    ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (   swapPtr,
						    &(p_data->depthCueColour));

    return (ptr);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapPatternEntry) (swapPtr, p_data, numx, numy) 
pexSwap		    *swapPtr;
pexPatternEntry	    *p_data;
CARD16		    numx;
CARD16		    numy;
{
    int i, max_colours;
    pexColour *pc = (pexColour *)(p_data + 1);

    max_colours = numx * numy;
    for (i=0; i<max_colours; i++) 
	pc = (pexColour *) SwapColour (swapPtr, pc, p_data->colourType);

    SWAP_COLOUR_TYPE (p_data->colourType);  

    return ((unsigned char *)pc);
}

unsigned char *
SWAP_FUNC_PREFIX(SwapPipelineContextAttr) (swapPtr, itemMask, p_data)
pexSwap	*swapPtr;
CARD32 	*itemMask;
CARD8 	*p_data;
{
    CARD8 *ptr = p_data;

    if (itemMask[0] & PEXPCMarkerType) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCMarkerScale) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCMarkerColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[0] & PEXPCMarkerBundleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCTextFont) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCTextPrecision) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCCharExpansion) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCCharSpacing) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCTextColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[0] & PEXPCCharHeight) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCCharUpVector) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCTextPath) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCTextAlignment) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD16);
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD16);
    }

    if (itemMask[0] & PEXPCAtextHeight) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCAtextUpVector) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCAtextPath) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCAtextAlignment) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD16);
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD16);
    }

    if (itemMask[0] & PEXPCAtextStyle) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCTextBundleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCLineType) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCLineWidth) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCLineColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[0] & PEXPCCurveApproximation) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[0] & PEXPCPolylineInterp) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCLineBundleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCInteriorStyle) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCInteriorStyleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCSurfaceColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[0] & PEXPCSurfaceReflAttr) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[0] & PEXPCSurfaceReflModel) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCSurfaceInterp) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[0] & PEXPCBfInteriorStyle) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCBfInteriorStyleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCBfSurfaceColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[1] & PEXPCBfSurfaceReflAttr) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[1] & PEXPCBfSurfaceReflModel) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCBfSurfaceInterp) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCSurfaceApproximation) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCCullingMode) {
	SWAP_CARD16((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCDistinguishFlag) {
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCPatternSize) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCPatternRefPt) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCPatternRefVec1) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCPatternRefVec2) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCInteriorBundleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCSurfaceEdgeFlag) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCSurfaceEdgeType) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCSurfaceEdgeWidth) {
	SWAP_FLOAT ((*((FLOAT *)ptr)));
	ptr += sizeof(FLOAT);
    }

    if (itemMask[1] & PEXPCSurfaceEdgeColour) {
	ptr = SWAP_FUNC_PREFIX(SwapColourSpecifier) (swapPtr,
						     (pexColourSpecifier *)ptr);
    }

    if (itemMask[1] & PEXPCEdgeBundleIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCLocalTransform) {
	int i, j;
	for (i=0; i<4; i++)
	    for (j=0; j<4; j++) {
		SWAP_FLOAT((*((FLOAT *)ptr)));
		ptr += sizeof(FLOAT);
	    }
    }

    if (itemMask[1] & PEXPCGlobalTransform) {
	int i, j;
	for (i=0; i<4; i++)
	    for (j=0; j<4; j++) {
		SWAP_FLOAT((*((FLOAT *)ptr)));
		ptr += sizeof(FLOAT);
	    }
    }

    if (itemMask[1] & PEXPCModelClip) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCModelClipVolume) {
	CARD32 i, numHalfSpace;
	pexHalfSpace *ph;
	numHalfSpace = *((CARD32 *)ptr);
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	for (i=0, ph = (pexHalfSpace *)ptr; i<numHalfSpace; i++, ph++) {
	    SwapHalfSpace (swapPtr, ph);
	}
	ptr = (CARD8 *)ph;
    }

    if (itemMask[1] & PEXPCViewIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCLightState) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCDepthCueIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCSetAsfValues) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCPickId) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCHlhsrIdentifier) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCNameSet) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCColourApproxIndex) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCRenderingColourModel) {
	SWAP_CARD16((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (itemMask[1] & PEXPCParaSurfCharacteristics) {
	INT16 type;
	type = *((INT16 *)ptr);
	SWAP_INT16((*((INT16 *)ptr)));
	ptr += sizeof(CARD16);
	SWAP_CARD16((*((CARD16 *)ptr)));
	ptr += sizeof(CARD16);
	switch (type) {
	    case PEXPSCNone:
	    case PEXPSCImpDep: break;

	    case PEXPSCIsoCurves: {
		SWAP_CARD16((*((CARD16 *)ptr)));
		ptr += sizeof(CARD16);
		SWAP_CARD16((*((CARD16 *)ptr)));
		ptr += sizeof(CARD16);
		SWAP_CARD16((*((CARD16 *)ptr)));
		ptr += sizeof(CARD16);
		SWAP_CARD16((*((CARD16 *)ptr)));
		ptr += sizeof(CARD16);
		break;
	    }

	    case PEXPSCMcLevelCurves:
	    case PEXPSCWcLevelCurves: {
		CARD16 i, num;
		SWAP_COORD3D((*((pexCoord3D *)ptr)));
		ptr += sizeof(pexCoord3D);
		SWAP_VECTOR3D((*((pexVector3D *)ptr)));
		ptr += sizeof(pexVector3D);
		num = *((CARD16 *)ptr);
		SWAP_CARD16((*((CARD16 *)ptr)));
		ptr += sizeof(CARD32);
		for (i=0; i<num; i++) {
		    SWAP_COORD3D((*((pexCoord3D *)ptr)));
		    ptr += sizeof(pexCoord3D);
		}
	    }
	}
    }


    return (ptr);
}

void
SWAP_FUNC_PREFIX(SwapPickDevAttr) (swapPtr, im, pdata)
pexSwap		*swapPtr;
CARD32		im;
unsigned char	*pdata;
{
    unsigned char *ptr = pdata;
    int len, i;

    if (im & PEXPDPickStatus) {
	ptr += sizeof(CARD32);
    };

    if (im & PEXPDPickPath) {
	len = (int)(*ptr);
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	for (i=0; i<len; i++, ptr += sizeof(pexPickPath)) {
	    SWAP_PICK_PATH ((*((pexPickPath *)ptr)));
	};
    };

    if (im & PEXPDPickPathOrder) {
	ptr += sizeof(CARD32);
    };

    if (im & PEXPDPickIncl) {
	SWAP_NAMESET ((*((pexNameSet *)ptr)));
	ptr += sizeof(pexNameSet);
    };

    if (im & PEXPDPickExcl) {
	SWAP_NAMESET ((*((pexNameSet *)ptr)));
	ptr += sizeof(pexNameSet);
    };

    if (im & PEXPDPickDataRec) {
	len = (int)(*ptr);
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	ptr += (len + 3)/4;		    /*	pad it out  */
    };

    if (im & PEXPDPickPromptEchoType) {
	SWAP_ENUM_TYPE_INDEX ((*ptr));
	ptr += sizeof(pexEnumTypeIndex);
    };

    if (im & PEXPDPickEchoVolume) {
	SwapViewport(swapPtr, (pexViewport *)ptr);
	ptr += sizeof(pexViewport);	
    }
}


void
SWAP_FUNC_PREFIX(SwapRendererAttributes) (swapPtr, im, p_data)
pexSwap	*swapPtr;
CARD32	im;
CARD8	*p_data;
{
    CARD8 *ptr = p_data;
    CARD32 num, i;

    if (im & PEXRDPipelineContext) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDCurrentPath) {
	num = *((CARD32 *)ptr);
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	for (i=0; i<num; i++, ptr += sizeof(pexElementRef))
	    SWAP_ELEMENT_REF((*((pexElementRef *)ptr)));
    }

    if (im & PEXRDMarkerBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDTextBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDLineBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDInteriorBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDEdgeBundle) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDViewTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDColourTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDDepthCueTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDLightTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDColourApproxTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDPatternTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDTextFontTable) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDHighlightIncl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDHighlightExcl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDInvisibilityIncl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDInvisibilityExcl) {
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDRendererState) {
	SWAP_CARD16 ((*((CARD16 *)ptr)));
	ptr += sizeof(CARD32);
    }

    if (im & PEXRDHlhsrMode) {
	SWAP_INT16 ((*((INT16 *)ptr)));
	ptr += sizeof(CARD32);
	
    }

    if (im & PEXRDNpcSubvolume) {
	SwapNpcSubvolume (swapPtr, (pexNpcSubvolume *)ptr);
	ptr += sizeof(pexNpcSubvolume);
    }

    if (im & PEXRDViewport) {
	SwapViewport (swapPtr, (pexViewport *)ptr);
	ptr += sizeof(pexViewport);
    }

    if (im & PEXRDClipList) {
	num = *((CARD32 *)ptr);
	SWAP_CARD32 ((*((CARD32 *)ptr)));
	ptr += sizeof(CARD32);
	SwapDeviceRects (swapPtr, num, (pexDeviceRect *)ptr);
    }
}
