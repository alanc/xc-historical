/* $XConsortium$ */
/***************************************************************************
 * Copyright 1994 Network Computing Devices;
 * Portions Copyright 1988 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 **************************************************************************/

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#include "misc.h" 
#include "os.h" 
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#define _XRECORD_SERVER_
#include "record.h"
#include "recordstr.h" 

/*
 * Local Global Variables
 */
static int      RecordReqCode;
static int      RecordEventBase;
static int      RecordErrorBase;

static RESTYPE   RTConfig = 0; 
static RESTYPE   XRecordClass;    /* Resource class for this extension */
static RESTYPE   XRecordType;     /* Resource type for this extension */
static RESTYPE   XRecordDelete;   /* Resource type for intercepted clients */

#define VERIFY_RCONFIG(id, config, client) \
{ \
    config = (RecordConfig *)LookupIDByType(id, RTConfig); \
    if (!config) \
    { \
     		ErrorF("%s:  BadConfiguration Error\n", XRecordExtName); \
		client->errorValue = id; \
		return RecordErrorBase + XRecordBadConfiguration; \
    } \
}

#define VERIFY_FLAGS(flags) \
{ \
        if( (flags.events.first > XRecordMaxEvent) || \
	    (flags.events.last  > XRecordMaxEvent) || \
            (flags.errors.first > XRecordMaxError) || \
            (flags.errors.last  > XRecordMaxError) || \
            (flags.core_requests.first > XRecordMaxCoreRequest) || \
            (flags.core_requests.last > XRecordMaxCoreRequest)  || \
            (flags.core_replies.first > XRecordMaxCoreRequest) || \
            (flags.core_replies.last > XRecordMaxCoreRequest)  || \
            (flags.ext_requests.ext_major > XRecordMaxExtRequest)  || \
            (flags.ext_replies.ext_major > XRecordMaxExtRequest)   || \
            (flags.ext_requests.ext_major != 0 && \
	    flags.ext_requests.ext_major < XRecordMinExtRequest)  || \
            (flags.ext_replies.ext_major != 0 && \
	    flags.ext_replies.ext_major < XRecordMinExtRequest) ) \
            	return BadValue; \
}   


typedef struct
{
  	ClientPtr            client; 
} XRecordEnv;

/*----------------------------*
 *  Global Data Declarations  *
 *----------------------------*/
globaldef int_function XRecordEventProcVector[128L] = {NULL};
globaldef int_function EventProcVector[128L] = {NULL};
globaldef int_function XRecordProcVector[256L] = {XRecordRequestVector};
static INT16    VectoredEvents[128L]  = {0L};

extern int	currentMaxClients; 
extern int (* ProcVector[256L]) ();  
extern int (* InitialVector[3]) ();
extern void_function EventSwapVector[128L]; 

static XRecordEnv *XTenv[MAXCLIENTS]         = {NULL};  
       
typedef struct _RecordInterceptClientList { 
    XID		intercept_id;
    struct _RecordInterceptClientList *next;
} RecordInterceptClientList;

typedef struct 	_RecordConfig
{
    ClientPtr		control_client;	/* Control connection */
    ClientPtr		data_client;  	/* Data connection */
    XRecordConfig      	id;		/* config XID */
    XRecordFlags        flags;          /* Protocol to be intercepted */
    BOOL		enabled; 	/* Is configuration enabled */ 
    RecordInterceptClientList   *pInterceptClients; /* Clients to intercept */
} RecordConfig, *RecordConfigPtr;
   
static int		RecordNumConfigs = 0; 
static RecordConfig 	**RecordConfigList = NULL; 

/*----------------------------*
 *  Forward Declarations
 *----------------------------*/
static void 	RecordSwapClientProto(); 
static int 	FreeConfig();
static int 	FreeDelete();
static void 	_SwapProc ();
void 		sReplyRecordGetCur();
void 		sReplyRecordQueryVersion(); 

