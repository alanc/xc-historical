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

#define NEED_EVENTS
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Private Bool
_XimPreGetAttributes(ic, vl, return_name)
    Xic		 ic;
    XIMArg	*vl;
    char	**return_name;
{
    XIMArg	*p;
    XRectangle	*p_rect;
    XPoint	*p_point;
    XIMCallback	*p_callback;

    for(p = vl; p->name != NULL; p++) {
	if(strcmp(p->name, XNArea) == 0) {
	    if((p_rect = (XRectangle *)Xmalloc(sizeof(XRectangle))) == NULL) {
		*return_name = p->name;
		return(False);
	    }
	    p_rect->x       = ic->core.preedit_attr.area.x;
	    p_rect->y       = ic->core.preedit_attr.area.y;
	    p_rect->width   = ic->core.preedit_attr.area.width;
	    p_rect->height  = ic->core.preedit_attr.area.height;
	    *((XRectangle **)(p->value)) = p_rect;

	} else if(strcmp(p->name, XNAreaNeeded) == 0) {
	    if((p_rect = (XRectangle *)Xmalloc(sizeof(XRectangle))) == NULL) {
		*return_name = p->name;
		return(False);
	    }
	    p_rect->x  = p_rect->y  = 0;
	    p_rect->width   = ic->core.preedit_attr.area_needed.width;
	    p_rect->height  = ic->core.preedit_attr.area_needed.height;
	    *((XRectangle **)(p->value)) = p_rect;

	} else if(strcmp(p->name, XNSpotLocation) == 0) {
	    if((p_point = (XPoint *)Xmalloc(sizeof(XPoint))) == NULL) {
		*return_name = p->name;
		return(False);
	    }
	    p_point->x = ic->core.preedit_attr.spot_location.x;
	    p_point->y = ic->core.preedit_attr.spot_location.y;
	    *((XPoint **)(p->value)) = p_point;

	} else if(strcmp(p->name, XNColormap) == 0) {
	    *((Colormap *)(p->value)) = ic->core.preedit_attr.colormap;

	} else if(strcmp(p->name, XNStdColormap) == 0) {
	    *((Atom *)(p->value)) = ic->core.preedit_attr.std_colormap;

	} else if(strcmp(p->name, XNBackground) == 0) {
	    *((unsigned long *)(p->value)) = ic->core.preedit_attr.background;

	} else if(strcmp(p->name, XNForeground) == 0) {
	    *((unsigned long *)(p->value)) = ic->core.preedit_attr.foreground;

	} else if(strcmp(p->name, XNBackgroundPixmap) == 0) {
	    *((Pixmap *)(p->value)) = ic->core.preedit_attr.background_pixmap;

	} else if(strcmp(p->name, XNFontSet) == 0) {
	    *((XFontSet *)(p->value)) = ic->core.preedit_attr.fontset;

	} else if(strcmp(p->name, XNLineSpace) == 0) {
	    *((int *)(p->value)) = ic->core.preedit_attr.line_space;

	} else if(strcmp(p->name, XNCursor) == 0) {
	    *((Cursor *)(p->value)) = ic->core.preedit_attr.cursor;

	} else if(strcmp(p->name, XNPreeditStartCallback) == 0) {
	    if((int)ic->core.preedit_attr.callbacks.start.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.preedit_attr.callbacks.start.client_data;
		p_callback->callback =
		    ic->core.preedit_attr.callbacks.start.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNPreeditDrawCallback) == 0) {
	    if((int)ic->core.preedit_attr.callbacks.draw.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.preedit_attr.callbacks.draw.client_data;
		p_callback->callback =
		    ic->core.preedit_attr.callbacks.draw.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNPreeditDoneCallback) == 0) {
	    if((int)ic->core.preedit_attr.callbacks.done.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.preedit_attr.callbacks.done.client_data;
		p_callback->callback =
		    ic->core.preedit_attr.callbacks.done.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNPreeditCaretCallback) == 0) {
	    if((int)ic->core.preedit_attr.callbacks.caret.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.preedit_attr.callbacks.caret.client_data;
		p_callback->callback =
		    ic->core.preedit_attr.callbacks.caret.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }
	} else {
	    *return_name = p->name;
	    return(False);
	}
    }
    return(True);
}

