/* $XConsortium$ */
/******************************************************************

        Copyright 1992, 1993 by FUJITSU LIMITED
        Copyright 1993 by Fujitsu Open Systems Solutions, Inc.

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of FUJITSU LIMITED and
Fujitsu Open Systems Solutions, Inc. not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.
FUJITSU LIMITED and Fujitsu Open Systems Solutions, Inc. makes no representations
about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

FUJITSU LIMITED AND FUJITSU OPEN SYSTEMS SOLUTIONS, INC. DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL FUJITSU OPEN SYSTEMS SOLUTIONS, INC.
AND FUJITSU LIMITED BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

    Authors: Shigeru Yamada 		(yamada@ossi.com)
             Jeffrey Bloomfield		(jeffb@ossi.com)

*****************************************************************/

#ifdef NOTDEF
#include "Xlibint.h"
#include "XlcGeneric.h"

unsigned char SS2, SS3;

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

    register unsigned char ch;
    register int chr_len = 0;
    register int sshift =  False;
    register int inbuf_len = *from_left;
    register int outbuf = 0;
    register int outbuf_len = *to_left;
    register int shift_mult = 0;
    register unsigned int chrcode;

    unsigned int wc_encode;
    unsigned int wc_tmp = 0;

    int cs0flg = False;
    int cs1flg = False;
    int errflg =  0;
    int length;
    int outbuf_flg = False;
    int ret;

    Bool new_char;

    XPointer *inbufptr = from;
    wchar_t **outbufptr = (wchar_t **)to;
    int wcstr = (int)*outbufptr;

    CodeSet *list_codesets = XLC_GENERIC(lcd, codeset_list); 

    unsigned long wc_shift = XLC_GENERIC(lcd, wc_shift_bits);
    int codeset_num = XLC_GENERIC(lcd, codeset_num);

    SS2 = 0x8e;
    SS3 = 0x8f;

    if (codeset_num > 2) {
	SS2 = (unsigned char) *list_codesets[2]->parse_info->encoding;
	if (codeset_num > 3)
	    SS3 = (unsigned char) *list_codesets[3]->parse_info->encoding;
    }

    if (outbuf_len > 0)		/* relies on to_left initialized to 0 */
	outbuf_flg = True;	/* in _Xlcmbtowc()		      */

    for (new_char = True; inbuf_len > 0;) {

	ch = *(*inbufptr)++;

	if (!(ch & 0x80)) {   	/* left-side char (CS0) */
	    if( cs0flg == True) {
		new_char = True;
		cs0flg = False;
	    }
	    length = 1;
	    **outbufptr = (wchar_t)ch;
	    (*outbufptr)++;

	    inbuf_len--;
	    continue;
	}
	else if (ch == SS2) {		/* CS2 */
	    if (sshift == True || cs1flg == True) {
		cs1flg = False;
		errflg++;
		continue;
	    }
	    length = list_codesets[2]->length;
	    wc_encode = list_codesets[2]->wc_encoding;
	    chrcode = 0;
	    sshift = True;
	    cs0flg = True;
	    inbuf_len--;
	    continue;
	}
	else if (ch == SS3) {		/* CS3 */
	    if (sshift == True || cs1flg == True) {
		cs1flg = False;
		errflg++;
		continue;
	    }
	    length = list_codesets[3]->length;
	    wc_encode = list_codesets[3]->wc_encoding;
	    chrcode = 0;
	    sshift = True;
	    cs0flg = True;
	    inbuf_len--;
	    continue;

	} else {			/* CS1 */
	    if (sshift == False) {
		length = list_codesets[1]->length;
		wc_encode = list_codesets[1]->wc_encoding;
	    }
	    chrcode = ch & 0x7F; 	/* turn off high bit */
	    cs0flg = True;
	    cs1flg = True;
	    inbuf_len--;
	}

	if (new_char) {			/* begin new character */
	    chr_len = length;
	    shift_mult = length-1;
	    new_char = False;
	}

	if (length < 1) {
	    break;
	}

	chrcode <<= (wc_shift * shift_mult);
	shift_mult--;
	wc_tmp |= chrcode; 

	if (--chr_len == 0) {
	    wc_tmp |= wc_encode;
	    **outbufptr = wc_tmp;
	    (*outbufptr)++;

	    new_char = True;
	    sshift = False;
	    cs0flg = False;
	    cs1flg = False;
	    wc_tmp  = (unsigned int)0;

	    if (outbuf_flg == True)		/* for len in _Xlcmbstowcs() */
		if (outbuf++ >= outbuf_len) {
		    errflg++;
		    break;
		}
	}

    }	/* end for */

    if (cs0flg == True || cs1flg == True){	/* error check on last char */
	errflg++;
    }

    if (errflg) {
	return -1;
    }

    if ((ret = (int)(*outbufptr) - wcstr) > 0) {
        *to_left = (*to_left) - ret;     /* values used by _Xlcmbstocs() and */
        *from_left = 0;                  /* _Xlcmbtowc()                     */
        return ret;
    }

    return -1;
}


