/* $XConsortium$ */
/******************************************************************

              Copyright 1991, 1992 by TOSHIBA Corp.
              Copyright 1992 by FUJITSU LIMITED

 Permission to use, copy, modify, distribute, and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation, and that the name of TOSHIBA Corp. and
 FUJITSU LIMITED not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior permission.
 TOSHIBA Corp. and FUJITSU LIMITED makes no representations about the
 suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 TOSHIBA CORP. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH REGARD
 TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS, IN NO EVENT SHALL TOSHIBA CORP. AND FUJITSU LIMITED BE
 LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Author   : Katsuhisa Yano       TOSHIBA Corp.
                                 mopi@osa.ilab.toshiba.co.jp
 Modifier : Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include "Xlibint.h"
#include "XlcGeneric.h"
#include "XomGeneric.h"
#include <X11/Xos.h>
#include <X11/Xatom.h>

#define MAXFONTS		1000
#define AVERAGE_WIDTH_FIELD	12
#define CHARSET_REGISTRY_FIELD	(AVERAGE_WIDTH_FIELD + 1)

static void free_fontset();
extern int _XmbGenericTextEscapement(), _XwcGenericTextEscapement();
extern int _XmbGenericTextExtents(), _XwcGenericTextExtents();
extern Status _XmbGenericTextPerCharExtents(), _XwcGenericTextPerCharExtents();
extern int _XmbGenericDrawString(), _XwcGenericDrawString();
extern void _XmbGenericDrawImageString(), _XwcGenericDrawImageString();
extern int _XlcCompareISOLatin1();

/* method list */
static XFontSetMethodsRec fontset_methods = {
    free_fontset,
    _XmbGenericTextEscapement,
    _XmbGenericTextExtents,
    _XmbGenericTextPerCharExtents,
    _XmbGenericDrawString,
    _XmbGenericDrawImageString,
    _XwcGenericTextEscapement,
    _XwcGenericTextExtents,
    _XwcGenericTextPerCharExtents,
    _XwcGenericDrawString,
    _XwcGenericDrawImageString
};

static void
free_fontset(dpy, font_set)
    Display *dpy;
    XFontSet font_set;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData font_data;
    int count;

    if (font_data = gen->font_data) {
	for (count = gen->font_data_num; count-- > 0; font_data++) {
	    if (font_data->charset_name)
		XFree(font_data->charset_name);
	}
	XFree(gen->font_data);
    }
}

static FontData
add_fontset(font_set)
    XFontSet font_set;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData new;
    int num;

    if (num = gen->font_data_num)
        new = (FontData) Xrealloc(gen->font_data,
                                 (num + 1) * sizeof(FontDataRec));
    else
        new = (FontData) Xmalloc(sizeof(FontDataRec));

    if (new == NULL)
        return NULL;

    gen->font_data_num = num + 1;
    gen->font_data = new;

    new += num;
    bzero((char *) new, sizeof(FontDataRec));

    return new;
}

static Bool
load_fontset(font_set)
    XFontSet font_set;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData font_data;
    XLCd lcd = font_set->core.lcd;
    char **value;
    int num, i;

    gen->font_data_num = 0;

    for (i = 0; ; i++) {
	char name[8];

	sprintf(name, "cs%d", i);
	_XlcGetResource(lcd, "XLC_FONTSET", name, &value, &num);
	if (num > 0) {
	    char *tmp;
	    int len, side;

	    for ( ; num-- > 0; value++) {
		if ((font_data = add_fontset(font_set)) == NULL)
		    goto err;
		tmp = strrchr(*value, ':');
		len = tmp ? (tmp - *value) : strlen(*value);
		font_data->charset_name = (char *) Xmalloc(len + 1);
		if (font_data->charset_name == NULL)
		    goto err;
		strncpy(font_data->charset_name, *value, len);
		font_data->charset_name[len] = '\0';
		font_data->side = XlcGL;
		if (tmp) 
		    font_data->side = _XlcNCompareISOLatin1(tmp+1, "GR", 2) ?
				      XlcGL : XlcGR;
		font_data->cs_num = i;
	    }
	} else
	    break;	/* XXX */
    }

    return True;

