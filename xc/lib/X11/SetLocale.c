/*
 * $XConsortium: XSetLocale.c,v 1.22 91/04/03 10:39:55 rws Exp $
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

#include <X11/Xos.h>
#include "Xlibint.h"
#include "Xlocaleint.h"

#define MAXLOCALE	64	/* buffer size of locale name */

XLocale		_Xlocale_ = (XLocale)0;		/* global locale */
XLocaleTable   *_Xlctbl_ = (XLocaleTable *)0;	/* locale data base table */
#ifndef lint
static int lock;
static int lock_tbl;
#endif

#define XREALLOC(p, size)	((p) ? Xrealloc(p, size) : Xmalloc(size))

static XLocaleDB *
_XSetLocaleDB(lc_name)
    char	*lc_name;	/* locale name */
{
    XLocaleDB	     *template;
    extern XLocaleDB *_XlcGetLocaleTemplate();
    extern XLocaleDB *_XlcLoadTemplate();

    LockMutex(&lock_tbl);

    if (_Xlctbl_ == (XLocaleTable *)0) {
	_Xlctbl_ = (XLocaleTable *) Xmalloc(sizeof(XLocaleTable));
	if (!_Xlctbl_) {
	    UnlockMutex(&lock_tbl);
	    return (XLocaleDB *)0;
	}
	_Xlctbl_->num_loc = 0;
	_Xlctbl_->template = (XLocaleList *)0;
    }
    /* set current locale from template. */
    if ((template = _XlcGetLocaleTemplate(lc_name)) == (XLocaleDB *)0) {
	if ((template = _XlcLoadTemplate(lc_name)) == (XLocaleDB *)0) {
	    UnlockMutex(&lock_tbl);
	    return((XLocaleDB *)0);
	}
    }
    UnlockMutex(&lock_tbl);
    return (template);
}

#ifndef X_NOT_STDC_ENV
static char *_XGetOSLocaleName();
#endif

/*ARGSUSED*/
static Bool
_XChangeLocale(xlocale, lc_category, lc_name)
    XLocale	xlocale;
    int		lc_category;	/* locale category */ /* not used */
    char       *lc_name;	/* locale name */
{
    int		i, len;
    char       *p;
    char       *lc_alias;
    char	lang[256];
    char	*_XlcResolveName();

    if (lc_name == NULL && xlocale)
	return (True);

    if (!xlocale)
	return (False);

    /*
     * if lc_name that points null-string ("") are given, we must take locale
     * name from environment.
     */
    if (*lc_name == '\0') {
#ifndef X_NOT_STDC_ENV
	lc_name = _XGetOSLocaleName();
#else
	lc_name = setlocale(LC_CTYPE, (char *)NULL);
#endif
    }

    if (! (xlocale->lc_lang = XREALLOC(xlocale->lc_lang,
				      (unsigned)strlen(lc_name) + 1)))
	return (False);
    strcpy(xlocale->lc_lang, lc_name);

    /* extract locale name */
    lc_alias = _XlcResolveName(lc_name);

    /* set Modifiers */
    if (!_XlcSetLocaleModifiers(xlocale, lc_alias))
	return (False);

    len = strlen(lc_alias);
    for (i = 0, p = lc_alias;  i < len; i++)
	if (*p++ == '@') break;
    strncpy(lang, lc_alias, i);
    lang[i] = '\0';

    if (! (xlocale->xlc_db = _XSetLocaleDB(lang)))
	return (False);

    return (True);
}

/*ARGSUSED*/
XLocale
_XSetLocale(lc_category, lc_name)
    int		lc_category;	/* locale category */ /* not used */
    char       *lc_name;	/* locale name */
{
    XLocale     xlocale;

    if (lc_name == NULL) {
	if (_Xlocale_)
	    return (_Xlocale_);
	else
	    lc_name = "C"; /* Setting gloval locale with "C" */
    }

    if (! (xlocale = (XLocale) Xmalloc(sizeof(XLocaleRec))))
	return ((XLocale)0);
    xlocale->lc_lang = NULL;
    if (! _XChangeLocale(xlocale, lc_category, lc_name)) {
	_XlcFreeLocale(xlocale);
	return ((XLocale)0);
    }
    return (xlocale);
}

char *
_Xsetlocale(lc_category, lc_name)
    int             lc_category;    /* locale category */
    char           *lc_name;        /* locale name */
{
    LockMutex(&lock);

    if (_Xlocale_) {
	char save[256];
	strcpy(save, setlocale(lc_category, NULL)); /* save */
	if (!_XChangeLocale(_Xlocale_, lc_category, lc_name))
	    if (!_XChangeLocale(_Xlocale_, lc_category, save))
		_XChangeLocale(_Xlocale_, lc_category, "C");
    } else {
	if (!(_Xlocale_ = _XSetLocale(lc_category, lc_name)))
	    _Xlocale_ = _XSetLocale(lc_category, "C");
    }
    UnlockMutex(&lock);

    if (!_Xlocale_)
	return NULL; 

    return _Xlocale_->xlc_db->lc_name;
}

#ifdef X_NOT_STDC_ENV
/* alternative setlocale() for OS does not have */
static char locale_name[MAXLOCALE] = "C";
#ifndef lint
static int lock_name;
#endif

/*ARGSUSED*/
#if NeedFunctionPrototypes
char *
_X_setlocale(
    int		  category,
    _Xconst char *name
)
#else
char *
_X_setlocale(category, name)
    int		category;
    char       *name;
#endif
{
    extern char *getenv();
    char	*lang;

    if (name != NULL) {
	LockMutex(&lock_name);
	if (*name == '\0') {
	    lang = getenv("LANG");
	    if (*lang)
		strcpy(locale_name, lang);
	} else {
	    strcpy(locale_name, name);
	}
	UnlockMutex(&lock_name);
	if (!_Xsetlocale(LC_CTYPE, locale_name))
	    return (NULL);
    }
    return locale_name;
}
#else
/*
 * _XGetOSLocaleName is an implementation dependent routine that
 * derives the locale name as used in the sample implementation from
 * that returned by setlocale  for example HP would use the following:
 */
#ifndef lint
static int lock_name;
#endif

#ifdef hpux
static char *
_XGetOSLocaleName()
{
    char           *lc_name;
    char           *end;
    int             len;
    static char     new_name[MAXLOCALE];

    LockMutex(&lock_name);

    lc_name = setlocale(LC_CTYPE, NULL);  /* "/:<locale_name>;/" */
    lc_name = strchr (lc_name, ':');
    lc_name++;
    end = strchr(lc_name, ';');
    len = end - lc_name;
    strncpy(new_name, lc_name, len);
    *(new_name + len) = '\0';
    UnlockMutex(&lock_name);
    return new_name;
}
#else
static char *
_XGetOSLocaleName()
{
    return setlocale(LC_CTYPE, NULL);
}
#endif
#endif  /* X_NOT_STDC_ENV */