/*
* ** Procedures to support config objects  
*/
static int
FreeConfig(env, id)
    pointer         env;
    XID             id;
{
     RecordConfig 	*pConfig = (RecordConfig *)env;    

     if (RecordConfigList)     
     {
	int i, found = 0;

        for (i = 0; i < RecordNumConfigs; i++)
        {
            if (RecordConfigList[i] == pConfig)
            {   
		RecordInterceptClientList *pCur, *pPrev; 
		found = i;  

    		for (pPrev = NULL, pCur = pConfig->pInterceptClients;
	 	    pCur;
	 	    pPrev = pCur, pCur = pCur->next)
    		{	 
		    RecordSwapClientProto(pCur->intercept_id, pConfig->flags, xTrue);    
                    /* FreeResource(pCur->intercept_id, RT_NONE); */                                  
	    	    if (pPrev)
		    	pPrev->next = pCur->next;
	            else
		    	pConfig->pInterceptClients = pCur->next;
	            xfree(pCur);                       
                }
                break; 
    	   }                                                             
        }
        if (found <= RecordNumConfigs && RecordConfigList[i] == pConfig)
        {
            for (i = found; i <= RecordNumConfigs; i++)
            {
                RecordConfigList[i] = RecordConfigList[i+1];
            }        
      	    if(RecordNumConfigs) 
            {
		RecordNumConfigs--; 
#ifdef VERBOSE

    		ErrorF("%s:  Free config: 0x%1x\n",
          		XRecordExtName, pConfig);
#endif   
            }
	}     
        while(pConfig->pInterceptClients)
	    FreeResource(pConfig->pInterceptClients->intercept_id, RT_NONE); 
    }  
    xfree(pConfig); 
    return Success;
}

/* MZ: 
Protocol specification currently undefined wrt what happens when 
intercepted client goes away before recording client(s).  In current
implementation: 
                - protocol is swapped back
                - config becomes disabled 
*/ 

static int
FreeInterceptedClient(value, id)
    pointer value; /* must conform to DeleteType */
    XID   id;
{
    RecordConfig       		*pConfig = (RecordConfig *)value;
    RecordInterceptClientList 	*pCur, *pPrev = NULL; 

    for (pCur = pConfig->pInterceptClients; pCur; pCur = pCur->next)
    {
	if (pCur->intercept_id == id)
	{  
	    RecordSwapClientProto(pCur->intercept_id, 
		pConfig->flags, xTrue);
            FreeResource(pCur->intercept_id, RT_NONE);  
            pConfig->enabled = xFalse;                       
	    if (pPrev)
		pPrev->next = pCur->next;
	    else
		pConfig->pInterceptClients = pCur->next;
	    xfree(pCur);
	    break; 
	}
    }
    return (Success); 
}

/*
* ** Procedures to support protocol requests  
*/ 
static int 
XRecordCreateEnv(client)
    ClientPtr client;
{
    XRecordEnv 	*penv;
    int 	status = Success;
    CARD32	i = 0L;

    if (client->index > MAXCLIENTS)
    {
        status = BadImplementation;
    }
    else if ((XTenv[client->index] = (XRecordEnv *)xalloc(sizeof(XRecordEnv)))
        == NULL)
    {
        status = BadAlloc;
    }
    if (status == Success)
    {
        penv = XTenv[client->index];
        penv->client = client;
        AddResource(FakeClientID(client->index), XRecordType,
            (pointer)(client->index) );  
#ifdef VERBOSE
        ErrorF("%s:  Client '%d' Connection Accepted\n", XRecordExtName, 
            penv->client->index);
#endif
    }
    return(status);
}

static int 
XRecordDestroyEnv(value, id)
    pointer value;  /* pointer to client->index */
    XID id;         /* resource id who's deletion caused the callback */
{
    XRecordEnv 	*penv = XTenv[(int)value];

#ifdef VERBOSE
    ErrorF("%s:  Client '%d' Disconnected\n", XRecordExtName, 
        penv->client->index);
#endif
    xfree(penv);
    XTenv[(int)value] = NULL; 
    return;
}
                       
static int 
XRecordCloseDown(extEntry)
    ExtensionEntry *extEntry;
{                                           
    int i;

    for (i=0L; i<MAXCLIENTS; i++)
    {
        if (XTenv[i] != NULL)
        {
            XRecordDestroyEnv((pointer)i,0L);
        }
    }
    xfree(RecordConfigList);
    RecordConfigList = NULL; 
    RecordNumConfigs = 0; 
    RTConfig = 0; 
    return;
}

