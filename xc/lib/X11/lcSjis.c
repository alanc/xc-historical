/* $XConsortium: lcSjis.c,v 1.3 93/09/20 10:33:39 rws Exp $ */
/****************************************************************

        Copyright 1992, 1993 by FUJITSU LIMITED
        Copyright 1993 by Fujitsu Open Systems Solutions, Inc.

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of FUJITSU LIMITED and
Fujitsu Open Systems Solutions, Inc. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.
FUJITSU LIMITED and Fujitsu Open Systems Solutions, Inc. makes no
representations about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

FUJITSU LIMITED AND FUJITSU OPEN SYSTEMS SOLUTIONS, INC. DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL FUJITSU OPEN SYSTEMS
SOLUTIONS, INC. AND FUJITSU LIMITED BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

    Author:  Jeffrey Bloomfield		(jeffb@ossi.com)
	     Shigeru Yamada		(yamada@ossi.com)
*****************************************************************/

#include "Xlibint.h"
#include "XlcGeneric.h"
#include <ctype.h>

#define CS0	codesets[0]		/* Codeset 0 - 7-bit ASCII	*/
#define CS1	codesets[1]		/* Codeset 1 - Kanji		*/
#define CS2	codesets[2]		/* Codeset 2 - Half-Kana	*/
#define CS3	codesets[3]		/* Codeset 3 - User defined	*/

#define GR	0x80	/* begins right-side (non-ascii) region */
#define GL	0x7f    /* ends left-side (ascii) region        */

typedef unsigned char   Uchar;

/* Acceptable range for 2nd byte of SJIS multibyte char */
#define VALID_MULTIBYTE(c) \
			((0x40<=((Uchar)c) && ((Uchar)c)<=0x7e) \
			|| (0x80<=((Uchar)c) && ((Uchar)c)<=0xfc))

#ifndef iskanji
#define iskanji(c)	((0x81<=((Uchar)c) && ((Uchar)c)<=0x9f) \
			|| (0xe0<=((Uchar)c) && ((Uchar)c)<=0xef))
#endif /* !iskanji */

#ifndef iskana
#define iskana(c)	(0xa1<=((Uchar)c) && ((Uchar)c)<=0xdf)
#endif /* !iskana */

#define	isuserdef(c)	(0xf0<=((Uchar)c) && ((Uchar)c)<=0xfc)

#define isundef(c)	((0xfd<=((Uchar)c) && ((Uchar)c)<=0xff) \
			|| ((Uchar)c)==0x80 || ((Uchar)c)==0xa0)

#define isleftside(c)	(((c) & GR) ? 0 : 1)
#define isrightside(c)	(!isleftside(c))

#define BIT8OFF(c)	((c) & GL)
#define BIT8ON(c)	((c) | GR)


/* structs used for CharSet (XXXtocs/cstoXXX) converters */

typedef struct _ConvRec {
    XlcConv from_conv;
    XlcConv to_conv;
} ConvRec, *Conv;

typedef struct _StateRec {
    XLCd lcd;
    XlcCharSet charset;
    XlcCharSet GL_charset;
    XlcCharSet GR_charset;
} StateRec, *State;

static int init_state();
static void jis_to_sjis();
static void sjis_to_jis();


/*
 * SJISmb <-> SJISwc Converters - Notes
 *
 * 16-bit widechar format is limited to 14 data bits.  Since the 2nd byte
 * of SJIS multibyte chars are in the ranges of 0x40 - 7E and 0x80 - 0xFC,
 * SJIS cannot map directly into 16 bit widechar format within the confines
 * of a single codeset.  Therefore, for SJIS widechar conversion SJIS Kanji
 * is mapped into the JIS codeset.  (The algorithms used in jis_to_sjis() and
 * sjis_to_jis() are from Ken Lunde (lunde@mv.us.adobe.com) and are in the
 * public domain.)
 */

/*---------------------Direct Converter Functions-----------------------------*/

static int
sjis_mbstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{


    XLCd lcd = (XLCd)conv->state;

    register int chr_len = 0;
    register int inbuf_len = *from_left;
    register int outbuf = 0;
    register int outbuf_len = *to_left;
    register int shift_mult = 0;
    register unsigned int chrcode = 0;

    unsigned char ch, ch2;
    unsigned int wc_encode = 0;
    unsigned int wc_tmp = 0;

    unsigned int j1, j2;

    Bool new_char;

    int errflg = 0;
    int firstbyte;
    int length;
    int outbuf_flg = False;
    int ret;

    XPointer inbufptr = *from;
    wchar_t *outbufptr = (wchar_t *) *to;
    int wcstr = (int)outbufptr;

    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list); 
    unsigned long wc_shift = XLC_GENERIC(lcd, wc_shift_bits);

    if (outbuf_len > 0)         /* relies on to_left initialized to 0 */
	outbuf_flg = True;      /* in _Xlcmbtowc()		      */

     for(new_char = True, firstbyte = True; inbuf_len > 0;) {

	ch = *inbufptr++;

	if (firstbyte) {
	    if (isascii(ch)) {
		length = CS0->length;
		*outbufptr++ = (wchar_t)ch;
		inbuf_len--;
		continue;
	    }
	    else if (iskanji(ch)) {
		firstbyte = False;
		inbuf_len--;
		length = CS1->length;
		wc_encode = CS1->wc_encoding;
		ch2 = *inbufptr;
		sjis_to_jis(&ch, &ch2);
		chrcode = ch;
	    }
	    else if (iskana(ch)) {
		length = CS2->length;
		wc_encode = CS2->wc_encoding;
		chrcode = BIT8OFF(ch);
		inbuf_len--;
	    }
	    else if (isuserdef(ch)) {
		firstbyte = False;
		inbuf_len--;
		length = CS3->length;
		wc_encode = CS3->wc_encoding;
		ch2 = *inbufptr;
		sjis_to_jis(&ch, &ch2);
		chrcode = ch;
	    }
	    else if (isundef(ch)) {	/* ignore input and continue */
		inbuf_len--;
		continue;
	    }
	} else {	     /* additional byte(s) of multibyte char */
	    inbuf_len--;

	    if (!VALID_MULTIBYTE((unsigned char) *(inbufptr-1))) {
		errflg++;
		continue;
	    }
	    chrcode = ch2;
	}

	if (new_char) {
	    chr_len = length;
	    shift_mult = length - 1;
	    new_char = False;
	}

	if (length < 1)
	    break;

	chrcode <<= (wc_shift * shift_mult);
	shift_mult--;
	wc_tmp |= chrcode;
	if (--chr_len == 0) {
	    wc_tmp |= wc_encode;
	    *outbufptr++ = wc_tmp;

	    firstbyte = True;
	    new_char = True;
	    wc_tmp = (unsigned int)0;

	    if (outbuf_flg == True)
		if (outbuf++ >= outbuf_len) {
		    errflg++;
		    break;
		}
	}

    } /* end for */

    if (errflg) {
	return -1;
    }

