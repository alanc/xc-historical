/* $XConsortium: setauth.c,v 1.2 93/11/30 15:30:21 mor Exp $ */
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
                strcmp (entries[i].address,
		_IcePaAuthDataEntries[j].address) == 0 &&
                strcmp (entries[i].auth_name,
		_IcePaAuthDataEntries[j].auth_name) == 0)
		break;

	if (j < _IcePaAuthDataEntryCount)
	{
	    free (_IcePaAuthDataEntries[j].protocol_name);
	    free (_IcePaAuthDataEntries[j].address);
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

	_IcePaAuthDataEntries[j].address = (char *) malloc (
	    strlen (entries[i].address) + 1);
	strcpy (_IcePaAuthDataEntries[j].address,
	    entries[i].address);

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
