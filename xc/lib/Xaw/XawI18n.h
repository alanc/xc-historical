/* $XConsortium: XawI18n.h,v 1.0 94/01/01 00:00:00 kaleb Exp $ */

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

/* it'd be nice if we could agree that IRIX 5.x is SVR4. */
#if defined(SVR4) || defined(sgi)
#ifndef NCR
#include <wctype.h>
#include <widec.h>
#define wcslen(c) wslen(c)
#define wcscpy(d,s) wscpy(d,s)
#define wcsncpy(d,s,l) wsncpy(d,s,l)
#else
/* this all goes away when NCR installs their widechar package */
#include <libw.h>
#define iswprint(c) wisprint(c)
#define iswspace(c) _iswspace(c)
extern int _iswspace(wchar_t);
#define wcslen(c) _Xwcslen(c)
#define wcscpy(d,s) _Xwcscpy(d,s)
#define wcsncpy(d,s,l) _Xwcsncpy(d,s,l)
#endif
#endif

#if (defined(luna) && defined(MACH)) || defined(hpux) || (defined(__osf__) && defined(__WCHAR_T_LEN)) || defined(WIN32)
#include <wchar.h>
#endif

/* now deal with the exceptions */

#if defined(sony) && !defined(SVR4)
#include <jctype.h>
#define iswspace(c) jisspace(c)
#define iswprint(c) jisalpha(c) || jisnumeric(c) || jiskigou(c) || jisspace(c)
#endif

#if defined(__osf__) && !defined(__WCHAR_T_LEN)
/* OSF/1 1.x on DECstation 3100 */
#include <jctype.h>
#define iswspace(c) isjspace(c)
#define iswprint(c) isjalphanum(c) || isjpunct(c) || isjspace(c)
#endif

#if defined(ultrix) || (defined(sun) && !defined(SVR4)) || defined(bsdi)
#define wcslen(c) _Xwcslen(c)
#define wcscpy(d,s) _Xwcscpy(d,s)
#define wcsncpy(d,s,l) _Xwcsncpy(d,s,l)
#endif

extern wchar_t atowc (
#if NeedFunctionPrototypes
    unsigned char	c
#endif
);


/* 
 * At this point the only place these are undefined is on, e.g. 
 * SunOS 4.x and Ultrix, whose locale support only includes LC_CTYPE 
 * anyway.
 */

#include <ctype.h>

#ifndef iswprint
#define iswprint(c) (isascii(c) && isprint(c))
#endif

#ifndef iswspace
#define iswspace(c) (isascii(c) && isspace(c))
#endif

