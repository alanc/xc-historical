/* $XConsortium: sm_manager.c,v 1.9 93/09/28 10:38:00 mor Exp $ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

#include <X11/SM/SMlib.h>
#include <X11/SM/SMlibint.h>

#include <stdio.h>
#include <sys/types.h>
#ifndef WIN32

#include <sys/socket.h>

#ifdef UNIXCONN
#include <sys/un.h>
#endif

#ifdef TCPCONN
# include <sys/param.h>
# include <netinet/in.h>
# ifndef hpux
#  ifdef apollo
#   ifndef NO_TCP_H
#    include <netinet/tcp.h>
#   endif
#  else
#   include <netinet/tcp.h>
#  endif
# endif
#endif

#ifdef DNETCONN
#include <netdnet/dn.h>
#endif

#else

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>
#undef close
#define close closesocket

#endif

#include <arpa/nameser.h>
#include <resolv.h>

static ns_inaddrtohostname ();



Status
SmsInitialize (vendor, release, newClientProc, managerData,
    errorLength, errorStringRet)

char 		 *vendor;
char 		 *release;
SmsNewClientProc newClientProc;
SmPointer	 managerData;
int  		 errorLength;
char 		 *errorStringRet;

{
    if (errorStringRet && errorLength > 0)
	*errorStringRet = '\0';

    if (!newClientProc)
    {
	strncpy (errorStringRet,
	    "The SmsNewClientProc callback can't be NULL", errorLength);

	return (0);
    }

    if (!_SmsOpcode)
    {
	void _SmsProtocolSetupNotifyProc ();

	if ((_SmsOpcode = IceRegisterForProtocolReply ("XSMP",
	    vendor, release, _SmVersionCount, _SmsVersions,
	    _SmsProtocolSetupNotifyProc,
	    _SmAuthCount, _SmsAuthRecs, NULL)) < 0)
	{
	    strncpy (errorStringRet,
	        "Could not register XSMP protocol with ICE", errorLength);

	    return (0);
	}
    }

    _SmsNewClientProc = newClientProc;
    _SmsNewClientData = managerData;

    return (1);
}



void
_SmsProtocolSetupNotifyProc (iceConn,
    majorVersion, minorVersion, vendor, release)

IceConn iceConn;
int	majorVersion;
int	minorVersion;
char  	*vendor;
char 	*release;

{
    SmsConn  	smsConn;

    smsConn = (SmsConn) malloc (sizeof (struct _SmsConn));

    smsConn->iceConn = iceConn;
    smsConn->proto_major_version = majorVersion;
    smsConn->proto_minor_version = minorVersion;
    smsConn->vendor = vendor;
    smsConn->release = release;
    smsConn->client_id = NULL;

    smsConn->save_yourself_in_progress = False;
    smsConn->interaction_allowed = SmInteractStyleNone;
    smsConn->can_cancel_shutdown = False;
    smsConn->interact_in_progress = False;

    _SmsConnectionObjs[_SmsConnectionCount++] = smsConn;


    /*
     * Now give the session manager the new smsConn and get back the
     * callbacks to invoke when messages arrive from the client.
     */

    (*_SmsNewClientProc) (smsConn, _SmsNewClientData, &smsConn->callbacks);
}



char *
SmsClientHostName (smsConn)

SmsConn smsConn;

