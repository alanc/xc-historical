/* $XConsortium: fserve.c,v 1.5 91/05/29 15:28:11 keith Exp $ */
/*
 *
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Network Computing
 * Devices makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 *
 * $NCDId: @(#)fserve.c,v 1.7 1991/06/10 17:42:44 lemke Exp $
 */
/*
 * font server specific font access
 */

#include	<X11/X.h>
#include	<X11/Xos.h>
#include	"FS.h"
#include	"FSproto.h"
#include	"fontmisc.h"
#include	"fontstruct.h"
#include	"fservestr.h"
#include	<errno.h>

#ifdef NCD
#include	<ncd/nvram.h>
#endif

#define	check_conn(conn)	if (conn->fs_fd == -1) return AllocError

extern int  errno;

extern FontPtr find_old_font();

extern int  fs_build_range();

static int  fs_read_glyphs();
static int  fs_read_list();
static int  fs_read_list_info();

static int  fs_font_type;
extern unsigned long fs_fd_mask[];

static int  fs_block_handler();
static int  fs_wakeup();

/*
 * Font server access
 *
 * the basic idea for the non-blocking access is to have the function
 * called multiple times until the actual data is returned, instead
 * of ClientBlocked.
 *
 * the first call to the function will cause the request to be sent to
 * the font server, and a block record to be stored in the fpe's list
 * of outstanding requests.  the FS block handler also sticks the
 * proper set of fd's into the select mask.  when data is ready to be
 * read in, the FS wakup handler will be hit.  this will read the
 * data off the wire into the proper block record, and then signal the
 * client that caused the block so that it can restart.  it will then
 * call the access function again, which will realize that the data has
 * arrived and return it.
 */


/* XXX this should probably be a macro once its fully debugged */
/* ARGSUSED */
static void
_fs_add_req_log(conn, opcode)
    FSFpePtr    conn;
    int         opcode;
{

#ifdef DEBUG
    conn->reqbuffer[conn->reqindex++] = opcode;
    if (conn->reqindex == REQUEST_LOG_SIZE)
	conn->reqindex = 0;
#endif

    conn->current_seq++;
}

static Bool
fs_name_check(name)
    char       *name;
{
    if (name[0] == ':')
	return TRUE;
    return FALSE;
}

/*
 * sends the stuff that's meaningful to a newly opened or reset FS
 */
static int
fs_send_init_packets(fpe)
    FontPathElementPtr fpe;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsSetResolutionReq req;
    int         num_res;
    fsResolution *res;
    extern fsResolution *GetClientResolutions();

    res = GetClientResolutions(&num_res);
    if (num_res == 0)
	return Successful;
    req.reqType = FS_SetResolution;
    req.num_resolutions = num_res;
    req.length = (sizeof(fsSetResolutionReq) +
		  (num_res * sizeof(fsResolution)) + 3) >> 2;

    _fs_add_req_log(conn, FS_SetResolution);
    _fs_write(conn, (char *) &req, sizeof(fsSetResolutionReq));
    _fs_write_pad(conn, (char *) res, (num_res * sizeof(fsResolution)));

    return Successful;
}

/*
 * the wakeup handlers have to be set when the FPE is open, and not
 * removed until it is freed, in order to handle unexpected data, like
 * events
 */
/* ARGSUSED */
static int
fs_init_fpe(fpe, format)
    FontPathElementPtr fpe;
    fsBitmapFormat format;
{
    FSFpePtr    conn;
    char       *name;

    /* open font server */
    /* create FS specific fpe info */
    errno = 0;
    name = index(fpe->name, ':');

    if (name)
	name++;			/* skip ':' */
    else
	name = fpe->name;

    conn = _fs_open_server(name);
    if (conn) {
	fpe->private = (pointer) conn;
	if (init_fs_handlers(fpe, fs_block_handler) !=
		Successful)
	    return AllocError;
	_fs_set_bit(fs_fd_mask, conn->fs_fd);

#ifdef NCD
	if (configData.ExtendedFontDiags)
	    printf("Connected to font server \"%s\"\n", name);
#endif

	return fs_send_init_packets(fpe);
    }

#ifdef DEBUG
    fprintf(stderr, "failed to connect to FS \"%s\"\n", name);
#endif

#ifdef NCD
    if (configData.ExtendedFontDiags)
	printf("Failed to connect to font server \"%s\"\n", name);
#endif

    return (errno == ENOMEM) ? AllocError : BadFontPath;
}

static int
fs_reset_fpe(fpe)
    FontPathElementPtr fpe;
{
    return fs_send_init_packets(fpe);
}

/*
 * this shouldn't be called till all refs to the FPE are gone
 */

static int
fs_free_fpe(fpe)
    FontPathElementPtr fpe;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    assert(fpe->refcount == 0);

    /* close font server */
    (void) close(conn->fs_fd);

    _fs_bit_clear(fs_fd_mask, conn->fs_fd);
    remove_fs_handlers(fpe, fs_block_handler, !_fs_any_bit_set(fs_fd_mask));

    xfree((char *) conn);
    fpe->private = (pointer) 0;

#ifdef NCD
    if (configData.ExtendedFontDiags)
	printf("Disconnected from font server \"%s\"\n", fpe->name);
#endif

    return Successful;
}

static      FSBlockDataPtr
fs_new_block_rec(fpe, client, type)
    FontPathElementPtr fpe;
    pointer     client;
    int         type;
{
    FSBlockDataPtr blockrec,
                br;
    FSFpePtr    fsfpe = (FSFpePtr) fpe->private;

    blockrec = (FSBlockDataPtr) xalloc(sizeof(FSBlockDataRec));
    if (!blockrec)
	return (FSBlockDataPtr) 0;
    switch (type) {
    case FS_OPEN_FONT:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedFontRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;
    case FS_LOAD_GLYPHS:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedGlyphRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;

    case FS_LIST_FONTS:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedListRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;
    case FS_LIST_WITH_INFO:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedListInfoRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;
    case FS_LOAD_EXTENTS:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedExtentRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;
    case FS_LOAD_BITMAPS:
	blockrec->data = (pointer) xalloc(sizeof(FSBlockedBitmapRec));
	if (!blockrec->data) {
	    xfree(blockrec);
	    return (FSBlockDataPtr) 0;
	}
	break;
    default:
	assert(0);
    }
    blockrec->client = client;
    blockrec->sequence_number = fsfpe->current_seq;
    blockrec->type = type;
    blockrec->next = (FSBlockDataPtr) 0;

    /* stick it on the end of the list (since its expected last) */
    br = (FSBlockDataPtr) fsfpe->blocked_requests;
    if (!br) {
	fsfpe->blocked_requests = (pointer) blockrec;
    } else {
	while (br->next)
	    br = br->next;
	br->next = blockrec;
    }

    return blockrec;
}