err:
    free_fontset(lcd);

    return False;
}

static XFontSet
init_fontset(lcd)
    XLCd lcd;
{
    XFontSet fontset;

    fontset = (XFontSet) Xmalloc(sizeof(XomGenericRec));
    if (fontset == NULL)
	return NULL;
    bzero(fontset, sizeof(XomGenericRec));

    fontset->methods = &fontset_methods;
    fontset->core.lcd = lcd;

    return fontset;

err:
    Xfree(fontset);

    return NULL;
}

static char *
get_fontname(dpy, fs)
    Display *dpy;
    XFontStruct	*fs;
{
    unsigned long fp;
    char *fname = NULL;

    if (XGetFontProperty(fs, XA_FONT, &fp))
	fname = XGetAtomName(dpy, fp); 

    return fname;
}

static Bool
check_charset(xlfd_name, charset)
    char *xlfd_name;
    char *charset;
{
    char *charset_field;
    int len1, len2;

    len1 = strlen(xlfd_name);
    len2 = strlen(charset);
    if (len1 < len2)
	return False;

    /* XXX */
    charset_field = xlfd_name + (len1 - len2);
    if (!_XlcCompareISOLatin1(charset_field, charset))
	return True;
    return False;
}

static int
check_fontname(dpy, font_set, fn_list, fs_list, list_num, found_num)
    Display *dpy;
    XFontSet font_set;
    char **fn_list;
    XFontStruct *fs_list;
    int list_num;
    int found_num;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData data;
    char *fname, *prop_fname;
    int data_num;

    while (list_num--) {
	fname = *fn_list++;
 	prop_fname = get_fontname(dpy, fs_list);

	data = gen->font_data;
	data_num = gen->font_data_num;

	for ( ; data_num-- > 0; data++) {
	    if (data->font_name)
		continue;

	    if (check_charset(fname, data->charset_name))
		goto found;
	    else if (prop_fname && 
		     check_charset(prop_fname, data->charset_name)) {
		fname = prop_fname;
found:
		if (data->font_name = Xmalloc(strlen(fname) + 1)) {
		    strcpy(data->font_name, fname);
		    found_num++;
		}
		break;
	    }
	}
	if (prop_fname)
	    Xfree(prop_fname);
	if (found_num == data_num)
	    return found_num;
	fs_list++;
    }

    return found_num;
}

static Bool
init_generic_part(dpy, font_set)
    Display *dpy;
    XFontSet font_set;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData font_data = gen->font_data;
    int num = gen->font_data_num;
    XLCd lcd = font_set->core.lcd;
    CodeSet codeset;

    for ( ; num-- > 0; font_data++) {
	if (font_data->font_name == NULL)
	    continue;

	if (font_data->font == NULL) {
	    font_data->font = XLoadQueryFont(dpy, font_data->font_name);
	    if (font_data->font->min_byte1 || font_data->font->max_byte1)
		font_data->is_xchar2b = True;
	    else
		font_data->is_xchar2b = False;
	}

	codeset = *(XLC_GENERIC(lcd, codeset_list) + font_data->cs_num);
	font_data->charset = *codeset->charset_list;
    }

    return True;
}

