/* $XConsortium: imRmAttr.c,v 1.3 93/09/18 11:01:00 rws Exp $ */
/******************************************************************

           Copyright 1992, 1993 by FUJITSU LIMITED

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

#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Private XIMrmResourceList
_XimAttrNameToResource(res_list, res_num, name)
    register XIMrmResourceList	 res_list;
    unsigned int		 res_num;
    char			*name;
{
    XrmQuark			 quark;
    register unsigned int	 i;

    if (!name)
	return (XIMrmResourceList)NULL;
    quark = XrmPermStringToQuark(name);

    for (i = 0; i < res_num; i++, res_list++) {
	if (res_list->xrm_name == quark)
	    return res_list;
    }
    return (XIMrmResourceList)NULL;
}

Private Bool
_XimGetNestedListSeparator(id, res_list, res_num)
    CARD16		*id;
    XIMResourceList	 res_list;		/* LISTofIMATTR or IMATTR */
    unsigned int	 res_num;
{
    XIMrmResourceList	 res;

    if (!(res = _XimAttrNameToResource((XIMrmResourceList)res_list,
					res_num, XNSeparatorofNestedList)))
	return False;
    *id = (CARD16)res->id;
    return True;
}

Public char *
_XimEncodeAttrIDList(res_list, res_num, arg, idList, num)
    XIMResourceList	 res_list;
    unsigned int	 res_num;
    XIMArg		*arg;
    CARD16		*idList;
    INT16		*num;
{
    INT16		 new_num;
    register XIMArg	*p;
    XIMrmResourceList	 res;
    char		*name;
    CARD16		 id;

    if (!arg)
	return (char *)NULL;

    *num = 0;
    for (p = arg; p->name; p++) {
	if (!(res = _XimAttrNameToResource((XIMrmResourceList)res_list,
							res_num, p->name)))
	    return p->name;

	idList[(*num)++] = res->id;
	if (res->xrm_size == XimType_NEST) {
	    if (name = _XimEncodeAttrIDList(res_list, res_num,
			(XIMArg *)p->value, &idList[*num], &new_num))
		return name;

	    *num += new_num;
	    if (!(_XimGetNestedListSeparator(&id, res_list, res_num)))
		return False;
	    idList[(*num)++] = id;		/* Separator of NEST */
	}
    }
    return (char *)NULL;
}

