/*
 * font control
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * %W%	%G%
 *
 */

#include        "FS.h"
#include        "FSproto.h"
#include	<stdio.h>
#include	<X11/Xos.h>
#include	"clientstr.h"
#include	"resource.h"
#include	"difsfontst.h"
#include	"fontstruct.h"
#include	"closestr.h"
#include	"globals.h"

extern void (*ReplySwapVector[NUM_PROC_VECTORS]) ();

static FontPathElementPtr *font_path_elements = (FontPathElementPtr *) 0;
static int  num_fpes = 0;
static FPEFunctions *fpe_functions = (FPEFunctions *) 0;
static int  num_fpe_types = 0;

static int  num_slept_fpes = 0;
static int  size_slept_fpes = 0;
static FontPathElementPtr *slept_fpes = (FontPathElementPtr *) 0;

#define	NUM_IDS_PER_CLIENT	5

int
FontToFSError(err)
    int         err;
{
    switch (err) {
    case Successful:
	return FSSuccess;
    case AllocError:
	return FSBadAlloc;
    case BadFontName:
    case BadFontPath:
	return FSBadName;
    case BadFontFormat:
	return FSBadFormat;
    case BadCharRange:
	return FSBadRange;
    default:
	return err;
    }
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
	fsfree(fpe->name);
	fsfree(fpe);
    }
}

/*
 * tries to find the font name in the font name cache
 */
FontPtr
FindCachedFontName(name, namelen)
    char       *name;
    int         namelen;
{
    return NullFont;
}

