/************************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

/* $XConsortium: dixfonts.c,v 1.9 89/07/16 17:24:57 rws Exp $ */

#define NEED_REPLIES
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "cursorstr.h"
#include "misc.h"
#include "opaque.h"
#include "dixfontstr.h"
#include "osstruct.h"

#define QUERYCHARINFO(pci, pr)  *(pr) = (pci)->metrics

extern pointer fosNaturalParams;

extern EncodedFontPtr 	defaultFont;

/*
 * adding RT_FONT prevents conflict with default cursor font
 */
Bool
SetDefaultFont( defaultfontname)
    char *	defaultfontname;
{
extern	EncodedFontPtr	OpenFont();
	EncodedFontPtr	pf;

    pf = OpenFont( (unsigned)strlen( defaultfontname), defaultfontname);
    if ((pf==NullEncodedFont) || 
	 (!AddResource(FakeClientID(0), RT_FONT, (pointer)pf)))
	return FALSE;
    defaultFont = pf;
    return TRUE;
}

/*
 * Check reference count first, load font only if necessary.
 */
EncodedFontPtr 
OpenFont(lenfname, pfontname)
    unsigned	lenfname;
    char *	pfontname;
{
    EncodedFontPtr 	pfont= NullEncodedFont;
    int		i;
    ScreenPtr	pscr;
    Mask	unread;

    unread = fpLookupFont(pfontname,lenfname,&pfont,FONT_STANDARD_TABLES,
							 fosNaturalParams);
    if (unread!=0) {
#ifdef notdef
	ErrorF(  "OpenFont: read failed on file %s\n", ppathname);
#endif
	return NullEncodedFont;
    }


    if (pfont->refcnt != 0) {
	pfont->refcnt += 1;
	return pfont;
    }

    /*
     * this is a new font, set up pfont->pCS->ppCI[*].pPriv to point
     * to the character bitmaps.
     * Once we've copied the pictures, we can free pBitOffsets to save
     * memory but we have to be careful to free the bitmaps (if necessary)
     * in CloseFont (XXX! Not implemented yet)
     */
     for (i=0;i<pfont->pCS->nChars;i++) {
	pfont->pCS->ci.pCI[i].pPriv=	(pointer)pfont->pCS->pBitOffsets[i];
     }

    /*
     * since this font has been newly read off disk, ask each screen to
     * realize it.
     */
    pfont->refcnt = 1;
#ifdef totallybogusifdef
    pfont->devPriv= (pointer *)Xalloc(sizeof(pointer)*MAXSCREENS);
    if (pfont->devPriv==NULL) {
	fpUnloadFont(pfont,TRUE);
	return(NullEncodedFont);
    }
    bzero(pfont->devPriv,sizeof(pointer)*MAXSCREENS);
#endif

    for (i = 0; i < screenInfo.numScreens; i++) {
	pscr = screenInfo.screens[i];
        if ( pscr->RealizeFont)
	    ( *pscr->RealizeFont)( pscr, pfont);
    }
    return pfont;
}

/*
 * Decrement font's ref count, and free storage if ref count equals zero
 */
/*ARGSUSED*/
int
CloseFont(pfont, fid)
    EncodedFontPtr 	pfont;
    Font	fid;
{
    int		nscr;
    ScreenPtr	pscr;

    if (pfont == NullEncodedFont)
        return(Success);
    if (--pfont->refcnt == 0)
    {
	/*
	 * since the last reference is gone, ask each screen to
	 * free any storage it may have allocated locally for it.
	 */
	for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
	{
	    pscr = screenInfo.screens[nscr];
	    if ( pscr->UnrealizeFont)
		( *pscr->UnrealizeFont)( pscr, pfont);
	}
	if (pfont == defaultFont)
	    defaultFont = NULL;
#ifdef totallybogusifdef
	if (pfont->devPriv) {
	    Xfree(pfont->devPriv);
	    pfont->devPriv=	NULL;
	}
#endif
	fpUnloadFont(pfont,TRUE);
    }
    return(Success);
}


/***====================================================================***/

	/*\
	 * Sets up pReply as the correct QueryFontReply for
	 * pFont with the first nProtoCCIStructs char infos.
	\*/

/* 5/23/89 (ef) -- XXX! Does this already exist somewhere? */
static xCharInfo xciNoSuchChar = { 0,0,0,0,0,0};

