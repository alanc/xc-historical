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


#include "X.h"
#include "Xproto.h"
#include "PEXproto.h"
#include "misc.h"
#include "os.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "pexUtils.h"

#define LOCAL_FLAG
#include "dipex.h"
#include "pexSwap.h"
#undef LOCAL_FLAG


unsigned long add_pad_of[] = {0, 3, 2, 1};

unsigned int ColourSpecSizes[] = {
    sizeof(CARD32),			    /* PEXIndexedColour */
    3 * sizeof(FLOAT),			    /* PEXRgbFloatColour */
    3 * sizeof(FLOAT),			    /* PEXCieFloatColour */
    3 * sizeof(FLOAT),			    /* PEXHsvFloatColour */
    3 * sizeof(FLOAT),			    /* PEXHlsFloatColour */
    sizeof(CARD32),			    /* PEXRgb8Colour */
    2 * sizeof(CARD32),			    /* PEXRgb16Colour */
};

extern ddpex43rtn ddpexInit();

extern ddBuffer *pPEXBuffer;
extern ddBuffer *diInitPEXBuffer();
extern RequestFunction PEXRequest[];
extern CARD32 SwapCARD32();
extern CARD16 SwapCARD16();
static int DeletePexClient();

/*
 ******************************************************************************
 *
 *	Function:	PexExtensionInit
 *
 ******************************************************************************
 */
PexExtensionInit()
{
    ExtensionEntry *AddExtension();
    ExtensionEntry *PextEntry = 0;
    int ProcPEXDispatch();
    void PEXResetProc();
    extern int	DeleteStructure(), FreeSearchContext(), FreeRenderer(),
		FreePickMeasure(), dipexFreePhigsWks(), FreePipelineContext(),
		FreeNameSet(), FreeLUT(), FreePEXFont(), FreeWksDrawable();
    extern ddpex43rtn OpenPEXFont();
    extern Atom MakeAtom();

    (void) MakeAtom (PEX_NAME_STRING, (unsigned)(strlen(PEX_NAME_STRING)), TRUE);
    PEXStructType   = CreateNewResourceType (DeleteStructure);
    PEXSearchType   = CreateNewResourceType (FreeSearchContext);
    PEXRendType	    = CreateNewResourceType (FreeRenderer);
    PEXPickType	    = CreateNewResourceType (FreePickMeasure);
    PEXWksType	    = CreateNewResourceType (dipexFreePhigsWks);
    PEXPipeType	    = CreateNewResourceType (FreePipelineContext);
    PEXNameType	    = CreateNewResourceType (FreeNameSet);
    PEXLutType	    = CreateNewResourceType (FreeLUT);
    PEXFontType	    = CreateNewResourceType (FreePEXFont);
    PEXContextType  = CreateNewResourceType (DeletePexClient);

    PEXClass	    = CreateNewResourceClass();
    PEXWksDrawableType
		    = CreateNewResourceType (FreeWksDrawable);

    if (!PEXStructType || !PEXSearchType || !PEXRendType || !PEXWksDrawableType
	|| !PEXWksType || !PEXPipeType   || !PEXNameType || !PEXFontType 
	|| !PEXLutType || !PEXPickType   || !PEXClass    || !PEXContextType ) {
	    ErrorF("PEXExtensionInit: Could not create PEX resource types");
	    return;
	}

    /*
	Init buffer for varying-sized replies from ddpex
     */
    pPEXBuffer = diInitPEXBuffer();

    if (!pPEXBuffer) {
	ErrorF("PEXExtensionInit: failed to alloc buffer\n");
	return; }

    /*
     *	Init any device dependent stuff
     */

    if (ddpexInit()) {
	ErrorF("PEXExtensionInit: Couldn't init ddPEX!");
	return; }

    /*
     *  Open up the default font
     */
    defaultPEXFont=(diFontHandle)Xalloc((unsigned long)(sizeof(ddFontResource)));
    if (!defaultPEXFont) {
	ErrorF("PEXExtensionInit: Memory error--could not allocate default PEX font");
	return; }
	
    defaultPEXFont->id = FakeClientID(0);
    
    if ( OpenPEXFont(	(ddULONG)(strlen(DEFAULT_PEX_FONT_NAME)), 
			(ddUCHAR *)(DEFAULT_PEX_FONT_NAME),
			defaultPEXFont) != Success) {
	char errmsg[80];
	char *static_message =
		    "PEXExtensionInit: Couldn't open default PEX font file ";
	bcopy(	static_message, errmsg, strlen(static_message));
	bcopy(	DEFAULT_PEX_FONT_NAME,&(errmsg[strlen(&(errmsg[0]))]),
		strlen(DEFAULT_PEX_FONT_NAME));
	ErrorF(errmsg);
	Xfree(defaultPEXFont);
	defaultPEXFont = 0; 
	return; }
    
    if (!AddResource(	defaultPEXFont->id, PEXFontType,
			(pointer)(defaultPEXFont))) {
	ErrorF("PEXExtensionInit: Couldn't add default PEX font resource.");
	return;
    }

    PextEntry = AddExtension(   PEX_NAME_STRING, 0, (PEXMaxError+1),
				    ProcPEXDispatch, ProcPEXDispatch,
				    PEXResetProc, StandardMinorOpcode);
    if (!PextEntry) {
	ErrorF("PEXExtensionInit: AddExtensions failed\n");
	return; }

    PexReqCode = PextEntry->base;
    PexErrorBase = PextEntry->errorBase;
    
}


