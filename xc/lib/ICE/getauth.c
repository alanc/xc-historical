/* $XConsortium: getauth.c,v 1.2 93/12/07 11:04:08 mor Exp $ */
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

static Bool auth_valid ();


/*
 * The IceGetPoAuthData functions is implementation dependent.
 * The SI relies on an .ICEauthority file.
 */

void
IceGetPoAuthData (protocolName, address, authName, authDataLenRet, authDataRet)

char		*protocolName;
char		*address;
char		*authName;
unsigned short	*authDataLenRet;
char		**authDataRet;

{
    IceAuthFileEntry    *entry;

    entry = IceGetAuthFileEntry (protocolName, address, authName);

    if (entry)
    {
	*authDataLenRet = entry->auth_data_length;

	if ((*authDataRet = (char *) malloc (entry->auth_data_length)) != NULL)
	    memcpy (*authDataRet, entry->auth_data, entry->auth_data_length);
    }
    else
    {
	*authDataLenRet = 0;
	*authDataRet = NULL;
    }

    IceFreeAuthFileEntry (entry);
}



void
IceGetPaAuthData (protocolName, address, authName, authDataLenRet, authDataRet)

char		*protocolName;
char		*address;
char		*authName;
unsigned short	*authDataLenRet;
char		**authDataRet;

{
    IceAuthDataEntry	*entry;
    int			found = 0;
    int			i;

    for (i = 0; i < _IcePaAuthDataEntryCount && !found; i++)
    {
	entry = &_IcePaAuthDataEntries[i];

	found =
	    strcmp (protocolName, entry->protocol_name) == 0 &&
            strcmp (address, entry->address) == 0 &&
            strcmp (authName, entry->auth_name) == 0;
    }

    if (found)
    {
	*authDataLenRet = entry->auth_data_length;

	if ((*authDataRet = (char *) malloc (entry->auth_data_length)) != NULL)
	    memcpy (*authDataRet, entry->auth_data, entry->auth_data_length);
    }
    else
    {
	*authDataLenRet = 0;
	*authDataRet = NULL;
    }
}



void
_IceGetPoValidAuthIndices (protocol_name, address,
    num_auth_names, auth_names, num_indices_ret, indices_ret)

char	*protocol_name;
char	*address;
int	num_auth_names;
char	**auth_names;
int	*num_indices_ret;
int	*indices_ret;		/* in/out arg */

{
    FILE    		*auth_file;
    char    		*filename;
    IceAuthFileEntry    *entry;
    int			index_ret, i;

    *num_indices_ret = 0;

    if (!(filename = IceAuthFileName ()))
	return;

    if (access (filename, R_OK) != 0)		/* checks REAL id */
	return;

    if (!(auth_file = fopen (filename, "rb")))
	return;

    for (;;)
    {
	if (!(entry = IceReadAuthFileEntry (auth_file)))
	    break;

	if (strcmp (protocol_name, entry->protocol_name) == 0 &&
	    strcmp (address, entry->address) == 0 &&
	    auth_valid (entry->auth_name, num_auth_names,
	    auth_names, &index_ret))
	{
	    /*
	     * Make sure we didn't store this index already.
	     */

	    for (i = 0; i < *num_indices_ret; i++)
		if (index_ret == indices_ret[i])
		    break;

	    if (i >= *num_indices_ret)
	    {
		indices_ret[*num_indices_ret] = index_ret;
		*num_indices_ret += 1;
	    }
	}

	IceFreeAuthFileEntry (entry);
    }

    fclose (auth_file);
}



void
_IceGetPaValidAuthIndices (protocol_name, address,
    num_auth_names, auth_names, num_indices_ret, indices_ret)

char	*protocol_name;
char	*address;
int	num_auth_names;
char	**auth_names;
int	*num_indices_ret;
int	*indices_ret;		/* in/out arg */

{
    int			index_ret;
    int			i, j;
    IceAuthDataEntry	*entry;

    *num_indices_ret = 0;

    for (i = 0;	i < _IcePaAuthDataEntryCount; i++)
    {
	entry = &_IcePaAuthDataEntries[i];

	if (strcmp (protocol_name, entry->protocol_name) == 0 &&
            strcmp (address, entry->address) == 0 &&
	    auth_valid (entry->auth_name, num_auth_names,
	    auth_names, &index_ret))
	{
	    /*
	     * Make sure we didn't store this index already.
	     */

	    for (j = 0; j < *num_indices_ret; j++)
		if (index_ret == indices_ret[j])
		    break;

	    if (j >= *num_indices_ret)
	    {
		indices_ret[*num_indices_ret] = index_ret;
		*num_indices_ret += 1;
	    }
	}
    }
}



/*
 * local routines
 */

static Bool
auth_valid (auth_name, num_auth_names, auth_names, index_ret)

char	*auth_name;
int	num_auth_names;
char	**auth_names;
int	*index_ret;

{
    /*
     * Check if auth_name is in auth_names.  Return index.
     */

    int i;

    for (i = 0; i < num_auth_names; i++)
	if (strcmp (auth_name, auth_names[i]) == 0)
	{
	    break;
	}
   
    if (i < num_auth_names)
    {
	*index_ret = i;
	return (1);
    }
    else
	return (0);
}
