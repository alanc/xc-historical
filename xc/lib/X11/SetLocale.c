/*
 * $XConsortium: XSetLocale.c,v 1.16 91/04/02 15:23:18 rws Exp $
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

XLocale		_Xlocale_ = NULL;	/* global locale */
XLocaleTable   *_xlctbl_ = NULL;	/* locale data base table */

XLocale
_XlcGetCurrentLocale()
{
    return (_Xlocale_);
}

static XLocaleDB *
_XSetLocaleDB(lc_name)
    char	*lc_name;	/* locale name */
{
    XLocaleDB	     *template;
    extern XLocaleDB *_XlcGetLocaleTemplate();
    extern XLocaleDB *_XlcLoadTemplate();

    if (_xlctbl_ == NULL) {
	_xlctbl_ = (XLocaleTable *) Xmalloc(sizeof(XLocaleTable));
	if (!_xlctbl_)
	    return (XLocaleDB *)NULL;
	_xlctbl_->num_loc = 0;
	_xlctbl_->template = NULL;
    }
    /* set current locale from template. */
    if ((template = _XlcGetLocaleTemplate(lc_name)) == NULL) {
	if ((template = _XlcLoadTemplate(lc_name)) == NULL)
	    return(NULL);
    }
    return (template);
}

char *
_XlcSetLocaleModifiers(xlocale, modifiers)
    XLocale     xlocale;
    char	*modifiers;
{
    int		i, len;
    char       *p, *q;

    if (!xlocale) {
	if (!_Xlocale_) {
	    _Xsetlocale(LC_ALL, setlocale(LC_ALL, (char *)NULL));
	    if (!_Xlocale_)
		return (NULL);
	}
	xlocale = _Xlocale_;
    }
    if (modifiers == NULL)
	return (xlocale->lc_modifier);

/* for modifiers */
    len = strlen(modifiers);
    for (i = 0, p = modifiers;  i < len; i++)
	if (*p++ == '@') break;
    xlocale->lc_modifier = Xmalloc((unsigned)strlen(p) + 1);
    if (!xlocale->lc_modifier) {
	return (NULL);
    }
    strcpy(xlocale->lc_modifier, p);

/* for im : @im=xxxx,... */
    len = strlen(p);
    for (i = 0;  i < len; i++, p++) {
	if (strncmp(p, "im=", 3) == NULL) {
	    p += 3;
	    break;
	}
    }
    len = strlen(p);
    for (i = 0, q = p;  i < len; i++)
        if (*q++ == '@' || *q++ == ',') break;

    xlocale->lc_im = Xmalloc((unsigned)i + 1);
    if (!xlocale->lc_im) {
	Xfree(xlocale->lc_modifier);
	return (NULL);
    }
    strncpy(xlocale->lc_im, p, i);
    *(xlocale->lc_im + i) = '\0';

    return (xlocale->lc_modifier);
}

/*ARGSUSED*/
XLocale
_XSetLocale(lc_category, lc_name)
    int		lc_category;	/* locale category */
    char       *lc_name;	/* locale name */
{
    int		i, len;
    char       *p;
    char       *lc_alias;
    char	lang[256];
    XLocale     xlocale;
    char	*_XlcResolveName();

    if (lc_name == NULL) {
	if (_Xlocale_)
	    return (_Xlocale_);
	else
	    lc_name = "C";
    }
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

    xlocale = (XLocale) Xmalloc(sizeof(XLocaleRec));
    if (!xlocale)
	return ((XLocale)NULL);

    xlocale->lc_lang = Xmalloc((unsigned)strlen(lc_name) + 1);
    if (!xlocale->lc_lang) {
	Xfree((char *)xlocale);
	return ((XLocale)NULL);
    }
    strcpy(xlocale->lc_lang, lc_name);

    /* extract locale name */
    lc_alias = _XlcResolveName(lc_name);

    /* set Modifiers */
    if (!_XlcSetLocaleModifiers(xlocale, lc_alias)) {
	Xfree(xlocale->lc_lang);
	Xfree((char *)xlocale);
	return ((XLocale)NULL);
    }

    len = strlen(lc_alias);
    for (i = 0, p = lc_alias;  i < len; i++)
	if (*p++ == '@') break;
    strncpy(lang, lc_alias, i);
    lang[i] = '\0';

    xlocale->xlc_db = _XSetLocaleDB(lang) ;
    if(!xlocale->xlc_db) {
	Xfree(xlocale->lc_im);
	Xfree(xlocale->lc_modifier);
	Xfree(xlocale->lc_lang);
	Xfree((char *)xlocale);
	return ((XLocale)NULL);
    }
    return (xlocale);
}

#if NeedFunctionPrototypes
char *
_Xsetlocale(
    int             lc_category,    /* locale category */
    _Xconst char   *lc_name        /* locale name */
)
#else
char *
_Xsetlocale(lc_category, lc_name)
    int             lc_category;    /* locale category */
    char           *lc_name;        /* locale name */
#endif
{
    XLocale tmp = _Xlocale_;
    
    _Xlocale_ = _XSetLocale(lc_category, lc_name);

    if (_Xlocale_ == NULL)
	return NULL; 

    if(tmp != NULL && tmp != _Xlocale_)
	_XFreeLocale(tmp);

    return _Xlocale_->xlc_db->lc_name;
}

#ifdef X_NOT_STDC_ENV
/* alternative setlocale() for OS does not have */
static char locale_name[MAXLOCALE] = "C";

/*ARGSUSED*/
char *
_XSetLocale(category, name)
    int		category;
    char       *name;
{
    extern char *getenv();

    if (name != NULL) {
	if (*name == NULL) {
	    strcpy(locale_name, getenv("LANG"));
	} else {
	    strcpy(locale_name, name);
	}
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
#ifdef hpux
static char *
_XGetOSLocaleName()
{
    char           *lc_name;
    char           *end;
    int             len;
    static char     new_name[MAXLOCALE];

    lc_name = setlocale(LC_CTYPE, NULL);  /* "/:<locale_name>;/" */
    lc_name = strchr (lc_name, ':');
    lc_name++;
    end = strchr(lc_name, ';');
    len = end - lc_name;
    strncpy(new_name, lc_name, len);
    *(new_name + len) = '\0';
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