void
QueryFont( pFont, pReply, nProtoCCIStructs)
    EncodedFontPtr 	 pFont;
    xQueryFontReply	*pReply;	/* caller must allocate this storage */
    int			 nProtoCCIStructs;
{
    CharSetPtr 		 pCS = pFont->pCS;
    CharInfoPtr 	 *ppCI;
    FontPropPtr		 pFP;
    int			 i;
    xFontProp 		*prFP;
    xCharInfo 		*prCI;

    /* pr->length set in dispatch */
    pReply->minCharOrByte2=	pFont->firstCol;
    pReply->defaultChar=	pFont->defaultCh;
    pReply->maxCharOrByte2=	pFont->lastCol;
    pReply->drawDirection=	pCS->drawDirection;
    pReply->allCharsExist=	pFont->allExist;
    pReply->minByte1=		pFont->firstRow;
    pReply->maxByte1=		pFont->lastRow;
    pReply->fontAscent=		pCS->fontAscent;
    pReply->fontDescent=	pCS->fontDescent;

    pReply->minBounds=	pFont->inkMin;
    pReply->maxBounds=	pFont->inkMax;

    pReply->nFontProps= pCS->nProps; 
    pReply->nCharInfos=	nProtoCCIStructs; 


    for ( i=0, pFP=pCS->props, prFP=(xFontProp *)(&pReply[1]);
	  i < pCS->nProps;
	  i++, pFP++, prFP++)
    {
	prFP->name=	pFP->name;
	prFP->value=	pFP->value;
    }

    for ( i=0, ppCI = pFont->ppInkCI, prCI=(xCharInfo *)(prFP);
	  i<nProtoCCIStructs;
	  i++, ppCI++, prCI++) {
	if (*ppCI)	*prCI=	(*ppCI)->metrics;
	else		*prCI=	xciNoSuchChar;
    }
    return;
}

typedef struct _LFWIclosure {
    ClientPtr		    client;
    FontPathPtr		    fpaths;
    int			    current;
    xListFontsWithInfoReply *reply;
    int			    length;
} LFWIclosureRec, *LFWIclosurePtr;

pointer
StartListFontsWithInfo (client, length, pattern, maxNames)
    ClientPtr	client;
    int		length;
    char	*pattern;
    int		maxNames;
{
    LFWIclosurePtr  c;

    c = (LFWIclosurePtr) xalloc (sizeof (*c));
    if (!c)
	return NULL;
    c->client = client;
    c->fpaths = ExpandFontNamePattern (length, pattern, maxNames);
    if (!c->fpaths)
    {
	xfree (c);
	return NULL;
    }
    c->current = 0;
    c->reply = 0;
    c->length = 0;
    return (pointer) c;
}

int
NextListFontsWithInfo (closure, replyP, replyLengthP, nameP, nameLengthP)
    pointer		    closure;
    xListFontsWithInfoReply **replyP;
    int			    *replyLengthP;
    char		    **nameP;
    int			    *nameLengthP;
{
    LFWIclosurePtr	    c;
    char		    *name;
    int			    nameLength;
    int			    replyLength;
    xListFontsWithInfoReply *reply, *nreply;
    EncodedFontPtr	    pRtrnFont;
    Mask		    unread;
    CharSetRec		    charset;
    EncodedFontRec	    font;

    c = (LFWIclosurePtr) closure;
    for (;;)
    {
	if (c->current == c->fpaths->npaths)
	    return 0;
    	bzero (&font, sizeof (font));
    	bzero (&charset, sizeof (charset));
    	pRtrnFont = &font;
    	FONTCHARSET(&font) = &charset;
    	name = c->fpaths->paths[c->current];
    	nameLength = c->fpaths->length[c->current];
    	c->current++;
    	unread = fpLookupFont (name, nameLength, &pRtrnFont,
			       FONT_BDF_ENCODINGS|FONT_METRICS|FONT_INK_METRICS|
			       FONT_ACCELERATORS|FONT_PROPERTIES,fosNaturalParams);
    	if (pRtrnFont != NULL && unread == 0)
    	{
    	    replyLength = sizeof (xListFontsWithInfoReply) +
		      	  pRtrnFont->pCS->nProps * sizeof (xFontProp);
    	    if (replyLength > c->length)
    	    {
	    	nreply = (xListFontsWithInfoReply *) xalloc (replyLength);
	    	if (nreply)
	    	{
	    	    xfree (c->reply);
	    	    c->reply = nreply;
	    	    c->length = replyLength;
	    	}
    	    }
	    /* loop termination -
 	     * found a real font and enough space for reply
	     */
    	    if (replyLength <= c->length)
	    	break;
    	}
    }
    reply = c->reply;
    reply->type = X_Reply;
    reply->length = (replyLength - sizeof(xGenericReply)
		     + nameLength + 3) >> 2;
    QueryFont(pRtrnFont, (xQueryFontReply *) reply, 0);
    reply->sequenceNumber = c->client->sequence;
    reply->nameLength = nameLength;
    reply->nReplies = c->fpaths->npaths - c->current;
    *replyP = reply;
    *replyLengthP = replyLength;
    *nameP = name;
    *nameLengthP = nameLength;
    if (pRtrnFont == &font)
	fpUnloadFont (pRtrnFont, FALSE);
    return 1;
}

FinishListFontsWithInfo (closure)
    pointer closure;
{
    LFWIclosurePtr  c;

    c = (LFWIclosurePtr) closure;
    FreeFontRecord (c->fpaths);
    xfree (c->reply);
    xfree (c);
}