Private Bool
_XimAttributeToValue(ic, res, buf, value, mode)
    Xic			  ic;
    XIMrmResourceList	  res;
    CARD16		 *buf;
    XPointer		  value;
    BITMASK32		  mode;
{
    switch (res->xrm_size) {
    case XimType_SeparatorOfNestedList:
	break;

    case XimType_CARD8:
    case XimType_CARD16:
    case XimType_CARD32:
    case XimType_Window:
	value = 0;
	_XCopyToArg((XPointer)&buf[2], (XPointer *)&value, buf[1]);
	break;

    case XimType_STRING8:
	if (!(value))
	    return False;
	_XCopyToArg((XPointer)&buf[2], (XPointer *)&value, buf[1]);
	break;

    case XimType_XIMStyles:
	{
	    INT16		 num = buf[2];
	    CARD32		*style_list = (CARD32 *)&buf[4];
	    XIMStyles		*rep;
	    XIMStyle		*style;
	    register int	 i;
	    char		*p;
	    int			 alloc_len;

	    if (!(value))
		return False;

	    alloc_len = sizeof(XIMStyles) + sizeof(XIMStyle) * num;
	    if (!(p = Xmalloc(alloc_len)))
		return False;

	    rep   = (XIMStyles *)p;
	    style = (XIMStyle *)&p[sizeof(XIMStyles)];

	    for (i = 0; i < num; i++)
		style[i] = (XIMStyle)style_list[i];

	    rep->count_styles = (unsigned short)num;
	    rep->supported_styles = style;
	    *((XIMStyles **)value) = rep;
	    break;
	}

    case XimType_XRectangle:
	{
	    CARD16	*buf_s = &buf[2];
	    XRectangle	*rep;

	    if (!(value))
		return False;

	    if (!(rep = (XRectangle *)Xmalloc(sizeof(XRectangle))))
		return False;

	    rep->x      = buf_s[0];
	    rep->y      = buf_s[1];
	    rep->width  = buf_s[2];
	    rep->height = buf_s[3];
	    *((XRectangle **)value) = rep;
	    break;
	}

    case XimType_XPoint:
	{
	    CARD16	*buf_s = &buf[2];
	    XPoint	*rep;

	    if (!(value))
		return False;

	    if (!(rep = (XPoint *)Xmalloc(sizeof(XPoint))))
		return False;

	    rep->x = buf_s[0];
	    rep->y = buf_s[1];
	    *((XPoint **)value) = rep;
	    break;
	}

    case XimType_XFontSet:
	{
	    INT16	 len = buf[2];
	    char	*base_name = (char *)&buf[3];
	    XFontSet	 rep = (XFontSet)NULL;

	    if (!(value))
		return False;
	    if (!ic)
		return False;

	    if (mode ==_XIM_PREEDIT_ATTR) {
		if ((len == ic->private.proto.preedit_font_length)
		 && (!strncmp(base_name, ic->private.proto.preedit_font, len)));
		    rep = ic->core.preedit_attr.fontset;
	    } else if (mode == _XIM_STATUS_ATTR) {
		if ((len == ic->private.proto.status_font_length)
		 && (!strncmp(base_name, ic->private.proto.status_font, len)));
		    rep = ic->core.status_attr.fontset;
	    }
	    if (!rep) {
		/*
		 * Not yet
		 */
		break;
	    }
			
	    *((XFontSet *)value) = rep;
	    break;
	}

    case XimType_XIMOptions:
	{
	    INT16		 num = buf[2];
	    CARD32		*option_list = (CARD32 *)&buf[4];
	    XIMOptionsList	*rep;
	    XIMOptions		*option;
	    register int	 i;
	    char		*p;
	    int			 alloc_len;

	    if (!(value))
		return False;

	    alloc_len = sizeof(XIMOptions) + sizeof(CARD32) + num;
	    if (!(p = Xmalloc(alloc_len)))
		return False;

	    rep    = (XIMOptionsList *)p;
	    option = (XIMOptions *)&p[sizeof(XIMOptionsList)];

	    for (i = 0; i < num; i++)
		option[i] = (XIMOptions)option_list[i];

	    rep->count_options      = num;
	    rep->supported_options = option;
	    *((XIMOptionsList **)value) = rep;
	    break;
	}

    case XimType_XIMHotKeyTriggers:
	{
	    INT32		       num = *((CARD32 *)&buf[2]);
	    register CARD32	      *key_list = (CARD32 *)&buf[4];
	    register XIMHotKeyTrigger *key;
	    register int	       i;
	    XIMHotKeyTriggers	      *rep;
	    char		      *p;
	    int			       alloc_len;

	    if (!(value))
		return False;

	    alloc_len = sizeof(XIMHotKeyTriggers)
		      + sizeof(XIMHotKeyTrigger) + num;
	    if (!(p = Xmalloc(alloc_len)))
		return False;

	    rep = (XIMHotKeyTriggers *)p;
	    key = (XIMHotKeyTrigger *)&p[sizeof(XIMHotKeyTriggers)];

	    for (i = 0; i < num; i++, key_list += 3) {
		key[i].keysym        = (KeySym)key_list[0]; /* keysym */
		key[i].modifier      = (int)key_list[1];    /* modifier */
		key[i].modifier_mask = (int)key_list[2];    /* modifier_mask */
	    }

	    rep->num_hot_key = num;
	    rep->key         = key;
	    *((XIMHotKeyTriggers **)value) = rep;
	    break;
	}

    default:
	return False;
    }
    return True;
}

