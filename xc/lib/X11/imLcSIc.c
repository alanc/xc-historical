/* $XConsortium: imLcSIc.c,v 1.1 93/09/17 13:27:27 rws Exp $ */
/******************************************************************

          Copyright 1990, 1991, 1992, 1993 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of FUJITSU LIMITED
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
FUJITSU LIMITED makes no representations about the suitability of
this software for any purpose. 
It is provided "as is" without express or implied warranty.

FUJITSU LIMITED DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL FUJITSU LIMITED BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author: Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Public char *
_XimSetICValueData(ic, top, values, mode)
    Xic			 ic;
    XPointer		 top;
    XIMArg		*values;
    unsigned long	 mode;
{
    register  XIMArg	*p;
    XIMResourceList	 res;
    char		*name;
    int			 check;
    XrmQuark		 pre_quark;
    XrmQuark		 sts_quark;

    pre_quark = XrmStringToQuark(XNPreeditAttributes);
    sts_quark = XrmStringToQuark(XNStatusAttributes);

    for(p = values; p->name != NULL; p++) {
	if((res = _XimGetICResourceListRec(ic, p->name))
						 == (XIMResourceList)NULL) {
	    return(p->name);
	}
	if(res->xrm_name == pre_quark) {
	    if(name = _XimSetICValueData(ic,
			(XPointer)(&((XimDefICValues *)top)->preedit_attr),
			(XIMArg *)p->value, (mode | XIM_PREEDIT_ATTR))) {
		return(name);
	    }
	} else if(res->xrm_name == sts_quark) {
	    if(name = _XimSetICValueData(ic,
			(XPointer)(&((XimDefICValues *)top)->status_attr),
			(XIMArg *)p->value, (mode | XIM_STATUS_ATTR))) {
		return(name);
	    }
	} else {
	    check = _XimCheckICMode(res, mode);
	    if(check == XIM_CHECK_INVALID) {
		continue;
	    } else if(check == XIM_CHECK_ERROR) {
		return(p->name);
	    }

	    if(_XimDecodeLocalICAttr(res, top, p->value, mode) == False) {
		return(p->name);
	    }
	}
    }
    return(NULL);
}

Public char *
_XimLocalSetICValues(ic, values)
    Xic			 ic;
    XIMArg		*values;
{
    XimDefICValues	 ic_values;
    char		*name = (char *)NULL;

    _XimGetCurrentICValues(ic, &ic_values);
    name = _XimSetICValueData(ic, (XPointer)&ic_values,
						 values, XIM_SETICVALUES);
    _XimSetCurrentICValues(ic, &ic_values);
    return(name);
}