Private Bool
_XimStatusGetAttributes(ic, vl, return_name)
    Xic		 ic;
    XIMArg 	*vl;
    char	**return_name;
{
    XIMArg	*p;
    XRectangle	*p_rect;
    XIMCallback	*p_callback;

    for(p = vl; p->name != NULL; p++) {
	if(strcmp(p->name, XNArea) == 0) {
	    if((p_rect = (XRectangle *)Xmalloc(sizeof(XRectangle))) == NULL) {
		*return_name = p->name;
		return(False);
	    }
	    p_rect->x       = ic->core.status_attr.area.x;
	    p_rect->y       = ic->core.status_attr.area.y;
	    p_rect->width   = ic->core.status_attr.area.width;
	    p_rect->height  = ic->core.status_attr.area.height;
	    *((XRectangle **)(p->value)) = p_rect;

	} else if(strcmp(p->name, XNAreaNeeded) == 0) {
	    if((p_rect = (XRectangle *)Xmalloc(sizeof(XRectangle))) == NULL) {
		*return_name = p->name;
		return(False);
	    }
	    p_rect->x  = p_rect->y  = 0;
	    p_rect->width   = ic->core.status_attr.area_needed.width;
	    p_rect->height  = ic->core.status_attr.area_needed.height;
	    *((XRectangle **)(p->value)) = p_rect;

	} else if(strcmp(p->name, XNColormap) == 0) {
	    *((Colormap *)(p->value)) = ic->core.status_attr.colormap;

	} else if(strcmp(p->name, XNStdColormap) == 0) {
	    *((Atom *)(p->value)) = ic->core.status_attr.std_colormap;

	} else if(strcmp(p->name, XNBackground) == 0) {
	    *((unsigned long *)(p->value)) = ic->core.status_attr.background;

	} else if(strcmp(p->name, XNForeground) == 0) {
	    *((unsigned long *)(p->value)) = ic->core.status_attr.foreground;

	} else if(strcmp(p->name, XNBackgroundPixmap) == 0) {
	    *((Pixmap *)(p->value)) = ic->core.status_attr.background_pixmap;

	} else if(strcmp(p->name, XNFontSet) == 0) {
	    *((XFontSet *)(p->value)) = ic->core.status_attr.fontset;

	} else if(strcmp(p->name, XNLineSpace) == 0) {
	    *((int *)(p->value)) = ic->core.status_attr.line_space;

	} else if(strcmp(p->name, XNCursor) == 0) {
	    *((Cursor *)(p->value)) = ic->core.status_attr.cursor;

	} else if(strcmp(p->name, XNStatusStartCallback) == 0) {
	    if((int)ic->core.status_attr.callbacks.start.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.status_attr.callbacks.start.client_data;
		p_callback->callback =
		    ic->core.status_attr.callbacks.start.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNStatusDrawCallback) == 0) {
	    if((int)ic->core.status_attr.callbacks.draw.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.status_attr.callbacks.draw.client_data;
		p_callback->callback =
		    ic->core.status_attr.callbacks.draw.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }

	} else if(strcmp(p->name, XNStatusDoneCallback) == 0) {
	    if((int)ic->core.status_attr.callbacks.done.callback) {
		if((p_callback =
			(XIMCallback *)Xmalloc(sizeof(XIMCallback))) == NULL) {
		    *return_name = p->name;
		    return(False);
		}
		p_callback->client_data =
		    ic->core.status_attr.callbacks.done.client_data;
		p_callback->callback =
		    ic->core.status_attr.callbacks.done.callback;
		*((XIMCallback **)(p->value)) = p_callback;
	    } else {
		*return_name = p->name;
		return(False);
	    }
	} else {
	    *return_name = p->name;
	    return(False);
	}
    }
    return(True);
}

char *
_XimLocalGetICValues(ic, values)
    Xic		 ic;
    XIMArg	*values;
{
    XIMArg	*p;
    char	*p_char;
    char	*return_name = NULL;
    int		 len;

    for(p = values; p->name != NULL; p++) {
	if(strcmp(p->name, XNInputStyle) == 0) {
	    if(ic->private.local.value_mask & XIM_INPUTSTYLE) {
		*((XIMStyle *)(p->value)) = ic->core.input_style;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNClientWindow)==0) {
	    if(ic->private.local.value_mask & XIM_CLIENTWINDOW) {
		*((Window *)(p->value)) = ic->core.client_window;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNFocusWindow)==0) {
	    if(ic->private.local.value_mask & XIM_FOCUSWINDOW) {
		*((Window *)(p->value)) = ic->core.focus_window;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNResourceName)==0) {
	    if(ic->core.im->core.res_name != (char *)NULL) {
		    len = strlen(ic->core.im->core.res_name);
		if((p_char = (char *)Xmalloc(len+1)) == NULL) {
		    return_name = p->name;
		    break;
		}
		strcpy(p_char, ic->core.im->core.res_name);
		*((char **)(p->value)) = p_char;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNResourceClass)==0) {
	    if(ic->core.im->core.res_class != (char *)NULL) {
		len = strlen(ic->core.im->core.res_class);
		if((p_char = (char *)Xmalloc(len+1)) == NULL) {
		    return_name = p->name;
		    break;
		}
		strcpy(p_char, ic->core.im->core.res_class);
		*((char **)(p->value)) = p_char;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNGeometryCallback)==0) {
	    if(ic->private.local.value_mask & XIM_GEOMETRYCALLBACK) {
		*((XIMCallback *)(p->value)) = ic->core.geometry_callback;
	    } else {
		return_name = p->name;
		break;
	    }
	} else if(strcmp(p->name, XNFilterEvents)==0) {
	    *((unsigned long *)(p->value)) = ic->core.filter_events;
	} else if(strcmp(p->name, XNPreeditAttributes)==0) {
	    if( _XimPreGetAttributes(ic, (XIMArg *)(p->value), &return_name) == False)
		break;
	} else if(strcmp(p->name, XNStatusAttributes)==0) {
	    if( _XimStatusGetAttributes(ic, (XIMArg *)(p->value), &return_name) == False)
		break;
	} else {
	    return_name = p->name;
	    break;
	}
    }
    return(return_name);
}
