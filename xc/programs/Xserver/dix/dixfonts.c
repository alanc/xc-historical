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

/* $XConsortium: dixfonts.c,v 1.19 91/05/11 10:26:16 keith Exp $ */

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
#include "closestr.h"

#ifdef DEBUG
#include	<stdio.h>
#endif

#define QUERYCHARINFO(pci, pr)  *(pr) = (pci)->metrics

static Mask FontFormat = 
#if IMAGE_BYTE_ORDER == LSBFirst
    BitmapFormatByteOrderLSB |
#else
    BitmapFormatByteOrderMSB |
#endif
#if BITMAP_BIT_ORDER == LSBFirst
    BitmapFormatBitOrderLSB |
#else
    BitmapFormatBitOrderMSB |
#endif
    BitmapFormatImageRectMin |
#if GLYPHPADBYTES == 1
    BitmapFormatScanlinePad8 |
#endif
#if GLYPHPADBYTES == 2
    BitmapFormatScanlinePad16 |
#endif
#if GLYPHPADBYTES == 4
    BitmapFormatScanlinePad32 |
#endif
#if GLYPHPADBYTES == 8
    BitmapFormatScanlinePad64 |
#endif
    BitmapFormatScanlineUnit8;

static int  FinishListFontsWithInfo();

extern pointer fosNaturalParams;
extern FontPtr defaultFont;

extern void (*ReplySwapVector[256]) ();

static FontPathElementPtr *font_path_elements = (FontPathElementPtr *) 0;
static int  num_fpes = 0;
static FPEFunctions *fpe_functions = (FPEFunctions *) 0;
static int  num_fpe_types = 0;
static unsigned char *font_path_string;

static int  num_slept_fpes = 0;
static int  size_slept_fpes = 0;
static FontPathElementPtr *slept_fpes = (FontPathElementPtr *) 0;

int
FontToXError(err)
    int         err;
{
    switch (err) {
    case Successful:
	return Success;
    case AllocError:
	return BadAlloc;
    case BadFontName:
    case BadFontPath:
	return BadName;
    case BadFontFormat:	/* is there something better? */
    case BadCharRange:
	return BadValue;
    default:
	return err;
    }
}


/*
 * adding RT_FONT prevents conflict with default cursor font
 */
Bool
SetDefaultFont(defaultfontname)
    char       *defaultfontname;
{
    int         err;
    FontPtr     pf;
    XID         fid;

    fid = FakeClientID(0);
    err = OpenFont(serverClient, fid, FontLoadAll | FontOpenSync,
		   (unsigned) strlen(defaultfontname), defaultfontname);
    if (err != Success)
	return FALSE;
    pf = (FontPtr) LookupIDByType(fid, RT_FONT);
    if (pf == (FontPtr) NULL)
	return FALSE;
    defaultFont = pf;
    return TRUE;
}

/*
 * note that the font wakeup queue is not refcounted.  this is because
 * an fpe needs to be added when its inited, and removed when its finally
 * freed, in order to handle any data that isn't requested, like FS events.
 *
 * since the only thing that should call these routines is the renderer's
 * init_fpe() and free_fpe(), there shouldn't be any problem in using
 * freed data.
 */
void
QueueFontWakeup(fpe)
    FontPathElementPtr fpe;
{
    int         i;
    FontPathElementPtr *new;

    for (i = 0; i < num_slept_fpes; i++) {
	if (slept_fpes[i] == fpe) {

#ifdef DEBUG
	    fprintf(stderr, "re-queueing fpe wakeup\n");
#endif

	    return;
	}
    }
    if (num_slept_fpes == size_slept_fpes) {
	new = (FontPathElementPtr *)
	    xrealloc(slept_fpes,
		     sizeof(FontPathElementPtr) * (size_slept_fpes + 4));
	if (!new)
	    return;
	slept_fpes = new;
	size_slept_fpes += 4;
    }
    slept_fpes[num_slept_fpes] = fpe;
    num_slept_fpes++;
}

void
RemoveFontWakeup(fpe)
    FontPathElementPtr fpe;
{
    int         i,
                j;

    for (i = 0; i < num_slept_fpes; i++) {
	if (slept_fpes[i] == fpe) {
	    for (j = i; j < num_slept_fpes; j++) {
		slept_fpes[j] = slept_fpes[j + 1];
	    }
	    num_slept_fpes--;
	    return;
	}
    }
}

