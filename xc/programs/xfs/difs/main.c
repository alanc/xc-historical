/*
 * Font server main routine
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)main.c	4.1	5/2/91
 *
 */

#include	"FS.h"
#include	"FSproto.h"
#include	"clientstr.h"
#include	"resource.h"
#include	"misc.h"
#include	"globals.h"
#include	"servermd.h"
#include	"site.h"

char       *ConnectionInfo;
int         ConnInfoLen;

extern void InitProcVectors();
extern void InitFonts();
extern void InitAtoms();
extern void InitExtensions();
extern void ProcessCmdLine();

extern int  serverNum;
char       *configfilename;

main(argc, argv)
    int         argc;
    char      **argv;
{
    int         i;

    argcGlobal = argc;
    argvGlobal = argv;

    serverNum = 0;
    configfilename = DEFAULT_CONFIG_FILE;

    /* init stuff */
    ProcessCmdLine(argc, argv);


    while (1) {
	serverGeneration++;
	OsInit();
	if (serverGeneration == 1) {
	    /* do first time init */
	    CreateSockets();
	    InitProcVectors();
	    clients = (ClientPtr *) fsalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i = MINCLIENT; i < MAXCLIENTS; i++)
		clients[i] = NullClient;
	    /* make serverClient */
	    serverClient = (ClientPtr) fsalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
	    serverClient->sequence = 0;
	    serverClient->clientGone = FALSE;
	    serverClient->index = SERVER_CLIENT;
	}
	ResetSockets();
	/* init per-cycle stuff */

	clients[SERVER_CLIENT] = serverClient;
	currentMaxClients = MINCLIENT;

	if (!InitClientResources(serverClient))
	    FatalError("couldn't init server resources");

	InitExtensions();
	InitAtoms();
	InitFonts();
	if (ReadConfigFile(configfilename) != FSSuccess)
	    FatalError("couldn't parse config file");
	if (!create_connection_block())
	    FatalError("couldn't create connection block");

#ifdef DEBUG
	fprintf(stderr, "Entering Dispatch loop\n");
#endif

	Dispatch();

	/* clean up per-cycle stuff */
	CloseDownExtensions();
	if (dispatchException & DE_TERMINATE)
	    break;
	fsfree(ConnectionInfo);
    }

    exit(0);
}

void
NotImplemented()
{
    NoopDDA();			/* dummy to get difsutils.o to link */
    FatalError("Not implemented");
}

static Bool
create_connection_block()
{
    fsConnSetupAccept setup;
    char       *pBuf;

    setup.release_number = VENDOR_RELEASE;
    setup.vendor_len = strlen(VENDOR_STRING);
    setup.max_request_len = MAX_REQUEST_SIZE;
    setup.length = (sizeof(fsConnSetupAccept) + setup.vendor_len + 3) >> 2;

    ConnInfoLen = sizeof(fsConnSetupAccept) + ((setup.vendor_len + 3) & ~3);
    ConnectionInfo = (char *) fsalloc(ConnInfoLen);
    if (!ConnectionInfo)
	return FALSE;

    bcopy((char *) &setup, ConnectionInfo, sizeof(fsConnSetup));
    pBuf = ConnectionInfo + sizeof(fsConnSetup);
    bcopy(VENDOR_STRING, pBuf, (int) setup.vendor_len);

    return TRUE;
}