/* Note:  use sizeof(wchar_t) because it may have been redefined in Xlib.h: */

    if ((ret = ((int)(outbufptr) - wcstr)/sizeof(wchar_t)) > 0) {
        *to_left = (*to_left) - ret;     /* used by caller in XlcStd.c for  */
        *from_left = 0;                  /* NULL termination & return value */
        return ret;
    }

    return -1;
}


/* Useful macros for sjis_wctombs */

#define byte1		(length == clength - 1)		    /* 1st char byte */
#define byte2		(byte1 == 0)
#define kanji		(wch_encode == CS1->wc_encoding)
#define kana		(wch_encode == CS2->wc_encoding)

static int
sjis_wcstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t *inbufptr = (wchar_t *) *from;
    register XPointer outbufptr = *to;
    register wchar_t  wch;
    register length, clength;
    unsigned char tmp;
    unsigned char t1, t2;
    register wchar_t outbuf = 0;
    register int outbuf_len = *to_left;
    int outbuf_flg = False;
    int to_bytes;
    int errflg = 0;
    int ret;
    int wch_len = *from_left, buf_len = *to_left; 


    XLCd lcd = (XLCd)conv->state;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    unsigned long wc_encoding_mask = XLC_GENERIC(lcd, wc_encode_mask);
    unsigned long wc_shift = XLC_GENERIC(lcd, wc_shift_bits);
    unsigned long wch_encode;
    XlcSide side = 0;


    if (outbuf_len > 0)
	outbuf_flg = True;

    for (to_bytes = 0; wch_len > 0 && buf_len > 0; wch_len--, buf_len--) {

	wch = *inbufptr++;
	wch_encode = wch & wc_encoding_mask;

	if (wch_encode == CS0->wc_encoding) {	   /* ascii:  CS0 */
	    clength = length = CS0->length;
	    side = CS0->side;
	    wch = wch ^ CS0->wc_encoding;
	}
	else if (wch_encode == CS1->wc_encoding) { /* Kanji: CS1 */
	    clength = length = CS1->length;
	    side = CS1->side;
	    wch = wch ^ CS1->wc_encoding;
	}
	else if (wch_encode == CS2->wc_encoding) { /* Kana:  CS2 */
	    clength = length = CS2->length;
	    side = CS2->side;
	    wch = wch ^ CS2->wc_encoding;
	}
	else if (wch_encode == CS3->wc_encoding) { /* Userdef:  CS3 */
	    clength = length = CS3->length;
	    side = CS3->side;
	    wch = wch ^ CS3->wc_encoding;
	}

	switch(side) {
	    case XlcGR:
		side = 0x80;
		break;
	    case XlcGL:
		side = XlcGL;
		break;
	    default:
		errflg++;
		break;
	}

	do {
	    length--;

	    tmp = BIT8OFF((wch>>length * wc_shift)) | side;

	    if ((byte1 && kanji) || kana)
		tmp = BIT8ON(tmp);

	    if (byte1 && kanji) {
		t1 = BIT8OFF(tmp);
		to_bytes++;
		continue;
	    }
	    if (byte2 && kanji) {
		t2 = BIT8OFF(tmp);
		jis_to_sjis(&t1, &t2);
		*outbufptr++ = (char)t1;
		tmp = t2;
	    }

	    *outbufptr++ = (char)tmp;
	    to_bytes++;
	} while (length); 

	if (outbuf_flg == True)
	    if (outbuf++ >= outbuf_len) {
		errflg++;
		break;
	    }

    }	/* end for */

    if (errflg)
	return -1;

    if ((ret = to_bytes) > 0) {
	*to_left = (*to_left) - to_bytes; /* used by caller in XlcStd.c for  */
	*from_left = 0;			  /* NULL termination & return value */
	return to_bytes;
    }

    return -1;
}
#undef byte1
#undef byte2
#undef kanji
#undef kana


/*
 * sjis<->jis conversion for widechar kanji (See Note at top of file)
 */
static void
sjis_to_jis(p1, p2)
    unsigned char *p1;
    unsigned char *p2;
{
  register unsigned char c1 = *p1;
  register unsigned char c2 = *p2;
  register unsigned char adjust = c2 < 0x9f;
  register unsigned char rowOffset = c1 < 0xa0 ? 0x70 : 0xb0;
  register unsigned char cellOffset = adjust ? (0x1f + (c2 > 0x7f)) : 0x7e;

  *p1 = ((c1 - rowOffset) << 1) - adjust;
  *p2 -= cellOffset;
}

static void
jis_to_sjis(p1, p2)
    unsigned char *p1;
    unsigned char *p2;
{
  register unsigned char c1 = *p1;
  register unsigned char c2 = *p2;
  register unsigned char rowOffset = c1 < 0x5f ? 0x70 : 0xb0;
  register unsigned char cellOffset = c1 % 2 ? 0x1f + (c2 > 0x5f) : 0x7e;

  *p1 = ((c1 + (unsigned char) 1) >> 1) + rowOffset;
  *p2 = c2 + cellOffset;
}