/* ARGSUSED */
int
FontWakeup(data, count, LastSelectMask)
    pointer     data;
    int         count;
    long       *LastSelectMask;
{
    int         i;
    FontPathElementPtr fpe;

    if (count < 0)
	return Success;		/* ignore -1 return from select XXX */
    /* wake up any fpe's that may be waiting for information */
    for (i = 0; i < num_slept_fpes; i++) {
	fpe = slept_fpes[i];
	(void) (*fpe_functions[fpe->type].wakeup_fpe) (fpe, LastSelectMask);
    }

    return Success;
}

static Bool
doOpenFont(client, c)
    ClientPtr   client;
    OFclosurePtr c;
{
    FontPtr     pfont = NullFont;
    FontPathElementPtr fpe;
    int         err;
    ScreenPtr   pScr;
    int         i;
    char       *alias,
               *newname;
    int         newlen;

    while (c->current_fpe < c->num_fpes) {
	fpe = c->fpe_list[c->current_fpe];
	err = (*fpe_functions[fpe->type].open_font)
	    ((pointer) client, fpe, c->flags,
	     c->fontname, c->fnamelen, FontFormat,
		BitmapFormatMaskByte |
		BitmapFormatMaskBit |
		BitmapFormatMaskImageRectangle |
		BitmapFormatMaskScanLinePad |
		BitmapFormatMaskScanLineUnit,
		c->fontid, &pfont, &alias);

	if (err == FontNameAlias && alias) {
	    newlen = strlen(alias);
	    newname = (char *) xrealloc(c->fontname, newlen);
	    if (!newname) {
		err = AllocError;
		break;
	    }
	    bcopy(alias, newname, newlen);
	    c->fontname = newname;
	    c->fnamelen = newlen;
	    c->current_fpe = 0;
	    continue;
	}
	if (err == BadFontName) {
	    c->current_fpe++;
	    continue;
	}
	if (err == Suspended) {
	    if (!c->slept) {
		c->slept = TRUE;
		ClientSleep(client, doOpenFont, (pointer) c);
	    }
	    return TRUE;
	}
	break;
    }
#ifdef kludge	 /* :-) */
    if (err == BadFontName) {
	err = FontOpenScaled(c->num_fpes, c->fpe_list,
			     c->fontname, c->fnamelen, &pfont,
		       BITMAP_BIT_ORDER, IMAGE_BYTE_ORDER, GLYPHPADBYTES, 1);
    }
#endif
    if (err != Successful)
	goto dropout;
    if (!pfont) {
	err = BadFontName;
	goto dropout;
    }
    if (!AddResource(c->fontid, RT_FONT, (pointer) pfont)) {
	err = AllocError;
	goto dropout;
    }
    if (pfont->refcnt == 0) {
	for (i = 0; i < screenInfo.numScreens; i++) {
	    pScr = screenInfo.screens[i];
	    if (pScr->RealizeFont)
		(*pScr->RealizeFont) (pScr, pfont);
	}
	pfont->fpe = fpe;
	UseFPE(pfont->fpe);
    }
    pfont->refcnt++;
dropout:
    if (err != Successful && c->client != serverClient) {
	SendErrorToClient(c->client, X_OpenFont, 0,
			  c->fontid, FontToXError(err));
    }
    if (c->slept)
	ClientWakeup(c->client);
    for (i = 0; i < c->num_fpes; i++) {
	FreeFPE(c->fpe_list[i]);
    }
    xfree(c->fpe_list);
    xfree(c->fontname);
    xfree(c);
    return TRUE;
}

int
OpenFont(client, fid, flags, lenfname, pfontname)
    ClientPtr   client;
    XID         fid;
    Mask        flags;
    unsigned    lenfname;
    char       *pfontname;
{
    OFclosurePtr c;
    int         i;

    c = (OFclosurePtr) xalloc(sizeof(OFclosureRec));
    if (!c)
	return BadAlloc;
    c->fontname = (char *) xalloc(lenfname);
    if (!c->fontname) {
	xfree(c);
	return BadAlloc;
    }
    /*
     * copy the current FPE list, so that if it gets changed by another client
     * while we're blocking, the request still appears atomic
     */
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	xfree(c->fontname);
	xfree(c);
	return BadAlloc;
    }
    bcopy(pfontname, c->fontname, lenfname);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->fontid = fid;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->fnamelen = lenfname;
    c->slept = FALSE;
    c->flags = flags;

    (void) doOpenFont(client, c);
    return Success;
}