static Bool
RecordSelectForIntercept(pConfig, client_id, addtolist)
    RecordConfig *pConfig;
    XID client_id;
    Bool addtolist; 
{
    RecordInterceptClientList *pClients,  *pCur, *pPrev = NULL; 
 
    if(addtolist)
    { 
	for (pClients = pConfig->pInterceptClients; 
	    pClients; pClients = pClients->next)
        {
	    if (pClients->intercept_id == client_id) /*already there */
	    return Success; 
        } 
        /* add new client to pConfig->pInterceptClients */
        pClients = (RecordInterceptClientList *) xalloc(sizeof(RecordInterceptClientList));
        if (!pClients)
	    return BadAlloc;    	
        pClients->intercept_id = client_id;
        if (!AddResource(pClients->intercept_id, XRecordDelete, pConfig))
        {
	    xfree(pClients);
	    return BadAlloc;
    	}
        pClients->next = pConfig->pInterceptClients;
        pConfig->pInterceptClients = pClients;
        return Success;
   } 
   else 
   {
   	for (pCur = pConfig->pInterceptClients; pCur; pCur = pCur->next)
        {
	    if (pCur->intercept_id == client_id) 			
	    {
		if(pPrev)
		    pPrev->next = pCur->next;
		else 
		    pConfig->pInterceptClients = pCur->next;
		RecordSwapClientProto(pCur->intercept_id, 
		pConfig->flags, xTrue);
		FreeResource(pCur->intercept_id, RT_NONE);  
		xfree(pCur);  
                break;                       
            }
        }
        return Success;        
   }
}

static RecordConfig * 
XRecordCreateConfig(client, id, flags)
    ClientPtr		client;
    XRecordConfig 	id;
    XRecordFlags	*flags; 
{
    RecordConfig	*pConfig;
  
    if((pConfig = (RecordConfig*)Xcalloc(sizeof(RecordConfig))) == NULL) 
	return (RecordConfig *)NULL; 

    if (!AddResource(id, RTConfig, (pointer) pConfig)) 
    {
	xfree((pointer)pConfig);
        ErrorF("%s:  Cannot add resource '0x%lx'\n", 
	XRecordExtName, id);
	return (RecordConfig *)NULL;
    } 
    pConfig->id = id;
    pConfig->pInterceptClients = NULL;   
    pConfig->data_client = (ClientPtr)NULL;
    pConfig->control_client = (ClientPtr)NULL;
    pConfig->enabled = xFalse; 
    (void)memcpy(&(pConfig->flags), flags, SIZEOF(XRecordFlags) ); 

    RecordConfigList[RecordNumConfigs++] = pConfig;  
	 
#ifdef VERBOSE 
    ErrorF("%s:  Configuration '0x%lx' Created\n", XRecordExtName, id);
#endif
    return pConfig;   
}

static void 
RecordSwapClientProto(id, flags, delete) 
    XID          id;
    XRecordFlags *flags; 
    BOOL	delete; 
{
    register int i, j = 0L;  

    for(i = currentMaxClients; --i > 0;)
    { 
        if( clients[i] && clients[i]->requestVector != InitialVector &&
            id == clients[i]->clientAsMask ) 
	{
#ifdef VERBOSE               
	    ErrorF("%s:  Client [%d]  Mask 0x%lx\n", XRecordExtName,
            i, clients[i]->clientAsMask);  
#endif
   	    for(j=0L; j<XRecordMaxExtRequest; j++)
            {     
		if( (j >= flags->core_requests.first && 
		  	j <= flags->core_requests.last) || 
                     	(j == flags->ext_requests.ext_major) )
		{ 			                       	
                     _SwapProc( &(XRecordProcVector[j]),
		     &(clients[i]->requestVector[j]) ); 
		}                
            }           
       	    for(j=0L; j<XRecordMaxEvent; j++)
            { 
		if(flags->events.last) 
                {
		    if(j >= flags->events.first && j <= flags->events.last) 
                    {  
			if(!delete)
			{ 
                       	    if (++(VectoredEvents[j]) == 1L) 
 				_SwapProc(&(XRecordEventProcVector[j]), 
                            	&(EventProcVector[j])); 
			    else
                                ++(VectoredEvents[j]); 
			} 
			else			    
			{                           
			    if (VectoredEvents[j] == 1L)	
				_SwapProc(&(XRecordEventProcVector[j]), 
				&(EventProcVector[j]));
			    else if(--(VectoredEvents[j]) == 1L)
				_SwapProc(&(XRecordEventProcVector[j]), 
				&(EventProcVector[j]));
			    else
				--(VectoredEvents[j]);
			}
		    }                  

	        }                  
    	    } 
        }      
    }
} 

