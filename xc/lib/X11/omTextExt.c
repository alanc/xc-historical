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
_XomGenericTextExtents(font_set, type, text, length, overall_ink,
		       overall_logical)
    XFontSet font_set;
    char *type;
    XPointer text;
    int length;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZE], *buf;
    int direction, logical_ascent, logical_descent, tmp_ascent, tmp_descent;
    XCharStruct overall, tmp_overall;
    int buf_len, left;
    Bool first = True;

    conv = _XomOpenConverter(font_set, type, XomNGlyphIndex);
    if (conv == NULL)
	return 0;	/* XXX */
    
    bzero(&overall, sizeof(XCharStruct));
    logical_ascent = logical_descent = 0;

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
	    XTextExtents16(font, xchar2b_buf, buf_len, &direction,
			   &tmp_ascent, &tmp_descent, &tmp_overall);
	else
	    XTextExtents(font, (char *) xchar2b_buf, buf_len, &direction,
			 &tmp_ascent, &tmp_descent, &tmp_overall);

	if (first) {	/* initialize overall */
	    overall = tmp_overall;
	    logical_ascent = tmp_ascent;
	    logical_descent = tmp_descent;
	    first = False;
	} else {
	    overall.lbearing = min(overall.lbearing,
				   overall.width + tmp_overall.lbearing);
	    overall.rbearing = max(overall.rbearing,
				   overall.width + tmp_overall.rbearing);
	    overall.ascent = max(overall.ascent, tmp_overall.ascent);
	    overall.descent = max(overall.descent, tmp_overall.descent);
	    overall.width += tmp_overall.width;
	    logical_ascent = max(logical_ascent, tmp_ascent);
	    logical_descent = max(logical_descent, tmp_descent);
	}
    }

    _XomCloseConverter(conv);

    if (overall_ink) {
	overall_ink->x = overall.lbearing;
	overall_ink->y = -(overall.ascent);
	overall_ink->width = overall.rbearing - overall.lbearing;
	overall_ink->height = overall.ascent + overall.descent;
    }

    if (overall_logical) {
	overall_logical->x = 0;
        overall_logical->y = -(logical_ascent);
	overall_logical->width = overall.width;
        overall_logical->height = logical_ascent + logical_descent;
    }

    return overall.width;
}

int
_XmbGenericTextExtents(font_set, text, length, overall_ink, overall_logical)
    XFontSet font_set;
    char *text;
    int length;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
{
    return _XomGenericTextExtents(font_set, XlcNMultiByte, (XPointer) text,
				  length, overall_ink, overall_logical);
}

int
_XwcGenericTextExtents(font_set, text, length, overall_ink, overall_logical)
    XFontSet font_set;
    wchar_t *text;
    int length;
    XRectangle *overall_ink;
    XRectangle *overall_logical;
{
    return _XomGenericTextExtents(font_set, XlcNWideChar, (XPointer) text,
				  length, overall_ink, overall_logical);
}