/*
 * Decrement font's ref count, and free storage if ref count equals zero
 */
/*ARGSUSED*/
int
CloseFont(pfont, fid)
    FontPtr     pfont;
    Font        fid;
{
    int         nscr;
    ScreenPtr   pscr;
    FontPathElementPtr fpe;

    if (pfont == NullFont)
	return (Success);
    if (--pfont->refcnt == 0) {
	/*
	 * since the last reference is gone, ask each screen to free any
	 * storage it may have allocated locally for it.
	 */
	for (nscr = 0; nscr < screenInfo.numScreens; nscr++) {
	    pscr = screenInfo.screens[nscr];
	    if (pscr->UnrealizeFont)
		(*pscr->UnrealizeFont) (pscr, pfont);
	}
	if (pfont == defaultFont)
	    defaultFont = NULL;
	fpe = pfont->fpe;
	(*fpe_functions[fpe->type].close_font) (fpe, pfont);
	FreeFPE(fpe);
    }
    return (Success);
}


/***====================================================================***/

 /*
  * \ Sets up pReply as the correct QueryFontReply for pFont with the first
  * nProtoCCIStructs char infos. \
  */

/* 5/23/89 (ef) -- XXX! Does this already exist somewhere? */
static xCharInfo xciNoSuchChar = {0, 0, 0, 0, 0, 0};

void
QueryFont(pFont, pReply, nProtoCCIStructs)
    FontPtr     pFont;
    xQueryFontReply *pReply;	/* caller must allocate this storage */
    int         nProtoCCIStructs;
{
    FontPropPtr pFP;
    int         r,
                c,
                i;
    xFontProp  *prFP;
    xCharInfo  *prCI;
    xCharInfo  *charInfos[256];
    char        chars[512];
    int         nrows,
                ncols;
    int         ninfos;
    int         count;

    /* pr->length set in dispatch */
    pReply->minCharOrByte2 = pFont->info.firstCol;
    pReply->defaultChar = pFont->info.defaultCh;
    pReply->maxCharOrByte2 = pFont->info.lastCol;
    pReply->drawDirection = pFont->info.drawDirection;
    pReply->allCharsExist = pFont->info.allExist;
    pReply->minByte1 = pFont->info.firstRow;
    pReply->maxByte1 = pFont->info.lastRow;
    pReply->fontAscent = pFont->info.fontAscent;
    pReply->fontDescent = pFont->info.fontDescent;

    pReply->minBounds = pFont->info.ink_minbounds;
    pReply->maxBounds = pFont->info.ink_maxbounds;

    pReply->nFontProps = pFont->info.nprops;
    pReply->nCharInfos = nProtoCCIStructs;

    for (i = 0, pFP = pFont->info.props, prFP = (xFontProp *) (&pReply[1]);
	    i < pFont->info.nprops;
	    i++, pFP++, prFP++) {
	prFP->name = pFP->name;
	prFP->value = pFP->value;
    }

    ninfos = 0;
    ncols = pFont->info.lastCol - pFont->info.firstCol + 1;
    prCI = (xCharInfo *) (prFP);
    for (r = pFont->info.firstRow;
	    ninfos < nProtoCCIStructs && r <= pFont->info.lastRow;
	    r++) {
	i = 0;
	for (c = pFont->info.firstCol; c <= pFont->info.lastCol; c++) {
	    chars[i++] = r;
	    chars[i++] = c;
	}
	(*pFont->get_metrics) (pFont, ncols, chars, TwoD16Bit,
			       &count, charInfos);
	i = 0;
	for (i = 0; i < count && ninfos < nProtoCCIStructs; i++) {
	    if (charInfos[i])
		*prCI = *charInfos[i];
	    else
		*prCI = xciNoSuchChar;
	    prCI++;
	    ninfos++;
	}
    }
    return;
}

