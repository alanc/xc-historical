/*
 * $XConsortium: XSetLocale.c,v 1.8 91/02/14 16:29:34 rws Exp $
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
 *		 Hiroshi Kuribyashi	OMRON Corporation
 *   
 */

#include <stdio.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include "Xlocale.h"

#define MAXLOCALE       64      /* buffer size of locale name */

#define CheckCategory(category) ((category != XLC_ALL &&                \
                                 category != XLC_CODESET &&             \
                                 category != XLC_FONTSET &&             \
                                 category != XLC_INPUTMETHOD)? False: True)

#define LocaleNameOfCategory(locale, category) \
                ((category == XLC_ALL)? locale->lc_name: \
                ((category == XLC_FONTSET)? locale->lc_fs_name: \
                ((category == XLC_CODESET)? locale->lc_cs_name: \
                ((category == XLC_INPUTMETHOD)? locale->lc_im_name: \
                                                NULL))))

static char    *_XlcLocaleFromEnviron();
static char    *_XlcGetDefaultLocaleName();

extern Bool     _XlcInitializeLocale();
extern XLocale *_XlcGetLocaleTemplate();
extern XLocale *_XlcGetCurrentLocale();

XLocale        *_xlocale_ = NULL;

char           *
_Xsetlocale(lc_category, lc_name)
    int             lc_category;    /* locale category */
    char           *lc_name;        /* locale name */
{
    XLocale        *current,
                   *template;

    if ((lc_name == NULL) && (_Xmbtype() < 0))
        /*
         * never setting locale.
         */
        return NULL; 

    if (_XlcInitializeLocale() == False) {
        return NULL;;
    }
    /* allocate temporary locale for retrieval */
    if (CheckCategory(lc_category) == False)
        return (NULL);

    current = _XlcGetCurrentLocale();

    if (lc_name == NULL) {
        /* query the current locale name */
        if (current == NULL) {
            return NULL;
        }
        return LocaleNameOfCategory(current, lc_category);
    }
    /*
     * if lc_name that points null-string ("") are given, we must take locale
     * name from resources or environment.
     */
    if (*lc_name == '\0') {
        lc_name = _XlcGetDefaultLocaleName(lc_category);
    }
    if (*lc_name == '@' && lc_category == XLC_INPUTMETHOD) {
	/*
  	 * set modifiers, current only for XLC_INPUTMETHOD.
         * support the XSetLocaleModifiers("im=value").
         */
	strcpy(current->lc_im, lc_name);
	return(current->lc_im);
    }

    /*
     * set current locale from template.
     */
    if ((template = _XlcGetLocaleTemplate(lc_name)) == NULL) {
        return NULL;
    }
    switch (lc_category) {
    case XLC_ALL:
        /* all categories */
        current->lc_name = template->lc_name;
    case XLC_FONTSET:
        current->lc_fs_name = template->lc_fs_name;
        current->lc_fontset = template->lc_fontset;
        if (lc_category != XLC_ALL)
            break;
    case XLC_CODESET:
        current->lc_cs_name = template->lc_cs_name;
        current->lc_codeset = template->lc_codeset;
        if (lc_category != XLC_ALL)
            break;
    case XLC_INPUTMETHOD:
        /*
         * only lc_im is malloc'ed to store XLC_INPUTMETHOD.
         */
        strcpy(current->lc_im, template->lc_im);
        current->lc_im_name = template->lc_im_name;
    }
    _xlocale_ = current;       /* for OS locale */
    return current->lc_name;
}

static char    *
_XlcGetDefaultLocaleName(category)
    int             category;
{
    char           *locale_name;
    char           *malloc();

    if ((locale_name = _XlcLocaleFromEnviron(category)) != NULL) {
        return locale_name;
    }
    locale_name = malloc(strlen("C") + 1);
    strcpy(locale_name, "C");

    return locale_name;
}

/*
 * _XlcLocaleFromEnviron(): This routine gets locale name from environment
 * variables.
 */
static char    *
_XlcLocaleFromEnviron(category)
    int             category;
{
    char           *s;
    char           *getenv();

    switch (category) {
    case XLC_FONTSET:
        if ((s = getenv("LC_MESSAGE")) != NULL)
            return (s);
    case XLC_CODESET:
    case XLC_INPUTMETHOD:
    case XLC_ALL:
        return (getenv("LANG"));
    default:
        return (NULL);
    }

}

#ifdef DISPLAYSTORELOCALEDATABASE
/*
 * _XlcGetFromResource(): This routine gets the default of locale name from X
 * resource manager XA_RESOURCE_MANAGER property for a specified category.
 * The category is interpreted to X resource name.
 */
static char    *
_XlcLocaleFromResource(dpy, category)
    Display        *dpy;
    int             category;
{
    Window          win = DefaultRootWindow(dpy);
    unsigned char  *prop;
    Atom            actual_type;
    int             actual_format;
    unsigned long   leftover;
    unsigned long   nitems;
    char            progname[MAXLOCALE],
                    catname[MAXLOCALE];

    if (XGetWindowProperty(dpy, win, XA_WM_COMMAND, 0L, (long) BUFSIZ,
                           False, XA_STRING, &actual_type, &actual_format,
                           &nitems, &leftover, &prop) != Success) {
        return NULL;
    }
    /*
     * The property is a liner null-separated list of strings. so we copy it
     * directly with the pointer prop, not (char **)prop[0].
     */
    if (actual_type == XA_STRING && actual_format == 8 && nitems != 0) {
        (void) strcpy(progname, prop);
        Xfree((char *) prop);
    } else {
        (void) strcpy(progname, "*");
    }

    CategoryName(category, catname);
    return (XGetDefault(dpy, progname, catname));
}

/*
 * CategoryName(): This routine interpretes a category to its name, and
 * returns the order number of this category.
 */
static int
CategoryName(category, catname)
    int             category;
    char           *catname;
{
    switch (category) {
    case XLC_CODESET:
        (void) strcpy(catname, "codeset");
        return (1);
    case XLC_FONTSET:
        (void) strcpy(catname, "fontset");
        return (2);
    case XLC_INPUTMETHOD:
        (void) strcpy(catname, "inputmethod");
        return (3);
    case XLC_ALL:
        (void) strcpy(catname, "language");
        return (4);
    default:
        (void) strcpy(catname, "");
        return (-1);
    }
}
#endif  /* DISPLAYSTORELOCALEDATABASE */