static void
set_fontset_extents(font_set)
    XFontSet font_set;
{
    XRectangle *ink = &font_set->core.font_set_extents.max_ink_extent;
    XRectangle *logical = &font_set->core.font_set_extents.max_logical_extent;
    XFontStruct **font_list, *font;
    XCharStruct overall;
    int logical_ascent, logical_descent;
    int	num = font_set->core.num_of_fonts;

    font_list = font_set->core.font_struct_list;
    font = *font_list++;
    overall = font->max_bounds;
    overall.lbearing = font->min_bounds.lbearing;
    logical_ascent = font->ascent;
    logical_descent = font->descent;

    while (--num > 0) {
	font = *font_list++;
	overall.lbearing = min(overall.lbearing, font->min_bounds.lbearing);
	overall.rbearing = max(overall.rbearing, font->max_bounds.rbearing);
	overall.ascent = max(overall.ascent, font->max_bounds.ascent);
	overall.descent = max(overall.descent, font->max_bounds.descent);
	overall.width = max(overall.width, font->max_bounds.width);
	logical_ascent = max(logical_ascent, font->ascent);
	logical_descent = max(logical_descent, font->descent);
    }

    ink->x = overall.lbearing;
    ink->y = -(overall.ascent);
    ink->width = overall.rbearing - overall.lbearing;
    ink->height = overall.ascent + overall.descent;

    logical->x = 0;
    logical->y = -(logical_ascent);
    logical->width = overall.width;
    logical->height = logical_ascent + logical_descent;
}

static Bool
init_core_part(font_set, base_name)
    XFontSet font_set;
    char *base_name;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData font_data;
    int font_data_num;
    XFontStruct **font_struct_list;
    char **font_name_list, *font_name_buf;
    int	count, length;

    font_data = gen->font_data;
    font_data_num = gen->font_data_num;
    count = length = 0;

    for ( ; font_data_num-- > 0; font_data++) {
	if (font_data->font == NULL)
	    continue;

	length += strlen(font_data->font_name) + 1;
	count++;
    }
    if (count == 0)
        return False;

    font_struct_list = (XFontStruct **) Xmalloc(sizeof(XFontStruct *) * count);
    if (font_struct_list == NULL)
	return False;

    font_name_list = (char **) Xmalloc(sizeof(char *) * count);
    if (font_name_list == NULL)
	goto err;

    font_name_buf = (char *) Xmalloc(length);
    if (font_name_buf == NULL)
	goto err;

    font_set->core.base_name_list = base_name;
    font_set->core.num_of_fonts = count;
    font_set->core.font_name_list = font_name_list;
    font_set->core.font_struct_list = font_struct_list;
    font_set->core.context_dependent = False;

    font_data = gen->font_data;
    font_data_num = gen->font_data_num;

    for ( ; font_data_num-- > 0; font_data++) {
	if (font_data->font == NULL)
	    continue;

	*font_struct_list++ = font_data->font;
	strcpy(font_name_buf, font_data->font_name);
	Xfree(font_data->font_name);
	font_data->font_name = font_name_buf;
	*font_name_list++ = font_name_buf;
	font_name_buf += strlen(font_name_buf) + 1;
    }

    set_fontset_extents(font_set);

    return True;

err:
    if (font_name_list)
	Xfree(font_name_list);
    Xfree(font_struct_list);

    return False;
}

static Bool
set_missing_list(font_set, missing_charset_list, missing_charset_count)
    XFontSet font_set;
    char ***missing_charset_list;
    int *missing_charset_count;
{
    XomGenericPart *gen = XOM_GENERIC(font_set);
    FontData font_data;
    char **charset_list, *charset_buf;
    int	count, length, font_data_num;

    font_data = gen->font_data;
    font_data_num = gen->font_data_num;
    count = length = 0;

    for ( ; font_data_num-- > 0; font_data++) {
	if (font_data->font) 
	    continue;
	
	length += strlen(font_data->charset_name) + 1;
	count++;
    }

    if (count < 1)
	return True;

    charset_list = (char **) Xmalloc(sizeof(char *) * count);
    if (charset_list == NULL)
	return False;

    charset_buf = (char *) Xmalloc(length);
    if (charset_buf == NULL) {
	Xfree(charset_list);
	return False;
    }

    *missing_charset_list = charset_list;
    *missing_charset_count = count;

    font_data = gen->font_data;
    font_data_num = gen->font_data_num;

    for ( ; font_data_num-- > 0; font_data++) {
	if (font_data->font) 
	    continue;

	strcpy(charset_buf, font_data->charset_name);
	*charset_list++ = charset_buf;
	charset_buf += strlen(charset_buf) + 1;
    } 

    return True;
}