static void
fs_remove_blockrec(conn, blockrec)
    FSFpePtr    conn;
    FSBlockDataPtr blockrec;
{
    FSBlockDataPtr br,
                last;

    last = (FSBlockDataPtr) 0;
    br = (FSBlockDataPtr) conn->blocked_requests;
    while (br) {
	if (br == blockrec) {
	    if (last)
		last->next = br->next;
	    else
		conn->blocked_requests = (pointer) br->next;
	    xfree(br->data);
	    xfree(br);
	    return;
	}
	last = br;
	br = br->next;
    }
    assert(0);
}

static void
fs_cleanup_font(bfont)
    FSBlockedFontPtr bfont;
{
    FontPtr     pfont;
    FSFontPtr   fsfont;
    FSFontDataRec *fsd;

    pfont = bfont->pfont;
    fsfont = (FSFontPtr) pfont->fontPrivate;
    fsd = (FSFontDataRec *) pfont->fpePrivate;

    /* xfree better be able to handle NULL */
    xfree(fsfont->encoding);
    xfree(fsfont->bitmaps);
    xfree(fsfont);
    bfont->pfont = (FontPtr) 0;
    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);
    xfree(pfont);

    bfont->errcode = AllocError;

    /* make sure the FS knows we choked on it */
    fs_send_close_font(fsd->fpe, bfont->fontid);
    xfree(fsd);
}

static int
fs_read_open_font(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FontPtr     newfont;
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSFontDataPtr fsd = (FSFontDataPtr) (bfont->pfont->fpePrivate);
    fsOpenBitmapFontReply rep;

    check_conn(conn);

    /* pull out the OpenFont reply */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));

    if (rep.type == FS_Error) {
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return BadFontName;
    } else {			/* get rest of reply */
	_fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
		 sizeof(fsOpenBitmapFontReply) - sizeof(fsReplyHeader));
    }

    /* make sure the sequence number is correct */
    assert(rep.type == FS_Reply);

    if (rep.originalid) {
	newfont = find_old_font(blockrec->client, rep.originalid);
	if (!newfont) {
	    /* XXX - something nasty happened */
	}
	assert(newfont->fpe);
	assert(newfont->fpePrivate);

	fs_send_close_font(fpe, bfont->fontid);

	xfree(bfont->pfont);
	bfont->fontid = rep.originalid;
	bfont->pfont = newfont;
	fsd->fontid = rep.originalid;

	bfont->state = FS_DONE_REPLY;
	return AccessDone;
    } else {
	bfont->state = FS_INFO_REPLY;
	/* ask for the next stage */
	fs_send_query_info(fpe, blockrec);
	return StillWorking;
    }
}

static int
fs_read_query_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXInfoReply rep;
    fsPropInfo  pi;
    fsPropOffset *po;
    pointer     pd;
    unsigned long prop_len;

    check_conn(conn);

    /* pull out the QueryXInfo reply */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsQueryXInfoReply) - sizeof(fsReplyHeader));

    /* check sequence number */
    assert(rep.type == FS_Reply);

    /* move the data over */
    (void) fs_convert_header(&rep.header, &bfont->pfont->info);

    _fs_read(conn, (char *) &pi, sizeof(fsPropInfo));
    prop_len = pi.num_offsets * sizeof(fsPropOffset);
    po = (fsPropOffset *) xalloc(prop_len);
    pd = (pointer) xalloc(pi.data_len);
    if (!po || !pd) {
	xfree(pd);
	xfree(po);
	/* clear the wire */
	_fs_drain_bytes(conn, prop_len + pi.data_len);
	/* clean up the font */
	fs_cleanup_font(bfont);
	return AllocError;
    }
    _fs_read_pad(conn, (char *) po, prop_len);
    _fs_read_pad(conn, (char *) pd, pi.data_len);
    (void) fs_convert_props(&pi, po, pd, &bfont->pfont->info);
    xfree(po);
    xfree(pd);

    bfont->state = FS_EXTENT_REPLY;

    fs_send_query_extents(fpe, blockrec);
    return StillWorking;
}

static int
fs_read_extent_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXExtents8Reply rep;
    int         i;
    CharInfoPtr ci,
                pCI;
    FSFontPtr   fsfont = (FSFontPtr) bfont->pfont->fontPrivate;
    fsCharInfo *fsci,
               *fscip;

    check_conn(conn);

    /* read the QueryXExtents reply */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsQueryXExtents8Reply) - sizeof(fsReplyHeader));

    /* check sequence number */
    assert(rep.type == FS_Reply);

    /* move the data over */
    ci = pCI = (CharInfoPtr) xalloc(sizeof(CharInfoRec) * rep.num_extents);
/* XXX this could be done with an ALLOCATE_LOCAL */
    fsci = (fsCharInfo *) xalloc(sizeof(fsCharInfo) * rep.num_extents);
    if (!pCI || !fsci) {
	xfree(pCI);
	xfree(fsci);
	/* clear the unusable data */
	_fs_drain_bytes(conn, sizeof(fsCharInfo) * rep.num_extents);
	fs_cleanup_font(bfont);
	return AllocError;
    }
    fsfont->encoding = pCI;
