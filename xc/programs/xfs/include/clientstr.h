/* $XConsortium: clientstr.h,v 1.5 93/08/24 18:49:42 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
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

#ifndef _CLIENTSTR_H_
#define	_CLIENTSTR_H_

#include	"FS.h"
#include	"client.h"
#include	"auth.h"
#include	"misc.h"

typedef struct _Client {
    int         index;
    pointer     osPrivate;
    int         noClientException;
    int         (**requestVector) ();
    pointer     requestBuffer;
    int         clientGone;
    int         sequence;
    Bool        swapped;
    long        last_request_time;
    void        (*pSwapReplyFunc) ();
    AuthContextPtr auth;
    int		auth_generation;
    AuthContextPtr default_auth;
    char       *catalogues;
    int         num_catalogues;
    Mask        eventmask;
    fsResolution *resolutions;
    int         num_resolutions;
    int		major_version;	/* client-major-protocol-version */
    int		minor_version;
}           ClientRec;

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) ();
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;


extern void CloseDownClient();

#endif				/* _CLIENTSTR_H_ */
