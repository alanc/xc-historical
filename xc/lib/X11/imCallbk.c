/* $XConsortium: imCallbk.c,v 1.7 94/07/06 14:47:01 kaleb Exp kaleb $ */
/***********************************************************************
Copyright 1993 by Digital Equipment Corporation, Maynard, Massachusetts,
Copyright 1994 by FUJITSU LIMITED
Copyright 1994 by Sony Corporation

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital, FUJITSU
LIMITED and Sony Corporation not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.  

DIGITAL, FUJITSU LIMITED AND SONY CORPORATION DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL DIGITAL, FUJITSU LIMITED
AND SONY CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author: Hiroyuki Miyamoto  Digital Equipment Corporation
                             miyamoto@jrd.dec.com
  Modifier: Takashi Fujiwara FUJITSU LIMITED
			     fujiwara@a80.tech.yk.fujitsu.co.jp
	    Makoto Wakamatsu Sony Corporation
		 	     makoto@sm.sony.co.jp
				
***********************************************************************/

#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"
#include "XlcPubI.h"

#define sz_CARD8                 1
#define sz_INT8                  1
#define sz_CARD16                2
#define sz_INT16                 2
#define sz_BITMASK16             sz_CARD16
#define sz_CARD32                4
#define sz_INT32                 4
#define sz_BITMASK32             sz_CARD32
#define sz_XIMID                 sizeof(XIMID)
#define sz_XICID                 sizeof(XICID)
#define sz_XIMATTRID             sizeof(XIMATTRID)
#define sz_XICATTRID             sizeof(XICATTRID)
#define sz_ximPacketHeader       (XIM_HEADER_SIZE + sz_XIMID + sz_XICID)
#define sz_ximGeometry           0
#define sz_ximStrConversion      (sz_CARD32 + sz_CARD32 + sz_CARD32 + sz_CARD32)
#define sz_ximPreeditStart       0
#define sz_ximPreeditStartReply  sz_INT32
#define sz_ximPreeditCaret       (sz_INT32 + sz_CARD32 + sz_CARD32)
#define sz_ximPreeditCaretReply  sz_CARD32
#define sz_ximPreeditDone        0
#define sz_ximStatusStart        0
#define sz_ximStatusDone         0

typedef enum {
    XimCbSuccess, 
    XimCbNoCallback, 
    XimCbError, 
    XimCbQueued, 
    XimCbBadContextID, 
    XimCbBadOpcode
} XimCbStatus;

typedef XimCbStatus (*XimCb)(
#if NeedFunctionPrototypes
			     Xim, Xic, char*, int
#endif
			     );

#define PACKET_TO_MAJOROPCODE(p) (*(CARD8*)((CARD8*)(p)))
#define PACKET_TO_MINOROPCODE(p) (*(CARD8*)((CARD8*)(p) + sz_CARD8))
#define PACKET_TO_LENGTH(p) (*(CARD16*)((CARD8*)(p) + sz_CARD8 + sz_CARD8))
#define PACKET_TO_IMID(p) (*(XIMID*)((CARD8*)(p) + XIM_HEADER_SIZE))
#define PACKET_TO_ICID(p) (*(XICID*)((CARD8*)(p) + XIM_HEADER_SIZE + sz_XIMID))

#define _XimWriteData(im,len,data) \
    (im->private.proto.write((im),(len),(XPointer)(data)))
#define _XimReadData(im,buf,buf_len,len) \
    (im->private.proto.read((im),(XPointer)(buf),(buf_len),&(len)))
#define _XimFlushData(im) im->private.proto.flush((im))