/* XXX - hack - use real default char */
    fsfont->pDefault = &pCI[0];

    _fs_read_pad(conn, (char *) fsci, sizeof(fsCharInfo) * rep.num_extents);
    for (i = 0, fscip = fsci; i < rep.num_extents; i++, ci++, fscip++) {
	fs_convert_char_info(fscip, ci);
    }

    xfree(fsci);
    bfont->state = FS_GLYPHS_REPLY;

    if (bfont->flags & FontLoadBitmaps) {
	fs_send_query_bitmaps(fpe, blockrec);
	return StillWorking;
    }
    return Successful;
}

/*
 * XXX should probably continue to read here if we can, but must be sure
 * its our packet waiting, rather than another interspersed
 */
static int
fs_do_open_font(fpe, blockrec, readheader)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
    Bool        readheader;
{
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    switch (bfont->state) {
    case FS_OPEN_REPLY:
	if (readheader) {
	    /* get the next header */
	    if (_fs_read(conn, (char *) &blockrec->header,
			 sizeof(fsReplyHeader)))
		return BadName;
	}
	bfont->errcode = fs_read_open_font(fpe, blockrec);
	if (bfont->errcode != StillWorking) {	/* already loaded, or error */
	    /* if font's already loaded, massage error code */
	    if (bfont->errcode == AccessDone)
		bfont->errcode = Successful;
	    return bfont->errcode;
	}
	/* if more data to read or Sync, fall thru, else return */
	if (!(bfont->flags & FontOpenSync)) {
	    return bfont->errcode;
	} else {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 sizeof(fsReplyHeader)))
		return BadName;
	}
    case FS_INFO_REPLY:
	bfont->errcode = fs_read_query_info(fpe, blockrec);
	if (bfont->errcode != StillWorking)
	    return bfont->errcode;
	if (!(bfont->flags & FontOpenSync)) {
	    return bfont->errcode;
	    /* if more data to read, fall thru, else return */
	} else {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 sizeof(fsReplyHeader)))
		return BadName;
	}
    case FS_EXTENT_REPLY:
	bfont->errcode = fs_read_extent_info(fpe, blockrec);
	if (bfont->errcode != StillWorking)
	    return bfont->errcode;
	if (!(bfont->flags & FontOpenSync)) {
	    return bfont->errcode;
	} else if (bfont->flags & FontLoadBitmaps) {
	    if (_fs_read(conn, (char *) &blockrec->header,
			 sizeof(fsReplyHeader)))
		return BadName;
	}
	/* fall thru */
    case FS_GLYPHS_REPLY:
	if (bfont->flags & FontLoadBitmaps) {
	    bfont->errcode = fs_read_glyphs(fpe, blockrec);
	}
	return bfont->errcode;
    default:
	assert(0);
	return bfont->errcode;
    }
}

/* ARGSUSED */
static int
fs_block_handler(data, wt, LastSelectMask)
    pointer     data;
    struct timeval **wt;
    long       *LastSelectMask;
{
    _fs_or_bits(LastSelectMask, LastSelectMask, fs_fd_mask);
    return Successful;
}

static void
fs_handle_unexpected(conn)
    FSFpePtr    conn;
{
    fsReplyHeader rep;

#ifdef DEBUG
    fprintf(stderr, "unexpected wakeup\n");
#endif

    if (_fs_read(conn, (char *) &rep, sizeof(fsReplyHeader)) == -1) {
	/* connection got toasted */

#ifdef DEBUG
	fprintf(stderr, "connection was dropped\n");
#endif

	conn->fs_fd = -1;
	return;
    }
    if (rep.type == FS_Event && rep.pad == KeepAlive) {
	fsNoopReq   req;

	/* ping it back */
	req.reqType = FS_Noop;
	req.length = sizeof(fsNoopReq) >> 2;
	_fs_add_req_log(conn, FS_Noop);
	_fs_write(conn, (char *) &req, sizeof(fsNoopReq));
    }
    /* this should suck up unexpected replies and events */
    _fs_eat_rest_of_error(conn, (fsError *) & rep);
}

static int
fs_wakeup(fpe, LastSelectMask)
    FontPathElementPtr fpe;
    unsigned long *LastSelectMask;
{
    FSBlockDataPtr blockrec,
                br;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    int         err;
    fsReplyHeader rep;

    /* see if there's any data to be read */
    if (_fs_is_bit_set(LastSelectMask, conn->fs_fd)) {
	/* see if we have any outstanding requests */
	br = (FSBlockDataPtr) conn->blocked_requests;

	/*
	 * make sure it isn't spurious - mouse events seem to trigger extra
	 * problems
	 */
	if (_fs_data_ready(conn) <= 0) {
	    return FALSE;
	}
	if (!br) {
	    fs_handle_unexpected(conn);
	    return FALSE;
	}
	/* get the header */
	if (_fs_read(conn, (char *) &rep, sizeof(fsReplyHeader)) == -1)
	    return FALSE;

	/* find the matching block record */
	while (br) {
	    if (br->sequence_number == (rep.sequenceNumber - 1)) {
		break;
	    }
	    br = br->next;
	}
	blockrec = br;
	assert(blockrec);

	bcopy((char *) &rep, (char *) &blockrec->header, sizeof(fsReplyHeader));

	/* go read it, and if we're done, wake up the appropriate client */
	switch (blockrec->type) {
	case FS_OPEN_FONT:
	    err = fs_do_open_font(fpe, blockrec, FALSE);
	    break;
	case FS_LOAD_GLYPHS:
	    err = fs_read_glyphs(fpe, blockrec);
	    break;
	case FS_LIST_FONTS:
	    err = fs_read_list(fpe, blockrec);
	    break;
	case FS_LIST_WITH_INFO:
	    err = fs_read_list_info(fpe, blockrec);
	    break;
	case FS_LOAD_EXTENTS:
	    err = fs_read_extents(fpe, blockrec);
	    break;
	case FS_LOAD_BITMAPS:
	    err = fs_read_bitmaps(fpe, blockrec);
	    break;
	default:
	    assert(0);
	}

	if (err != StillWorking)
	    ClientSignal(blockrec->client);
	/*
	 * Xx we could loop here and eat any additional replies, but it should
	 * feel more responsive for other clients if we come back later
	 */
    }
    return FALSE;
}

