/*
 * $XConsortium: XFSWrap.c,v 11.1 91/04/01 18:13:55 gildea Exp $
 */

/*
 * Copyright 1991 by the Massachusetts Institute of Technology
 * Copyright 1991 by the Open Software Foundation
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
 */				

#include "Xlibint.h"
#include "Xlcint.h"

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
    XLCd lcd = _XlcCurrentLC();

    if (!lcd)
	return (XFontSet) NULL;
    return (*lcd->methods->create_fontset)
			(lcd, dpy, (char *)base_font_name_list,
			 missing_charset_list, missing_charset_count,
			 def_string);
}

/*ARGSUSED*/
int
XFontsOfFontSet(dpy, font_set, font_struct_list, font_name_list)
    Display        *dpy;
    XFontSet        font_set;
    XFontStruct  ***font_struct_list;
    char         ***font_name_list;
{
    *font_name_list   = font_set->core.font_name_list;
    *font_struct_list = font_set->core.font_struct_list;
    return font_set->core.num_of_fonts;
}

/*ARGSUSED*/
char *
XBaseFontNameListOfFontSet(dpy, font_set)
    Display        *dpy;
    XFontSet        font_set;
{
    return font_set->core.base_name_list;
}

/*ARGSUSED*/
char *
XLocaleOfFontSet(dpy, font_set)
    Display        *dpy;
    XFontSet        font_set;
{
    return font_set->core.lcd->core.name;
}

/*ARGSUSED*/
XFontSetExtents *
XExtentsOfFontSet(dpy, font_set)
    Display        *dpy;
    XFontSet        font_set;
{
    return &font_set->core.font_set_extents;
}

void
XFreeFontSet(dpy, font_set)
    Display        *dpy;
    XFontSet        font_set;
{
    (*font_set->methods->free) (font_set);
    Xfree ((char *) font_set);
}
