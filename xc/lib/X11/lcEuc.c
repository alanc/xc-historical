/* $XConsortium: lcEuc.c,v 1.6 94/01/20 18:06:38 rws Exp $ */
/******************************************************************

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

    Authors: Shigeru Yamada 		(yamada@ossi.com)
             Jeffrey Bloomfield		(jeffb@ossi.com)

*****************************************************************/

#include "Xlibint.h"
#include "XlcGeneric.h"


#include <ctype.h>
#ifdef WIN32
#define isascii __isascii
#endif


#define CS0     codesets[0]             /* Codeset 0 - 7-bit ASCII      */
#define CS1     codesets[1]             /* Codeset 1 - Kanji            */
#define CS2     codesets[2]             /* Codeset 2 - Half-Kana        */
#define CS3     codesets[3]             /* Codeset 3 - User defined     */

#define SS2	0x8e	/* Single-shift char: CS2 */
#define SS3	0x8f    /* Single-shift char: CS3 */

#define GR	0x80	/* begins right-side (non-ascii) region */
#define GL	0x7f    /* ends left-side (ascii) region        */

#define isleftside(c)	(((c) & GR) ? 0 : 1)
#define isrightside(c)	(!isleftside(c))

#define BIT8OFF(c)	((c) & GL)
#define BIT8ON(c)	((c) | GR)

typedef unsigned char   Uchar;
typedef unsigned long   Ulong;
typedef unsigned int	Uint;

static CodeSet GetCodeSetFromCharSet();
static CodeSet wc_codeset();


static int
euc_mbstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{

    XLCd lcd = (XLCd)conv->state;

    register Uchar ch;
    register int chr_len = 0;
    register int sshift =  False;
    register int shift_mult = 0;
    register Uint chrcode;

    Uint wc_encode = 0;
    Uint wc_tmp = 0;

    int cs0flg = False;
    int cs1flg = False;
    int length;
    int num_conv;
    int unconv_num = 0;

    Bool new_char;

    XPointer inbufptr = *from;
    wchar_t *outbufptr = (wchar_t *) *to;
    wchar_t *outbuf_base = outbufptr;

    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list); 
    Ulong wc_shift = XLC_GENERIC(lcd, wc_shift_bits);


    if (*from_left > *to_left)
	*from_left = *to_left;

    for (new_char = True; *from_left > 0;) {

	ch = *inbufptr++;

	if (isleftside(ch)) {				/* CS0 */
	    if( cs0flg == True) {
		new_char = True;
		cs0flg = False;
	    }
	    length = CS0->length;
	    *outbufptr++ = (wchar_t)ch;
	    (*from_left)--;
	    continue;
	}
	else if (ch == SS2) {				/* CS2 */
	    if (sshift == True || cs1flg == True) {
		cs1flg = False;
		unconv_num++;
		continue;
	    }
	    length = CS2->length;
	    wc_encode = CS2->wc_encoding;
	    chrcode = 0;
	    sshift = True;
	    cs0flg = True;
	    (*from_left)--;
	    continue;
	}
	else if (ch == SS3) {				/* CS3 */
	    if (sshift == True || cs1flg == True) {
		cs1flg = False;
		unconv_num++;
		continue;
	    }
	    length = CS3->length;
	    if (*from_left < 1 )
		unconv_num++;
	    wc_encode = CS3->wc_encoding;
	    chrcode = 0;
	    sshift = True;
	    cs0flg = True;
	    (*from_left)--;
	    continue;

	} else {					/* CS1 */
	    if (sshift == False) {
		length = CS1->length;
		if (*from_left < 1)
		    unconv_num++;
		wc_encode = CS1->wc_encoding;
	    }
	    chrcode = BIT8OFF(ch);
	    cs0flg = True;
	    cs1flg = True;
	    (*from_left)--;
	}

	if (new_char) {			/* begin new character */
	    chr_len = length;
	    shift_mult = length - 1;
	    new_char = False;
	}

	chrcode <<= (wc_shift * shift_mult);
	shift_mult--;
	wc_tmp |= chrcode; 

	if (--chr_len == 0) {
	    wc_tmp |= wc_encode;
	    *outbufptr++ = wc_tmp;

	    new_char = True;
	    sshift = False;
	    cs0flg = False;
	    cs1flg = False;
	    wc_tmp  = (Uint)0;
	}

    }	/* end for */

    *to = (XPointer)outbufptr;

    if (cs0flg == True || cs1flg == True)	/* error check on last char */
	unconv_num++;

    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0) {
        *to_left = (*to_left) - num_conv;
        return unconv_num;
    }

    return -1;
}