/*
 * sends the actual request out
 */
/* ARGSUSED */
static int
fs_send_open_font(client, fpe, flags, name, namelen, format, fmask, id, ppfont)
    pointer     client;
    FontPathElementPtr fpe;
    Mask        flags;
    char       *name;
    int         namelen;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    XID         id;
    FontPtr    *ppfont;
{
    FontPtr     newfont;
    FSBlockDataPtr blockrec;
    FSBlockedFontPtr blockedfont;
    FSFontDataPtr fsd;
    FSFontPtr   fsfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsOpenBitmapFontReq openreq;
    int         err = Suspended;
    unsigned char buf[256];
    XID         newid;

    check_conn(conn);

    newid = GetNewFontClientID ();

    /* make the font */
    newfont = (FontPtr) xalloc(sizeof(FontRec));

    /* and the FS data */
    fsd = (FSFontDataPtr) xalloc(sizeof(FSFontDataRec));

    fsfont = (FSFontPtr) xalloc(sizeof(FSFontRec));

    if (!newfont || !fsd || !fsfont) {
lowmem:
	xfree((char *) newfont);
	xfree((char *) fsd);
	xfree((char *) fsfont);
	return AllocError;
    }
    bzero((char *) newfont, sizeof(FontRec));
    bzero((char *) fsfont, sizeof(FSFontRec));
    bzero((char *) fsd, sizeof(FSFontDataRec));

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_OPEN_FONT);
    if (!blockrec) {
	goto lowmem;
    }
    newfont->refcnt = 0;
    newfont->maxPrivate = -1;
    newfont->devPrivates = (pointer *) 0;
    newfont->format = format;
    newfont->fpe = fpe;
    newfont->fpePrivate = (pointer) fsd;
    newfont->fontPrivate = (pointer) fsfont;
    fs_init_font(newfont);

    fsd->fontid = newid;
    fsd->fpe = fpe;

/* XXX - hack */
    /* for now, always load everything at startup time */
    flags |= FontLoadBitmaps;

    blockedfont = (FSBlockedFontPtr) blockrec->data;
    blockedfont->fontid = newid;
    blockedfont->pfont = newfont;
    blockedfont->state = FS_OPEN_REPLY;
    blockedfont->flags = flags;
    blockedfont->format = format;

    /* save the ID */
    if (!StoreFontClientFont(blockedfont->pfont, blockedfont->fontid)) {
	goto lowmem;
    }
    /* do an FS_OpenFont, FS_QueryXInfo and FS_QueryXExtents */
    buf[0] = (unsigned char) namelen;
    bcopy(name, (char *) &buf[1], namelen);
    namelen++;
    openreq.reqType = FS_OpenBitmapFont;
    openreq.fid = newid;
    openreq.format_hint = format;
    openreq.format_mask = fmask;
    openreq.length = (sizeof(fsOpenBitmapFontReq) + namelen + 3) >> 2;

    _fs_add_req_log(conn, FS_OpenBitmapFont);
    _fs_write(conn, (char *) &openreq, sizeof(fsOpenBitmapFontReq));
    _fs_write_pad(conn, (char *) buf, namelen);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	bcopy(name, buf, MIN(256, namelen));
	buf[MIN(256, namelen)] = '\0';
	printf("Requesting font \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    if (flags & FontOpenSync) {
	err = fs_do_open_font(fpe, blockrec, TRUE);
	if (blockedfont->errcode == Successful) {
	    *ppfont = blockedfont->pfont;
	} else {
	    xfree(blockedfont->pfont);
	}
	fs_remove_blockrec(conn, blockrec);
    }
    return err;
}

static int
fs_send_query_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXInfoReq inforeq;

    bfont = (FSBlockedFontPtr) blockrec->data;

    inforeq.reqType = FS_QueryXInfo;
    inforeq.id = bfont->fontid;
    inforeq.length = sizeof(fsQueryXInfoReq) >> 2;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXInfo);
    _fs_write(conn, (char *) &inforeq, sizeof(fsQueryXInfoReq));

    return Successful;
}

static int
fs_send_query_extents(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXExtents8Req extreq;

    bfont = (FSBlockedFontPtr) blockrec->data;

    extreq.reqType = FS_QueryXExtents8;
    extreq.range = fsTrue;
    extreq.fid = bfont->fontid;
    extreq.num_ranges = 0;
    extreq.length = sizeof(fsQueryXExtents8Req) >> 2;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXExtents8);
    _fs_write(conn, (char *) &extreq, sizeof(fsQueryXExtents8Req));

    return Successful;
}

static int
fs_send_query_bitmaps(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedFontPtr bfont;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXBitmaps8Req bitreq;


    bfont = (FSBlockedFontPtr) blockrec->data;

    /* send the request */
    bitreq.reqType = FS_QueryXBitmaps8;
    bitreq.fid = bfont->fontid;
    bitreq.format = bfont->format;
    bitreq.range = TRUE;
    bitreq.length = sizeof(fsQueryXBitmaps8Req) >> 2;
    bitreq.num_ranges = 0;

    blockrec->sequence_number = conn->current_seq;
    _fs_add_req_log(conn, FS_QueryXBitmaps8);
    _fs_write(conn, (char *) &bitreq, sizeof(fsQueryXBitmaps8Req));

    return Successful;
}

/* ARGSUSED */
static int
fs_open_font(client, fpe, flags, name, namelen, format, fmask, id, ppfont,
	     alias)
    pointer     client;
    FontPathElementPtr fpe;
    Mask        flags;
    char       *name;
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    int         namelen;
    XID         id;
    FontPtr    *ppfont;
    char      **alias;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSBlockDataPtr blockrec;
    FSBlockedFontPtr blockedfont;
    int         err;

    check_conn(conn);

    *alias = (char *) 0;
    /* XX if we find the blockrec for the font */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec != (FSBlockDataPtr) 0) {
	if (blockrec->type == FS_OPEN_FONT &&
		blockrec->client == client) {
	    blockedfont = (FSBlockedFontPtr) blockrec->data;
	    err = blockedfont->errcode;
	    if (err == Successful) {
		*ppfont = blockedfont->pfont;
	    } else {
		xfree(blockedfont->pfont);
	    }
	    /* cleanup */
	    fs_remove_blockrec(conn, blockrec);
	    return err;
	}
	blockrec = blockrec->next;
    }
    return fs_send_open_font(client, fpe, flags, name, namelen, format, fmask,
			     id, ppfont);
}

