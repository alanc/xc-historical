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
#include <X11/Xmd.h>
#include <X11/Xutil.h>
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Private Bool
_XimPreSetAttributes(ic, vl, mask, return_name)
    Xic			 ic;
    XIMArg		*vl;
    int		 	*mask;
    char		*return_name;
{
    XIMArg		*p;
    XStandardColormap	*colormap_ret;
    int			 list_ret;
    XFontStruct		**struct_list;
    char		**name_list;
    int 		 i, len;
    int			 count;
    char 		*tmp;

    for(p = vl; p && p->name != NULL; p++) {
	if(strcmp(p->name, XNArea)==0) {
	    ic->core.preedit_attr.area.x = ((XRectangle *)p->value)->x;
	    ic->core.preedit_attr.area.y = ((XRectangle *)p->value)->y;
	    ic->core.preedit_attr.area.width = ((XRectangle *)p->value)->width;
	    ic->core.preedit_attr.area.height = ((XRectangle *)p->value)->height;
	    
	} else if(strcmp(p->name, XNAreaNeeded)==0) {
	    ic->core.preedit_attr.area_needed.width  = ((XRectangle *)p->value)->width;
	    ic->core.preedit_attr.area_needed.height = ((XRectangle *)p->value)->height;
	} else if(strcmp(p->name, XNSpotLocation)==0) {
	    ic->core.preedit_attr.spot_location.x = ((XPoint *)p->value)->x;
	    ic->core.preedit_attr.spot_location.y = ((XPoint *)p->value)->y;
	} else if(strcmp(p->name, XNColormap)==0) {
	    ic->core.preedit_attr.colormap = (Colormap)p->value;
	} else if(strcmp(p->name, XNStdColormap)==0) {
	    if( XGetRGBColormaps(ic->core.im->core.display,
				 ic->core.focus_window, &colormap_ret,
				 &count, (Atom)p->value) != 0) {
		ic->core.preedit_attr.std_colormap = (Atom)p->value;
	    } else {
		return_name = p->name;
		return(False);
	    }
	    
	} else if(strcmp(p->name, XNBackground)==0) {
	    ic->core.preedit_attr.background = (unsigned long)p->value;
	} else if(strcmp(p->name, XNForeground)==0) {
	    ic->core.preedit_attr.foreground = (unsigned long)p->value;
	} else if(strcmp(p->name, XNBackgroundPixmap)==0) {
	    ic->core.preedit_attr.background_pixmap = (Pixmap)p->value;
	} else if(strcmp(p->name, XNFontSet)==0) {
	    ic->core.preedit_attr.fontset = (XFontSet)p->value;
	} else if(strcmp(p->name, XNLineSpace)==0) {
	    ic->core.preedit_attr.line_space = (long)p->value;
	} else if(strcmp(p->name, XNCursor)==0) {
	    ic->core.preedit_attr.cursor = (Cursor)p->value;
	} else if(strcmp(p->name, XNPreeditStartCallback)==0) {
	    ic->core.preedit_attr.callbacks.start.client_data =
		((XIMCallback *)p->value)->client_data;
	    ic->core.preedit_attr.callbacks.start.callback =
		((XIMCallback *)p->value)->callback;
	    *mask |= XIM_PREEDITCALLBACK;
	} else if(strcmp(p->name, XNPreeditDoneCallback)==0) {
	    ic->core.preedit_attr.callbacks.done.client_data =
		((XIMCallback *)p->value)->client_data;
	    ic->core.preedit_attr.callbacks.done.callback =
		((XIMCallback *)p->value)->callback;
	    *mask |= XIM_PREEDITCALLBACK;
	} else if(strcmp(p->name, XNPreeditDrawCallback)==0) {
	    ic->core.preedit_attr.callbacks.draw.client_data =
		((XIMCallback *)p->value)->client_data;
	    ic->core.preedit_attr.callbacks.draw.callback =
		((XIMCallback *)p->value)->callback;
	    *mask |= XIM_PREEDITCALLBACK;
	} else if(strcmp(p->name, XNPreeditCaretCallback)==0) {
	    ic->core.preedit_attr.callbacks.caret.client_data =
		((XIMCallback *)p->value)->client_data;
	    ic->core.preedit_attr.callbacks.caret.callback =
		((XIMCallback *)p->value)->callback;
	    *mask |= XIM_PREEDITCALLBACK;
	}
    }
    return(True);
}

