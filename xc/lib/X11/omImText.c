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

extern int _XomGenericTextExtents(), _XomGenericDrawString();

#define GET_VALUE_MASK	(GCFunction | GCForeground | GCBackground | GCFillStyle)
#define SET_VALUE_MASK	(GCFunction | GCForeground | GCFillStyle)

static void
_XomGenericDrawImageString(dpy, d, font_set, gc, x, y, type, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    char *type;
    XPointer text;
    int length;
{
    XGCValues values;
    XRectangle extent;

    XGetGCValues(dpy, gc, GET_VALUE_MASK, &values);

    XSetFunction(dpy, gc, GXcopy);
    XSetForeground(dpy, gc, values.background);
    XSetFillStyle(dpy, gc, FillSolid);

    _XomGenericTextExtents(font_set, type, text, length, 0, &extent);
    XFillRectangle(dpy, d, gc, x + extent.x, y + extent.y, extent.width,
		   extent.height);

    XChangeGC(dpy, gc, SET_VALUE_MASK, &values);

    _XomGenericDrawString(dpy, d, font_set, gc, x, y, type, text, length);
}

void
_XmbGenericDrawImageString(dpy, d, font_set, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    char *text;
    int length;
{
    _XomGenericDrawImageString(dpy, d, font_set, gc, x, y, XlcNMultiByte,
			       (XPointer) text, length);
}

void
_XwcGenericDrawImageString(dpy, d, font_set, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XFontSet font_set;
    GC gc;
    int x, y;
    wchar_t *text;
    int length;
{
    _XomGenericDrawImageString(dpy, d, font_set, gc, x, y, XlcNWideChar,
			       (XPointer) text, length);
}
