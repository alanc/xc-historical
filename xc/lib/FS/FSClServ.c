/* $XConsortium: FSClServ.c,v 1.3 94/01/31 11:31:34 mor Exp $ */

/* @(#)FSClServ.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of M.I.T., Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include	"FSlibint.h"

extern FSServer *_FSHeadOfServerList;

FSCloseServer(svr)
    FSServer     *svr;
{
    _FSExtension *ext;
    FSServer    **sv = &_FSHeadOfServerList;
    FSServer     *s = _FSHeadOfServerList;
    extern void _FSFreeQ();

    svr->flags |= FSlibServerClosing;
    FSSync(svr, 1);		/* throw out pending events */
    ext = svr->ext_procs;
    while (ext) {
	if (ext->close_server != NULL)
	    (*ext->close_server) (svr, &ext->codes);
	ext = ext->next;
    }
    _FSDisconnectServer(svr->trans_conn);
    while (s != NULL) {
	if (s == svr) {
	    *sv = s->next;
	    _FSFreeServerStructure(svr);
	    break;
	}
	sv = &(s->next);
	s = *sv;
    }
    if (_FSHeadOfServerList == NULL) {
	_FSFreeQ();
    }
}
