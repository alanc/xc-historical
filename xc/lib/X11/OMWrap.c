/* $XConsortium: OMWrap.c,v 1.2 94/02/06 15:05:37 rws Exp $ */
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
#include "Xlcint.h"

XOM
#if NeedFunctionPrototypes
XOpenOM(Display *dpy, XrmDatabase rdb, _Xconst char *res_name,
	_Xconst char *res_class)
#else
XOpenOM(dpy, rdb, res_name, res_class)
    Display *dpy;
    XrmDatabase rdb;
    char *res_name;
    char *res_class;
#endif
{
    XLCd lcd = _XOpenLC((char *) NULL);

    if (lcd == NULL)
	return (XOM) NULL;

    if (lcd->methods->open_om == NULL)
	_XInitOM(lcd);

    if (lcd->methods->open_om)
	return (*lcd->methods->open_om)(lcd, dpy, rdb, res_name, res_class);

    return (XOM) NULL;
}

Status
XCloseOM(om)
    XOM om;
{
    XOC oc, next;
    XLCd lcd = om->core.lcd;

    next = om->core.oc_list;

    while (oc = next) {
	next = oc->core.next;
	(*oc->methods->destroy)(oc);
    }

    om->core.oc_list = NULL;

    _XCloseLC(lcd);

    return (*om->methods->close)(om);
}

#if NeedVarargsPrototypes
char *
XSetOMValues(XOM om, ...)
#else
char *
XSetOMValues(om, va_alist)
    XOM om;
    va_dcl
#endif
{
    va_list var;
    XlcArgList args;
    char *ret;
    int num_args;

    Va_start(var, om);
    _XlcCountVaList(var, &num_args);
    va_end(var);

    Va_start(var, om);
    _XlcVaToArgList(var, num_args, &args);
    va_end(var);

    if (args == (XlcArgList) NULL)
	return (char *) NULL;

    ret = (*om->methods->set_values)(om, args, num_args);

    Xfree(args);

    return ret;
}

#if NeedVarargsPrototypes
char *
XGetOMValues(XOM om, ...)
#else
char *
XGetOMValues(om, va_alist)
    XOM om;
    va_dcl
#endif
{
    va_list var;
    XlcArgList args;
    char *ret;
    int num_args;

    Va_start(var, om);
    _XlcCountVaList(var, &num_args);
    va_end(var);

    Va_start(var, om);
    _XlcVaToArgList(var, num_args, &args);
    va_end(var);

    if (args == (XlcArgList) NULL)
	return (char *) NULL;
    
    ret = (*om->methods->get_values)(om, args, num_args);

    Xfree(args);

    return ret;
}

Display *
XDisplayOfOM(om)
    XOM om;
{
    return om->core.display;
}

char *
XLocaleOfOM(om)
    XOM om;
{
    return om->core.lcd->core->name;
}