static int
doListFontsHelper(client, c, func)
    ClientPtr   client;
    LFclosurePtr c;
    Bool        (*func) ();
{
    FontPathElementPtr fpe;
    int         err = Successful;

    /* try each fpe in turn, returning if one wants to be blocked */
    while (c->current_fpe < c->num_fpes && c->names->nnames < c->max_names) {
	fpe = c->fpe_list[c->current_fpe];

	err = (*fpe_functions[fpe->type].list_fonts)
	    (c->client, fpe, c->pattern, c->patlen,
	     c->max_names - c->names->nnames, c->names);

	if (err == Suspended) {
	    if (!c->slept) {
		c->slept = TRUE;
		ClientSleep(client, func, c);
	    }
	    return Suspended;
	}
	if (err != Successful)
	    break;
	c->current_fpe++;
    }
    return err;
}

static Bool
doListFonts(client, c)
    ClientPtr   client;
    LFclosurePtr c;
{
    int         err;
    FontPathElementPtr fpe;
    xListFontsReply reply;
    FontNamesPtr names;
    int         stringLens,
                i,
                nnames;
    char       *bufptr,
               *bufferStart;
    int         count;

    err = doListFontsHelper(client, c, doListFonts);
    if (err == Suspended)
	return TRUE;

    if (err != Successful) {
	SendErrorToClient(client, X_ListFonts, 0, 0, FontToXError(err));
	goto bail;
    }
    names = c->names;
    nnames = names->nnames;
    client = c->client;
    stringLens = 0;
    for (i = 0; i < nnames; i++)
	stringLens += names->length[i];

    reply.type = X_Reply;
    reply.length = (stringLens + nnames + 3) >> 2;
    reply.nFonts = nnames;
    reply.sequenceNumber = client->sequence;

    bufptr = bufferStart = (char *) ALLOCATE_LOCAL(reply.length << 2);

    if (!bufptr && reply.length) {
	SendErrorToClient(client, X_ListFonts, 0, 0, BadAlloc);
	goto bail;
    }
    /*
     * since WriteToClient long word aligns things, copy to temp buffer and
     * write all at once
     */
    for (i = 0; i < nnames; i++) {
	*bufptr++ = names->length[i];
	bcopy(names->names[i], bufptr, names->length[i]);
	bufptr += names->length[i];
    }
    WriteReplyToClient(client, sizeof(xListFontsReply), &reply);
    (void) WriteToClient(client, stringLens + nnames, bufferStart);
    DEALLOCATE_LOCAL(bufferStart);
bail:
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    xfree(c->fpe_list);
    FreeFontNames (c->names);
    xfree(c->pattern);
    xfree(c);
    return TRUE;
}

static      LFclosurePtr
MakeListFontsClosure(client, pattern, length, max_names)
    ClientPtr   client;
    unsigned char *pattern;
    unsigned int length;
    unsigned int max_names;
{
    LFclosurePtr c;
    int         i;
    FontNamesPtr all_names;

    c = (LFclosurePtr) xalloc(sizeof(LFclosureRec));
    if (!c)
	return 0;
    c->pattern = (char *) xalloc(length);
    if (!c->pattern) {
	xfree(c);
	return 0;
    }
    c->names = MakeFontNamesRecord(max_names < 100 ? max_names : 100);
    if (!c->names) {
	xfree(c->pattern);
	xfree(c);
	return 0;
    }
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	FreeFontNames (c->names);
	xfree (c->pattern);
	xfree (c);
	return 0;
    }
    bcopy(pattern, c->pattern, length);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->patlen = length;
    c->client = client;
    c->max_names = max_names;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->slept = FALSE;
    return c;
}

int
ListFonts(client, pattern, length, max_names)
    ClientPtr   client;
    unsigned char *pattern;
    unsigned int length;
    unsigned int max_names;
{
    LFclosurePtr c;

    c = MakeListFontsClosure(client, pattern, length, max_names);
    if (!c)
	return BadAlloc;

    (void) doListFonts(client, c);
    return Success;
}

