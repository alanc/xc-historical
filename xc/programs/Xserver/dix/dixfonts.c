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

/* $XConsortium: dixfonts.c,v 1.11 91/02/14 19:35:36 keith Exp $ */

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

#define QUERYCHARINFO(pci, pr)  *(pr) = (pci)->metrics

static int FinishListFontsWithInfo();
static int finishOpenFont();

extern pointer fosNaturalParams;
extern EncodedFontPtr defaultFont;

extern void (*ReplySwapVector[256]) ();

static FontPathElementPtr *font_path_elements = (FontPathElementPtr *) 0;
static int  num_fpes = 0;
static FPEFunctions *fpe_functions = (FPEFunctions *) 0;
static int  num_fpe_types = 0;
static unsigned char *font_path_string;

static int  num_slept_fpes = 0;
static int  size_slept_fpes = 0;
static FontPathElementPtr *slept_fpes = (FontPathElementPtr *) 0;

/*
 * adding RT_FONT prevents conflict with default cursor font
 */
Bool
SetDefaultFont(defaultfontname)
    char       *defaultfontname;
{
    int         err;
    EncodedFontPtr pf;
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

    /* wake up any fpe's that may be waiting for information */
    for (i = 0; i < num_slept_fpes; i++) {
	fpe = slept_fpes[i];
	(void) (*fpe_functions[fpe->type].wakeup_fpe) (fpe, LastSelectMask);
    }

    return Success;
}

static Bool
doOpenFont(client, closure)
    ClientPtr   client;
    OFclosurePtr closure;
{
    EncodedFontPtr pfont = NullEncodedFont;
    FontPathElementPtr fpe;
    int         err;

start_again:
    while (closure->current_fpe < closure->num_fpes) {
	fpe = closure->fpe_list[closure->current_fpe];
	err = (*fpe_functions[fpe->type].open_font)
	    (closure, fpe, closure->flags,
	     closure->fontname, closure->fnamelen, &pfont);

#ifdef notyet
/* XXX -- needs some work -- does the font name really belong there? */
	if (pfont->type == FontAliasType) {
	    closure->lenfname = pfont->namelen;
	    closure->fname = pfont->name;
	    closure->current_fpe = 0;
	    goto start_again;
	}
#endif

	if (err == BadName) {
	    closure->current_fpe++;
	    continue;
	}
	if (err == Suspended) {
	    if (!closure->slept) {
		closure->slept = TRUE;
		ClientSleep(client, doOpenFont, closure);
	    }
	    return TRUE;
	}
	break;
    }
    err = finishOpenFont(closure, pfont);
    return TRUE;
}

