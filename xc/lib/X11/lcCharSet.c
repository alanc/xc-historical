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

#include <stdio.h>
#include "Xlibint.h"
#include "XlcPublic.h"

#if NeedVarargsPrototypes
#include <stdarg.h>
#define Va_start(a, b)	va_start(a, b)
#else
#include <varargs.h>
#define Va_start(a, b)	va_start(a)
#endif

#if NeedVarargsPrototypes
char *
_XlcGetCSValues(XlcCharSet charset, ...)
#else
char *
_XlcGetCSValues(charset, va_alist)
    XlcCharSet charset;
    va_dcl
#endif
{
    va_list var;
    XlcArgList arg_list, list;
    char *ret;
    int count;

    Va_start(var, charset);

    for (count = 0; va_arg(var, char *); count++)
	va_arg(var, XPointer);

    va_end(var);

    arg_list = (XlcArgList) Xmalloc(sizeof(XlcArg) * count + 1);
    if (arg_list == (XlcArgList) NULL)
	return (char *) NULL;
    
    Va_start(var, charset);

    for (list = arg_list; list->name = va_arg(var, char *); list++)
	list->value = va_arg(var, XPointer);

    va_end(var);

    if (charset->get_values)
	ret = (*charset->get_values)(charset, arg_list);
    else
	ret = arg_list->name;

    Xfree(arg_list);

    return ret;
}

typedef struct _XlcCharSetListRec {
    XlcCharSet charset;
    struct _XlcCharSetListRec *next;
} XlcCharSetListRec, *XlcCharSetList;

static XlcCharSetList charset_list = NULL;

XlcCharSet
_XlcGetCharSet(name)
    char *name;
{
    XlcCharSetList list;
    XrmQuark xrm_name;
    
    xrm_name = XrmStringToQuark(name);

    for (list = charset_list; list; list = list->next) {
	if (xrm_name == list->charset->xrm_name)
	    return (XlcCharSet) list->charset;
    }

    return (XlcCharSet) NULL;
}

Bool
_XlcAddCharSet(charset)
    XlcCharSet charset;
{
    XlcCharSetList list;

    if (_XlcGetCharSet(charset->name))
	return False;

    list = (XlcCharSetList) Xmalloc(sizeof(XlcCharSetListRec));
    if (list == NULL)
	return False;
    
    list->charset = charset;
    list->next = charset_list;
    charset_list = list;

    return True;
}

static char *
get_values(charset, arg_list)
    register XlcCharSet charset;
    register XlcArgList arg_list;
{
    /* XXX */
    for ( ; arg_list->name; arg_list++) {
	if (!strcmp(arg_list->name, XlcNName))
	    *((char **) arg_list->value) = charset->name;
	else if (!strcmp(arg_list->name, XlcNEncodingName))
	    *((char **) arg_list->value) = charset->encoding_name;
	else if (!strcmp(arg_list->name, XlcNSide))
	    *((XlcSide *) arg_list->value) = charset->side;
	else if (!strcmp(arg_list->name, XlcNCharSize))
	    *((int *) arg_list->value) = charset->char_size;
	else if (!strcmp(arg_list->name, XlcNSetSize))
	    *((int *) arg_list->value) = charset->set_size;
	else if (!strcmp(arg_list->name, XlcNControlSequence))
	    *((char **) arg_list->value) = charset->ct_sequence;
	else
	    return arg_list->name;
    }

    return (char *) NULL;
}

XlcCharSet
_XlcCreateDefaultCharSet(name, ct_sequence)
    char *name;
    char *ct_sequence;
{
    XlcCharSet charset;

    charset = (XlcCharSet) Xmalloc(sizeof(XlcCharSetRec));
    if (charset == NULL)
	return (XlcCharSet) NULL;
    bzero(charset, sizeof(XlcCharSetRec));
    
    charset->name = name;
    charset->ct_sequence = ct_sequence;
    charset->get_values = get_values;

    _XlcParseCharSet(charset);

    return (XlcCharSet) charset;
}
