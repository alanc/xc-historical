/* $XConsortium: XIE.h,v 1.3 94/01/12 19:36:23 rws Exp $ */
/*
 * $NCDOr: dispatch.c,v 1.2 1993/11/19 21:28:48 keithp Exp keithp $
 * $NCDId: @(#)dispatch.c,v 1.13 1994/02/03 01:48:48 lemke Exp $
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "assert.h"
#include "Xproto.h"
#include "opaque.h"
#include "lbx.h"
#include "lbxdata.h"
#include "wire.h"
#include "dixstruct.h"
#include "input.h"
#include "servermd.h"

extern int (* InitialVector[3]) ();
extern int (* ProcVector[256]) ();
extern int (* SwappedProcVector[256]) ();
extern void (* EventSwapVector[128]) ();
extern void (* ReplySwapVector[256]) ();
extern void Swap32Write(), SLHostsExtend(), SQColorsExtend(), WriteSConnectionInfo();
extern void WriteSConnSetupPrefix();
extern char *ClientAuthorized();
extern Bool InsertFakeRequest();
static void KillAllClients();
static void DeleteClientFromAnySelections();
extern void ProcessWorkQueue();

static int nextFreeClientID; /* always MIN free client ID */

static int	nClients;	/* number active clients */

char *display_name = 0;
char dispatchException = 0;
char isItTimeToYield;
Bool lbxUseLbx = TRUE;

#define MAJOROP ((xReq *)client->requestBuffer)->reqType

int
Dispatch ()
{
    register int        *clientReady;     /* array of request ready clients */
    register int	result;
    register ClientPtr	client;
    register int	nready;
    register long	**icheck = checkForInput;

    nextFreeClientID = 1;
/*    InitSelections(); */
    nClients = 0;

    clientReady = (int *) xalloc(sizeof(int) * MaxClients);
    if (!clientReady)
	FatalError("couldn't create client ready array");

    if (!ConnectToServer (display_name))
	FatalError("couldn't connect to X server");
    SendInitLBXPackets();
    
    while (!dispatchException)
    {
	nready = WaitForSomething(clientReady);

       /***************** 
	*  Handle events in round robin fashion, doing input between 
	*  each round 
	*****************/

	while (!dispatchException && (--nready >= 0))
	{
	    client = clients[clientReady[nready]];
	    if (! client)
	    {
		/* KillClient can cause this to happen */
		continue;
	    }
#ifdef NOTDEF
	    /* XXX what to do about grabs ? */
	    /* GrabServer activation can cause this to be true */
	    if (grabbingClient && (client != onlyClient))
		break;
#endif
	    isItTimeToYield = FALSE;
 
            requestingClient = client;
	    while (!isItTimeToYield)
	    {
		/* now, finally, deal with client requests */

	        result = ReadRequestFromClient(client);
	        if (result <= 0) 
	        {
		    if (result < 0)
			CloseDownClient(client);
		    break;
	        }

		client->sequence++;
#ifdef DEBUG
		if (client->requestLogIndex == MAX_REQUEST_LOG)
		    client->requestLogIndex = 0;
		client->requestLog[client->requestLogIndex] = MAJOROP;
		client->requestLogIndex++;
#endif
                LBXSequenceNumber(client)++;
		if (result > (MAX_REQUEST_SIZE << 2))
		    result = BadLength;
		else
		    result = (* client->requestVector[MAJOROP])(client);
	    
#ifdef NOTDEF
		if (result != Success) 
		{
		    if (client->noClientException != Success)
                        CloseDownClient(client);
                    else
		        SendErrorToClient(client, MAJOROP,
					  MinorOpcodeOfRequest(client),
					  client->errorValue, result);
		    break;
	        }
#endif
	    }
	    FlushAllOutput();
	}
    }
    KillAllClients();
    LbxCleanupSession();
    xfree (clientReady);
    dispatchException &= ~DE_RESET;
    return (dispatchException & DE_TERMINATE);
}

void
SetInputCheck(c0, c1)
    long *c0, *c1;
{
    checkForInput[0] = c0;
    checkForInput[1] = c1;
}

/************************
 * int NextAvailableClient(ospriv)
 *
 * OS dependent portion can't assign client id's because of CloseDownModes.
 * Returns NULL if there are no free clients.
 *************************/

extern unsigned long	StandardRequestLength ();