static int
euc_wcstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    wchar_t **from;
    int *from_left;
    unsigned char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    wchar_t **inbufptr = from;
    register unsigned char **outbufptr = to;
    register wchar_t  wch;
    register length;
    unsigned char tmp;
    int to_bytes;
    int errflg = 0;
    int ret;
    unsigned int u;
    int wch_len = *from_left, buf_len = *to_left; 


    XLCd lcd = (XLCd)conv->state;
    CodeSet *cset = XLC_GENERIC(lcd, codeset_list);
    unsigned long wc_encoding_mask = XLC_GENERIC(lcd, wc_encode_mask);
    unsigned long wc_shift = XLC_GENERIC(lcd, wc_shift_bits);

    unsigned long wch_encode;
    unsigned long sshift;
    XlcSide side;

    CodeSet *list_codesets = XLC_GENERIC(lcd, codeset_list); 
    int codeset_num = XLC_GENERIC(lcd, codeset_num);

    SS2 = 0x8e;
    SS3 = 0x8f;

    if (codeset_num > 2) {
	SS2 = (unsigned char) *list_codesets[2]->parse_info->encoding;
	if (codeset_num > 3)
	    SS3 = (unsigned char) *list_codesets[3]->parse_info->encoding;
    }


#   define CS0	cset[0]->wc_encoding
#   define CS1	cset[1]->wc_encoding
#   define CS2	cset[2]->wc_encoding
#   define CS3	cset[3]->wc_encoding

     for (to_bytes = 0; wch_len > 0 && buf_len > 0; wch_len--, buf_len--) {

	wch = *(*inbufptr)++;

	wch_encode = wch & wc_encoding_mask;

	if (wch_encode == CS0 ) {
	    length = cset[0]->length;
	    side = cset[0]->side;
	    wch = wch ^ CS0;
	    sshift = 0;
	}
	else if (wch_encode == CS1) {
	    length = cset[1]->length;
	    side = cset[1]->side;
	    wch = wch ^ CS1;
	    sshift = 0;
	}
	else if (wch_encode == CS2) {
	    length = cset[2]->length;
	    side = cset[2]->side;
	    wch = wch ^ CS2;
	    sshift = SS2;
	}
	else if (wch_encode == CS3) {
	    length = cset[3]->length;
	    side = cset[3]->side;
	    wch = wch ^ CS3;
	    sshift = SS3;
	}
	else
	    errflg++;

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

	if (sshift) {
	    **outbufptr = (unsigned char)sshift;
	    (*outbufptr)++;
	    to_bytes++;
	}

	while (--length + 1) {
	    tmp = ((wch>>(wchar_t)(length * wc_shift)) & 0x7F) | side;
	    **outbufptr = (unsigned char)tmp;
	    (*outbufptr)++;
	    to_bytes++;
	}
    }
    if (errflg)
	return -1;

    if ((ret = to_bytes) > 0) {
	*to_left = (*to_left) - to_bytes;/* _Xlcmbstoc() for NULL/ret value */
	*from_left = 0;
	return to_bytes;
    }

    return -1;
}

static int
EUC_mbtocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
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

    if (from == NULL || *from == NULL)
	return init_state(state);
    
    src = *from;
    dst = *to;

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
	*to = dst;
	*to_left -= length;
    }
end:
    *from = src;
    *from_left -= encoding_len + length;
    state->charset = charset;
    if (num_args > 0)
	*((XlcCharSet *) args[0]) = charset;

    return 0;
}

