/*
 * $XConsortium: SetLocale.c,v 1.34 93/09/07 20:06:13 rws Exp $
 */

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
 *	Authors: Li Yuhong		OMRON Corporation
 *		 Tetsuya Kato		NTT Software Corporation
 *		 Hiroshi Kuribayashi	OMRON Corporation
 *   
 */

#include "Xlibint.h"
#include "Xlcint.h"
#include <X11/Xlocale.h>
#include <X11/Xos.h>

#ifdef X_LOCALE

/* alternative setlocale() for when the OS does not provide one */

#ifdef X_NOT_STDC_ENV
extern char *getenv();
#endif

#define MAXLOCALE	64	/* buffer size of locale name */

#if NeedFunctionPrototypes
char *
_Xsetlocale(
    int		  category,
    _Xconst char *name
)
#else
char *
_Xsetlocale(category, name)
    int		category;
    char       *name;
#endif
{
    static char *xsl_name;
    char *old_name;
    XrmMethods methods;
    XPointer state;

    if (category != LC_CTYPE && category != LC_ALL)
	return NULL;
    if (!name) {
	if (xsl_name)
	    return xsl_name;
	return "C";
    }
    if (!*name)
	name = getenv("LC_CTYPE");
    if (!name || !*name)
	name = getenv("LANG");
    if (!name || !*name)
	name = "C";
    old_name = xsl_name;
    xsl_name = (char *)name;
    methods = _XrmInitParseInfo(&state);
    xsl_name = old_name;
    if (!methods)
	return NULL;
    name = (*methods->lcname)(state);
    xsl_name = Xmalloc(strlen(name) + 1);
    if (!xsl_name) {
	xsl_name = old_name;
	(*methods->destroy)(state);
	return NULL;
    }
    strcpy(xsl_name, name);
    if (old_name)
	Xfree(old_name);
    (*methods->destroy)(state);
    return xsl_name;
}

#else /* X_LOCALE */

/*
 * _XlcMapOSLocaleName is an implementation dependent routine that derives
 * the LC_CTYPE locale name as used in the sample implementation from that
 * returned by setlocale.
 */

char *
_XlcMapOSLocaleName(osname, siname)
    char *osname;
    char *siname;
{
#if defined(hpux) || defined(__bsdi__) || defined(sun) || defined(SVR4) || defined(sgi) || defined(__osf__) || defined(AIXV3) || defined(ultrix) || defined(WIN32)
#ifdef hpux
#define SKIPCOUNT 2
#define STARTCHAR ':'
#define ENDCHAR ';'
#else
#ifdef ultrix
#define SKIPCOUNT 2
#define STARTCHAR '\001'
#define ENDCHAR '\001'
#else
#ifdef WIN32
#define SKIPCOUNT 1
#define STARTCHAR '='
#define ENDCHAR ';'
#else
#if defined(__osf__) || defined(AIXV3)
#define STARTCHAR ' '
#define ENDCHAR ' '
#else
#if !defined(sun) || defined(SVR4)
#define STARTCHAR '/'
#endif
#define ENDCHAR '/'
#endif
#endif
#endif
#endif

    char           *start;
    char           *end;
    int             len;
#ifdef SKIPCOUNT
    int		    n;
#endif

    start = osname;
#ifdef SKIPCOUNT
    for (n = SKIPCOUNT;
	 --n >= 0 && start && (start = strchr (start, STARTCHAR));
	 start++)
	;
#endif
#ifdef STARTCHAR
    if (start && (start = strchr (start, STARTCHAR))) {
#endif
	start++;
	if (end = strchr (start, ENDCHAR)) {
	    len = end - start;
	    strncpy(siname, start, len);
	    *(siname + len) = '\0';
	    return siname;
#ifdef STARTCHAR
	}
#endif
    }
#undef STARTCHAR
#undef ENDCHAR
#endif
    return osname;
}

#endif  /* X_LOCALE */
