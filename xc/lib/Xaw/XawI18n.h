/* $XConsortium: XawI18n.h,v 1.7 94/04/01 10:17:17 rws Exp $ */

/************************************************************
Copyright 1993 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifdef HAS_WCTYPE_H
#include <wctype.h>
#include <widec.h>
#define wcslen(c) wslen(c)
#define wcscpy(d,s) wscpy(d,s)
#define wcsncpy(d,s,l) wsncpy(d,s,l)
#endif

#ifdef HAS_WCHAR_H
#include <wchar.h>
#endif

#ifdef AIXV3
#include <ctype.h>
#endif

#ifdef NCR
#define iswspace(c) _Xaw_iswspace(c)
extern int _Xaw_iswspace(wchar_t);
#endif

#ifdef sony
#ifndef SVR4
#include <jctype.h>
#define iswspace(c) jisspace(c)
#endif
#endif

#ifdef USE_XWCHAR_STRING
#define wcslen(c) _Xwcslen(c)
#define wcscpy(d,s) _Xwcscpy(d,s)
#define wcsncpy(d,s,l) _Xwcsncpy(d,s,l)
#ifdef macII
#define mbtowc(wc,s,l) _Xmbtowc(wc,s,l)
#endif
#endif

extern wchar_t _Xaw_atowc (
#if NeedFunctionPrototypes
    unsigned char	c
#endif
);

#ifndef HAS_ISW_FUNCS
#include <ctype.h>
#ifndef iswspace
#define iswspace(c) (isascii(c) && isspace(c))
#endif
#endif
