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

#ifndef _XOMGENERIC_H_
#define _XOMGENERIC_H_

#include "XomPublic.h"

#define XOM_GENERIC(font_set)	(&((XomGeneric) font_set)->gen)

/*
 * XFontSet dependent data
 */

typedef struct _FontDataRec {
    char *charset_name;
    char *font_name;
    XFontStruct *font;
    XlcCharSet charset;
    XlcSide side;
    Bool is_xchar2b;
    int cs_num;
} FontDataRec, *FontData;

typedef struct _XomGenericPart {
    int font_data_num;
    FontData font_data;
    XlcConv mb_to_glyph;
    XlcConv wc_to_glyph;
} XomGenericPart;

typedef struct _XomGenericRec {
    XFontSetMethods methods;
    XFontSetCoreRec core;	
    XomGenericPart gen;
} XomGenericRec, *XomGeneric;

#endif  /* _XOMGENERIC_H_ */
