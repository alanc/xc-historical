/*
 * authorization hooks for the server
 *
 * $XConsortium: auth.c,v 1.19 94/02/08 14:55:09 gildea Exp $
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

#ifdef K5AUTH
# include   <krb5/krb5.h>
#endif
# include   "X.h"
# include   "Xauth.h"
# include   "misc.h"
# include   "dixstruct.h"
# include   <sys/types.h>
# include   <sys/stat.h>

struct protocol {
    unsigned short   name_length;
    char    *name;
    int     (*Add)();	    /* new authorization data */
    XID	    (*Check)();	    /* verify client authorization data */
    int     (*Reset)();	    /* delete all authorization data entries */
    XID	    (*ToID)();	    /* convert cookie to ID */
    int	    (*FromID)();    /* convert ID to cookie */
    int	    (*Remove)();    /* remove a specific cookie */
};

#ifdef HASXDMAUTH
extern int  XdmAddCookie ();
extern XID  XdmCheckCookie ();
extern int  XdmResetCookie ();
extern XID  XdmToID ();
extern int  XdmFromID (), XdmRemoveCookie ();
#endif

#ifdef SECURE_RPC
extern int  SecureRPCAdd();
extern XID  SecureRPCCheck();
extern int  SecureRPCReset();
extern XID  SecureRPCToID();
extern int  SecureRPCFromID(), SecureRPCRemove();
#endif

#ifdef K5AUTH
extern int K5Add();
extern XID K5Check();
extern int K5Reset();
extern XID K5ToID();
extern int K5FromID(), K5Remove();
#endif

static struct protocol   protocols[] = {
#ifdef HASXDMAUTH
{   (unsigned short) 19,    "XDM-AUTHORIZATION-1",
		XdmAddCookie,	XdmCheckCookie,	XdmResetCookie,
		XdmToID,	XdmFromID,	XdmRemoveCookie,
},
#endif
#ifdef SECURE_RPC
{   (unsigned short) 9,    "SUN-DES-1",
		SecureRPCAdd,	SecureRPCCheck,	SecureRPCReset,
		SecureRPCToID,	SecureRPCFromID,SecureRPCRemove,
},
#endif
#ifdef K5AUTH
{   (unsigned short) 14, "MIT-KERBEROS-5",
		K5Add, K5Check, K5Reset,
		K5ToID, K5FromID, K5Remove,
},
#endif
};

# define NUM_AUTHORIZATION  (sizeof (protocols) /\
			     sizeof (struct protocol))

/*
 * Initialize all classes of authorization by reading the
 * specified authorization file
 */

static char *authorization_file = (char *)NULL;

static int  AuthorizationIndex = 0;
static Bool ShouldLoadAuth = TRUE;

InitAuthorization (file_name)
char	*file_name;
{
    authorization_file = file_name;
}

int
LoadAuthorization ()
{
    FILE    *f;
    Xauth   *auth;
    int	    i;
    int	    count = 0;

    ShouldLoadAuth = FALSE;
    if (!authorization_file)
	return 0;
    f = fopen (authorization_file, "r");
    if (!f)
	return 0;
    AuthorizationIndex = 0;
    while (auth = XauReadAuth (f)) {
	for (i = 0; i < NUM_AUTHORIZATION; i++) {
	    if (protocols[i].name_length == auth->name_length &&
		memcmp (protocols[i].name, auth->name, (int) auth->name_length) == 0)
	    {
		++count;
		(*protocols[i].Add) (auth->data_length, auth->data,
					 ++AuthorizationIndex);
	    }
	}
	XauDisposeAuth (auth);
    }
    fclose (f);
    return count;
}

#ifdef XDMCP
/*
 * XdmcpInit calls this function to discover all authorization
 * schemes supported by the display
 */
RegisterAuthorizations ()
{
    int	    i;

    for (i = 0; i < NUM_AUTHORIZATION; i++)
	XdmcpRegisterAuthorization (protocols[i].name,
				    (int)protocols[i].name_length);
}
#endif

XID
CheckAuthorization (name_length, name, data_length, data, client, reason)
    unsigned short	name_length;
    char	*name;
    unsigned short	data_length;
    char	*data;
    ClientPtr client;
    char	**reason;	/* failure message.  NULL for default msg */
{
    int	i;
    struct stat buf;
    static time_t lastmod = 0;

    if (!authorization_file || stat(authorization_file, &buf))
    {
	lastmod = 0;
	ShouldLoadAuth = TRUE;	/* stat lost, so force reload */
    }
    else if (buf.st_mtime > lastmod)
    {
	lastmod = buf.st_mtime;
	ShouldLoadAuth = TRUE;
    }
    if (ShouldLoadAuth)
    {
	if (LoadAuthorization())
	    DisableLocalHost(); /* got at least one */
	else
	    EnableLocalHost ();
    }
    if (name_length)
	for (i = 0; i < NUM_AUTHORIZATION; i++) {
	    if (protocols[i].name_length == name_length &&
		memcmp (protocols[i].name, name, (int) name_length) == 0)
	    {
		return (*protocols[i].Check) (data_length, data, client, reason);
	    }
	}
    return (XID) ~0L;
}

ResetAuthorization ()
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++)
	(*protocols[i].Reset)();
    ShouldLoadAuth = TRUE;
}

XID
AuthorizationToID (name_length, name, data_length, data)
unsigned short	name_length;
char	*name;
unsigned short	data_length;
char	*data;
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
    	if (protocols[i].name_length == name_length &&
	    memcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].ToID) (data_length, data);
    	}
    }
    return (XID) ~0L;
}

AuthorizationFromID (id, name_lenp, namep, data_lenp, datap)
XID id;
unsigned short	*name_lenp;
char	**namep;
unsigned short	*data_lenp;
char	**datap;
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
	if ((*protocols[i].FromID) (id, data_lenp, datap)) {
	    *name_lenp = protocols[i].name_length;
	    *namep = protocols[i].name;
	    return 1;
	}
    }
    return 0;
}

RemoveAuthorization (name_length, name, data_length, data)
unsigned short	name_length;
char	*name;
unsigned short	data_length;
char	*data;
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
    	if (protocols[i].name_length == name_length &&
	    memcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].Remove) (data_length, data);
    	}
    }
    return 0;
}

AddAuthorization (name_length, name, data_length, data)
unsigned short	name_length;
char	*name;
unsigned short	data_length;
char	*data;
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
    	if (protocols[i].name_length == name_length &&
	    memcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].Add) (data_length, data,
					++AuthorizationIndex);
    	}
    }
    return 0;
}