Public char *
#if NeedFunctionPrototypes
_XimDecodeATTRIBUTE(
    Xic			 ic,
    XIMResourceList	 res_list,
    unsigned int	 res_num,
    CARD16		*data,
    INT16		 data_len,
    XIMArg		*arg,
    BITMASK32		 mode)
#else
_XimDecodeATTRIBUTE(ic, res_list, res_num,  data, data_len, arg, mode)
    Xic			 ic;
    XIMResourceList	 res_list;
    unsigned int	 res_num;
    CARD16		*data;
    INT16		 data_len;
    XIMArg		*arg;
    BITMASK32		 mode;
#endif /* NeedFunctionPrototypes */
{
    register XIMArg	*p;
    XIMrmResourceList	 res;
    CARD16		*buf;
    INT16		 len;
    INT16		 length;
    char		*name;
    INT16		 min_len = sizeof(CARD16)	/* sizeof attributeID */
			 	 + sizeof(INT16);	/* sizeof length */

    if (!arg)
	return (char *)NULL;

    for (p = arg; p->name; p++) {
	if (!(res = _XimAttrNameToResource((XIMrmResourceList)res_list,
							res_num, p->name)))
	    return p->name;

	for (len = data_len, buf = data; len >= min_len; ) {
	    if (res->id == buf[0])
		break;

	    length = buf[1];
	    length += XIM_PAD(length) + min_len;
	    buf = (CARD16 *)((char *)buf + length);
	    len -= length;
	}
	if (len < min_len)
	    return p->name;

	if (res->xrm_size == XimType_NEST) {
	    if (res->xrm_name == XrmPermStringToQuark(XNPreeditAttributes)) {
	        if ((name = _XimDecodeATTRIBUTE(ic, res_list, res_num,
			&buf[2], buf[1], (XIMArg *)p->value,
			_XIM_PREEDIT_ATTR)))
		    return name;
	    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusAttributes)) {
	        if ((name = _XimDecodeATTRIBUTE(ic, res_list, res_num,
			&buf[2], buf[1], (XIMArg *)p->value,
			_XIM_STATUS_ATTR)))
		    return name;
	    }
	} else {
	    if (!(_XimAttributeToValue(ic, res, buf, (XPointer)p->value, mode)))
		return name;
	}
    }
    return (char *)NULL;
}