int XRecordRequestVector(client)
    ClientPtr client;
{
    REQUEST(xResourceReq);
    register int	i;

    for(i = 0; i < RecordNumConfigs; i++)
    {
    	if(RecordConfigList[i]->enabled)                   
        {  
    	    RecordInterceptClientList 	*pCur; 

    	    for (pCur = RecordConfigList[i]->pInterceptClients;
	 	pCur;  pCur = pCur->next)
    	    {
		if (pCur->intercept_id == clients[client->index]->clientAsMask)
	        {   	
		    xRecordEnableConfigReply 	rep;

	  	    rep.hdr.type        	= X_Reply;
    	            rep.hdr.detail 		= X_RecordEnableConfig;
    	            rep.hdr.sequenceNumber  	= RecordConfigList[i]->data_client->sequence; 
                    rep.hdr.length      	= (sizeof(xRecordEnableConfigReply) - sizeof(xGenericReply)) >> 2;
                    rep.id_base			= pCur->intercept_id;  
                    rep.nReplies		= client->index; /*Arbitrary number */
                    rep.client_swapped 		= client->swapped;
                    rep.client_seq     		= client->sequence; 
               	    rep.direction		= FromClient; 
	
               	    (void)memcpy(&(rep.data.u.req), stuff, stuff->length*sizeof(CARD32)); 
#ifdef VERBOSE
     		    ErrorF("%s:  Client: 0x%lx Config: 0x%lx XRecordRequestVector[%d] (%d - %d)\n", 
		    XRecordExtName, 
		    pCur->intercept_id, 
		    RecordConfigList[i]->id, 
                    stuff->reqType,
                    RecordConfigList[i]->flags.core_requests.first, 
		    RecordConfigList[i]->flags.core_requests.last); 
#endif
   	            WriteToClient(RecordConfigList[i]->data_client, 
			sizeof(xRecordEnableConfigReply), (char *)&rep); 

		    break; 
             	} 
            }
        } 	  
    } 
    return((*XRecordProcVector[stuff->reqType])(client)); 
}

int XRecordEventVector(client, x_event)
    ClientPtr client;
    xEvent    *x_event;
{  
    register int	i;

    for(i = 0; i < RecordNumConfigs; i++)
    {
    	if(RecordConfigList[i]->enabled)                   
        {  
    		
    	    RecordInterceptClientList 	*pCur; 

    	    for (pCur = RecordConfigList[i]->pInterceptClients;
	 	pCur;
	 	pCur = pCur->next)
    	    {
		if (pCur->intercept_id == clients[client->index]->clientAsMask &&
		x_event->u.u.type >= RecordConfigList[i]->flags.events.first &&
		x_event->u.u.type <= RecordConfigList[i]->flags.events.last) 
	        {  	              
		    xRecordEnableConfigReply 	rep;

	     	    if(x_event->u.u.type == X_Error && 
			RecordConfigList[i]->flags.errors.first == 0 && 
			RecordConfigList[i]->flags.errors.last == 0)
                        return; 

	  	    rep.hdr.type        	= X_Reply;
    	            rep.hdr.detail 		= X_RecordEnableConfig;
    	            rep.hdr.sequenceNumber  	= RecordConfigList[i]->data_client->sequence; 
                    rep.hdr.length      	= (sizeof(xRecordEnableConfigReply) - sizeof(xGenericReply)) >> 2;
                    rep.nReplies		= client->index; /*Arbitrary number */
                    rep.id_base			= pCur->intercept_id; 
               	    rep.client_swapped		= client->swapped;
                    rep.client_seq          	= client->sequence; 
                    rep.direction		= FromServer; 

                    (void)memcpy(&(rep.data.u.event), x_event, sizeof(xEvent)); 
#ifdef VERBOSE
   		    ErrorF("%s:  Client: 0x%lx Config: 0x%lx XRecordEventVector[%d]  (%d - %d)\n", 
		    	XRecordExtName, 
			pCur->intercept_id, 
			RecordConfigList[i]->id, 
			x_event->u.u.type, 
			RecordConfigList[i]->flags.events.first, RecordConfigList[i]->flags.events.last);  
#endif
   	            WriteToClient(RecordConfigList[i]->data_client, 
			sizeof(xRecordEnableConfigReply), (char *)&rep); 
		    break; 
                }
	     } 
         }
    }   
    return;
}

