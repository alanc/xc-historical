/* $XConsortium: imRm.c,v 1.1 93/09/17 13:27:39 rws Exp $ */
/******************************************************************

	  Copyright 1990, 1991, 1992,1993 by FUJITSU LIMITED

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

typedef struct _XimValueOffsetInfo {
    char		*name;
    XrmQuark		 quark;
    unsigned int	 offset;
    Bool		 (*defaults)(
#if NeedFunctionPrototypes
	struct _XimValueOffsetInfo *, XPointer, XPointer, unsigned long
#endif
			 );
    Bool		 (*decode)(
#if NeedFunctionPrototypes
	struct _XimValueOffsetInfo *, XPointer, XPointer
#endif
			 );
    Bool		 (*encode)(
#if NeedFunctionPrototypes
	struct _XimValueOffsetInfo *, XPointer, XPointer
#endif
			 );
} XimValueOffsetInfoRec, *XimValueOffsetInfo;

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

static XIMICAttributes supported_local_icattributes[] = {
    XNInputStyle,
    XNClientWindow,
    XNFocusWindow,
    XNResourceName,
    XNResourceClass,
    XNGeometryCallback,
    XNFilterEvents,
    XNDestroyCallback,
    XNStringConversionCallback,
    XNStringConversion,
    XNResetState,
    XNResetReturn,
    XNHotKey,
    XNHotKeyState,
    XNPreeditAttributes,
    XNStatusAttributes,
    XNArea,
    XNAreaNeeded,
    XNSpotLocation,
    XNColormap,
    XNStdColormap,
    XNForeground,
    XNBackground,
    XNBackgroundPixmap,
    XNFontSet,
    XNLineSpace,
    XNCursor,
    XNPreeditStartCallback,
    XNPreeditDoneCallback,
    XNPreeditDrawCallback,
    XNPreeditCaretCallback,
    XNStatusStartCallback,
    XNStatusDoneCallback,
    XNStatusDrawCallback,
    XNPreeditState,
    (XIMICAttributes)NULL			/* dummy */
};

static XIMStyle supported_local_styles[] = {
    XIMPreeditNone	| XIMStatusNone,
    XIMPreeditNothing	| XIMStatusNothing,
    0						/* dummy */
};

