/* $XConsortium: imDefIc.c,v 1.3 93/09/18 11:00:44 rws Exp $ */
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
CheckIfMandatoryIsSet(ic)
    Xic 	ic;
{
    if(!(ic->private.proto.mandatory_mask & _XIM_INPUT_STYLE_MASK))
	return False;

    if(ic->core.input_style & XIMPreeditCallbacks) {
	/* Not yet */
	return False;
    } else if(ic->core.input_style & XIMStatusCallbacks) {
	/* Not yet */
	return False;
    }
    return True;
}

Public void
_XimFreeRemakeArg(arg)
    XIMArg	        *arg;
{
    register XIMArg	*p;

    for (p = arg; p->name; p++) {
	if (p->name)
	    Xfree(p->name);
    }
    Xfree(arg);
    return;
}

Private XIMArg *
_XimRemakeArg(arg, num)
    XIMArg		*arg;
    int			 num;
{
    XIMArg		*argp;
    XIMArg		*argp_ret;
    register XIMArg	*p;
    char		*name;
    int			 len = (num + 1) * sizeof(XIMArg);

    if (!(argp_ret = (XIMArg *)Xmalloc(len)))
	return NULL;
    bzero(argp_ret, len);

    for (argp = argp_ret, p = arg; (p->name && num); p++) {
	if (!(strcmp(p->name, XNClientWindow))
	 || !(strcmp(p->name, XNFocusWindow))) {
	    if (!(name = (char *)Xmalloc(strlen(p->name) + 1))) {
		_XimFreeRemakeArg(arg);
		return NULL;
	    }
	    (void)strcpy(name, p->name);
	    argp->name = name;
	    argp->value = p->value;
	    argp++;
	    num--;
	}
    }
    return argp_ret;
}

Private Bool
_XimGetICValuesCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
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
    return False;
}

Private char *
_XimProtoGetICValues(xic, arg)
    XIC		 xic ;
    XIMArg	*arg ;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 nCard;
    INT16	 len;
    XPointer	 reply;
    char	*name;

    if (name = _XimEncodeAttrIDList(im->core.ic_resources,
			im->core.ic_num_resources, arg, &buf_s[3], &nCard))
	return name;				/* list of ic-attr-id */

    len = sizeof(INT16) * nCard;
    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = len;				/* length of ic-attr-id */
    len += sizeof(INT16);                       /* sizeof length of attr */
    XIM_SET_PAD(&buf_s[2], len);		/* pad */
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_GET_IC_VALUES, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return arg->name;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
					_XimGetICValuesCheck, (XPointer)ic)))
	return arg->name;

    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    name = _XimDecodeATTRIBUTE(ic, im->core.ic_resources,
			im->core.ic_num_resources, &buf_s[4], buf_s[2],
			arg, _XIM_TOP_ATTR);
    Xfree(reply);
    return name;
}

Private Bool
_XimSetICValuesCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
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
    return False;
}

Private char *
_XimProtoSetICValues(xic, arg)
    XIC		 xic;
    XIMArg	*arg;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    INT16	 buf_size;
    XPointer	reply;
    BITMASK32	 flag = 0L;
    char	*error_name;
    int		 is_window = 0;
    XIMArg	*argp;

    buf_size = BUFSIZE			/* buffer size */
	     - XIM_HEADER_SIZE		/* packet header size */
	     - sizeof(INT16)		/* sizeof length of attribute */
	     - sizeof(CARD16);		/* sizeof unused */
    if (error_name = _XimEncodeATTRIBUTE(ic, im->core.ic_resources,
			im->core.ic_num_resources, &buf_s[4], buf_size, arg,
			&len, &flag, &is_window, _XIM_TOP_ATTR))
	return error_name;

    XFlush(im->core.display);
    if (is_window) {
	if (IS_PREVIOUS_FORWARDEVENT(im)) {
	    UNMARK_PREVIOUS_FORWARDEVENT(im);
	    if (!(_XimSync(im, ic)))
		return((char *)NULL); /* XXX */
	    if (!IS_FABLICATED(ic)) {
		MARK_NEED_PENDING_CALL(im);
		if (!(argp = _XimRemakeArg(arg, is_window)))
		    return((char *)NULL); /* XXX */
		(void)_XimRegPendingProc(im, ic, argp,
					(void (*)())_XimProtoSetICValues);
		return((char *)NULL);
	    }
	}
     }