/* -------------------charset converter functions ------------------------- */

static int
sjis_mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State)conv->state;
    XLCd lcd = state->lcd;
    XlcCharSet charset;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    int length = 0;
    register char *src = *from, *dst = *to;



    if(*from_left <= 0)
	return -1;

    if (from == NULL || *from == NULL)
	return init_state(state);

    if (dst) {
	if (iskanji(*src)) {
	    charset = *CS1->charset_list;
	    *dst++ = *src++;		/* 1st byte of 2-byte char    */
	}
	else if (isuserdef(*src)) {
	    charset = *CS3->charset_list;
	    *dst++ = *src++;		/* 1st byte of 2-byte char    */
	}
	else if (isascii(*src))
	    charset = *CS0->charset_list;
	else if (iskana(*src)) {
	    charset = *CS2->charset_list;
	}
	else if (isundef(*src))	 	/* all bets are off if we can't */
	    charset = NULL;		/* classify what we have        */

	if (charset == NULL)		/* undefined char or other failure */
	    return -1;

	*dst++ = *src++;
	*to = dst;

	length = charset->char_size; 
	*to_left -= length;
    }

    *from = src;
    *from_left -= length;

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    return 0;
}


static int
sjis_mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State)conv->state;
    XLCd lcd = state->lcd;
    char *tmp_from, *tmp_to;
    int tmp_from_left, tmp_to_left;
    XlcCharSet charset, tmp_charset;
    XPointer tmp_args[1];
    int unconv_num = 0, ret;



    if (from == NULL || *from == NULL)
	return init_state(state);

    tmp_args[0] = (XPointer) &charset; /* set charset in calling function */

    if((ret = sjis_mbtocs(conv, from, from_left, to, to_left, tmp_args, 1)< 0))
	return ret;
    unconv_num += ret;

    tmp_from = *from;
    tmp_from_left = *from_left;
    tmp_to = *to;
    tmp_to_left = *to_left;


    tmp_args[0] = (XPointer) &tmp_charset;
    while (1) {
	ret = sjis_mbtocs(conv, &tmp_from, &tmp_from_left, &tmp_to,
		     &tmp_to_left, tmp_args, 1);

	if (ret > 0) {
	    unconv_num += ret;
	    continue;
	}

	if (ret < 0 || tmp_charset != charset)
	    break;
	
	*from = tmp_from;
	*from_left = tmp_from_left;
	*to = tmp_to;
	*to_left = tmp_to_left;
    }

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;
    
    return unconv_num;
}


static CodeSet
wc_parse_codeset(lcd, wcstr)
    XLCd lcd;
    wchar_t *wcstr;
{
    register CodeSet *codeset;
    unsigned long wc_encoding;
    register int num;

    wc_encoding = *wcstr & XLC_GENERIC(lcd, wc_encode_mask);
    num = XLC_GENERIC(lcd, codeset_num);
    codeset = XLC_GENERIC(lcd, codeset_list);
    while (num-- > 0) {
	if (wc_encoding == (*codeset)->wc_encoding)
	    return *codeset;
	codeset++;
    }

    return NULL;
}


static int
sjis_wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State)conv->state;
    XLCd lcd = state->lcd;
    wchar_t *wcptr = (wchar_t *) *from;
    register char *bufptr = (char *) *to;
    wchar_t wch;
    char *tmpptr;
    register length;
    CodeSet codeset;
    unsigned long wc_encoding;
    int wcstr_len = *from_left, buf_len = *to_left;
    int kanjiflg = 0;
    char *hi, *lo;



    if (from == NULL || *from == NULL)
	return init_state(state);

    codeset = wc_parse_codeset(lcd, wcptr);
    wc_encoding = codeset->wc_encoding;

    if (wcstr_len < buf_len / codeset->length)
	buf_len = wcstr_len * codeset->length;

    for ( ; wcstr_len > 0 && buf_len > 0; wcptr++, wcstr_len--) {
	wch = *wcptr;
	if ((wch & XLC_GENERIC(lcd, wc_encode_mask)) != wc_encoding)
	    break;

	length = codeset->length;

	if(length == 2)
	    kanjiflg = True;

	buf_len -= length;
	bufptr += length;

	tmpptr = bufptr - 1;
	if ((*codeset->charset_list)->side == XlcGL) {
	    while (length--) {
		*tmpptr-- = (unsigned char) BIT8OFF(wch);
		wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
	    }
	} else if ((*codeset->charset_list)->side == XlcGR) {
	    while (length--) {
		*tmpptr-- = (unsigned char) BIT8ON(wch);
		wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
	    }
	    if(kanjiflg) {
		*(tmpptr+1) = BIT8OFF(*(tmpptr+1));
		*(tmpptr+2) = BIT8OFF(*(tmpptr+2));
		jis_to_sjis(tmpptr+1, tmpptr+2);
		kanjiflg = False;
	    }
	} else {
	    while (length--) {
		*tmpptr-- = (unsigned char) wch;
		wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
	    }
	}
    }

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = *codeset->charset_list;

    *from_left -= wcptr - (wchar_t *) *from;
    *from = (XPointer) wcptr;

    *to_left -= bufptr - *to;
    *to = bufptr;

    return 0;
}



static CodeSet
_XlcGetCodeSetFromCharSet(lcd, charset)
    XLCd lcd;
    XlcCharSet charset;
{
    register CodeSet *codeset = XLC_GENERIC(lcd, codeset_list);
    register XlcCharSet *charset_list;
    register codeset_num, num_charsets;

    codeset_num = XLC_GENERIC(lcd, codeset_num);

    for ( ; codeset_num-- > 0; codeset++) {
	num_charsets = (*codeset)->num_charsets;
	charset_list = (*codeset)->charset_list;

	for ( ; num_charsets-- > 0; charset_list++)
	    if (*charset_list == charset)
		return *codeset;
    }

    return (CodeSet) NULL;
}