Private Bool
_XimStatusSetAttributes(ic, vl, mask, return_name)
    Xic			 ic;
    XIMArg		*vl;
    int			*mask;
    char		*return_name;
{
    XIMArg		*p;
    XStandardColormap 	*colormap_ret;
    int			 list_ret;
    XFontStruct		**struct_list;
    char		**name_list;
    int 		 i, len;
    int			 count;
    char 		*tmp;

    for(p = vl; p && p->name != NULL; p++) {
	if(strcmp(p->name, XNArea)==0) {
	    ic->core.status_attr.area.x = ((XRectangle *)p->value)->x;
	    ic->core.status_attr.area.y = ((XRectangle *)p->value)->y;
	    ic->core.status_attr.area.width = ((XRectangle *)p->value)->width;
	    ic->core.status_attr.area.height = ((XRectangle *)p->value)->height;
	} else if(strcmp(p->name, XNAreaNeeded)==0) {
	    ic->core.status_attr.area_needed.width  = ((XRectangle *)p->value)->width;
	    ic->core.status_attr.area_needed.height = ((XRectangle *)p->value)->height;
	} else if(strcmp(p->name, XNColormap)==0) {
	    ic->core.status_attr.colormap = (Colormap)p->value;
	} else if(strcmp(p->name, XNStdColormap)==0) {
	    if(XGetRGBColormaps(ic->core.im->core.display,
				ic->core.focus_window, &colormap_ret,
				&count, (Atom)p->value) !=0) {
		ic->core.status_attr.std_colormap = (Atom)p->value;
	    } else {
		return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNBackground)==0) {
	    ic->core.status_attr.background = (unsigned long)p->value;
	} else if(strcmp(p->name, XNForeground)==0) {
	    ic->core.status_attr.foreground = (unsigned long)p->value;
	} else if(strcmp(p->name, XNBackgroundPixmap)==0) {
	    ic->core.status_attr.background_pixmap = (Pixmap)p->value;
	} else if(strcmp(p->name, XNFontSet)==0) {
	    ic->core.status_attr.fontset = (XFontSet)p->value;
	} else if(strcmp(p->name, XNLineSpace)==0) {
	    ic->core.status_attr.line_space = (long)p->value;
	} else if(strcmp(p->name, XNCursor)==0) {
	    ic->core.status_attr.cursor = (Cursor)p->value;
	} else if(strcmp(p->name, XNStatusStartCallback)==0) {
	    ic->core.status_attr.callbacks.start.client_data =
				((XIMCallback *)p->value)->client_data;
	    ic->core.status_attr.callbacks.start.callback =
				((XIMCallback *)p->value)->callback;
	    *mask |= XIM_STATUSCALLBACK;
	} else if(strcmp(p->name, XNStatusDoneCallback)==0) {
	    ic->core.status_attr.callbacks.done.client_data =
				((XIMCallback *)p->value)->client_data;
	    ic->core.status_attr.callbacks.done.callback =
				((XIMCallback *)p->value)->callback;
	    *mask |= XIM_STATUSCALLBACK;
	} else if(strcmp(p->name, XNStatusDrawCallback)==0) {
	    ic->core.status_attr.callbacks.draw.client_data =
				((XIMCallback *)p->value)->client_data;
	    ic->core.status_attr.callbacks.draw.callback =
				((XIMCallback *)p->value)->callback;
	    *mask |= XIM_STATUSCALLBACK;
	}
    }
    return(True);
}

char *
_XimSetICValueData(ic, values, mode, mask)
    Xic		 ic;
    XIMArg	*values;
    int		 mode;
    int		*mask;
{
    XIMArg	*p;
    char	*return_name = NULL;

    for(p = values; p->name != NULL; p++) {
	if(strcmp(p->name, XNInputStyle) == 0) {
	    if(mode == XIM_CREATEIC) {
		ic->core.input_style = (XIMStyle)p->value;
		*mask |= XIM_INPUTSTYLE;
	    } else {
		; /* Currently Fixed value */
	    }
	} else if(strcmp(p->name, XNClientWindow)==0) {
	    if(ic->core.client_window == (Window)NULL) {
		ic->core.client_window = (Window)p->value;
		ic->core.focus_window  = ic->core.client_window;
		*mask |= (XIM_CLIENTWINDOW | XIM_FOCUSWINDOW);
	    } else {
		return_name = p->name;
		break; /* Can't change this value */
	    }
	} else if(strcmp(p->name, XNFocusWindow)==0) {
	    ic->core.focus_window = (Window)p->value;
	    *mask |= XIM_FOCUSWINDOW;
	} else if(strcmp(p->name, XNResourceName)==0) {
	    ic->core.im->core.res_name = (char *)p->value;
	    *mask |= XIM_RESOURCENAME;
	} else if(strcmp(p->name, XNResourceClass)==0) {
	    ic->core.im->core.res_class = (char *)p->value;
	    *mask |= XIM_RESOURCECLASS;
	} else if(strcmp(p->name, XNGeometryCallback)==0) {
	    ic->core.geometry_callback.client_data =
		((XIMCallback *)p->value)->client_data;
	    ic->core.geometry_callback.callback =
		((XIMCallback *)p->value)->callback;
	    *mask |= XIM_GEOMETRYCALLBACK;
	} else if(strcmp(p->name, XNPreeditAttributes)==0) {
	   if(_XimPreSetAttributes(ic, (XIMArg *)(p->value), mask, return_name)
	       == False)
		break;
	} else if(strcmp(p->name, XNStatusAttributes)==0) {
	    if(_XimStatusSetAttributes(ic, (XIMArg *)p->value, mask,
				       return_name) == False)
		break;
	} else {
	    break;
	}
    }
    return(return_name);
}

char *
_XimLocalSetICValues(ic, values)
    Xic		 ic;
    XIMArg	*values;
{
    /*
     * Not yet
     */
     return NULL;
}