static int
finishOpenFont(closure, pfont)
    OFclosurePtr closure;
    FontPtr     pfont;
{
    int         i,
                err = Success;
    ScreenPtr   pscr;

    if (!pfont) {
	err = BadName;
	goto dropout;
    }
    if (!AddResource(closure->fontid, RT_FONT, (pointer) pfont)) {
	err = BadAlloc;
	goto dropout;
    }
    if (pfont->refcnt == 0) {
	for (i = 0; i < screenInfo.numScreens; i++) {
	    pscr = screenInfo.screens[i];
	    if (pscr->RealizeFont)
		(*pscr->RealizeFont) (pscr, pfont);
	}
    }
    pfont->refcnt++;
dropout:
    if (err != Success) {
	SendErrorToClient(closure->client, X_OpenFont, 0,
			  closure->fontid, err);
    }
    if (closure->slept)
	ClientWakeup(closure->client);
    for (i = 0; i < closure->num_fpes; i++) {
	FreeFPE(closure->fpe_list[i]);
    }
    xfree(closure->fpe_list);
    xfree(closure->fontname);
    xfree(closure);
    return err;
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
    EncodedFontPtr pfont;
    Font        fid;
{
    int         nscr;
    ScreenPtr   pscr;

    if (pfont == NullEncodedFont)
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

#ifdef totallybogusifdef
	if (pfont->devPriv) {
	    Xfree(pfont->devPriv);
	    pfont->devPriv = NULL;
	}
#endif

	(void) (*fpe_functions[pfont->type].close_font) (pfont, TRUE);
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
    EncodedFontPtr pFont;
    xQueryFontReply *pReply;	/* caller must allocate this storage */
    int         nProtoCCIStructs;
{
    CharSetPtr  pCS = pFont->pCS;
    CharInfoPtr *ppCI;
    FontPropPtr pFP;
    int         i;
    xFontProp  *prFP;
    xCharInfo  *prCI;

    /* pr->length set in dispatch */
    pReply->minCharOrByte2 = pFont->firstCol;
    pReply->defaultChar = pFont->defaultCh;
    pReply->maxCharOrByte2 = pFont->lastCol;
    pReply->drawDirection = pCS->drawDirection;
    pReply->allCharsExist = pFont->allExist;
    pReply->minByte1 = pFont->firstRow;
    pReply->maxByte1 = pFont->lastRow;
    pReply->fontAscent = pCS->fontAscent;
    pReply->fontDescent = pCS->fontDescent;

    pReply->minBounds = pFont->inkMin;
    pReply->maxBounds = pFont->inkMax;

    pReply->nFontProps = pCS->nProps;
    pReply->nCharInfos = nProtoCCIStructs;


    for (i = 0, pFP = pCS->props, prFP = (xFontProp *) (&pReply[1]);
	    i < pCS->nProps;
	    i++, pFP++, prFP++) {
	prFP->name = pFP->name;
	prFP->value = pFP->value;
    }

    for (i = 0, ppCI = pFont->ppInkCI, prCI = (xCharInfo *) (prFP);
	    i < nProtoCCIStructs;
	    i++, ppCI++, prCI++) {
	if (*ppCI)
	    *prCI = (*ppCI)->metrics;
	else
	    *prCI = xciNoSuchChar;
    }
    return;
}

static Bool
NextListFontsWithInfo(client, c)
    ClientPtr   client;
    LFWIclosurePtr c;
{
    int         err;
    xListFontsWithInfoReply *reply;
    FontPathElementPtr fpe;

    while (1) {
	/* quit at last fpe or when all names found */
	if (c->current_fpe == c->num_fpes || c->maxNames <= 0) {
	    (void) FinishListFontsWithInfo(client, c);
	    return TRUE;
	}
	fpe = c->fpe_list[c->current_fpe];

	/* start new fpe */
	if (c->numfonts == -1) {
	    err = (*fpe_functions[fpe->type].start_list_fonts_with_info)
		(c, fpe);
	    if (err == Suspended) {
		/* don't want it on the sleep Q more than once */
		if (!c->slept) {
		    ClientSleep(client, NextListFontsWithInfo, c);
		    c->slept = TRUE;
		}
		return TRUE;
	    }
	    if (c->numfonts == -1) {	/* nothing interesting in this fpe */
		c->current_fpe++;
		continue;
	    }
	}
	/* get the next reply */
	err = (*fpe_functions[fpe->type].list_next_font_with_info) (c, fpe);

	if (err == Suspended) {
	    if (!c->slept) {
		ClientSleep(client, NextListFontsWithInfo, c);
		c->slept = TRUE;
	    }
	    return TRUE;
	}
	if (err == Success) {
	    c->current_fpe++;
	    c->numfonts = -1;
	    /* last one for fpe, so clean it up */
	    (void) (*fpe_functions[fpe->type].finish_list_fonts_with_info) (c);
	    continue;
	}
	/* decrement the number of names we ask the next guy for */
	c->maxNames--;
	reply = c->reply;
	reply->type = X_Reply;
	reply->length = (c->length - sizeof(xGenericReply)
			 + c->namelen + 3) >> 2;
	reply->sequenceNumber = c->client->sequence;
	reply->nameLength = c->namelen;
	reply->nReplies = c->numfonts;
	WriteReplyToClient(client, c->length, reply);
	(void) WriteToClient(client, c->namelen, c->fontname);
    }
}

static int
FinishListFontsWithInfo(client, c)
    ClientPtr   client;
    LFWIclosurePtr c;
{
    xListFontsWithInfoReply last_reply;
    int         i;

    bzero((char *) &last_reply, sizeof(xListFontsWithInfoReply));
    last_reply.type = X_Reply;
    last_reply.sequenceNumber = client->sequence;
    last_reply.length = (sizeof(xListFontsWithInfoReply)
			 - sizeof(xGenericReply)) >> 2;
    WriteReplyToClient(client, sizeof(xListFontsWithInfoReply), &last_reply);

    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++) {
	FreeFPE(c->fpe_list[i]);
    }
    xfree(c->fpe_list);
    xfree(c->pattern);
    xfree(c->reply);
    xfree(c);

    return Success;
}

/*
 * note that the 'numfonts' member of the LFWIclosure is being overloaded.
 * it contains both the number of replies remaining (the hint for the
 * X protocol) and special cases -1 as menaing that an fpe is finished
 */

int
StartListFontsWithInfo(client, length, pattern, maxNames)
    ClientPtr   client;
    int         length;
    char       *pattern;
    int         maxNames;
{
    LFWIclosurePtr c;
    int         i;

    c = (LFWIclosurePtr) xalloc(sizeof(*c));
    if (!c)
	return BadAlloc;
    c->pattern = (char *) xalloc(length);
    if (!c->pattern) {
	xfree(c);
	return BadAlloc;
    }
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	xfree(c->pattern);
	xfree(c);
	return BadAlloc;
    }
    bcopy(pattern, (char *) c->pattern, length);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->slept = FALSE;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->reply = 0;
    c->length = 0;
    c->numfonts = -1;
    c->patlen = length;
    c->maxNames = maxNames;

    (void) NextListFontsWithInfo(client, c);
    return Success;
}


