/* $XConsortium: lcGenConv.c,v 1.2 93/09/23 12:31:24 rws Exp $ */
/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */

#include "Xlibint.h"
#include "XlcGeneric.h"

typedef struct _StateRec {
    XLCd lcd;
    XlcCharSet charset;
    XlcCharSet GL_charset;
    XlcCharSet GR_charset;
} StateRec, *State;

static void
init_state(conv)
    XlcConv conv;
{
    register State state = (State) conv->state;
    register XLCdGenericPart *gen = XLC_GENERIC_PART(state->lcd);
    register CodeSet codeset;

    if (codeset = gen->initial_state_GL)
	state->GL_charset = *codeset->charset_list;
    if (codeset = gen->initial_state_GR)
	state->GR_charset = *codeset->charset_list;

    if (state->GL_charset == NULL)
	if (codeset = *gen->codeset_list)
	    state->GL_charset = *codeset->charset_list;
}

static int
compare(src, encoding, length)
    register char *src;
    register char *encoding;
    register int length;
{
    char *start = src;

    while (length-- > 0) {
	if (*src++ != *encoding++)
	    return 0;
	if (*encoding == '\0')
	    return src - start;
    }

    return 0;
}

static int
mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XLCd lcd = state->lcd;
    register char *src, *dst;
    unsigned char ch, *mb_parse_table;
    ParseInfo *parse_list, parse_info;
    XlcCharSet charset;
    int length, number, encoding_len = 0;
    register i;

    src = *((char **) from);
    dst = *((char **) to);

    if (*from_left < 1 || (dst && *to_left < 1))
	return -1;

    if (mb_parse_table = XLC_GENERIC(lcd, mb_parse_table)) {
	number = mb_parse_table[(unsigned char) *src];
	if (number > 0) {
	    parse_list = XLC_GENERIC(lcd, mb_parse_list) + number - 1;
	    for ( ; parse_info = *parse_list; parse_list++) {
		encoding_len = compare(src, parse_info->encoding, *from_left);
		if (encoding_len > 0) {
		    switch (parse_info->type) {
			case E_SS:
			    src += encoding_len;
			    charset = *parse_info->codeset->charset_list;
			    goto found;
			case E_LSL:
			case E_LSR:
			    src += encoding_len;
			    charset = *parse_info->codeset->charset_list;
			    if (parse_info->type == E_LSL)
			    	state->GL_charset = charset;
			    else
			    	state->GR_charset = charset;
			    length = 0;
			    goto end;
			case E_GL:
			    charset = state->GL_charset;
			    goto found;
			case E_GR:
			    charset = state->GR_charset;
			    goto found;
		    }
		}
	    }
	}
    }

    if ((*src & 0x80) && state->GR_charset)
	charset = state->GR_charset;
    else
	charset = state->GL_charset;

found:
    if (charset == NULL)
	return -1;

    length = charset->char_size;
    if (length > *from_left - encoding_len)
	return -1;

    if (dst) {
	if (length > *to_left)
	    return -1;
	if (charset->side == XlcGL) {
	    for (i = 0; i < length; i++)
		*dst++ = *src++ & 0x7f;
	} else if (charset->side == XlcGR) {
	    for (i = 0; i < length; i++)
		*dst++ = *src++ | 0x80;
	} else {
	    for (i = 0; i < length; i++)
		*dst++ = *src++;
	}
	*to = (XPointer) dst;
	*to_left -= length;
    }
end:
    *from = (XPointer) src;
    *from_left -= encoding_len + length;
    state->charset = charset;
    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    return 0;
}

