/* $XConsortium: imDefIc.c,v 1.4 93/09/18 12:34:43 rws Exp $ */
/******************************************************************

           Copyright 1991, 1992 by Sun Microsystems, Inc.
           Copyright 1992, 1993 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and FUJITSU LIMITED makes no representations about
the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc. AND FUJITSU LIMITED
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Private Bool
#if NeedFunctionPrototypes
_XimGetICValuesCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimGetICValuesCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    Xic		 ic = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_GET_IC_VALUES_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid)
     && (buf_s[2] & XIM_ICID_VALID)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Private char *
_XimProtoGetICValues(xic, arg)
    XIC			 xic ;
    XIMArg		*arg ;
{
    Xic			 ic = (Xic)xic;
    Xim			 im = (Xim)ic->core.im;
    register XIMArg	*p;
    register XIMArg	*pp;
    register int	 n;
    int			 buf_len;
    CARD8		*buf;
    CARD16		*buf_s;
    INT16		 len;
    XPointer		 reply;
    char		*encode_name;
    char		*decode_name;

    for (n = 0, p = arg; p && p->name; p++) {
	n++;
	if ((strcmp(p->name, XNPreeditAttributes) == 0)
	 || (strcmp(p->name, XNStatusAttributes) == 0)) {
	     n++;
	     for (pp = (XIMArg *)p->value; pp && pp->name; pp++)
	 	n++;
	}
    }

    if (!n)
	return (char *)NULL;

    buf_len =  sizeof(CARD16) * n;
    buf_len += XIM_HEADER_SIZE
	     + sizeof(CARD16)
	     + sizeof(CARD16)
	     + sizeof(INT16)
	     + XIM_PAD(2 + buf_len);

    if (!(buf = (CARD8 *)Xmalloc(buf_len)))
	return arg->name;
    buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

    encode_name = _XimEncodeICAttrIDList(ic, ic->private.proto.ic_resources,
				ic->private.proto.ic_num_resources, arg,
				&buf_s[3], &len, XIM_GETICVALUES);

    if (!len)
	return encode_name;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = len;				/* length of ic-attr-id */
    len += sizeof(INT16);                       /* sizeof length of attr */
    XIM_SET_PAD(&buf_s[2], len);		/* pad */
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_GET_IC_VALUES, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf))) {
	Xfree(buf);
	return arg->name;
    }
    _XimFlush(im);
    Xfree(buf);
    if (!(_XimRecv(im, &len, &reply, _XimGetICValuesCheck, (XPointer)ic)))
	return arg->name;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return arg->name;
    }

    decode_name = _XimDecodeICATTRIBUTE(ic, ic->private.proto.ic_resources,
			ic->private.proto.ic_num_resources, &buf_s[4], buf_s[2],
			arg, XIM_GETICVALUES);
    Xfree(reply);
    if (decode_name)
	return decode_name;
    else
	return encode_name;
}

Private Bool
#if NeedFunctionPrototypes
_XimSetICValuesCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimSetICValuesCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    Xic		 ic = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_SET_IC_VALUES_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid)
     && (buf_s[2] & XIM_ICID_VALID)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Private char *
_XimProtoSetICValues(xic, arg)
    XIC			 xic;
    XIMArg		*arg;
{
    Xic			 ic = (Xic)xic;
    Xim			 im = (Xim)ic->core.im;
    char		*attr_buf;
    int			 buf_len;
    CARD8		*buf;
    CARD16		*buf_s;
    INT16		 len;
    XPointer		reply;
    BITMASK32		 flag = 0L;
    char		*name;

    name = _XimEncodeICATTRIBUTE(ic, ic->private.proto.ic_resources,
			ic->private.proto.ic_num_resources, arg,
			&attr_buf, &len, &flag, XIM_SETICVALUES);

    if (!len)
	return name;

    buf_len = XIM_HEADER_SIZE
            + sizeof(CARD16)
            + sizeof(CARD16)
            + sizeof(INT16)
            + sizeof(CARD16)
            + len;

    if (!(buf = (CARD8 *)Xmalloc(buf_len))) {
	Xfree(attr_buf);
	return arg->name;
    }
    buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

#ifdef EXT_MOVE
    if (_XimExtenMove(im, ic, flag, (CARD16 *)attr_buf, len))
	return name;
#endif

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = len;				/* length of ic-attribute */
    buf_s[3] = 0;				/* unused */
    (void)memcpy((char *)&buf_s[4], attr_buf, len);
    Xfree(attr_buf);
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(CARD16)			/* sizeof icid */
	 + sizeof(INT16)			/* sizeof length of attribute */
	 + sizeof(CARD16);			/* sizeof unused */

    _XimSetHeader((XPointer)buf, XIM_SET_IC_VALUES, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf))) {
	Xfree(buf);
	return arg->name;
    }
    _XimFlush(im);
    Xfree(buf);
    ic->private.proto.waitCallback = True;
    if (!(_XimRecv(im, &len, &reply, _XimSetICValuesCheck, (XPointer)ic))) {
	ic->private.proto.waitCallback = False;
	return arg->name;
    }
    ic->private.proto.waitCallback = False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return arg->name;
    }
    Xfree(reply);

    return name;
}

