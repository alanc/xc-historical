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
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include "Xlibint.h"
#include "Xlcint.h"
#include "XlcPublic.h"
#include "Ximint.h"

Public  char *		_XimLocalGetIMValues( );
Public  Status		_XimThaiCloseIM( );
extern  XIC		_XimThaiCreateIC( );

Private XIMMethodsRec      Xim_im_thai_methods = {
    _XimThaiCloseIM,           /* close */
    _XimLocalGetIMValues,       /* get_values */
    _XimThaiCreateIC,          /* create_ic */
};

extern Bool _XimLocalProcessingResource(
#if NeedFunctionPrototypes
    Xim	im
#endif
);

#define THAI_LANGUAGE_NAME 	"th"

Bool
_XimCheckIfThaiProcessing(im)
    Xim          im;
{
    char *language;

    _XlcGetLCValues(im->core.lcd, XlcNLanguage, &language, NULL);
    if(strcmp(language, THAI_LANGUAGE_NAME) == 0) {
	return(True);
    }
    return(False);
}

Bool
_XimThaiOpenIM(im)
    Xim		 im;
{
    im->methods = &Xim_im_thai_methods;

    im->private.local.styles = (XIMStyles *)Xmalloc(sizeof(XIMStyles));
    if(im->private.local.styles) {
	im->private.local.styles->supported_styles = 
			(XIMStyle *)Xmalloc(sizeof(XIMStyle) * 2);
	im->private.local.styles->count_styles = 2;
	if(im->private.local.styles->supported_styles == NULL) {
	    Xfree(im->private.local.styles);
	    im->private.local.styles = NULL;
	    return(False);
	}
	im->private.local.styles->supported_styles[0] =
					XIMPreeditNone | XIMStatusNone;
	im->private.local.styles->supported_styles[1] =
					XIMPreeditNothing | XIMStatusNothing;
    } else {
	return(False);
    }

    im->private.local.current_ic = (XIC)NULL;
    return(True);
}

Public void
_XimThaiIMFree(im)
    Xim		im;
{
    Xfree(im->private.local.styles->supported_styles);
    Xfree(im->private.local.styles);
    if(im->core.im_name)
	Xfree(im->core.im_name);
    return;
}

Public Status
_XimThaiCloseIM(xim)
    XIM		xim;
{
    Xim		im = (Xim)xim;
    XIC		ic;

    while(ic = im->core.ic_chain)
	XDestroyIC(ic);
    _XimThaiIMFree(im);
    return(True);
}
