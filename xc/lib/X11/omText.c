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

int
_XomGenericDrawString(dpy, d, font_set, gc, x, y, type, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    char *type;
    XPointer text;
    int length;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZE], *buf;
    int buf_len, left, start_x = x;

    conv = _XomOpenConverter(font_set, type, XomNGlyphIndex);
    if (conv == NULL)
	return -1;
    
    args[0] = (XPointer) &font;
    args[1] = (XPointer) &is_xchar2b;

    while (length > 0) {
	buf = xchar2b_buf;
	left = buf_len = BUFSIZE;

	if (_XomConvert(conv, (XPointer *) &text, &length, (XPointer *) &buf,
			&left, args, 2) < 0)
	    break;
	buf_len -= left;

	XSetFont(dpy, gc, font->fid);

	if (is_xchar2b) {
	    XDrawString16(dpy, d, gc, x, y, xchar2b_buf, buf_len);
	    x += XTextWidth16(font, xchar2b_buf, buf_len);
        } else {
	    XDrawString(dpy, d, gc, x, y, (char *) xchar2b_buf, buf_len);
	    x += XTextWidth(font, (char *) xchar2b_buf, buf_len);
	}
    }

    _XomCloseConverter(conv);
    x -= start_x;

    return x;
}

int
_XmbGenericDrawString(dpy, d, font_set, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    char *text;
    int length;
{
    return _XomGenericDrawString(dpy, d, font_set, gc, x, y, XlcNMultiByte,
				 (XPointer) text, length);
}

int
_XwcGenericDrawString(dpy, d, font_set, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    wchar_t *text;
    int length;
{
    return _XomGenericDrawString(dpy, d, font_set, gc, x, y, XlcNWideChar,
				 (XPointer) text, length);
}