int
CacheFontName(name, namelen, pfont)
    char       *name;
    int         namelen;
    FontPtr     pfont;
{
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
	    fsrealloc(slept_fpes,
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
	return FSSuccess;	/* ignore -1 return from select XXX */
    /* wake up any fpe's that may be waiting for information */
    for (i = 0; i < num_slept_fpes; i++) {
	fpe = slept_fpes[i];
	(void) (*fpe_functions[fpe->type].wakeup_fpe) (fpe, LastSelectMask);
    }

    return FSSuccess;
}

static Bool
add_id_to_list(ids, fid)
    FontIDListPtr ids;
    Font        fid;
{
    Font       *newlist;

    /*
     * assumes the list is packed tightly
     */
    if (ids->num == ids->size) {
	/* increase size of array */
	newlist = (Font *) fsrealloc(ids->client_list,
			      sizeof(Font) * ids->size + NUM_IDS_PER_CLIENT);
	if (!newlist)
	    return FALSE;
	ids->client_list = newlist;
	ids->size += NUM_IDS_PER_CLIENT;
    }
    ids->client_list[ids->num++] = fid;
    return TRUE;
}

static void
remove_id_from_list(ids, fid)
    FontIDListPtr ids;
    Font        fid;
{
    int         i,
                j;

    for (i = 0; i < ids->num; i++) {
	if (ids->client_list[i] == fid) {
	    /* a bcopy() might be better here */
	    for (j = i + 1; j < ids->num; j++) {
		ids->client_list[j - 1] = ids->client_list[j];
	    }
	    ids->num--;
	    return;
	}
    }
    assert(0);
}

static      FontIDListPtr
make_clients_id_list()
{
    FontIDListPtr ids;
    Font       *fids;

    ids = (FontIDListPtr) fsalloc(sizeof(FontIDListRec));
    fids = (Font *) fsalloc(sizeof(Font) * NUM_IDS_PER_CLIENT);
    if (!ids || !fids) {
	fsfree(ids);
	fsfree(fids);
	return (FontIDListPtr) 0;
    }
    bzero((char *) fids, sizeof(Font) * NUM_IDS_PER_CLIENT);
    ids->client_list = fids;
    ids->size = NUM_IDS_PER_CLIENT;
    ids->num = 0;
    return ids;
}

static Bool
do_open_font(client, c)
    ClientPtr   client;
    OFclosurePtr c;
{
    FontPtr     pfont = NullFont;
    FontPathElementPtr fpe;
    int         err;
    int         i;
    char       *alias,
               *newname;
    int         newlen;
    ClientFontPtr cfp;
    fsOpenBitmapFontReply rep;
    Font        orig,
               *fids;
    FontIDListPtr *idlist,
                ids;

    while (c->current_fpe < c->num_fpes) {
	fpe = c->fpe_list[c->current_fpe];
	err = (*fpe_functions[fpe->type].open_font)
	    ((pointer) c->client, fpe, c->flags,
	     c->fontname, c->fnamelen, c->format, c->format_mask,
	     c->fontid, &pfont, &alias);

	if (err == FontNameAlias && alias) {
	    newlen = strlen(alias);
	    newname = (char *) fsrealloc(c->fontname, newlen);
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
		ClientSleep(client, do_open_font, (pointer) c);
	    }
	    return TRUE;
	}
	break;
    }
    if (err != Successful) {
	goto dropout;
    }
    if (!pfont) {
	err = BadFontName;
	goto dropout;
    }
    cfp = (ClientFontPtr) fsalloc(sizeof(ClientFontRec));
    if (!cfp) {
	err = AllocError;
	goto dropout;
    }
    cfp->font = pfont;
    cfp->clientindex = c->client->index;

    /* either pull out the original id or make the array */
    if (pfont->refcnt != 0) {
	idlist = (FontIDListPtr *) pfont->svrPrivate;
	ids = idlist[c->client->index];
	if (!ids) {
	    ids = make_clients_id_list();
	    if (!ids) {
		err = AllocError;
		fsfree(ids);
		fsfree(cfp);
		goto dropout;
	    }
	    idlist[c->client->index] = ids;
	}
	orig = ids->client_list[0];
	if (!orig) {
	    add_id_to_list(ids, c->fontid);
	}
    } else {
	idlist = (FontIDListPtr *) fsalloc(sizeof(FontIDListPtr) * MAXCLIENTS);
	if (!idlist) {
	    err = AllocError;
	    fsfree(cfp);
	    goto dropout;
	}
	ids = make_clients_id_list();
	if (!ids) {
	    err = AllocError;
	    fsfree(idlist);
	    fsfree(cfp);
	    goto dropout;
	}
	bzero((char *) idlist, (sizeof(FontIDListPtr) * MAXCLIENTS));
	idlist[c->client->index] = ids;
	orig = (Font) 0;
	pfont->svrPrivate = (pointer) idlist;
	add_id_to_list(ids, c->fontid);
    }
    if (!orig &&
	 !AddResource(c->client->index, c->fontid, RT_FONT, (pointer) cfp)) {
	fsfree(cfp);
	fsfree(pfont->svrPrivate);
	pfont->svrPrivate = (pointer) 0;
	err = AllocError;
	goto dropout;
    }
    /* send the reply */
    rep.type = FS_Reply;
    rep.originalid = orig;
    rep.sequenceNumber = client->sequence;
    rep.length = sizeof(fsOpenBitmapFontReply) >> 2;
    WriteReplyToClient(client,
		       sizeof(fsOpenBitmapFontReply), &rep);
    if (pfont->refcnt == 0) {
	pfont->fpe = fpe;
	UseFPE(pfont->fpe);
    }
    pfont->refcnt++;
dropout:
    if (err != Successful) {
	SendErrToClient(c->client, FontToFSError(err), (pointer) &(c->fontid));
    }
    if (c->slept)
	ClientWakeup(c->client);
    for (i = 0; i < c->num_fpes; i++) {
	FreeFPE(c->fpe_list[i]);
    }
    fsfree(c->fpe_list);
    fsfree(c->fontname);
    fsfree(c);
    return TRUE;
}

