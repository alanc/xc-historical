/* $XConsortium: setauth.c,v 1.1 93/11/24 15:44:29 mor Exp $ */
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


/*
 * The following routines are for manipulating the authentication data
 * bound to each listen object.  The reason this data is not retrieved from
 * the .ICEauthority file is because this would be a security hole.
 * The connecting client would be able to simply write an entry in the
 * file knowing that the accepting side would use it in authentication.
 *
 * NOTE: These routines are to be used by the client accepting connections
 *	 or Protocol Setups.
 */

void
IceSetAuthenticationData (listenObj, numEntries, entries)

IceListenObj		listenObj;
int			numEntries;
IceAuthDataEntry	*entries;

{
    int i;

    if (listenObj->auth_data_entries)
    {
	for (i = 0; i < listenObj->auth_data_entry_count; i++)
	{
	    free (listenObj->auth_data_entries[i].protocol_name);
	    free (listenObj->auth_data_entries[i].auth_name);
	    free (listenObj->auth_data_entries[i].auth_data);
	}

	free ((char *) listenObj->auth_data_entries);
    }

    listenObj->auth_data_entry_count = numEntries;

    listenObj->auth_data_entries = (IceAuthDataEntry *) malloc (
	numEntries * sizeof (IceAuthDataEntry));

    for (i = 0; i < numEntries; i++)
    {
	listenObj->auth_data_entries[i].protocol_name = (char *) malloc (
	    strlen (entries[i].protocol_name) + 1);
	strcpy (listenObj->auth_data_entries[i].protocol_name,
	    entries[i].protocol_name);

	listenObj->auth_data_entries[i].auth_name = (char *) malloc (
            strlen (entries[i].auth_name) + 1);
	strcpy (listenObj->auth_data_entries[i].auth_name,
	    entries[i].auth_name);

	listenObj->auth_data_entries[i].auth_data_length =
            entries[i].auth_data_length;
	listenObj->auth_data_entries[i].auth_data = (char *) malloc (
            entries[i].auth_data_length);
	memcpy (listenObj->auth_data_entries[i].auth_data,
            entries[i].auth_data, entries[i].auth_data_length);
    }
}



IceAuthDataEntry *
IceGetAuthenticationData (listenObj, protocolName, authName)

IceListenObj	listenObj;
char		*protocolName;
char		*authName;

{
    IceAuthDataEntry	*entry;
    int			found = 0;
    int			i;

    for (i = 0; i < listenObj->auth_data_entry_count && !found; i++)
    {
	entry = &listenObj->auth_data_entries[i];

	found = strcmp (protocolName, entry->protocol_name) == 0 &&
            strcmp (authName, entry->auth_name) == 0;
    }

    if (found)
	return (entry);
    else
	return (NULL);
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