ClientPtr
NextAvailableClient(ospriv)
    pointer ospriv;
{
    register int i;
    register ClientPtr client;
    xReq data;

    i = nextFreeClientID;
    if (i == MAXCLIENTS)
	return (ClientPtr)NULL;
    clients[i] = client = (ClientPtr)xalloc(sizeof(ClientRec));
    if (!client)
	return (ClientPtr)NULL;
    if (!MakeLBXStuff(client)) {
    	xfree(client);
        return (ClientPtr)NULL;
    }
    client->index = i;
    client->sequence = 0; 
    client->clientAsMask = ((Mask)i) << CLIENTOFFSET;
    client->closeDownMode = DestroyAll;
    client->clientGone = FALSE;
    client->awaitingSetup = TRUE;
    client->saveSet = (pointer *)NULL;
    client->noClientException = Success;
    client->public.requestLength = StandardRequestLength;
#ifdef DEBUG
    client->requestLogIndex = 0;
#endif
    client->requestVector = InitialVector;
    client->osPrivate = ospriv;
    client->server = servers[0];    /* XXX want to use multiple servers */
    client->swapped = FALSE;
    if (!InitClientResources(client))
    {
	xfree(client);
	return (ClientPtr)NULL;
    }
    if (i == currentMaxClients)
	currentMaxClients++;
    while ((nextFreeClientID < MAXCLIENTS) && clients[nextFreeClientID])
	nextFreeClientID++;
    if (client->server)
    {
	data.reqType = 1;
	data.length = (sz_xReq + sz_xConnClientPrefix) >> 2;
	if (!InsertFakeRequest(client, (char *)&data, sz_xReq))
	{
	    xfree (client);
	    return (ClientPtr) NULL;
	}
    }
    return(client);
}

int
ProcInitialConnection(client)
    register ClientPtr client;
{
    REQUEST(xReq);
    register xConnClientPrefix *prefix;
    int whichbyte = 1;

    prefix = (xConnClientPrefix *)((char *)stuff + sz_xReq);
    if ((prefix->byteOrder != 'l') && (prefix->byteOrder != 'B'))
	return (client->noClientException = -1);
    if (((*(char *) &whichbyte) && (prefix->byteOrder == 'B')) ||
	(!(*(char *) &whichbyte) && (prefix->byteOrder == 'l')))
    {
	client->swapped = TRUE;
/*	SwapConnClientPrefix(prefix); */
    }
    stuff->reqType = 2;
    stuff->length += ((prefix->nbytesAuthProto + 3) >> 2) +
		     ((prefix->nbytesAuthString + 3) >> 2);
    if (client->swapped)
    {
	swaps(&stuff->length, whichbyte);
    }
    ResetCurrentRequest(client);
    return (client->noClientException);
}

int
ProcEstablishConnection(client)
    register ClientPtr client;
{
    char *reason, *auth_proto, *auth_string;
    register xConnClientPrefix *prefix;
    register xWindowRoot *root;
    register int i;
    REQUEST(xReq);

    prefix = (xConnClientPrefix *)((char *)stuff + sz_xReq);
    auth_proto = (char *)prefix + sz_xConnClientPrefix;
    auth_string = auth_proto + ((prefix->nbytesAuthProto + 3) & ~3);
    if ((prefix->majorVersion != X_PROTOCOL) ||
	(prefix->minorVersion != X_PROTOCOL_REVISION))
	reason = "Protocol version mismatch";
    else
	reason = ClientAuthorized(client,
				  (unsigned short)prefix->nbytesAuthProto,
				  auth_proto,
				  (unsigned short)prefix->nbytesAuthString,
				  auth_string);
    if (reason)
    {
	xConnSetupPrefix csp;
	char pad[3];

	csp.success = xFalse;
	csp.lengthReason = strlen(reason);
	csp.length = (csp.lengthReason + 3) >> 2;
	csp.majorVersion = X_PROTOCOL;
	csp.minorVersion = X_PROTOCOL_REVISION;
	if (client->swapped)
/*	    WriteSConnSetupPrefix(client, &csp) */;
	else
	    (void)WriteToClient(client, sz_xConnSetupPrefix, (char *) &csp);
        (void)WriteToClient(client, (int)csp.lengthReason, reason);
	if (csp.lengthReason & 3)
	    (void)WriteToClient(client, (int)(4 - (csp.lengthReason & 3)),
				pad);
	return (client->noClientException = -1);
    }

    nClients++;
/*    client->requestVector = client->swapped ? SwappedProcVector : ProcVector; */
    client->requestVector = ProcVector;
    client->sequence = 0;
    LBXSequenceNumber(client) = 0;
    /* wait for X server to kill client */
    client->closeDownMode = RetainPermanent;
    if (!NewClient (client))
	return (client->noClientException = -1);
    WriteToServer (client, (stuff->length << 2) - sz_xReq, prefix);
#ifdef NOTDEF
    ((xConnSetup *)ConnectionInfo)->ridBase = client->clientAsMask;
    ((xConnSetup *)ConnectionInfo)->ridMask = RESOURCE_ID_MASK;
    /* fill in the "currentInputMask" */
    root = (xWindowRoot *)(ConnectionInfo + connBlockScreenStart);
    for (i=0; i<screenInfo.numScreens; i++) 
    {
	register int j;
	register xDepth *pDepth;

        root->currentInputMask = WindowTable[i]->eventMask |
			         wOtherEventMasks (WindowTable[i]);
	pDepth = (xDepth *)(root + 1);
	for (j = 0; j < root->nDepths; j++)
	{
	    pDepth = (xDepth *)(((char *)(pDepth + 1)) +
				pDepth->nVisuals * sizeof(xVisualType));
	}
	root = (xWindowRoot *)pDepth;
    }
    if (client->swapped)
    {
	WriteSConnSetupPrefix(client, &connSetupPrefix);
	WriteSConnectionInfo(client,
			     (unsigned long)(connSetupPrefix.length << 2),
			     ConnectionInfo);
    }
    else
    {
	(void)WriteToClient(client, sizeof(xConnSetupPrefix),
			    (char *) &connSetupPrefix);
	(void)WriteToClient(client, (int)(connSetupPrefix.length << 2),
			    ConnectionInfo);
    }
#endif
    return (client->noClientException);
}