{
    union {
	struct sockaddr		sa;
#ifdef UNIXCONN
	struct sockaddr_un	un;
#endif /* UNIXCONN */
#ifdef TCPCONN
	struct sockaddr_in	in;
#endif /* TCPCONN */
#ifdef DNETCONN
	struct sockaddr_dn	dn;
#endif /* DNETCONN */
    } from;

    int		fromlen = sizeof (from);
    char 	addr[1024];
    char	*hostname;

    if (getpeername (IceConnectionNumber (smsConn->iceConn),
	&from.sa, &fromlen) < 0)
    {
	return (NULL);
    }

    if (!fromlen)
	strcpy (addr, "local");
    else
    {
	switch (from.sa.sa_family)
	{
	case AF_UNSPEC:
#ifdef UNIXCONN
	case AF_UNIX:
#endif
	    strcpy (addr, "local");
	    break;
#ifdef TCPCONN
	case AF_INET:
            ns_inaddrtohostname (&from.in.sin_addr, addr, sizeof (addr));
	    break;
#endif
#ifdef DNETCONN
	case AF_DECnet:
	    sprintf (addr, "%s", dnet_ntoa (from.dn.sdn_add));
	    break;
#endif
	default:
	    strcpy (addr, "???");
	}
    }

    hostname = strdup (addr);

    return (hostname);
}



void
SmsRegisterClientReply (smsConn, clientId)

SmsConn smsConn;
char	*clientId;

{
    IceConn			iceConn = smsConn->iceConn;
    int				extra;
    smRegisterClientReplyMsg 	*pMsg;
    char 			*pData;

    extra = ARRAY8_BYTES (strlen (clientId));

    IceGetHeaderExtra (iceConn, _SmsOpcode, SM_RegisterClientReply,
	SIZEOF (smRegisterClientReplyMsg), WORD64COUNT (extra),
	smRegisterClientReplyMsg, pMsg, pData);

    STORE_ARRAY8 (pData, strlen (clientId), clientId);

    IceFlush (iceConn);

    smsConn->client_id = (char *) malloc (strlen (clientId) + 1);
    strcpy (smsConn->client_id, clientId);
}



void
SmsSaveYourself (smsConn, saveType, shutdown, interactStyle, fast)

SmsConn smsConn;
int	saveType;
Bool 	shutdown;
int	interactStyle;
Bool	fast;

{
    IceConn		iceConn = smsConn->iceConn;
    smSaveYourselfMsg	*pMsg;

    IceGetHeader (iceConn, _SmsOpcode, SM_SaveYourself,
	SIZEOF (smSaveYourselfMsg), smSaveYourselfMsg, pMsg);

    pMsg->saveType = saveType;
    pMsg->shutdown = shutdown;
    pMsg->interactStyle = interactStyle;
    pMsg->fast = fast;

    IceFlush (iceConn);

    smsConn->save_yourself_in_progress = True;

    if (interactStyle == SmInteractStyleNone ||
	interactStyle == SmInteractStyleErrors ||
	interactStyle == SmInteractStyleAny)
    {
	smsConn->interaction_allowed = interactStyle;
    }
    else
    {
	smsConn->interaction_allowed = SmInteractStyleNone;
    }

    smsConn->can_cancel_shutdown = 
	shutdown && interactStyle == SmInteractStyleAny;
}



void
SmsInteract (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_Interact);
    IceFlush (iceConn);

    smsConn->interact_in_progress = True;
}



void
SmsDie (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_Die);
    IceFlush (iceConn);
}



void
SmsShutdownCancelled (smsConn)

SmsConn smsConn;

{
    IceConn	iceConn = smsConn->iceConn;

    IceSimpleMessage (iceConn, _SmsOpcode, SM_ShutdownCancelled);
    IceFlush (iceConn);

    smsConn->can_cancel_shutdown = False;
}



void
SmsReturnProperties (smsConn, numProps, props)

SmsConn	smsConn;
int	numProps;
SmProp  **props;

{
    IceConn			iceConn = smsConn->iceConn;
    int 			bytes;
    smPropertiesReplyMsg	*pMsg;
    char 			*pBuf;
    char			*pStart;

    IceGetHeader (iceConn, _SmsOpcode, SM_PropertiesReply,
	SIZEOF (smPropertiesReplyMsg), smPropertiesReplyMsg, pMsg);

    LISTOF_PROP_BYTES (numProps, props, bytes);
    pMsg->length += WORD64COUNT (bytes);

    pBuf = pStart = IceAllocScratch (iceConn, bytes);

    STORE_LISTOF_PROPERTY (pBuf, numProps, props);

    IceWriteData (iceConn, bytes, pStart);
    IceFlush (iceConn);
}



