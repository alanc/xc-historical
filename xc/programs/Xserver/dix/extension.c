/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Header: extension.c,v 1.22 87/07/26 16:18:44 susan Exp $ */

#include "X.h"
#define NEED_REPLIES
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "gcstruct.h"
#include "scrnintstr.h"

#define EXTENSION_BASE  128
#define EXTENSION_EVENT_BASE  64
#define LAST_EVENT  128
#define LAST_ERROR 0x7fffffff

typedef struct _ProcEntry {
    char *name;
    ExtensionLookupProc proc;
} ProcEntryRec, *ProcEntryPtr;


typedef struct _ScreenProcEntry {
    int num;
    ProcEntryPtr procList;
} ScreenProcEntry;

ScreenProcEntry AuxillaryScreenProcs[MAXSCREENS];

ExtensionEntry *extensions = (ExtensionEntry *)NULL;
extern int (* ProcVector[]) ();
extern int (* SwappedProcVector[]) ();
extern int (* ReplySwapVector[256]) ();

int lastEvent = EXTENSION_EVENT_BASE;
int lastError = FirstExtensionError;
int NumExtensions = 0;

void
InitExtensions()
{
    int i;

    lastEvent = EXTENSION_EVENT_BASE;
    lastError = FirstExtensionError;
    NumExtensions = 0;

    for (i=0; i<MAXSCREENS; i++)
        AuxillaryScreenProcs[i].num = 0;
}


ExtensionEntry *AddExtension(name, NumEvents, NumErrors, MainProc, 
			      SwappedMainProc)
    char *name;
    int NumEvents;
    int NumErrors;
    int (* MainProc)();
    int (* SwappedMainProc)();
{
    int i;

    if (! (MainProc || SwappedMainProc))
        return((ExtensionEntry *) NULL);
    if ((lastEvent + NumEvents > LAST_EVENT) || 
	        (unsigned)(lastError + NumErrors > LAST_ERROR))
        return((ExtensionEntry *) NULL);

    i = NumExtensions;
    NumExtensions += 1;
    extensions = (ExtensionEntry *) Xrealloc(extensions,
			      NumExtensions * sizeof(ExtensionEntry));
    extensions[i].name = (char *)Xalloc(strlen(name));
    bcopy(name, extensions[i].name , strlen(name));
    extensions[i].index = i;
    extensions[i].base = i + EXTENSION_BASE;
    ProcVector[i + EXTENSION_BASE] = MainProc;
    SwappedProcVector[i + EXTENSION_BASE] = SwappedMainProc;
    if (NumEvents)
    {
        extensions[i].eventBase = lastEvent;
	extensions[i].eventLast = lastEvent + NumEvents;
	lastEvent += NumEvents;
    }
    else
    {
        extensions[i].eventBase = 0;
        extensions[i].eventLast = 0;
    }
    if (NumErrors)
    {
        extensions[i].errorBase = lastError;
	extensions[i].errorLast = lastError + NumErrors;
	lastError += NumErrors;
    }
    else
    {
        extensions[i].errorBase = 0;
        extensions[i].errorLast = 0;
    }
    return(&extensions[i]);
}


int
ProcQueryExtension(client)
    ClientPtr client;
{
    xQueryExtensionReply reply;
    int i;
    REQUEST(xQueryExtensionReq);

    REQUEST_AT_LEAST_SIZE(xQueryExtensionReq);
    
    reply.type = X_Reply;
    reply.length = 0;
    reply.major_opcode = 0;
    reply.sequenceNumber = client->sequence;

    if ( ! NumExtensions )
        reply.present = xFalse;
    else
    {
        for (i=0; i<NumExtensions; i++)
	{
            if (! strncmp(&stuff[1], extensions[i].name, stuff->nbytes))
                break;
	}
        if (i == NumExtensions)
            reply.present = xFalse;
        else
        {            
            reply.present = xTrue;
	    reply.major_opcode = extensions[i].base;
	    reply.first_event = extensions[i].eventBase;
	    reply.first_error = extensions[i].errorBase;
	}
    }
    WriteReplyToClient(client, sizeof(xQueryExtensionReply), &reply);
    return(client->noClientException);
}

int
ProcListExtensions(client)
    ClientPtr client;
{
    xListExtensionsReply reply;
    char *bufptr, *buffer;

    REQUEST(xReq);
    REQUEST_SIZE_MATCH(xReq);

    reply.type = X_Reply;
    reply.nExtensions = NumExtensions;
    reply.length = 0;

    if ( NumExtensions )
    {
        register int i;	
        int len = 0;

        for (i=0;  i<NumExtensions; i++)
	    len += strlen(extensions[i].name) + 1;
        reply.length = (len + 3) >> 2;
	buffer = bufptr = (char *)ALLOCATE_LOCAL(len);
        for (i=0;  i<NumExtensions; i++)
        {
	    *bufptr++ = len = strlen(extensions[i].name);
	    bcopy(extensions[i].name, bufptr,  len);
	    bufptr += len;
	}
    }
    WriteReplyToClient(client, sizeof(xListExtensionsReply), &reply);
    if (reply.length)
    {
        WriteReplyToClient(client, reply.length << 2, buffer);
	DEALLOCATE_LOCAL(buffer);
    }
    return(client->noClientException);
}


ExtensionLookupProc 
LookupProc(name, pGC)
    char *name;
    GCPtr pGC;
{
    register int i;
    ScreenProcEntry spentry;
    spentry  = AuxillaryScreenProcs[pGC->pScreen->myNum];
    if (spentry.num)    
    {
        for (i = 0; i < spentry.num; i++)
            if (strcmp(name, spentry.procList[i].name) == 0)
                return(spentry.procList[i].proc);
    }
    return (ExtensionLookupProc)NULL;
}


void
RegisterProc(name, pGC, proc)
    char *name;
    GC *pGC;
    ExtensionLookupProc proc;
{
    ScreenProcEntry *spentry;
    ProcEntryPtr procEntry = (ProcEntryPtr)NULL;
    int i;

    spentry = &AuxillaryScreenProcs[pGC->pScreen->myNum];
    /* first replace duplicates */
    if (spentry->num)
    {
        for (i = 0; i < spentry->num; i++)
            if (strcmp(name, spentry->procList[i].name) == 0)
	    {
                procEntry = &spentry->procList[i];
		break;
	    }
    }
    if (procEntry)
        procEntry->proc = proc;
    else
    {
	if (spentry->num)
	    spentry->procList = (ProcEntryPtr)
		Xrealloc(spentry->procList,
		    sizeof(ProcEntryRec) * (spentry->num+1));
	else
	    spentry->procList = (ProcEntryPtr)
		Xalloc(sizeof(ProcEntryRec));
        procEntry = &spentry->procList[spentry->num];
        procEntry->name = (char *)Xalloc(strlen(name)+1);
        strcpy(procEntry->name, name);
        procEntry->proc = proc;
        spentry->num++;        
    }
}

    
