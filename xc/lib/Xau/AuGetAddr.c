/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include "Xauth.h"

static
binaryEqual (a, b, len)
register char	*a, *b;
register int	len;
{
    while (len--)
	if (*a++ != *b++)
	    return 0;
    return 1;
}

Xauth *
XauGetAuthByAddr (family, address_length, address,
			  number_length, number,
			  name_length, name)
char	family;
unsigned short	address_length;
char	*address;
unsigned short	number_length;
char	*number;
unsigned short	name_length;
char	*name;
{
    FILE    *auth_file;
    char    *auth_name;
    Xauth   *entry;

    auth_name = XauFileName ();
    if (!auth_name)
	return 0;
    auth_file = fopen (auth_name, "r");
    if (!auth_file)
	return 0;
    for (;;) {
	entry = XauReadAuth (auth_file);
	if (!entry)
	    break;
	/*
	 * entries with empty address/number match all requests
	 */
	if (entry->address_length == 0 && entry->number_length == 0)
	    break;
	if (entry->family == family &&
	    entry->address_length == address_length &&
	    entry->number_length == number_length &&
	    binaryEqual (entry->address, address, address_length) &&
	    binaryEqual (entry->number, number, number_length) &&
	    (name_length == 0 ||
	     entry->name_length == name_length &&
 	     binaryEqual (entry->name, name, name_length)))
	    break;
	XauDisposeAuth (entry);
    }
    (void) fclose (auth_file);
    return entry;
}