XFontSet
_XomGenericCreateFontSet(lcd, dpy, base_name, name_list, count,
			 missing_list, missing_count)
    XLCd lcd;
    Display *dpy;
    char *base_name;
    char **name_list;		
    int count;
    char ***missing_list;
    int *missing_count;	
{
    XFontSet font_set;
    XomGenericPart *gen;
    FontData font_data;
    char *name, **name_list_ptr, **fn_list, buf[BUFSIZE];
    XFontStruct *fs_list;
    int font_data_num, length, fn_num, found_num = 0;
    Bool is_found;
    int dash_num;
    char *name_tmp;

    *missing_list = NULL;
    *missing_count = 0;

    if ((font_set = init_fontset(lcd)) == NULL)
        return (XFontSet) NULL;

    if (load_fontset(font_set) == False)
	goto err;
    gen = XOM_GENERIC(font_set);

    name_list_ptr = name_list;
    while (count--) {
        name = *name_list_ptr++;
 	length = strlen(name);
 	/* XXX */
 	if (length > 1 && name[length - 1] == '*' &&  name[length - 2] == '-') {
 	    (void) strcpy(buf, name);
 	    is_found = False;
 
	    font_data = gen->font_data;
	    font_data_num = gen->font_data_num;

	    for ( ; font_data_num-- > 0; font_data++) {
 		if (font_data->font_name)
 		    continue;

		dash_num = 0;
		name_tmp = buf;
		while (name_tmp = strchr(name_tmp, '-')) {
		  name_tmp++;
		  dash_num++;
		}
 		if (length > 2 && name[length - 3] == '*')
		    (void) strcpy(buf + length - ((dash_num==14)?3:1), 
				  font_data->charset_name);
 		else {
 		    buf[length] = '-';
 		    (void) strcpy(buf + length + 1, font_data->charset_name);
 		}
 		fn_list = XListFonts(dpy, buf, 1, &fn_num);
 		if (fn_num == 0)
 		    continue;
 
		font_data->font_name = (char *) Xmalloc(strlen(*fn_list) + 1);
 		if (font_data->font_name == NULL)
 		    goto err;
 		(void) strcpy(font_data->font_name, *fn_list);
 
 		XFreeFontNames(fn_list);
 		found_num++;
 		is_found = True;
 	    }
 	    if (found_num == gen->font_data_num)
 		break;
 	    if (is_found == True)
 		continue;
 	}
 
	fn_list = XListFontsWithInfo(dpy, name, MAXFONTS, &fn_num, &fs_list);
	if (fn_num == 0) {
	    char *p;
	    int n = 0;

	    (void) strcpy(buf, name);

	    p = name = buf;
	    while (p = (char *)strchr(p, '-')) {
		p++;
		n++;
	    }
	    p = name + strlen(name) - 1;
	    if (n == AVERAGE_WIDTH_FIELD && *p != '-')
		(void) strcat(name, "-*");
	    else if (n == CHARSET_REGISTRY_FIELD && *p == '-')
		(void) strcat(name, "*");
	    else
		continue;

	    fn_list = XListFontsWithInfo(dpy, name, MAXFONTS,
							&fn_num, &fs_list);
	    if (fn_num == 0)
		continue;
	}
	found_num = check_fontname(dpy, font_set, fn_list, fs_list, fn_num,
				   found_num);
	XFreeFontInfo(fn_list, fs_list, fn_num);
	if (found_num == gen->font_data_num)
	    break;
    }

    if (found_num == 0) {
	set_missing_list(font_set, missing_list, missing_count);
	goto err;
    }

    if (init_generic_part(dpy, font_set) == False)
	goto err;

    if (init_core_part(font_set, base_name) == False)
	goto err;

    if (set_missing_list(font_set, missing_list, missing_count) == False)
	goto err;

    XFreeStringList(name_list);		

    return font_set;

err:
    if (font_set->core.font_name_list)
	XFreeStringList(font_set->core.font_name_list);
    if (font_set->core.font_struct_list)
	Xfree(font_set->core.font_struct_list);

    free_fontset(dpy, font_set);

    return (XFontSet) NULL;
}
