/* $XConsortium: XawI18n.c,v 1.0 94/01/01 00:00:00 kaleb Exp $ */

/* Copyright 1991 NCR Corporation - Dayton, Ohio, USA */
/*
 * Copyright 1990, 1991 by OMRON Corporation, NTT Software Corporation,
 *                      and Nippon Telegraph and Telephone Corporation
 * Copyright 1991 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of OMRON, NTT Software, NTT, and M.I.T.
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission. OMRON, NTT Software,
 * NTT, and M.I.T. make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OMRON, NTT SOFTWARE, NTT, AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL OMRON, NTT SOFTWARE, NTT, OR M.I.T. BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *	Author: Li Yuhong	OMRON Corporation
 *
 */

#include <X11/IntrinsicP.h>
#include "XawI18n.h"

#if NeedFunctionPrototypes
wchar_t atowc(
    unsigned char c)
#else
wchar_t atowc(c)
    unsigned char c;
#endif
{
    wchar_t  wc;
    unsigned char str[2];

    str[0] = c;
    str[1] = '\0';

    mbtowc(&wc, str, 1);
    return wc;
}

#ifdef NCR

int _iswspace(wchar_t w)
{
    int ret = 0;

    wchar_t s = atowc(' ');

    if (s == w)
	ret = 1;

    return ret;
}

#endif