static int
sjis_cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State)conv->state;
    XLCd lcd = state->lcd;
    register char *csptr = *from;
    register char *bufptr = *to;
    int csstr_len = *from_left;
    register buf_len = *to_left;
    int num;
    CodeSet codeset;
    EncodingType type;
    int cvt_length;



    if (from == NULL || *from == NULL)
	return init_state(state);

    if (num_args < 1)
	return -1;
    
    if (!(codeset = _XlcGetCodeSetFromCharSet(lcd, (XlcCharSet) args[0])))
	return -1;

    cvt_length = 0;

    csstr_len /= codeset->length;
    buf_len /= codeset->length;
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    
    cvt_length += buf_len * codeset->length;

    if (bufptr) {
	while (buf_len--) {
	    num = codeset->length;
	    while (num--) {
		*bufptr++ = *csptr++;
	    }
	}
    }

    *from_left -= csptr - *from;
    *from = csptr;
    if (bufptr)
	*to += cvt_length;
    *to_left -= cvt_length;

    return 0;
}


static int
sjis_cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State)conv->state;
    XLCd lcd = state->lcd;
    register char *csptr = (char *) *from;
    wchar_t *bufptr = (wchar_t *) *to;
    int csstr_len = *from_left;
    register buf_len = *to_left;
    wchar_t wch;
    unsigned long code_mask, wc_encoding;
    int num, length, wc_shift_bits;
    CodeSet codeset;
    register CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    int cs_num;



    if (from == NULL || *from == NULL)
	return init_state(state);

    if (num_args < 1)
	return -1;

    if (!(codeset = _XlcGetCodeSetFromCharSet(lcd, (XlcCharSet) args[0])))
	return -1;

    cs_num = codeset->cs_num;
    length = codeset->length;
    csstr_len /= length;
    if (csstr_len < buf_len)
	buf_len = csstr_len;

    code_mask = XLC_GENERIC(lcd, wc_encode_mask);
    wc_encoding = codeset->wc_encoding;
    wc_shift_bits = XLC_GENERIC(lcd, wc_shift_bits);

    *to_left -= buf_len;

    if (bufptr) {
	*to += (buf_len * sizeof(wchar_t)); /* sizeof(): for wchar_t offset */
	while (buf_len--) {

	    if ( cs_num == CS1->cs_num) 	/* iskanji(*csptr) */
		sjis_to_jis(csptr, csptr+1);

	    wch = (wchar_t) BIT8OFF(*csptr);
	    csptr++;

	    num = length - 1;
	    while (num--) {
		wch = (wch << wc_shift_bits) | BIT8OFF(*csptr);
		csptr++;
	    }
	    *bufptr++ = wch | wc_encoding;
	}
    }

    *from_left -= csptr - *from;
    *from = csptr;

    return 0;
}


static int
sjis_convert(lc_conv, from, from_left, to, to_left, args, num_args)
    XlcConv lc_conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    Conv conv = (Conv) lc_conv->state;
    XlcConv from_conv = conv->from_conv;
    XlcConv to_conv = conv->to_conv;
    XlcCharSet charset;
    char buf[BUFSIZE], *cs;
    XPointer tmp_args[1];
    int cs_left, ret, length, unconv_num = 0;


    if (from == (XPointer *) NULL || *from == (XPointer) NULL) {
	ret = (*from_conv->methods->convert)(from_conv, NULL, NULL, NULL, NULL,
					     NULL, 0);
	if (ret < 0)
	    return ret;

	return (*to_conv->methods->convert)(to_conv, NULL, NULL, NULL, NULL,
					    NULL, 0);
    }

    while (*from_left > 0) {

	cs = buf;
	cs_left = BUFSIZE;
	tmp_args[0] = (XPointer) &charset;
	ret = (*from_conv->methods->convert)(from_conv, from, from_left, &cs,
					     &cs_left, tmp_args, 1);
	if (ret < 0)
	    break;

	length = cs_left = cs - buf;
	cs = buf;
	tmp_args[0] = (XPointer)charset;
	ret = (*to_conv->methods->convert)(to_conv, &cs, &cs_left, to, to_left,
					   tmp_args, 1);
	if (ret < 0) {
	    unconv_num += length / charset->char_size;
	    continue;
	}
	
	if (*to_left < 1)
	    break;
    }

    return unconv_num;
}


static int
init_state(state)
    register State state;
{
    register XLCdGenericPart *private = XLC_GENERIC_PART(state->lcd);
    register CodeSet codeset;

    if (codeset = private->initial_state_GL)
	state->GL_charset = *codeset->charset_list;

    if (codeset = private->initial_state_GR)
	state->GR_charset = *codeset->charset_list;

    if (state->GL_charset == NULL)
	if (codeset = *private->codeset_list)
	    state->GL_charset = *codeset->charset_list;

    return 0;
}


static void
close_converter(conv)
    XlcConv conv;
{
    if (conv->state) {
        _XlcFree((char *) conv->state);
    }
     _XlcFree((char *) conv);
}


/* BEGIN: Utility Functions for sjis_cttocs() and sjis_cstoct(): */

/* NOTE:  If sjis_cttocs() and sjis_cstoct are found to be general enough
 *        they should replace cttocs() and cstocs() in XlcCT.c.  If such
 *        is the happy case, this section may be deleted, since the following
 *        functions are duplicates of those in XlcCT.c.
 */


typedef struct _CTDataRec {
    char *name;
    char *encoding;		/* Compound Text encoding */
} CTDataRec, *CTData;

typedef struct _CTInfoRec {
    XlcCharSet charset;
    int encoding_len;
    char *encoding;		/* Compound Text encoding */
    int ext_segment_len;
    char *ext_segment;		/* extended segment */
    int set_size;		/* num of graphic characters */
} CTInfoRec, *CTInfo;