#if NeedFunctionPrototypes
Public Bool _XimCbDispatch(Xim xim, INT16 len, XPointer data, XPointer call_data);
Private XimCbStatus _XimGeometryCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimStrConversionCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimPreeditStartCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimPreeditDoneCallback(Xim, Xic, char*, int);
Private void _read_text_from_packet(Xim, char*, XIMText*);
Private void _free_memory_for_text(XIMText*);
Private XimCbStatus _XimPreeditDrawCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimPreeditCaretCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimStatusStartCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimStatusDoneCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimStatusDrawCallback(Xim, Xic, char*, int);
Private XimCbStatus _XimPreeditStateNotifyCallback(Xim, Xic, char *, int);
#else
Public Bool _XimCbDispatch();
Private XimCbStatus _XimGeometryCallback();
Private XimCbStatus _XimStrConversionCallback();
Private XimCbStatus _XimPreeditStartCallback();
Private XimCbStatus _XimPreeditDoneCallback();
Private void _read_text_from_packet();
Private void _free_memory_for_text();
Private XimCbStatus _XimPreeditDrawCallback();
Private XimCbStatus _XimPreeditCaretCallback();
Private XimCbStatus _XimStatusStartCallback();
Private XimCbStatus _XimStatusDoneCallback();
Private XimCbStatus _XimStatusDrawCallback();
Private XimCbStatus _XimPreeditStateNotifyCallback();
#endif /* NeedFunctionPrototypes */

#if __STDC__
#define Const const
#else
#define Const /**/
#endif
#if __STDC__ && ((defined(sun) && defined(SVR4)) || defined(WIN32))
#define RConst /**/
#else
#define RConst Const
#endif

/* NOTE:
 * the table below depends on the protocol number 
 * defined in the IM Protocol document.
 */
static RConst XimCb callback_table[] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #000-009 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #010-019 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #020-029 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #030-039 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #040-049 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #050-059 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, /* #060-069 */
    _XimGeometryCallback,	/* #070 */
    _XimStrConversionCallback,	/* #071 */
    NULL,			/* #072 */
    _XimPreeditStartCallback,	/* #073 */
    NULL,			/* #074 */
    _XimPreeditDrawCallback,	/* #075 */
    _XimPreeditCaretCallback,	/* #076 */
    NULL,			/* #077 */
    _XimPreeditDoneCallback,	/* #078 */
    _XimStatusStartCallback,	/* #079 */
    _XimStatusDrawCallback,	/* #080 */
    _XimStatusDoneCallback,	/* #081 */
    _XimPreeditStateNotifyCallback	/* #082 */
    };


Private Bool
#if NeedFunctionPrototypes
_XimIsReadyForProcess(Xic ic)
#else
_XimIsReadyForProcess(ic)
  Xic ic;
#endif
{
    return(!ic->private.proto.waitCallback); /* check HM */
}

Private void
#if NeedFunctionPrototypes
_XimProcessPendingCallbacks(Xic ic)
#else
_XimProcessPendingCallbacks(ic)
  Xic ic;
#endif
{
    XimPendingCallback pcbq;

    while (((pcbq = ic->private.proto.pend_cb_que) != (XimPendingCallback)NULL) 
	   && _XimIsReadyForProcess(ic)) {
	XimCbStatus s;

	s = (*callback_table[pcbq->major_opcode])(pcbq->im, 
						  pcbq->ic, 
						  pcbq->proto, 
						  pcbq->proto_len);
	ic->private.proto.pend_cb_que = pcbq->next;
	Xfree(pcbq);		/* free memory of XimPendingCallback */
    }
}

Private void
#if NeedFunctionPrototypes
_XimPutCbIntoQueue(Xic ic, XimPendingCallback call_data)
#else
_XimPutCbIntoQueue(ic, call_data)
  Xic ic;
  XimPendingCallback call_data;
#endif
{
    XimPendingCallback pcbq = ic->private.proto.pend_cb_que;

    /* Queuing is FIFO
     */
    while (pcbq != (XimPendingCallback)NULL) {
	if (pcbq->next == (XimPendingCallback)NULL) {
	    break;
	}
	pcbq = pcbq->next;
    }
    if (pcbq == (XimPendingCallback)NULL) {
	ic->private.proto.pend_cb_que = call_data;
    }
    else {
	pcbq->next = call_data;
    }
}

Public Bool
#if NeedFunctionPrototypes
_XimCbDispatch(Xim xim, 
	       INT16 len, 
	       XPointer data, 
	       XPointer call_data)