Private  Bool
_XimDefaultStyles(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;			/* unused */
    unsigned long	  mode;			/* unused */
{
    XIMStyles		 *styles;
    XIMStyles		**out;
    register int	  i;
    unsigned int	  n;
    int			  len;
    char		 *tmp;

    n = XIMNumber(supported_local_styles) - 1;
    len = sizeof(XIMStyles) + sizeof(XIMStyle) * n;
    if((tmp = (char *)Xmalloc(len)) == (char *)NULL) {
	return(False);
    }
    bzero(tmp, len);

    styles = (XIMStyles *)tmp;
    if (n > 0) {
	styles->count_styles = (unsigned short)n;
	styles->supported_styles = (XIMStyle *)(tmp + sizeof(XIMStyles));
	for(i = 0; i < n; i++) {
	    styles->supported_styles[i] = supported_local_styles[i];
	}
    }

    out = (XIMStyles **)((char *)top + info->offset);
    *out = styles;
    return(True);
}

Private  Bool
_XimDefaultFocusWindow(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Window		*out;

    if(ic->core.client_window == (Window)NULL) {
	return(True);
    }

    out = (Window *)((char *)top + info->offset);
    *out = ic->core.client_window;
    return(True);
}

Private  Bool
_XimDefaultResName(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;
    unsigned long	  mode;
{
    Xic			  ic = (Xic)parm;
    Xim			  im = (Xim)ic->core.im;
    int			  len;
    char		 *name;
    char		**out;

    if(im->core.res_name == (char *)NULL) {
	return(True);
    }
    len = strlen(im->core.res_name);
    if((name = (char *)Xmalloc(len + 1)) == (char *)NULL) {
	return(False);
    }
    (void)strcpy(name, im->core.res_name);
    name[len] = '\0';

    out = (char **)((char *)top + info->offset);
    *out = name;
    return(True);
}

Private  Bool
_XimDefaultResClass(info, top, parm, mode)
    XimValueOffsetInfo	   info;
    XPointer	 	   top;
    XPointer	 	   parm;
    unsigned long	   mode;
{
    Xic			  ic = (Xic)parm;
    Xim			  im = (Xim)ic->core.im;
    int			  len;
    char		 *class;
    char		**out;

    if(im->core.res_class == (char *)NULL) {
	return(True);
    }
    len = strlen(im->core.res_class);
    if((class = (char *)Xmalloc(len + 1)) == (char *)NULL) {
	return(False);
    }
    (void)strcpy(class, im->core.res_class);
    class[len] = '\0';

    out = (char **)((char *)top + info->offset);
    *out = class;
    return(True);
}

Private  Bool
_XimDefaultDestroyCB(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;
    unsigned long	  mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    XIMCallback		*out;

    out = (XIMCallback *)((char *)top + info->offset);
    *out = im->core.destroy_callback;
    return(True);
}

Private  Bool
_XimDefaultResetState(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;
    unsigned long	  mode;
{
    XIMResetState	*out;

    out = (XIMResetState *)((char *)top + info->offset);
    *out = XIMInitialState;
    return(True);
}

Private  Bool
_XimDefaultResetReturn(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;
    unsigned long	  mode;
{
    XIMResetReturn	*out;

    out = (XIMResetReturn *)((char *)top + info->offset);
    *out = XIMInitialState;
    return(True);
}

Private  Bool
_XimDefaultHotKeyState(info, top, parm, mode)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  parm;
    unsigned long	  mode;
{
    XIMHotKeyState	*out;

    out = (XIMHotKeyState *)((char *)top + info->offset);
    *out = XIMHotKeyStateOFF;
    return(True);
}

Private  Bool
_XimDefaultArea(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    Window		 root_return;
    int			 x_return, y_return;
    unsigned int	 width_return, height_return;
    unsigned int	 border_width_return;
    unsigned int	 depth_return;
    XRectangle		 area;
    XRectangle		*out;

    if(ic->core.focus_window == (Window)NULL) {
	return(True);
    }
    if(XGetGeometry(im->core.display, (Drawable)ic->core.focus_window,
		&root_return, &x_return, &y_return, &width_return,
		&height_return, &border_width_return, &depth_return)
		== (Status)NULL) {
	return(True);
    }
    area.x	= 0;
    area.y	= 0;
    area.width	= width_return;
    area.height	= height_return;

    out = (XRectangle *)((char *)top + info->offset);
    *out = area;
    return(True);
}

Private  Bool
_XimDefaultColormap(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    XWindowAttributes	 win_attr;
    Colormap		*out;

    if(ic->core.client_window == (Window)NULL) {
	return(True);
    }
    if(XGetWindowAttributes(im->core.display, ic->core.client_window,
					&win_attr) == (Status)NULL) {
	return(True);
    }
   
    out = (Colormap *)((char *)top + info->offset);
    *out = win_attr.colormap;
    return(True);
}

Private  Bool
_XimDefaultStdColormap(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Atom		*out;

    out = (Atom *)((char *)top + info->offset);
    *out = (Atom)0;
    return(True);
}

Private  Bool
_XimDefaultFg(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    unsigned long	 fg;
    unsigned long	*out;

    fg = WhitePixel(im->core.display, DefaultScreen(im->core.display));
    out = (unsigned long *)((char *)top + info->offset);
    *out = fg;
    return(True);
}

Private  Bool
_XimDefaultBg(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    unsigned long	 bg;
    unsigned long	*out;

    bg = BlackPixel(im->core.display, DefaultScreen(im->core.display));
    out = (unsigned long *)((char *)top + info->offset);
    *out = bg;
    return(True);
}

Private  Bool
_XimDefaultBgPixmap(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Pixmap		*out;

    out = (Pixmap *)((char *)top + info->offset);
    *out = (Pixmap)0;
    return(True);
}

Private  Bool
_XimDefaultFontSet(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    char		**missing_list;
    int			 missing_count;
    char		*def_string;
    XFontSet		 fontset;
    XFontSet		*out;

    fontset = XCreateFontSet(im->core.display, XIM_LOCAL_DEFAULT_FONT_NAME,
			&missing_list, &missing_count, &def_string);
    
    out = (XFontSet *)((char *)top + info->offset);
    *out = fontset;
    return(True);
}

Private  Bool
_XimDefaultLineSpace(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Xic			 ic = (Xic)parm;
    Xim			 im = (Xim)ic->core.im;
    XFontSet		 fontset;
    XFontSetExtents	*fset_extents;
    int			 line_space;
    int			*out;

    if(mode & XIM_PREEDIT_ATTR) {
	fontset = ic->core.preedit_attr.fontset;
    } else if(mode & XIM_STATUS_ATTR) {
	fontset = ic->core.status_attr.fontset;
    } else {
	return(True);
    }

    fset_extents = XExtentsOfFontSet(fontset);
    line_space = fset_extents->max_logical_extent.height;
    out = (int *)((char *)top + info->offset);
    *out = line_space;
    return(True);
}

Private  Bool
_XimDefaultCursor(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    Cursor		*out;

    out = (Cursor *)((char *)top + info->offset);
    *out = (Cursor)0;
    return(True);
}

Private  Bool
_XimDefaultPreeditState(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    XIMPreeditState	*out;

    out = (XIMPreeditState *)((char *)top + info->offset);
    *out = XIMPreeditDisable;
    return(True);
}

Private  Bool
_XimDefaultNest(info, top, parm, mode)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 parm;
    unsigned long	 mode;
{
    return(True);
}

Private  Bool
_XimDecodeCallback(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMCallback		*out;

    out = (XIMCallback *)((char *)top + info->offset);
    *out = *((XIMCallback *)val);
    return(True);
}

Private  Bool
_XimDecodeString(info, top, val)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  val;
{
    int			  len;
    char		 *string;
    char		**out;

    if(val == (XPointer)NULL) {
	return(False);
    }
    len = strlen((char *)val);
    if((string = (char *)Xmalloc(len + 1)) == (char *)NULL) {
	return(False);
    }
    (void)strcpy(string, (char *)val);
    string[len] = '\0';

    out = (char **)((char *)top + info->offset);
    if(*out) {
	Xfree(*out);
    }
    *out = string;
    return(True);
}

Private  Bool
_XimDecodeStyle(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMStyle		*out;

    out = (XIMStyle *)((char *)top + info->offset);
    *out = (XIMStyle)val;
    return(True);
}

Private  Bool
_XimDecodeWindow(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Window		*out;

    out = (Window *)((char *)top + info->offset);
    *out = (Window)val;
    return(True);
}

Private  Bool
_XimDecodeStringConv(info, top, val)
    XimValueOffsetInfo		 info;
    XPointer		 	 top;
    XPointer		 	 val;
{
    XIMStringConversionText	*out;

    /*
     * Not yet
     */
    return(True);
}

Private  Bool
_XimDecodeResetState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMResetState	*out;

    out = (XIMResetState *)((char *)top + info->offset);
    *out = (XIMResetState)val;
    return(True);
}

Private  Bool
_XimDecodeResetReturn(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMResetReturn	*out;

    out = (XIMResetReturn *)((char *)top + info->offset);
    *out = (XIMResetReturn)val;
    return(True);
}

Private  Bool
_XimDecodeHotKey(info, top, val)
    XimValueOffsetInfo	  info;
    XPointer	 	  top;
    XPointer	 	  val;
{
    XIMHotKeyTriggers	 *hotkey = (XIMHotKeyTriggers *)val;
    XIMHotKeyTriggers	**out;
    XIMHotKeyTriggers	 *key_list;
    XIMHotKeyTrigger	 *key;
    char		 *tmp;
    int			  num;
    int			  len;
    register int	  i;

    if(hotkey == (XIMHotKeyTriggers *)NULL) {
	return(True);
    }

    if((num = hotkey->num_hot_key) == 0) {
	return(True);
    }

    len = sizeof(XIMHotKeyTriggers) + sizeof(XIMHotKeyTrigger) * num;
    if((tmp = (char *)Xmalloc(len)) == (char *)NULL) {
	return(False);
    }

    key_list = (XIMHotKeyTriggers *)tmp;
    key = (XIMHotKeyTrigger *)(tmp + sizeof(XIMHotKeyTriggers));

    for(i = 0; i < num; i++) {
	key[i] = hotkey->key[i];
    }

    key_list->num_hot_key = num;
    key_list->key = key;

    out = (XIMHotKeyTriggers **)((char *)top + info->offset);
    *out = key_list;
    return(True);
}

Private  Bool
_XimDecodeHotKetState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMHotKeyState	*out;

    out = (XIMHotKeyState *)((char *)top + info->offset);
    *out = (XIMHotKeyState)val;
    return(True);
}

Private  Bool
_XimDecodeRectangle(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XRectangle		*out;

    out = (XRectangle *)((char *)top + info->offset);
    *out = *((XRectangle *)val);
    return(True);
}

Private  Bool
_XimDecodeSpot(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XPoint		*out;

    out = (XPoint *)((char *)top + info->offset);
    *out = *((XPoint *)val);
    return(True);
}

Private  Bool
_XimDecodeColormap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Colormap		*out;

    out = (Colormap *)((char *)top + info->offset);
    *out = (Colormap)val;
    return(True);
}

Private  Bool
_XimDecodeStdColormap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Atom		*out;

    out = (Atom *)((char *)top + info->offset);
    *out = (Atom)val;
    return(True);
}

Private  Bool
_XimDecodeLong(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    unsigned long	*out;

    out = (unsigned long *)((char *)top + info->offset);
    *out = (unsigned long)val;
    return(True);
}

Private  Bool
_XimDecodeBgPixmap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Pixmap		*out;

    out = (Pixmap *)((char *)top + info->offset);
    *out = (Pixmap)val;
    return(True);
}

Private  Bool
_XimDecodeFontSet(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XFontSet		*out;

    out = (XFontSet *)((char *)top + info->offset);
    *out = (XFontSet)val;
    return(True);
}

Private  Bool
_XimDecodeLineSpace(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    int			*out;

    out = (int *)((char *)top + info->offset);
    *out = (int)val;
    return(True);
}

Private  Bool
_XimDecodeCursor(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Cursor		*out;

    out = (Cursor *)((char *)top + info->offset);
    *out = (Cursor)val;
    return(True);
}

Private  Bool
_XimDecodePreeditState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMPreeditState	*out;

    out = (XIMPreeditState *)((char *)top + info->offset);
    *out = (XIMPreeditState)val;
    return(True);
}

Private  Bool
_XimDecodeNest(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    return(True);
}

Private  Bool
_XimEncodeStyles(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMStyles		*styles;
    XIMStyles		*out;
    register int	 i;
    unsigned int	 num;
    int			 len;
    char		*tmp;

    if(val == (XPointer)NULL) {
	return(False);
    }

    styles = *((XIMStyles **)((char *)top + info->offset));
    num = styles->count_styles;

    len = sizeof(XIMStyles) + sizeof(XIMStyle) * num;
    if((tmp = (char *)Xmalloc(len)) == (char *)NULL) {
	return(False);
    }
    bzero(tmp, len);

    out = (XIMStyles *)tmp;
    if(num >0) {
	out->count_styles = (unsigned short)num;
	out->supported_styles = (XIMStyle *)(tmp + sizeof(XIMStyles));
    
	for(i = 0; i < num; i++) {
	    out->supported_styles[i] = styles->supported_styles[i];
	}
    }
    *((XIMStyles **)val) = out;
    return(True);
}

