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
#include "XomPublic.h"

static int
_XomGenericTextEscapement(font_set, type, text, length)
    XFontSet font_set;
    char *type;
    XPointer text;
    int length;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZE], *buf;
    int buf_len, left, width = 0;

    conv = _XomOpenConverter(font_set, type, XomNGlyphIndex);
    if (conv == NULL)
	return width;
    
    args[0] = (XPointer) &font;
    args[1] = (XPointer) &is_xchar2b;

    while (length > 0) {
	buf = xchar2b_buf;
	left = buf_len = BUFSIZE;

	if (_XomConvert(conv, (XPointer *) &text, &length, (XPointer *) &buf,
			&left, args, 2) < 0)
	    break;
	buf_len -= left;

	if (is_xchar2b)
	    width += XTextWidth16(font, xchar2b_buf, buf_len);
	else
	    width += XTextWidth(font, (char *) xchar2b_buf, buf_len);
    }

    _XomCloseConverter(conv);

    return width;
}

int
_XmbGenericTextEscapement(font_set, text, length)
    XFontSet font_set;
    char *text;
    int length;
{
    return _XomGenericTextEscapement(font_set, XlcNMultiByte,
				     (XPointer) text, length);
}

int
_XwcGenericTextEscapement(font_set, text, length)
    XFontSet font_set;
    wchar_t *text;
    int length;
{
    return _XomGenericTextEscapement(font_set, XlcNWideChar,
				     (XPointer) text, length);
}
