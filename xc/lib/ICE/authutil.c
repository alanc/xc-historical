/* $XConsortium$ */
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
#include <X11/Xos.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

extern long time ();
extern unsigned	sleep ();

static int binaryEqual ();
static int read_short ();
static int read_counted_string ();
static int write_short ();
static int write_counted_string ();
static int address_check ();



char *
IceAuthFileName ()

{
    char    	*name;
    static char	*buf;
    static int	bsize;
    int	    	size;
#ifdef WIN32
    char    	dir[128];
#endif

    if (name = getenv ("ICEAUTHORITY"))
	return (name);

    name = getenv ("HOME");

    if (!name)
    {
#ifdef WIN32
	strcpy (dir, "/users/");
	if (name = getenv ("USERNAME"))
	{
	    strcat (dir, name);
	    name = dir;
	}
	if (!name)
#endif
	return (NULL);
    }

    size = strlen (name) + strlen (".ICEauthority") + 2;

    if (size > bsize)
    {
	if (buf)
	    free (buf);
	buf = malloc ((unsigned) size);
	if (!buf)
	    return (NULL);
	bsize = size;
    }

    strcpy (buf, name);
    strcat (buf, "/.ICEauthority" + (name[1] == '\0' ? 1 : 0));

    return (buf);
}



int
IceLockAuthFile (file_name, retries, timeout, dead)

char	*file_name;
int	retries;
int	timeout;
long	dead;

{
#ifndef WIN32
    char	creat_name[1025], link_name[1025];
    struct stat	statb;
    long	now;
    int		creat_fd = -1;

    if (strlen (file_name) > 1022)
	return (ICE_AUTH_LOCK_ERROR);

    strcpy (creat_name, file_name);
    strcat (creat_name, "-c");
    strcpy (link_name, file_name);
    strcat (link_name, "-l");

    if (stat (creat_name, &statb) != -1)
    {
	now = time ((long *) 0);

	/*
	 * NFS may cause ctime to be before now, special
	 * case a 0 deadtime to force lock removal
	 */

	if (dead == 0 || now - statb.st_ctime > dead)
	{
	    unlink (creat_name);
	    unlink (link_name);
	}
    }
    
    while (retries > 0)
    {
	if (creat_fd == -1)
	{
	    creat_fd = creat (creat_name, 0666);

	    if (creat_fd == -1)
	    {
		if (errno != EACCES)
		    return (ICE_AUTH_LOCK_ERROR);
	    }
	    else
		close (creat_fd);
	}

	if (creat_fd != -1)
	{
	    if (link (creat_name, link_name) != -1)
		return (ICE_AUTH_LOCK_SUCCESS);

	    if (errno == ENOENT)
	    {
		creat_fd = -1;	/* force re-creat next time around */
		continue;
	    }

	    if (errno != EEXIST)
		return (ICE_AUTH_LOCK_ERROR);
	}

	sleep ((unsigned) timeout);
	--retries;
    }

    return (ICE_AUTH_LOCK_TIMEOUT);
#else
    return (ICE_AUTH_LOCK_SUCCESS);
#endif
}



void
IceUnlockAuthFile (file_name)

char	*file_name;

{
#ifndef WIN32
    char	creat_name[1025], link_name[1025];

    if (strlen (file_name) > 1022)
	return;

    strcpy (creat_name, file_name);
    strcat (creat_name, "-c");
    strcpy (link_name, file_name);
    strcat (link_name, "-l");

    unlink (creat_name);
    unlink (link_name);
#endif
}



IceAuthFileEntry *
IceReadAuthFileEntry (auth_file)

FILE	*auth_file;