doListFontsWithInfo(client, c)
    ClientPtr   client;
    LFWIclosurePtr c;
{
    FontPathElementPtr	fpe;
    int			err = Successful;
    char		*name;
    int			namelen;
    int			numFonts;
    FontInfoRec		fontInfo,
			*pFontInfo;
    xListFontsWithInfoReply *reply;
    int			length;
    xFontProp		*pFP;
    int			i;

    while (c->current.current_fpe < c->num_fpes) 
    {
	fpe = c->fpe_list[c->current.current_fpe];
	err = Successful;
	if (!c->current.list_started) {
	    err = (*fpe_functions[fpe->type].start_list_fonts_with_info)
		(client, fpe, c->current.pattern, c->current.patlen,
 		 c->current.max_names, &c->current.private);
	    if (err == Suspended) {
		if (!c->slept) {
		    ClientSleep(client, doListFontsWithInfo, c);
		    c->slept = TRUE;
		}
		return TRUE;
	    }
	    if (err == Successful)
		c->current.list_started = TRUE;
	}
	if (err == Successful) {
	    name = 0;
	    pFontInfo = &fontInfo;
	    err = (*fpe_functions[fpe->type].list_next_font_with_info)
		(client, fpe, &name, &namelen, &pFontInfo, 
		    &numFonts, c->current.private);
	    if (err == Suspended) {
		if (!c->slept) {
		    ClientSleep(client, doListFontsWithInfo, c);
		    c->slept = TRUE;
		}
		return TRUE;
	    }
	}
	/*
	 * When we get an alias back, save our state and
	 * reset back to the start of the FPE looking for
	 * the specified name.  As soon as a real font is found
	 * for the alias, pop back to the old state
	 */
	if (err == FontNameAlias)
	{
	    if (!c->haveSaved)
		c->saved = c->current;
	    c->current.pattern = name;
	    c->current.patlen = namelen;
	    c->current.max_names = 1;
	    c->current.current_fpe = 0;
	    c->current.private = 0;
	    c->current.list_started = FALSE;
	    c->haveSaved = TRUE;
	    c->savedNumFonts = numFonts;
	    c->savedName = (char *) pFontInfo;
	}
	/*
	 * At the end of this FPE, step to the next.  If
	 * we've finished processing an alias, pop state back.
	 * If we've sent enough font names, quit.
	 */
	else if (err == BadFontName)
	{
	    c->current.list_started = FALSE;
	    c->current.current_fpe++;
	    err = Successful;
	    if (c->haveSaved)
	    {
		if (c->current.max_names == 0 ||
		    c->current.current_fpe == c->num_fpes)
		{
		    c->haveSaved = FALSE;
		    c->saved.max_names -= (1 - c->current.max_names);
		    c->current = c->saved;
	    	}
	    }
	    if (c->current.max_names == 0)
		break;
	} else if (err == Successful) {
	    length = sizeof(*reply) + pFontInfo->nprops * sizeof(xFontProp);
	    reply = c->reply;
	    if (c->length < length) {
		reply = (xListFontsWithInfoReply *) xrealloc(c->reply, length);
		if (!reply) {
		    err = AllocError;
		    break;
		}
		c->reply = reply;
		c->length = length;
	    }
	    if (c->haveSaved)
	    {
		numFonts = c->savedNumFonts;
		name = c->savedName;
		namelen = strlen (name);
	    }
	    reply->type = X_Reply;
	    reply->length = (sizeof *reply - sizeof(xGenericReply) +
			     pFontInfo->nprops * sizeof(xFontProp) +
			     namelen + 3) >> 2;
	    reply->sequenceNumber = client->sequence;
	    reply->nameLength = namelen;
	    reply->minBounds = pFontInfo->ink_minbounds;
	    reply->maxBounds = pFontInfo->ink_maxbounds;
	    reply->minCharOrByte2 = pFontInfo->firstCol;
	    reply->maxCharOrByte2 = pFontInfo->lastCol;
	    reply->defaultChar = pFontInfo->defaultCh;
	    reply->nFontProps = pFontInfo->nprops;
	    reply->drawDirection = pFontInfo->drawDirection;
	    reply->minByte1 = pFontInfo->firstRow;
	    reply->maxByte1 = pFontInfo->lastRow;
	    reply->allCharsExist = pFontInfo->allExist;
	    reply->fontAscent = pFontInfo->fontAscent;
	    reply->fontDescent = pFontInfo->fontDescent;
	    reply->nReplies = numFonts;
	    pFP = (xFontProp *) (reply + 1);
	    for (i = 0; i < pFontInfo->nprops; i++) {
		pFP->name = pFontInfo->props[i].name;
		pFP->value = pFontInfo->props[i].value;
		pFP++;
	    }
	    WriteReplyToClient(client, length, reply);
	    (void) WriteToClient(client, namelen, name);
	    if (pFontInfo == &fontInfo) {
		xfree(fontInfo.props);
		xfree(fontInfo.isStringProp);
	    }
	    --c->current.max_names;
	    if (c->current.max_names < 0)
		abort ();
	}
    }
    if (err == Successful) {
	reply = c->reply;
	length = sizeof(xListFontsWithInfoReply);
	if (c->length < length);
	{
	    reply = (xListFontsWithInfoReply *) xrealloc(c->reply, length);
	    if (reply) {
		c->reply = reply;
		c->length = length;
	    } else
		err = AllocError;
	}
	if (err == Successful) {
	    bzero((char *) reply, sizeof(xListFontsWithInfoReply));
	    reply->type = X_Reply;
	    reply->sequenceNumber = client->sequence;
	    reply->length = (sizeof(xListFontsWithInfoReply)
			     - sizeof(xGenericReply)) >> 2;
	    WriteReplyToClient(client, length, reply);
	}
    }
    if (err != Successful)
	SendErrorToClient(client, X_ListFontsWithInfo, 0, 0, FontToXError(err));
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    xfree (c->fpe_list);
    xfree (c->current.pattern);
    xfree (c);
    return TRUE;
}

