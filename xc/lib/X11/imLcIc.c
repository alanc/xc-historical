/* $XConsortium$ */
/******************************************************************

                Copyright 1992 by FUJITSU LIMITED

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
#include "Xlcint.h"
#include "Xlibint.h"
#include "Ximint.h"

Private void		_XimLocalDestroyIC( );
Private void		_XimLocalSetFocus( );
Private void		_XimLocalUnSetFocus( );
Private char *		_XimLocalMbReset( );
Private wchar_t *	_XimLocalWcReset( );
extern char *		_XimLocalSetICValues( );
extern char *		_XimLocalGetICValues( );
extern int		_XimLocalMbLookupString( );
extern int		_XimLocalWcLookupString( );
extern Bool		_XimLocalFilter( );
extern char *		_XimSetICValueData( );

Private XICMethodsRec Local_ic_methods = {
    _XimLocalDestroyIC, 	/* destroy */
    _XimLocalSetFocus,  	/* set_focus */
    _XimLocalUnSetFocus,	/* unset_focus */
    _XimLocalSetICValues,	/* set_values */
    _XimLocalGetICValues,	/* get_values */
    _XimLocalMbReset,		/* mb_reset */
    _XimLocalWcReset,		/* wc_reset */
    _XimLocalMbLookupString,	/* mb_lookup_string */
    _XimLocalWcLookupString,	/* wc_lookup_string */
};

XIC
_XimLocalCreateIC(im, values)
    XIM			 im;
    XIMArg		*values;
{
    Xic			 ic;

    if((ic = (Xic)Xmalloc(sizeof(XicRec))) == (Xic)NULL) {
	return ((XIC)NULL);
    }
    bzero((char *)ic,      sizeof(XicRec));

    ic->methods = &Local_ic_methods;
    ic->core.im = im;
    ic->core.filter_events = KeyPressMask;
    ic->private.local.context    = ((Xim)im)->private.local.top;
    ic->private.local.composed   = (DefTree *)NULL;

    if(_XimSetICValueData(ic, values,
		XIM_CREATEIC, &(ic->private.local.value_mask)))
	goto Set_Error;

    /* The Value must be set */
    if(!(ic->private.local.value_mask & XIM_INPUTSTYLE))
	goto Set_Error;
    if(!(   (ic->core.input_style == (XIMPreeditNothing | XIMStatusNothing))
         || (ic->core.input_style == (XIMPreeditNone | XIMStatusNone)) ) )
	goto Set_Error;

    if(ic->core.input_style & XIMPreeditCallbacks)
	/* Preedit Callback */
	if(!(ic->private.local.value_mask & XIM_PREEDITCALLBACK))
	    goto Set_Error;
    if(ic->core.input_style & XIMStatusCallbacks)
	/* Status Callback */
	if(!(ic->private.local.value_mask & XIM_STATUSCALLBACK))
	    goto Set_Error;

    _XRegisterFilterByType(ic->core.im->core.display, ic->core.focus_window,
			   KeyPress, KeyPress, _XimLocalFilter, (XPointer)ic);
    return ((XIC)ic);

Set_Error :
    Xfree(ic);
    return((XIC)NULL);
}

Private void
_XimLocalDestroyIC(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    if(((Xim)ic->core.im)->private.local.current_ic == (XIC)ic) {
	_XimLocalUnSetFocus(ic);
    }
    return;
}

Private void
_XimLocalSetFocus(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    XIC	 current_ic = ((Xim)ic->core.im)->private.local.current_ic;

    if (current_ic != (XIC)NULL) {
	_XimLocalUnSetFocus(current_ic);
    }
    ((Xim)ic->core.im)->private.local.current_ic = (XIC)ic;

    _XRegisterFilterByType(ic->core.im->core.display, ic->core.focus_window,
			KeyPress, KeyPress, _XimLocalFilter, (XPointer)ic);
    return;
}

Private void
_XimLocalUnSetFocus(xic)
    XIC	 xic;
{
    Xic  ic = (Xic)xic;
    ((Xim)ic->core.im)->private.local.current_ic = (XIC)NULL;

    _XUnregisterFilter(ic->core.im->core.display, ic->core.focus_window,
			_XimLocalFilter, (XPointer)ic);
    return;
}

Private char *
_XimLocalMbReset(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    ic->private.local.composed = (DefTree *)NULL;
    ic->private.local.context  = ((Xim)ic->core.im)->private.local.top;
    return((char *)NULL);
}

Private wchar_t *
_XimLocalWcReset(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    ic->private.local.composed = (DefTree *)NULL;
    ic->private.local.context  = ((Xim)ic->core.im)->private.local.top;
    return((wchar_t *)NULL);
}