{
    IceAuthFileEntry   	local;
    IceAuthFileEntry   	*ret;

    if (read_counted_string (&local.protocol_name_length,
	&local.protocol_name, auth_file) == 0)
	return (NULL);

    if (read_counted_string (&local.protocol_data_length,
	&local.protocol_data, auth_file) == 0)
    {
	if (local.protocol_name) free (local.protocol_name);
	return (NULL);
    }

    if (read_counted_string (&local.address_list_length,
	&local.address_list, auth_file) == 0)
    {
	if (local.protocol_name) free (local.protocol_name);
	if (local.protocol_data) free (local.protocol_data);
	return (NULL);
    }

    if (read_counted_string (&local.auth_name_length,
	&local.auth_name, auth_file) == 0)
    {
	if (local.protocol_name) free (local.protocol_name);
	if (local.protocol_data) free (local.protocol_data);
	if (local.address_list) free (local.address_list);
	return (NULL);
    }

    if (read_counted_string (&local.auth_data_length,
	&local.auth_data, auth_file) == 0)
    {
	if (local.protocol_name) free (local.protocol_name);
	if (local.protocol_data) free (local.protocol_data);
	if (local.address_list) free (local.address_list);
	if (local.auth_name) free (local.auth_name);
	return (NULL);
    }

    if (!(ret = (IceAuthFileEntry *) malloc (sizeof (IceAuthFileEntry))))
    {
	if (local.protocol_name) free (local.protocol_name);
	if (local.protocol_data) free (local.protocol_data);
	if (local.address_list) free (local.address_list);
	if (local.auth_name) free (local.auth_name);
	if (local.auth_data) free (local.auth_data);
	return (NULL);
    }

    *ret = local;

    return (ret);
}



void
IceDisposeAuthFileEntry (auth)

IceAuthFileEntry	*auth;

{
    if (auth)
    {
	if (auth->protocol_name) free (auth->protocol_name);
	if (auth->protocol_data) free (auth->protocol_data);
	if (auth->address_list) free (auth->address_list);
	if (auth->auth_name) free (auth->auth_name);
	if (auth->auth_data) free (auth->auth_data);
	free ((char *) auth);
    }
}



Status
IceWriteAuthFileEntry (auth_file, auth)

FILE			*auth_file;
IceAuthFileEntry	*auth;

{
    if (write_counted_string (auth->protocol_name_length,
	auth->protocol_name, auth_file) == 0)
	return (0);

    if (write_counted_string (auth->protocol_data_length,
	auth->protocol_data, auth_file) == 0)
	return (0);

    if (write_counted_string (auth->address_list_length,
	auth->address_list, auth_file) == 0)
	return (0);

    if (write_counted_string (auth->auth_name_length,
	auth->auth_name, auth_file) == 0)
	return (0);

    if (write_counted_string (auth->auth_data_length,
	auth->auth_data, auth_file) == 0)
	return (0);

    return (1);
}



Status
IceGetAuthNamesFromAuthFile (address_length, address,
    num_names_ret, names_lengths_ret, names_ret)

unsigned  address_length;
char	  *address;
unsigned  *num_names_ret;
unsigned  **names_lengths_ret;
char	  ***names_ret;

{
    FILE    		*auth_file;
    char    		*filename;
    unsigned		temp_lengths[MAX_ICE_AUTH_NAMES];
    char		*temp_names[MAX_ICE_AUTH_NAMES];
    IceAuthFileEntry    *entry;
    int			i;

    *num_names_ret = 0;
    *names_lengths_ret = NULL;
    *names_ret = NULL;

    if (!(filename = IceAuthFileName ()))
	return (0);

    if (access (filename, R_OK) != 0)		/* checks REAL id */
	return (0);

    if (!(auth_file = fopen (filename, "rb")))
	return (0);

    for (; *num_names_ret < MAX_ICE_AUTH_NAMES; )
    {
	if (!(entry = IceReadAuthFileEntry (auth_file)))
	    break;

	if (address_check (address_length, address,
	    entry->address_list_length, entry->address_list))
	{
	    for (i = 0; i < *num_names_ret; i++)
		if (binaryEqual (temp_names[i], entry->auth_name,
		    entry->auth_name_length))
		{
		    IceDisposeAuthFileEntry (entry);
		    break;
		}

	    if (i >= *num_names_ret)
	    {
		/*
		 * If there are multiple entries for a given address
		 * and auth name, only count the first.
		 */

		temp_lengths[*num_names_ret] = entry->auth_name_length;
		temp_names[*num_names_ret] = (char *) malloc (
		    entry->auth_name_length);
		memcpy (temp_names[*num_names_ret], entry->auth_name,
		    entry->auth_name_length);

		*num_names_ret += 1;
	    }
	}

	IceDisposeAuthFileEntry (entry);
    }

    fclose (auth_file);

    if (*num_names_ret > 0)
    {
	*names_lengths_ret = (unsigned *) malloc (
	    *num_names_ret * sizeof (unsigned));
	memcpy (*names_lengths_ret, temp_lengths,
	    *num_names_ret * sizeof (unsigned));

	*names_ret = (char **) malloc (
	    *num_names_ret * sizeof (char *));
	memcpy (*names_ret, temp_names,
	    *num_names_ret * sizeof (char *));
    }

    return (1);
}