static CTDataRec default_ct_data[] =
{
    { "ISO8859-1", "\033(B" },
    { "ISO8859-1", "\033-A" },
    { "ISO8859-2", "\033-B" },
    { "ISO8859-3", "\033-C" },
    { "ISO8859-4", "\033-D" },
    { "ISO8859-7", "\033-F" },
    { "ISO8859-6", "\033-G" },
    { "ISO8859-8", "\033-H" },
    { "ISO8859-5", "\033-L" },
    { "ISO8859-9", "\033-M" },
    { "JISX0201.1976-0", "\033(J" },
    { "JISX0201.1976-0", "\033)I" },

    { "GB2312.1980-0", "\033$(A" },
    { "GB2312.1980-0", "\033$)A" },
    { "JISX0208.1983-0", "\033$(B" },
    { "JISX0208.1983-0", "\033$)B" },
    { "KSC5601.1987-0", "\033$(C" },
    { "KSC5601.1987-0", "\033$)C" },
    { "JISX0212.1990-0", "\033$(D" },
    { "JISX0212.1990-0", "\033$)D" },
    { "CNS11643.1986-0", "\033$(G" },
    { "CNS11643.1986-1", "\033$(H" },

    /* Non-Standard Character Set Encodings */
    { "TIS620.2533-1", "\033-T"},
} ; 

#define XctC0		0x0000
#define XctHT		0x0009
#define XctNL		0x000a
#define XctESC		0x001b
#define XctGL		0x0020
#define XctC1		0x0080
#define XctCSI		0x009b
#define XctGR		0x00a0

#define XctCntrlFunc	0x0023
#define XctMB		0x0024
#define XctOtherCoding	0x0025
#define XctGL94		0x0028
#define XctGR94		0x0029
#define XctGR96		0x002d
#define XctNonStandard	0x002f
#define XctIgnoreExt	0x0030
#define XctNotIgnoreExt	0x0031
#define XctLeftToRight	0x0031
#define XctRightToLeft	0x0032
#define XctDirection	0x005d
#define XctDirectionEnd	0x005d

#define XctGL94MB	0x2428
#define XctGR94MB	0x2429
#define XctExtSeg	0x252f
#define XctOtherSeg	0x2f00

#define XctESCSeq	0x1b00
#define XctCSISeq	0x9b00

#define SKIP_I(str)	while (*(str) >= 0x20 && *(str) <=  0x2f) (str)++;
#define SKIP_P(str)	while (*(str) >= 0x30 && *(str) <=  0x3f) (str)++;

typedef struct {
    XlcSide side;
    int char_size;
    int set_size;
    XlcCharSet charset;
    int ext_seg_length;
    int version;
} CTParseRec, *CTParse;

static CTInfo ct_list = NULL;
static ct_list_num = 0;

static XlcCharSet
_XlcGetCharSetFromEncoding(encoding, length)
    register char *encoding;
    register int length;
{
    register CTInfo ct_info;
    register int i;

    for (ct_info = ct_list, i = 0; i < ct_list_num; ct_info++, i++) {
	if (length >= ct_info->encoding_len &&
	    !strncmp(ct_info->encoding, encoding, ct_info->encoding_len))
	    return ct_info->charset;
    }

    return (XlcCharSet) NULL;
}

static unsigned int
_XlcParseCT(parse, text, length)
    register CTParse parse;
    char **text;
    int *length;
{
    unsigned int ret = 0;
    unsigned char ch;
    register unsigned char *str = (unsigned char *) *text;


    switch (ch = *str++) {
	case XctESC:
	    if (*str == XctOtherCoding && *(str + 1) == XctNonStandard
		&& *(str + 2) >= 0x30 && *(str + 2) <= 0x3f && *length >= 6) {

		/* non-standard encodings */
		parse->side = XlcGLGR;
		parse->set_size = 0;
		str += 2;
		if (*str <= 0x34) {
		    parse->char_size = *str - 0x30;
		    ret = XctExtSeg;
		    /* XXX */
		    parse->charset = _XlcGetCharSetFromEncoding(*text, *length);
		} else
		    ret = XctOtherSeg;
		str++;
		parse->ext_seg_length = (*str - 128) * 128 + *(str + 1) - 128;
		str += 2;

		goto done;
	    } else if (*str == XctCntrlFunc && *length >= 4 &&
		       *(str + 1) >= 0x20 && *(str + 1) <= 0x2f &&
		       (*(str + 2) == XctIgnoreExt ||
			*(str + 2) == XctNotIgnoreExt)) {
		
		/* ignore extension or not */
		str++;
		parse->version = *str++ - 0x20;
		ret = *str++;

		goto done;
	    }
	    
	    if (*str == XctMB) {	/* multiple-byte sets */
		parse->char_size = 2;
		str++;
	    } else
		parse->char_size = 1;
	
	    switch (*str) {
		case XctGL94:
		    parse->side = XlcGL;
		    parse->set_size = 94;
		    ret = (parse->char_size == 1) ? XctGL94 : XctGL94MB;
		    break;
		case XctGR94:
		    parse->side = XlcGR;
		    parse->set_size = 94;
		    ret = (parse->char_size == 1) ? XctGR94 : XctGR94MB;
		    break;
		case XctGR96:
		    if (parse->char_size == 1) {
			parse->side = XlcGR;
			parse->set_size = 96;
			ret = XctGR96;
		    }
		    break;
	    }
	    if (ret) {
		str++;
		if (*str >= 0x24 && *str <= 0x2f) {	/* non-standard */
		    ret = 0;
		    str++;
		}
	    }

	    SKIP_I(str)

	    if (ret && *str < 0x40)			/* non-standard */
		ret = 0;

	    if (*str < 0x30 || *str > 0x7e || (char *) str - *text >= *length)
		break;
	    
	    if (ret == 0)
		ret = XctESCSeq;
	    else {
		if (parse->char_size == 2) {
		    if (*str >= 0x70)
			parse->char_size = 4;
		    else if (*str >= 0x60)
			parse->char_size = 3;
		}
		parse->charset = _XlcGetCharSetFromEncoding(*text, *length);
	    }
	    str++;
	    goto done;
	case XctCSI:
	    /* direction */
	    if (*str == XctLeftToRight && *(str + 1) == XctDirection) {
		ret = XctLeftToRight;
		str += 2;
		goto done;
	    } else if (*str == XctRightToLeft && *(str + 1) == XctDirection) {
		ret = XctRightToLeft;
		str += 2;
		goto done;
	    } else if (*str == XctDirectionEnd) {
		ret = XctDirectionEnd;
		str++;
		goto done;
	    }

	    SKIP_P(str)
	    SKIP_I(str)

	    if (*str < 0x40 && *str > 0x7e)
		break;

	    ret = XctCSISeq;
	    str++;
	    goto done;
    }

    if (ch & 0x80) {
	if (ch < 0xa0)
	    ret = XctC1;
	else
	    ret = XctGR;
    } else {
	if (ch == XctHT || ch == XctNL)
	    ret = ch;
	else if (ch < 0x20)
	    ret = XctC0;
	else
	    ret = XctGL;
    }

    return ret;

done:
    *length -= (char *) str - *text;
    *text = (char *) str;

    return ret;
}