int
OpenFont(client, fid, format, format_mask, namelen, name)
    ClientPtr   client;
    Font        fid;
    fsBitmapFormat format;
    fsBitmapFormatMask format_mask;
    int         namelen;
    char       *name;
{
    FontPtr     pfont;
    fsOpenBitmapFontReply rep;
    OFclosurePtr c;
    Font        orig;
    FontIDListPtr *idlist,
                ids;
    int         i;

    /* check name cache */
    pfont = FindCachedFontName(name, namelen);
    if (pfont) {		/* found it */
	ClientFontPtr cfp;

	idlist = (FontIDListPtr *) pfont->svrPrivate;
	ids = idlist[client->index];
	orig = ids->client_list[0];
	if (!orig) {
	    cfp = (ClientFontPtr) fsalloc(sizeof(ClientFontRec));
	    if (!cfp) {
		SendErrToClient(client, FSBadAlloc, (pointer) 0);
		return FSBadAlloc;
	    }
	    cfp->font = pfont;
	    cfp->clientindex = client->index;
	    if (!AddResource(client->index, fid, RT_FONT, (pointer) cfp)) {
		SendErrToClient(client, FSBadAlloc, (pointer) 0);
		return FSBadAlloc;
	    }
	}
	if (!add_id_to_list(ids, fid)) {
	    SendErrToClient(client, FSBadAlloc, (pointer) 0);
	    return FSBadAlloc;
	}
	pfont->refcnt++;
	rep.type = FS_Reply;
	rep.originalid = orig;
	rep.sequenceNumber = client->sequence;
	rep.length = sizeof(fsOpenBitmapFontReply) >> 2;
	WriteReplyToClient(client,
			   sizeof(fsOpenBitmapFontReply), &rep);
	return FSSuccess;
    }
    c = (OFclosurePtr) fsalloc(sizeof(OFclosureRec));
    if (!c)
	return FSBadAlloc;
    c->fontname = (char *) fsalloc(namelen);
    if (!c->fontname) {
	fsfree(c);
	return FSBadAlloc;
    }
    /*
     * copy the current FPE list, so that if it gets changed by another client
     * while we're blocking, the request still appears atomic
     */
    c->fpe_list = (FontPathElementPtr *)
	fsalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	fsfree(c->fontname);
	fsfree(c);
	return FSBadAlloc;
    }
    bcopy(name, c->fontname, namelen);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->fontid = fid;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->fnamelen = namelen;
    c->slept = FALSE;
    c->flags = (FontLoadInfo | FontLoadProps);
    c->format = format;
    c->format_mask = format_mask;

    (void) do_open_font(client, c);
    return FSSuccess;
}

static int
close_font(pfont)
    FontPtr     pfont;
{
    FontPathElementPtr fpe;

    assert(pfont);
    if (--pfont->refcnt == 0) {
	fpe = pfont->fpe;
	fsfree((char *) pfont->svrPrivate);
	(*fpe_functions[fpe->type].close_font) (fpe, pfont);
	FreeFPE(fpe);
    }
    return FSSuccess;
}

int
CloseClientFont(cfp, fid)
    ClientFontPtr cfp;
    FSID        fid;
{
    FontIDListPtr *idlist,
                ids;

    assert(cfp);
    /* clear original id */
    idlist = (FontIDListPtr *) cfp->font->svrPrivate;
    ids = idlist[cfp->clientindex];
    remove_id_from_list(ids, fid);
    return close_font(cfp->font);
}

/*
 * search all the knwon FPE prefixes looking for one to match the given
 * FPE name
 */
static int
determine_fpe_type(name)
    char       *name;
{
    int         i,
                j,
                len;
    FontNamesPtr names;
    char        namebuf[64];
    char       *t;

    /* copy the prefix */
    t = index(name, ':');
    len = t - name;
    bcopy(name, namebuf, len);
    namebuf[len] = '\0';

    for (i = 0; i < num_fpe_types; i++) {
	names = fpe_functions[i].renderer_names;
	for (j = 0; j < names->nnames; j++) {
	    if (strncmp(namebuf, names->names[j],
			min(len, names->length[j])) == 0) {
		return i;
	    }
	}
    }
    return -1;
}