Private Bool
#if NeedFunctionPrototypes
_XimDestroyICCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimDestroyICCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    Xic		 ic = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_DESTROY_IC_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Private void
_XimProtoICFree(ic)
    Xic		 ic;
{
    if (ic->private.proto.preedit_font)
	Xfree(ic->private.proto.preedit_font);
    if (ic->private.proto.status_font)
	Xfree(ic->private.proto.status_font);
    if (ic->private.proto.commit_info)
	_XimFreeCommitInfo(ic);
    if (ic->private.proto.ic_resources)
	Xfree(ic->private.proto.ic_resources);
    if (ic->private.proto.ic_inner_resources)
	Xfree(ic->private.proto.ic_inner_resources);

    if (ic->core.hotkey)
	Xfree(ic->core.hotkey);

    return;
}

Private void
_XimProtoDestroyIC(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim	 	 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;

    if (IS_SERVER_CONNECTED(im)) {
	buf_s[0] = im->private.proto.imid;		/* imid */
	buf_s[1] = ic->private.proto.icid;		/* icid */

	len = sizeof(CARD16)			/* sizeof imid */
	    + sizeof(CARD16);			/* sizeof icid */

	_XimSetHeader((XPointer)buf, XIM_DESTROY_IC, 0, &len);
	(void)_XimSend(im, len, (XPointer)buf);
	_XimFlush(im);
	(void)_XimRecv(im, &len, &reply, _XimDestroyICCheck, (XPointer)ic);
	Xfree(reply);
    }
    _XimUnregisterFilter(ic);
    _XimProtoICFree(ic);
    return;
}

Private void
_XimProtoSetFocus(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
 
    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_SET_IC_FOCUS, 0, &len);
    (void)_XimSend(im, len, (XPointer)buf);
    _XimFlush(im);
    return;
}

Private void
_XimProtoUnsetFocus(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_UNSET_IC_FOCUS, 0, &len);
    (void)_XimSend(im, len, (XPointer)buf);
    _XimFlush(im);
    return;
}

Private Bool
#if NeedFunctionPrototypes
_XimResetICCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimResetICCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    Xic		 ic = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_RESET_IC_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid)
     && (buf_s[2] & XIM_ICID_VALID)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Private char *
_XimCommitedMbString(im, ic, buf)
    Xim			 im;
    Xic			 ic;
    XPointer		 buf;
{
    CARD16		*buf_s = (CARD16 *)buf;
    XimCommitInfo	 info;
    XimCommitInfo	 next;
    int			 len;
    int			 new_len;
    char		*commit;
    char		*new_commit = NULL;
    char		*str;
    Status		 status;

    if (ic->core.reset_return != XIMReturnNULL) {
	len = 0;
	for (info = ic->private.proto.commit_info; info; info = info->next) {
	    len += info->string_len;
	}
	len += buf_s[0];

	if (!(commit = (char *)Xmalloc(len + 1)))
	    goto Error_On_Reset;

	str = commit;
	for (info = ic->private.proto.commit_info; info; info = info->next) {
	    (void)memcpy(str, info->string, info->string_len);
	    str += info->string_len;
	}
	(void)memcpy(str, (char *)&buf_s[1], (int)&buf_s[0]);
	commit[len] = '\0';

	new_len = _Ximctstombs(im, commit, len, NULL, 0, &status);
	if (status != XLookupNone) {
	    if (!(new_commit = Xmalloc(new_len + 1))) {
		Xfree(commit);
		goto Error_On_Reset;
	    }
	    (void)_Ximctstombs(im, commit, len, new_commit, new_len, NULL);
	    new_commit[new_len] = '\0';
	}
	Xfree(commit);
    }

Error_On_Reset:
    for (info = ic->private.proto.commit_info; info;) {
	next = info->next;
	Xfree(info);
	info = next;
    }
    return new_commit;
}