static int
euc_wcstombs(conv, from, from_left, to, to_left, args, num_args)
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
    XPointer outbuf_base = outbufptr;
    wchar_t  wch;
    register length;
    Uchar tmp;
    int num_conv;

    XLCd lcd = (XLCd)conv->state;
    CodeSet codeset;
    Ulong wc_encoding_mask = XLC_GENERIC(lcd, wc_encode_mask);
    Ulong wc_shift = XLC_GENERIC(lcd, wc_shift_bits);
    Ulong wch_encode;


    if (*from_left > *to_left)
        *from_left = *to_left;

    for (; *from_left > 0 ; (*from_left)-- ) {

	wch = *inbufptr++;

	if (!(codeset = wc_codeset(lcd, wch)))
	    return -1;

	if (codeset->parse_info)	/* put out SS2 or SS3 */
	    *outbufptr++ = *codeset->parse_info->encoding;

	length = codeset->length;

	wch ^= (wchar_t)codeset->wc_encoding;

	do {
	    length--;
	    tmp = (wch>>(wchar_t)(length * wc_shift));

	    if (codeset->side == XlcGR)
		tmp = BIT8ON(tmp);

	    *outbufptr++ = (Uchar)tmp;
	} while (length);
    }

    *to = (XPointer)outbufptr;

    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0) {
	*to_left -= num_conv;
	return 0;
    }

    return -1;
}


static int
euc_mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    XlcCharSet charset;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    int length;
    register char *src = *from, *dst = *to;


    if (dst) {
	if (isleftside(*src)) { 			/* 7-bit (CS0) */
   	    charset = *CS0->charset_list;
	}
	else if ((Uchar)*src == SS2) {			/* half-kana (CS2) */
	    charset = *CS2->charset_list;
	    src++;
	    (*from_left)--;
	}
	else if ((Uchar)*src == SS3) {			/* user-def */
	    charset = *CS3->charset_list;
	    src++;
	    (*from_left)--;
	}
	else  { 					/* Kanji (CS1) */
	    charset = *CS1->charset_list;
	}

	if(*from_left < charset->char_size || *to_left < charset->char_size)
	    return -1;

	length = charset->char_size;
	do {
	    switch (charset->side) {
	    case XlcGL:
		*dst++ = BIT8OFF(*src++);
		break;
	    case XlcGR:
		*dst++ = BIT8ON(*src++);
		break;
	    default:
		*dst++ = *src++;
		break;
	    }
	} while (--length);

	*to = dst;
	*from = src;
	*from_left -= charset->char_size;
	*to_left -= charset->char_size;
      } 

    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    return 0;
}


static int
euc_mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    char *tmp_from, *tmp_to;
    int tmp_from_left, tmp_to_left;
    XlcCharSet charset, tmp_charset;
    XPointer tmp_args[1];
    int unconv_num = 0, ret;


/* Determine the charset of the segment and convert one characater: */

    tmp_args[0] = (XPointer) &charset; /* charset from euc_mbtocs() */
    while
      ((ret = euc_mbtocs(conv, from, from_left, to, to_left, tmp_args, 1)) > 0)
	unconv_num += ret;
    if ( ret < 0 )
	return ret;

    tmp_from = *from;
    tmp_from_left = *from_left;
    tmp_to_left = *to_left;
    tmp_to = *to;


