/* $XConsortium: main.c,v 1.4 94/02/20 11:14:04 dpw Exp $ */
/*
 * $NCDOr$
 * $NCDId: @(#)main.c,v 1.13 1994/03/04 00:12:28 lemke Exp $
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
#include "Xproto.h"
#include "input.h"
#include "misc.h"
#include "os.h"
#include "resource.h"
#include "lbx.h"
#include "opaque.h"
#include "servermd.h"
#include "site.h"
#include "wire.h"

char	*display = "10";

CARD32	ScreenSaverTime = 0;

extern Bool NoticeServer ();

XServerPtr  servers[MAX_SERVERS];

extern char *display;

main (argc, argv)
    int	    argc;
    char    **argv;
{
    int	    i;

    display = "1";
    ProcessCommandLine (argc, argv);
    AdjustProcVector();
    while (1)
    {
	serverGeneration++;
	OsInit ();
	if (serverGeneration == 1)
	{
	    CreateWellKnownSockets ();
	    clients = (ClientPtr *)xalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i=1; i<MAXCLIENTS; i++) 
		clients[i] = NullClient;
	    serverClient = (ClientPtr)xalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
            serverClient->sequence = 0;
            serverClient->closeDownMode = RetainPermanent;
            serverClient->clientGone = FALSE;
            serverClient->server = servers[0];
	    serverClient->index = 0;
	    serverClient->clientAsMask = (Mask)0;
	    serverClient->noClientException = Success;
            serverClient->awaitingSetup = FALSE;

	}
	TagsInit();
        if (!InitClientResources(serverClient))
            FatalError("couldn't init server resources");
        InitDeleteFuncs();
        clients[0] = serverClient;
        currentMaxClients = 1;

	if (Dispatch () != 0)
	    break;
        FreeAllResources();
        FreeAtoms();
        FreeColors();
        FreeTags();
    }
    exit (0);
}

void
MarkClientException (client)
    ClientPtr	client;
{
}