Private  Bool
_XimEncodeCallback(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMCallback		*in;
    XIMCallback		*callback;

    in = (XIMCallback *)((char *)top + info->offset);
    if(callback = (XIMCallback *)Xmalloc(sizeof(XIMCallback))) {
	return(False);
    }
    callback->client_data = in->client_data;
    callback->callback    = in->callback;

    *((XIMCallback **)val) = callback;
    return(True);
}

Private  Bool
_XimEncodeString(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    int			 len = 0;
    char		*in;
    char		*string;

    in = *((char **)((char *)top + info->offset));
    if(in != (char *)NULL) {
	len = strlen(in);
    }
    if((string = (char *)Xmalloc(len + 1)) == (char *)NULL) {
	return(False);
    }
    if(in != (char *)NULL) {
	(void)strcpy(string, in);
    }
    string[len] = '\0';
    *((char **)val) = string;
    return(True);
}

Private  Bool
_XimEncodeStyle(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMStyle		*in;

    in = (XIMStyle *)((char *)top + info->offset);
    *((XIMStyle *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeWindow(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Window		*in;

    in = (Window *)((char *)top + info->offset);
    *((Window *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeStringConv(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    /*
     * Not yet
     */
    return(True);
}

Private  Bool
_XimEncodeResetState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMResetState	*in;

    in = (XIMResetState *)((char *)top + info->offset);
    *((XIMResetState *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeResetReturn(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMResetReturn	*in;

    in = (XIMResetReturn *)((char *)top + info->offset);
    *((XIMResetReturn *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeHotKey(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMHotKeyTriggers	*in;
    XIMHotKeyTriggers	*hotkey;
    XIMHotKeyTrigger	*key;
    char		*tmp;
    int			 num;
    int			 len;
    register int	 i;

    in = *((XIMHotKeyTriggers **)((char *)top + info->offset));
    num = in->num_hot_key;
    len = sizeof(XIMHotKeyTriggers) + sizeof(XIMHotKeyTrigger) * num;
    if((tmp = (char *)Xmalloc(len)) == (char *)NULL) {
	return(False);
    }

    hotkey = (XIMHotKeyTriggers *)tmp;
    key = (XIMHotKeyTrigger *)(tmp + sizeof(XIMHotKeyTriggers));

    for(i = 0; i < num; i++) {
	key[i] = in->key[i];
    }
    hotkey->num_hot_key = num;
    hotkey->key = key;

    *((XIMHotKeyTriggers **)val) = hotkey;
    return(True);
}

Private  Bool
_XimEncodeHotKetState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMHotKeyState	*in;

    in = (XIMHotKeyState *)((char *)top + info->offset);
    *((XIMHotKeyState *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeRectangle(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XRectangle		*in;
    XRectangle		*rect;

    in = (XRectangle *)((char *)top + info->offset);
    if((rect = (XRectangle *)Xmalloc(sizeof(XRectangle)))
						 == (XRectangle *)NULL) {
	return(False);
    }
    *rect = *in;
    *((XRectangle **)val) = rect;
    return(True);
}

Private  Bool
_XimEncodeSpot(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XPoint		*in;
    XPoint		*spot;

    in = (XPoint *)((char *)top + info->offset);
    if((spot = (XPoint *)Xmalloc(sizeof(XPoint))) == (XPoint *)NULL) {
	return(False);
    }
    *spot = *in;
    *((XPoint **)val) = spot;
    return(True);
}

Private  Bool
_XimEncodeColormap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Colormap		*in;

    in = (Colormap *)((char *)top + info->offset);
    *((Colormap *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeStdColormap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Atom		*in;

    in = (Atom *)((char *)top + info->offset);
    *((Atom *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeLong(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    unsigned long	*in;

    in = (unsigned long *)((char *)top + info->offset);
    *((unsigned long *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeBgPixmap(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Pixmap		*in;

    in = (Pixmap *)((char *)top + info->offset);
    *((Pixmap *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeFontSet(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XFontSet		*in;

    in = (XFontSet *)((char *)top + info->offset);
    *((XFontSet *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeLineSpace(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    int		*in;

    in = (int *)((char *)top + info->offset);
    *((int *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeCursor(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    Cursor		*in;

    in = (Cursor *)((char *)top + info->offset);
    *((Cursor *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodePreeditState(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    XIMPreeditState	*in;

    in = (XIMPreeditState *)((char *)top + info->offset);
    *((XIMPreeditState *)val) = *in;
    return(True);
}

Private  Bool
_XimEncodeNest(info, top, val)
    XimValueOffsetInfo	 info;
    XPointer	 	 top;
    XPointer	 	 val;
{
    return(True);
}

static	XIMResource	im_resources[] = {
    {XNQueryInputStyle,		   XimType_XIMStyles,		0, 0, 0},
    {XNDestroyCallback,		   0,				0, 0, 0},
    {XNResourceName,		   XimType_STRING8,		0, 0, 0},
    {XNResourceClass,		   XimType_STRING8,		0, 0, 0}
};

static	XIMResource	im_inner_resources[] = {
    {XNDestroyCallback,		   0,				0, 0, 0},
    {XNResourceName,		   XimType_STRING8,		0, 0, 0},
    {XNResourceClass,		   XimType_STRING8,		0, 0, 0}
};

static	XIMResource	ic_resources[] = {
    {XNInputStyle,		   XimType_CARD32,		0, 0, 0},
    {XNClientWindow,		   XimType_Window,		0, 0, 0},
    {XNFocusWindow,		   XimType_Window,		0, 0, 0},
    {XNResourceName,		   XimType_STRING8,		0, 0, 0},
    {XNResourceClass,		   XimType_STRING8,		0, 0, 0},
    {XNGeometryCallback,	   0,				0, 0, 0},
    {XNFilterEvents,		   XimType_CARD32,		0, 0, 0},
    {XNDestroyCallback,		   0,				0, 0, 0},
    {XNStringConversionCallback,   0,				0, 0, 0},
    {XNStringConversion,	   XimType_XIMStringConversion,	0, 0, 0},
    {XNResetState,		   0,				0, 0, 0},
    {XNResetReturn,		   0,				0, 0, 0},
    {XNHotKey,			   XimType_XIMHotKeyTriggers,	0, 0, 0},
    {XNHotKeyState,		   XimType_XIMHotKeyState, 	0, 0, 0},
    {XNPreeditAttributes,	   XimType_NEST,		0, 0, 0},
    {XNStatusAttributes,	   XimType_NEST,		0, 0, 0},
    {XNArea,			   XimType_XRectangle,		0, 0, 0},
    {XNAreaNeeded,		   XimType_XRectangle,		0, 0, 0},
    {XNSpotLocation,		   XimType_XPoint,		0, 0, 0},
    {XNColormap,		   XimType_CARD32,		0, 0, 0},
    {XNStdColormap,		   XimType_CARD32,		0, 0, 0},
    {XNForeground,		   XimType_CARD32,		0, 0, 0},
    {XNBackground,		   XimType_CARD32,		0, 0, 0},
    {XNBackgroundPixmap,	   XimType_CARD32,		0, 0, 0},
    {XNFontSet,			   XimType_XFontSet,		0, 0, 0},
    {XNLineSpace,		   XimType_CARD32,		0, 0, 0},
    {XNCursor,			   XimType_CARD32,		0, 0, 0},
    {XNPreeditStartCallback,	   0,				0, 0, 0},
    {XNPreeditDoneCallback,	   0,				0, 0, 0},
    {XNPreeditDrawCallback,	   0,				0, 0, 0},
    {XNPreeditCaretCallback,	   0,				0, 0, 0},
    {XNStatusStartCallback,	   0,				0, 0, 0},
    {XNStatusDoneCallback,	   0,				0, 0, 0},
    {XNStatusDrawCallback,	   0,				0, 0, 0},
    {XNPreeditState,		   0,				0, 0, 0}
};

static	XIMResource	ic_inner_resources[] = {
    {XNResourceName,		   XimType_STRING8,		0, 0, 0},
    {XNResourceClass,		   XimType_STRING8,		0, 0, 0},
    {XNGeometryCallback,	   0,				0, 0, 0},
    {XNDestroyCallback,		   0,				0, 0, 0},
    {XNStringConversionCallback,   0,				0, 0, 0},
    {XNResetReturn,		   0,				0, 0, 0},
    {XNPreeditStartCallback,	   0,				0, 0, 0},
    {XNPreeditDoneCallback,	   0,				0, 0, 0},
    {XNPreeditDrawCallback,	   0,				0, 0, 0},
    {XNPreeditCaretCallback,	   0,				0, 0, 0},
    {XNStatusStartCallback,	   0,				0, 0, 0},
    {XNStatusDoneCallback,	   0,				0, 0, 0},
    {XNStatusDrawCallback,	   0,				0, 0, 0},
};

static XimValueOffsetInfoRec im_attr_info[] = {
    {XNQueryInputStyle,		 0,
	XOffsetOf(XimDefIMValues, styles),
	_XimDefaultStyles,	 NULL,			_XimEncodeStyles},

    {XNDestroyCallback,		 0,
	XOffsetOf(XimDefIMValues, destroy_callback),
	NULL,		 	 _XimDecodeCallback,	_XimEncodeCallback},

    {XNResourceName,		 0,
	XOffsetOf(XimDefIMValues, res_name),
	NULL,		 	 _XimDecodeString,	_XimEncodeString},

    {XNResourceClass,		 0,
	XOffsetOf(XimDefIMValues, res_class),
	NULL,		 	 _XimDecodeString,	_XimEncodeString}
};

static XimValueOffsetInfoRec ic_attr_info[] = {
    {XNInputStyle,		 0,
	XOffsetOf(XimDefICValues, input_style),
	NULL,			 _XimDecodeStyle,	_XimEncodeStyle},

    {XNClientWindow,		 0,
	XOffsetOf(XimDefICValues, client_window),
	NULL,			 _XimDecodeWindow,	_XimEncodeWindow},

    {XNFocusWindow,		 0,
	XOffsetOf(XimDefICValues, focus_window),
	_XimDefaultFocusWindow,  _XimDecodeWindow,	_XimEncodeWindow},

    {XNResourceName,		 0,
	XOffsetOf(XimDefICValues, res_name),
	_XimDefaultResName,	 _XimDecodeString,	_XimEncodeString},

    {XNResourceClass,		 0,
	XOffsetOf(XimDefICValues, res_class),
	_XimDefaultResClass,	 _XimDecodeString,	_XimEncodeString},

    {XNGeometryCallback,	 0,
	XOffsetOf(XimDefICValues, geometry_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNFilterEvents,		 0,
	XOffsetOf(XimDefICValues, filter_events),
	NULL,			 NULL,			_XimEncodeLong},

    {XNDestroyCallback,		 0,
	XOffsetOf(XimDefICValues, destroy_callback),
	_XimDefaultDestroyCB,	 _XimDecodeCallback,	_XimEncodeCallback},

    {XNStringConversionCallback, 0,
	XOffsetOf(XimDefICValues, string_conversion_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNStringConversion,	 0,
	XOffsetOf(XimDefICValues, string_conversion),
	NULL,			 _XimDecodeStringConv,	_XimEncodeStringConv},

    {XNResetState,		 0,
	XOffsetOf(XimDefICValues, reset_state),
	_XimDefaultResetState,	 _XimDecodeResetState,	_XimEncodeResetState},

    {XNResetReturn,		 0,
	XOffsetOf(XimDefICValues, reset_return),
	_XimDefaultResetReturn,	 _XimDecodeResetReturn,	_XimEncodeResetReturn},

    {XNHotKey,			 0,
	XOffsetOf(XimDefICValues, hotkey),
	NULL,			 _XimDecodeHotKey,	_XimEncodeHotKey},

    {XNHotKeyState,		 0,
	XOffsetOf(XimDefICValues, hotkey_state),
	_XimDefaultHotKeyState,	 _XimDecodeHotKetState,	_XimEncodeHotKetState},

    {XNPreeditAttributes,	 0,
	XOffsetOf(XimDefICValues, preedit_attr),
	_XimDefaultNest,	 _XimDecodeNest,	_XimEncodeNest},

    {XNStatusAttributes,	 0,
	XOffsetOf(XimDefICValues, status_attr),
	_XimDefaultNest,	 _XimDecodeNest,	_XimEncodeNest},
};

static XimValueOffsetInfoRec ic_pre_attr_info[] = {
    {XNArea,			 0,
	XOffsetOf(ICPreeditAttributes, area),
	_XimDefaultArea,	 _XimDecodeRectangle,	_XimEncodeRectangle},

    {XNAreaNeeded,		 0,
	XOffsetOf(ICPreeditAttributes, area_needed),
	NULL,			 _XimDecodeRectangle,	_XimEncodeRectangle},

    {XNSpotLocation,		 0,
	XOffsetOf(ICPreeditAttributes, spot_location),
	NULL,			 _XimDecodeSpot,	_XimEncodeSpot},

    {XNColormap,		 0,
	XOffsetOf(ICPreeditAttributes, colormap),
	_XimDefaultColormap,	 _XimDecodeColormap,	_XimEncodeColormap},

    {XNStdColormap,		 0,
	XOffsetOf(ICPreeditAttributes, std_colormap),
	_XimDefaultStdColormap,	 _XimDecodeStdColormap,	_XimEncodeStdColormap},

    {XNForeground,		 0,
	XOffsetOf(ICPreeditAttributes, foreground),
	_XimDefaultFg,		 _XimDecodeLong,	_XimEncodeLong},

    {XNBackground,		 0,
	XOffsetOf(ICPreeditAttributes, background),
	_XimDefaultBg,		 _XimDecodeLong,	_XimEncodeLong},

    {XNBackgroundPixmap,	 0,
	XOffsetOf(ICPreeditAttributes, background_pixmap),
	_XimDefaultBgPixmap, 	 _XimDecodeBgPixmap,	_XimEncodeBgPixmap},

    {XNFontSet,			 0,
	XOffsetOf(ICPreeditAttributes, fontset),
	_XimDefaultFontSet,	 _XimDecodeFontSet,	_XimEncodeFontSet},

    {XNLineSpace,		 0,
	XOffsetOf(ICPreeditAttributes, line_spacing),
	_XimDefaultLineSpace,	 _XimDecodeLineSpace,	_XimEncodeLineSpace},

    {XNCursor,			 0,
	XOffsetOf(ICPreeditAttributes, cursor),
	_XimDefaultCursor,	 _XimDecodeCursor,	_XimEncodeCursor},

    {XNPreeditStartCallback,	 0,
	XOffsetOf(ICPreeditAttributes, start_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNPreeditDoneCallback,	 0,
	XOffsetOf(ICPreeditAttributes, done_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNPreeditDrawCallback,	 0,
	XOffsetOf(ICPreeditAttributes, draw_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNPreeditCaretCallback,	 0,
	XOffsetOf(ICPreeditAttributes, caret_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNPreeditState,		 0,
	XOffsetOf(ICPreeditAttributes, preedit_state),
	_XimDefaultPreeditState, _XimDecodePreeditState,_XimEncodePreeditState}
};

static XimValueOffsetInfoRec ic_sts_attr_info[] = {
    {XNArea,			 0,
	XOffsetOf(ICStatusAttributes, area),
	_XimDefaultArea,	 _XimDecodeRectangle,	_XimEncodeRectangle},

    {XNAreaNeeded,		 0,
	XOffsetOf(ICStatusAttributes, area_needed),
	NULL,			 _XimDecodeRectangle,	_XimEncodeRectangle},

    {XNColormap,		 0,
	XOffsetOf(ICStatusAttributes, colormap),
	_XimDefaultColormap,	 _XimDecodeColormap,	_XimEncodeColormap},

    {XNStdColormap,		 0,
	XOffsetOf(ICStatusAttributes, std_colormap),
	_XimDefaultStdColormap,	 _XimDecodeStdColormap,	_XimEncodeStdColormap},

    {XNForeground,		 0,
	XOffsetOf(ICStatusAttributes, foreground),
	_XimDefaultFg,		 _XimDecodeLong,	_XimEncodeLong},

    {XNBackground,		 0,
	XOffsetOf(ICStatusAttributes, background),
	_XimDefaultBg,		 _XimDecodeLong,	_XimEncodeLong},

    {XNBackgroundPixmap,	 0,
	XOffsetOf(ICStatusAttributes, background_pixmap),
	_XimDefaultBgPixmap, 	 _XimDecodeBgPixmap,	_XimEncodeBgPixmap},

    {XNFontSet,			 0,
	XOffsetOf(ICStatusAttributes, fontset),
	_XimDefaultFontSet,	 _XimDecodeFontSet,	_XimEncodeFontSet},

    {XNLineSpace,		 0,
	XOffsetOf(ICStatusAttributes, line_spacing),
	_XimDefaultLineSpace,	 _XimDecodeLineSpace,	_XimEncodeLineSpace},

    {XNCursor,			 0,
	XOffsetOf(ICStatusAttributes, cursor),
	_XimDefaultCursor,	 _XimDecodeCursor,	_XimEncodeCursor},

    {XNStatusStartCallback,	 0,
	XOffsetOf(ICStatusAttributes, start_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNStatusDoneCallback,	 0,
	XOffsetOf(ICStatusAttributes, done_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback},

    {XNStatusDrawCallback,	 0,
	XOffsetOf(ICStatusAttributes, draw_callback),
	NULL,			 _XimDecodeCallback,	_XimEncodeCallback}
};

typedef struct _XimIMMode {
    char		*name;
    XrmQuark		 quark;
    unsigned short	 mode;
} XimIMMode;

static XimIMMode	im_mode[] = {
    {XNQueryInputStyle,		0,
		(XIM_MODE_IM_DEFAULT | XIM_MODE_IM_GET)},
    {XNDestroyCallback,		0,
		(XIM_MODE_IM_DEFAULT | XIM_MODE_IM_SET | XIM_MODE_IM_GET)},
    {XNResourceName,		0,
		(XIM_MODE_IM_DEFAULT | XIM_MODE_IM_SET | XIM_MODE_IM_GET)},
    {XNResourceClass,		0,
		(XIM_MODE_IM_DEFAULT | XIM_MODE_IM_SET | XIM_MODE_IM_GET)}
};

typedef struct _XimICMode {
    char		*name;
    XrmQuark		 quark;
    unsigned short	 preedit_callback_mode;
    unsigned short	 preedit_position_mode;
    unsigned short	 preedit_area_mode;
    unsigned short	 preedit_nothing_mode;
    unsigned short	 preedit_none_mode;
    unsigned short	 status_callback_mode;
    unsigned short	 status_area_mode;
    unsigned short	 status_nothing_mode;
    unsigned short	 status_none_mode;
} XimICMode;

static XimICMode	ic_mode[] = {
    {XNInputStyle, 0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_GET),
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_GET),
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_GET),
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_GET),
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_GET)},
    {XNClientWindow, 0,
		(XIM_MODE_PRE_ONCE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_ONCE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_ONCE | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_ONCE | XIM_MODE_PRE_GET),
		0,
		(XIM_MODE_STS_ONCE | XIM_MODE_STS_GET),
		(XIM_MODE_STS_ONCE | XIM_MODE_STS_GET),
		(XIM_MODE_STS_ONCE | XIM_MODE_STS_GET),
		0},
    {XNFocusWindow, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNResourceName, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNResourceClass, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNGeometryCallback, 0,
		0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0},
    {XNFilterEvents, 0,
		XIM_MODE_PRE_GET,
		XIM_MODE_PRE_GET,
		XIM_MODE_PRE_GET,
		XIM_MODE_PRE_GET,
		0,
		XIM_MODE_STS_GET,
		XIM_MODE_STS_GET,
		XIM_MODE_STS_GET,
		XIM_MODE_STS_GET},
    {XNDestroyCallback, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0},
    {XNStringConversionCallback, 0,
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0},
    {XNStringConversion, 0,
		XIM_MODE_PRE_SET,
		XIM_MODE_PRE_SET,
		XIM_MODE_PRE_SET,
		XIM_MODE_PRE_SET,
		XIM_MODE_PRE_SET,
		0,
		0,
		0,
		0},
    {XNResetState, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNResetReturn, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNHotKey, 0,
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNHotKeyState, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNPreeditAttributes, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNStatusAttributes, 0,
		0,
		0,
		0,
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNArea, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0},
    {XNAreaNeeded, 0,
		0,
		0,
		(XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		(XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0},
    {XNSpotLocation, 0,
		0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0,
		0,
		0},
    {XNColormap, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNStdColormap, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNForeground, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNBackground, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNBackgroundPixmap, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNFontSet, 0,
		0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNLineSpace, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNCursor, 0,
		0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		(XIM_MODE_STS_DEFAULT | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0},
    {XNPreeditStartCallback, 0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0},
    {XNPreeditDoneCallback, 0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0},
    {XNPreeditDrawCallback, 0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0},
    {XNPreeditCaretCallback, 0,
		(XIM_MODE_PRE_CREATE | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0},
    {XNPreeditState, 0,
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		(XIM_MODE_PRE_DEFAULT | XIM_MODE_PRE_SET | XIM_MODE_PRE_GET),
		0,
		0,
		0,
		0,
		0},
    {XNStatusStartCallback, 0,
		0,
		0,
		0,
		0,
		0,
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0,
		0},
    {XNStatusDoneCallback, 0,
		0,
		0,
		0,
		0,
		0,
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0,
		0},
    {XNStatusDrawCallback, 0,
		0,
		0,
		0,
		0,
		0,
		(XIM_MODE_STS_CREATE | XIM_MODE_STS_SET | XIM_MODE_STS_GET),
		0,
		0,
		0}
};

Private Bool
_XimSetResourceList(res_list, list_num, resource, num_resource, id)
    XIMResourceList	*res_list;
    unsigned int	*list_num;
    XIMResourceList	 resource;
    unsigned int	 num_resource;
    unsigned short	 id;
{
    register int	 i;
    int			 len;
    XIMResourceList	 res;

    len = sizeof(XIMResource) * num_resource;
    if((res = (XIMResourceList)Xmalloc(len)) == (XIMResourceList)NULL) {
	return(False);
    }
    bzero((char *)res, len);

    for(i = 0; i < num_resource; i++, id++) {
	res[i]    = resource[i];
	res[i].id = id;
    }

    _XIMCompileResourceList(res, num_resource);
    *res_list  = res;
    *list_num  = num_resource;
    return(True);
}

Public Bool
_XimSetIMResourceList(res_list, list_num)
    XIMResourceList	*res_list;
    unsigned int	*list_num;
{
    return(_XimSetResourceList(res_list, list_num,
				im_resources, XIMNumber(im_resources), 100));
}

Public Bool
_XimSetICResourceList(res_list, list_num)
    XIMResourceList	*res_list;
    unsigned int	*list_num;
{
    return(_XimSetResourceList(res_list, list_num,
				ic_resources, XIMNumber(ic_resources), 200));
}

Public Bool
_XimSetInnerIMResourceList(res_list, list_num)
    XIMResourceList	*res_list;
    unsigned int	*list_num;
{
    return(_XimSetResourceList(res_list, list_num,
		im_inner_resources, XIMNumber(im_inner_resources), 100));
}

Public Bool
_XimSetInnerICResourceList(res_list, list_num)
    XIMResourceList	*res_list;
    unsigned int	*list_num;
{
    return(_XimSetResourceList(res_list, list_num,
		ic_inner_resources, XIMNumber(ic_inner_resources), 200));
}

Private XIMResourceList
_XimGetResourceListRecByMode(res_list, list_num, mode)
    XIMResourceList	 res_list;
    unsigned int	 list_num;
    unsigned short	 mode;
{
    register int	 i;

    for(i = 0; i < list_num; i++) {
	if (res_list[i].mode & mode) {
	    return((XIMResourceList)&res_list[i]);
	}
    }
    return((XIMResourceList)NULL);
}

Public Bool
_XimCheckCreateICValues(res_list, list_num)
    XIMResourceList	 res_list;
    unsigned int	 list_num;
{
    if(_XimGetResourceListRecByMode(res_list, list_num,
			XIM_MODE_IC_CREATE) == (XIMResourceList)NULL) {
	return True;
    }
    return False;
}

Private XIMResourceList
_XimGetResourceListRecByQuark(res_list, list_num, quark)
    XIMResourceList	 res_list;
    unsigned int	 list_num;
    XrmQuark		 quark;
{
    register int	 i;

    for(i = 0; i < list_num; i++) {
	if (res_list[i].xrm_name == quark) {
	    return((XIMResourceList)&res_list[i]);
	}
    }
    return((XIMResourceList)NULL);
}

Public XIMResourceList
_XimGetResourceListRec(res_list, list_num, name)
    XIMResourceList	 res_list;
    unsigned int	 list_num;
    char		*name;
{
    register int	 i;
    XrmQuark		 quark = XrmStringToQuark(name);

    return(_XimGetResourceListRecByQuark(res_list, list_num, quark));
}

Public XIMResourceList
_XimGetIMResourceListRec(im, name)
    Xim		 im;
    char	*name;
{
    return(_XimGetResourceListRec(im->core.im_resources,
				im->core.im_num_resources, name));
}

Public XIMResourceList
_XimGetICResourceListRec(ic, name)
    Xic		 ic;
    char	*name;
{
    return(_XimGetResourceListRec(ic->private.local.ic_resources,
				ic->private.local.ic_num_resources, name));
}

Public void
_XimSetIMMode(res_list, list_num)
    XIMResourceList	res_list;
    unsigned int	list_num;
{
    XIMResourceList	res;
    unsigned int	n = XIMNumber(im_mode);
    register int	i;

    for(i = 0; i < n; i++) {
	if((res = _XimGetResourceListRecByQuark(res_list,
			list_num, im_mode[i].quark)) == (XIMResourceList)NULL) {
	    continue;
	}
	res->mode = im_mode[i].mode;
    }
    return;
}

Private int
_XimCheckSetIMDefaultsMode(res)
    XIMResourceList	res;
{
    if(res->mode & XIM_MODE_IM_DEFAULT) {
	return(XIM_CHECK_VALID);
    }
    return(XIM_CHECK_INVALID);
}

Private int
_XimCheckSetIMValuesMode(res)
    XIMResourceList	res;
{
    if(res->mode & XIM_MODE_IM_SET) {
	return(XIM_CHECK_VALID);
    }
    return(XIM_CHECK_INVALID);
}

Private int
 _XimCheckGetIMValuesMode(res)
    XIMResourceList	res;
{
    if(res->mode & XIM_MODE_IM_GET) {
	return(XIM_CHECK_VALID);
    }
    return(XIM_CHECK_INVALID);
}

Public int
 _XimCheckIMMode(res, mode)
    XIMResourceList	res;
    unsigned long	mode;
{
    if(res->mode == 0) {
	return(XIM_CHECK_INVALID);
    }
    if(mode & XIM_SETIMDEFAULTS) {
	return(_XimCheckSetIMDefaultsMode(res));
    } else if (mode & XIM_SETIMVALUES) {
	return(_XimCheckSetIMValuesMode(res));
    } else if (mode & XIM_GETIMVALUES) {
	return(_XimCheckGetIMValuesMode(res));
    } else {
	return(XIM_CHECK_ERROR);
    }
}

Public void
_XimSetICMode(res_list, list_num, style)
    XIMResourceList	res_list;
    unsigned int	list_num;
    XIMStyle		style;
{
    XIMResourceList	res;
    unsigned int	n = XIMNumber(ic_mode);
    register int	i;
    unsigned int	pre_offset;
    unsigned int	sts_offset;

    if(style & XIMPreeditArea) {
	pre_offset = XOffsetOf(XimICMode, preedit_area_mode);
    } else if(style & XIMPreeditCallbacks) {
	pre_offset = XOffsetOf(XimICMode, preedit_callback_mode);
    } else if(style & XIMPreeditPosition) {
	pre_offset = XOffsetOf(XimICMode, preedit_position_mode);
    } else if(style & XIMPreeditNothing) {
	pre_offset = XOffsetOf(XimICMode, preedit_nothing_mode);
    } else {
	pre_offset = XOffsetOf(XimICMode, preedit_none_mode);
    }

    if(style & XIMStatusArea) {
	sts_offset = XOffsetOf(XimICMode, status_area_mode);
    } else if(style & XIMStatusCallbacks) {
	sts_offset = XOffsetOf(XimICMode, status_callback_mode);
    } else if(style & XIMStatusNothing) {
	sts_offset = XOffsetOf(XimICMode, status_nothing_mode);
    } else {
	sts_offset = XOffsetOf(XimICMode, status_none_mode);
    }

    for(i = 0; i < n; i++) {
	if((res = _XimGetResourceListRecByQuark(res_list,
			list_num, ic_mode[i].quark)) == (XIMResourceList)NULL) {
	    continue;
	}
	res->mode = ( (*(unsigned short *)((char *)&ic_mode[i] + pre_offset))
		    | (*(unsigned short *)((char *)&ic_mode[i] + sts_offset)));
    }
    return;
}

Private int
_XimCheckSetICDefaultsMode(res, mode)
    XIMResourceList	res;
    unsigned long	mode;
{
    if(mode & XIM_PREEDIT_ATTR) {
	if(!(res->mode & XIM_MODE_PRE_MASK)) {
	    return(XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_PRE_CREATE) {
	    return(XIM_CHECK_ERROR);
	} else if (!(res->mode & XIM_MODE_PRE_DEFAULT)) {
	    return(XIM_CHECK_INVALID);
	}

    } else if(mode & XIM_STATUS_ATTR) {
	if(!(res->mode & XIM_MODE_STS_MASK)) {
	    return(XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_STS_CREATE) {
	    return(XIM_CHECK_ERROR);
	}
	if(!(res->mode & XIM_MODE_STS_DEFAULT)) {
	    return(XIM_CHECK_INVALID);
	}

    } else {
	if(!res->mode) {
	    return(XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_IC_CREATE) {
	    return(XIM_CHECK_ERROR);
	}
	if(!(res->mode & XIM_MODE_IC_DEFAULT)) {
	    return(XIM_CHECK_INVALID);
	}
    }
    return(XIM_CHECK_VALID);
}

Private int
_XimCheckCreateICMode(res, mode)
    XIMResourceList	res;
    unsigned long	mode;
{
    if(mode & XIM_PREEDIT_ATTR) {
	if(!(res->mode & XIM_MODE_PRE_MASK)) {
	    return(XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_PRE_CREATE) {
	    res->mode &= ~XIM_MODE_PRE_CREATE;
	} else if(res->mode & XIM_MODE_PRE_ONCE) {
	    res->mode &= ~XIM_MODE_PRE_ONCE;
	} else if(res->mode & XIM_MODE_PRE_DEFAULT) {
	    res->mode &= ~XIM_MODE_PRE_DEFAULT;
	} else if (!(res->mode & XIM_MODE_PRE_SET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else if(mode & XIM_STATUS_ATTR) {
	if(!(res->mode & XIM_MODE_STS_MASK)) {
	    return (XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_STS_CREATE) {
	    res->mode &= ~XIM_MODE_STS_CREATE;
	} else if(res->mode & XIM_MODE_STS_ONCE) {
	    res->mode &= ~XIM_MODE_STS_ONCE;
	} else if(res->mode & XIM_MODE_STS_DEFAULT) {
	    res->mode &= ~XIM_MODE_STS_DEFAULT;
	} else if (!(res->mode & XIM_MODE_STS_SET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else {
	if(!res->mode) {
	    return (XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_IC_CREATE) {
	    res->mode &= ~XIM_MODE_IC_CREATE;
	} else if(res->mode & XIM_MODE_IC_ONCE) {
	    res->mode &= ~XIM_MODE_IC_ONCE;
	} else if(res->mode & XIM_MODE_IC_DEFAULT) {
	    res->mode &= ~XIM_MODE_IC_DEFAULT;
	} else if (!(res->mode & XIM_MODE_IC_SET)) {
	    return(XIM_CHECK_ERROR);
	}
    }
    return(XIM_CHECK_VALID);
}

Private int
_XimCheckSetICValuesMode(res, mode)
    XIMResourceList	res;
    unsigned long	mode;
{
    if(mode & XIM_PREEDIT_ATTR) {
	if(!(res->mode & XIM_MODE_PRE_MASK)) {
	    return(XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_PRE_ONCE) {
	    res->mode &= ~XIM_MODE_PRE_ONCE;
	} else if(!(res->mode & XIM_MODE_PRE_SET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else if(mode & XIM_STATUS_ATTR) {
	if(!(res->mode & XIM_MODE_STS_MASK)) {
	    return (XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_STS_ONCE) {
	    res->mode &= ~XIM_MODE_STS_ONCE;
	} else if(!(res->mode & XIM_MODE_STS_SET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else {
	if(!res->mode) {
	    return (XIM_CHECK_INVALID);
	}

	if(res->mode & XIM_MODE_IC_ONCE) {
	    res->mode &= ~XIM_MODE_IC_ONCE;
	} else if(!(res->mode & XIM_MODE_IC_SET)) {
	    return(XIM_CHECK_ERROR);
	}
    }
    return(XIM_CHECK_VALID);
}

Private int
_XimCheckGetICValuesMode(res, mode)
    XIMResourceList	res;
    unsigned long	mode;
{
    if(mode & XIM_PREEDIT_ATTR) {
	if(!(res->mode & XIM_MODE_PRE_MASK)) {
	    return(XIM_CHECK_INVALID);
	}

	if(!(res->mode & XIM_MODE_PRE_GET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else if(mode & XIM_STATUS_ATTR) {
	if(!(res->mode & XIM_MODE_STS_MASK)) {
	    return (XIM_CHECK_INVALID);
	}

	if(!(res->mode & XIM_MODE_STS_GET)) {
	    return(XIM_CHECK_ERROR);
	}

    } else {
	if(!res->mode) {
	    return (XIM_CHECK_INVALID);
	}

	if(!(res->mode & XIM_MODE_IC_GET)) {
	    return(XIM_CHECK_ERROR);
	}
    }
    return(XIM_CHECK_VALID);
}

Public int
 _XimCheckICMode(res, mode)
    XIMResourceList	res;
    unsigned long	 mode;
{
    if(mode &XIM_SETICDEFAULTS) {
	return(_XimCheckSetICDefaultsMode(res, mode));
    } else if (mode & XIM_CREATEIC) {
	return(_XimCheckCreateICMode(res, mode));
    } else if (mode & XIM_SETICVALUES) {
	return(_XimCheckSetICValuesMode(res, mode));
    } else if (mode & XIM_GETICVALUES) {
	return(_XimCheckGetICValuesMode(res, mode));
    } else {
	return(XIM_CHECK_ERROR);
    }
}

Public Bool
_XimSetLocalIMDefaults(im, top)
    Xim			 im;
    XPointer		 top;
{
    XimValueOffsetInfo	 info;
    unsigned int	 num;
    register int	 i;
    XIMResourceList	 res;
    int			 check;

    info = im_attr_info;
    num  = XIMNumber(im_attr_info);

    for(i = 0; i < num; i++) {
	if((res = _XimGetResourceListRecByQuark(
				(XIMResourceList)im->core.im_resources,
				im->core.im_num_resources,
				info[i].quark)) == (XIMResourceList)NULL) { 
	    return(False);
	}

	check = _XimCheckIMMode(res, XIM_SETIMDEFAULTS);
	if(check == XIM_CHECK_INVALID) {
	    continue;
	} else if (check == XIM_CHECK_ERROR) {
	    return(False);
	}

	if(!info[i].defaults) {
	    continue;
	}
	if(info[i].defaults(&info[i], top, (XPointer)NULL, 0) == False) {
	    return(False);
	}
    }
    return(True);
}

Public Bool
_XimSetICDefaults(ic, top, mode)
    Xic			 ic;
    XPointer		 top;
    unsigned long	 mode;
{
    unsigned int	 num;
    XimValueOffsetInfo	 info;
    register int	 i;
    XIMResourceList	 res;
    int			 check;
    XrmQuark		 pre_quark;
    XrmQuark		 sts_quark;

    pre_quark = XrmStringToQuark(XNPreeditAttributes);
    sts_quark = XrmStringToQuark(XNStatusAttributes);

    if(mode & XIM_PREEDIT_ATTR) {
	info = ic_pre_attr_info;
	num  = XIMNumber(ic_pre_attr_info);
    } else if(mode & XIM_STATUS_ATTR) {
	info = ic_sts_attr_info;
	num  = XIMNumber(ic_sts_attr_info);
    } else {
	info = ic_attr_info;
	num  = XIMNumber(ic_attr_info);
    }

    for(i = 0; i < num; i++) {
	if(info[i].quark == pre_quark) {
	    if(_XimSetICDefaults(ic,
			(XPointer)((char *)top + info[i].offset),
			(mode | XIM_PREEDIT_ATTR) == False)) {
		return(False);
	    }
	} else if (info[i].quark == sts_quark) {
	    if(_XimSetICDefaults(ic,
			(XPointer)((char *)top + info[i].offset),
			(mode | XIM_STATUS_ATTR) == False)) {
		return(False);
	    }
	} else {
	    if((res = _XimGetResourceListRecByQuark(
			ic->private.local.ic_resources,
			ic->private.local.ic_num_resources,
			info[i].quark)) == (XIMResourceList)NULL) {
		return(False);
	    }

	    check = _XimCheckICMode(res, mode);
	    if (check == XIM_CHECK_INVALID) {
		continue;
	    } else if (check == XIM_CHECK_ERROR) {
		return(False);
	    }

	    if (!info[i].defaults) {
		continue;
	    }
	    if (info[i].defaults(&info[i], top, (XPointer)ic, mode) == False) {
		return(False);
	    }
	}
    }
    return(True);
}

Private Bool
_XimDecodeAttr(info, num, res, top, val)
    XimValueOffsetInfo	 info;
    unsigned int	 num;
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
{
    register int	 i;

    for(i = 0; i < num; i++ ) {
	if(info[i].quark == res->xrm_name) {
	    if(!info[i].decode) {
		return(False);
	    }
	    return(info[i].decode(&info[i], top, val));
	}
    }
    return(False);
}

Public Bool
_XimDecodeLocalIMAttr(res, top, val)
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
{
    return(_XimDecodeAttr(im_attr_info, XIMNumber(im_attr_info),
					res, top, val));
}

Public Bool
_XimDecodeLocalICAttr(res, top, val, mode)
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
    unsigned long	 mode;
{
    unsigned int	 num;
    XimValueOffsetInfo	 info;

    if(mode & XIM_PREEDIT_ATTR) {
	info = ic_pre_attr_info;
	num  = XIMNumber(ic_pre_attr_info);
    } else if(mode & XIM_STATUS_ATTR) {
	info = ic_sts_attr_info;
	num  = XIMNumber(ic_sts_attr_info);
    } else {
	info = ic_attr_info;
	num  = XIMNumber(ic_attr_info);
    }

    return(_XimDecodeAttr(info, num, res, top, val));
}

Public Bool
_XimCheckLocalInputStyle(ic, top, values, styles)
    Xic			 ic;
    XPointer		 top;
    XIMArg		*values;
    XIMStyles		*styles;
{
    XrmQuark		 quark = XrmStringToQuark(XNInputStyle);
    register XIMArg	*p;
    XIMResourceList	 res;
    unsigned int	 num;
    register int	 i;

    if (styles) {
	num = styles->count_styles;
    } else {
	return(False);
    }
    for(p = values; p && p->name != NULL; p++) {
	if(quark == XrmStringToQuark(p->name)) {
	    if((res = _XimGetICResourceListRec(ic, p->name))
						 == (XIMResourceList)NULL) {
		return(False);
	    }
	    if(_XimDecodeLocalICAttr(res, top, p->value, 0) == False) {
		return(False);
	    }
	    for(i = 0; i < num; i++) {
		if(styles->supported_styles[i]
			 == ((XimDefICValues *)top)->input_style) {
		    return(True);
		}
	    }
	    return(False);
	}
    }
    return(False);
}

Private Bool
_XimEncodeAttr(info, num, res, top, val)
    XimValueOffsetInfo	 info;
    unsigned int	 num;
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
{
    register int	 i;

    for(i = 0; i < num; i++ ) {
	if(info[i].quark == res->xrm_name) {
	    if(!info[i].encode) {
		return(False);
	    }
	    return(info[i].encode(&info[i], top, val)); 
	}
    }
    return(False);
}

Public Bool
_XimEncodeLocalIMAttr(res, top, val)
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
{
    return(_XimEncodeAttr(im_attr_info, XIMNumber(im_attr_info),
					res, top, val));
}

Public Bool
_XimEncodeLocalICAttr(res, top, val, mode)
    XIMResourceList	 res;
    XPointer		 top;
    XPointer		 val;
    unsigned long	 mode;
{
    unsigned int	 num;
    XimValueOffsetInfo	 info;

    if(mode & XIM_PREEDIT_ATTR) {
	info = ic_pre_attr_info;
	num  = XIMNumber(ic_pre_attr_info);
    } else if(mode & XIM_STATUS_ATTR) {
	info = ic_sts_attr_info;
	num  = XIMNumber(ic_sts_attr_info);
    } else {
	info = ic_attr_info;
	num  = XIMNumber(ic_attr_info);
    }

    return(_XimEncodeAttr(info, num, res, top, val));
}

Public void
_XimGetCurrentIMValues(im, im_values)
    Xim			 im;
    XimDefIMValues	*im_values;
{
    bzero((char *)im_values, sizeof(XimDefIMValues));

    im_values->styles		= im->core.styles;
    im_values->extensions	= im->core.extensions;
    im_values->options		= im->core.options;
    im_values->icattributes	= im->core.icattributes;
    im_values->destroy_callback = im->core.destroy_callback;
    im_values->res_name		= im->core.res_name;
    im_values->res_class	= im->core.res_class;
    return;
}

Public void
_XimSetCurrentIMValues(im, im_values)
    Xim			 im;
    XimDefIMValues	*im_values;
{
    im->core.styles		= im_values->styles;
    im->core.extensions		= im_values->extensions;
    im->core.options		= im_values->options;
    im->core.icattributes	= im_values->icattributes;
    im->core.destroy_callback	= im_values->destroy_callback;
    im->core.res_name		= im_values->res_name;
    im->core.res_class		= im_values->res_class;
    return;
}

Public void
_XimGetCurrentICValues(ic, ic_values)
    Xic			 ic;
    XimDefICValues	*ic_values;
{
    bzero((char *)ic_values, sizeof(XimDefICValues));

    ic_values->input_style	 = ic->core.input_style;
    ic_values->client_window	 = ic->core.client_window;
    ic_values->focus_window	 = ic->core.focus_window;
    ic_values->filter_events	 = ic->core.filter_events;
    ic_values->geometry_callback = ic->core.geometry_callback;
    ic_values->res_name		 = ic->core.res_name;
    ic_values->res_class	 = ic->core.res_class;
    ic_values->destroy_callback	 = ic->core.destroy_callback;
    ic_values->preedit_state_notify_callback
				 = ic->core.preedit_state_notify_callback;
    ic_values->string_conversion_callback
				 = ic->core.string_conversion_callback;
    ic_values->string_conversion = ic->core.string_conversion;
    ic_values->reset_state	 = ic->core.reset_state;
    ic_values->reset_return	 = ic->core.reset_return;
    ic_values->hotkey		 = ic->core.hotkey;
    ic_values->hotkey_state	 = ic->core.hotkey_state;
    ic_values->preedit_attr	 = ic->core.preedit_attr;
    ic_values->status_attr	 = ic->core.status_attr;
    return;
}

Public void
_XimSetCurrentICValues(ic, ic_values)
    Xic			 ic;
    XimDefICValues	*ic_values;
{
    ic->core.input_style	= ic_values->input_style;
    ic->core.client_window	= ic_values->client_window;
    ic->core.focus_window	= ic_values->focus_window;
    ic->core.filter_events	= ic_values->filter_events;
    ic->core.geometry_callback	= ic_values->geometry_callback;
    ic->core.res_name		= ic_values->res_name;
    ic->core.res_class		= ic_values->res_class;
    ic->core.destroy_callback 	= ic_values->destroy_callback;
    ic->core.string_conversion_callback
				= ic_values->string_conversion_callback;
    ic->core.string_conversion	= ic_values->string_conversion;
    ic->core.reset_state	= ic_values->reset_state;
    ic->core.reset_return	= ic_values->reset_return;
    ic->core.hotkey		= ic_values->hotkey;
    ic->core.hotkey_state	= ic_values->hotkey_state;
    ic->core.preedit_attr	= ic_values->preedit_attr;
    ic->core.status_attr	= ic_values->status_attr;
    return;
}

Private void
_XimInitialIMOffsetInfo()
{
    unsigned int	 n = XIMNumber(im_attr_info);
    register int	 i;

    for(i = 0; i < n; i++) {
	im_attr_info[i].quark = XrmStringToQuark(im_attr_info[i].name);
    }
    return;
}

Private void
_XimInitialICOffsetInfo()
{
    unsigned int	 n;
    register int	 i;

    n = XIMNumber(ic_attr_info);
    for(i = 0; i < n; i++) {
	ic_attr_info[i].quark = XrmStringToQuark(ic_attr_info[i].name);
    }

    n = XIMNumber(ic_pre_attr_info);
    for(i = 0; i < n; i++) {
	ic_pre_attr_info[i].quark = XrmStringToQuark(ic_pre_attr_info[i].name);
    }

    n = XIMNumber(ic_sts_attr_info);
    for(i = 0; i < n; i++) {
	ic_sts_attr_info[i].quark = XrmStringToQuark(ic_sts_attr_info[i].name);
    }
    return;
}

Private void
_XimInitialIMMode()
{
    unsigned int	n = XIMNumber(im_mode);
    register int	i;

    for(i = 0; i < n; i++) {
	im_mode[i].quark = XrmStringToQuark(im_mode[i].name);
    }
    return;
}

Private void
_XimInitialICMode()
{
    unsigned int	n = XIMNumber(ic_mode);
    register int	i;

    for(i = 0; i < n; i++) {
	ic_mode[i].quark = XrmStringToQuark(ic_mode[i].name);
    }
    return;
}

Public void
_XimInitialResourceInfo()
{
    static Bool	init_flag = False;

    if(init_flag == True) {
	return;
    }
    _XimInitialIMOffsetInfo();
    _XimInitialICOffsetInfo();
    _XimInitialIMMode();
    _XimInitialICMode();
    init_flag = True;
    return;
}