/* ARGSUSED */
static int
fs_send_close_font(fpe, id)
    FontPathElementPtr fpe;
    Font        id;
{
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsCloseReq  req;

    check_conn(conn);

    /* tell the font server to close the font */
    req.reqType = FS_CloseFont;
    req.length = sizeof(fsCloseReq) >> 2;
    req.id = id;
    _fs_add_req_log(conn, FS_CloseFont);
    _fs_write(conn, (char *) &req, sizeof(fsCloseReq));

    return Successful;
}

/* ARGSUSED */
static int
fs_close_font(fpe, pfont)
    FontPathElementPtr fpe;
    FontPtr     pfont;
{
    FSFontDataPtr fsd = (FSFontDataPtr) pfont->fpePrivate;

    fs_send_close_font(fpe, fsd->fontid);
    (*pfont->unload_font) (pfont);
    xfree(fsd);

    xfree(pfont->info.isStringProp);
    xfree(pfont->info.props);
    xfree(pfont);

    return Successful;
}

static int
fs_read_glyphs(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedGlyphPtr bglyph = (FSBlockedGlyphPtr) blockrec->data;
    FSBlockedFontPtr bfont = (FSBlockedFontPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    FSFontDataPtr fsd = (FSFontDataPtr) (bglyph->pfont->fpePrivate);
    FSFontPtr   fsdata = (FSFontPtr) bglyph->pfont->fontPrivate;
    fsQueryXBitmaps8Reply rep;
    fsOffset   *ppbits;
    pointer     pbitmaps;
    int         glyph_size,
                offset_size,
                i;

    check_conn(conn);

    /* get reply header */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return AllocError;
    }
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsQueryXBitmaps8Reply) - sizeof(fsReplyHeader));

    assert(rep.type == FS_Reply);

    /* allocate space for glyphs */
    offset_size = sizeof(fsOffset) * rep.num_chars;
    glyph_size = (rep.length << 2) - sizeof(fsQueryXBitmaps8Reply)
	- offset_size;
    ppbits = (fsOffset *) xalloc(offset_size);
    pbitmaps = (pointer) xalloc(glyph_size);
    if (!pbitmaps || !ppbits) {
	xfree(pbitmaps);
	xfree(ppbits);

	/* clear wire */
	_fs_drain_bytes_pad(conn, offset_size);
	_fs_drain_bytes_pad(conn, glyph_size);

	/* XXX may want to do something else for an FS_LOAD_GLYPHS */
	fs_cleanup_font(bfont);
	return AllocError;
    }
    /* read offsets */
    (void) _fs_read_pad(conn, (char *) ppbits, offset_size);

    /* adjust them */
    for (i = 0; i < rep.num_chars; i++) {
	ppbits[i].position = ppbits[i].position + (int) pbitmaps;
	fsdata->encoding[i].bits = (char *) ppbits[i].position;
    }

    /* read glyphs according to the range */
    (void) _fs_read_pad(conn, (char *) pbitmaps, glyph_size);

    fsdata->bitmaps = pbitmaps;

    xfree(ppbits);

    fsd->complete = TRUE;
    if (blockrec->type == FS_LOAD_GLYPHS)
	bglyph->done = TRUE;
    else
	bfont->state = FS_DONE_REPLY;
    return Successful;
}


static int
fs_send_load_glyphs(client, pfont, nchars, item_size, data)
    pointer     client;
    FontPtr     pfont;
    unsigned int nchars;
    int         item_size;
    unsigned char *data;
{
    FSBlockDataPtr blockrec;
    FSBlockedGlyphPtr blockedglyph;
    fsRange     range;
    int         res;
    fsQueryXBitmaps8Req req;
    int         err;
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FontPathElementPtr fpe = fsd->fpe;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    check_conn(conn);

    /*
     * see if the desired glyphs already exist, and return Successful if they
     * do, otherwise build up character range/character string
     */
    res = fs_build_range(pfont, nchars, item_size, &range, data);
    if (res == AccessDone)
	return Successful;

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LOAD_GLYPHS);
    if (!blockrec)
	return AllocError;
    blockedglyph = (FSBlockedGlyphPtr) blockrec->data;
    blockedglyph->pfont = pfont;
    blockedglyph->expected_range = range;

    /* send the request */
    req.reqType = FS_QueryXBitmaps8;
    req.fid = ((FSFontDataPtr) pfont->fpePrivate)->fontid;
    req.format = pfont->format;
    req.range = TRUE;
    req.length = sizeof(fsQueryXBitmaps8Req) >> 2;
    req.num_ranges = 0;
    _fs_add_req_log(conn, FS_QueryXBitmaps8);
    _fs_write(conn, (char *) &req, sizeof(fsQueryXBitmaps8Req));

/* XXX -- hack -- need to support blocking eventually */
    err = fs_read_glyphs(fpe, blockrec);

    fs_remove_blockrec(conn, blockrec);

    return err;
}

int
fs_load_glyphs(client, pfont, nchars, item_size, data)
    pointer     client;
    FontPtr     pfont;
    unsigned int nchars;
    int         item_size;
    unsigned char *data;
{

#ifdef notyet
    FSBlockDataPtr blockrec;
    FSBlockedGlyphPtr blockedglyph;

    /* see if the result is already there */
    /* XXX - this is pretty lame -- need a better solution */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LOAD_GLYPHS && blockrec->client == c->client) {
	    blockedglyph = (FSBlockedListPtr) blockrec->data;
	    if (blockedglyph->pfont == pfont && blockedglyph->done) {
		fs_remove_blockrec(conn, blockrec);
		return Successful;
	    }
	}
	blockrec = blockrec->next;
    }
