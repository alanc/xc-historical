/* $XConsortium: swapreq.c,v 1.6 92/11/18 21:30:11 gildea Exp $ */
/*
 * swapped requests
 */
/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	"misc.h"
#include	"FSproto.h"
#include	"clientstr.h"
#include	"globals.h"

extern int  (*ProcVector[NUM_PROC_VECTORS]) ();

void
SwapLongs(list, count)
    long       *list;
    unsigned long count;
{
    int         n;
    register char *longs = (char *)list;

    while (count >= 8) {
	swapl(longs + 0, n);
	swapl(longs + 4, n);
	swapl(longs + 8, n);
	swapl(longs + 12, n);
	swapl(longs + 16, n);
	swapl(longs + 20, n);
	swapl(longs + 24, n);
	swapl(longs + 28, n);
	longs += 32;
	count -= 8;
    }
    if (count != 0) {
	do {
	    swapl(longs, n);
	    longs += 4;
	} while (--count != 0);
    }
}

/* Byte swap a list of shorts */

void
SwapShorts(list, count)
    short *list;
    register unsigned long count;
{
    register char *shorts = (char *)list;
    register int n;

    while (count >= 16) {
	swaps(shorts + 0, n);
	swaps(shorts + 2, n);
	swaps(shorts + 4, n);
	swaps(shorts + 6, n);
	swaps(shorts + 8, n);
	swaps(shorts + 10, n);
	swaps(shorts + 12, n);
	swaps(shorts + 14, n);
	swaps(shorts + 16, n);
	swaps(shorts + 18, n);
	swaps(shorts + 20, n);
	swaps(shorts + 22, n);
	swaps(shorts + 24, n);
	swaps(shorts + 26, n);
	swaps(shorts + 28, n);
	swaps(shorts + 30, n);
	shorts += 32;
	count -= 16;
    }
    if (count != 0) {
	do {
	    swaps(shorts, n);
	    shorts += 2;
	} while (--count != 0);
    }
}

/*
 * used for all requests that have nothing but 'length' swapped
 */
int
SProcSimpleRequest(client)
    ClientPtr   client;
{
    REQUEST(fsReq);
    stuff->length = lswaps(stuff->length);
    return ((*ProcVector[stuff->reqType]) (client));
}

/*
 * used for all requests that have nothing but 'length' & a resource id swapped
 */
int
SProcResourceRequest(client)
    ClientPtr   client;
{
    REQUEST(fsResourceReq);
    stuff->length = lswaps(stuff->length);
    stuff->id = lswapl(stuff->id);
    return ((*ProcVector[stuff->reqType]) (client));
}

static void
swap_auth(data, num)
    pointer     data;
    int         num;
{
    pointer     p;
    unsigned char t;
    CARD16      namelen,
                datalen;
    int         i;

    p = data;
    for (i = 0; i < num; i++) {
	namelen = *(CARD16 *) p;
	t = p[0];
	p[0] = p[1];
	p[1] = t;
	p += 2;
	datalen = *(CARD16 *) p;
	t = p[0];
	p[0] = p[1];
	p[1] = t;
	p += 2;
	p += (namelen + 3) & ~3;
	p += (datalen + 3) & ~3;
    }
}

int
SProcCreateAC(client)
    ClientPtr   client;
{
    REQUEST(fsCreateACReq);
    stuff->length = lswaps(stuff->length);
    stuff->acid = lswapl(stuff->acid);
    swap_auth((pointer) &stuff[1], stuff->num_auths);
    return ((*ProcVector[stuff->reqType]) (client));
}

int
SProcSetResolution(client)
    ClientPtr   client;
{
    REQUEST(fsSetResolutionReq);
    stuff->length = lswaps(stuff->length);
    stuff->num_resolutions = lswaps(stuff->num_resolutions);
    SwapShorts((short *) &stuff[1], stuff->num_resolutions);

    return ((*ProcVector[stuff->reqType]) (client));
}


int
SProcQueryExtension(client)
    ClientPtr   client;
{
    REQUEST(fsQueryExtensionReq);
    stuff->length = lswaps(stuff->length);
    stuff->nbytes = lswaps(stuff->nbytes);
    return ((*ProcVector[FS_QueryExtension]) (client));
}

int
SProcListCatalogues(client)
    ClientPtr   client;
{
    REQUEST(fsListCataloguesReq);
    stuff->length = lswaps(stuff->length);
    stuff->maxNames = lswapl(stuff->maxNames);
    stuff->nbytes = lswaps(stuff->nbytes);
    return ((*ProcVector[FS_ListCatalogues]) (client));
}

int
SProcListFonts(client)
    ClientPtr   client;
{
    REQUEST(fsListFontsReq);
    stuff->length = lswaps(stuff->length);
    stuff->maxNames = lswapl(stuff->maxNames);
    stuff->nbytes = lswaps(stuff->nbytes);
    return ((*ProcVector[FS_ListFonts]) (client));
}

int
SProcListFontsWithXInfo(client)
    ClientPtr   client;
{
    REQUEST(fsListFontsWithXInfoReq);
    stuff->length = lswaps(stuff->length);
    stuff->maxNames = lswapl(stuff->maxNames);
    stuff->nbytes = lswaps(stuff->nbytes);
    return ((*ProcVector[FS_ListFontsWithXInfo]) (client));
}

int
SProcOpenBitmapFont(client)
    ClientPtr   client;
{
    REQUEST(fsOpenBitmapFontReq);
    stuff->length = lswaps(stuff->length);
    stuff->fid = lswapl(stuff->fid);
    stuff->format_hint = lswapl(stuff->format_hint);
    stuff->format_mask = lswapl(stuff->format_mask);
    return ((*ProcVector[FS_OpenBitmapFont]) (client));
}

int
SProcQueryXExtents(client)
    ClientPtr   client;
{
    REQUEST(fsQueryXExtents8Req); /* 8 and 16 are the same here */
    stuff->length = lswaps(stuff->length);
    stuff->fid = lswapl(stuff->fid);
    stuff->num_ranges = lswapl(stuff->num_ranges);

    return ((*ProcVector[stuff->reqType]) (client));
}

int
SProcQueryXBitmaps(client)
    ClientPtr   client;
{
    REQUEST(fsQueryXBitmaps8Req); /* 8 and 16 are the same here */
    stuff->length = lswaps(stuff->length);
    stuff->fid = lswapl(stuff->fid);
    stuff->format = lswapl(stuff->format);
    stuff->num_ranges = lswapl(stuff->num_ranges);

    return ((*ProcVector[stuff->reqType]) (client));
}

SwapConnClientPrefix(pCCP)
    fsConnClientPrefix *pCCP;
{
    pCCP->major_version = lswaps(pCCP->major_version);
    pCCP->minor_version = lswaps(pCCP->minor_version);
    pCCP->auth_len = lswaps(pCCP->auth_len);
    swap_auth((pointer) &pCCP[1], pCCP->num_auths);
}