int
StartListFontsWithInfo(client, length, pattern, max_names)
    ClientPtr   client;
    int         length;
    char       *pattern;
    int         max_names;
{
    int		    err;
    int		    i;
    LFWIclosurePtr  c;

    if (!(c = (LFWIclosurePtr) xalloc(sizeof *c)))
	goto badAlloc;
    if (!(c->current.pattern = (char *) xalloc (length)))
    {
	xfree (c);
	goto badAlloc;
    }
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list)
    {
	xfree (c->current.pattern);
	xfree (c);
	goto badAlloc;
    }
    bcopy (pattern, c->current.pattern, length);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->num_fpes = num_fpes;
    c->reply = 0;
    c->length = 0;
    c->current.patlen = length;
    c->current.current_fpe = 0;
    c->current.max_names = max_names;
    c->current.list_started = FALSE;
    c->current.private = 0;
    c->savedNumFonts = 0;
    c->haveSaved = FALSE;
    c->slept = FALSE;
    doListFontsWithInfo(client, c);
    return TRUE;
badAlloc:
    SendErrorToClient(client, X_ListFontsWithInfo, 0, 0, BadAlloc);
    return TRUE;
}

/* does the necessary magic to figure out the fpe type */
static int
DetermineFPEType(pathname)
    char       *pathname;
{
    int         i;

    for (i = 0; i < num_fpe_types; i++) {
	if ((*fpe_functions[i].name_check) (pathname))
	    return i;
    }
    return -1;
}


static void
FreeFontPath(list, n)
    FontPathElementPtr *list;
    int         n;
{
    int         i;

    for (i = 0; i < n; i++) {
	FreeFPE(list[i]);
    }
    xfree((char *) list);
}

/* XXX fix this to use reset_fpe() */

static int
SetFontPathElements(npaths, paths, bad)
    int         npaths;
    unsigned char *paths;
    int        *bad;
{
    int         i,
                err;
    unsigned int len;
    unsigned char *cp = paths;
    FontPathElementPtr fpe,
               *fplist;

    fplist = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * npaths);
    if (!fplist) {
	*bad = 0;
	return BadAlloc;
    }
    for (i = 0; i < npaths; i++) {
	len = (unsigned int) (*cp++);
	if (len) {
	    fpe = (FontPathElementPtr) xalloc(sizeof(FontPathElementRec));
	    if (!fpe) {
		err = BadAlloc;
		goto bail;
	    }
	    fpe->name = (char *) xalloc(len + 1);
	    if (!fpe->name) {
		xfree(fpe);
		err = BadAlloc;
		goto bail;
	    }
	    fpe->refcount = 1;
	    fplist[i] = fpe;
	    strncpy(fpe->name, (char *) cp, (int) len);
	    cp += len;
	    fpe->name[len] = '\0';
	    fpe->name_length = len;
	    fpe->type = DetermineFPEType(fpe->name);
	    if (fpe->type == -1) {
		xfree(fpe->name);
		xfree(fpe);
		err = BadValue;
		goto bail;
	    }
	    err = (*fpe_functions[fpe->type].init_fpe) (fpe, FontFormat);
	    if (err != Successful) {
		xfree(fpe->name);
		xfree(fpe);
		err = BadValue;
		goto bail;
	    }
	}
    }
    FreeFontPath(font_path_elements, num_fpes);
    font_path_elements = fplist;
    num_fpes = npaths;
    return Success;