static int
EUC_mbstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    State state = (State) conv->state;
    char *tmp_from, *tmp_to;
    int tmp_from_left, tmp_to_left;
    XlcCharSet charset, tmp_charset;
    StateRec tmp_state;
    XPointer tmp_args[1];
    int unconv_num = 0, ret;

    if (from == NULL || *from == NULL)
	return init_state(state);

    tmp_args[0] = (XPointer) &charset;

    do {
	ret = EUC_mbtocs(conv, from, from_left, to, to_left, tmp_args, 1);
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
	ret = EUC_mbtocs(conv, &tmp_from, &tmp_from_left, &tmp_to,
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
EUCwc_parse_codeset(lcd, wcstr)
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
EUC_wcstocs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    wchar_t **from;
    int *from_left;
    char **to;
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
	return init_state(state);

    wcptr = *from;
    bufptr = *to;
    wcstr_len = *from_left;
    buf_len = *to_left;

    codeset = EUCwc_parse_codeset(lcd, wcptr);
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

    *from_left -= wcptr - *from;
    *from = wcptr;

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
EUC_cstombs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    char **to;
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

    if (from == NULL || *from == NULL)
	return init_state(state);
    
    csptr = *from;
    bufptr = *to;
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
	    if (charset->side == XlcGL) {
		while (num--)
		    *bufptr++ = *csptr++ & 0x7f;
	    } else if (charset->side == XlcGR) {
		while (num--)
		    *bufptr++ = *csptr++ | 0x80;
	    } else {
		while (num--)
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
EUC_cstowcs(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    char **from;
    int *from_left;
    wchar_t **to;
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

    if (from == NULL || *from == NULL)
	return init_state(state);
    
    csptr = *from;
    bufptr = *to;
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

    if (bufptr)
	*to += buf_len;
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

    *from_left -= csptr - *from;
    *from = csptr;

    return 0;
}


static void
close_cs_converter(conv)
    XlcConv conv;
{
    if (conv->state) {
	_XlcFree((char *) conv->state);
    }

    _XlcFree((char *) conv);
}

static void
reset_cs_converter(conv)
    XlcConv conv;
{
    if (conv->state)
	init_state(conv->state);
}

static XlcConvMethodsRec mbtocs_methods = {
    close_cs_converter,
    EUC_mbtocs,
    reset_cs_converter
} ;

static XlcConvMethodsRec mbstocs_methods = {
    close_cs_converter,
    EUC_mbstocs,
    reset_cs_converter
} ;

static XlcConvMethodsRec wcstocs_methods = {
    close_cs_converter,
    EUC_wcstocs,
    reset_cs_converter
} ;

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
open_from_converter(lcd, from, to)
    XLCd lcd;
    char *from;
    char *to;
{
    XlcConv conv;
    State state;
    XlcConvMethods methods;
    XlcConvType from_type, to_type;

    from_type = get_conversion_type(from);
    to_type = get_conversion_type(to);

    switch (from_type) {
	case XlcMultiByteType:
	    if (to_type == XlcCharType)
		methods = &mbtocs_methods;
	    else
		methods = &mbstocs_methods;
	    break;
	case XlcWideCharType:
	    methods = &wcstocs_methods;
	    break;
	default:
	    return (XlcConv) NULL;
    }

    conv = (XlcConv) _XlcAlloc(sizeof(XlcConvRec));
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;
    
    conv->methods = methods;
    conv->state = (XPointer) _XlcAlloc(sizeof(StateRec));
    if (conv->state == (XPointer) NULL)
	goto err;
    
    state = (State) conv->state;
    state->lcd = lcd;
    init_state(state);
    
    return conv;

err:
    close_cs_converter(conv);

    return (XlcConv) NULL;
}


static XlcConvMethodsRec cstombs_methods = {
    close_cs_converter,
    EUC_cstombs,
    reset_cs_converter
} ;

static XlcConvMethodsRec cstowcs_methods = {
    close_cs_converter,
    EUC_cstowcs,
    reset_cs_converter
} ;

static XlcConv
open_to_converter(lcd, from, to)
    XLCd lcd;
    char *from;
    char *to;
{
    XlcConv conv;
    State state;
    XlcConvMethods methods;
    XlcConvType from_type, to_type;

    from_type = get_conversion_type(from);
    to_type = get_conversion_type(to);

    switch (to_type) {
	case XlcMultiByteType:
	    methods = &cstombs_methods;
	    break;
	case XlcWideCharType:
	    methods = &cstowcs_methods;
	    break;
	default:
	    return (XlcConv) NULL;
    }

    conv = (XlcConv) _XlcAlloc(sizeof(XlcConvRec));
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;
    
    conv->methods = methods;
    conv->state = (XPointer) _XlcAlloc(sizeof(StateRec));
    if (conv->state == (XPointer) NULL)
	goto err;
    
    state = (State) conv->state;
    state->lcd = lcd;
    init_state(state);
    
    return conv;

err:
    close_cs_converter(conv);

    return (XlcConv) NULL;
}

static void
close_euc_converter(conv)
    XlcConv conv;
{
    _XlcFree((char *) conv);
}

static XlcConvMethodsRec mbstowcs_methods = {
    close_euc_converter,
    euc_mbstowcs,
    NULL
} ;

static XlcConvMethodsRec wcstombs_methods = {
    close_euc_converter,
    euc_wcstombs,
    NULL
} ;

static XlcConv
open_euc_converter(lcd, from, to)
    XLCd lcd;
    char *from;
    char *to;
{
    XlcConv conv;
    XlcConvType from_type, to_type;

    from_type = get_conversion_type(from);
    to_type = get_conversion_type(to);

    conv = (XlcConv) _XlcAlloc(sizeof(XlcConvRec));
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;
    
    if (from_type == XlcMultiByteType)
	conv->methods = &mbstowcs_methods;
    else
	conv->methods = &wcstombs_methods;

    conv->state = (XPointer) lcd;
    
    return conv;

err:
    close_euc_converter(conv);

    return (XlcConv) NULL;
}


static int
convert(lc_conv, from, from_left, to, to_left, args, num_args)
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

	tmp_args[0] = (XPointer) charset;

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

static void
close_converter(lc_conv)
    XlcConv lc_conv;
{
    Conv conv = (Conv) lc_conv->state;

    if (conv) {
	if (conv->from_conv)
	    (*conv->from_conv->methods->close)(conv->from_conv);
	if (conv->to_conv)
	    (*conv->to_conv->methods->close)(conv->to_conv);

	_XlcFree((char *) conv);
    }

    _XlcFree((char *) lc_conv);
}

static void
reset_converter(lc_conv)
    XlcConv lc_conv;
{
    Conv conv = (Conv) lc_conv->state;

    if (conv) {
	if (conv->from_conv && conv->from_conv->methods->reset)
	    (*conv->from_conv->methods->reset)(conv->from_conv);
	if (conv->to_conv && conv->to_conv->methods->reset)
	    (*conv->to_conv->methods->reset)(conv->to_conv);
    }
}

static XlcConvMethodsRec conv_methods = {
    close_converter,
    convert,
    reset_converter
} ;

static XlcConv
open_converter(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    XlcConv lc_conv;
    Conv conv;
    XlcConvType from_type, to_type;

    from_type = get_conversion_type(from);
    to_type = get_conversion_type(to);

    if (to_type == XlcCharSetType || to_type == XlcCharType)
	return open_from_converter(from_lcd, from, to);

    if (from_type == XlcCharSetType || from_type == XlcCharType)
	return open_to_converter(from_lcd, from, to);

    if ((from_type == XlcMultiByteType && to_type == XlcWideCharType)
	|| (from_type == XlcWideCharType && to_type == XlcMultiByteType))
	return open_euc_converter(from_lcd, from, to);

    lc_conv = (XlcConv) _XlcAlloc(sizeof(XlcConvRec));
    if (lc_conv == (XlcConv) NULL)
	return (XlcConv) NULL;
    
    lc_conv->methods = &conv_methods;

    lc_conv->state = (XPointer) _XlcAlloc(sizeof(ConvRec));
    if (lc_conv->state == (XPointer) NULL)
	goto err;
    
    conv = (Conv) lc_conv->state;
    conv->from_conv = conv->to_conv = (XlcConv) NULL;

    conv->from_conv = open_from_converter(from_lcd, from, XlcNCharSet);
    if (conv->from_conv == (XlcConv) NULL)
	goto err;

    conv->to_conv = open_to_converter(from_lcd, XlcNCharSet, to);
    if (conv->to_conv == (XlcConv) NULL)
	goto err;

    return lc_conv;

err:
    close_converter(lc_conv);

    return (XlcConv) NULL;
}

XLCd
_XlcEucLoader(name)
    char *name;
{
    XLCd lcd;

    lcd = _XlcCreateLC(name, _xlcGenericMethods);
    if (lcd == (XLCd) NULL)
	return lcd;
    
    if ((_XlcCompareISOLatin1(XLC_PUBLIC_PART(lcd)->codeset, "euc"))) {
	_XlcDestroyLC(lcd);
	return (XLCd) NULL;
    }

    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_converter);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_converter);
    _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNChar, open_converter);

    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_converter);

    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_converter);
    _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_converter);

    return lcd;
}
#else /* not NOTDEF */
#ifndef lint
static int dummy;       /* prevent ranlibs from complaining */
#endif  
#endif /* ifdef NOTDEF */