#endif

    /* didn't find waiting record, so send a new one */
    return fs_send_load_glyphs(client, pfont, nchars, item_size, data);
}


/*
 * FS chaining extent loader
 */

static int
fs_read_extents(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedExtentPtr bextent = (FSBlockedExtentPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXExtents8Reply rep;
    fsCharInfo *extents;
    int         size;

    check_conn(conn);

    /* get reply header */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return AllocError;
    }
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsQueryXExtents8Reply) - sizeof(fsReplyHeader));

    assert(rep.type == FS_Reply);

    /* allocate space for glyphs */
    bextent->nextents = rep.num_extents;
    size = rep.num_extents * sizeof(fsCharInfo);
    extents = (fsCharInfo *) xalloc(size);
    if (!extents) {
	_fs_drain_bytes(conn, size);
	return AllocError;
    }
    /* read extents */
    (void) _fs_read_pad(conn, (char *) extents, size);

    bextent->extents = extents;

    bextent->done = TRUE;
    return Successful;
}

static int
fs_send_load_extents(client, pfont, flags, nranges, range)
    pointer     client;
    FontPtr     pfont;
    int         nranges;
    fsRange    *range;
{
    FSBlockDataPtr blockrec;
    FSBlockedExtentPtr blockedextent;
    int         res;
    fsQueryXBitmaps8Req req;
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FontPathElementPtr fpe = fsd->fpe;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    check_conn(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LOAD_EXTENTS);
    if (!blockrec)
	return AllocError;
    blockedextent = (FSBlockedExtentPtr) blockrec->data;
    blockedextent->pfont = pfont;
    blockedextent->expected_ranges = range;
    blockedextent->nranges = nranges;

    /*
     * see if the desired extents already exist, and return Successful if they
     * do, otherwise build up character range/character string
     */
    res = fs_check_extents(pfont, flags, nranges, range, blockrec);
    if (res == AccessDone)
	return Successful;

    /* send the request */
    req.reqType = FS_QueryXExtents8;
    req.fid = ((FSFontDataPtr) pfont->fpePrivate)->fontid;
    req.range = TRUE;
    req.length = (sizeof(fsQueryXExtents8Req) + sizeof(fsRange) * nranges) >> 2;
    req.num_ranges = nranges;
    _fs_add_req_log(conn, FS_QueryXExtents8);
    _fs_write(conn, (char *) &req, sizeof(fsQueryXExtents8Req));
    if (nranges)
	_fs_write(conn, (char *) range, sizeof(fsRange) * nranges);

    return Suspended;
}

int
fs_load_extents(client, pfont, flags, nranges, range,
		nextents, extents)
    pointer     client;
    FontPtr     pfont;
    int         nranges;
    fsRange    *range;
    unsigned long *nextents;
    fsCharInfo **extents;
{
    FSBlockDataPtr blockrec;
    FSBlockedExtentPtr blockedextent;
    FSFpePtr    conn = (FSFpePtr) pfont->fpe->private;

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LOAD_EXTENTS && blockrec->client == client) {
	    blockedextent = (FSBlockedExtentPtr) blockrec->data;
	    if (blockedextent->pfont == pfont && blockedextent->done) {

		/* copy the data */
		*nextents = blockedextent->nextents;
		*extents = blockedextent->extents;

		fs_remove_blockrec(conn, blockrec);
		return Successful;
	    }
	}
	blockrec = blockrec->next;
    }

    /* didn't find waiting record, so send a new one */
    return fs_send_load_extents(client, pfont, flags, nranges, range);
}


/*
 * almost identical to the above, but meant for FS chaining
 */

static int
fs_read_bitmaps(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedBitmapPtr bbitmap = (FSBlockedBitmapPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsQueryXBitmaps8Reply rep;
    fsOffset   *ppbits;
    pointer     pbitmaps;
    int         glyph_size,
                offset_size;

    check_conn(conn);

    /* read reply header */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return AllocError;
    }
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsQueryXBitmaps8Reply) - sizeof(fsReplyHeader));

    assert(rep.type == FS_Reply);

    /* allocate space for glyphs */
    bbitmap->nglyphs = rep.num_chars;
    offset_size = sizeof(fsOffset) * rep.num_chars;
    glyph_size = (rep.length << 2) - sizeof(fsQueryXBitmaps8Reply)
	- offset_size;
    bbitmap->size = glyph_size;
    ppbits = (fsOffset *) xalloc(offset_size);
    pbitmaps = (pointer) xalloc(glyph_size);
    if (!pbitmaps || !ppbits) {
	xfree(pbitmaps);
	xfree(ppbits);
	_fs_drain_bytes_pad(conn, offset_size + glyph_size);
	return AllocError;
    }
    /* read offsets */
    (void) _fs_read_pad(conn, (char *) ppbits, offset_size);

    bbitmap->offsets = ppbits;

    /* read glyphs according to the range */
    (void) _fs_read_pad(conn, (char *) pbitmaps, glyph_size);

    bbitmap->gdata = pbitmaps;

    bbitmap->done = TRUE;
    return Successful;
}

static int
fs_send_load_bitmaps(client, pfont, format, flags, nranges, range)
    pointer     client;
    FontPtr     pfont;
    fsBitmapFormat format;
    int         nranges;
    fsRange    *range;
{
    FSBlockDataPtr blockrec;
    FSBlockedBitmapPtr blockedbitmap;
    int         res;
    fsQueryXBitmaps8Req req;
    FSFontDataPtr fsd = (FSFontDataPtr) (pfont->fpePrivate);
    FontPathElementPtr fpe = fsd->fpe;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    check_conn(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LOAD_BITMAPS);
    if (!blockrec)
	return AllocError;
    blockedbitmap = (FSBlockedBitmapPtr) blockrec->data;
    blockedbitmap->pfont = pfont;
    blockedbitmap->expected_ranges = range;
    blockedbitmap->nranges = nranges;

    /*
     * see if the desired glyphs already exist, and return Successful if they
     * do, otherwise build up character range/character string
     */
    res = fs_check_bitmaps(pfont, format, flags, nranges, range, blockrec);
    if (res == AccessDone)
	return Successful;

    /* send the request */
    req.reqType = FS_QueryXBitmaps8;
    req.fid = ((FSFontDataPtr) pfont->fpePrivate)->fontid;
    req.format = format;
    req.range = TRUE;
    req.length = (sizeof(fsQueryXBitmaps8Req) + sizeof(fsRange) * nranges) >> 2;
    req.num_ranges = nranges;
    _fs_add_req_log(conn, FS_QueryXBitmaps8);
    _fs_write(conn, (char *) &req, sizeof(fsQueryXBitmaps8Req));
    _fs_write(conn, (char *) range, sizeof(fsRange) * nranges);

    return Suspended;
}