bail:
    *bad = i;
    while (--i >= 0)
	FreeFPE(fplist[i]);
    xfree(fplist);
    return err;
}

/* XXX -- do we need to pass error down to each renderer? */
int
SetFontPath(client, npaths, paths, error)
    ClientPtr   client;
    int         npaths;
    unsigned char *paths;
    int        *error;
{
    unsigned char *new_font_path_string;
    int         len,
                err = Success;

    if (npaths == 0) {
	return SetDefaultFontPath(defaultFontPath, error);
    } else {
	len = strlen(paths) + 1;
	new_font_path_string = (unsigned char *) xalloc(len);
	if (!new_font_path_string)
	    return BadAlloc;

	err = SetFontPathElements(npaths, paths, error);

	if (err == Success) {
	    bcopy(paths, new_font_path_string, len);
	    xfree(font_path_string);
	    font_path_string = new_font_path_string;
	} else {
	    xfree(new_font_path_string);
	}
    }
    return err;
}

SetDefaultFontPath(path, error)
    char       *path;
    int	       *error;
{
    unsigned char *cp,
               *pp,
               *nump,
               *newpath,
               *new_font_path_string;
    int         num = 1,
                len,
                err,
                size = 0;

    /* get enough for string, plus values -- use up commas */
    len = strlen(path) + 1;
    nump = cp = newpath = (unsigned char *) ALLOCATE_LOCAL(len);
    if (!newpath)
	return BadAlloc;
    pp = (unsigned char *) path;
    cp++;
    while (*pp) {
	*cp++ = *pp++;
	size++;
	if (*pp == ',') {
	    *nump = (unsigned char) size;
	    nump = cp++;
	    pp++;
	    num++;
	    size = 0;
	}
    }
    *nump = (unsigned char) size;

    new_font_path_string = (unsigned char *) xalloc(len);
    if (!new_font_path_string)
	return BadAlloc;

    err = SetFontPathElements(num, newpath, error);

    /* stash it for GetFontPath */
    if (err == Success) {
	bcopy(newpath, new_font_path_string, len);
	xfree(font_path_string);
	font_path_string = new_font_path_string;
    } else
	xfree(new_font_path_string);

    DEALLOCATE_LOCAL(newpath);

    return err;
}

unsigned char *
GetFontPath(count, length)
    int        *count,
               *length;
{
    int         i,
                size;
    unsigned char *bp;

    *count = num_fpes;
    bp = font_path_string;
    *length = 0;
    for (i = 0; i < num_fpes; i++) {
	size = (int) *bp++;
	bp += size;
	*length += size;
    }
    return font_path_string;
}

LoadGlyphs(client, pfont, nchars, item_size, data)
    ClientPtr   client;
    FontPtr     pfont;
    unsigned    nchars;
    int         item_size;
    unsigned char *data;
{

#ifdef NOTDEF
/* under construction */
    /* either returns Success, ClientBlocked, or some nasty error */
    return (*fpe_functions[pfont->type].load_glyphs)
	(client, pfont, nchars, item_size, data);
#endif
}

/* XXX -- these two funcs may want to be broken into macros */
void
UseFPE(fpe)
    FontPathElementPtr fpe;
{
    fpe->refcount++;
}

void
FreeFPE(fpe)
    FontPathElementPtr fpe;
{
    fpe->refcount--;
    if (fpe->refcount == 0) {
	(*fpe_functions[fpe->type].free_fpe) (fpe);
	xfree(fpe->name);
	xfree(fpe);
    }
}

InitFonts ()
{
    FontFileRegisterFpeFunctions ();
/*    SpeedoRegisterFpeFunctions (); */
    fs_register_fpe_functions ();
}

GetDefaultPointSize ()
{
    return 120;
}

struct resolution {
    CARD16	x_resolution B16;
    CARD16	y_resolution B16;
    CARD16	point_size B16;
};

