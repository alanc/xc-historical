/* $XConsortium: register.c,v 1.6 93/11/08 16:34:17 mor Exp $ */
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

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>

int
IceRegisterForProtocolSetup (protocolName, vendor, release,
    versionCount, versionRecs, authCount, authNames, authProcs, IOErrorProc)

char			*protocolName;
char			*vendor;
char			*release;
int			versionCount;
IcePoVersionRec		*versionRecs;
int			authCount;
char		        **authNames;
IcePoAuthProc		*authProcs;
IceIOErrorProc		IOErrorProc;

{
    _IcePoProtocol 	*p;
    int			opcodeRet, i;

    for (i = 1; i <= _IceLastMajorOpcode; i++)
	if (strcmp (protocolName, _IceProtocols[i - 1].protocol_name) == 0)
	{
	    if (_IceProtocols[i - 1].orig_client != NULL)
	    {
		/*
		 * We've already registered this protocol.
		 */
		
		return (i);
	    }
	    else
	    {
		break;
	    }
	}
	    
    if (i <= _IceLastMajorOpcode)
    {
	p = _IceProtocols[i - 1].orig_client;
	opcodeRet = i;
    }
    else if (_IceLastMajorOpcode == 255 ||
	versionCount < 1 ||
	strlen (protocolName) == 0)
    {
	return (-1);
    }
    else
    {
	char *name;

	_IceProtocols[_IceLastMajorOpcode].protocol_name = name =
	    (char *) malloc (strlen (protocolName) + 1);
	strcpy (name, protocolName);

	p = _IceProtocols[_IceLastMajorOpcode].orig_client =
	    (_IcePoProtocol *) malloc (sizeof (_IcePoProtocol));

	_IceProtocols[_IceLastMajorOpcode].accept_client = NULL;

	opcodeRet = ++_IceLastMajorOpcode;
    }

    p->vendor = (char *) malloc (strlen (vendor) + 1);
    strcpy (p->vendor, vendor);

    p->release = (char *) malloc (strlen (release) + 1);
    strcpy (p->release, release);

    p->version_count = versionCount;

    p->version_recs = (IcePoVersionRec *) malloc (
	versionCount * sizeof (IcePoVersionRec));
    memcpy (p->version_recs, versionRecs,
	versionCount * sizeof (IcePoVersionRec));

    if ((p->auth_count = authCount) > 0)
    {
	p->auth_names = (char **) malloc (
	    authCount * sizeof (char *));

	p->auth_procs = (IcePoAuthProc *) malloc (
	    authCount * sizeof (IcePoAuthProc));

	for (i = 0; i < authCount; i++)
	{
	    p->auth_names[i] =
	        (char *) malloc (strlen (authNames[i]) + 1);
	    strcpy (p->auth_names[i], authNames[i]);

	    p->auth_procs[i] = authProcs[i];
	}
    }
    else
    {
	p->auth_names = NULL;
	p->auth_procs = NULL;
    }

    p->io_error_proc = IOErrorProc;

    return (opcodeRet);
}



int
IceRegisterForProtocolReply (
    protocolName, vendor, release,
    versionCount, versionRecs,
    protocolSetupNotifyProc,
    authCount, authNames, authProcs,
    hostBasedAuthProc, hostBasedAuthProcClientData,
    IOErrorProc)

char				*protocolName;
char				*vendor;
char				*release;
int				versionCount;
IcePaVersionRec			*versionRecs;
IceProtocolSetupNotifyProc	protocolSetupNotifyProc;
int				authCount;
char				**authNames;
IcePaAuthProc			*authProcs;
IceHostBasedAuthProc		hostBasedAuthProc;
IcePointer			hostBasedAuthProcClientData;
IceIOErrorProc			IOErrorProc;

{
    _IcePaProtocol 	*p;
    int			opcodeRet, i;

    for (i = 1; i <= _IceLastMajorOpcode; i++)
	if (strcmp (protocolName, _IceProtocols[i - 1].protocol_name) == 0)
	{
	    if (_IceProtocols[i - 1].accept_client != NULL)
	    {
		/*
		 * We've already registered this protocol.
		 */
		
		return (i);
	    }
	    else
	    {
		break;
	    }
	}
	    

    if (i <= _IceLastMajorOpcode)
    {
	p = _IceProtocols[i - 1].accept_client;
	opcodeRet = i;
    }
    else if (_IceLastMajorOpcode == 255 ||
	versionCount < 1 ||
	strlen (protocolName) == 0)
    {
	return (-1);
    }
    else
    {
	char *name;

	_IceProtocols[_IceLastMajorOpcode].protocol_name = name =
	    (char *) malloc (strlen (protocolName) + 1);
	strcpy (name, protocolName);

	_IceProtocols[_IceLastMajorOpcode].orig_client = NULL;

	p = _IceProtocols[_IceLastMajorOpcode].accept_client =
	    (_IcePaProtocol *) malloc (sizeof (_IcePaProtocol));

	opcodeRet = ++_IceLastMajorOpcode;
    }

    p->vendor = (char *) malloc (strlen (vendor) + 1);
    strcpy (p->vendor, vendor);

    p->release = (char *) malloc (strlen (release) + 1);
    strcpy (p->release, release);

    p->version_count = versionCount;

    p->version_recs = (IcePaVersionRec *) malloc (
	versionCount * sizeof (IcePaVersionRec));
    memcpy (p->version_recs, versionRecs,
	versionCount * sizeof (IcePaVersionRec));

    p->protocol_setup_notify_proc = protocolSetupNotifyProc;

    if ((p->auth_count = authCount) > 0)
    {
	p->auth_names = (char **) malloc (
	    authCount * sizeof (char *));

	p->auth_procs = (IcePaAuthProc *) malloc (
	    authCount * sizeof (IcePaAuthProc));

	for (i = 0; i < authCount; i++)
	{
	    p->auth_names[i] =
	        (char *) malloc (strlen (authNames[i]) + 1);
	    strcpy (p->auth_names[i], authNames[i]);

	    p->auth_procs[i] = authProcs[i];
	}
    }
    else
    {
	p->auth_names = NULL;
	p->auth_procs = NULL;
    }

    p->host_based_auth_proc = hostBasedAuthProc;
    p->host_based_auth_proc_client_data = hostBasedAuthProcClientData;

    p->io_error_proc = IOErrorProc;

    return (opcodeRet);
}

