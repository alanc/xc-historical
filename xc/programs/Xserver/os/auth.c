/*
 * authorization hooks for the server
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

# include   "X.h"
# include   "Xauth.h"

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

extern int  MitAddCookie ();
extern XID  MitCheckCookie ();
extern int  MitResetCookie ();
extern XID  MitToID ();
extern int  MitFromID (), MitRemoveCookie ();

static struct protocol   protocols[] = {
{   (unsigned short) 18,    "MIT-MAGIC-COOKIE-1",
		MitAddCookie,	MitCheckCookie,	MitResetCookie,
		MitToID,	MitFromID,	MitRemoveCookie,
},
};

# define NUM_AUTHORIZATION  (sizeof (protocols) /\
			     sizeof (struct protocol))

/*
 * Initialize all classes of authorization by reading the
 * specified authorization file
 */

static char *authorization_file;

static int  AuthorizationIndex = 0;

InitAuthorization (file_name)
char	*file_name;
{
    FILE    *f;
    Xauth   *auth;
    int	    i;
    int	    count = 0;

    authorization_file = file_name;
    f = fopen (authorization_file, "r");
    if (!f)
	return 0;
    while (auth = XauReadAuth (f)) {
	for (i = 0; i < NUM_AUTHORIZATION; i++) {
	    if (protocols[i].name_length == auth->name_length &&
		bcmp (protocols[i].name, auth->name, (int) auth->name_length) == 0)
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

XID
CheckAuthorization (name_length, name, data_length, data)
unsigned short	name_length;
char	*name;
unsigned short	data_length;
char	*data;
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
    	if (protocols[i].name_length == name_length &&
	    bcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].Check) (data_length, data);
    	}
    }
    return (XID) -1;
}

ResetAuthorization ()
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++)
	(*protocols[i].Reset)();
    AuthorizationIndex = 0;
    if (InitAuthorization (authorization_file))
	DisableLocalHost ();
    else
	EnableLocalHost ();
}

XID
AuthorizationToID (name_length, name, data_length, data)
{
    int	i;

    for (i = 0; i < NUM_AUTHORIZATION; i++) {
    	if (protocols[i].name_length == name_length &&
	    bcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].ToID) (data_length, data);
    	}
    }
    return (XID) -1;
}

AuthorizationFromID (id, name_lenp, namep, data_lenp, datap)
XID id;
unsigned short	*name_lenp;
char	**namep;
unsigned short	*data_lenp;
char	**datap;
{
    int	i;
    XID	ret;

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
	    bcmp (protocols[i].name, name, (int) name_length) == 0)
    	{
	    return (*protocols[i].Remove) (data_length, data);
    	}
    }
    return 0;
}
