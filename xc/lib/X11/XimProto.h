/* $XConsortium$ */
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

#ifndef _XIMPROTO_H
#define _XIMPROTO_H

/*
 * Default Preconnection selection target
 */
#define XIM_SERVERS		"XIM_SERVERS"
#define XIM_LOCALES		"LOCALES"
#define XIM_TRANSPORT		"TRANSPORT"

/*
 * categories in XIM_SERVERS
 */
#define XIM_SERVER_CATEGORY	"@server="
#define XIM_LOCAL_CATEGORY	"@locale="
#define XIM_TRANSPORT_CATEGORY	"@transport="

/*
 * Xim implementation revision
 */
#define PROTOCOLMAJORVERSION		0
#define PROTOCOLMINORVERSION		0

/*
 * Major Protocol number
 */
#define	XIM_CONNECT			0
#define	XIM_CONNECT_REPLY		1

#define XIM_AUTH_REQUIRED		2
#define XIM_AUTH_REPLY			3
#define XIM_AUTH_NEXT			4
#define XIM_AUTH_SETUP			5
#define XIM_AUTH_NG			6

#define	XIM_ERROR			7

#define	XIM_DISCONNECT			8
#define	XIM_DISCONNECT_REPLY		9
#define	XIM_OPEN			10
#define	XIM_OPEN_REPLY			11
#define	XIM_CLOSE			12
#define	XIM_CLOSE_REPLY			13
#define	XIM_REGISTER_TRIGGERKEYS	14
#define	XIM_TRIGGER_NOTIFY		15
#define	XIM_TRIGGER_NOTIFY_REPLY	16
#define	XIM_SET_EVENT_MASK		17
#define	XIM_ENCODING_NEGOTIATION	18
#define	XIM_ENCODING_NEGOTIATION_REPLY	19
#define	XIM_QUERY_EXTENSION		20
#define	XIM_QUERY_EXTENSION_REPLY	21
#define	XIM_GET_IM_VALUES		22
#define	XIM_GET_IM_VALUES_REPLY		23
#define XIM_CREATE_IC			24
#define	XIM_CREATE_IC_REPLY		25
#define	XIM_DESTROY_IC			26
#define	XIM_DESTROY_IC_REPLY		27
#define XIM_SET_IC_VALUES		28
#define	XIM_SET_IC_VALUES_REPLY		29
#define XIM_GET_IC_VALUES		30
#define XIM_GET_IC_VALUES_REPLY		31
#define	XIM_SET_IC_FOCUS		32
#define	XIM_UNSET_IC_FOCUS		33
#define	XIM_FORWARD_EVENT		34
#define	XIM_SYNC			35
#define	XIM_SYNC_REPLY			36
#define	XIM_COMMIT			37
#define	XIM_RESET_IC			38
#define	XIM_RESET_IC_REPLY		39

#define	XIM_GEOMETRY			40
#define	XIM_STR_CONVERSION		41
#define	XIM_STR_CONVERSION_REPLY	42

#define	XIM_PREEDIT_START		43
#define	XIM_PREEDIT_START_REPLY		44
#define	XIM_PREEDIT_DRAW		45
#define	XIM_PREEDIT_CARET		46
#define XIM_PREEDIT_CARET_REPLY		47
#define	XIM_PREEDIT_DONE		48

#define	XIM_STATUS_START		49
#define	XIM_STATUS_DRAW			50
#define	XIM_STATUS_DONE			51

#define XIM_EXTENSION			52

/*
 * values for the flag of XIM_ERROR
 */
#define	XIM_IMID_VALID			0x0001
#define	XIM_ICID_VALID			0x0002

/*
 * XIM Error Code
 */
#define XIM_BadAlloc			1
#define XIM_BadStyle			2
#define XIM_BadClientWindow		3
#define XIM_BadFocusWindow		4
#define XIM_BadArea			5
#define XIM_BadSpotLocation		6
#define XIM_BadColormap			7
#define XIM_BadAtom			8
#define XIM_BadPixel			9
#define XIM_BadPixmap			10
#define XIM_BadName			11
#define XIM_BadCursor			12
#define XIM_BadProtocol			13
#define XIM_BadForeground		14
#define XIM_BadBackground		15
#define XIM_LocaleNotSupported		16
#define XIM_BadSomething		999

/*
 * byte order
 */
#define BIGENDIAN	(CARD8)0x42	/* MSB first */
#define LITTLEENDIAN	(CARD8)0x6c	/* LSB first */

/*
 * values for the type of XIMATTR & XICATTR
 */
#define	XimType_SeparatorOfNestedList	0
#define	XimType_CARD8			1
#define	XimType_CARD16			2
#define	XimType_CARD32			3
#define	XimType_STRING8			4
#define	XimType_Window			5
#define	XimType_XIMStyles		10
#define	XimType_XRectangle		11
#define	XimType_XPoint			12
#define XimType_XFontSet		13
#define XimType_XIMOptions		14
#define XimType_XIMHotKeyTriggers	15
#define	XimType_NEST			0x7fff

/*
 * name of Separator of NestedList attribute
 */
#define	XNSeparatorofNestedList	"separatorofNestedList" /* XXX */

/*
 * name for the list of encodings supported in the IM library
 */
#define	XIM_COMPOUND_TEXT	"Compound-Text"		/* XXX */

/*
 * values for the category of XIM_ENCODING_NEGITIATON_REPLY
 */
#define	XIM_Encoding_NameCategory	0
#define	XIM_Encoding_DetailCategory	1

/*
 * value for the index of XIM_ENCODING_NEGITIATON_REPLY
 */
#define	XIM_Default_Encoding_IDX	-1

/*
 * value for the flag of XIM_FORWARD_EVENT
 */
#define XimSYNCHRONUS		  0x0001

/*
 * request packet header size
 */
#define XIM_HEADER_SIZE						\
	  sizeof(CARD8)		/* sizeof mejor-opcode */	\
	+ sizeof(CARD8)		/* sizeof minor-opcode */	\
	+ sizeof(INT16)		/* sizeof length */

/*
 * Client Message data size
 */
#define	XIM_CM_DATA_SIZE	20

/*
 * XIM data structure
 */
typedef CARD16	BITMASK16;
typedef CARD32	BITMASK32;
typedef CARD32	EVENTMASK;

typedef CARD16	XIMID;		/* Input Method ID */
typedef CARD16	XICID;		/* Input Context ID */

/*
 * Padding macro
 */
#define	XIM_PAD(length) ((4 - ((length) % 4)) % 4)

#define XIM_SET_PAD(ptr, length)					\
    {									\
	register int	 Counter = XIM_PAD((int)length);		\
	if (Counter) {							\
	    register char	*Ptr = (char *)(ptr) + (length);	\
	    length += Counter;						\
	    for (; Counter; --Counter, ++Ptr)				\
		*Ptr = '\0';						\
	}								\
    }

#endif /* _XIMPROTO_H */
