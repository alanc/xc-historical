/* $XConsortium: register.c,v 1.1 93/08/17 18:59:02 mor Exp $ */
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
    versionCount, versionRecs, authCount, authRecs)

char			*protocolName;
char			*vendor;
char			*release;
int			versionCount;
IceOCLversionRec	*versionRecs;
int			authCount;
IceOCLauthRec		*authRecs;

{
    _IceOCLprotocol 	*p;
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
	    (_IceOCLprotocol *) malloc (sizeof (_IceOCLprotocol));

	_IceProtocols[_IceLastMajorOpcode].accept_client = NULL;

	opcodeRet = ++_IceLastMajorOpcode;
    }

    p->vendor = (char *) malloc (strlen (vendor) + 1);
    strcpy (p->vendor, vendor);

    p->release = (char *) malloc (strlen (release) + 1);
    strcpy (p->release, release);

    p->version_count = versionCount;

    p->version_recs = (IceOCLversionRec *) malloc (
	versionCount * sizeof (IceOCLversionRec));
    bcopy (versionRecs, p->version_recs,
	versionCount * sizeof (IceOCLversionRec));

    if ((p->auth_count = authCount) > 0)
    {
	p->auth_recs = (IceOCLauthRec *) malloc (
	    authCount * sizeof (IceOCLauthRec));

	for (i = 0; i < authCount; i++)
	{
	    p->auth_recs[i].auth_name =
	        (char *) malloc (strlen (authRecs[i].auth_name) + 1);
	    strcpy (p->auth_recs[i].auth_name, authRecs[i].auth_name);

	    p->auth_recs[i].auth_proc = authRecs[i].auth_proc;
	}
    }
    else
    {
	p->auth_recs = NULL;
    }

    return (opcodeRet);
}



int
IceRegisterForProtocolReply (protocolName, vendor, release,
    versionCount, versionRecs, protocolSetupNotifyCB, authCount, authRecs)

char				*protocolName;
char				*vendor;
char				*release;
int				versionCount;
IceACLversionRec		*versionRecs;
IceProtocolSetupNotifyCB	protocolSetupNotifyCB;
int				authCount;
IceACLauthRec			*authRecs;

{
    _IceACLprotocol 	*p;
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
	    (_IceACLprotocol *) malloc (sizeof (_IceACLprotocol));

	opcodeRet = ++_IceLastMajorOpcode;
    }

    p->vendor = (char *) malloc (strlen (vendor) + 1);
    strcpy (p->vendor, vendor);

    p->release = (char *) malloc (strlen (release) + 1);
    strcpy (p->release, release);

    p->version_count = versionCount;

    p->version_recs = (IceACLversionRec *) malloc (
	versionCount * sizeof (IceACLversionRec));
    bcopy (versionRecs, p->version_recs,
	versionCount * sizeof (IceACLversionRec));

    p->protocol_setup_notify_cb = protocolSetupNotifyCB;

    if ((p->auth_count = authCount) > 0)
    {
	p->auth_recs = (IceACLauthRec *) malloc (
	    authCount * sizeof (IceACLauthRec));

	for (i = 0; i < authCount; i++)
	{
	    p->auth_recs[i].auth_name =
	        (char *) malloc (strlen (authRecs[i].auth_name) + 1);
	    strcpy (p->auth_recs[i].auth_name, authRecs[i].auth_name);

	    p->auth_recs[i].auth_proc = authRecs[i].auth_proc;
	}
    }
    else
    {
	p->auth_recs = NULL;
    }

    return (opcodeRet);
}

