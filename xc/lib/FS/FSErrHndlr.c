/* $XConsortium: FSErrHndlr.c,v 1.4 94/04/17 20:15:11 dpw Exp $ */

/* @(#)FSErrHndlr.c	4.1	91/05/02
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

Copyright (c) 1987, 1994  X Consortium

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
