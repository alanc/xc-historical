/*
 * xdm - display manager daemon
 *
 * $XConsortium$
 *
 * Copyright 1994 Massachusetts Institute of Technology
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
 * Author:  Stephen Gildea, X Consortium
 */

/*
 * krb5auth
 *
 * generate Kerberos Version 5 authorization records
 */

#include "dm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <krb5/krb5.h>
#include <krb5/kdb.h>			/* for TGTNAME */

/*ARGSUSED*/
Krb5InitAuth (name_len, name)
    unsigned short  name_len;
    char	    *name;
{
    krb5_init_ets();		/* initialize error_message() tables */
}

Xauth *
Krb5GetAuthForUid(namelen, name, uid)
    unsigned short namelen;
    char *name;
    int uid;
{
    Xauth   *new;
    char filename_buf[30];
    struct stat statbuf;

    new = (Xauth *) malloc (sizeof *new);
    if (!new)
	return (Xauth *) 0;
    new->family = FamilyWild;
    new->address_length = 0;
    new->address = 0;
    new->number_length = 0;
    new->number = 0;

    /*
     * we'd like to use krb5_cc_default_name() here, but it
     * doesn't take a UID as an argument, sigh.
     * If uid < 0, no one has logged in yet.
     */
    if (uid >= 0)
    {
	sprintf(filename_buf, "/tmp/krb5cc_%d", uid);
	new->data = (char *) malloc (8 + strlen(filename_buf) + 1);
	if (!new->data)
	{
	    free ((char *) new);
	    return (Xauth *) 0;
	}
	strcpy(new->data, "UU:FILE:");
	strcat(new->data, filename_buf);
	new->data_length = strlen(new->data);
    }
    else
    {
	new->data = NULL;
	new->data_length = 0;
    }

    new->name = (char *) malloc (namelen);
    if (!new->name)
    {
	free ((char *) new->data);
	free ((char *) new);
	return (Xauth *) 0;
    }
    memmove( new->name, name, namelen);
    new->name_length = namelen;
    return new;
}


Xauth *
Krb5GetAuth (namelen, name)
    unsigned short  namelen;
    char	    *name;
{
    return Krb5GetAuthForUid(namelen, name, -1);
}


int preauth_search_list[] = {
	0,			
	KRB5_PADATA_ENC_TIMESTAMP,
	-1
	};

/*
 * Krb5Init - lifted from kinit.c
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * Get TGT.
 * Returns 0 if successful, 1 if not.
 */
int
Krb5Init(name, passwd)
    char *name;
    char *passwd;
{
    krb5_ccache ccache = NULL;
    krb5_error_code code;
    krb5_principal me;
    krb5_creds my_creds;
    krb5_principal server;
    krb5_address **my_addresses;
    krb5_timestamp now;
    int	i;

    if (ccache == NULL) {
	 if (code = krb5_cc_default(&ccache)) {
	      com_err("xdm", code, "while getting default ccache");
	      return 1;
	 }
    }

    if (code = krb5_parse_name (name, &me)) {
	 com_err ("xdm", code, "when parsing name %s", name);
	 return 1;
    }

    code = krb5_cc_initialize (ccache, me);
    if (code != 0) {
	com_err ("xdm", code, "when initializing cache %s",
		 krb5_cc_default_name());
	return 1;
    }

    memset((char *)&my_creds, 0, sizeof(my_creds));
    
    my_creds.client = me;

    if (code = krb5_build_principal_ext(&server,
					krb5_princ_realm(me)->length,
					krb5_princ_realm(me)->data,
					6, "krbtgt",
					krb5_princ_realm(me)->length,
					krb5_princ_realm(me)->data,
					0)) {
	com_err("xdm", code, "while building server name");
	return 1;
    }

    my_creds.server = server;

    code = krb5_os_localaddr(&my_addresses);
    if (code != 0) {
	com_err ("xdm", code, "when getting my address");
	return 1;
    }
    if (code = krb5_timeofday(&now)) {
	com_err("xdm", code, "while getting time of day");
	return 1;
    }
    my_creds.times.starttime = 0;	/* start timer when request
					   gets to KDC */
    my_creds.times.endtime = now + 60*60*8; /* 8 hours */
    my_creds.times.renew_till = 0;

    for (i = 0; preauth_search_list[i] >= 0; i++) {
	code = krb5_get_in_tkt_with_password(0, my_addresses,
					     preauth_search_list[i],
					     ETYPE_DES_CBC_CRC,
					     KEYTYPE_DES,
					     passwd,
					     ccache,
					     &my_creds, 0);
	if (code != KRB5KDC_PREAUTH_FAILED &&
	    code != KRB5KRB_ERR_GENERIC)
	    break;
    }
    
    krb5_free_principal(server);
    krb5_free_addresses(my_addresses);
    
    if (code) {
	if (code == KRB5KRB_AP_ERR_BAD_INTEGRITY) {
	    char *my_name = NULL;
	    code = krb5_unparse_name(me, &my_name);
	    LogError ("password incorrect for Krb5 principal \"%s\"\n",
		      code ? name : my_name);
	    if (my_name)
		free (my_name);
	}
	else
	    com_err ("xdm", code, "while getting initial credentials");
	return 1;
    }
    return 0;
}