Private Bool
_XimValueToAttribute(ic, res, buf, buf_size, value, is_window, len, mode)
    Xic			 ic;
    XIMrmResourceList	 res;
    CARD16		*buf;
    INT16		 buf_size;
    XPointer		 value;
    int			*is_window;
    INT16		*len;
    BITMASK32		 mode;
{
    Xim			 im = (Xim)ic->core.im;

    switch (res->xrm_size) {
    case XimType_SeparatorOfNestedList:
	*len = 0;
	break;

    case XimType_CARD8:
	*len = sizeof(CARD8);
	if (buf_size < *len)
	    return False;

	*((CARD8 *)buf) = (CARD8)value;
	break;

    case XimType_CARD16:
	*len = sizeof(CARD16);
	if (buf_size < *len)
	    return False;

	*buf = (CARD16)value;
	break;

    case XimType_CARD32:
	*len = sizeof(CARD32);
	if (buf_size < *len)
	    return False;

	*((CARD32 *)buf) = (CARD32)value;
	break;

    case XimType_STRING8:
	if (!value)
	    return False;

	*len = strlen((char *)value);
	if (buf_size < *len)
	    return False;

	memcpy((char *)buf, (char *)value, *len);
	break;

    case XimType_Window:
	*len = sizeof(CARD32);
	if (buf_size < *len)
	    return False;

	if (IS_PREVIOUS_FORWARDEVENT(im)) {
	    *len = 0;
	    (*is_window)++;
	} else
	    *((CARD32 *)buf) = (CARD32)value;
	break;

    case XimType_XRectangle:
	{
	    XRectangle	*rect = (XRectangle *)value;

	    if (!rect)
		return False;

	    *len = sizeof(INT16)		/* sizeof X */
	         + sizeof(INT16)		/* sizeof Y */
	         + sizeof(CARD16)		/* sizeof width */
	         + sizeof(CARD16);		/* sizeof height */
	    if (buf_size < *len)
		return False;

	    buf[0] = (CARD16)rect->x;		/* X */
	    buf[1] = (CARD16)rect->y;		/* Y */
	    buf[2] = (CARD16)rect->width;	/* width */
	    buf[3] = (CARD16)rect->height;	/* heght */
	    break;
	}

    case XimType_XPoint:
	{
	    XPoint	*point = (XPoint *)value;

	    if (!point)
		return False;

	    *len = sizeof(INT16)		/* sizeof X */
	         + sizeof(INT16);		/* sizeof Y */
	    if (buf_size < *len)
		return False;

	    buf[0] = (CARD16)point->x;		/* X */
	    buf[1] = (CARD16)point->y;		/* Y */
	    break;
	}

    case XimType_XFontSet:
	{
	    XFontSet	 font = (XFontSet)value;
	    char	*base_name = NULL;
	    int		 length;

	    if (!font)
		return False;

	    if (mode == _XIM_PREEDIT_ATTR) {
		base_name = ic->private.proto.preedit_font;
		length	  = ic->private.proto.preedit_font_length;
	    } else if (mode == _XIM_STATUS_ATTR) {
		base_name = ic->private.proto.status_font;
		length	  = ic->private.proto.status_font_length;
	    }

	    if (!base_name)
		return False;

	    *len = sizeof(CARD16)		/* sizeof length of Base name */
		 + length;			/* sizeof Base font name list */
	    XIM_SET_PAD(buf, *len);		/* pad */
	    if (buf_size < *len)
		return False;

	    buf[0] = (INT16)length;		/* length of Base font name */
	    memcpy((char *)&buf[1], base_name, length);
						/* Base font name list */
	    break;
	}

    case XimType_XIMOptions:
	{
	    XIMOptionsList	*option = (XIMOptionsList *)value;
	    INT16		 num = (INT16)option->count_options;
	    CARD32		*option_list = (CARD32 *)&buf[2];
	    register int	 i;

	    if (!option)
		return False;

	    *len = sizeof(INT16)		/* sizeof number of Options */
		 + sizeof(CARD16)		/* sizeof unused */
		 + sizeof(CARD32) * num;	/* sizeof Options List */
	    if (buf_size < *len)
		return False;

	    buf[0] = num;			/* sizeof number of Options */
	    buf[1] = 0;				/* unused */
	    for (i = 0; i < num; i++)
		option_list[i] = (CARD32)option->supported_options[i];
						/* options list */

	    break;
	}

    case XimType_XIMHotKeyTriggers:
	{
	    XIMHotKeyTriggers	*hotkey = (XIMHotKeyTriggers *)value;
	    INT32		 num = (INT32)hotkey->num_hot_key;
	    CARD32		*key = (CARD32 *)&buf[2];
	    register int	 i;

	    if (!hotkey)
		return False;

	    *len = sizeof(INT32)		/* sizeof number of key list */
	        + (sizeof(CARD32)		/* sizeof keysyn */
	        +  sizeof(CARD32)		/* sizeof modifier */
	        +  sizeof(CARD32))		/* sizeof modifier_mask */
	        * num;				/* number of key list */
	    if (buf_size < *len)
		return False;

	    *((CARD32 *)buf) = num;		/* number of key list */
	    for (i = 0; i < num; i++, key += 3) {
		key[0] = (CARD32)(hotkey->key[i].keysym);
						/* keysym */
		key[1] = (CARD32)(hotkey->key[i].modifier);
						/* modifier */
		key[2] = (CARD32)(hotkey->key[i].modifier_mask);
						/* modifier_mask */
	    }
	    break;
	}

    default:
	return False;
    }
    return True;
}