#else
_XimCbDispatch(xim, len, data, call_data)
  Xim xim;
  INT16 len;
  XPointer data;
  XPointer call_data;
#endif
{
    /* `data' points to the beginning of the packet defined in IM Protocol doc.
     */
    int major_opcode = PACKET_TO_MAJOROPCODE(data);
    XIMID imid = PACKET_TO_IMID(data);
    XICID icid = PACKET_TO_ICID(data);
    Xim im = (Xim)call_data;	/* check HM */
    Xic ic = _XimICOfXICID(im, icid);
    char* proto;
    int proto_len;
    XimCbStatus status;

    /* check validity of im/ic
     */
    if ((imid != im->private.proto.imid) || !ic) {
	status = XimCbBadContextID;
	goto quit;
    }

    /* process pending callbacks
     */
    _XimProcessPendingCallbacks(ic);

    /* check if the protocol should be processed here
     */
    if (major_opcode > 82) {
	status = XimCbBadOpcode;
	goto quit;
    }
    if (!callback_table[major_opcode]) {
	status = XimCbBadOpcode;
	goto quit;
    }

    /* move the pointer ahead by the IM Protocol packet header size
     */
    proto = (char*)data + sz_ximPacketHeader;
    proto_len = (int)len - sz_ximPacketHeader;

    /* check if it can be processed right away
     * and if no, queue the protocol, otherwise invoke a callback
     */
    if (!_XimIsReadyForProcess(ic)) {

	/* queue the protocol
	 */
	XimPendingCallback pcb;

	pcb = (XimPendingCallback)Xmalloc(sizeof(XimPendingCallbackRec));
	if (pcb) {
	    pcb->major_opcode = major_opcode;
	    pcb->im = im;
	    pcb->ic = ic;
	    pcb->proto = proto;
	    pcb->proto_len = proto_len;
	    pcb->next = (XimPendingCallback)NULL; /* queue is FIFO */
	    _XimPutCbIntoQueue(ic, pcb);

	    status = XimCbQueued;
	}
	else {
	    status = XimCbError;
	}
    }
    else {
	
	/* invoke each callback according to the major opcode.
	 * `proto' points to the next address of IM-ID and IC-ID.
	 * `proto_len' specifies the packet length.
	 */
	status = (*callback_table[major_opcode])(im, 
						 ic, 
						 proto, 
						 proto_len);
    }

  quit:
    /* wrap up
     */
    switch (status) {
      case XimCbSuccess:
      case XimCbNoCallback:
      case XimCbError:
      case XimCbQueued:
	return(True);
	break;
      case XimCbBadContextID:
      case XimCbBadOpcode:
	return(False);
	break;
    }
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimGeometryCallback(Xim im, 
		     Xic ic, 
		     char* proto, 
		     int len)
#else
_XimGeometryCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.geometry_callback;

    /* invoke the callack
     */
    if (cb && cb->callback) {    
	(*cb->callback)((XIC)ic, cb->client_data, (XPointer)NULL);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimStrConversionCallback(Xim im, 
			  Xic ic, 
			  char* proto, 
			  int len)
#else
_XimStrConversionCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.string_conversion_callback; /* check HM */
    XIMStringConversionCallbackStruct cbrec;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	int p = XIM_HEADER_SIZE;
	cbrec.position = (XIMStringConversionPosition)
	    *(CARD32*)&proto[p]; p += sz_CARD32;
	cbrec.type = (XIMStringConversionType)
	    *(CARD32*)&proto[p]; p += sz_CARD32;
	cbrec.operation = (XIMStringConversionOperation)
	    *(CARD32*)&proto[p]; p += sz_CARD32;
	cbrec.factor = (unsigned short)
	    *(CARD32*)&proto[p];

	(*cb->callback)((XIC)ic, cb->client_data, &cbrec);
    }
    else {

	/* no callback registered
	 */
	_XimError(im, ic, 
		  (CARD16)XIM_BadSomething, 
		  (INT16)len, 
		  (CARD16)XIM_STR_CONVERSION, 
		  (char*)proto); /* send XIM_ERROR */
	return(XimCbNoCallback);
    }

    /* send a reply
     */
    {
	CARD8	*buf;
	INT16	 buf_len;
	int	 p, length_in_bytes, i;
	void	*tmp;

	/* Assumption:
	 * `cbrec.text->length' means the string length in characters
	 */
	{
	    length_in_bytes = (cbrec.text->encoding_is_wchar)?
		sizeof(wchar_t) * cbrec.text->length: /* wchar */
		strlen(cbrec.text->string.mbs);	/* mb */
	    buf_len = XIM_HEADER_SIZE + 
		sz_CARD16 +
		2 + length_in_bytes + 
		XIM_PAD(2 + length_in_bytes) +
		2 + 2 + sz_CARD32 * cbrec.text->length;
	    buf = (CARD8*)Xmalloc(buf_len);
	}
	_XimSetHeader((XPointer)buf, XIM_STR_CONVERSION_REPLY, 0, &buf_len);
	buf_len -= XIM_HEADER_SIZE; /* added by _XimSetHeader (HACK) */
	p = XIM_HEADER_SIZE;
	*(CARD16*)&buf[p] = (CARD16)im->private.proto.imid; p += sz_CARD16;
	*(CARD16*)&buf[p] = (CARD16)ic->private.proto.icid; p += sz_CARD16;
	*(CARD16*)&buf[p] = (CARD16)cbrec.text->length; p += sz_CARD16;
	memcpy(&buf[p],&cbrec.text->string.mbs,length_in_bytes);
	p += length_in_bytes;
	*(CARD16*)&buf[p] = (CARD16)(sz_CARD32*cbrec.text->length);
	p += XIM_PAD(2);
	for (i = 0; i < (int)cbrec.text->length; i++) {
	    *(CARD32*)&buf[p] = (CARD32)cbrec.text->feedback[i];
	    p += sz_CARD32;
	}

	if (!(_XimWriteData(im, buf_len, buf))) {
	    return(XimCbError);
	}
	_XimFlushData(im);

	Xfree(buf);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimPreeditStartCallback(Xim im, 
			 Xic ic, 
			 char* proto, 
			 int len)
#else
_XimPreeditStartCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.preedit_attr.start_callback;
    int ret;

    /* invoke the callback
     */
    if (cb && cb->callback){
	ret = (*(int (*)(
#if NeedFunctionPrototypes
			XIC, XPointer, XPointer
#endif
			))(cb->callback))((XIC)ic, 
					   cb->client_data, 
					   (XPointer)NULL);
    }
    else {

	/* no callback registered 
	 */
	_XimError(im, ic, 
		  (CARD16)XIM_BadSomething, 
		  (INT16)len, 
		  (CARD16)XIM_PREEDIT_START, 
		  (char*)proto); /* send XIM_ERROR */
	return(XimCbNoCallback);
    }

    /* send a reply
     */
    {
	CARD8 buf[sz_ximPacketHeader + sz_ximPreeditStartReply];
	INT16 buf_len = sz_XIMID + sz_XICID + sz_ximPreeditStartReply;
	int p;

	_XimSetHeader((XPointer)buf, XIM_PREEDIT_START_REPLY, 0, &buf_len);
	p = XIM_HEADER_SIZE;
	*(CARD16*)&buf[p] = (CARD16)im->private.proto.imid; p += sz_CARD16;
	*(CARD16*)&buf[p] = (CARD16)ic->private.proto.icid; p += sz_CARD16;
	*(INT32*)&buf[p]  = (INT32)ret;

	if (!(_XimWriteData(im, buf_len, buf))) {
	    return(XimCbError);
	}
	_XimFlushData(im);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimPreeditDoneCallback(Xim im, 
			Xic ic, 
			char* proto, 
			int len)
#else
_XimPreeditDoneCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.preedit_attr.done_callback;
    int ret;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	(*cb->callback)((XIC)ic, cb->client_data, (XPointer)NULL);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private void
#if NeedFunctionPrototypes
_read_text_from_packet(Xim im, 
		       char* buf, 
		       XIMText* text)
#else
_read_text_from_packet(im, buf, text)
  Xim im;
  char* buf;
  XIMText* text;
#endif
{
    int status;

    status = (int)*(BITMASK32*)buf; buf += sz_BITMASK32;

    /* string part
     */
    if (status & 0x00000001) /* "no string" bit on */ {
	text->length = 0;
	text->string.multi_byte = NULL;
	buf += sz_CARD16;	/* skip "length of preedit string" */
	buf += 2;		/* pad */
    }
    else {
	int tmp_len;
	char* tmp_buf;
	Status s;

	tmp_len = (int)*(CARD16*)buf;
	buf += sz_CARD16;
	if (tmp_buf = (char*)Xmalloc(tmp_len + 1)) {
	    memcpy(tmp_buf, buf, tmp_len);
	    tmp_buf[tmp_len] = '\0';

	    text->encoding_is_wchar = False;
	    text->length = im->methods->ctstowcs((XIM)im, 
					tmp_buf, tmp_len, 
					(wchar_t*)NULL, 0, 
					&s); /* CT? HM */
	    if (s != XLookupNone) {
		if (text->string.multi_byte
		    = (char*)Xmalloc(text->length * XLC_PUBLIC(im->core.lcd,mb_cur_max))) {
			int tmp;
			tmp = im->methods->ctstombs((XIM)im,
					   tmp_buf, tmp_len, 
					   text->string.multi_byte, text->length, 
					   &s);
			text->string.multi_byte
			  = (char*)Xrealloc(text->string.multi_byte, tmp+1);
			text->string.multi_byte[tmp] = '\0';
		}
	    }

	    Xfree(tmp_buf);
	}
	buf += tmp_len;

	buf += XIM_PAD(sz_CARD16 + tmp_len); /* pad */
    }

    /* feedback part
     */
    if (status & 0x00000002) /* "no feedback" bit on */ {
	text->feedback = (XIMFeedback*)NULL;
    }
    else {
	int i, j;

	i = (int)*(CARD16*)buf; buf += sz_CARD16;
	buf += sz_CARD16; /* skip `unused' */
	text->feedback = (XIMFeedback*)Xmalloc(i);
	j = 0;
	while (i > 0) {
	    text->feedback[j] = (XIMFeedback)*(CARD32*)buf;
	    buf += sz_CARD32;
	    i -= sz_CARD32;
	    j++;
	}
    }
}

Private void
#if NeedFunctionPrototypes
_free_memory_for_text(XIMText* text)
#else
_free_memory_for_text(text)
  XIMText* text;
#endif
{
    if (text) {
	if (text->string.multi_byte)
	    Xfree(text->string.multi_byte);
	if (text->feedback)
	    Xfree(text->feedback);
	Xfree(text);
    }
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimPreeditDrawCallback(Xim im, 
			Xic ic, 
			char* proto, 
			int len)
#else
_XimPreeditDrawCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.preedit_attr.draw_callback;
    XIMPreeditDrawCallbackStruct cbs;
    int p;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	p = 0;
	cbs.caret      = (int)*(INT32*)&proto[p]; p += sz_INT32;
	cbs.chg_first  = (int)*(INT32*)&proto[p]; p += sz_INT32;
	cbs.chg_length = (int)*(INT32*)&proto[p]; p += sz_INT32;
	if (cbs.text = (XIMText*)Xmalloc(sizeof(XIMText))) {
	    _read_text_from_packet(im, (char*)&proto[p], (XIMText*)cbs.text);
	}

	(*cb->callback)((XIC)ic, cb->client_data, &cbs);

	_free_memory_for_text((XIMText*)cbs.text);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimPreeditCaretCallback(Xim im, 
			 Xic ic, 
			 char* proto,
			 int len)
#else
_XimPreeditCaretCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.preedit_attr.caret_callback;
    XIMPreeditCaretCallbackStruct cbs;
    int p;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	p = XIM_HEADER_SIZE;
	cbs.position  = (int)*(INT32*)&proto[p]; p += sz_INT32;
	cbs.direction = (int)*(CARD32*)&proto[p]; p += sz_CARD32;
	cbs.style     = (int)*(CARD32*)&proto[p]; p += sz_CARD32;

	(*cb->callback)((XIC)ic, cb->client_data, &cbs);
    }
    else {

	/* no callback registered 
	 */
	_XimError(im, ic, 
		  (CARD16)XIM_BadSomething, 
		  (INT16)len, 
		  (CARD16)XIM_PREEDIT_CARET, 
		  (char*)proto); /* send XIM_ERROR */
	return(XimCbNoCallback);
    }

    /* Send a reply
     */
    {
	CARD8 buf[sz_ximPacketHeader + sz_ximPreeditCaretReply];
	INT16 len = sz_XIMID + sz_XICID + sz_ximPreeditCaretReply;
	int p;

	_XimSetHeader((XPointer)buf, XIM_PREEDIT_CARET_REPLY, 0, &len);
	p = XIM_HEADER_SIZE;
	*(CARD16*)&buf[p] = (CARD16)im->private.proto.imid; p += sz_CARD16;
	*(CARD16*)&buf[p] = (CARD16)ic->private.proto.icid; p += sz_CARD16;
	*(CARD32*)&buf[p] = (CARD32)cbs.position;

	if (!(_XimWriteData(im, len, buf))) {
	    return(XimCbError);
	}
	_XimFlushData(im);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimStatusStartCallback(Xim im, 
			Xic ic, 
			char* proto, 
			int len)
#else
_XimStatusStartCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.status_attr.start_callback;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	(*cb->callback)((XIC)ic, cb->client_data, (XPointer)NULL);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimStatusDoneCallback(Xim im, 
		       Xic ic, 
		       char* proto, 
		       int len)
#else
_XimStatusDoneCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.status_attr.done_callback;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	(*cb->callback)((XIC)ic, cb->client_data, (XPointer)NULL);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimStatusDrawCallback(Xim im, 
		       Xic ic, 
		       char* proto, 
		       int len)
#else
_XimStatusDrawCallback(im, ic, proto, len)
  Xim im;
  Xic ic;
  char* proto;
  int len;
#endif
{
    XIMCallback* cb = &ic->core.status_attr.draw_callback;
    XIMStatusDrawCallbackStruct cbs;

    /* invoke the callback
     */
    if (cb && cb->callback) {
	int p;

	p = XIM_HEADER_SIZE;
	cbs.type = (XIMStatusDataType)*(CARD32*)&proto[p]; p += sz_CARD32;
	if (cbs.type == XIMTextType) {
	    if (cbs.data.text = (XIMText*)Xmalloc(sizeof(XIMText))) {
		_read_text_from_packet(im, proto, cbs.data.text);
	    }
	}
	else if (cbs.type == XIMBitmapType) {
	    cbs.data.bitmap = (Pixmap)*(CARD32*)&proto[p];
	}

	(*cb->callback)((XIC)ic, cb->client_data, (XPointer)NULL);

	_free_memory_for_text((XIMText *)&cbs.data.text);
    }
    else {

	/* no callback registered
	 */
	return(XimCbNoCallback);
    }

    return(XimCbSuccess);
}

Private XimCbStatus
#if NeedFunctionPrototypes
_XimPreeditStateNotifyCallback( Xim im, Xic ic, char* proto, int len )
#else
_XimPreeditStateNotifyCallback( im, ic, proto, len )
    Xim		 im;
    Xic		 ic;
    char	*proto;
    int		 len;
#endif
{
    XIMCallback	*cb = &ic->core.preedit_attr.state_notify_callback;

    /* invoke the callack
     */
    if( cb  &&  cb->callback ) {    
	XIMPreeditStateNotifyCallbackStruct cbrec;

	cbrec.state = *(BITMASK32 *)proto;
	(*cb->callback)( (XIC)ic, cb->client_data, &cbrec );
    }
    else {
	/* no callback registered
	 */
	return( XimCbNoCallback );
    }

    return( XimCbSuccess );
}