/* Convert remainder of the segment: */

    tmp_args[0] = (XPointer) &tmp_charset;
    while( (ret = euc_mbtocs(conv, &tmp_from, &tmp_from_left, &tmp_to,
      &tmp_to_left, tmp_args, 1)) >= 0 ) {

	if (ret > 0) {
	    unconv_num += ret;
	    continue;
	}

	if (tmp_charset != charset)  /* quit on end of segment */
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


static int
euc_wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    wchar_t *wcptr = (wchar_t *) *from;
    register char *bufptr = (char *) *to;
    wchar_t wch;
    char *tmpptr;
    register length;
    CodeSet codeset;
    Ulong wc_encoding;
    int wcstr_len = *from_left, buf_len = *to_left;


    if (!(codeset = wc_codeset(lcd, *wcptr)))
	return -1;
    wc_encoding = codeset->wc_encoding;

    if (wcstr_len < buf_len / codeset->length)
	buf_len = wcstr_len * codeset->length;

    for ( ; wcstr_len > 0 && buf_len > 0; wcptr++, wcstr_len--) {
	wch = *wcptr;

	if ((wch & XLC_GENERIC(lcd, wc_encode_mask)) != wc_encoding)
	    break;

	length = codeset->length;

	buf_len -= length;
	bufptr += length;
	tmpptr = bufptr - 1;

	while (length--) {
	    *tmpptr-- = codeset->length == 1 && codeset->side == XlcGR ?
	      BIT8ON(wch) : BIT8OFF(wch);
	    wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
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


static int
euc_cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    register char *csptr = *from;
    register char *bufptr = *to;
    int csstr_len = *from_left;
    register buf_len = *to_left;
    int length;
    CodeSet codeset;
    int cvt_length;


    if (num_args < 1)
	return -1;
    
    if (!(codeset = GetCodeSetFromCharSet(lcd, (XlcCharSet) args[0])))
	return -1;
    cvt_length = 0;

    csstr_len /= codeset->length;
    buf_len /= codeset->length;

    if (codeset->parse_info)
	csstr_len *= 2;

    if (csstr_len < buf_len)
	buf_len = csstr_len;

    cvt_length += buf_len * codeset->length;

    if (bufptr) {
	while (buf_len--) {
	    if (codeset->parse_info)	/* put out SS2 or SS3 */
		*bufptr++ = *codeset->parse_info->encoding;

	    length = codeset->length;
	    while (length--)
		*bufptr++ = codeset->side == XlcGR ?
		  BIT8ON(*csptr++) : BIT8OFF(*csptr++);
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
euc_cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    register char *csptr = *from;
    wchar_t *bufptr = (wchar_t *) *to;
    wchar_t *toptr = (wchar_t *) *to;
    int csstr_len = *from_left;
    register buf_len = *to_left;
    wchar_t wch;
    int length;
    Ulong wc_shift_bits = XLC_GENERIC(lcd, wc_shift_bits);
    CodeSet codeset;

    
    if (num_args < 1)
	return -1;
    
    if (!(codeset = GetCodeSetFromCharSet(lcd, (XlcCharSet) args[0])))
        return -1;

    csstr_len /= codeset->length;
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    *to_left -= buf_len;

    if (bufptr) {

	toptr += buf_len;
	*to = (XPointer) toptr;
	
        while (buf_len--) {

            wch = (wchar_t) BIT8OFF(*csptr);
            csptr++;

            length = codeset->length - 1;
            while (length--) {
                wch = (wch << wc_shift_bits) | BIT8OFF(*csptr);
                csptr++;
            }
            *bufptr++ = wch | codeset->wc_encoding;
        }
    }
    *from_left -= csptr - *from;
    *from = csptr;

    return 0;
}


static CodeSet
wc_codeset(lcd, wch)
    XLCd lcd;
    wchar_t wch;
{

    register CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    register int end = XLC_GENERIC(lcd, codeset_num);
    register Ulong widech = (Ulong)(wch & XLC_GENERIC(lcd, wc_encode_mask));

    for (; --end >= 0; codesets++)
	if ( widech == (*codesets)->wc_encoding )
	    return *codesets;

    return NULL;
}


static CodeSet
GetCodeSetFromCharSet(lcd, charset)
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


static XlcConv
create_conv(lcd, methods)
    XLCd lcd;
    XlcConvMethods methods;
{
    XlcConv conv;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == NULL)
	return (XlcConv) NULL;
    
    conv->methods = methods;
    conv->state = (XPointer) lcd;
    return conv;
}


/*
 *    Stripped down Direct CT converters for EUC
 *
 */

#define BADCHAR(min_ch, c)  (BIT8OFF(c) < (char)min_ch && BIT8OFF(c) != 0x0 && \
			     BIT8OFF(c) != '\t' && BIT8OFF(c) != '\n' && \
			     BIT8OFF(c) != 0x1b)


typedef struct _CTDataRec {
    int side;
    int length;
    char *name;
    Ulong wc_encoding;
    char sshift;
    char *ct_encoding;
    int ct_encoding_len;
    int set_size;
    Uchar min_ch;
} CTDataRec, *CTData;

typedef struct _StateRec {
    CTData GL_charset;
    CTData GR_charset;
    CTData charset;
} StateRec, *State;

static CTDataRec ctdata[] = /* data for direct CT converters */
{
    { XlcGL, 1, "ISO8859-1:GL",       0, 0, "\033(B" , 3, 0, 0 },
    { XlcGR, 1, "ISO8859-1:GR",       0, 0, "\033-A" , 3, 0, 0 },
    { XlcGL, 1, "JISX0201.1976-0:GL", 0, 0, "\033(J" , 3, 0, 0 },
    { XlcGR, 1, "JISX0201.1976-0:GR", 0, 0, "\033)I" , 3, 0, 0 },
    { XlcGL, 2, "JISX0208.1983-0:GL", 0, 0, "\033$(B", 4, 0, 0 },
    { XlcGR, 2, "JISX0208.1983-0:GR", 0, 0, "\033$)B", 4, 0, 0 },
    { XlcGL, 2, "JISX0212.1990-0:GL", 0, 0, "\033$(D", 4, 0, 0 },
    { XlcGR, 2, "JISX0212.1990-0:GR", 0, 0, "\033$)D", 4, 0, 0 },
};

CTData ctd_endp = ctdata + ((sizeof(ctdata) / sizeof(CTDataRec))) - 1;
static CTData ctdptr[sizeof(ctdata) / sizeof(CTDataRec)];
static enum { Ascii, Kanji, Kana, Userdef } cs_nums;


/*
 * initCTptr(): Set ctdptr[] to point at ctdata[], indexed by codeset_num.
 */

static void
initCTptr(lcd)
    XLCd lcd;
{
    int num_codesets = XLC_GENERIC(lcd, codeset_num);
    int num_charsets;
    int i, j;
    CodeSet *codesets = XLC_GENERIC(lcd, codeset_list);
    CodeSet codeset;
    XlcCharSet charset;
    CTData ctdp = ctdata;

    for (i = 0; i < num_codesets; i++) {

	codeset = codesets[i];
	num_charsets = codesets[i]->num_charsets;

	for (j = 0; j < num_charsets; j++) {

	    charset = codesets[i]->charset_list[0];

	    for (ctdp = ctdata; ctdp <= ctd_endp; ctdp++)

		if (! strcmp(ctdp->name, charset->name)) {

		    ctdptr[codeset->cs_num] = ctdp;

		    ctdptr[codeset->cs_num]->wc_encoding = codeset->wc_encoding;

		    ctdptr[codeset->cs_num]->set_size =
		      codeset->charset_list[0]->set_size;

		    ctdptr[codeset->cs_num]->min_ch =
		      charset->set_size == 94 &&
		      (ctdptr[codeset->cs_num]->length > 1 ||
		      ctdptr[codeset->cs_num]->side == XlcGR) ? 0x21 : 0x20;

		    if (codeset->parse_info) {
			ctdptr[codeset->cs_num]->sshift =
			  *codeset->parse_info->encoding;
		    }

		    break;
		}
	}
    }
}

static int
euc_ctstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XLCd lcd = (XLCd)conv->state;
    Ulong wc_shift_bits = XLC_GENERIC(lcd, wc_shift_bits);
    register XPointer inbufptr = *from;
    register wchar_t *outbufptr = (wchar_t *) *to;
    wchar_t *outbuf_base = outbufptr;
    register clen, length;
    int num_conv;
    register shift_mult;
    wchar_t wc_tmp;
    wchar_t wch;
    Ulong wc_encoding;
    CTData ctdp = ctdata;



    if (*from_left > *to_left)
	*from_left = *to_left;

    for (length = ctdata[Ascii].length; *from_left > 0; (*from_left) -= length)
    {
	if (*inbufptr == '\033') {
	    for (ctdp = ctdata; ctdp <= ctd_endp ; ctdp++) {

		if(!strncmp(inbufptr, ctdp->ct_encoding, ctdp->ct_encoding_len))
		{
		    inbufptr += ctdp->ct_encoding_len;
		    (*from_left) -= ctdp->ct_encoding_len;
		    length = ctdp->length;
		    break;
		}
	    }

	    if (ctdp > ctd_endp) 	/* failed to match CT sequence */
		return -1;
	}

	wc_encoding = (ctdp == ctdptr[Kana] && isleftside(*inbufptr)) ?
	    ctdptr[Ascii]->wc_encoding: ctdp->wc_encoding;

	shift_mult = length - 1;
	wch = (wchar_t)0;
	clen = length;

	do {
	    wc_tmp = BIT8OFF(*inbufptr++) << (wc_shift_bits * shift_mult);
	    wch |= wc_tmp;
	    shift_mult--;
	} while (--clen);

	*outbufptr++ = wch | wc_encoding;
    }

    *to = (XPointer)outbufptr;

    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0) {
	(*to_left) -= num_conv;
	return 0;
    }

    return -1;
}


