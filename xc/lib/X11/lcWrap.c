/*
 * $XConsortium: lcWrap.c,v 11.12 93/09/18 10:28:08 rws Exp $
 */

/*
 * Copyright 1991 by the Massachusetts Institute of Technology
 * Copyright 1991 by the Open Software Foundation
 * Copyright 1993 by the TOSHIBA Corp.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Open Software Foundation and M.I.T.
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Open Software
 * Foundation and M.I.T. make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OPEN SOFTWARE FOUNDATION AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OPEN SOFTWARE FOUNDATIONN OR M.I.T. BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *		 M. Collins		OSF  
 *
 *		 Katsuhisa Yano		TOSHIBA Corp.
 */				

#include "Xlibint.h"
#include "Xlcint.h"
#include <X11/Xlocale.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

#ifdef X_NOT_STDC_ENV
extern char *getenv();
#endif

extern void _XlcInitLoader(
#if NeedFunctionPrototypes
    void
#endif
);

#if NeedFunctionPrototypes
char *
XSetLocaleModifiers(
    _Xconst char   *modifiers
)
#else
char *
XSetLocaleModifiers(modifiers)
    char        *modifiers;
#endif
{
    XLCd lcd = _XlcCurrentLC();
    char *user_mods;

    if (!lcd)
	return (char *) NULL;
    if (!modifiers)
	return lcd->core->modifiers;
    user_mods = getenv("XMODIFIERS");
    modifiers = (*lcd->methods->map_modifiers) (lcd,
						user_mods, (char *)modifiers);
    if (modifiers)
	lcd->core->modifiers = (char *)modifiers;
    return (char *)modifiers;
}

Bool
XSupportsLocale()
{
    return _XlcCurrentLC() != (XLCd)NULL;
}

Bool _XlcValidModSyntax(mods, valid_mods)
    char *mods;
    char **valid_mods;
{
    int i;
    char **ptr;

    while (mods && (*mods == '@')) {
	mods++;
	if (*mods == '@')
	    break;
	for (ptr = valid_mods; *ptr; ptr++) {
	    i = strlen(*ptr);
	    if (strncmp(mods, *ptr, i) || (mods[i] != '='))
		continue;
	    mods = strchr(mods+i+1, '@');
	    break;
	}
    }
    return !mods || !*mods;
}

static Const char *im_valid[] = {"im", (char *)NULL};

/*ARGSUSED*/
char *
_XlcDefaultMapModifiers (lcd, user_mods, prog_mods)
    XLCd lcd;
    char *user_mods;
    char *prog_mods;
{
    int i;
    char *mods;

    if (!_XlcValidModSyntax(prog_mods, (char **)im_valid))
	return (char *)NULL;
    if (!_XlcValidModSyntax(user_mods, (char **)im_valid))
	return (char *)NULL;
    i = strlen(prog_mods) + 1;
    if (user_mods)
	i += strlen(user_mods);
    mods = Xmalloc(i);
    if (mods) {
	strcpy(mods, prog_mods);
	if (user_mods)
	    strcat(mods, user_mods);
    }
    return mods;
}

#ifndef	lint
static lock;
#endif /* lint */

static XLCd *lcd_list;

typedef struct _XlcLoaderListRec {
    struct _XlcLoaderListRec *next;
    XLCdLoadProc proc;
} XlcLoaderListRec, *XlcLoaderList;

static XlcLoaderList loader_list = NULL;

void
_XlcRemoveLoader(proc)
    XLCdLoadProc proc;
{
    XlcLoaderList loader, prev;

    if (loader_list == NULL)
	return;

    prev = loader = loader_list;
    if (loader->proc == proc) {
	loader_list = loader->next;
	Xfree(loader);
	return;
    }

    while (loader = loader->next) {
	if (loader->proc == proc) {
	    prev->next = loader->next;
	    Xfree(loader);
	    return;
	}
	prev = loader;
    }

    return;
}