int
fs_load_bitmaps(client, pfont, format, flags, nranges, range,
		size, nglyphs, offsets, gdata)
    pointer     client;
    FontPtr     pfont;
    fsBitmapFormat format;
    int         nranges;
    fsRange    *range;
    unsigned long *size;
    unsigned long *nglyphs;
    fsOffset  **offsets;
    pointer    *gdata;
{
    FSBlockDataPtr blockrec;
    FSBlockedBitmapPtr blockedbitmap;
    FSFpePtr    conn = (FSFpePtr) pfont->fpe->private;

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LOAD_BITMAPS && blockrec->client == client) {
	    blockedbitmap = (FSBlockedBitmapPtr) blockrec->data;
	    if (blockedbitmap->pfont == pfont && blockedbitmap->done) {

		/* copy the data */
		*size = blockedbitmap->size;
		*nglyphs = blockedbitmap->nglyphs;
		*offsets = blockedbitmap->offsets;
		*gdata = blockedbitmap->gdata;

		fs_remove_blockrec(conn, blockrec);
		return Successful;
	    }
	}
	blockrec = blockrec->next;
    }

    /* didn't find waiting record, so send a new one */
    return fs_send_load_bitmaps(client, pfont, format, flags, nranges, range);
}

static int
fs_read_list(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedListPtr blist = (FSBlockedListPtr) blockrec->data;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsListFontsReply rep;
    char       *data,
               *dp;
    int         length,
                i;

    assert(blist->done == FALSE);

    check_conn(conn);

    blist->done = TRUE;

    /* read reply header */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	return AllocError;
    }
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsListFontsReply) - sizeof(fsReplyHeader));

    assert(rep.type == FS_Reply);

    length = (rep.length << 2) - sizeof(fsListFontsReply);
    data = (char *) xalloc(length);
    if (!data) {
	_fs_drain_bytes_pad(conn, length);
	return AllocError;
    }
    /* read the list */
    (void) _fs_read_pad(conn, data, length);

    /* copy data into FontPathRecord */
    dp = data;
    for (i = 0; i < rep.nFonts; i++) {
	length = *dp++;
	if (AddFontNamesName(blist->names, dp, length) != Successful) {
	    blist->errcode = AllocError;
	    break;
	}
	dp += length;
    }

    xfree(data);
    return Successful;
}

static int
fs_send_list_fonts(client, fpe, pattern, patlen, maxnames, newnames)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         patlen;
    int         maxnames;
    FontNamesPtr newnames;
{
    FSBlockDataPtr blockrec;
    FSBlockedListPtr blockedlist;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsListFontsReq req;

    check_conn(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LIST_FONTS);
    if (!blockrec)
	return AllocError;
    blockedlist = (FSBlockedListPtr) blockrec->data;
    blockedlist->patlen = patlen;
    blockedlist->errcode = Successful;
    blockedlist->names = newnames;
    blockedlist->done = FALSE;

    /* send the request */
    req.reqType = FS_ListFonts;
    req.maxNames = maxnames;
    req.nbytes = patlen;
    req.length = (sizeof(fsListFontsReq) + patlen + 3) >> 2;
    _fs_add_req_log(conn, FS_ListFonts);
    _fs_write(conn, (char *) &req, sizeof(fsListFontsReq));
    _fs_write_pad(conn, (char *) pattern, patlen);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	char        buf[256];

	bcopy(pattern, buf, MIN(256, patlen));
	buf[MIN(256, patlen)] = '\0';
	printf("Listing fonts on pattern \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    return Suspended;
}

static int
fs_list_fonts(client, fpe, pattern, patlen, maxnames, newnames)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         patlen;
    int         maxnames;
    FontNamesPtr newnames;
{
    FSBlockDataPtr blockrec;
    FSBlockedListPtr blockedlist;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    int         err;

    check_conn(conn);

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LIST_FONTS && blockrec->client == client) {
	    blockedlist = (FSBlockedListPtr) blockrec->data;
	    if (blockedlist->patlen == patlen && blockedlist->done) {
		err = blockedlist->errcode;
		fs_remove_blockrec(conn, blockrec);
		return err;
	    }
	}
	blockrec = blockrec->next;
    }

    /* didn't find waiting record, so send a new one */
    return fs_send_list_fonts(client, fpe, pattern, patlen, maxnames, newnames);
}

