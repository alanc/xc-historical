/* $XConsortium$ */
/******************************************************************

          Copyright 1992 by FUJITSU LIMITED
          Copyright 1993 by Digital Equipment Corporation

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of FUJITSU LIMITED and
Digital Equipment Corporation not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.  FUJITSU LIMITED and Digital Equipment Corporation
makes no representations about the suitability of this software for
any purpose.  It is provided "as is" without express or implied
warranty.

FUJITSU LIMITED AND DIGITAL EQUIPMENT CORPORATION DISCLAIM ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL 
FUJITSU LIMITED AND DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF 
THIS SOFTWARE.

  Author:    Takashi Fujiwara     FUJITSU LIMITED 
                               	  fujiwara@a80.tech.yk.fujitsu.co.jp
  Modifier:  Franky Ling          Digital Equipment Corporation
	                          frankyling@hgrd01.enet.dec.com

******************************************************************/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include "Xlcint.h"
#include "Xlibint.h"
#include "Ximint.h"

Private void		_XimThaiDestroyIC( );
Private void		_XimThaiSetFocus( );
Private void		_XimThaiUnSetFocus( );
Private char *		_XimThaiMbReset( );
Private wchar_t *	_XimThaiWcReset( );
extern char *		_XimLocalSetICValues( );
extern char *		_XimLocalGetICValues( );
extern int		_XimLocalMbLookupString( );
extern int		_XimLocalWcLookupString( );
extern Bool		_XimThaiFilter( );
extern char *		_XimSetICValueData( );

Private XICMethodsRec Thai_ic_methods = {
    _XimThaiDestroyIC, 	/* destroy */
    _XimThaiSetFocus,  	/* set_focus */
    _XimThaiUnSetFocus,	/* unset_focus */
    _XimLocalSetICValues,	/* set_values */
    _XimLocalGetICValues,	/* get_values */
    _XimThaiMbReset,		/* mb_reset */
    _XimThaiWcReset,		/* wc_reset */
    _XimLocalMbLookupString,	/* mb_lookup_string */
    _XimLocalWcLookupString,	/* wc_lookup_string */
};

XIC
_XimThaiCreateIC(im, values)
    XIM			 im;
    XIMArg		*values;
{
    Xic			 ic;

    if((ic = (Xic)Xmalloc(sizeof(XicRec))) == (Xic)NULL) {
	return ((XIC)NULL);
    }
    bzero((char *)ic,      sizeof(XicRec));

    ic->methods = &Thai_ic_methods;
    ic->core.im = im;
    ic->core.filter_events = KeyPressMask;
    if ((ic->private.local.context = (DefTree *)Xmalloc(sizeof(DefTree)))
		== (DefTree *)NULL)
	goto Set_Error;
    if ((ic->private.local.context->mb = (char *)Xmalloc(10))
		== (char *)NULL)
	goto Set_Error;
    if ((ic->private.local.context->wc = (wchar_t *)Xmalloc(10*sizeof(wchar_t)))
		== (wchar_t *)NULL)
	goto Set_Error;
    if ((ic->private.local.composed = (DefTree *)Xmalloc(sizeof(DefTree)))
	    == (DefTree *)NULL)
	goto Set_Error;
    if ((ic->private.local.composed->mb = (char *)Xmalloc(10))
		== (char *)NULL)
	goto Set_Error;
    if ((ic->private.local.composed->wc = (wchar_t *)Xmalloc(10*sizeof(wchar_t)))
		== (wchar_t *)NULL)
	goto Set_Error;

    ic->private.local.thai.comp_state = 0;
    ic->private.local.thai.keysym = 0;
    ic->private.local.thai.input_mode = 0;

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
			   KeyPress, KeyPress, _XimThaiFilter, (XPointer)ic);
    return ((XIC)ic);

Set_Error :
    Xfree(ic);
    return((XIC)NULL);
}

Private void
_XimThaiDestroyIC(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    if(((Xim)ic->core.im)->private.local.current_ic == (XIC)ic) {
	_XimThaiUnSetFocus(ic);
    }

    Xfree(ic->private.local.context->mb);
    Xfree(ic->private.local.context->wc);
    Xfree(ic->private.local.context);
    Xfree(ic->private.local.composed->mb);
    Xfree(ic->private.local.composed->wc);
    Xfree(ic->private.local.composed);
    return;
}

Private void
_XimThaiSetFocus(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    XIC	 current_ic = ((Xim)ic->core.im)->private.local.current_ic;

    if (current_ic != (XIC)NULL) {
	_XimThaiUnSetFocus(current_ic);
    }
    ((Xim)ic->core.im)->private.local.current_ic = (XIC)ic;

    _XRegisterFilterByType(ic->core.im->core.display, ic->core.focus_window,
			KeyPress, KeyPress, _XimThaiFilter, (XPointer)ic);
    return;
}

Private void
_XimThaiUnSetFocus(xic)
    XIC	 xic;
{
    Xic  ic = (Xic)xic;
    ((Xim)ic->core.im)->private.local.current_ic = (XIC)NULL;

    _XUnregisterFilter(ic->core.im->core.display, ic->core.focus_window,
			_XimThaiFilter, (XPointer)ic);
    return;
}

Private char *
_XimThaiMbReset(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    ic->private.local.thai.comp_state = 0;
    ic->private.local.thai.keysym = 0;
    ic->private.local.composed->mb[0] = '\0';
    ic->private.local.composed->wc[0] = 0;
    return((char *)NULL);
}

Private wchar_t *
_XimThaiWcReset(xic)
    XIC	 xic;
{
    Xic	 ic = (Xic)xic;
    ic->private.local.thai.comp_state = 0;
    ic->private.local.thai.keysym = 0;
    ic->private.local.composed->mb[0] = '\0';
    ic->private.local.composed->wc[0] = 0;
    return((wchar_t *)NULL);
}