Bool
_XlcAddLoader(proc, position)
    XLCdLoadProc proc;
    XlcPosition position;
{
    XlcLoaderList loader, last;

    _XlcRemoveLoader(proc);		/* remove old loader, if exist */

    loader = (XlcLoaderList) Xmalloc(sizeof(XlcLoaderListRec));
    if (loader == NULL)
	return False;

    loader->proc = proc;

    if (loader_list == NULL)
	position = XlcHead;

    if (position == XlcHead) {
	loader->next = loader_list;
	loader_list = loader;
    } else {
	last = loader_list;
	while (last->next)
	    last = last->next;
	
	loader->next = NULL;
	last->next = loader;
    }

    return True;
}

XLCd
_XlcGetLC(name)
    char *name;
{
    XLCd lcd;
    int i;
    XlcLoaderList loader;

    LockMutex(_Xglobal_lock);

    /*
     * if first-time, build a list and load the one needed
     */
    if (!lcd_list) {
	lcd_list = (XLCd *) Xmalloc (sizeof(XLCd));
	if (!lcd_list)
	    goto bad;
	lcd_list[0] = (XLCd) NULL;
    }

    /*
     * search for needed lcd, if found return it
     */
    for (i = 0; lcd_list[i]; i++) {
	lcd = lcd_list[i];
	if (!strcmp (lcd->core->name, name))
	    goto found;
    }

    if (!loader_list)
	_XlcInitLoader();

    /*
     * not there, so try to get and add to list
     */
    for (loader = loader_list; loader; loader = loader->next) {
	lcd = (*loader->proc)(name);
	if (lcd) {
	    XLCd *new_list;

	    new_list = (XLCd *) Xrealloc ((char *)lcd_list,
					  (sizeof (XLCd) * (i+2)));
	    if (!new_list)
		goto bad;
	    lcd_list = new_list;
	    lcd_list[i]   = lcd;
	    lcd_list[i+1] = (XLCd) NULL;
	    goto found;
	}
    }

bad:
    lcd = (XLCd) NULL;

found:
    UnlockMutex(_Xglobal_lock);
    return lcd;
}

/*
 * Get the XLCd for the current locale
 */

XLCd
_XlcCurrentLC()
{
    return _XlcGetLC(setlocale(LC_CTYPE, (char *) NULL));
}

XrmMethods
_XrmInitParseInfo(state)
    XPointer *state;
{
    XLCd lcd =  _XlcCurrentLC();
    
    if (lcd == (XLCd) NULL)
	return (XrmMethods) NULL;
    
    return (*lcd->methods->init_parse_info)(lcd, state);
}

int
XmbTextPropertyToTextList(dpy, text_prop, list_ret, count_ret)
    Display *dpy;
    XTextProperty *text_prop;
    char ***list_ret;
    int *count_ret;
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return XLocaleNotSupported;

    return (*lcd->methods->mb_text_prop_to_list)(lcd, dpy, text_prop, list_ret,
						 count_ret);
}

int
XwcTextPropertyToTextList(dpy, text_prop, list_ret, count_ret)
    Display *dpy;
    XTextProperty *text_prop;
    wchar_t ***list_ret;
    int *count_ret;
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return XLocaleNotSupported;

    return (*lcd->methods->wc_text_prop_to_list)(lcd, dpy, text_prop, list_ret,
						 count_ret);
}

int
XmbTextListToTextProperty(dpy, list, count, style, text_prop)
    Display *dpy;
    char **list;
    int count;
    XICCEncodingStyle style;
    XTextProperty *text_prop;
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return XLocaleNotSupported;

    return (*lcd->methods->mb_text_list_to_prop)(lcd, dpy, list, count, style,
						 text_prop);
}

int
XwcTextListToTextProperty(dpy, list, count, style, text_prop)
    Display *dpy;
    wchar_t **list;
    int count;
    XICCEncodingStyle style;
    XTextProperty *text_prop;
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return XLocaleNotSupported;

    return (*lcd->methods->wc_text_list_to_prop)(lcd, dpy, list, count, style,
						 text_prop);
}

void
XwcFreeStringList(list)
    wchar_t **list;
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return;

    (*lcd->methods->wc_free_string_list)(lcd, list);
}

char *
XDefaultString()
{
    XLCd lcd = _XlcCurrentLC();
    
    if (lcd == NULL)
	return (char *) NULL;
    
    return (*lcd->methods->default_string)(lcd);
}