void
IceFreeAuthNames (count, names)

unsigned 	count;
char		**names;

{
    if (names)
    {
	int i;

	for (i = 0; i < count; i++)
	    free (names[i]);

	free ((char *) names);
    }
}



IceAuthFileEntry *
IceGetAuthFileEntry (protocol_name_length, protocol_name,
    address_length, address, auth_name_length, auth_name)

unsigned  protocol_name_length;
char	  *protocol_name;
unsigned  address_length;
char	  *address;
unsigned  auth_name_length;
char	  *auth_name;

{
    FILE    		*auth_file;
    char    		*filename;
    IceAuthFileEntry    *entry;
    int	    		auth_index;

    if (!(filename = IceAuthFileName ()))
	return (NULL);

    if (access (filename, R_OK) != 0)		/* checks REAL id */
	return (NULL);

    if (!(auth_file = fopen (filename, "rb")))
	return (NULL);

    for (;;)
    {
	if (!(entry = IceReadAuthFileEntry (auth_file)))
	    break;

	if (protocol_name_length == entry->protocol_name_length &&
	    binaryEqual (protocol_name, entry->protocol_name,
	    protocol_name_length) && address_check (address_length, address,
	    entry->address_list_length, entry->address_list) &&
            auth_name_length == entry->auth_name_length &&
            binaryEqual (auth_name, entry->auth_name, auth_name_length))
	{
	    break;
	}

	IceDisposeAuthFileEntry (entry);
    }

    fclose (auth_file);

    return (entry);
}



IceAuthFileEntry *
IceGetBestAuthFileEntry (protocol_name_length, protocol_name,
    address_length, address, num_auth_names, auth_names_lengths, auth_names)

unsigned  protocol_name_length;
char	  *protocol_name;
unsigned  address_length;
char	  *address;
unsigned  num_auth_names;
unsigned  *auth_names_lengths;
char	  **auth_names;

{
    FILE    		*auth_file;
    char    		*filename;
    IceAuthFileEntry    *entry;
    IceAuthFileEntry    *best_entry;
    int	    		auth_index;
    int	    		best_auth_index;

    if (!(filename = IceAuthFileName ()))
	return (NULL);

    if (access (filename, R_OK) != 0)		/* checks REAL id */
	return (NULL);

    if (!(auth_file = fopen (filename, "rb")))
	return (NULL);

    best_entry = NULL;
    best_auth_index = num_auth_names;

    for (;;)
    {
	if (!(entry = IceReadAuthFileEntry (auth_file)))
	    break;

	if (protocol_name_length == entry->protocol_name_length &&
	    binaryEqual (protocol_name, entry->protocol_name,
	    protocol_name_length) && address_check (address_length, address,
	    entry->address_list_length, entry->address_list))
	{
	    if (best_auth_index == 0)
	    {
		best_entry = entry;
		break;
	    }

	    for (auth_index = 0; auth_index < best_auth_index; auth_index++)
		if (auth_names_lengths[auth_index] ==
		    entry->auth_name_length &&
		    !(strncmp (auth_names[auth_index], entry->auth_name,
		    entry->auth_name_length)))
		{
		    break;
		}

	    if (auth_index < best_auth_index)
	    {
		if (best_entry)
		    IceDisposeAuthFileEntry (best_entry);

		best_entry = entry;
		best_auth_index = auth_index;

		if (auth_index == 0)
		    break;

		continue;
	    }
	}

	IceDisposeAuthFileEntry (entry);
    }

    fclose (auth_file);

    return (best_entry);
}



/*
 * Internal routines to ICElib
 */


void
_IceGetAuthNames (address_length, address,
    num_names_ret, names_lengths_ret, names_ret)

unsigned  address_length;
char	  *address;
unsigned  *num_names_ret;
unsigned  **names_lengths_ret;
char	  ***names_ret;