static void
free_font_path(list, n)
    FontPathElementPtr *list;
    int         n;
{
    int         i;

    for (i = 0; i < n; i++) {
	FreeFPE(list[i]);
    }
    fsfree((char *) list);
}

static      FontPathElementPtr
find_existing_fpe(list, num, name, len)
    FontPathElementPtr *list;
    int         num;
    char       *name;
    int         len;
{
    FontPathElementPtr fpe;
    int         i;

    for (i = 0; i < num; i++) {
	fpe = list[i];
	if (fpe->name_length == len && bcmp(name, fpe->name, len) == 0)
	    return fpe;
    }
    return (FontPathElementPtr) 0;
}

/*
 * does the work of setting up the fpe list
 *
 * paths should be a counted string
 */
static int
set_font_path_elements(npaths, paths, bad)
    int         npaths;
    char       *paths;
    int        *bad;
{
    int         i,
                err;
    unsigned int len, sublen;
    char       *cp = paths;
    char	*colon;
    FontPathElementPtr fpe,
               *fplist;

    fplist = (FontPathElementPtr *)
	fsalloc(sizeof(FontPathElementPtr) * npaths);
    if (!fplist) {
	*bad = 0;
	return FSBadAlloc;
    }
    for (i = 0; i < npaths; i++) {
	len = (unsigned int) (*cp++);
	if (len) {
	    /* if its already in our active list, just reset it */
	    /*
	     * note that this can miss FPE's in limbo -- may be worth catching
	     * them, though it'd muck up refcounting
	     */
	    fpe = find_existing_fpe(font_path_elements, num_fpes, cp, len);
	    if (fpe) {
		cp += len;
		err = (*fpe_functions[fpe->type].reset_fpe) (fpe);
		UseFPE(fpe);	/* since it'll be decref'd later when freed
				 * from the old list */
		fplist[i] = fpe;
		continue;
	    }
	    sublen = len;
	    colon = cp;
	    while (sublen && *colon != ':')
	    {
		sublen--;
		colon++;
	    }
	    if (!sublen)
	    {
		err = FSBadName;
		goto bail;
	    }
	    colon++;
	    sublen = len - (colon - cp);
	    /* must be new -- make it */
	    fpe = (FontPathElementPtr) fsalloc(sizeof(FontPathElementRec));
	    if (!fpe) {
		err = FSBadAlloc;
		goto bail;
	    }
	    fpe->name = (char *) fsalloc(sublen + 1);
	    if (!fpe->name) {
		fsfree(fpe);
		err = FSBadAlloc;
		goto bail;
	    }
	    fpe->refcount = 1;
	    fplist[i] = fpe;
	    fpe->type = determine_fpe_type(cp);
	    
	    strncpy(fpe->name, (char *) colon, (int) sublen);
	    cp += len;
	    fpe->name[sublen] = '\0';
	    fpe->name_length = len;
	    if (fpe->type == -1) {
		fsfree(fpe->name);
		fsfree(fpe);
		err = FSBadName;
		goto bail;
	    }
	    err = (*fpe_functions[fpe->type].init_fpe) (fpe);
	    if (err != Successful) {
		fsfree(fpe->name);
		fsfree(fpe);
		err = FontToFSError(err);
		goto bail;
	    }
	}
    }
    free_font_path(font_path_elements, num_fpes);
    font_path_elements = fplist;
    num_fpes = npaths;
    return FSSuccess;
bail:
    *bad = i;
    while (--i >= 0)
	FreeFPE(fplist[i]);
    fsfree(fplist);
    return err;
}

/*
 * expects comma seperated string
 */