static int
finishListFonts(closure)
    LFclosurePtr closure;
{
    xListFontsReply reply;
    FontPathPtr fpr,
               *fpr_list;
    int         stringLens,
                i,
                j,
                total_fonts;
    char       *bufptr,
               *bufferStart;
    int         count;
    ClientPtr   client;

    count = closure->npaths;
    fpr_list = closure->all_paths;
    client = closure->client;
    stringLens = 0;
    total_fonts = 0;
    for (j = 0; j < count; j++) {
	fpr = fpr_list[j];
	for (i = 0; i < fpr->npaths; i++) {
	    stringLens += fpr->length[i];
	    total_fonts++;
	}
    }

    reply.type = X_Reply;
    reply.length = (stringLens + total_fonts + 3) >> 2;
    reply.nFonts = total_fonts;
    reply.sequenceNumber = client->sequence;

    bufptr = bufferStart = (char *) ALLOCATE_LOCAL(reply.length << 2);
    if (!bufptr && reply.length) {
	for (j = 0; j < count; j++)
	    FreeFontRecord(fpr_list[j]);

	xfree(fpr_list);
	for (i = 0; i < closure->num_fpes; i++) {
	    FreeFPE(closure->fpe_list[i]);
	}
	xfree(closure->fpe_list);
	xfree(closure->pattern);
	xfree(closure->paths);
	if (closure->slept)
	    ClientWakeup(client);
	xfree(closure);
	SendErrorToClient(client, X_ListFonts, 0, 0, BadAlloc);
	return (BadAlloc);
    }
    /*
     * since WriteToClient long word aligns things, copy to temp buffer and
     * write all at once
     */
    for (j = 0; j < count; j++) {
	fpr = fpr_list[j];
	for (i = 0; i < fpr->npaths; i++) {
	    *bufptr++ = fpr->length[i];
	    bcopy(fpr->paths[i], bufptr, fpr->length[i]);
	    bufptr += fpr->length[i];
	}
    }
    WriteReplyToClient(client, sizeof(xListFontsReply), &reply);
    (void) WriteToClient(client, stringLens + total_fonts, bufferStart);
    for (j = 0; j < count; j++)
	FreeFontRecord(fpr_list[j]);
    xfree(fpr_list);
    DEALLOCATE_LOCAL(bufferStart);

    if (closure->slept)
	ClientWakeup(client);
    for (i = 0; i < closure->num_fpes; i++) {
	FreeFPE(closure->fpe_list[i]);
    }
    xfree(closure->fpe_list);
    xfree(closure->pattern);
    xfree(closure->paths);
    xfree(closure);

    return (client->noClientException);
}

static Bool
doListFonts(client, c)
    LFclosurePtr c;
{
    int         err;
    int         path_count = 0;
    LFclosureRec newc;
    FontPathElementPtr fpe;

    /* try each fpe in turn, returning if one wnats to be blocked */
    while (c->current_fpe < c->num_fpes) {
	fpe = c->fpe_list[c->current_fpe];

	err = (*fpe_functions[fpe->type].list_fonts)
	    (c, fpe, c->pattern, c->patlen, c->maxnames, &newc);

	if (err == Suspended) {
	    if (!c->slept) {
		c->slept = TRUE;
		ClientSleep(client, doListFonts, c);
	    }
	    return TRUE;
	}
	if (err == Success) {
	    c->all_paths[c->npaths++] = newc.paths;

	    c->maxnames -= newc.paths->npaths;
	    /* quit if we've got as many as we want */
	    if (c->maxnames < 0) {
		break;
	    }
	}
	c->current_fpe++;
    }

    (void) finishListFonts(c);

    return TRUE;
}

int
ListFonts(client, pattern, patlen, maxnames)
    ClientPtr   client;
    unsigned char *pattern;
    unsigned int patlen;
    unsigned int maxnames;
{
    LFclosurePtr c;
    int         i;
    FontPathPtr *all_paths;

    c = (LFclosurePtr) xalloc(sizeof(LFclosureRec));
    if (!c) {
	return BadAlloc;
    }
    c->all_paths = (FontPathPtr *) xalloc(sizeof(FontPathPtr) * num_fpes);
    if (!c->all_paths) {
	xfree(c);
	return BadAlloc;
    }
    c->pattern = (char *) xalloc(patlen);
    if (!c->pattern) {
	xfree(c->all_paths);
	xfree(c);
	return BadAlloc;
    }
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    bcopy(pattern, c->pattern, patlen);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }

    c->patlen = patlen;
    c->client = client;
    c->npaths = 0;
    c->maxnames = maxnames;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->slept = FALSE;

    (void) doListFonts(client, c);

    return Success;
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

