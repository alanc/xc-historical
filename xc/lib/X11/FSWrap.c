/*
 * $XConsortium: FSWrap.c,v 11.10 94/01/20 18:01:37 rws Exp $
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
#include <ctype.h>
#include <X11/Xos.h>


#define	MAXLIST	256

char **
_XParseBaseFontNameList(str, num)
    char           *str;
    int            *num;
{
    char           *plist[MAXLIST];
    char          **list;
    char           *ptr;

    *num = 0;
    if (!str || !*str) {
	return (char **)NULL;
    }
    while (*str && isspace(*str))
	str++;
    if (!*str)
	return (char **)NULL;

    if (!(ptr = Xmalloc((unsigned)strlen(str) + 1))) {
	return (char **)NULL;
    }
    strcpy(ptr, str);

    while (1) {
	char	*back;

	plist[*num] = ptr;
	if ((ptr = strchr(ptr, ','))) {
	    back = ptr;
	} else {
	    back = plist[*num] + strlen(plist[*num]);
	}
	while (isspace(*(back - 1)))
	    back--;
	*back = '\0';
	(*num)++;
	if (!ptr)
	    break;
	ptr++;
	while (*ptr && isspace(*ptr))
	    ptr++;
	if (!*ptr)
	    break;
    }
    if (!(list = (char **) Xmalloc((unsigned)sizeof(char *) * (*num + 1)))) {
	Xfree(ptr);
	return (char **)NULL;
    }
    memcpy((char *)list, (char *)plist, sizeof(char *) * (*num));
    *(list + *num) = NULL;

    return list;
}

static char **
copy_string_list(string_list, list_count)
    char **string_list;
    int list_count;
{
    char **string_list_ret, **list_src, **list_dst, *dst;
    int length, count;

    if (string_list == NULL)
	return (char **) NULL;

    string_list_ret = (char **) Xmalloc(sizeof(char *) * list_count);
    if (string_list_ret == NULL)
	return (char **) NULL;

    list_src = string_list;
    count = list_count;
    for (length = 0; count-- > 0; list_src++)
	length += strlen(*list_src) + 1;

    dst = (char *) Xmalloc(length);
    if (dst == NULL) {
	Xfree(string_list_ret);
	return (char **) NULL;
    }

    list_src = string_list;
    count = list_count;
    list_dst = string_list_ret;
    for ( ;  count-- > 0; list_src++) {
	strcpy(dst, *list_src);
	*list_dst++ = dst;
	dst += strlen(dst) + 1;
    }

    return string_list_ret;
}

#if NeedFunctionPrototypes
XFontSet
XCreateFontSet (
    Display        *dpy,
    _Xconst char   *base_font_name_list,
    char         ***missing_charset_list,
    int            *missing_charset_count,
    char          **def_string)
#else
XFontSet
XCreateFontSet (dpy, base_font_name_list, missing_charset_list,
	        missing_charset_count, def_string)
    Display        *dpy;
    char           *base_font_name_list;
    char         ***missing_charset_list;
    int            *missing_charset_count;
    char          **def_string;
#endif
{
    XOM om;
    XOC oc;
    XOMCharSetList *list;

    *missing_charset_list = NULL;
    *missing_charset_count = 0;

    om = XOpenOM(dpy, NULL, NULL, NULL);
    if (om == NULL)
	return (XFontSet) NULL;
    
    if (oc = XCreateOC(om, XNBaseFontName, base_font_name_list, NULL)) {
	list = &oc->core.missing_list;
	oc->core.om_automatic = True;
    } else
	list = &om->core.required_charset;
    
    *missing_charset_list = copy_string_list(list->charset_list,
					     list->charset_count);
    *missing_charset_count = list->charset_count;

    if (list->charset_list && *missing_charset_list == NULL)
	oc = NULL;

    if (oc && def_string) {
	*def_string = oc->core.default_string;
	if (!*def_string)
	    *def_string = "";
    }
    
    if (oc == NULL)
	XCloseOM(om);

    return (XFontSet) oc;
}

int
XFontsOfFontSet(font_set, font_struct_list, font_name_list)
    XFontSet        font_set;
    XFontStruct  ***font_struct_list;
    char         ***font_name_list;
{
    *font_name_list   = font_set->core.font_info.font_name_list;
    *font_struct_list = font_set->core.font_info.font_struct_list;
    return font_set->core.font_info.num_font;
}

char *
XBaseFontNameListOfFontSet(font_set)
    XFontSet        font_set;
{
    return font_set->core.base_name_list;
}

char *
XLocaleOfFontSet(font_set)
    XFontSet        font_set;
{
    return font_set->core.om->core.lcd->core->name;
}

extern Bool XContextDependentDrawing(font_set)
    XFontSet        font_set;
{
    return font_set->core.context_dependent;
}

Bool
XDirectionalDependentDrawing(font_set)
    XFontSet        font_set;
{
    return font_set->core.directional_dependent;
}

Bool
XContextualDrawing(font_set)
    XFontSet        font_set;
{
    return font_set->core.contextual_drawing;
}

XFontSetExtents *
XExtentsOfFontSet(font_set)
    XFontSet        font_set;
{
    return &font_set->core.font_set_extents;
}

void
XFreeFontSet(dpy, font_set)
    Display        *dpy;
    XFontSet        font_set;
{
    XCloseOM(font_set->core.om);
}