int
SetFontCatalogue(str, badpath)
    char       *str;
    int        *badpath;
{
    int         len,
                npaths;
    char       *paths,
               *end,
               *p;
    int         err;

    len = strlen(str) + 1;
    paths = p = (char *) ALLOCATE_LOCAL(len);
    npaths = 0;

    while (*str) {
	end = index(str, ',');
	if (!end) {
	    end = str + strlen(str);
	}
	*p++ = len = end - str;
	bcopy(str, p, len);
	npaths++;
	str += len;		/* skip entry */
	if (*str == ',')
	    str++;		/* skip any comma */
	p += len;
    }

    err = set_font_path_elements(npaths, paths, badpath);

    DEALLOCATE_LOCAL(paths);

    return err;
}

static int
do_list_fonts_helper(client, c, func)
    ClientPtr   client;
    LFclosurePtr c;
    Bool        (*func) ();
{
    FontPathElementPtr fpe;
    int         err = Successful;

    /* try each fpe in turn, returning if one wants to be blocked */
    while (c->current_fpe < c->num_fpes && c->names->nnames <= c->maxnames) {
	fpe = c->fpe_list[c->current_fpe];

	err = (*fpe_functions[fpe->type].list_fonts)
	    ((pointer) c->client, fpe, c->pattern, c->patlen,
	     c->maxnames - c->names->nnames, c->names);

	if (err == Suspended) {
	    if (!c->slept) {
		c->slept = TRUE;
		ClientSleep(client, func, (pointer) c);
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
do_list_fonts(client, c)
    ClientPtr   client;
    LFclosurePtr c;
{
    int         err;
    fsListFontsReply reply;
    FontNamesPtr names;
    int         stringLens,
                i,
                nnames;
    char       *bufptr,
               *bufferStart;

    err = do_list_fonts_helper(client, c, do_list_fonts);
    if (err == Suspended)
	return TRUE;

    if (err != Successful) {
	SendErrToClient(client, FontToFSError(err), (pointer) 0);
	goto bail;
    }
    names = c->names;
    nnames = names->nnames;
    client = c->client;
    stringLens = 0;
    for (i = 0; i < nnames; i++)
	stringLens += names->length[i];

    reply.type = FS_Reply;
    reply.length = (sizeof(fsListFontsReply) + stringLens + nnames + 3) >> 2;
    reply.following = 0;
    reply.nFonts = nnames;
    reply.sequenceNumber = client->sequence;

    bufptr = bufferStart = (char *) ALLOCATE_LOCAL(reply.length << 2);

    if (!bufptr && reply.length) {
	SendErrToClient(client, FSBadAlloc, (pointer) 0);
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
    WriteReplyToClient(client, sizeof(fsListFontsReply), &reply);
    (void) WriteToClient(client, stringLens + nnames, bufferStart);
    DEALLOCATE_LOCAL(bufferStart);
bail:
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    fsfree(c->fpe_list);
    FreeFontNames(names);
    fsfree(c->pattern);
    fsfree(c);
    return TRUE;
}

static      LFclosurePtr
make_list_fonts_closure(client, pattern, length, maxnames)
    ClientPtr   client;
    unsigned char *pattern;
    unsigned int length;
    unsigned int maxnames;
{
    LFclosurePtr c;
    int         i;

    c = (LFclosurePtr) fsalloc(sizeof(LFclosureRec));
    if (!c)
	return (LFclosurePtr) 0;
    c->pattern = (char *) fsalloc(length);
    if (!c->pattern) {
	fsfree(c);
	return (LFclosurePtr) 0;
    }
    c->names = MakeFontNamesRecord(maxnames < 100 ? maxnames : 100);
    if (!c->names) {
	fsfree(c->pattern);
	fsfree(c);
	return (LFclosurePtr) 0;
    }
    c->fpe_list = (FontPathElementPtr *)
	fsalloc(sizeof(FontPathElementPtr) * num_fpes);
    bcopy(pattern, c->pattern, length);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->patlen = length;
    c->client = client;
    c->maxnames = maxnames;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->slept = FALSE;
    return c;
}

int
ListFonts(client, length, pattern, maxnames)
    ClientPtr   client;
    unsigned int length;
    unsigned char *pattern;
    unsigned int maxnames;
{
    LFclosurePtr c;

    if (!num_fpes)
	return FSSuccess;

    c = make_list_fonts_closure(client, pattern, length, maxnames);
    if (!c)
	return FSBadAlloc;

    (void) do_list_fonts(client, c);
    return FSSuccess;
}

do_list_fonts_with_info(client, c)
    ClientPtr   client;
    LFWXIclosurePtr c;
{
    FontPathElementPtr fpe;
    FontNamesPtr names;
    int         err = Successful;
    char       *name;
    int         namelen;
    int         numFonts;
    FontInfoRec fontInfo,
               *pFontInfo;
    fsListFontsWithXInfoReply *reply;
    int         length;
    fsPropInfo *prop_info;
    fsFontHeader hdr;
    int         lenpropdata;
    int         i;

    names = c->names;
    while (c->current_name < names->nnames) {
	while (c->current_fpe < c->num_fpes) {
	    fpe = c->fpe_list[c->current_fpe];
	    err = Successful;
	    if (!c->list_started) {
		err = (*fpe_functions[fpe->type].start_list_fonts_with_info)
		    ((pointer) c->client, fpe, c->name, c->namelen, 1,
		     &c->private);
		if (err == Suspended) {
		    if (!c->slept) {
			ClientSleep(client, do_list_fonts_with_info,
				    (pointer) c);
			c->slept = TRUE;
		    }
		    return TRUE;
		}
		if (err == Successful)
		    c->list_started = TRUE;
	    }
	    if (err == Successful) {
		name = 0;
		pFontInfo = &fontInfo;
		err = (*fpe_functions[fpe->type].list_next_font_with_info)
		    ((pointer) c->client, fpe, &name, &namelen,
		     &pFontInfo, &numFonts, c->private);
		if (err == Suspended) {
		    if (!c->slept) {
			ClientSleep(client, do_list_fonts_with_info,
				    (pointer) c);
			c->slept = TRUE;
		    }
		    return TRUE;
		}
		c->list_started = FALSE;
	    }
	    if (err == FontNameAlias && name) {
		c->name = name;
		c->namelen = namelen;
		c->current_fpe = 0;
		continue;
	    }
	    if (err == BadFontName) {
		c->current_fpe++;
		continue;
	    }
	    if (err == Successful) {
		name = names->names[c->current_name];
		namelen = names->length[c->current_name];
		length = sizeof(*reply) + pFontInfo->nprops * sizeof(xFontProp);
		reply = c->reply;
		if (c->length < length) {
		    reply = (fsListFontsWithXInfoReply *)
			fsrealloc(c->reply, length);
		    if (!reply) {
			err = AllocError;
			break;
		    }
		    c->reply = reply;
		    c->length = length;
		}
		err = LoadFontHeader(pFontInfo, &hdr, &prop_info);
		if (err != Successful) {
		    break;
		}
		lenpropdata = sizeof(fsPropInfo) +
		    prop_info->num_offsets * sizeof(fsPropOffset) +
		    prop_info->data_len;
		reply->type = FS_Reply;
		reply->length = (sizeof(fsListFontsWithXInfoReply) +
				 sizeof(fsFontHeader) +
				 lenpropdata +
				 namelen + 3) >> 2;
		reply->sequenceNumber = client->sequence;
		reply->nameLength = namelen;
		reply->nReplies = names->nnames - (c->current_name + 1);
		reply->header = hdr;
		WriteReplyToClient(client, sizeof(fsListFontsWithXInfoReply),
				   reply);
		(void) WriteToClient(client, namelen, name);
		if (client->swapped)
		    SwapPropInfo(prop_info);
		(void) WriteToClient(client, lenpropdata, (char *) prop_info);
		if (pFontInfo == &fontInfo) {
		    fsfree(fontInfo.props);
		    fsfree(fontInfo.isStringProp);
		}
		fsfree(prop_info);
	    }
	    break;
	}
	c->current_name++;
	if (c->current_name < names->nnames) {
	    c->name = names->names[c->current_name];
	    c->namelen = names->length[c->current_name];
	}
	c->current_fpe = 0;
	if (err == BadFontName) {
	    /*
	     * fake things out in case this is the last one and we're about to
	     * drop out of the loop
	     */
	    err = Successful;
	    continue;
	}
	if (err != Successful)
	    break;
    }
    if (err == Successful) {
	reply = c->reply;
	length = sizeof(fsListFontsWithXInfoReply);
	if (c->length < length) {
	    reply = (fsListFontsWithXInfoReply *) fsrealloc(c->reply, length);
	    if (reply) {
		c->reply = reply;
		c->length = length;
	    } else
		err = AllocError;
	}
	if (err == Successful) {
	    bzero((char *) reply, sizeof(fsListFontsWithXInfoReply));
	    reply->type = FS_Reply;
	    reply->sequenceNumber = client->sequence;
	    reply->length = sizeof(fsListFontsWithXInfoReply) >> 2;
	    WriteReplyToClient(client, length, reply);
	}
    }
    if (err != Successful)
	SendErrToClient(client, FontToFSError(err), (pointer) 0);
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    fsfree(c->fpe_list);
    FreeFontNames(names);
    fsfree(c);
    return TRUE;
}

do_start_list_fonts_with_info(client, lfc)
    ClientPtr   client;
    LFclosurePtr lfc;
{
    int         err;
    int         i;
    LFWXIclosurePtr c;

    err = do_list_fonts_helper(client, lfc, do_start_list_fonts_with_info);
    if (err == Suspended)
	return TRUE;
    if (err != Successful) {
	SendErrToClient(client, FontToFSError(err), (pointer) 0);
	goto bail;
    }
    c = (LFWXIclosurePtr) fsalloc(sizeof *c);
    if (!c) {
	SendErrToClient(client, FSBadAlloc, (pointer) 0);
	goto bail;
    }
    c->client = client;
    c->current_fpe = 0;
    c->num_fpes = lfc->num_fpes;
    c->fpe_list = lfc->fpe_list;
    c->names = lfc->names;
    c->current_name = 0;
    c->slept = lfc->slept;
    if (c->slept) {
	ClientWakeup(client);
	c->slept = FALSE;
    }
    c->name = c->names->names[0];
    c->namelen = c->names->length[0];
    c->reply = 0;
    c->length = 0;
    c->list_started = FALSE;
    c->private = 0;
    fsfree(lfc->pattern);
    do_list_fonts_with_info(client, c);
    return TRUE;
bail:
    if (lfc->slept)
	ClientWakeup(client);
    for (i = 0; i < lfc->num_fpes; i++)
	FreeFPE(lfc->fpe_list[i]);
    fsfree(lfc->fpe_list);
    FreeFontNames(lfc->names);
    fsfree(lfc->pattern);
    fsfree(lfc);
    return TRUE;
}

int
StartListFontsWithInfo(client, length, pattern, maxNames)
    ClientPtr   client;
    int         length;
    unsigned char *pattern;
    int         maxNames;
{
    LFclosurePtr c;

    c = make_list_fonts_closure(client, pattern, length, maxNames);
    if (!c)
	return FSBadAlloc;
    (void) do_start_list_fonts_with_info(client, c);
    return FSSuccess;
}

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
    new = (FPEFunctions *) fsrealloc(fpe_functions,
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
}

/* convenience functions for FS interface */

FontPtr
find_old_font(client, id)
    ClientPtr   client;
    FSID        id;
{
    ClientFontPtr cfp;

    cfp = (ClientFontPtr) LookupIDByType(client->index, id, RT_FONT);

    return cfp->font;
}

Font
GetNewFontClientID()
{
    return FakeClientID(0);
}

int
StoreFontClientFont(pfont, id)
    FontPtr	pfont;
    Font	id;
{
    return AddResource(0, id, RT_FONT, (pointer)pfont);
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
