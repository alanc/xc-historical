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
 Modifier : Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include "Xlibint.h"
#include "XlcPublicI.h"

#if NeedVarargsPrototypes
#include <stdarg.h>
#define Va_start(a, b)	va_start(a, b)
#else
#include <varargs.h>
#define Va_start(a, b)	va_start(a)
#endif

#if NeedVarargsPrototypes
char *
_XlcGetLCValues(XLCd lcd, ...)
#else
char *
_XlcGetLCValues(lcd, va_alist)
    XLCd lcd;
    va_dcl
#endif
{
    va_list var;
    XlcArgList arg_list, list;
    char *ret;
    int count;
    XLCdPublicMethodsPart *methods = XLC_PUBLIC_METHODS(lcd);

    Va_start(var, lcd);

    for (count = 0; va_arg(var, char *); count++)
	va_arg(var, XPointer);

    va_end(var);

    arg_list = (XlcArgList) Xmalloc(sizeof(XlcArg) * count + 1);
    if (arg_list == (XlcArgList) NULL)
	return (char *) NULL;
    
    Va_start(var, lcd);

    for (list = arg_list; list->name = va_arg(var, char *); list++)
	list->value = va_arg(var, XPointer);

    va_end(var);

    ret = (*methods->get_values)(lcd, arg_list);

    Xfree(arg_list);

    return ret;
}

void
_XlcDestroyLC(lcd)
    XLCd lcd;
{
    XLCdPublicMethods methods = (XLCdPublicMethods) lcd->methods;

    (*methods->pub.destroy)(lcd);
}

XLCd
_XlcCreateLC(name, methods)
    char *name;
    XLCdMethods methods;
{
    XLCdPublicMethods pub_methods = (XLCdPublicMethods) methods;
    XLCd lcd;

    lcd = (*pub_methods->pub.create)(name, methods);
    if (lcd == NULL)
	return (XLCd) NULL;

    if (lcd->core->name == NULL) {
	lcd->core->name = (char*) Xmalloc(strlen(name) + 1);
	if (lcd->core->name == NULL) 
	    goto err;
	strcpy(lcd->core->name, name);
    }
    
    if (lcd->methods == NULL)
	lcd->methods = methods;

    if ((*pub_methods->pub.initialize)(lcd) == False)
	goto err;
    
    return lcd;

err:
    _XlcDestroyLC(lcd);

    return (XLCd) NULL;
}