/*
* ** Initialize the Record extension  
*/                       
void XRecordExtensionInit()
{
    register ExtensionEntry 	*extEntry;
    unsigned int 		i;
    Atom			a;   
   
    RTConfig = CreateNewResourceType(FreeConfig)|RC_NEVERRETAIN;

    if(RTConfig == 0 || (extEntry = AddExtension(XRecordExtName,
              	XRecordNumEvents, XRecordNumErrors,
               	ProcRecordDispatch, sProcRecordDispatch, 
                XRecordCloseDown, StandardMinorOpcode)) == NULL) 
    {                                       
        ErrorF("%s:  Extension failed to Initialise.\n", 
		XRecordExtName);  
        return;
    }
#ifdef VERBOSE
    ErrorF("%s:  AddExtension assigned Major Opcode '%d'\n",
           XRecordExtName, extEntry->base);
#endif 
    RecordReqCode = extEntry->base;
    RecordErrorBase   = extEntry->errorBase;
    RecordEventBase   = extEntry->eventBase;
 
    RecordConfigList = (RecordConfig **)xrealloc( 
	RecordConfigList, (RecordNumConfigs+1) * sizeof(RecordConfig *) ); 
    if(!RecordConfigList)
    {
        ErrorF("%s:  Setup cannot create RecordConfigList\n",
          XRecordExtName);
	return; 
    }
    if ((a = MakeAtom(XRecordExtName,strlen(XRecordExtName),1L)) == None ||
        (XRecordType  = CreateNewResourceType(XRecordDestroyEnv)) == 0L ||
        (XRecordDelete  = CreateNewResourceType(FreeInterceptedClient)|RC_NEVERRETAIN) == 0L )
    {
        ErrorF("%s:  Setup can't create new resource types (%d, %d, %d, %d)\n",
          XRecordExtName, a, XRecordClass, XRecordType, XRecordDelete);
        return;
    }
#ifdef VERBOSE
        ErrorF("%s:  Max Events (%d)  Max Requests (%d)\n",
          XRecordExtName, XRecordMaxEvent, XRecordMaxExtRequest);
#endif 

    for (i=0L; i<=XRecordMaxEvent; i++)
    {
        VectoredEvents[i] = 0L; 
        EventProcVector[i] = NULL;
        XRecordEventProcVector[i] = XRecordEventVector;
    }

    /* MZ: X_Error == event[0] */
    EventProcVector[X_Error] = NULL;
    XRecordEventProcVector[X_Error] = XRecordEventVector;

    for (i=0L; i<XRecordMaxExtRequest; i++)
    {
        XRecordProcVector[i] = XRecordRequestVector;
    }
#ifdef VERBOSE
    ErrorF("%s:  Protocol version: %d.%d successfully loaded\n", 
	XRecordExtName, XRecordMajorVersion, XRecordMinorVersion); 
#endif 

    return;
}

/*
* ** Protocol dispatch procedure   
*/
static int ProcRecordDispatch(client)
    ClientPtr client;
{
    REQUEST(xReq);
    register int status = Success;

    if (XTenv[client->index] == NULL)
        status = XRecordCreateEnv(client);
    if (status == Success)
    {
        switch (stuff->data)
    	{
      	    case X_RecordQueryVersion:
		return ProcRecordQueryVersion(client);
      	    case X_RecordCreateConfig:
		return ProcRecordCreateConfig(client);
      	    case X_RecordFreeConfig:
		return ProcRecordFreeConfig(client);
      	    case X_RecordChangeConfig:
		return ProcRecordChangeConfig(client);
      	    case X_RecordGetConfig:
		return ProcRecordGetConfig(client);
      	    case X_RecordEnableConfig:
		return ProcRecordEnableConfig(client);
           default:
#ifdef VERBOSE
            ErrorF("%s:  Invalid Request.  Minor opcode=%d\n",
                   XRecordExtName, stuff->data);
#endif 
		return BadRequest;
        } 
    }
    return (client->noClientException);
}

