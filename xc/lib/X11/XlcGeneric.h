/* $XConsortium: XlcGeneric.h,v 1.1 93/09/17 13:30:36 rws Exp $ */
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

#ifndef _XLCGENERIC_H_
#define _XLCGENERIC_H_

#include "XlcPubI.h"

typedef struct _ParseInfoRec *ParseInfo;

typedef struct _CodeSetRec {
    XlcCharSet *charset_list;
    int num_charsets;
    int cs_num;
    XlcSide side;
    int length;
    ParseInfo parse_info;
    unsigned long wc_encoding;
} CodeSetRec, *CodeSet;

typedef enum {
    E_GL,			/* GL encoding */
    E_GR,			/* GR encoding */
    E_SS,			/* single shift */
    E_LSL,			/* locking shift left */
    E_LSR,			/* locking shift right */
    E_LAST
} EncodingType;

typedef struct _ParseInfoRec {
    EncodingType type;
    char *encoding;
    CodeSet codeset;
} ParseInfoRec;

/*
 * XLCd private data
 */

#define XLC_GENERIC(lcd, x)	(((XLCdGeneric) lcd->core)->gen.x)
#define XLC_GENERIC_PART(lcd)	(&(((XLCdGeneric) lcd->core)->gen))

typedef struct _XLCdGenericPart {
    int codeset_num;
    CodeSet *codeset_list;
    unsigned char *mb_parse_table;
    int mb_parse_list_num;
    ParseInfo *mb_parse_list;
    unsigned long wc_encode_mask;
    unsigned long wc_shift_bits;
    CodeSet initial_state_GL;
    CodeSet initial_state_GR;
} XLCdGenericPart;

typedef struct _XLCdGenericRec {
    XLCdCoreRec core;	
    XLCdPublicPart pub;
    XLCdGenericPart gen;	
} XLCdGenericRec, *XLCdGeneric;

extern XLCdMethods _xlcGenericMethods;

#endif  /* _XLCGENERIC_H_ */