#ifndef NOT_EXT_MOVE
    if (_XimExtenMove(im, ic, flag, &buf_s[4], len))
	return((char *)NULL);
#endif
    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = len;				/* length of ic-attribute */
    buf_s[3] = 0;				/* unused */
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(CARD16)			/* sizeof icid */
	 + sizeof(INT16)			/* sizeof length of attribute */
	 + sizeof(CARD16);			/* sizeof unused */

    _XimSetHeader((XPointer)buf, XIM_SET_IC_VALUES, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return arg->name;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
					_XimSetICValuesCheck, (XPointer)ic)))
	return arg->name;

    Xfree(reply);
    return((char *)NULL);
}

Private Bool
_XimDestroyICCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
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
_XimProtoDestroyIC(xic)
    XIC		 xic;
{
    Xic		 ic = (Xic)xic;
    Xim	 	 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_DESTROY_IC, 0, &len);
    (void)im->private.proto.send(im, len, (XPointer)buf);
    im->private.proto.flush(im);
    (void)im->private.proto.recv(im, &len, &reply,
				_XimDestroyICCheck, (XPointer)ic);
    Xfree(reply);
    _XimUnregisterKeyFilter(ic);
    if (ic->private.proto.xim_commit)
	Xfree(ic->private.proto.xim_commit);
    if (ic->private.proto.preedit_font)
	Xfree(ic->private.proto.preedit_font);
    if (ic->private.proto.status_font)
	Xfree(ic->private.proto.status_font);
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

    XFlush(im->core.display);
    if (IS_PREVIOUS_FORWARDEVENT(im)) {
	UNMARK_PREVIOUS_FORWARDEVENT(im);
	(void)_XimSync(im, ic);
	if (!IS_FABLICATED(ic)) {
	    MARK_NEED_PENDING_CALL(im);
	    (void)_XimRegPendingProc(im, ic, NULL, _XimProtoSetFocus);
	    return;
	}
    }

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_SET_IC_FOCUS, 0, &len);
    (void)im->private.proto.send(im, len, buf);
    im->private.proto.flush(im);
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

    XFlush(im->core.display);
    if (IS_PREVIOUS_FORWARDEVENT(im)) {
	UNMARK_PREVIOUS_FORWARDEVENT(im);
	(void)_XimSync(im, ic);
	if (!IS_FABLICATED(ic)) {
	    MARK_NEED_PENDING_CALL(im);
	    (void)_XimRegPendingProc(im, ic, NULL, _XimProtoUnsetFocus);
	    return;
	}
    }

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_UNSET_IC_FOCUS, 0, &len);
    (void)im->private.proto.send(im, len, buf);
    im->private.proto.flush(im);
    return;
}

Private Bool
_XimResetICCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
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
    return False;
}