/*
* ** Protocol requests    
*/

/*
* ** Query extension 
*/
int ProcRecordQueryVersion(client)
    ClientPtr client;
{
    REQUEST(xRecordQueryVersionReq);
    xRecordQueryVersionReply 	rep; 
    int 		n;  

    REQUEST_SIZE_MATCH(xRecordQueryVersionReq);
    rep.hdr.type        	= X_Reply;
    rep.hdr.detail 	  	= X_RecordQueryVersion;
    rep.hdr.sequenceNumber  	= client->sequence; 
    rep.hdr.length          	= 0;

    /* if requested version is higher than the version of this extension */
    /* use this extension version */
    if ((stuff->majorVersion > XRecordMajorVersion) ||
        ((stuff->majorVersion == XRecordMajorVersion) &&
          (stuff->minorVersion >= XRecordMinorVersion)))
    {
        rep.majorVersion  	= XRecordMajorVersion;
        rep.minorVersion  	= XRecordMinorVersion;
    }
    /* if requested version is lower than supported by the extension */
    /* use the lowest version supported by this extension */
    else if ((stuff->majorVersion < XRecordLowestMajorVersion) ||
        ((stuff->majorVersion == XRecordLowestMajorVersion) &&
          (stuff->minorVersion <= XRecordLowestMinorVersion)))
    {
        rep.majorVersion = XRecordLowestMajorVersion;
        rep.minorVersion = XRecordLowestMinorVersion;
    }
    else
    {
        rep.majorVersion = stuff->majorVersion;
        rep.minorVersion = stuff->minorVersion;
    }

    if(client->swapped) {
    	swaps(&rep.hdr.sequenceNumber, n);
    	swapl(&rep.hdr.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    } 

    WriteToClient(client, sizeof(xRecordQueryVersionReply), (char *)&rep);

    return(client->noClientException);
}

/*
* ** Create config  
*/
static int
ProcRecordCreateConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordCreateConfigReq);
    RecordConfigPtr pConfig; 

    REQUEST_SIZE_MATCH(xRecordCreateConfigReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client); 
    VERIFY_FLAGS(stuff->record_flags); 
    if (!XRecordCreateConfig(client, stuff->cid, &stuff->record_flags))
	return BadAlloc;

    return (client->noClientException);
}

/*
* ** Destroy a config object 
*/	
static int
ProcRecordFreeConfig(client)
    ClientPtr       client;
{
    RecordConfigPtr    pConfig;
    REQUEST(xRecordFreeConfigReq);
  
    REQUEST_SIZE_MATCH(xRecordFreeConfigReq);
    VERIFY_RCONFIG(stuff->cid, pConfig, client); 

    pConfig->enabled = xFalse; 
    FreeResource(pConfig->id, RT_NONE); 
#ifdef VERBOSE
    ErrorF("%s:  Free resource '0x%lx'\n", 
	XRecordExtName, stuff->cid); 
#endif  
 
    return(client->noClientException);
}

/*
* ** Change a config object 
*/	
static int 
ProcRecordChangeConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordChangeConfigReq);
    RecordConfigPtr 	pConfig;
    pointer 		*pResource;
    int 		clientIndex, status; 

    REQUEST_SIZE_MATCH(xRecordChangeConfigReq);
    VERIFY_RCONFIG(stuff->cid, pConfig, client); 
    VERIFY_FLAGS(stuff->record_flags);

    pResource = (pointer *)LookupIDByClass(stuff->id_base, RC_ANY);
    clientIndex = CLIENT_ID(stuff->id_base);   
    if(pResource == NULL || clientIndex == NULL)
    {
	client->errorValue = stuff->id_base;
        return BadMatch;
    }
    if(client->clientAsMask == stuff->id_base)
    {
	client->errorValue = stuff->id_base;
        return BadMatch;
    } 
    (void)memcpy(&(pConfig->flags), &stuff->record_flags, SIZEOF(XRecordFlags) );
    pConfig->control_client = client; 

    /* Create delete resource for intercept_id */
    status = RecordSelectForIntercept(pConfig, stuff->id_base, stuff->add); 
    if (status != Success)
		return status;

    RecordSwapClientProto(stuff->id_base, &stuff->record_flags, xFalse); 

    return(client->noClientException);
}
 