void
SmsCleanUp (smsConn)

SmsConn smsConn;

{
    int i;

    for (i = 0; i < _SmsConnectionCount; i++)
	if (_SmsConnectionObjs[i] == smsConn)
	    break;

    if (i < _SmsConnectionCount)
    {
	IceProtocolShutdown (smsConn->iceConn, _SmsOpcode);

	if (i < _SmsConnectionCount - 1)
	{
	    _SmsConnectionObjs[i] =
		_SmsConnectionObjs[_SmsConnectionCount - 1];
	}

	_SmsConnectionCount--;

	if (smsConn->vendor)
	    free (smsConn->vendor);

	if (smsConn->release)
	    free (smsConn->release);

	if (smsConn->client_id)
	    free (smsConn->client_id);

	free ((char *) smsConn);
    }
}



/*
 * Get a host name from an Internet address.
 * Thanks to Keith Packard for this code.
 */

#define DATA_LEN 4096

struct ns_answer {
	u_short	type;
	u_short	class;
	u_short	len;
	u_char	data[DATA_LEN];
};

static
hostorderheader (h)
	HEADER	*h;
{
	h->id = ntohs(h->id);
	h->qdcount = ntohs(h->qdcount);
	h->ancount = ntohs(h->ancount);
	h->nscount = ntohs(h->nscount);
	h->arcount = ntohs(h->arcount);
}

static
do_query (type, class, string, answers, max)
	int			type, class;
	char			*string;
	struct ns_answer	*answers;
	int			max;
{
	int		i;
	int		msglen, anslen;
	u_char		msg[PACKETSZ];
	u_char		ans[PACKETSZ];
	HEADER		*h;
	u_char		*a, *eom;
	int		len;

	msglen = res_mkquery (QUERY, string, class, type,
			   (char *) 0, 0, (char *) 0,
			   msg, sizeof msg);
	anslen = res_send(msg, msglen, ans, sizeof ans);
	if (anslen < sizeof (HEADER))
		return 0;
	h = (HEADER *) ans;
	hostorderheader(h);
	a = ans + sizeof (HEADER);
	eom = ans + anslen;
	/* skip query stuff */
	if (h->qdcount)
	{
		for (i = 0; i < h->qdcount; i++)
		{
			len = dn_skipname(a, eom);
			if (len < 0)
				return 0;
			a += len;
			a += 2;	/* type */
			a += 2;	/* class */
		}
	}
	for (i = 0; i < h->ancount && i < max; i++)
	{
		len = dn_skipname (a, eom);
		if (len < 0)
			return 0;
		a += len;
		GETSHORT (answers[i].type, a);	/* type */
		GETSHORT (answers[i].class, a);	/* class */
		a += 4;				/* ttl */
		GETSHORT (len, a);		/* dlen */
		if (len > DATA_LEN)
			len = DATA_LEN;
		bcopy (a, answers[i].data, len);
		answers[i].len = len;
	}
	return i;
}

static
ns_inaddrtohostname (addr,name,max)
	struct in_addr	*addr;
	char		*name;
	int		max;
{
	struct ns_answer	answer;
	char			addrstring[256];
	u_char			*a;
	int			ret;
	int			l;

	a = (u_char *) addr;
	sprintf (addrstring, "%d.%d.%d.%d.IN-ADDR.ARPA",
	    a[3], a[2], a[1], a[0]);
	ret = do_query (T_PTR, C_IN, addrstring, &answer, 1);
	if (ret <= 0)
		return 0;
	if (answer.type != T_PTR || answer.class != C_IN ||
	    answer.len > max)
		return 0;
	a = (u_char *) answer.data;
	while (l = *a++) {
		while (l--)
			*name++ = (char) *a++;
		if (*a)
			*name++ = '.';
	}
	*name = '\0';
	return 1;
}
