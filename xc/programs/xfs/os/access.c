/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)access.c	4.1	91/05/02
 *
 */

#include        <sys/param.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	"client.h"
#include	"misc.h"
#include	"site.h"
#include	"accstr.h"
#include	"osdep.h"

long        MaxClients = DEFAULT_CLIENT_LIMIT;

void
AccessSetConnectionLimit(num)
    int         num;
{
    if (num > MAXSOCKS) {
	ErrorF("Client limit of %d too high; using default of %d\n",
	       num, DEFAULT_CLIENT_LIMIT);
	return;
    }
    MaxClients = num;
}

int
AddHost(list, addr)
    HostList   *list;
    HostAddress *addr;
{
    HostAddress *new;

    new = (HostAddress *) fsalloc(sizeof(HostAddress));
    if (!new)
	return FSBadAlloc;
    new->address = (pointer) fsalloc(addr->addr_len);
    if (!new->address) {
	fsfree((char *) addr);
	return FSBadAlloc;
    }
    new->type = addr->type;
    new->addr_len = addr->addr_len;
    bcopy((char *) addr->address, (char *) new->address, new->addr_len);

    new->next = *list;
    *list = new;
    return FSSuccess;
}

int
RemoveHost(list, addr)
    HostList   *list;
    HostAddress *addr;
{
    HostAddress *t,
               *last;

    last = (HostAddress *) 0;
    t = *list;
    while (t) {
	if (t->type == addr->type &&
		t->addr_len == addr->addr_len &&
		bcmp((char *) t->address, (char *) addr->address,
		     min(t->addr_len, addr->addr_len)) == 0) {
	    if (last) {
		last->next = t->next;
	    } else {
		*list = t->next;
	    }
	    fsfree((char *) t->address);
	    fsfree((char *) t);
	    return FSSuccess;
	}
	last = t;
	t = t->next;
    }
    return FSBadName;		/* bad host name */
}

Bool
ValidHost(list, addr)
    HostList    list;
    HostAddress *addr;
{
    HostAddress *t;

    t = list;
    while (t) {
	if (t->type == addr->type &&
		t->addr_len == addr->addr_len &&
		bcmp((char *) t->address, (char *) addr->address,
		     min(t->addr_len, addr->addr_len)) == 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * XXX
 *
 * needs massive amounts of OS-dependent work (big surprise)
 */
int
GetHostAddress(addr)
    HostAddress *addr;
{
    char        hname[64];
    struct hostent *hp;

    addr->addr_len = sizeof(struct in_addr);
    addr->address = (pointer) fsalloc(addr->addr_len);
    if (!addr->address)
	return FSBadAlloc;
    addr->type = HOST_AF_INET;
    gethostname(hname, sizeof(hname));
    hp = gethostbyname(hname);
    if (hp) {
	bcopy((char *) hp->h_addr, (char *) addr->address, addr->addr_len);
    } else {
	fsfree((char *) addr->address);
	return FSBadName;
    }
    return FSSuccess;
}

/* ARGSUSED */
int
CheckClientAuthorization(client, client_auth, accept, index, size, auth_data)
    ClientPtr   client;
    AuthPtr     client_auth;
    int        *accept;
    int        *index;
    int        *size;
    char      **auth_data;
{
    *size = 0;
    *accept = AuthSuccess;
    *index = 0;
    return FSSuccess;
}