{
    unsigned		temp_lengths[MAX_ICE_AUTH_NAMES];
    char		*temp_names[MAX_ICE_AUTH_NAMES];
    IceAuthDataEntry    *entry;
    int			i, j;

    *num_names_ret = 0;
    *names_lengths_ret = NULL;
    *names_ret = NULL;

    for (i = 0;
	i < _IceAuthDataEntryCount && *num_names_ret < MAX_ICE_AUTH_NAMES;
	i++)
    {
	entry = &_IceAuthDataEntries[i];

	if (address_check (address_length, address,
	    entry->address_list_length, entry->address_list))
	{
	    for (j = 0; j < *num_names_ret; j++)
		if (binaryEqual (temp_names[j], entry->auth_name,
		    entry->auth_name_length))
		{
		    break;
		}

	    if (j >= *num_names_ret)
	    {
		/*
		 * If there are multiple entries for a given address
		 * and auth name, only count the first.
		 */

		temp_lengths[*num_names_ret] = entry->auth_name_length;
		temp_names[*num_names_ret] = (char *) malloc (
		    entry->auth_name_length);
		memcpy (temp_names[*num_names_ret], entry->auth_name,
		    entry->auth_name_length);

		*num_names_ret += 1;
	    }
	}
    }

    if (*num_names_ret > 0)
    {
	*names_lengths_ret = (unsigned *) malloc (
	    *num_names_ret * sizeof (unsigned));
	memcpy (*names_lengths_ret, temp_lengths,
	    *num_names_ret * sizeof (unsigned));

	*names_ret = (char **) malloc (
	    *num_names_ret * sizeof (char *));
	memcpy (*names_ret, temp_names,
	    *num_names_ret * sizeof (char *));
    }
}



IceAuthDataEntry *
_IceGetAuthDataEntry (protocol_name_length, protocol_name,
    address_length, address, auth_name_length, auth_name)

unsigned  protocol_name_length;
char	  *protocol_name;
unsigned  address_length;
char	  *address;
unsigned  auth_name_length;
char	  *auth_name;

{
    IceAuthDataEntry	*entry;
    int			found = 0;
    int			i;

    for (i = 0; i < _IceAuthDataEntryCount && !found; i++)
    {
	entry = &_IceAuthDataEntries[i];

	found =
	    entry->protocol_name_length == protocol_name_length &&
	    binaryEqual (entry->protocol_name, protocol_name,
		protocol_name_length) &&
	    address_check (address_length, address,
	        entry->address_list_length, entry->address_list) &&
	    entry->auth_name_length == auth_name_length &&
	    binaryEqual (entry->auth_name, auth_name, auth_name_length);
    }

    if (found)
	return (entry);
    else
	return (NULL);
}



/*
 * local routines
 */

static int
binaryEqual (a, b, len)

register char		*a, *b;
register unsigned	len;

{
    while (len--)
	if (*a++ != *b++)
	    return 0;
    return 1;
}


static int
read_short (shortp, file)

unsigned short	*shortp;
FILE		*file;

{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}


static int
read_counted_string (countp, stringp, file)

unsigned short	*countp;
char	**stringp;
FILE	*file;

{
    unsigned short  len;
    char	    *data;

    if (read_short (&len, file) == 0)
	return (0);

    if (len == 0)
    {
	data = 0;
    }
    else
    {
    	data = malloc ((unsigned) len);

    	if (!data)
	    return (0);

    	if (fread (data, (int) sizeof (char), (int) len, file) != len)
	{
	    free (data);
	    return (0);
    	}
    }

    *stringp = data;
    *countp = len;

    return (1);
}


static int
write_short (s, file)

unsigned short	s;
FILE		*file;

{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;

    if (fwrite ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return (0);

    return (1);
}


static int
write_counted_string (count, string, file)

unsigned short	count;
char		*string;
FILE		*file;

{
    if (write_short (count, file) == 0)
	return (0);

    if (fwrite (string, (int) sizeof (char), (int) count, file) != count)
	return (0);

    return (1);
}


static char *
findchar (strptr, ch, bytes)

char *strptr;
char ch;
unsigned bytes;

{
    char *ptr = strptr;

    while (*ptr != ch && bytes)
    {
	ptr++;
	bytes--;
    }

    if (bytes > 0 && *ptr == ch)
	return (ptr);
    else
	return (NULL);
}


static int
address_check (address_length, address, address_list_length, address_list)

unsigned address_length;
char	 *address;
unsigned address_list_length;
char     *address_list;

{
    /* Check if address is in address_list */

    char *ptr = address_list;
    char *next;
    unsigned bytes;
    int bytesLeft = (int) address_list_length;

    while (bytesLeft > 0)
    {
	next = (char *) findchar (ptr, ',', bytesLeft);

	if (next)
	    bytes = next - ptr;
	else
	    bytes = bytesLeft;

	if (bytes == address_length &&
	    binaryEqual (ptr, address, address_length))
	{
	    return (1);
	}

	bytesLeft -= (bytes + 1);
	ptr += (bytes + 1);
    }

    return (0);
}
