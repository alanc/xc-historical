/* $XConsortium: lcConv.c,v 1.1 93/09/17 13:29:50 rws Exp $ */
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
#include "XlcPubI.h"

typedef XlcConv (*XlcConverter)();

typedef struct _XlcConverterListRec {
    XLCd from_lcd;
    char *from;
    XrmQuark from_type;
    XLCd to_lcd;
    char *to;
    XrmQuark to_type;
    XlcConverter converter;
    struct _XlcConverterListRec *next;
} XlcConverterListRec, *XlcConverterList;

static XlcConverterList conv_list = NULL;

static void
close_converter(conv)
    XlcConv conv;
{
    (*conv->methods->close)(conv);
}

static XlcConv
get_converter(from_lcd, from_type, to_lcd, to_type)
    XLCd from_lcd;
    XrmQuark from_type;
    XLCd to_lcd;
    XrmQuark to_type;
{
    register XlcConverterList list, prev = NULL;
    XlcConv conv;

    for (list = conv_list; list; list = list->next) {
	if (list->from_lcd == from_lcd && list->to_lcd == to_lcd 
	    && list->from_type == from_type && list->to_type == to_type) {

	    if (prev && prev != conv_list) {	/* XXX */
		prev->next = list->next;
		list->next = conv_list;
		conv_list = list;
	    }

	    return (*list->converter)(from_lcd, list->from, to_lcd, list->to);
	}

	prev = list;
    }
    
    return (XlcConv) NULL;
}

Bool
_XlcSetConverter(from_lcd, from, to_lcd, to, converter)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
    XlcConv (*converter)();
{
    register XlcConverterList list;
    register XrmQuark from_type, to_type;

    from_type = XrmStringToQuark(from);
    to_type = XrmStringToQuark(to);

    for (list = conv_list; list; list = list->next) {
	if (list->from_lcd == from_lcd && list->to_lcd == to_lcd 
	    && list->from_type == from_type && list->to_type == to_type) {

	    list->converter = converter;
	    return True;
	}
    }

    list = (XlcConverterList) Xmalloc(sizeof(XlcConverterListRec));
    if (list == NULL)
	return False;
    
    list->from_lcd = from_lcd;
    list->from = from;
    list->from_type = from_type;
    list->to_lcd = to_lcd;
    list->to = to;
    list->to_type = to_type;
    list->converter = converter;
    list->next = conv_list;
    conv_list = list;

    return True;
}

typedef struct _ConvRec {
    XlcConv from_conv;
    XlcConv to_conv;
} ConvRec, *Conv;

static int
indirect_convert(lc_conv, from, from_left, to, to_left, args, num_args)
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

    if (from == NULL || *from == NULL) {
	if (from_conv->methods->reset)
	    (*from_conv->methods->reset)(from_conv);

	if (to_conv->methods->reset)
	    (*to_conv->methods->reset)(to_conv);

	return 0;
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
close_indirect_converter(lc_conv)
    XlcConv lc_conv;
{
    Conv conv = (Conv) lc_conv->state;

    if (conv) {
	if (conv->from_conv)
	    close_converter(conv->from_conv);
	if (conv->to_conv)
	    close_converter(conv->to_conv);

	_XlcFree((char *) conv);
    }

    _XlcFree((char *) lc_conv);
}

static void
reset_indirect_converter(lc_conv)
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
    close_indirect_converter,
    indirect_convert,
    reset_indirect_converter
} ;

static XlcConv
open_indirect_converter(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    XlcConv lc_conv;
    Conv conv;
    XrmQuark from_type, to_type;
    static XrmQuark QCharSet = (XrmQuark) 0;

    if (QCharSet == (XrmQuark) 0)
	QCharSet = XrmStringToQuark(XlcNCharSet);

    from_type = XrmStringToQuark(from);
    to_type = XrmStringToQuark(to);

    if (from_type == QCharSet || to_type == QCharSet)
	return (XlcConv) NULL;

    lc_conv = (XlcConv) _XlcAlloc(sizeof(XlcConvRec));
    if (lc_conv == NULL)
	return (XlcConv) NULL;
    
    lc_conv->methods = &conv_methods;

    lc_conv->state = (XPointer) _XlcAlloc(sizeof(ConvRec));
    if (lc_conv->state == NULL)
	goto err;
    
    conv = (Conv) lc_conv->state;
    conv->from_conv = conv->to_conv = NULL;

    conv->from_conv = get_converter(from_lcd, from_type, from_lcd, QCharSet);
    if (conv->from_conv == NULL)
	conv->from_conv = get_converter((XLCd) NULL, from_type, (XLCd) NULL, QCharSet);
    if (conv->from_conv == NULL)
	goto err;

    conv->to_conv = get_converter(to_lcd, QCharSet, to_lcd, to_type);
    if (conv->to_conv == NULL)
	conv->to_conv = get_converter((XLCd) NULL, QCharSet, (XLCd) NULL, to_type);
    if (conv->to_conv == NULL)
	goto err;

    return lc_conv;

err:
    close_indirect_converter(lc_conv);

    return (XlcConv) NULL;
}

XlcConv
_XlcOpenConverter(from_lcd, from, to_lcd, to)
    XLCd from_lcd;
    char *from;
    XLCd to_lcd;
    char *to;
{
    XlcConv conv;
    XrmQuark from_type, to_type;

    from_type = XrmStringToQuark(from);
    to_type = XrmStringToQuark(to);

    if (conv = get_converter(from_lcd, from_type, to_lcd, to_type))
	return conv;
    
    return open_indirect_converter(from_lcd, from, to_lcd, to);
}

void
_XlcCloseConverter(conv)
    XlcConv conv;
{
    close_converter(conv);
}

int
_XlcConvert(conv, from, from_left, to, to_left, args, num_args)
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    return (*conv->methods->convert)(conv, from, from_left, to, to_left, args,
				     num_args);
}

void
_XlcResetConverter(conv)
    XlcConv conv;
{
    if (conv->methods->reset)
	(*conv->methods->reset)(conv);
}

typedef struct _AllocListRec {
  Bool is_used;
  int length;
  char *ptr;
  struct _AllocListRec *next;
} AllocListRec, *AllocList;

static AllocList alloc_list = NULL;

char *
_XlcAlloc(length)
    int length;
{
    register AllocList list, min_list, max_list;
    char *ptr;

    min_list = max_list = NULL;

    for (list = alloc_list; list; list = list->next) {
	if (list->is_used == False) {
	    if (length == list->length) {
		min_list = list;
		break;
	    }
	    if (length < list->length) {
		if (min_list && list->length > min_list->length)
		    continue;
		min_list = list;
	    } else if (min_list == NULL) {
		if (max_list && list->length < max_list->length)
		    continue;
		max_list = list;
	    }
	}
    }

    if (min_list) {
	list = min_list;
	goto done;
    } else if (max_list) {
	list = max_list;
	ptr = Xrealloc(list->ptr, length);
	if (ptr == NULL)
	    return NULL;
    } else {
	list = (AllocList) Xmalloc(sizeof(AllocListRec));
	if (list == NULL)
	    return NULL;

	ptr = Xmalloc(length);
	if (ptr == NULL) {
	    Xfree(list);
	    return NULL;
	}

	list->next = alloc_list;
	alloc_list = list;
    }

    list->ptr = ptr;
    list->length = length;
done:
    list->is_used = True;

    return list->ptr;
}

void
_XlcFree(ptr)
    register char *ptr;
{
    register AllocList list;

    for (list = alloc_list; list; list = list->next)
	if (list->ptr == ptr && list->is_used == True) {
	    list->is_used = False;
	    return;
	}
}
