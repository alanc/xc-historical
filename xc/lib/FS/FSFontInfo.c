/* $XConsortium: FSFontInfo.c,v 1.6 93/09/29 16:47:53 rws Exp $ */
/*
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software 
 * and its documentation for any purpose is hereby granted without fee, 
 * provided that the above copyright notice appear in all copies and 
 * that both that copyright notice and this permission notice appear 
 * in supporting documentation, and that the names of Network Computing 
 * Devices or Digital not be used in advertising or publicity pertaining 
 * to distribution of the software without specific, written prior 
 * permission. Network Computing Devices or Digital make no representations 
 * about the suitability of this software for any purpose.  It is provided 
 * "as is" without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND  DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES 
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS 
 * SOFTWARE.
 */

/*

Copyright (c) 1987  X Consortium

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

*/
#include	"FSlibint.h"

char      **
FSListFontsWithXInfo(svr, pattern, maxNames, count, info, pprops, offsets, prop_data)
    FSServer   *svr;
    char       *pattern;
    int         maxNames;
    int        *count;
    FSXFontInfoHeader ***info;
    FSPropInfo ***pprops;
    FSPropOffset ***offsets;
    unsigned char ***prop_data;
{
    long        nbytes;
    int         i,
                j;
    int         size = 0;
    FSXFontInfoHeader **fhdr = (FSXFontInfoHeader **) 0;
    FSPropInfo **pi = (FSPropInfo **) 0;
    FSPropOffset **po = (FSPropOffset **) 0;
    unsigned char **pd = (unsigned char **) 0;
    char      **flist = NULL;
    fsListFontsWithXInfoReply reply;
    fsListFontsWithXInfoReq *req;
    fsPropInfo local_pi;
    fsPropOffset local_po;
    Status status;

    GetReq(ListFontsWithXInfo, req);
    req->maxNames = maxNames;
    nbytes = req->nbytes = pattern ? strlen(pattern) : 0;
    req->length += (nbytes + 3) >> 2;
    _FSSend(svr, pattern, nbytes);

    for (i = 0;; i++) {
	if (FSProtocolVersion(svr) > 1)
	{
	    status = _FSReply(svr, (fsReply *) &reply, 0, fsFalse);
	    if (status != 0  &&  reply.nameLength == 0)	/* got last reply */
		break;
	    if (status)
		_FSRead(svr, ((char *) &reply) + SIZEOF(fsGenericReply),
			SIZEOF(fsListFontsWithXInfoReply) -
			SIZEOF(fsGenericReply));
	} else {
	    status = _FSReply(svr, (fsReply *) & reply,
			      ((SIZEOF(fsListFontsWithXInfoReply) -
				SIZEOF(fsGenericReply)) >> 2), fsFalse);
	}
	if (!status) {
	    for (j = (i - 1); j >= 0; j--) {
		FSfree((char *) fhdr[j]);
		FSfree((char *) pi[j]);
		FSfree((char *) po[j]);
		FSfree((char *) pd[j]);
		FSfree(flist[j]);
	    }
	    if (flist)
		FSfree((char *) flist);
	    if (fhdr)
		FSfree((char *) fhdr);
	    if (pi)
		FSfree((char *) pi);
	    if (po)
		FSfree((char *) po);
	    if (pd)
		FSfree((char *) pd);

	    SyncHandle();
	    return (char **) NULL;
	}
	if (reply.nameLength == 0)	/* got last reply in version 1 */
	    break;
	if ((i + reply.nReplies) >= size) {
	    size = i + reply.nReplies + 1;

	    if (fhdr) {
		FSXFontInfoHeader **tmp_fhdr = (FSXFontInfoHeader **)
		FSrealloc((char *) fhdr,
			  (unsigned) (sizeof(FSXFontInfoHeader *) * size));
		char      **tmp_flist = (char **) FSrealloc((char *) flist,
					 (unsigned) (sizeof(char *) * size));
		FSPropInfo **tmp_pi = (FSPropInfo **)
		FSrealloc((char *) pi,
			  (unsigned) (sizeof(FSPropInfo *) * size));
		FSPropOffset **tmp_po = (FSPropOffset **)
		FSrealloc((char *) po,
			  (unsigned) (sizeof(FSPropOffset *) * size));
		unsigned char **tmp_pd = (unsigned char **)
		FSrealloc((char *) pd,
			  (unsigned) (sizeof(unsigned char *) * size));

		if (!tmp_fhdr || !tmp_flist || !tmp_pi || !tmp_po || !tmp_pd) {
		    for (j = (i - 1); j >= 0; j--) {
			FSfree((char *) flist[j]);
			FSfree((char *) fhdr[j]);
			FSfree((char *) pi[j]);
			FSfree((char *) po[j]);
			FSfree((char *) pd[j]);
		    }
		    if (tmp_flist)
			FSfree((char *) tmp_flist);
		    else
			FSfree((char *) flist);
		    if (tmp_fhdr)
			FSfree((char *) tmp_fhdr);
		    else
			FSfree((char *) fhdr);
		    if (tmp_pi)
			FSfree((char *) tmp_pi);
		    else
			FSfree((char *) pi);
		    if (tmp_po)
			FSfree((char *) tmp_po);
		    else
			FSfree((char *) po);
		    if (tmp_pd)
			FSfree((char *) tmp_pd);
		    else
			FSfree((char *) pd);
		    goto clearwire;
		}
		fhdr = tmp_fhdr;
		flist = tmp_flist;
		pi = tmp_pi;
		po = tmp_po;
		pd = tmp_pd;
	    } else {
		if (!(fhdr = (FSXFontInfoHeader **)
		      FSmalloc((unsigned) (sizeof(FSXFontInfoHeader *) * size))))
		    goto clearwire;
		if (!(flist = (char **)
		      FSmalloc((unsigned) (sizeof(char *) * size)))) {
		    FSfree((char *) fhdr);
		    goto clearwire;
		}
		if (!(pi = (FSPropInfo **)
		      FSmalloc((unsigned) (sizeof(FSPropInfo *) * size)))) {
		    FSfree((char *) fhdr);
		    FSfree((char *) flist);
		    goto clearwire;
		}
		if (!(po = (FSPropOffset **)
		      FSmalloc((unsigned) (sizeof(FSPropOffset *) * size)))) {
		    FSfree((char *) fhdr);
		    FSfree((char *) flist);
		    FSfree((char *) pi);
		    goto clearwire;
		}
		if (!(pd = (unsigned char **)
		    FSmalloc((unsigned) (sizeof(unsigned char *) * size)))) {
		    FSfree((char *) fhdr);
		    FSfree((char *) flist);
		    FSfree((char *) pi);
		    FSfree((char *) po);
		    goto clearwire;
		}
	    }
	}
	fhdr[i] = (FSXFontInfoHeader *) FSmalloc(sizeof(FSXFontInfoHeader));
	if (!fhdr[i]) {
	    goto badmem;
	}
	FSUnpack_XFontInfoHeader(&reply, fhdr[i], FSProtocolVersion(svr));

	/* alloc space for the name */
	flist[i] = (char *) FSmalloc((unsigned int) (reply.nameLength + 1));
	if (FSProtocolVersion(svr) == 1)
	{
	    /* get the name */
	    if (!flist[i]) {
		nbytes = reply.nameLength + 3 & ~3;
		_FSEatData(svr, (unsigned long) nbytes);
		goto badmem;
	    }
	    _FSReadPad(svr, flist[i], (long) reply.nameLength);
	    flist[i][reply.nameLength] = '\0';
	}

	pi[i] = (FSPropInfo *) FSmalloc(sizeof(FSPropInfo));
	if (!pi[i]) {
	    FSfree((char *) fhdr[i]);
	    goto badmem;
	}
	_FSReadPad(svr, (char *) &local_pi, SIZEOF(fsPropInfo));
	pi[i]->num_offsets = local_pi.num_offsets;
	pi[i]->data_len = local_pi.data_len;

	po[i] = (FSPropOffset *)
	    FSmalloc(pi[i]->num_offsets * sizeof(FSPropOffset));
	if (!po[i]) {
	    FSfree((char *) fhdr[i]);
	    FSfree((char *) pi[i]);
	    goto badmem;
	}
	pd[i] = (unsigned char *) FSmalloc(pi[i]->data_len);
	if (!pd[i]) {
	    FSfree((char *) fhdr[i]);
	    FSfree((char *) pi[i]);
	    FSfree((char *) po[i]);
	    goto badmem;
	}
	/* get offsets */
	for (j=0; j<pi[i]->num_offsets; j++)
	{
	    _FSReadPad(svr, (char *) &local_po, SIZEOF(fsPropOffset));
	    po[i][j].name.position = local_po.name.position;
	    po[i][j].name.length = local_po.name.length;
	    po[i][j].value.position = local_po.value.position;
	    po[i][j].value.length = local_po.value.length;
	    po[i][j].type = local_po.type;
	}
	
	/* get prop data */
	if (FSProtocolVersion(svr) == 1)
	    _FSReadPad(svr, (char *) pd[i], pi[i]->data_len);
	else
	    _FSRead(svr, (char *) pd[i], pi[i]->data_len);

	if (FSProtocolVersion(svr) != 1)
	{
	    /* get the name */
	    if (!flist[i]) {
		nbytes = reply.nameLength + 3 & ~3;
		_FSEatData(svr, (unsigned long) nbytes);
		goto badmem;
	    }
	    _FSRead(svr, flist[i], (long) reply.nameLength);
	    flist[i][reply.nameLength] = '\0';

	    nbytes = pi[i]->data_len + reply.nameLength;
	    _FSEatData(svr, (unsigned long) (((nbytes+3)&~3) - nbytes));
	}
    }
    *info = fhdr;
    *count = i;
    *pprops = pi;
    *offsets = po;
    *prop_data = pd;
    SyncHandle();
    return flist;

badmem:
    for (j = (i - 1); j >= 0; j--) {
	FSfree((char *) pi[j]);
	FSfree((char *) po[j]);
	FSfree((char *) pd[j]);
	FSfree(flist[j]);
	FSfree((char *) fhdr[j]);
    }
    if (flist)
	FSfree((char *) flist);
    if (fhdr)
	FSfree((char *) fhdr);
    if (pi)
	FSfree((char *) pi);
    if (po)
	FSfree((char *) po);
    if (pd)
	FSfree((char *) pd);


clearwire:
    do {
	fsPropInfo  ti;

	_FSEatData(svr, (reply.nameLength + 3) & ~3);
	_FSReadPad(svr, (char *) &ti, SIZEOF(fsPropInfo));
	_FSEatData(svr, (SIZEOF(fsPropOffset) * ti.num_offsets));
	_FSEatData(svr, ti.data_len);
    } while (_FSReply(svr, (fsReply *) & reply,
		      ((SIZEOF(fsListFontsWithXInfoReply)
       - SIZEOF(fsGenericReply)) >> 2), fsFalse) && (reply.nameLength != 0));
    SyncHandle();
    return (char **) NULL;
}
