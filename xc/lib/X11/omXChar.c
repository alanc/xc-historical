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
#include "XlcPublic.h"
#include "XomGeneric.h"

typedef struct _StateRec {
    XLCd lcd;
    XlcConv conv;
    XomGeneric font_set;
} StateRec, *State;

static FontData
_XomGetFontDataFromCharSet(font_set, charset)
    XomGeneric font_set;
    XlcCharSet charset;
{
    register FontData font_data = XOM_GENERIC(font_set)->font_data;
    register num = XOM_GENERIC(font_set)->font_data_num;

    for ( ; num-- > 0; font_data++)
	if (font_data->charset == charset)
	    return font_data;

    return (FontData) NULL;
}

#ifdef MUSTCOPY
static void
cs_to_xchar2b(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++;
	to->byte2 = *from++;
	to++;
    }
}

static void
cs_to_xchar2b_gl(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++ & 0x7f;
	to->byte2 = *from++ & 0x7f;
	to++;
    }
}

static void
cs_to_xchar2b_gr(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++ | 0x80;
	to->byte2 = *from++ | 0x80;
	to++;
    }
}
#endif

static void
shift_to_gl(text, length)
    register char *text;
    register length;
{
    while (length-- > 0)
	*text++ &= 0x7f;
}

static void
shift_to_gr(text, length)
    register char *text;
    register length;
{
    while (length-- > 0)
	*text++ |= 0x80;
}

int
_XomConvert(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XPointer cs, lc_args[1];
    XlcCharSet charset;
    int length, cs_left, ret;
    FontData font_data;
    XomGeneric fontset = state->font_set;
#ifdef MUSTCOPY
    XChar2b *xchar2b;
    char *buf, buf_local[BUFSIZE];
#endif
    
    cs = *to;
    cs_left = *to_left;
    lc_args[0] = (XPointer) &charset;

    ret = _XlcConvert(state->conv, from, from_left, &cs, &cs_left, lc_args, 1);
    if (ret < 0)
	return -1;

    font_data = _XomGetFontDataFromCharSet((XomGeneric) fontset, charset);
    if (font_data == NULL || font_data->font == NULL)
	return -1;

    length = *to_left - cs_left;

#ifdef MUSTCOPY
    if (font_data->is_xchar2b) {
	buf = (length > BUFSIZE) ? Xmalloc(length) : buf_local;
	if (buf == NULL)
	    return -1;
	memcpy(buf, (char *) *to, length);

	xchar2b = (XChar2b *) *to;
	length >>= 1;

	if (font_data->side == charset->side)
	    cs_to_xchar2b(buf, xchar2b, length);
	else if (font_data->side == XlcGL)
	    cs_to_xchar2b_gl(buf, xchar2b, length);
	else if (font_data->side == XlcGR)
	    cs_to_xchar2b_gr(buf, xchar2b, length);
	else
	    cs_to_xchar2b(buf, xchar2b, length);
	
	if (buf != buf_local)
	    XFree(buf);

	*to = (XPointer) (xchar2b + length);
	*to_left -= length;
    } else
#endif
    {
	if (font_data->side != charset->side) {
	    if (font_data->side == XlcGL)
		shift_to_gl(*to, length);
	    else if (font_data->side == XlcGR)
		shift_to_gr(*to, length);
	}

	if (font_data->is_xchar2b)
	    length >>= 1;

	*to = cs;
	*to_left -= length;
    }

    *((XFontStruct **) args[0]) = font_data->font;
    *((Bool *) args[1]) = font_data->is_xchar2b;

    return ret;
}

void
_XomCloseConverter(conv)
    XlcConv conv;
{
    State state = (State) conv->state;
    
    _XlcResetConverter(state->conv);
}

XlcConv
_XomOpenConverter(xfontset, from_type, to_type)
    XFontSet xfontset;
    char *from_type;
    char *to_type;
{
    XomGeneric fontset = (XomGeneric) xfontset;
    XomGenericPart *gen = XOM_GENERIC(fontset);
    XLCd lcd = fontset->core.lcd;
    XlcConv conv;
    State state;

    if (strcmp(from_type, XlcNWideChar) == 0)
	conv = gen->wc_to_glyph;
    else
	conv = gen->mb_to_glyph;

    if (conv)
	return conv;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == NULL)
	return (XlcConv) NULL;

    conv->state = (XPointer) Xmalloc(sizeof(StateRec));
    if (conv->state == NULL)
	goto err;
    
    state = (State) conv->state;
    state->lcd = lcd;
    state->conv = _XlcOpenConverter(lcd, from_type, lcd, XlcNCharSet);
    if (state->conv == NULL)
	goto err;
    state->font_set = fontset;
    
    if (strcmp(from_type, XlcNWideChar) == 0)
	gen->wc_to_glyph = conv;
    else
	gen->mb_to_glyph = conv;

    return conv;

err:
    if (conv) {
	if (conv->state)
	    XFree(conv->state);

	XFree(conv);
    }

    return (XlcConv) NULL;
}