static int
mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XPointer tmp_from, tmp_to;
    int tmp_from_left, tmp_to_left;
    XlcCharSet charset, tmp_charset;
    StateRec tmp_state;
    XPointer tmp_args[1];
    int unconv_num = 0, ret;

    tmp_args[0] = (XPointer) &charset;

    do {
	ret = mbtocs(conv, from, from_left, to, to_left, tmp_args, 1);
	if (ret < 0)
	    return ret;
	unconv_num += ret;
    } while (ret > 0) ;

    tmp_from = *from;
    tmp_from_left = *from_left;
    tmp_to = *to;
    tmp_to_left = *to_left;
    tmp_state = *state;
    tmp_args[0] = (XPointer) &tmp_charset;
    
    while (1) {
	ret = mbtocs(conv, &tmp_from, &tmp_from_left, &tmp_to,
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
	tmp_state = *state;
    }

    *state = tmp_state;

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
wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XLCd lcd = state->lcd;
    wchar_t *wcptr;
    register char *bufptr;
    register wchar_t wch;
    char *tmpptr;
    register length;
    CodeSet codeset;
    unsigned long wc_encoding;
    int wcstr_len, buf_len;

    if (from == NULL || *from == NULL)
	return 0;

    wcptr = *((wchar_t **) from);
    bufptr = *((char **) to);
    wcstr_len = *from_left;
    buf_len = *to_left;

    codeset = wc_parse_codeset(lcd, wcptr);
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
	if ((*codeset->charset_list)->side == XlcGL) {
	    while (length--) {
		*tmpptr-- = (unsigned char) (wch & 0x7f);
		wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
	    }
	} else if ((*codeset->charset_list)->side == XlcGR) {
	    while (length--) {
		*tmpptr-- = (unsigned char) (wch | 0x80);
		wch >>= (wchar_t)XLC_GENERIC(lcd, wc_shift_bits);
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

    *from_left -= wcptr - *((wchar_t **) from);
    *from = (XPointer) wcptr;

    *to_left -= bufptr - *((char **) to);
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
cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    register char *csptr;
    register char *bufptr;
    int csstr_len;
    register buf_len;
    int num, encoding_len = 0;
    CodeSet codeset;
    XlcCharSet charset;
    EncodingType type;
    int cvt_length;

    csptr = *((char **) from);
    bufptr = *((char **) to);
    csstr_len = *from_left;
    buf_len = *to_left;

    if (num_args < 1)
	return -1;
    
    charset = (XlcCharSet) args[0];

    codeset = _XlcGetCodeSetFromCharSet(state->lcd, charset);
    if (codeset == NULL)
	return -1;

    cvt_length = 0;
    if (codeset->parse_info) {
	switch (type = codeset->parse_info->type) {
	    case E_SS:
		encoding_len = strlen(codeset->parse_info->encoding);
		break;
	    case E_LSL:
	    case E_LSR:
		if (type == E_LSL) {
		    if (charset == state->GL_charset)
			break;
		} else {
		    if (charset == state->GR_charset)
			break;
		}
		encoding_len = strlen(codeset->parse_info->encoding);
		if (encoding_len > buf_len)
		    return -1;
		cvt_length += encoding_len;
		if (bufptr) {
		    strcpy(bufptr, codeset->parse_info->encoding);
		    bufptr += encoding_len;
		}
		buf_len -= encoding_len;
		encoding_len = 0;
		if (type == E_LSL)
		    state->GL_charset = charset;
		else
		    state->GR_charset = charset;
		break;
	}
    }

    csstr_len /= codeset->length;
    buf_len /= codeset->length + encoding_len;
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    
    cvt_length += buf_len * (encoding_len + codeset->length);
    if (bufptr) {
	while (buf_len--) {
	    if (encoding_len) {
		strcpy(bufptr, codeset->parse_info->encoding);
		bufptr += encoding_len;
	    }
	    num = codeset->length;
	    if (codeset->side == XlcGL) {
		while (num--)
		    *bufptr++ = *csptr++ & 0x7f;
	    } else if (codeset->side == XlcGR) {
		while (num--)
		    *bufptr++ = *csptr++ | 0x80;
	    } else {
		while (num--)
		    *bufptr++ = *csptr++;
	    }
    	}
    }

    *from_left -= csptr - *((char **) from);
    *from = (XPointer) csptr;

    if (bufptr)
	*to = (XPointer) bufptr;
    *to_left -= cvt_length;

    return 0;
}

static int
cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    XLCd lcd = state->lcd;
    register char *csptr;
    wchar_t *bufptr;
    int csstr_len;
    register buf_len;
    register wchar_t wch;
    unsigned long code_mask, wc_encoding;
    int num, length, wc_shift_bits;
    CodeSet codeset;

    csptr = *((char **) from);
    bufptr = *((wchar_t **) to);
    csstr_len = *from_left;
    buf_len = *to_left;

    if (num_args < 1)
	return -1;
    
    codeset = _XlcGetCodeSetFromCharSet(lcd, (XlcCharSet) args[0]);
    if (codeset == NULL)
	return -1;

    length = codeset->length;
    csstr_len /= length;
    if (csstr_len < buf_len)
	buf_len = csstr_len;
    
    code_mask = ~XLC_GENERIC(lcd, wc_encode_mask);
    wc_encoding = codeset->wc_encoding;
    wc_shift_bits = XLC_GENERIC(lcd, wc_shift_bits);

    *to_left -= buf_len;

    if (bufptr) {
	while (buf_len--) {
	    wch = (wchar_t) (*csptr++ & 0x7f);
	    num = length - 1;
	    while (num--)
		wch = (wch << wc_shift_bits) | (*csptr++ & 0x7f);

	    *bufptr++ = (wch & code_mask) | wc_encoding;
	}
    }

    *from_left -= csptr - *((char **) from);
    *from = (XPointer) csptr;

    if (bufptr)
	*to = (XPointer) bufptr;

    return 0;
}


static void
close_converter(conv)
    XlcConv conv;
{
    if (conv->state) {
	Xfree((char *) conv->state);
    }

    Xfree((char *) conv);
}

static XlcConv
create_conv(lcd, methods)
    XLCd lcd;
    XlcConvMethods methods;
{
    XlcConv conv;
    State state;

    conv = (XlcConv) Xmalloc(sizeof(XlcConvRec));
    if (conv == NULL)
	return (XlcConv) NULL;
    
    conv->methods = (XlcConvMethods) Xmalloc(sizeof(XlcConvMethodsRec));
    if (conv->methods == NULL)
	goto err;
    *conv->methods = *methods;
    if (XLC_PUBLIC(lcd, is_state_depend))
	conv->methods->reset = init_state;

    conv->state = (XPointer) Xmalloc(sizeof(StateRec));
    if (conv->state == NULL)
	goto err;
    bzero((char *) conv->state, sizeof(StateRec));
    
    state = (State) conv->state;
    state->lcd = lcd;
    init_state(conv);
    
    return conv;

err:
    close_converter(conv);

    return (XlcConv) NULL;
}

static XlcConvMethodsRec mbstocs_methods = {
    close_converter,
    mbstocs,
    NULL
} ;

static XlcConv
open_mbstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbstocs_methods);
}

static XlcConvMethodsRec wcstocs_methods = {
    close_converter,
    wcstocs,
    NULL
} ;

static XlcConv
open_wcstocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &wcstocs_methods);
}

static XlcConvMethodsRec mbtocs_methods = {
    close_converter,
    mbtocs,
    NULL
} ;

static XlcConv
open_mbtocs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &mbtocs_methods);
}

static XlcConvMethodsRec cstombs_methods = {
    close_converter,
    cstombs,
    NULL
} ;

static XlcConv
open_cstombs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &cstombs_methods);
}

static XlcConvMethodsRec cstowcs_methods = {
    close_converter,
    cstowcs,
    NULL
} ;

static XlcConv
open_cstowcs(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    char *from_type;
    XLCd to_lcd;
    char *to_type;
{
    return create_conv(from_lcd, &cstowcs_methods);
}

XLCd
_XlcGenericLoader(name)
    char *name;
{
    XLCd lcd;

    lcd = _XlcCreateLC(name, _XlcGenericMethods);
    if (lcd == NULL)
	return lcd;

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_mbtocs);

    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);

    return lcd;
}
