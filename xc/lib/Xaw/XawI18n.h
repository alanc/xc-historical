/* $XConsortium: XawI18n.h,v 1.6 94/03/30 21:28:47 kaleb Exp $ */

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

/* Everyone does this differently. POSIX and Standard C don't have
 * nearly enough to be useful. Does ISO 10646 specify header files
 * and their contents? There's got to be a better way!
 */

#ifdef sgi
/* IRIX 5.x close enough for Xaw */
#define SVR4
#endif

#ifdef SVR4

#ifndef NO_WCHAR
#include <wctype.h>
#include <widec.h>
#define wcslen(c) wslen(c)
#define wcscpy(d,s) wscpy(d,s)
#define wcsncpy(d,s,l) wsncpy(d,s,l)
#else
#ifdef NCR
#define iswspace(c) _iswspace(c)
extern int _iswspace(wchar_t);
#define USE_XWCHAR_STRING
#endif
#endif

#ifdef sun
#define HAS_ISW_FUNCS
#endif

#else /* SVR4 */

#ifdef luna
#ifdef MACH
#define HAS_WCHAR_H
#endif
#endif

#ifdef hpux
#define HAS_WCHAR_H
#endif

#ifdef __osf__
#ifdef __WCHAR_T_LEN
#define HAS_WCHAR_H
#endif
#endif

#ifdef WIN32
#define HAS_WCHAR_H
#endif

#ifdef HAS_WCHAR_H
#include <wchar.h>
#endif

#ifdef luna
#ifdef MACH
#define HAS_ISW_FUNCS
#endif
#endif

#ifdef hpux
#define HAS_ISW_FUNCS
#endif

#endif /* !SVR4 */

/* now deal with the exceptions */

#ifdef ultrix
#define USE_XWCHAR_STRING
#endif

#ifdef sony
#ifndef SVR4
#include <jctype.h>
#define iswspace(c) jisspace(c)
#define USE_XWCHAR_STRING
#endif
#endif

#ifdef sun
#ifndef SVR4
#define USE_XWCHAR_STRING
#endif
#endif

#ifdef macII
#define USE_XWCHAR_STRING
#endif

#ifdef __bsdi__
#define USE_XWCHAR_STRING
#endif

#ifdef CRAY
#define USE_XWCHAR_STRING
#endif

#ifdef __osf__
#ifndef __WCHAR_T_LEN
#define USE_XWCHAR_STRING
#endif
#endif

#ifdef AMOEBA
#define USE_XWCHAR_STRING
#endif

#ifdef _MINIX
#define USE_XWCHAR_STRING
#endif

#ifdef __FreeBSD__
#define USE_XWCHAR_STRING
#endif

#ifdef __NetBSD__
#define USE_XWCHAR_STRING
#endif

#ifdef __linux__
#define USE_XWCHAR_STRING
#endif

#ifdef USE_XWCHAR_STRING
#define wcslen(c) _Xwcslen(c)
#define wcscpy(d,s) _Xwcscpy(d,s)
#define wcsncpy(d,s,l) _Xwcsncpy(d,s,l)
#ifdef macII
#define mbtowc(wc,s,l) _Xmbtowc(wc,s,l)
#endif
#endif

#ifdef AIXV3
#include <ctype.h>
#endif

extern wchar_t _Xaw_atowc (
#if NeedFunctionPrototypes
    unsigned char	c
#endif
);

/* 
 * At this point the only place these are undefined is on, e.g. BSD (like 
 * SunOS 4, Ultrix, BSDI, older Sony, and old MIPS OSF/1, whose locale 
 * support only handles C locale anyway.
 */
#ifndef HAS_ISW_FUNCS
#include <ctype.h>
#ifndef iswspace
#define iswspace(c) (isascii(c) && isspace(c))
#endif
#endif
