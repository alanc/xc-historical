/* $XConsortium$ */
/*
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
 */
#ifndef	_FSIO_H_
#define	_FSIO_H_

/* FS specific font FontPathElement data */
typedef struct _fs_fpe_data {
    int         fs_fd;
    int         current_seq;

    int         expected_replies;	/* # of outstanding requests */
/* XXX massive crock to get around stupid #include interferences */
    pointer     blocked_requests;
}           FSFpeRec, *FSFpePtr;

FSFpePtr    _fs_open_server();
void        _fs_bit_clear();

#endif				/* _FSIO_H_ */