/**********************
 * CloseDownClient
 *
 *  Client can either mark his resources destroy or retain.  If retained and
 *  then killed again, the client is really destroyed.
 *********************/

Bool terminateAtReset = FALSE;

void
CloseDownClient(client)
    register ClientPtr client;
{
    if (!client->clientGone)
    {
#ifdef NOTDEF
	/* ungrab server if grabbing client dies */
	if (grabbingClient &&  (onlyClient == client))
	{
	    grabbingClient = FALSE;
	    ListenToAllClients();
	}
#endif
	CloseClient (client);
	
	/* clean up after client */
#ifdef NOTDEF
	DeleteClientFromAnySelections(client);
	ReleaseActiveGrabs(client);
	DeleteClientFontStuff(client);
#endif
	
	/* X server is telling us this client is dead */
	if (client->closeDownMode == DestroyAll)
	{
	    client->clientGone = TRUE;  /* so events aren't sent to client */
	    CloseDownConnection(client);
	    FreeClientResources(client);
	    if (ClientIsAsleep (client))
		ClientSignal (client);
	    if (client->index < nextFreeClientID)
		nextFreeClientID = client->index;
	    clients[client->index] = NullClient;
	    if ((client->requestVector != InitialVector) &&
		(client->server && client->server->serverClient != client) &&
		(--nClients == 0))
	    {
		if (terminateAtReset)
		    dispatchException |= DE_TERMINATE;
		else
		    dispatchException |= DE_RESET;
	    }
	    FreeLBXStuff(client);
	    xfree(client);
	}
	else
	{
	    client->clientGone = TRUE;
	    CloseDownConnection(client);
	    --nClients;
	}
    }
    else
    {
	/* really kill resources this time */
        FreeClientResources(client);
	if (ClientIsAsleep (client))
	    ClientSignal (client);
	if (client->index < nextFreeClientID)
	    nextFreeClientID = client->index;
	clients[client->index] = NullClient;
	FreeLBXStuff(client);
        xfree(client);
	if (nClients == 0)
	{
	    if (terminateAtReset)
		dispatchException |= DE_TERMINATE;
	    else
		dispatchException |= DE_RESET;
	}
    }

    while (!clients[currentMaxClients-1])
      currentMaxClients--;
}

static void
KillAllClients()
{
    int i;
    for (i=1; i<currentMaxClients; i++)
    {
        if (clients[i])
	{
	    clients[i]->closeDownMode = DestroyAll;   
            CloseDownClient(clients[i]);
	}
    }
}

ProcStandardRequest (client)
    ClientPtr	client;
{
    REQUEST (xReq);
    
    MakeLBXReply(client);
    WriteReqToServer (client, stuff->length << 2, stuff);
    return Success;
}

ProcBadRequest (client)
    ClientPtr	client;
{
    return BadRequest;
}

/* turn off optional features */
AdjustProcVector()
{
    extern Bool lbxUseTags;
    int         i;

    /*
     * to turn off all LBX request reencodings, set all proc vectors to
     * ProcStandardRequest
     */
    if (!lbxUseLbx) {
    	for (i = 1; i < 256; i++) {
            ProcVector[i] = ProcStandardRequest;
        }
    }
    /*
     * to turn off tag support, simply remove the tagged versions of X
     * requests
     */

    if (!lbxUseTags) {
	ProcVector[X_GetModifierMapping] = ProcStandardRequest;
	ProcVector[X_GetKeyboardMapping] = ProcStandardRequest;
    }
}