static XlcCharSet
_XlcAddCT(name, encoding)
    char *name;
    char *encoding;
{
    CTInfo ct_info;
    XlcCharSet charset;
    CTParseRec parse;
    char *ct_ptr = encoding;
    char *ext_segment = NULL;
    int length;

    length = strlen(encoding);

    switch (_XlcParseCT(&parse, &ct_ptr, &length)) {
	case XctExtSeg:
	    /* XXX */
	    ext_segment = name;
	case XctGL94:
	case XctGL94MB:
	case XctGR94:
	case XctGR94MB:
	case XctGR96:
	    if (parse.charset)		/* existed */
		return parse.charset;
	    break;
	default:
	    return (XlcCharSet) NULL;
    }


    charset = _XlcCreateDefaultCharSet(name, encoding);

    if (charset == (XlcCharSet) NULL)
	return (XlcCharSet) NULL;

    _XlcAddCharSet(charset);

    if (ct_list == NULL)
	ct_info = (CTInfo) Xmalloc(sizeof(CTInfoRec));
    else
	ct_info = (CTInfo) Xrealloc(ct_list, sizeof(CTInfoRec) *
				       (ct_list_num + 1));
    if (ct_info == NULL)
	return (XlcCharSet) NULL;
    
    ct_list = ct_info;
    ct_info += ct_list_num++;

    ct_info->charset = charset;
    ct_info->encoding_len = strlen(encoding);
    ct_info->encoding = encoding;
    ct_info->ext_segment_len = ext_segment ? strlen(ext_segment) : 0;
    ct_info->ext_segment = ext_segment;
    ct_info->set_size = parse.set_size;

    return charset;
}

static CTInfo
_XlcGetCTInfoFromCharSet(charset)
    register XlcCharSet charset;
{
    register CTInfo ct_info;
    register int i;

    for (ct_info = ct_list, i = 0; i < ct_list_num; ct_info++, i++)
	if (ct_info->charset == charset)
	    return ct_info;

    return (CTInfo) NULL;
}

static Bool
_XlcInitCTInfo()
{
    register XlcCharSet charset;
    register CTData ct_data;
    register int num;

    if (ct_list == NULL) {
	num = sizeof(default_ct_data) / sizeof(CTDataRec);
	for (ct_data = default_ct_data; num-- > 0; ct_data++) {
	    charset = _XlcAddCT(ct_data->name, ct_data->encoding);
	    if (charset == NULL)
		continue;
	}
    }

    return True;
}


static int
_XlcCheckCTSequence(state, ctext, ctext_len)
    State state;
    char **ctext;
    int *ctext_len;
{
    XlcCharSet charset;
    CTParseRec parse;

    switch (_XlcParseCT(&parse, ctext, ctext_len)) {
	case XctExtSeg:
	    /* XXX */
	case XctGL94:
	case XctGL94MB:
	case XctGR94:
	case XctGR94MB:
	case XctGR96:
	    charset = parse.charset;
	    break;
	default:
	    /* XXX */
	    return 0;
    }

    if (charset == (XlcCharSet) NULL)
	return -1;      /* XXX  was 0????!!! */

    if (charset->side == XlcGL)
	state->GL_charset = charset;
    else if (charset->side == XlcGR)
	state->GR_charset = charset;

    return 0 ;
}

/* END: Utility Functions for sjis_cttocs() and sjis_cstoct(): */


/* sjis_cttocs() - convert compound text to charset (SJIS version)
 *
 * cttocs() processes strings containing one or more compound text (CT) 
 * segments.  On first invocation it processes the input string (from_left)
 * until it detects a new CT sequence or the end of the string.  When this
 * condition occurs, cttocs() passes the charset of the first segment via
 * args[0] and saves the charset of the next segment in state->charset.
 * Upon reinvocation, the ELSE part of the while loop is executed because
 * from_left does not begin with a CT sequence.  Because charset is now
 * NULL, cttocs() reinitializes charset and tmpcharset and the process
 * continues.
 */