Private Bool
_XimEncodeTopValue(ic, res, p, valid)
    Xic			 ic;
    XIMrmResourceList	 res;
    XIMArg		*p;
    Bool		*valid;
{
    if (res->xrm_name == XrmPermStringToQuark(XNInputStyle)) {
	if (ic->private.proto.mandatory_mask & _XIM_INPUT_STYLE_MASK) {
	    *valid = False;
	    return True;
	}
	ic->core.input_style = (XIMStyle)p->value;
	ic->private.proto.mandatory_mask |= _XIM_INPUT_STYLE_MASK;

    } else if (res->xrm_name == XrmPermStringToQuark(XNClientWindow)) {
	if (ic->private.proto.mandatory_mask & _XIM_CLIENT_WIN_MASK)
	    return False; /* Can't change this value */

        ic->core.client_window = (Window)p->value;
	if (ic->core.focus_window == (Window)0)
	    ic->core.focus_window = ic->core.client_window;
	_XimRegisterKeyFilter(ic);
	ic->private.proto.mandatory_mask |= _XIM_CLIENT_WIN_MASK;

    } else if (res->xrm_name == XrmPermStringToQuark(XNFocusWindow)) {
	if (ic->core.client_window) {
	    _XimUnregisterKeyFilter(ic);
	    ic->core.focus_window = (Window)p->value;
	    _XimRegisterKeyFilter(ic);
	} else /* client_window not yet */
	    ic->core.focus_window = (Window)p->value;
	ic->private.proto.mandatory_mask |= _XIM_FOCUS_WIN_MASK;

    } else if (res->xrm_name == XrmPermStringToQuark(XNGeometryCallback)) {
	ic->core.geometry_callback.client_data =
		((XIMCallback *)p->value)->client_data;
	ic->core.geometry_callback.callback =
		((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNPreeditAttributes)) {
	;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusAttributes)) {
	;

    } else {
	*valid = False;
    }
    return True;
}

Private Bool
_XimEncodeResValue(ic, p, valid)
    Xic			 ic;
    XIMArg		*p;
    Bool		*valid;
{
    if (strcmp(p->name, XNResourceName) == 0) {
	char	*tmp;
	if (!(tmp = (char *)Xmalloc(strlen((char *)p->value) + 1)))
	    return False;
	(void)strcpy(tmp, (char *)p->value);
	if (ic->core.res_name)
	    Xfree(ic->core.res_name);
	ic->core.res_name = tmp;

    } else if (strcmp(p->name, XNResourceClass) == 0) {
	char	*tmp;
	if (!(tmp = (char *)Xmalloc(strlen((char *)p->value) + 1)))
	    return False;
	(void)strcpy(tmp, (char *)p->value);
	if (ic->core.res_class)
	    Xfree(ic->core.res_class);
	ic->core.res_class = tmp;

    } else {
	return False;
    }
    return True;
}