static int
SetFontPathElements(npaths, paths)
    int         npaths;
    unsigned char *paths;
{
    int         i,
                err;
    unsigned int len;
    unsigned char *cp = paths;
    FontPathElementPtr fpe,
               *fplist;

    fplist = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * npaths);
    if (!fplist)
	return BadAlloc;
    for (i = 0; i < npaths; i++) {
	len = (unsigned int) (*cp++);
	if (len) {
	    fpe = (FontPathElementPtr) xalloc(sizeof(FontPathElementRec));
	    if (!fpe) {
	memfail:
		while (--i >= 0)
		    FreeFPE(fplist[i]);
		xfree(fplist);
		return BadAlloc;
	    }
	    fpe->name = (char *) xalloc(len + 1);
	    if (!fpe->name) {
		xfree((char *) fplist);
		goto memfail;
	    }
	    fpe->refcount = 1;
	    fplist[i] = fpe;
	    strncpy(fpe->name, (char *) cp, (int) len);
	    cp += len;
	    fpe->name[len] = '\0';
	    fpe->name_length = len;
	    fpe->type = DetermineFPEType(fpe->name);
	    if (fpe->type == -1) {	/* unrecognized fpe type */
		/* XXX error handling */
		continue;
	    }
	    err = (*fpe_functions[fpe->type].init_fpe) (fpe);
/* XXX need some more work here for errors -- do we quit or keep going? */
	    if (err != Success)
		return err;
	}
    }
    FreeFontPath(font_path_elements, num_fpes);
    font_path_elements = fplist;
    num_fpes = npaths;
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
    int         len,
                err = Success;

    if (npaths == 0) {
	SetDefaultFontPath(defaultFontPath);
    } else {
	err = SetFontPathElements(npaths, paths);

	if (err == Success) {
	    /* stash it for GetFontPath */
	    len = strlen(paths) + 1;
	    font_path_string = (unsigned char *) xalloc(len);
	    bcopy(paths, font_path_string, len);
	}
    }
    return err;
}

SetDefaultFontPath(path)
    char       *path;
{
    unsigned char *cp,
               *pp,
               *nump,
               *newpath;
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

    err = SetFontPathElements(num, newpath);

    /* stash it for GetFontPath */
    if (err == Success) {
	font_path_string = (unsigned char *) xalloc(len);
	bcopy(newpath, font_path_string, len);
    }
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
    /* either returns Success, ClientBlocked, or some nasty error */
    return (*fpe_functions[pfont->type].load_glyphs)
	(client, pfont, nchars, item_size, data);
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

/*
 * returns the type index of the new fpe
 *
 * should be called (only once!) by each type of fpe when initialized
 */
int
register_fpe_functions(name_check, open_func, wakeup_func, list_func,
		       start_list_info_func, list_next_info_func,
		       finish_list_info_func, close_func,
		       init_func, free_fpe_func, load_glyphs_func)
    Bool        (*name_check) ();
    int         (*open_func) ();
    int         (*wakeup_func) ();
    int         (*list_func) ();
    int         (*start_list_info_func) ();
    int         (*list_next_info_func) ();
    int         (*finish_list_info_func) ();
    int         (*close_func) ();
    int         (*init_func) ();
    int         (*free_fpe_func) ();
    int         (*load_glyphs_func) ();
{
    FPEFunctions *new;

    /* grow the list */
    new = (FPEFunctions *) xrealloc(fpe_functions,
				 (num_fpe_types + 1) * sizeof(FPEFunctions));
    if (!new)
	return -1;
    fpe_functions = new;

    fpe_functions[num_fpe_types].name_check = name_check;
    fpe_functions[num_fpe_types].open_font = open_func;
    fpe_functions[num_fpe_types].wakeup_fpe = wakeup_func;
    fpe_functions[num_fpe_types].list_fonts = list_func;
    fpe_functions[num_fpe_types].start_list_fonts_with_info =
	start_list_info_func;
    fpe_functions[num_fpe_types].list_next_font_with_info =
	list_next_info_func;
    fpe_functions[num_fpe_types].finish_list_fonts_with_info =
	finish_list_info_func;
    fpe_functions[num_fpe_types].close_font = close_func;
    fpe_functions[num_fpe_types].init_fpe = init_func;
    fpe_functions[num_fpe_types].free_fpe = free_fpe_func;
    fpe_functions[num_fpe_types].load_glyphs = load_glyphs_func;

    return num_fpe_types++;
}

InitFonts()
{
    fs_register_fpe_functions();
    pcf_register_fpe_functions();
}

FreeFonts()
{
    xfree(fpe_functions);
    num_fpe_types = 0;
    fpe_functions = (FPEFunctions *) 0;
}