/*
 ******************************************************************************
 *
 *	Function:	InitPexClient
 *
 ******************************************************************************
 */
pexContext *
InitPexClient(client)
ClientPtr client;
{
    pexContext *cntxtPtr;
    XID pexId;

    /* Allocate space for the context table */
    cntxtPtr = (pexContext *) Xalloc( (unsigned long)(sizeof( pexContext )
						+ sizeof(pexSwap)) );
    cntxtPtr->swap = (pexSwap *)(cntxtPtr+1);	/* this is silly */

    /* Create the resource id */
    pexId = PEXID( client, PEXCONTEXTTABLE );

    /* Add the connection context as a resource */
    if (!AddResource (pexId, PEXContextType, (pointer)cntxtPtr)) {
	unsigned short temp;
	temp = MinorOpcodeOfRequest(client);
	SendErrorToClient(  client, (unsigned) PexReqCode,
			    (temp), (unsigned long)0, (int) (BadAlloc));
	Xfree((pointer)cntxtPtr);
	return(0); }

    /* setup the default proc vectors */
    cntxtPtr->pexRequest	    = PEXRequest;
    cntxtPtr->pexSwapReply	    = 0;
    cntxtPtr->pexSwapRequestOC	    = 0;
    cntxtPtr->pexSwapReplyOC	    = 0;
    cntxtPtr->swap->ConvertCARD16   = SwapCARD16;
    cntxtPtr->swap->ConvertCARD32   = SwapCARD32;
    cntxtPtr->swap->ConvertFLOAT    = 0;

    /* save the client ptr */
    cntxtPtr->client = client;

    return( cntxtPtr );

}

/*
 ******************************************************************************
 *
 *	Function:	static DeletePexClient
 *
 *	On Entry:	Client quitting, Pex resources need to be freed
 *
 *	On Exit:	Pex client resources freed.
 *
 *	Notes:
 *
 ******************************************************************************
 */

static DeletePexClient(cntxtPtr, pexId)
pexContext *cntxtPtr;
int pexId;
{

    /* Free space allocated for the client context */
    if (cntxtPtr) Xfree( (pointer)cntxtPtr );

}


void PEXResetProc()
{
    extern ddpex43rtn FreePEXFont();
    extern void ddpexReset();
    diFontHandle    temp = defaultPEXFont;

    defaultPEXFont = 0;	/* force free */
    FreeResource(temp->id, RT_NONE);

    ddpexReset();
}


ErrorCode
PEXGenericResourceRequest (cntxtPtr, strmPtr)
pexContext *cntxtPtr;
pexReq *strmPtr;
{ }

