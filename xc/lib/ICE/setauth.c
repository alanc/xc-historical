/* $XConsortium: setauth.c,v 1.6 94/03/17 12:22:07 mor Exp $ */
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
#include "ICElibint.h"
#include <X11/ICE/ICEutil.h>


/*
 * IceSetPaAuthData is not a standard part of ICElib, it is specific
 * to the sample implementation.
 *
 * For the client that initiates a Protocol Setup, we look in the
 * .ICEauthority file to get authentication data.
 *
 * For the client accepting the Protocol Setup, we get the data
 * from an in-memory database of authentication data (set by the
 * application calling IceSetPaAuthData).  We have to get the data
 * from memory because getting it directly from the .ICEauthority
 * file is not secure - someone can just modify the contents of the
 * .ICEauthority file behind our back.
 */

int		 _IcePaAuthDataEntryCount = 0;
IceAuthDataEntry _IcePaAuthDataEntries[ICE_MAX_AUTH_DATA_ENTRIES];


void
IceSetPaAuthData (numEntries, entries)

int			numEntries;
IceAuthDataEntry	*entries;

{
    /*
     * _IcePaAuthDataEntries should really be a linked list.
     * On my list of TO DO stuff.
     */

    int i, j;

    for (i = 0; i < numEntries; i++)
    {
	for (j = 0; j < _IcePaAuthDataEntryCount; j++)
	    if (strcmp (entries[i].protocol_name,
		_IcePaAuthDataEntries[j].protocol_name) == 0 &&
                strcmp (entries[i].network_id,
		_IcePaAuthDataEntries[j].network_id) == 0 &&
                strcmp (entries[i].auth_name,
		_IcePaAuthDataEntries[j].auth_name) == 0)
		break;

	if (j < _IcePaAuthDataEntryCount)
	{
	    free (_IcePaAuthDataEntries[j].protocol_name);
	    free (_IcePaAuthDataEntries[j].network_id);
	    free (_IcePaAuthDataEntries[j].auth_name);
	    free (_IcePaAuthDataEntries[j].auth_data);
	}
	else
	{
	    _IcePaAuthDataEntryCount++;
	}

	_IcePaAuthDataEntries[j].protocol_name = (char *) malloc (
	    strlen (entries[i].protocol_name) + 1);
	strcpy (_IcePaAuthDataEntries[j].protocol_name,
	    entries[i].protocol_name);

	_IcePaAuthDataEntries[j].network_id = (char *) malloc (
	    strlen (entries[i].network_id) + 1);
	strcpy (_IcePaAuthDataEntries[j].network_id,
	    entries[i].network_id);

	_IcePaAuthDataEntries[j].auth_name = (char *) malloc (
            strlen (entries[i].auth_name) + 1);
	strcpy (_IcePaAuthDataEntries[j].auth_name,
	    entries[i].auth_name);

	_IcePaAuthDataEntries[j].auth_data_length =
            entries[i].auth_data_length;
	_IcePaAuthDataEntries[j].auth_data = (char *) malloc (
            entries[i].auth_data_length);
	memcpy (_IcePaAuthDataEntries[j].auth_data,
            entries[i].auth_data, entries[i].auth_data_length);
    }
}