struct resolution *
GetClientResolutions (num)
    int	*num;
{
    static struct resolution	res;
    int	    mm, pix;
    ScreenPtr	pScreen;

    pScreen = screenInfo.screens[0];
    res.x_resolution = (pScreen->width * 25.4) / pScreen->mmWidth;
    /*
     * XXX - we'll want this as long as bitmap instances are
     * prevalent so that we can match them from scalable fonts
     */
    if (res.x_resolution < 88)
	res.x_resolution = 75;
    else
	res.x_resolution = 100;
    res.y_resolution = (pScreen->height * 25.4) / pScreen->mmHeight;
    if (res.y_resolution < 88)
	res.y_resolution = 75;
    else
	res.y_resolution = 100;
    res.point_size = 120;
    *num = 1;
    return &res;
}

Font
GetNewFontClientID()
{
    return FakeClientID(0);
}

int
StoreFontClientFont(pfont, id)
    FontPtr     pfont;
    Font        id;
{
    return AddResource(id, RT_FONT, (pointer)pfont);
}
	
/*
 * returns the type index of the new fpe
 *
 * should be called (only once!) by each type of fpe when initialized
 */

int
RegisterFPEFunctions(name_func, init_func, free_func, reset_func,
	   open_func, close_func, list_func, start_lfwi_func, next_lfwi_func,
		     wakeup_func, render_names)
    Bool        (*name_func) ();
    int         (*init_func) ();
    int         (*free_func) ();
    int         (*reset_func) ();
    int         (*open_func) ();
    int         (*close_func) ();
    int         (*list_func) ();
    int         (*start_lfwi_func) ();
    int         (*next_lfwi_func) ();
    int         (*wakeup_func) ();
    FontNamesPtr render_names;
{
    FPEFunctions *new;

    /* grow the list */
    new = (FPEFunctions *) xrealloc(fpe_functions,
				 (num_fpe_types + 1) * sizeof(FPEFunctions));
    if (!new)
	return -1;
    fpe_functions = new;

    fpe_functions[num_fpe_types].name_check = name_func;
    fpe_functions[num_fpe_types].open_font = open_func;
    fpe_functions[num_fpe_types].close_font = close_func;
    fpe_functions[num_fpe_types].wakeup_fpe = wakeup_func;
    fpe_functions[num_fpe_types].list_fonts = list_func;
    fpe_functions[num_fpe_types].start_list_fonts_with_info =
	start_lfwi_func;
    fpe_functions[num_fpe_types].list_next_font_with_info =
	next_lfwi_func;
    fpe_functions[num_fpe_types].init_fpe = init_func;
    fpe_functions[num_fpe_types].free_fpe = free_func;
    fpe_functions[num_fpe_types].reset_fpe = reset_func;

    fpe_functions[num_fpe_types].renderer_names = render_names;
    return num_fpe_types++;
}

FreeFonts()
{
    xfree(fpe_functions);
    num_fpe_types = 0;
    fpe_functions = (FPEFunctions *) 0;
}

/* convenience functions for FS interface */

/* ARGSUSED */
FontPtr
find_old_font(client, id)
    ClientPtr	client;
    XID         id;
{
    return (FontPtr) LookupIDByType(id, RT_FONT);
}

static int  fs_handlers_installed = 0;
static unsigned int last_server_gen;

init_fs_handlers(fpe, block_handler)
    FontPathElementPtr fpe;
    int         (*block_handler) ();
{
    /* if server has reset, make sure the b&w handlers are reinstalled */
    if (last_server_gen < serverGeneration) {
	last_server_gen = serverGeneration;
	fs_handlers_installed = 0;
    }
    if (fs_handlers_installed == 0) {

#ifdef DEBUG
	fprintf(stderr, "adding FS b & w handlers\n");
#endif

	if (!RegisterBlockAndWakeupHandlers(block_handler,
					    FontWakeup, (pointer) 0))
	    return AllocError;
	fs_handlers_installed++;
    }
    QueueFontWakeup(fpe);
    return Successful;
}

remove_fs_handlers(fpe, block_handler, all)
    FontPathElementPtr fpe;
    int         (*block_handler) ();
    Bool        all;
{
    if (all) {
	/* remove the handlers if no one else is using them */
	if (--fs_handlers_installed == 0) {

#ifdef DEBUG
	    fprintf(stderr, "removing FS b & w handlers\n");
#endif

	    RemoveBlockAndWakeupHandlers(block_handler, FontWakeup,
					 (pointer) 0);
	}
    }
    RemoveFontWakeup(fpe);
}