static int
sjis_cttocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register State state = (State) conv->state;
    register unsigned char ch;
    XlcCharSet charset = NULL;
    XlcCharSet tmpcharset = NULL;
    char *ctptr, *bufptr;
    int ctext_len, buf_len;

    XLCd lcd = state->lcd;
    register XLCdGenericPart *private = XLC_GENERIC_PART(state->lcd);
    register CodeSet codeset;



    if (from == NULL || *from == NULL)
	return init_state(state);
    
    ctptr = *from;
    bufptr = *to;
    ctext_len = *from_left;
    buf_len = *to_left;

    while (ctext_len > 0 && buf_len > 0) {
	ch = *((unsigned char *) ctptr);

	if (ch == 0x1b || ch == 0x9b) {    /* Process CT sequence */

	    if (tmpcharset)
		tmpcharset = charset;
/*
 * _XlcCheckCTSequence() identifies the charset of the CT sequence and points
 * either state->GL_charset or state->GR_charset at the charset information.
 * Only one of {GL,GR}_charset will be set.  _XlcCheckCTSequence() also sets
 * ctptr to point to the first byte after the CT sequence.
 */
	    state->GL_charset = state->GR_charset = (XlcCharSet) NULL;

	    if ( _XlcCheckCTSequence(state, &ctptr, &ctext_len) == -1)
		return -1;

	    if (state->GL_charset)
		charset = state->GL_charset;
	    else if (state->GR_charset)
		charset = state->GR_charset;
	    else
		return -1;
	    state->charset = charset;

	    if (!tmpcharset)	        /* First invocation: initialize */
		tmpcharset = charset;

	} else {    /* Process remaining segment (character data) */

	    if ((ch < 0x20 && ch != '\n' && ch != '\t'))
		return -1;
	    
	    if (!charset) {	       /* Subsequent invocation: reinitialize. */
		if (!state->charset){  /* Continuation of Codeset 0 segment    */
		      init_state(state);
		      state->charset = state->GL_charset;
		}
		charset = state->charset; 
		tmpcharset = charset;
	    }
	    else if (tmpcharset != charset) {
		state->charset = charset;
		charset = tmpcharset;
		break;
	    }
	    *bufptr++ = *ctptr++;
	    ctext_len--;
	    buf_len--;
	}
    }

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    *from_left -= ctptr - *from;
    *from = ctptr;

    *to_left -= bufptr - *to;
    *to = bufptr;


    return 0;
}

static int
sjis_cstoct(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XlcSide side;
    unsigned char min_ch, max_ch;
    register unsigned char ch;
    int length, set_size, cvt_length;
    CTInfo ct_info;
    XlcCharSet charset;
    char *csptr, *ctptr;
    int csstr_len, ct_len;

    if (from == NULL || *from == NULL)
	return init_state(state);
    
    if (num_args < 1)
	return -1;
    
    csptr = *from;
    ctptr = *to;
    csstr_len = *from_left;
    ct_len = *to_left;
    
    charset = (XlcCharSet) args[0]; /* charset of current segment */


    ct_info = _XlcGetCTInfoFromCharSet(charset);

    if (ct_info == NULL)
	return -1;

    side = charset->side;
    length = charset->char_size;
    set_size = ct_info->set_size;

    cvt_length = 0;

/*
 * state->charset == charset of previous csoct() invocation 
 */
    if (ct_info->ext_segment || state->charset != charset ) {
	ct_len -= ct_info->encoding_len;

	if (ct_len < 0)
	    return -1;
	cvt_length += ct_info->encoding_len;

	if (ctptr) {
	    strcpy(ctptr, ct_info->encoding);
	    ctptr += ct_info->encoding_len;
	}
    }

    min_ch = 0x20;
    max_ch = 0x7f;

    if (set_size == 94) {
	max_ch--;
	if (length > 1 || side == XlcGR)
	    min_ch++;
    }

    while (csstr_len > 0 && ct_len > 0) {
	ch = *((unsigned char *) csptr++);

	if (ch < min_ch &&
	     (ch != 0x00 && ch != 0x09 && ch != 0x0a && ch != 0x0b))
		continue;
	cvt_length++;

	*ctptr++ = ch;
	csstr_len--;
	ct_len--;
    }

    state->charset = charset;

    *from_left -= csptr - *from;
    *from = csptr;

    if (ctptr)
	*to += cvt_length;
    *to_left -= cvt_length;

    return 0;
}


typedef enum {
    XlcNoneType,
    XlcMultiByteType,
    XlcWideCharType,
    XlcCompoundTextType,
    XlcStringType,
    XlcCharSetType,
    XlcCharType
} XlcConvType;


static int
get_conversion_type(type)
    char *type;
{
    if (strcmp(XlcNMultiByte, type) == 0)
	return XlcMultiByteType;

    if (strcmp(XlcNWideChar, type) == 0)
	return XlcWideCharType;

    if (strcmp(XlcNCompoundText, type) == 0)
	return XlcCompoundTextType;

    if (strcmp(XlcNString, type) == 0)
	return XlcStringType;

    if (strcmp(XlcNCharSet, type) == 0)
	return XlcCharSetType;

    if (strcmp(XlcNChar, type) == 0)
	return XlcCharType;
    
    return XlcNoneType;
}