Private Bool
_XimEncodePreeditValue(ic, res, p, valid)
    Xic			 ic;
    XIMrmResourceList	 res;
    XIMArg		*p;
    Bool		*valid;
{
    ICAttributesPtr	 attr = &ic->core.preedit_attr;

    if (res->xrm_name == XrmPermStringToQuark(XNArea)) {
	attr->area.x      = ((XRectangle *)p->value)->x;
	attr->area.y      = ((XRectangle *)p->value)->y;
	attr->area.width  = ((XRectangle *)p->value)->width;
	attr->area.height = ((XRectangle *)p->value)->height;
	
    } else if (res->xrm_name == XrmPermStringToQuark(XNAreaNeeded)) {
	attr->area_needed.width   = ((XRectangle *)p->value)->width;
	attr->area_needed.height  = ((XRectangle *)p->value)->height;

    } else if (res->xrm_name == XrmPermStringToQuark(XNSpotLocation)) {
	attr->spot_location.x = ((XPoint *)p->value)->x;
	attr->spot_location.y = ((XPoint *)p->value)->y;

    } else if (res->xrm_name == XrmPermStringToQuark(XNColormap)) {
	attr->colormap = (Colormap)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNForeground)) {
	attr->foreground = (unsigned long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNBackground)) {
	attr->background = (unsigned long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNBackgroundPixmap)) {
	attr->background_pixmap = (Pixmap)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNFontSet)) {
	int		  list_ret;
	XFontStruct	**struct_list;
	char		**name_list;
	char		 *tmp;
	int		  len;
	register int	  i;

	if (!p->value)
	    return False;

	attr->fontset = (XFontSet)p->value;
	if (ic->private.proto.preedit_font)
	    Xfree(ic->private.proto.preedit_font);

	list_ret = XFontsOfFontSet(attr->fontset, &struct_list, &name_list);
	for(i = 0, len = 0; i < list_ret; i++) {
	     len += (strlen(name_list[i]) + sizeof(char));
	}
	if(!(tmp = Xmalloc(len)))
	     return False;

	tmp[0] = '\0';
	for(i = 0; i < list_ret; i++) {
	    strcat(tmp, name_list[i]);
	    strcat(tmp, ",");
	}
	tmp[len] = 0;
	ic->private.proto.preedit_font        = tmp;
	ic->private.proto.preedit_font_length = len;

    } else if (res->xrm_name == XrmPermStringToQuark(XNLineSpace)) {
	attr->line_space = (long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNCursor)) {
	attr->cursor = (Cursor)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusStartCallback)) {
	attr->callbacks.start.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.start.callback  =
			((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusDoneCallback)) {
	attr->callbacks.done.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.done.callback  =
			((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusDrawCallback)) {
	attr->callbacks.draw.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.draw.callback  =
			((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNPreeditCaretCallback)) {
	attr->callbacks.caret.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.caret.callback  =
			((XIMCallback *)p->value)->callback;

    } else {
	*valid = False;
    }
    return True;
}

Private Bool
_XimEncodeStatusValue(ic, res, p, valid)
    Xic			 ic;
    XIMrmResourceList	 res;
    XIMArg		*p;
    Bool		*valid;
{
    ICAttributesPtr	 attr = &ic->core.status_attr;

    if (res->xrm_name == XrmPermStringToQuark(XNArea)) {
	attr->area.x      = ((XRectangle *)p->value)->x;
	attr->area.y      = ((XRectangle *)p->value)->y;
	attr->area.width  = ((XRectangle *)p->value)->width;
	attr->area.height = ((XRectangle *)p->value)->height;
	
    } else if (res->xrm_name == XrmPermStringToQuark(XNAreaNeeded)) {
	attr->area_needed.width   = ((XRectangle *)p->value)->width;
	attr->area_needed.height  = ((XRectangle *)p->value)->height;

    } else if (res->xrm_name == XrmPermStringToQuark(XNColormap)) {
	attr->colormap = (Colormap)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNForeground)) {
	attr->foreground = (unsigned long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNBackground)) {
	attr->background = (unsigned long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNBackgroundPixmap)) {
	attr->background_pixmap = (Pixmap)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNFontSet)) {
	int		  list_ret;
	XFontStruct	**struct_list;
	char		**name_list;
	char		 *tmp;
	int		  len;
	register int	  i;

	if (!p->value)
	    return False;

	attr->fontset = (XFontSet)p->value;
	if (ic->private.proto.status_font)
	    Xfree(ic->private.proto.status_font);

	list_ret = XFontsOfFontSet(attr->fontset, &struct_list, &name_list);
	for(i = 0, len = 0; i < list_ret; i++) {
	     len += (strlen(name_list[i]) + sizeof(char));
	}
	if(!(tmp = Xmalloc(len)))
	     return False;

	tmp[0] = '\0';
	for(i = 0; i < list_ret; i++) {
	    strcat(tmp, name_list[i]);
	    strcat(tmp, ",");
	}
	tmp[len] = 0;
	ic->private.proto.status_font        = tmp;
	ic->private.proto.status_font_length = len;

    } else if (res->xrm_name == XrmPermStringToQuark(XNLineSpace)) {
	attr->line_space = (long)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNCursor)) {
	attr->cursor = (Cursor)p->value;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusStartCallback)) {
	attr->callbacks.start.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.start.callback  =
			((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusDoneCallback)) {
	attr->callbacks.done.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.done.callback  =
			((XIMCallback *)p->value)->callback;

    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusDrawCallback)) {
	attr->callbacks.draw.client_data =
			((XIMCallback *)p->value)->client_data;
	attr->callbacks.draw.callback  =
			((XIMCallback *)p->value)->callback;

    } else {
	*valid = False;
    }
    return True;
}

