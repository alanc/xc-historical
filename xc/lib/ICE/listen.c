/* $XConsortium: listen.c,v 1.6 93/12/30 11:03:41 mor Exp $ */
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

Author: Ralph Mor, X Consortium
******************************************************************************/

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>
#include <X11/Xtrans.h>
#include <stdio.h>


Status
IceListenForConnections (countRet, listenObjsRet, errorLength, errorStringRet)

int		*countRet;
IceListenObj	**listenObjsRet;
int		errorLength;
char		*errorStringRet;

{
    struct _IceListenObj	*listenObjs;
    char			*networkId;
    int				fd, count, i, j;
    int				family, addrlen;
    Xtransaddr			*addr;
    Status			status = 1;
    XtransConnInfo		*trans_conns = NULL;


    if ((_ICETransMakeAllCOTSServerListeners (NULL, &count, &trans_conns) < 0)
	|| (count < 1))
    {
	*listenObjsRet = NULL;
	*countRet = 0;

        strncpy (errorStringRet,
	    "Cannot establish any listening sockets", errorLength);

	return (0);
    }

    if ((listenObjs = (struct _IceListenObj *) malloc (
	count * sizeof (struct _IceListenObj))) == NULL)
    {
	for (i = 0; i < count; i++)
	    _ICETransClose (trans_conns[i]);
	free ((char *) trans_conns);
	return (0);
    }

    *countRet = 0;

    for (i = 0; i < count; i++)
    {
	_ICETransGetMyAddr (trans_conns[i], &family, &addrlen, &addr);

	networkId = _ICETransGetMyNetworkId (family, addrlen, addr);

	free (addr);

	if (networkId)
	{
	    listenObjs[*countRet].trans_conn = trans_conns[i];
	    listenObjs[*countRet].network_id = networkId;
		
	    (*countRet)++;
	}
    }

    if (*countRet == 0)
    {
	*listenObjsRet = NULL;

        strncpy (errorStringRet,
	    "Cannot establish any listening sockets", errorLength);

	status = 0;
    }
    else
    {
	*listenObjsRet = (IceListenObj *) malloc (
	    *countRet * sizeof (IceListenObj));

	if (*listenObjsRet == NULL)
	{
	    strncpy (errorStringRet, "Malloc failed", errorLength);

	    status = 0;
	}
	else
	{
	    for (i = 0; i < *countRet; i++)
	    {
		(*listenObjsRet)[i] = (IceListenObj) malloc (
		    sizeof (struct _IceListenObj));

		if ((*listenObjsRet)[i] == NULL)
		{
		    strncpy (errorStringRet, "Malloc failed", errorLength);

		    for (j = 0; j < i; j++)
			free ((char *) (*listenObjsRet)[j]);

		    free ((char *) *listenObjsRet);

		    status = 0;
		}
		else
		{
		    *((*listenObjsRet)[i]) = listenObjs[i];
		}
	    }
	}
    }

    if (status == 1)
    {
	if (errorStringRet && errorLength > 0)
	    *errorStringRet = '\0';
	
	for (i = 0; i < *countRet; i++)
	{
	    (*listenObjsRet)[i]->host_based_auth_proc = NULL;
	}
    }
    else
    {
	for (i = 0; i < count; i++)
	    _ICETransClose (trans_conns[i]);
    }

    free ((char *) listenObjs);
    free ((char *) trans_conns);

    return (status);
}



int
IceGetListenDescrip (listenObj)

IceListenObj listenObj;

{
    return (_ICETransGetConnectionNumber (listenObj->trans_conn));
}



char *
IceGetListenNetworkId (listenObj)

IceListenObj listenObj;

{
    char *networkId;

    networkId = (char *) malloc (strlen (listenObj->network_id) + 1);

    if (networkId)
	strcpy (networkId, listenObj->network_id);

    return (networkId);
}



char *
IceComposeNetworkIdList (count, listenObjs)

int		count;
IceListenObj	*listenObjs;

{
    char *list;
    int len = 0;
    int i;

    if (count < 1 || listenObjs == NULL)
	return (NULL);

    for (i = 0; i < count; i++)
	len += (strlen (listenObjs[i]->network_id) + 1);

    list = (char *) malloc (len);

    if (list == NULL)
	return (NULL);
    else
    {
	list[0] = '\0';
	for (i = 0; i < count - 1; i++)
	{
	    strcat (list, listenObjs[i]->network_id);
	    strcat (list, ",");
	}

	strcat (list, listenObjs[count - 1]->network_id);
	list[strlen (list)] = '\0';

	return (list);
    }
}



void
IceFreeListenObj (listenObj)

IceListenObj listenObj;

{
    if (listenObj)
    {
	free (listenObj->network_id);
	free ((char *) listenObj);
    }
}



/*
 * Allow host based authentication for the ICE Connection Setup.
 * Do not confuse with the host based authentication callbacks that
 * can be set up in IceRegisterForProtocolReply.
 */

void
IceSetHostBasedAuthProc (listenObj, hostBasedAuthProc)

IceListenObj		listenObj;
IceHostBasedAuthProc	hostBasedAuthProc;

{
    listenObj->host_based_auth_proc = hostBasedAuthProc;
}
