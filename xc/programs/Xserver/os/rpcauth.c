/*
 * SECURE-RPC authentication mechanism
 *
 * $XConsortium: rpcauth.c,v 1.3 89/03/14 15:53:36 rws Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
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
 * Author:  Mayank Choudhary, Sun Microsystems
 */

#ifdef SECURE_RPC
#include <rpc/rpc.h>
#ifdef ultrix
#include <time.h>
#include <rpc/auth_des.h>
#endif
#include "Xauth.h"
#include "misc.h"
#include "X.h"
#include "os.h"

#define MAX_NAMELEN 120

typedef struct _netnamelist {
    char netname[MAX_NAMELEN];
    struct _netnamelist *next;
} NetNameList;

static NetNameList *netnamelist = NULL;

static struct auth {
    struct auth	*next;
    unsigned short	len;
    char	*data;
    XID		id;
} *rpc_auth;

static char * 
authdes_ezdecode(inmsg, len)
char *inmsg;
int  len;
{
    enum auth_stat  why;
    struct rpc_msg  msg;
    char            cred_area[MAX_AUTH_BYTES];
    char            verf_area[MAX_AUTH_BYTES];
    char            *temp_inmsg;
    struct svc_req  r;
    bool_t          res0, res1;
    XDR             xdr;
    SVCXPRT         xprt;

    temp_inmsg = (char *) xalloc(len);
    bcopy(inmsg, temp_inmsg, len);

    memset(&msg, 0, sizeof(msg));
    memset(&r, 0, sizeof(r));
    memset(cred_area, 0, sizeof(cred_area));
    memset(verf_area, 0, sizeof(verf_area));

    msg.rm_call.cb_cred.oa_base = cred_area;
    msg.rm_call.cb_verf.oa_base = verf_area;
    why = AUTH_FAILED; 
    xdrmem_create(&xdr, temp_inmsg, len, XDR_DECODE);

    if ((r.rq_clntcred = (caddr_t) xalloc(MAX_AUTH_BYTES)) == NULL)
        goto bad1;
    r.rq_xprt = &xprt;

    /* decode into msg */
    res0 = xdr_opaque_auth(&xdr, &(msg.rm_call.cb_cred)); 
    res1 = xdr_opaque_auth(&xdr, &(msg.rm_call.cb_verf));
    if ( ! (res0 && res1) )
         goto bad2;

    /* do the authentication */

    r.rq_cred = msg.rm_call.cb_cred;        /* read by opaque stuff */
    if (r.rq_cred.oa_flavor != AUTH_DES) {
        why = AUTH_TOOWEAK;
        goto bad2;
    }
    if ((why = _authenticate(&r, &msg)) != AUTH_OK) {
            goto bad2;
    }
    return (((struct authdes_cred *) r.rq_clntcred)->adc_fullname.name); 

bad2:
    Xfree(r.rq_clntcred);
bad1:
    return ((char *)0); /* ((struct authdes_cred *) NULL); */
}

int
SecureRPCAdd (data_length, data, id)
unsigned short	data_length;
char	*data;
XID	id;
{
    struct auth	*new, *auth;
    NetNameList *netname;

    for (auth = rpc_auth; auth; auth=auth->next)
      if (auth->len == data_length &&
	  (bcmp(auth->data, data, data_length) == 0))
	return 1;

    new = (struct auth *) xalloc (sizeof (struct auth));
    if (!new)
	return 0;
    new->data = (char *) xalloc ((unsigned) data_length);
    if (!new->data) {
	xfree(new);
	return 0;
    }
    new->next = rpc_auth;
    rpc_auth = new;
    bcopy (data, new->data, (int) data_length);
    new->len = data_length;
    new->id = id;

    /* Initialize NetNameList */
    /* Define self, the person who started the server that is */
    netnamelist = (NetNameList *)xalloc(sizeof (NetNameList));
    strcpy(netnamelist->netname, data);
    netnamelist->next = (NetNameList *)0;

    /* Read netnames from /etc/X?.hosts */
    
    return 1;
}

XID
SecureRPCCheck (data_length, data)
register unsigned short	data_length;
char	*data;
{
  struct auth	*auth;
  char *fullname;
  NetNameList *netname;

  for (auth = rpc_auth; auth; auth=auth->next) {
    if ((fullname = authdes_ezdecode(data, data_length)) != (char *)0)
    {
	for (netname = netnamelist; netname; netname = netname->next)
	    if (strcmp(netname->netname, fullname) == 0)
		return auth->id;
	return (XID) -1;
    }
  }
  return (XID) -1;
}

int
  SecureRPCReset ()
{
  struct auth	*auth, *next;
/* Why does main keep calling this after every connection terminates?  
  for (auth = rpc_auth; auth; auth=next) {
    next = auth->next;
    xfree (auth->data);
    xfree (auth);
  }
  rpc_auth = 0;
*/
}

XID
  SecureRPCToID (data_length, data)
unsigned short	data_length;
char	*data;
{
  struct auth	*auth;
  
  for (auth = rpc_auth; auth; auth=auth->next) {
    if (data_length == auth->len &&
	authdes_ezdecode(data, data_length) == 0)
      return auth->id;
  }
  return (XID) -1;
}

SecureRPCFromID (id, data_lenp, datap)
     XID id;
     unsigned short	*data_lenp;
     char	**datap;
{
  struct auth	*auth;
  
  for (auth = rpc_auth; auth; auth=auth->next) {
    if (id == auth->id) {
      *data_lenp = auth->len;
      *datap = auth->data;
      return 1;
    }
  }
  return 0;
}

SecureRPCRemove (data_length, data)
     unsigned short	data_length;
     char	*data;
{
  struct auth	*auth, *prev;
  
  prev = 0;
  for (auth = rpc_auth; auth; auth=auth->next) {
    if (data_length == auth->len &&
	bcmp (data, auth->data, data_length) == 0)
      {
	if (prev)
	  prev->next = auth->next;
	else
	  rpc_auth = auth->next;
	xfree (auth->data);
	xfree (auth);
	return 1;
      }
  }
  return 0;
}

AddNetName(family, length, pAddr)
     int family;
     unsigned length;
     pointer pAddr;
{
  char newnetname[MAX_NAMELEN];
  NetNameList *netname;
  
  if (family != FamilySecureRPC)
    return(BadValue);
  
  bcopy(pAddr, newnetname, length);
  newnetname[length] = '\0';
  
  for (netname = netnamelist; netname; netname = netname->next)
    if (strcmp(newnetname, netname->netname) == 0)
      return(Success);
  netname = (NetNameList *)xalloc(sizeof (NetNameList));
  if (!netname)
    return(BadAlloc);
  strcpy(netname->netname, newnetname);
  netname->next = netnamelist;
  netnamelist = netname;
  return (Success);
}

RemoveNetName(family, length, pAddr)
     int family;
     unsigned length;
     pointer pAddr;
{
  char newnetname[MAX_NAMELEN];
  NetNameList *prev, *curr;
  
  if (family != FamilySecureRPC)
    return(BadValue);
  
  bcopy(pAddr, newnetname, length);
  newnetname[length] = '\0';
  
  for (prev = curr = netnamelist; curr; prev=curr, curr = curr->next) {
    if (strcmp(newnetname, curr->netname) == 0) {
      if (prev == curr)
	netnamelist = curr->next;
      else
	prev->next = curr->next;
      xfree(curr);
      return (Success);
    }
  }
}
#endif SECURE_RPC