#define byte1			(length == codeset->length - 1)
#define byte2			(byte1 == 0)
#define kanji			(codeset->cs_num == 1)
#define kana			(codeset->cs_num == 2)
#define userdef			(codeset->cs_num == 3)

static int
euc_wcstocts(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register ct_len = *to_left;
    register wchar_t *inbufptr = (wchar_t *) *from;
    register char *ctptr = *to;
    XPointer ct_base = ctptr;
    wchar_t  wch;
    register length;
    Uchar tmp;
    Uchar t1 = 0, t2;
    int num_conv;

    StateRec ct_state;
    XLCd lcd = (XLCd)conv->state;
    CTData charset;
    CodeSet codeset;
    Ulong wc_encoding_mask = XLC_GENERIC(lcd, wc_encode_mask);
    Ulong wc_shift = XLC_GENERIC(lcd, wc_shift_bits);



/* Initial State: */
    ct_state.GL_charset = ctdptr[0]; /* Codeset 0 */
    ct_state.GR_charset = NULL;

    if (*from_left > *to_left)
	*from_left = *to_left;

    for (; *from_left > 0 ; (*from_left)-- ) {

	wch = *inbufptr++;

	if (!(codeset = wc_codeset(lcd, wch)))
	    return -1;

	charset = ctdptr[codeset->cs_num];

	length = codeset->length;
	wch ^= (wchar_t)codeset->wc_encoding;

	if ( (charset->side == XlcGR && charset != ct_state.GR_charset) ||
	     (charset->side == XlcGL && charset != ct_state.GL_charset) ) {

	    ct_len -= ctdptr[codeset->cs_num]->ct_encoding_len;
	    if (ct_len < 0)
		return -1;
	    if (ctptr) {
		strcpy(ctptr, ctdptr[codeset->cs_num]->ct_encoding);
		ctptr += ctdptr[codeset->cs_num]->ct_encoding_len;
	    }

	}

	if (charset->side == XlcGR)
	    ct_state.GR_charset = charset;
	else if (charset->side == XlcGL)
	    ct_state.GL_charset = charset;

	do {

	    length--;
	    tmp = wch>>(wchar_t)( (Ulong)length * wc_shift);

	    if (kana) {
		if (BADCHAR(charset->min_ch, (char)tmp))
		    break;
		*ctptr++ = (char)BIT8ON(tmp);
	    }

	    else if (byte1 && (kanji || userdef))
		t1 = tmp;

	    else if (byte2 && (kanji || userdef)) {
		if (BADCHAR(charset->min_ch, (char)t1) ||
		  BADCHAR(charset->min_ch, (char)tmp))
		    break;
		*ctptr++ = (char)t1;
		*ctptr++ = (char)tmp;
	    }

	    else {
		if (BADCHAR(charset->min_ch, (char)tmp))
		    break;
		*ctptr++ = (char)tmp;
	    }


	} while (length); 

    }	/* end for */

    *to = (XPointer)ctptr;


    if ((num_conv = (int)(ctptr - ct_base)) > 0) {
	(*to_left) -= num_conv;
	return 0;
    }

    return -1;
}
#undef byte1
#undef byte2
#undef kana
#undef kanji
#undef userdef


