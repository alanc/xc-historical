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

#include "xsm.h"

#ifdef X_NOT_STDC_ENV
#define Time_t long
extern Time_t time ();
#else
#include <time.h>
#define Time_t time_t
#endif

#define MAGIC_COOKIE_LEN 16



/*
 * Host Based Authentication Callback.  This callback is invoked if
 * the connecting client can't offer any authentication methods that
 * we can accept.  We can accept/reject based on the hostname.
 */

Bool
HostBasedProc (hostname)

char *hostname;

{
    return (0);	      /* For now, we don't support host based authentication */
}



static
GenerateMagicCookie (auth, len)

char		**auth;
unsigned short 	*len;

{
    long    ldata[2];
    int	    seed;
    int	    value;
    int	    i;
    
    *len = MAGIC_COOKIE_LEN;
    *auth = (char *) malloc (MAGIC_COOKIE_LEN + 1);

#ifdef ITIMER_REAL
    {
	struct timeval  now;
#if defined(SVR4) || defined(WIN32) || defined(VMS)
	gettimeofday (&now);
#else
	struct timezone zone;
	gettimeofday (&now, &zone);
#endif
	ldata[0] = now.tv_sec;
	ldata[1] = now.tv_usec;
    }
#else
    {
	long    time ();

	ldata[0] = time ((long *) 0);
	ldata[1] = getpid ();
    }
#endif
    seed = (ldata[0]) + (ldata[1] << 16);
    srand (seed);
    for (i = 0; i < (int) *len; i++)
    {
	value = rand ();
	(*auth)[i] = value & 0xff;
    }
    (*auth)[*len] = '\0';
}



/*
 * We use temporary files which contain commands to add/remove entries from
 * the .ICEauthority file.
 */

static void
write_iceauth (addfp, removefp, entry)

FILE		 *addfp;
FILE 		 *removefp;
IceAuthDataEntry *entry;

{
    fprintf (addfp,
	"add %s \"\" %s %s ",
	entry->protocol_name,
        entry->address,
        entry->auth_name);
    fprintfhex (addfp, entry->auth_data_length, entry->auth_data);
    fprintf (addfp, "\n");

    fprintf (removefp,
	"remove protoname=%s protodata=\"\" address=%s authname=%s\n",
	entry->protocol_name,
        entry->address,
        entry->auth_name);
}



/*
 * Provide authentication data to clients that wish to connect
 */

Status
set_auth (count, listenObjs, authDataEntries)

int			count;
IceListenObj		*listenObjs;
IceAuthDataEntry	**authDataEntries;

{
    FILE	*addfp;
    FILE	*removefp;
    int		i;

    if (!(addfp = fopen (".xsm-add-auth", "w")))
	return (0);

    if (!(removefp = fopen (".xsm-rem-auth", "w")))
	return (0);

    *authDataEntries = (IceAuthDataEntry *) malloc (
	count * 2 * sizeof (IceAuthDataEntry));

    for (i = 0; i < count * 2; i += 2)
    {
	(*authDataEntries)[i].address =
	    IceGetListenNetworkId (listenObjs[i/2]);
	(*authDataEntries)[i].protocol_name = "ICE";
	(*authDataEntries)[i].auth_name = "MIT-MAGIC-COOKIE-1";

	GenerateMagicCookie (&(*authDataEntries)[i].auth_data,
	    &(*authDataEntries)[i].auth_data_length);

	(*authDataEntries)[i+1].address =
	    IceGetListenNetworkId (listenObjs[i/2]);
	(*authDataEntries)[i+1].protocol_name = "XSMP";
	(*authDataEntries)[i+1].auth_name = "MIT-MAGIC-COOKIE-1";

	GenerateMagicCookie (&(*authDataEntries)[i+1].auth_data,
	    &(*authDataEntries)[i+1].auth_data_length);

	write_iceauth (addfp, removefp, &(*authDataEntries)[i]);
	write_iceauth (addfp, removefp, &(*authDataEntries)[i+1]);

	IceSetPaAuthData (2, &(*authDataEntries)[i]);

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedProc);
    }

    fclose (addfp);
    fclose (removefp);

    system ("iceauth source .xsm-add-auth");

    return (1);
}



/*
 * Free up authentication data.
 */

void
free_auth (count, authDataEntries)

int			count;
IceAuthDataEntry 	*authDataEntries;

{
    /* Each transport has entries for ICE and XSMP */

    int i;

    for (i = 0; i < count * 2; i++)
    {
	free (authDataEntries[i].address);
	free (authDataEntries[i].auth_data);
    }

    free ((char *) authDataEntries);

    system ("iceauth source .xsm-rem-auth");
}