Private char *
_XimProtoMbReset(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim	 	 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;
    char	*commit;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_RESET_IC, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf)))
	return NULL;
    _XimFlush(im);
    ic->private.proto.waitCallback = True;
    if (!(_XimRecv(im, &len, &reply, _XimResetICCheck, (XPointer)ic))) {
	ic->private.proto.waitCallback = False;
	return NULL;
    }
    ic->private.proto.waitCallback = False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return NULL;
    }

    commit = _XimCommitedMbString(im, ic, &buf_s[2]);

    Xfree(reply);
    return commit;
}

Private wchar_t *
_XimCommitedWcString(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    CARD16		*buf_s = (CARD16 *)buf;
    XimCommitInfo	 info;
    XimCommitInfo	 next;
    int			 len;
    int			 new_len;
    char		*commit;
    wchar_t		*new_commit = (wchar_t *)NULL;
    char		*str;
    Status		 status;

    if (ic->core.reset_return != XIMReturnNULL) {
	len = 0;
	for (info = ic->private.proto.commit_info; info; info = info->next) {
	    len += info->string_len;
	}
	len += buf_s[0];

	if (!(commit = (char *)Xmalloc(len + 1)))
	    goto Error_On_Reset;

	str = commit;
	for (info = ic->private.proto.commit_info; info; info = info->next) {
	    (void)memcpy(str, info->string, info->string_len);
	    str += info->string_len;
	}
	(void)memcpy(str, (char *)&buf_s[1], (int)&buf_s[0]);
	commit[len] = '\0';

	new_len = _Ximctstowcs(im, commit, len, NULL, 0, &status);
	if (status != XLookupNone) {
	    if (!(new_commit =
			 (wchar_t *)Xmalloc(sizeof(wchar_t) * (new_len + 1)))) {
		Xfree(commit);
		goto Error_On_Reset;
	    }
	    (void)_Ximctstowcs(im, commit, len, new_commit, new_len, NULL);
	    new_commit[new_len] = (wchar_t)'\0';
	}
	Xfree(commit);
    }

Error_On_Reset:
    for (info = ic->private.proto.commit_info; info;) {
	next = info->next;
	Xfree(info);
	info = next;
    }
    return new_commit;
}

Private wchar_t *
_XimProtoWcReset(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim	 	 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;
    wchar_t	*commit;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_RESET_IC, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf)))
	return NULL;
    _XimFlush(im);
    ic->private.proto.waitCallback = True;
    if (!(_XimRecv(im, &len, &reply, _XimResetICCheck, (XPointer)ic))) {
	ic->private.proto.waitCallback = False;
	return NULL;
    }
    ic->private.proto.waitCallback = False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return NULL;
    }

    commit = _XimCommitedWcString(im, ic, &buf_s[2]);

    Xfree(reply);
    return commit;
}

Private XICMethodsRec ic_methods = {
    _XimProtoDestroyIC,		/* destroy */
    _XimProtoSetFocus,		/* set_focus */
    _XimProtoUnsetFocus,	/* unset_focus */
    _XimProtoSetICValues,	/* set_values */
    _XimProtoGetICValues,	/* get_values */
    _XimProtoMbReset,		/* mb_reset */
    _XimProtoWcReset,		/* wc_reset */
    _XimProtoMbLookupString,	/* mb_lookup_string */
    _XimProtoWcLookupString,	/* wc_lookup_string */
};

Private Bool
_XimGetInputStyle(arg, input_style)
    XIMArg		*arg;
    XIMStyle		*input_style;
{
    register XIMArg	*p;

    for (p = arg; p && p->name; p++) {
	if (!(strcmp(p->name, XNInputStyle))) {
	    *input_style = (XIMStyle)p->value;
	    return True;
	}
    }
    return False;
}

