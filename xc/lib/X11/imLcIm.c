/* $XConsortium: imLcIm.c,v 1.1 93/09/17 13:27:06 rws Exp $ */
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
#include <ctype.h>

Public  char *		_XimLocalGetIMValues( );
Public  Status		_XimLocalCloseIM( );
extern  XIC		_XimLocalCreateIC( );

Private XIMMethodsRec      Xim_im_local_methods = {
    _XimLocalCloseIM,           /* close */
    _XimLocalGetIMValues,       /* get_values */
    _XimLocalCreateIC,          /* create_ic */
};

extern Bool _XimLocalProcessingResource(
#if NeedFunctionPrototypes
    Xim	im
#endif
);

extern int XimParseStringFile(
#if NeedFunctionPrototypes
    FILE	*fp,
    DefTree	**ptop
#endif
);

extern void _XimDestroyIMStructureList(
#if NeedFunctionPrototypes
    Xim	im
#endif
);

extern FILE *_XlcOpenLocaleFile(
#if NeedFunctionPrototypes
    char *dir,
    char *locale,
    char *name
#endif
);

Bool
_XimCheckIfLocalProcessing(im)
    Xim          im;
{
    FILE        *fp;
    char	*language, *codeset;
    char         buf[BUFSIZE];
    int          i, len;

    if(strcmp(im->core.im_name, "") == 0) {
	_XlcGetLCValues(im->core.lcd, XlcNLanguage, &language, NULL);
	fp = _XlcOpenLocaleFile(NULL, language, COMPOSE_FILE);
	if( fp != NULL ) {
	    fclose( fp );
	    return(True);
	}
        /* XXX --- */
	_XlcGetLCValues(im->core.lcd, XlcNCodeset, &codeset, NULL);
        len = strlen(codeset);
        for (i = 0; i < len; i++)
	    buf[i] = (char)tolower((int)codeset[i]);
        buf[i] = '\0';
	fp = _XlcOpenLocaleFile(NULL, buf, COMPOSE_FILE);
	if( fp != NULL ) {
	    fclose( fp );
	    return(True);
	}
        /* --- XXX */
	if(im->core.rdb != NULL)
	    return(_XimLocalProcessingResource(im));
	return(False);
    } else if(strcmp(im->core.im_name, "local") == 0 ||
	      strcmp(im->core.im_name, "none" ) == 0 ) {
	return(True);
    }
    return(False);
}

Private void
_XimCreateDefaultTree(im)
    Xim		im;
{
    FILE *fp;
    char *language, *codeset;
    char         buf[BUFSIZE];
    int          i, len;

    _XlcGetLCValues(im->core.lcd, XlcNLanguage, &language, NULL);
    fp = _XlcOpenLocaleFile(NULL, language, COMPOSE_FILE);
    im->private.local.top = (DefTree *)NULL;
    if (fp == (FILE *)NULL) {
        /* XXX --- */
	_XlcGetLCValues(im->core.lcd, XlcNCodeset, &codeset, NULL);
        len = strlen(codeset);
        for (i = 0; i < len; i++)
	    buf[i] = (char)tolower((int)codeset[i]);
        buf[i] = '\0';
	fp = _XlcOpenLocaleFile(NULL, buf, COMPOSE_FILE);
        if (fp == (FILE *)NULL) return;
        /* --- XXX */
    }
    (void)XimParseStringFile(fp, &im->private.local.top);
    fclose(fp);
}

Bool
_XimLocalOpenIM(im)
    Xim		 im;
{
    im->methods = &Xim_im_local_methods;

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

    _XimCreateDefaultTree(im);

    im->private.local.current_ic = (XIC)NULL;
    return(True);
}

Public void
XimFreeDefaultTree(top)
    DefTree *top;
{
    if (!top) return;
    if (top->succession) XimFreeDefaultTree(top->succession);
    if (top->next) XimFreeDefaultTree(top->next);
    if (top->mb) Xfree(top->mb);
    if (top->wc) Xfree(top->wc);
    Xfree(top);
}

Public void
_XimLocalIMFree(im)
    Xim		im;
{
    XimFreeDefaultTree(im->private.local.top);
    Xfree(im->private.local.styles->supported_styles);
    Xfree(im->private.local.styles);
    if(im->core.im_name)
	Xfree(im->core.im_name);
    return;
}

Public Status
_XimLocalCloseIM(xim)
    XIM		xim;
{
    Xim		im = (Xim)xim;
    XIC		ic;

    while(ic = im->core.ic_chain)
	XDestroyIC(ic);
    _XimLocalIMFree(im);
    _XimDestroyIMStructureList(im);
    return(True);
}

Public char *
_XimLocalGetIMValues(xim, values)
    XIM		 xim;
    XIMArg	*values;
{
    Xim		 im = (Xim)xim;
    XIMArg	*p;
    XIMStyles	**value;
    XIMStyles	*styles;
    unsigned short	i;

    for(p = values; p->name != NULL; p++) {
	if(strcmp(p->name, XNQueryInputStyle) == 0) {
	    if((styles = (XIMStyles *)Xmalloc(sizeof(XIMStyles) +
	        sizeof(XIMStyle) *
		im->private.local.styles->count_styles)) == NULL) {
		break;
	    }
	    styles->count_styles = im->private.local.styles->count_styles;
	    styles->supported_styles = (XIMStyle *)((char *)styles + sizeof(XIMStyles));
	    for(i=0; i < styles->count_styles; i++) {
		styles->supported_styles[i] = 
			im->private.local.styles->supported_styles[i];
	    }
	    value = (XIMStyles **)p->value;
	    *value = styles;
	} else {
	    break;
	}
    }
    return(p->name);
}