Private char *
_XimCommitedMbString(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    CARD16	*buf_s = (CARD16 *)buf;
    char	*preedit;
    char	*commit;
    char	*new_commit;
    int		 size;
    int		 commit_size = 0;
    int		 reset_size;
    int		 len;

    if (ic->private.proto.xim_commit) {
	preedit = ic->private.proto.xim_commit;
	commit_size = strlen(preedit);
    }
    reset_size = buf_s[0];
    if (!(size = commit_size + reset_size))
	return NULL;

    size++;
    if (!(commit = Xmalloc(size)))
	return NULL;

    if (!(new_commit = Xmalloc(size)))
	return NULL;

    if (ic->private.proto.xim_commit) {
	memcpy(commit, preedit, commit_size);
	memcpy(commit + commit_size, (char *)&buf_s[1], reset_size);
	Xfree(ic->private.proto.xim_commit);
	ic->private.proto.xim_commit = NULL;
    } else
	memcpy(commit, (char *)&buf_s[1], reset_size);
    commit[size] = '\0';

    len = _Xlcctstombs(im->core.lcd, new_commit, commit, size);
    Xfree(commit);
    if (!len) {
	Xfree(new_commit);
	return NULL;
    }
    new_commit[len] = '\0';
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
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return NULL;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
					_XimResetICCheck, (XPointer)ic)))
	return NULL;

    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
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
    CARD16	*buf_s = (CARD16 *)buf;
    char	*preedit;
    char	*commit;
    wchar_t	*new_commit;
    int		 size;
    int		 commit_size = 0;
    int		 reset_size;
    int		 len;

    if (ic->private.proto.xim_commit) {
	preedit = ic->private.proto.xim_commit;
	commit_size = strlen(preedit);
    }
    reset_size = buf_s[0];
    if (!(size = commit_size + reset_size))
	return (wchar_t *)NULL;

    size++;
    if (!(commit = Xmalloc(size)))
	return (wchar_t *)NULL;

    if (!(new_commit = (wchar_t *)Xmalloc(size * sizeof(wchar_t))))
	return (wchar_t *)NULL;

    if (ic->private.proto.xim_commit) {
	memcpy(commit, preedit, commit_size);
	memcpy(commit + commit_size, (char *)&buf_s[1], reset_size);
	Xfree(ic->private.proto.xim_commit);
	ic->private.proto.xim_commit = NULL;
    } else
	memcpy(commit, (char *)&buf_s[1], reset_size);
    commit[size] = '\0';

    len = _Xlcctstowcs(im->core.lcd, new_commit, commit, size);
    Xfree(commit);
    if (!len) {
	Xfree(new_commit);
	return (wchar_t *)NULL;
    }
    new_commit[len] = '\0';
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
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return NULL;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
					_XimResetICCheck, (XPointer)ic)))
	return NULL;

    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
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
_XimCreateICCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_CREATE_IC_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Public XIC
_XimProtoCreateIC(xim, arg)
    XIM		 xim;
    XIMArg	*arg;
{
    Xim		 im = (Xim)xim;
    Xic		 ic;
    CARD8	 buf[BUFSIZE];
    CARD16      *buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    INT16	 buf_size;
    XPointer	 reply;
    BITMASK32	 flag = 0L;
    char	*error_name = NULL;
    int		 is_window = 0;
    XIMArg	*argp;

    if ((ic = (Xic)Xmalloc(sizeof(XicRec))) == (Xic)NULL)
	return (XIC)NULL;

    bzero((char *)ic, sizeof(XicRec));
    ic->methods = &ic_methods;
    ic->core.im = (XIM)im;
    ic->core.filter_events = im->private.proto.filter_events;
    ic->private.proto.filter_event_mask      =  0L;
    ic->private.proto.intercept_event_mask   =  0L;
    ic->private.proto.select_event_mask      =  XIM_FORWARD_EVENT_MASKS;
    ic->private.proto.forward_event_mask     =  XIM_FORWARD_EVENT_MASKS;
    ic->private.proto.synchronous_event_mask = ~XIM_FORWARD_EVENT_MASKS;

    if (IS_SERVER_CONNECTED(im)) {
	buf_size = BUFSIZE	 	/* buffer size */
		 - XIM_HEADER_SIZE	/* packet header size */
		 - sizeof(INT16) 	/* sizeof length of attribute */
		 - sizeof(CARD16); 	/* sizeof unused */
	if ((error_name = _XimEncodeATTRIBUTE(ic, im->core.ic_resources,
			im->core.ic_num_resources, &buf_s[2], buf_size, arg,
			&len, &flag, &is_window, _XIM_TOP_ATTR)))
	    goto ErrorOnCreatingIC;

	if (!(CheckIfMandatoryIsSet(ic)))
	    goto ErrorOnCreatingIC;

	XFlush(im->core.display);
	if (is_window) {
	    if (IS_PREVIOUS_FORWARDEVENT(im)) {
		UNMARK_PREVIOUS_FORWARDEVENT(im);
		if (!(_XimSync(im, ic)))
		    goto ErrorOnCreatingIC;
		if (!IS_FABLICATED(ic)) {
		    MARK_NEED_PENDING_CALL(im);
		    if (!(argp = _XimRemakeArg(arg, is_window)))
			goto ErrorOnCreatingIC;
		    (void)_XimRegPendingProc(im, ic, argp,
					(void (*)())_XimProtoSetICValues);
		    goto ErrorOnCreatingIC;
		}
	    }
	}

	buf_s[0] = im->private.proto.imid;	/* imid */
	buf_s[1] = len;				/* length of ic-attribute */
	len += sizeof(CARD16)			/* sizeof imid */
	     + sizeof(INT16); 			/* sizeof length */

	_XimSetHeader((XPointer)buf, XIM_CREATE_IC, 0, &len);
	if (!(im->private.proto.send(im, len, (XPointer)buf)))
	    goto ErrorOnCreatingIC;
	im->private.proto.flush(im);
	if (!(im->private.proto.recv(im, &len, &reply, _XimCreateICCheck, 0)))
	    goto ErrorOnCreatingIC;

	buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
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
    return (XIC)NULL;
}