static int
fs_read_list_info(fpe, blockrec)
    FontPathElementPtr fpe;
    FSBlockDataPtr blockrec;
{
    FSBlockedListInfoPtr binfo = (FSBlockedListInfoPtr) blockrec->data;
    fsListFontsWithXInfoReply rep;
    FSFpePtr    conn = (FSFpePtr) fpe->private;
    fsPropInfo  pi;
    fsPropOffset *po;
    char       *name;
    pointer     pd;

    check_conn(conn);

    /* clean up anything from the last trip */
    if (binfo->pfi) {
	xfree(binfo->name);
	xfree(binfo->pfi->isStringProp);
	xfree(binfo->pfi->props);
    }
    /* get reply header */
    bcopy((char *) &blockrec->header, (char *) &rep, sizeof(fsReplyHeader));
    if (rep.type == FS_Error) {
/* XXX -- translate FS error */
	_fs_eat_rest_of_error(conn, (fsError *) & rep);
	binfo->errcode = AllocError;
	return AllocError;
    }
    _fs_read(conn, (char *) &rep + sizeof(fsReplyHeader),
	     sizeof(fsListFontsWithXInfoReply) - sizeof(fsReplyHeader));

    assert(rep.type == FS_Reply);

    if (rep.nameLength == 0) {
	binfo->status = FS_LFWI_FINISHED;
	binfo->errcode = BadFontName;
	binfo->name = (char *) 0;
	return Successful;
    }
    /* read the data */
    name = (char *) xalloc(rep.nameLength);
    binfo->pfi = (FontInfoPtr) xalloc(sizeof(FontInfoRec));
    if (!name || !binfo->pfi) {
	xfree(name);
	xfree(binfo->pfi);
	_fs_drain_bytes(conn,
			rep.length - (sizeof(fsListFontsWithXInfoReply) -
				      sizeof(fsReplyHeader)));
	binfo->errcode = AllocError;
	return AllocError;
    }
    (void) _fs_read_pad(conn, name, rep.nameLength);
    _fs_read_pad(conn, (char *) &pi, sizeof(fsPropInfo));
    po = (fsPropOffset *) xalloc(sizeof(fsPropOffset) * pi.num_offsets);
    pd = (pointer) xalloc(pi.data_len);
    if (!po || !pd) {
	xfree(name);
	xfree(po);
	xfree(pd);
	binfo->errcode = AllocError;
	return AllocError;
    }
    (void) _fs_read_pad(conn, (char *) po,
			(pi.num_offsets * sizeof(fsPropOffset)));
    (void) _fs_read_pad(conn, (char *) pd, pi.data_len);
    fs_convert_lfwi_reply(binfo->pfi, &rep, &pi, po, pd);
    xfree(po);
    xfree(pd);
    binfo->name = name;
    binfo->namelen = rep.nameLength;
    binfo->remaining = rep.nReplies;

    binfo->status = FS_LFWI_REPLY;
    binfo->errcode = Suspended;

    return Successful;
}

/* ARGSUSED */
static int
fs_start_list_with_info(client, fpe, pattern, len, maxnames, pdata)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pattern;
    int         len;
    int         maxnames;
    pointer    *pdata;
{
    FSBlockDataPtr blockrec;
    FSBlockedListInfoPtr blockedinfo;
    fsListFontsWithXInfoReq req;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    check_conn(conn);

    /* make a new block record, and add it to the end of the list */
    blockrec = fs_new_block_rec(fpe, client, FS_LIST_WITH_INFO);
    if (!blockrec)
	return AllocError;
    blockedinfo = (FSBlockedListInfoPtr) blockrec->data;
    bzero((char *) blockedinfo, sizeof(FSBlockedListInfoRec));
    blockedinfo->status = FS_LFWI_WAITING;
    blockedinfo->errcode = Suspended;

    /* send the request */
    req.reqType = FS_ListFontsWithXInfo;
    req.maxNames = maxnames;
    req.nbytes = len;
    req.length = (sizeof(fsListFontsWithXInfoReq) + len + 3) >> 2;
    _fs_add_req_log(conn, FS_ListFontsWithXInfo);
    (void) _fs_write(conn, (char *) &req, sizeof(fsListFontsWithXInfoReq));
    (void) _fs_write_pad(conn, pattern, len);

#ifdef NCD
    if (configData.ExtendedFontDiags) {
	char        buf[256];

	bcopy(pattern, buf, MIN(256, len));
	buf[MIN(256, len)] = '\0';
	printf("Listing fonts with info on pattern \"%s\" from font server \"%s\"\n",
	       buf, fpe->name);
    }
#endif

    return Successful;
}

/* ARGSUSED */
static int
fs_next_list_with_info(client, fpe, namep, namelenp, pFontInfo, numFonts,
		       private)
    pointer     client;
    FontPathElementPtr fpe;
    char      **namep;
    int        *namelenp;
    FontInfoPtr *pFontInfo;
    int        *numFonts;
    pointer     private;
{
    FSBlockDataPtr blockrec;
    FSBlockedListInfoPtr blockedinfo;
    FSFpePtr    conn = (FSFpePtr) fpe->private;

    check_conn(conn);

    /* see if the result is already there */
    blockrec = (FSBlockDataPtr) conn->blocked_requests;
    while (blockrec) {
	if (blockrec->type == FS_LIST_WITH_INFO &&
		blockrec->client == client) {
	    blockedinfo = (FSBlockedListInfoPtr) blockrec->data;
	    break;
	}
	blockrec = blockrec->next;
    }

    assert(blockrec);

    if (blockedinfo->status == FS_LFWI_WAITING)
	return Suspended;

    *namep = blockedinfo->name;
    *namelenp = blockedinfo->namelen;
    *pFontInfo = blockedinfo->pfi;
    *numFonts = blockedinfo->remaining;
    if (blockedinfo->status == FS_LFWI_FINISHED) {
	int         err = blockedinfo->errcode;

	fs_remove_blockrec(conn, blockrec);
	return err;
    }
    if (blockedinfo->status == FS_LFWI_REPLY) {
	blockedinfo->status = FS_LFWI_WAITING;
	return Successful;
    } else {
	return blockedinfo->errcode;
    }
}

/*
 * called at server init time
 */

void
fs_register_fpe_functions()
{
    static FontNamesPtr fs_names = (FontNamesPtr) 0;

    if (!fs_names) {
	fs_names = MakeFontNamesRecord(3);
	if (!fs_names)
	    return;
	if (!AddFontNamesName(fs_names, "fs", 2))
	    return;
    }
    fs_font_type = RegisterFPEFunctions(fs_name_check,
					fs_init_fpe,
					fs_free_fpe,
					fs_reset_fpe,
					fs_open_font,
					fs_close_font,
					fs_list_fonts,
					fs_start_list_with_info,
					fs_next_list_with_info,
					fs_wakeup,
					fs_names);

    if (fs_font_type == -1)
	FatalError("Installing font server functions");
}