#define byte1	(ctdp->length == clen)
#define kana    (ctdp == ctdptr[Kana] && isrightside(*inbufptr))
#define kanji   (ctdp == ctdptr[Kanji])
#define userdef (ctdp == ctdptr[Userdef])

static int
euc_ctstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register XPointer inbufptr = *from;
    register XPointer outbufptr = *to;
    XPointer outbuf_base = outbufptr;
    register clen, length;
    int num_conv;
    CTData ctdp = &ctdata[0];	/* default */



    if (*from_left > *to_left)
	*from_left = *to_left;

    for (length = ctdata[Ascii].length; *from_left > 0; (*from_left) -= length)
    {
	if (*inbufptr == '\033') {

	    for (ctdp = ctdata; ctdp <= ctd_endp ; ctdp++) {

		if(!strncmp(inbufptr, ctdp->ct_encoding, ctdp->ct_encoding_len))
		{
		    inbufptr += ctdp->ct_encoding_len;
		    (*from_left) -= ctdp->ct_encoding_len - 1;
		    length = ctdp->length;
		    break;
		}
	    }
	    if (ctdp > ctd_endp) 	/* failed to match CT sequence */
		return -1;
	}

	clen = length;
	do {

	    if (byte1)
		if (kanji) {
		    *inbufptr = BIT8ON(*inbufptr);
		    *(inbufptr+1) = BIT8ON(*(inbufptr+1));
		}
		else if (kana || userdef)
		    *outbufptr++ = ctdp->sshift;

	    *outbufptr++ = *inbufptr++;

	} while (--clen);
    }

    *to = outbufptr;

    if ((num_conv = (int)(outbufptr - outbuf_base)) > 0) {
	(*to_left) -= num_conv;
	return 0;
    }

    return -1;
}
#undef byte1
#undef kana
#undef kanji
#undef userdef


