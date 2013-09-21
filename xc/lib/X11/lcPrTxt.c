/* $XConsortium: lcPrTxt.c,v 1.3 94/01/20 18:07:03 rws Exp $ */
/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */

#include "Xlibint.h"
#include "XlcPubI.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static XPointer *
alloc_list(is_wide_char, count, nitems)
    Bool is_wide_char;
    int count;
    int nitems;
{
    if (is_wide_char) {
	wchar_t **wstr_list;

	wstr_list = (wchar_t **) Xmalloc(count * sizeof(wchar_t *));
	if (wstr_list == NULL)
	    return (XPointer *) NULL;

	*wstr_list = (wchar_t *) Xmalloc(nitems * sizeof(wchar_t));
	if (*wstr_list == NULL) {
	    Xfree(wstr_list);
	    return (XPointer *) NULL;
	}

	return (XPointer *) wstr_list;
    } else {
	char **str_list;

	str_list = (char **) Xmalloc(count * sizeof(char *));
	if (str_list == NULL)
	    return (XPointer *) NULL;

	*str_list = (char *) Xmalloc(nitems);
	if (*str_list == NULL) {
	    Xfree(str_list);
	    return (XPointer *) NULL;
	}

	return (XPointer *) str_list;
    }
}

static void
copy_list(is_wide_char, text, list, count)
    Bool is_wide_char;
    XPointer text;
    XPointer *list;
    int count;
{
    int length;

    if (is_wide_char) {
	wchar_t *wc_text, *wstr, **wstr_list;
	
	wc_text = (wchar_t *) text;
	wstr_list = (wchar_t **) list;

	for (wstr = *wstr_list; count > 0; count--, wstr_list++) {
	    _Xwcscpy(wstr, wc_text);
	    *wstr_list = wstr;
	    length = _Xwcslen(wstr) + 1;
	    wstr += length;
	    wc_text += length;
	}
    } else {
	char *mb_text, *str, **str_list;
	
	mb_text = (char *) text;
	str_list = (char **) list;

	for (str = *str_list; count > 0; count--, str_list++) {
	    strcpy(str, mb_text);
	    *str_list = str;
	    length = strlen(str) + 1;
	    str += length;
	    mb_text += length;
	}
    }
}

static int
_XTextPropertyToTextList(lcd, dpy, text_prop, to_type, list_ret, count_ret)
    XLCd lcd;
    Display *dpy;
    XTextProperty *text_prop;
    char *to_type;
    XPointer **list_ret;
    int *count_ret;
{
    XlcConv conv;
    char *from_type;
    XPointer from, to, buf;
    char *str_ptr, *last_ptr;
    Atom encoding;
    int from_left, to_left, buf_len, ret;
    int unconv_num, nitems = text_prop->nitems;
    Bool is_wide_char = False;

    if (strcmp(XlcNWideChar, to_type) == 0)
	is_wide_char = True;

    if (nitems <= 0) {
	*list_ret = NULL;
	*count_ret = 0;
	return Success;
    }

    if (text_prop->format != 8)
	return XConverterNotFound;

    encoding = text_prop->encoding;
    if (encoding == XA_STRING)
	from_type = XlcNString;
    else if (encoding == XInternAtom(dpy, "COMPOUND_TEXT", False))
	from_type = XlcNCompoundText;
    else if (encoding == XInternAtom(dpy, XLC_PUBLIC(lcd, encoding_name), False))
	from_type = XlcNMultiByte;
    else
	return XConverterNotFound;

    if (is_wide_char) {
	buf_len = text_prop->nitems + 1;
	buf = (XPointer) Xmalloc(buf_len * sizeof(wchar_t));
    } else {
	buf_len = text_prop->nitems * XLC_PUBLIC(lcd, mb_cur_max) + 1;
	buf = (XPointer) Xmalloc(buf_len);
    }
    if (buf == NULL)
	return XNoMemory;
    to = buf;
    to_left = buf_len;

    conv = _XlcOpenConverter(lcd, from_type, lcd, to_type);
    if (conv == NULL) {
	Xfree(buf);
	return XConverterNotFound;
    }

    last_ptr = str_ptr = (char *) text_prop->value;
    unconv_num = *count_ret = 0;

    while (1) {
	if (nitems == 0 || *str_ptr == 0) {
	    if (nitems)
		str_ptr++;
	    from = (XPointer) last_ptr;
	    from_left = str_ptr - last_ptr;
	    last_ptr = str_ptr;

	    ret = _XlcConvert(conv, &from, &from_left, &to, &to_left, NULL, 0);

	    if (ret < 0)
		continue;

	    unconv_num += ret;
	    (*count_ret)++;

	    if (nitems == 0)
		break;
	    _XlcResetConverter(conv);
	} else
	    str_ptr++;

	nitems--;
    }

    _XlcCloseConverter(conv);

    if (is_wide_char)
	*((wchar_t *) to) = (wchar_t) 0;
    else
	*((char *) to) = '\0';
    to_left--;

    *list_ret = alloc_list(is_wide_char, *count_ret, buf_len - to_left);
    if (*list_ret)
	copy_list(is_wide_char, buf, *list_ret, *count_ret);

    Xfree(buf);

    return unconv_num;
}

int
_XmbTextPropertyToTextList(lcd, dpy, text_prop, list_ret, count_ret)
    XLCd lcd;
    Display *dpy;
    XTextProperty *text_prop;
    char ***list_ret;
    int *count_ret;
{
    return _XTextPropertyToTextList(lcd, dpy, text_prop, XlcNMultiByte,
				    (XPointer **) list_ret, count_ret);
}

int
_XwcTextPropertyToTextList(lcd, dpy, text_prop, list_ret, count_ret)
    XLCd lcd;
    Display *dpy;
    XTextProperty *text_prop;
    wchar_t ***list_ret;
    int *count_ret;
{
    return _XTextPropertyToTextList(lcd, dpy, text_prop, XlcNWideChar,
				    (XPointer **) list_ret, count_ret);
}

void
_XwcFreeStringList(lcd, list)
    XLCd lcd;
    wchar_t **list;
{
    if (list) {
        if (*list)
	     Xfree(*list);
        Xfree(list);
    }
}
