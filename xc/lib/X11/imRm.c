/* $XConsortium$ */
/******************************************************************

          Copyright 1990, 1991, 1992 by FUJITSU LIMITED

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
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"
#include "Xresource.h"

#ifndef	isalnum
#define	isalnum(c)	\
    (('0' <= (c) && (c) <= '9')  || \
     ('A' <= (c) && (c) <= 'Z')  || \
     ('a' <= (c) && (c) <= 'z'))
#endif

Private void
_XimGetResourceName(im, res_name, res_class)
    Xim		 im;
    char	*res_name;
    char	*res_class;
{
    if(im->core.res_name == NULL) {
	strcpy(res_name, "*");
    } else {
	strcpy(res_name, im->core.res_name);
	strcat(res_name, ".");
    }
    if(im->core.res_class == NULL) {
	strcpy(res_class, "*");
    } else {
	strcpy(res_class, im->core.res_class);
	strcat(res_class, ".");
    }
    strcat(res_name, "xim.");
    strcat(res_class, "Xim.");
}

Public Bool
_XimLocalProcessingResource(im)
    Xim		 im;
{
    char	 res_name[256];
    char	 res_class[256];
    char	*str_type;
    XrmValue	 value;

    _XimGetResourceName(im, res_name, res_class);
    strcat(res_name, "localProcessing");
    strcat(res_class, "LocalProcessing");
    if(XrmGetResource(im->core.rdb, res_name, res_class,
		      &str_type, &value) == True) { 
	if(strcmp(value.addr, "True") == 0 || 
	   strcmp(value.addr, "true") == 0 || 
	   strcmp(value.addr, "Yes") == 0 || 
	   strcmp(value.addr, "yes") == 0 || 
	   strcmp(value.addr, "ON") == 0 || 
	   strcmp(value.addr, "on") == 0) {
	    return(True);
	}
    }
    return(False);
}