/*
* ** Get a config object 
*/
static int 
ProcRecordGetConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordGetConfigReq); 
    RecordConfigPtr 		pConfig;
    pointer 			*pResource;
    ClientPtr 			recordclient;
    xRecordGetConfigReply 	rep;
    register int		i; 
    int 			n;  
     
    REQUEST_SIZE_MATCH(xRecordGetConfigReq);
    VERIFY_RCONFIG(stuff->cid, pConfig, client); 
                 	
    for (i = 0; i < RecordNumConfigs; i++)
    {
        if (RecordConfigList[i] == pConfig && pConfig->control_client == NULL)
	return BadMatch;                       
    }
       
    rep.hdr.type        	= X_Reply;
    rep.hdr.detail 	  	= X_RecordGetConfig;
    rep.hdr.sequenceNumber  	= client->sequence; 
    rep.hdr.length      	= (sizeof(xRecordGetConfigReply) - sizeof(xGenericReply)) >> 2;
    rep.record_state.enabled = pConfig->enabled;
 
    memset(&(rep.record_state.intercepted), 0L, sizeof(XRecordFlags));
    (void)memcpy(&rep.record_state.intercepted, &pConfig->flags, sizeof(XRecordFlags) ); 
#ifdef VERBOSE
    ErrorF("%s:  Config '0x%lx' intercept: '0x%lx' Events: %d %d  Errors: %d %d  Core: %d %d %d %d\n", 
	XRecordExtName, stuff->cid, stuff->id_base, 
	pConfig->flags.events.first, pConfig->flags.events.last,
	pConfig->flags.errors.first, pConfig->flags.errors.last,
	pConfig->flags.core_requests.first, 
	pConfig->flags.core_requests.last, 
	pConfig->flags.core_replies.first, 
	pConfig->flags.core_replies.first);
#endif 

    if(client->swapped)
    {
    	swaps(&rep.hdr.sequenceNumber, n);
    	swapl(&rep.hdr.length, n);
        swaps(&rep.record_state.intercepted.ext_requests.ext_minor.first, n);
        swaps(&rep.record_state.intercepted.ext_requests.ext_minor.last,  n);
        swaps(&rep.record_state.intercepted.ext_replies.ext_minor.first,  n);
        swaps(&rep.record_state.intercepted.ext_replies.ext_minor.last,   n);
    }
       
    WriteToClient(client, sizeof(xRecordGetConfigReply), (char *)&rep);

    return(client->noClientException);
}

/*
* ** Enable  a config object 
*/
static int 
ProcRecordEnableConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordEnableConfigReq);
    RecordConfigPtr 		pConfig; 
    xRecordEnableConfigReply 	rep;
    register int		i; 
    int 			n;  
   
    REQUEST_SIZE_MATCH(xRecordEnableConfigReq);
    VERIFY_RCONFIG(stuff->cid, pConfig, client); 
    for (i = 0; i < RecordNumConfigs; i++)
    {          
        if (RecordConfigList[i] == pConfig && pConfig->control_client == NULL)
	    return BadMatch;                       
    }
    pConfig->data_client 	= client;
    pConfig->control_client 	= client;
    pConfig->enabled 		= stuff->enable;
#ifdef VERBOSE      
    ErrorF("%s:  Enable config '0x%lx': %d\n", 
	XRecordExtName, stuff->cid, stuff->enable);