Public char *
#if NeedFunctionPrototypes
_XimEncodeATTRIBUTE(
    Xic			 ic,
    XIMResourceList	 res_list,
    unsigned int	 res_num,
    CARD16		*buf,
    INT16		 buf_size,
    XIMArg		*arg,
    INT16		*total_length,
    BITMASK32		*flag,
    int			*is_window,
    BITMASK32		 mode)
#else
_XimEncodeATTRIBUTE(ic, res_list, res_num, buf, buf_size, arg, total_length, flag, is_window, mode)
    Xic			 ic;
    XIMResourceList	 res_list;
    unsigned int	 res_num;
    CARD16		*buf;
    INT16		 buf_size;
    XIMArg		*arg;
    INT16		*total_length;
    BITMASK32		*flag;
    int			*is_window;
    BITMASK32		 mode;
#endif /* NeedFunctionPrototypes */
{
    register XIMArg	*p;
    XIMrmResourceList	 res;
    INT16		 len;
    Bool		 valid;

    if (!arg)
	return (char *)NULL;

    *total_length = 0;
    for (p = arg; p->name; p++) {
	if (!(res = _XimAttrNameToResource((XIMrmResourceList)res_list,
							res_num, p->name))) {
	    if (!(_XimEncodeResValue(ic, p, valid)))
		return p->name;
	    continue;
	    }

	valid = True;
	switch (mode) {
	case _XIM_TOP_ATTR:
	    if (!(_XimEncodeTopValue(ic, res, p, &valid)))
		return p->value;
	    break;

	case _XIM_PREEDIT_ATTR:
	    if (!(_XimEncodePreeditValue(ic, res, p, &valid)))
		return p->value;
	    break;

	case _XIM_STATUS_ATTR:
	    if (!(_XimEncodeStatusValue(ic, res, p, &valid)))
		return p->value;
	    break;

	default:
	    valid = False;
	}
	if (!valid)
	    continue;

	buf_size -= (sizeof(CARD16)		/* sizeof attribute ID */
		   + sizeof(INT16));		/* sizeof value length */
	if (res->xrm_size == XimType_NEST) {
	    if (res->xrm_name == XrmPermStringToQuark(XNPreeditAttributes)) {
	    	if (_XimEncodeATTRIBUTE(ic, res_list, res_num,  &buf[2],
				buf_size, (XIMArg *)p->value,
				&len, flag, is_window, _XIM_PREEDIT_ATTR))
		    return p->name;
	    } else if (res->xrm_name == XrmPermStringToQuark(XNStatusAttributes)) {
	    	if (_XimEncodeATTRIBUTE(ic, res_list, res_num,  &buf[2],
				buf_size, (XIMArg *)p->value,
				&len, flag, is_window, _XIM_STATUS_ATTR))
		    return p->name;
	    }
	} else {
#ifndef NOT_EXT_MOVE
	    if (flag)
		*flag |= _XimExtenArgCheck(p);
#endif
	    if (!( _XimValueToAttribute(ic, res, &buf[2], buf_size,
				(XPointer)p->value, is_window, &len, mode)))
		return p->name;			/* value */
	}
	if (!len)
	    continue;

	buf[0] = res->id;			/* attribute ID */
	buf[1] = len;				/* value length */
	XIM_SET_PAD(&buf[2], len);		/* pad */
	len += sizeof(CARD16)			/* sizeof attribute ID */
	     + sizeof(INT16);			/* sizeof value length */
	buf = (CARD16 *)((char *)buf + len);
	*total_length += len;
    }
    return (char *)NULL;
}