static int
euc_mbstocts(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    register ct_len = *to_left;
    int cs_num;
    int clen, length;
    int unconv_num = 0;
    int num_conv;
    XPointer inbufptr = *from;
    register char *ctptr = *to;
    XPointer ct_base = ctptr;

    StateRec ct_state;
    CTData charset;


/* Initial State: */
    ct_state.GL_charset = ctdptr[Ascii];
    ct_state.GR_charset = NULL;


    if (*from_left > *to_left)
        *from_left = *to_left;

    for (;*from_left > 0; (*from_left) -= length) {

	if (isleftside(*inbufptr)) {		/* 7-bit (CS0) */
	    cs_num = Ascii;
	    charset = ctdptr[Ascii];
	}
	else if ((Uchar)*inbufptr == SS2) {	/* Kana */
	    cs_num = Kana;
	    charset = ctdptr[Kana];
	    inbufptr++;
	    (*from_left)--;
	}
	else if ((Uchar)*inbufptr == SS3) {	/* Userdef */
	    cs_num = Userdef;
	    charset = ctdptr[Userdef];
	    inbufptr++;
	    (*from_left)--;
	}
	else {
	    cs_num = Kanji;
	    charset = ctdptr[Kanji];
	}

	length = charset->length;

	if (BADCHAR(charset->min_ch, *inbufptr))
            continue;

	if ( (charset->side == XlcGR && charset != ct_state.GR_charset) ||
	     (charset->side == XlcGL && charset != ct_state.GL_charset) ) {

	    ct_len -= ctdptr[cs_num]->ct_encoding_len;
	    if (ct_len < 0)
		return -1;
	
	    if (ctptr) {
		strcpy(ctptr, ctdptr[cs_num]->ct_encoding);
		ctptr += ctdptr[cs_num]->ct_encoding_len;
	    }
	}

	if (charset->side == XlcGR)
	    ct_state.GR_charset = charset;
	else if (charset->side == XlcGL)
	    ct_state.GL_charset = charset;

	clen = length;
	do {
	    *ctptr++ = charset == ct_state.GR_charset ?
	      BIT8ON(*inbufptr++) : BIT8OFF(*inbufptr++);
	} while (--clen); 
    }

    *to = (XPointer)ctptr;

    if ((num_conv = (int)(ctptr - ct_base)) > 0) {
	(*to_left) -= num_conv;
        return unconv_num;
    }

    return -1;
}