#endif 
    rep.hdr.type        = X_Reply;
    rep.hdr.detail 	= X_RecordEnableConfig;
    rep.hdr.sequenceNumber  = client->sequence; 
    rep.hdr.length 	= (sizeof(xRecordEnableConfigReply) - sizeof(xGenericReply)) >> 2;

    /* Arbitrary number -- need to do something smarter */ 
    if(stuff->enable)
        rep.nReplies	= client->index; 
    else
        rep.nReplies	= 0; 
    rep.client_swapped	= client->swapped;
    rep.client_seq      = client->sequence; 
    rep.direction	= FromServer; 
 
    (void)memcpy(&(rep.data.u.req), stuff, stuff->length*sizeof(CARD32));  

    if(client->swapped)
    {
    	swaps(&rep.hdr.sequenceNumber, n);
    	swapl(&rep.hdr.length, n);
        swapl(&rep.nReplies, n);
        swapl(&rep.id_base, n);
        swapl(&rep.client_seq, n);
    }

    WriteToClient(client, sizeof(xRecordEnableConfigReply), (char *)&rep); 
          	
    return(client->noClientException);               
}

/*
* ** Procedures to support swapping   
*/

static void _SwapProc(f1,f2)
    register int (**f1)(), (**f2)();
{
    register int (*t1)() = *f1;
    *f1 = *f2;
    *f2 = t1;

    return;
}

static int 
sProcRecordDispatch(client)
    ClientPtr client;
{
    REQUEST(xReq);
    register int status = Success;

    if (XTenv[client->index] == NULL)     
        status = XRecordCreateEnv(client);    
    if (status == Success)
    {
        switch (stuff->data)
    	{
      	    case X_RecordQueryVersion:
		return sProcRecordQueryVersion(client);
      	    case X_RecordCreateConfig:
		return sProcRecordCreateConfig(client);
      	    case X_RecordFreeConfig:
		return sProcRecordFreeConfig(client);
      	    case X_RecordChangeConfig:
		return sProcRecordChangeConfig(client);
      	    case X_RecordGetConfig:
		return sProcRecordGetConfig(client);
      	    case X_RecordEnableConfig:
		return sProcRecordEnableConfig(client);
           default:
#ifdef VERBOSE
            ErrorF("%s:  Invalid Request.  Minor opcode=%d\n",
                   XRecordExtName, stuff->data);
#endif 
		return BadRequest;
        } 
    }
    return(client->noClientException);
}

static int 
sProcRecordQueryVersion(client)
    ClientPtr client;
{
    REQUEST(xRecordQueryVersionReq);
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordQueryVersionReq); 
    swaps(&stuff->majorVersion, n);
    swaps(&stuff->minorVersion,n);
    return ProcRecordQueryVersion(client); 
}

static 
int sProcRecordCreateConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordCreateConfigReq);
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordCreateConfigReq);  
    swapl(&stuff->cid, n); 
    swaps(&stuff->record_flags.ext_requests.ext_minor.first, n);
    swaps(&stuff->record_flags.ext_requests.ext_minor.last, n);
    swaps(&stuff->record_flags.ext_replies.ext_minor.first, n);
    swaps(&stuff->record_flags.ext_replies.ext_minor.last, n);
    return ProcRecordCreateConfig(client);
}
 
static int 
sProcRecordFreeConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordFreeConfigReq); 
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordFreeConfigReq);  
    swapl(&stuff->cid, n); 
    return ProcRecordFreeConfig(client);
}

static int 
sProcRecordChangeConfig(client)
    ClientPtr client;
{
    REQUEST(xRecordChangeConfigReq); 
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordChangeConfigReq); 
    swapl(&stuff->cid, n); 
    swapl(&stuff->id_base, n); 
    swaps(&stuff->record_flags.ext_requests.ext_minor.first, n);
    swaps(&stuff->record_flags.ext_requests.ext_minor.last, n);
    swaps(&stuff->record_flags.ext_replies.ext_minor.first, n);
    swaps(&stuff->record_flags.ext_replies.ext_minor.last, n);
    return ProcRecordChangeConfig(client);
}

static int 
sProcRecordGetConfig(request,client)
    ClientPtr client;
{
    REQUEST(xRecordGetConfigReq); 
    register char n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordGetConfigReq);
    swapl(&stuff->cid, n);  
    return ProcRecordGetConfig(client);
}

static int 
sProcRecordEnableConfig(request,client)
    ClientPtr client;
{
    REQUEST(xRecordEnableConfigReq); 
    register char 	n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xRecordEnableConfigReq);
    swapl(&stuff->cid, n);  
    return ProcRecordEnableConfig(client);
}