Private unsigned int
_XimCountNumberOfAttr(total, attr)
    INT16	 total;
    CARD16	*attr;
{
    unsigned int n = 0;
    INT16	 length;
    INT16	 min_len = sizeof(CARD16)	/* sizeof attrinute ID */
			 + sizeof(CARD16)	/* sizeof type of value */
			 + sizeof(INT16);	/* sizeof length of attribute */

    while (total > min_len) {
	length = attr[2];
	length += (min_len + XIM_PAD(length + 2));
	total -= length;
	attr = (CARD16 *)((char *)attr + length);
	n++;
    }
    return n;
}

Public Bool
_XimGetAttributeID(im, buf)
    Xim		     im;
    CARD16	    *buf;
{
    unsigned int     n;
    int		     alloc_len;
    XIMResourceList  res;
    char	    *names;
    int		     names_len;
    CARD16	    *bufp;
    register int     i;
    INT16	     len;
    INT16	     min_len = sizeof(CARD16)	/* sizeof attrinute ID */
			     + sizeof(CARD16)	/* sizeof type of value */
			     + sizeof(INT16);	/* sizeof length of attribute */
    /*
     * IM attribute ID
     */

    len = buf[0];
    if (!(n = _XimCountNumberOfAttr(len, &buf[1])))
	return False;
    alloc_len = sizeof(XIMResource) * n;
    buf++;

    if (!(res = (XIMResourceList)Xmalloc(len)))
	return False;
    bzero(res, len);

    names_len = 0;
    bufp = buf;
    for (i = 0; i < n; i++) {
	len = buf[2];
	names_len += (len + 1);
	len += (min_len + XIM_PAD(len + 2));
	buf = (CARD16 *)((char *)buf + len);
    }

    if (!(names = (char *)Xmalloc(names_len)))
	return False;

    buf = bufp;
    for (i = 0; i < n; i++) {
	len = buf[2];
	memcpy(names, (char *)&buf[3], len);
	names[len] = '\0';
	res[i].resource_name = names;
	res[i].resource_size = buf[1];
	res[i].id	     = buf[0];
	names += (len + 1);
	len += (min_len + XIM_PAD(len + 2));
	buf = (CARD16 *)((char *)buf + len);
    }

    _XIMCompileResourceList(res, n);
    im->core.im_resources     = res;
    im->core.im_num_resources = n;
    im->private.proto.im_attribute_name = names - names_len;

    /*
     * IC attribute ID
     */

    len = buf[0];
    if (!(n = _XimCountNumberOfAttr(len, &buf[2])))
	return False;
    alloc_len = sizeof(XIMResource) * n;
    buf += 2;

    if (!(res = (XIMResourceList)Xmalloc(len)))
	return False;
    bzero(res, len);

    names_len = 0;
    bufp = buf;
    for (i = 0; i < n; i++) {
	len = buf[2];
	names_len += (len + 1);
	len += (min_len + XIM_PAD(len + 2));
	buf = (CARD16 *)((char *)buf + len);
    }

    if (!(names = (char *)Xmalloc(names_len)))
	return False;

    buf = bufp;
    for (i = 0; i < n; i++) {
	len = buf[2];
	memcpy(names, (char *)&buf[3], len);
	names[len] = '\0';
	res[i].resource_name = names;
	res[i].resource_size = buf[1];
	res[i].id	     = buf[0];
	names += (len + 1);
	len += (min_len + XIM_PAD(len + 2));
	buf = (CARD16 *)((char *)buf + len);
    }

    _XIMCompileResourceList(res, n);
    im->core.ic_resources     = res;
    im->core.ic_num_resources = n;
    im->private.proto.ic_attribute_name = names - names_len;

    return True;
}