static XlcConv
open_converter(from_lcd, from, to_lcd, to)
XLCd from_lcd;
char *from;
XLCd to_lcd;
char *to;
{

/*
 * This routine sets up the data structures to invoke the appropriate
 * functions and to pass locale data for the requested conversion.
 *
 * There are two types of converters:
 *   single-hop (source->destination)
 *   double-hop (source->charset->destination)
 *
 * Each converter type has its own set of data structures (prefixed by
 * "sgl" (single) and "dbl" (double)).
 */

    static XlcConvRec sglconvrec;
    static XlcConvMethodsRec sglconv_methods;
    static XlcConvMethodsRec dblfrom_convrec;
    static XlcConvMethodsRec dblto_convrec;

    static XlcConvMethodsRec dbl_conv_methods = {
	close_converter,
	sjis_convert,
    };

    register XlcConv sglconv = &sglconvrec;
    register XlcConv dblconv = (XlcConv) NULL;
    register Conv dbl_conv;
    register State state;

    XlcConvType from_type, to_type;


    if ( ! (
      (dbl_conv = (Conv)_XlcAlloc(sizeof(ConvRec)))			&&
      (dbl_conv->from_conv = (XlcConv)_XlcAlloc(sizeof(XlcConvRec)))	&&
      (dbl_conv->to_conv = (XlcConv)_XlcAlloc(sizeof(XlcConvRec)))	&&
      (dblconv = (XlcConv)_XlcAlloc(sizeof(XlcConvRec)))		&&
      (state = (State)_XlcAlloc(sizeof(StateRec)))        	       ) )
	goto err;

    state->lcd = from_lcd;

    sglconv->methods = &sglconv_methods;
    sglconv->methods->close = close_converter;
    sglconv->state = (XPointer)from_lcd;

    dbl_conv->to_conv->methods = &dblto_convrec;
    dbl_conv->to_conv->methods->close = close_converter;
    dbl_conv->to_conv->state = (XPointer)state;
    dbl_conv->from_conv->state = (XPointer)state;
    dbl_conv->from_conv->methods = &dblfrom_convrec;
    dbl_conv->from_conv->methods->close = close_converter;

    dblconv->methods = &dbl_conv_methods;
    dblconv->state = (XPointer)dbl_conv;

    from_type = get_conversion_type(from);
    to_type = get_conversion_type(to);



/* Note:  For simplicity, cases XlcStringType/XlcCompoundTextType are 
 * combined.  If invalid arguments such as _XctOpenConverter(XlcNString,
 * XlcNString) are passed, _XctOpenConverter() returns NULL, causing
 * open_converter() to return (XlcConv)NULL.
 */

    switch (from_type) {
	case XlcStringType:
	    switch (to_type) {
#ifdef notdef
		case XlcCompoundTextType:
		    sglconv->methods =
		      _XctOpenConverter(from, XlcNCompoundText)->methods;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		case XlcCharType:
		case XlcCharSetType:
		    sglconv->methods =
		      _XctOpenConverter(from, XlcNCharSet)->methods;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
#endif
		default:
		    goto err;
	    }
	    break;
	case XlcCompoundTextType:
	    switch (to_type) {
#ifdef notdef
		case XlcStringType:
		    sglconv->methods =
		      _XctOpenConverter(from, XlcNString)->methods;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		case XlcCompoundTextType:
		    dbl_conv->from_conv->methods = _XctOpenConverter(from,
		      XlcNCharSet)->methods;
		    dbl_conv->to_conv->methods->convert = sjis_cstoct;
		    if (dbl_conv->from_conv->methods == (XlcConvMethods) NULL
		      || dbl_conv->to_conv->methods == (XlcConvMethods) NULL)
			goto err;
		    break;
#endif
		case XlcMultiByteType:
		    dbl_conv->from_conv->methods->convert = sjis_cttocs;
		    if (dbl_conv->from_conv->methods == (XlcConvMethods) NULL)
			goto err;
		    dbl_conv->to_conv->methods->convert = sjis_cstombs;
		    break;
		case XlcWideCharType:
		    dbl_conv->from_conv->methods->convert = sjis_cttocs;
		    if (dbl_conv->from_conv->methods == (XlcConvMethods) NULL)
			goto err;
		    dbl_conv->to_conv->methods->convert = sjis_cstowcs;
		    break;
#ifdef notdef
		case XlcCharType:
		case XlcCharSetType:
		    sglconv->methods->convert = sjis_cttocs;
		      _XctOpenConverter(from, to)->methods->convert;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
#endif
		default:
		    goto err;
	    }
	    return dblconv;
	case XlcMultiByteType:
	    switch (to_type) {
		case XlcWideCharType:
		    sglconv->methods->convert = sjis_mbstowcs;
		    sglconv->state = (XPointer)from_lcd;
		    return sglconv;
		case XlcCompoundTextType:
		    dbl_conv->from_conv->methods->convert = sjis_mbstocs;
		    dbl_conv->to_conv->methods->convert = sjis_cstoct;
		    if (dbl_conv->to_conv->methods == (XlcConvMethods) NULL)
			goto err;
		    break;	
		case XlcCharType:
		case XlcCharSetType:
		    sglconv->methods->convert = sjis_mbstocs;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		default:
		    goto err;
	    }
	    return dblconv;
	case XlcWideCharType:
	    switch (to_type) {
		case XlcMultiByteType:
		    sglconv->methods->convert = sjis_wcstombs;
		    sglconv->state = (XPointer)from_lcd;
		    return sglconv;
		case XlcCompoundTextType:
		    dbl_conv->from_conv->methods->convert = sjis_wcstocs;
		    dbl_conv->to_conv->methods->convert = sjis_cstoct;
		    if (dbl_conv->to_conv->methods == (XlcConvMethods) NULL)
			goto err;
		    break;
		case XlcCharType:
		case XlcCharSetType:
		    sglconv->methods->convert = sjis_wcstocs;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		default:
		    goto err;
	    }
	    return dblconv;
	case XlcCharType:
	case XlcCharSetType:
	    switch (to_type) {
		case XlcMultiByteType:
		    sglconv->methods->convert = sjis_cstombs;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		case XlcWideCharType:
		    sglconv->methods->convert = sjis_cstowcs;
		    sglconv->state = (XPointer)state;
		    return sglconv;
#ifdef notdef
		case XlcStringType:
		    sglconv->methods =
		      _XctOpenConverter(from, XlcNString)->methods;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
#endif
		case XlcCompoundTextType:
		    sglconv->methods->convert = sjis_cstoct;
		    if (sglconv->methods->convert == (int) NULL)
			goto err;
		    sglconv->state = (XPointer)state;
		    return sglconv;
		default:
		    goto err;
	    }
	    break;
    }

err:
    close_converter(dbl_conv, dblconv, state);
    return (XlcConv) NULL;
}


XLCd
_XlcSjisLoader(name)
    char *name;
{
    XLCd lcd;

    lcd = _XlcCreateLC(name, _xlcGenericMethods);
    if ( ! lcd || ! _XlcInitCTInfo() )
	return (XLCd) NULL;
    
    if ((_XlcCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "sjis"))) {
	_XlcDestroyLC(lcd);
	return (XLCd) NULL;
    }
    
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNWideChar, open_converter);

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_converter);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCompoundText, open_converter);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_converter);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_converter);

    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCompoundText, open_converter);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_converter);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNChar, open_converter);

    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_converter);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNCompoundText, open_converter);

    _XlcSetConverter(lcd, XlcNChar, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNChar, lcd, XlcNWideChar, open_converter);
    _XlcSetConverter(lcd, XlcNChar, lcd, XlcNCompoundText, open_converter);

    return lcd;
}