#undef BADCHAR


static void
close_converter(conv)
    XlcConv conv;
{
	Xfree((char *) conv);
}

enum { MBSTOCS, WCSTOCS, MBTOCS, CSTOMBS, CSTOWCS, MBSTOWCS, WCSTOMBS,
       CTSTOWCS, CTSTOMBS, WCSTOCTS, MBSTOCTS };

static XlcConvMethodsRec conv_methods[] = {
    {close_converter, euc_mbstocs,  NULL },
    {close_converter, euc_wcstocs,  NULL },
    {close_converter, euc_mbtocs,   NULL },
    {close_converter, euc_cstombs,  NULL },
    {close_converter, euc_cstowcs,  NULL },
    {close_converter, euc_mbstowcs, NULL },
    {close_converter, euc_wcstombs, NULL },
    {close_converter, euc_ctstowcs, NULL },
    {close_converter, euc_ctstombs, NULL },
    {close_converter, euc_wcstocts, NULL },
    {close_converter, euc_mbstocts, NULL },
};


static XlcConv
open_mbstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOCS]);
}

static XlcConv
open_wcstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOCS]);
}

static XlcConv
open_mbtocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBTOCS]);
}

static XlcConv
open_cstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CSTOMBS]);
}

static XlcConv
open_cstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CSTOWCS]);
}

static XlcConv
open_mbstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOWCS]);
}

static XlcConv
open_wcstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOMBS]);
}

static XlcConv
open_ctstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CTSTOWCS]);
}

static XlcConv
open_ctstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[CTSTOMBS]);
}

static XlcConv
open_wcstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[WCSTOCTS]);
}

static XlcConv
open_mbstocts(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &conv_methods[MBSTOCTS]);
}

XLCd
_XlcEucLoader(name)
    char *name;
{
    XLCd lcd;

    lcd = _XlcCreateLC(name, _XlcGenericMethods);
    if (lcd == NULL)
	return lcd;

    if ((_XlcCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "Euc"))) {
	_XlcDestroyLC(lcd);
	return (XLCd) NULL;
    }

    initCTptr(lcd);

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_mbtocs);

#ifndef FORCE_INDIRECT_CONVERTER
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNMultiByte, open_ctstombs);
    _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNWideChar, open_ctstowcs);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCompoundText, open_mbstocts);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_mbstowcs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCompoundText, open_wcstocts);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_wcstombs);
#endif

    return lcd;
}