Private Bool
#if NeedFunctionPrototypes
_XimCreateICCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimCreateICCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_CREATE_IC_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Public XIC
_XimProtoCreateIC(xim, arg)
    XIM			 xim;
    XIMArg		*arg;
{
    Xim			 im = (Xim)xim;
    Xic			 ic;
    INT16		 len;
    XIMResourceList	 res;
    unsigned int         num;
    XIMStyle		 input_style;
    char		*attr_buf = (char *)NULL;
    CARD8		*buf;
    CARD16		*buf_s;
    INT16		 buf_size;
    XPointer		 reply;

    if ((ic = (Xic)Xmalloc(sizeof(XicRec))) == (Xic)NULL)
	return (XIC)NULL;

    bzero((char *)ic, sizeof(XicRec));
    ic->methods = &ic_methods;
    ic->core.im = (XIM)im;
    ic->core.filter_events = im->private.proto.forward_event_mask;
    ic->private.proto.forward_event_mask =
				im->private.proto.forward_event_mask;
    ic->private.proto.synchronous_event_mask =
				im->private.proto.synchronous_event_mask;

    num = im->core.ic_num_resources;
    len = sizeof(XIMResource) * num;
    if (!(res = (XIMResourceList)Xmalloc(len)))
	return (XIC)NULL;
    (void)memcpy((char *)res, (char *)im->core.ic_resources, len);

    ic->private.proto.ic_resources     = res;
    ic->private.proto.ic_num_resources = num;

    num = im->private.proto.ic_num_inner_resources;
    len = sizeof(XIMResource) * num;
    if (!(res = (XIMResourceList)Xmalloc(len)))
	return (XIC)NULL;
    (void)memcpy((char *)res,
			 (char *)im->private.proto.ic_inner_resources, len);

    ic->private.proto.ic_inner_resources     = res;
    ic->private.proto.ic_num_inner_resources = num;

    if (IS_SERVER_CONNECTED(im)) {
	if (!(_XimGetInputStyle(arg, &input_style)))
	    goto ErrorOnCreatingIC;

	_XimSetICMode(ic->private.proto.ic_resources,
			ic->private.proto.ic_num_resources, input_style);
	_XimSetICMode(ic->private.proto.ic_inner_resources,
			ic->private.proto.ic_num_inner_resources, input_style);

	if (_XimEncodeICATTRIBUTE(ic,
			ic->private.proto.ic_resources,
			ic->private.proto.ic_num_resources,
			arg, &attr_buf, &len, 0, XIM_CREATEIC))
	    goto ErrorOnCreatingIC;

	if (!len)
	    goto ErrorOnCreatingIC;

	if (!(_XimCheckCreateICValues(ic->private.proto.ic_resources,
					ic->private.proto.ic_num_resources)))
	    goto ErrorOnCreatingIC;

	buf_size = XIM_HEADER_SIZE
		 + sizeof(CARD16)
		 + sizeof(INT16)
		 + len;

	if (!(buf = (CARD8 *)Xmalloc(buf_size)))
	    goto ErrorOnCreatingIC;
	buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

	buf_s[0] = im->private.proto.imid;	/* imid */
	buf_s[1] = len;				/* length of ic-attribute */
	(void)memcpy((char *)&buf_s[2], attr_buf, len);
	Xfree(attr_buf);
	attr_buf = (char *)NULL;
	len += sizeof(CARD16)			/* sizeof imid */
	     + sizeof(INT16); 			/* sizeof length */

	_XimSetHeader((XPointer)buf, XIM_CREATE_IC, 0, &len);
	if (!(_XimSend(im, len, (XPointer)buf))) {
	    Xfree(buf);
	    goto ErrorOnCreatingIC;
	}
	_XimFlush(im);
	Xfree(buf);
	ic->private.proto.waitCallback = True;
	if (!(_XimRecv(im, &len, &reply, _XimCreateICCheck, 0))) {
	    ic->private.proto.waitCallback = False;
	    goto ErrorOnCreatingIC;
	}
	ic->private.proto.waitCallback = False;
	buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
	if (*((CARD8 *)reply) == XIM_ERROR) {
	    _XimProcError(im, 0, (XPointer)&buf_s[3]);
	    Xfree(reply);
	    goto ErrorOnCreatingIC;
	}

	ic->private.proto.icid = buf_s[1];		/* icid */
	Xfree(reply);
	return (XIC)ic;
    } else {
	if (IS_UNCONNECTABLE(im))
	    goto ErrorOnCreatingIC;
	else {
	    /* 
	     * Not yet
	     */
	    return (XIC)ic;
	}
    }

ErrorOnCreatingIC:
    Xfree(ic);
    if (attr_buf)
	Xfree(attr_buf);
    return (XIC)NULL;
}
