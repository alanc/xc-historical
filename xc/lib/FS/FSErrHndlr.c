/* $XConsortium: FSErrHndlr.c,v 1.2 91/05/13 15:11:32 gildea Exp $ */

/* @(#)FSErrHndlr.c	4.1	91/05/02
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

extern int  _FSDefaultError();
extern int  _FSDefaultIOError();

int         (*
	     FSSetErrorHandler(handler)) ()
    int         (*handler) ();
{
    int         (*oldhandler) () = _FSErrorFunction;

    if (handler != NULL) {
	_FSErrorFunction = handler;
    } else {
	_FSErrorFunction = _FSDefaultError;
    }
    return oldhandler;
}

extern int  _FSIOError();

int         (*
	     FSSetIOErrorHandler(handler)) ()
    int         (*handler) ();
{
    int         (*oldhandler) () = _FSIOErrorFunction;

    if (handler != NULL) {
	_FSIOErrorFunction = handler;
    } else {
	_FSIOErrorFunction = _FSDefaultIOError;
    }
    return oldhandler;
}
