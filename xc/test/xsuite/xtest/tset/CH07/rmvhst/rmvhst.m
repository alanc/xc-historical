/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
>>TITLE XRemoveHost CH07
void

Display	*display = Dsp;
XHostAddress	*host = xthosts;
>>EXTERN

#include	"xthost.h"

>>ASSERTION Good B 1
A call to xname
removes the specified host from the access control list
for the display.
>>STRATEGY
Get current acl.
If current list is empty:
  Add a host with XAddHost.
  Get current acl.
  Check that it is not empty.
Call xname to remove host.
If call fails with a BadAccess:
  Report that assertion can not be tested from this host.
Get new acl.
Verify that host is not in new acl.
>>CODE
XHostAddress	*oldhosts;
XHostAddress	*newhosts;
int 	nhosts;
Bool	state;

	oldhosts = XListHosts(display, &nhosts, &state);

	if (nhosts == 0) {
		debug(1, "adding host");
		CATCH_ERROR(display);
		XAddHost(display, &xthosts[0]);
		RESTORE_ERROR(display);
		if (GET_ERROR(display) == BadAccess) {
			untested("This assertion cannot be tested from this host");
			report("  the server did not permit the access control list to be changed");
			return;
		}
		oldhosts = XListHosts(display, &nhosts, &state);
		if (nhosts == 0) {
			delete("Could not add any hosts to delete");
			return;
		}
	}
	if (isdeleted())
		return;

	/* Remove the first host */
	host = &oldhosts[0];
>>SET no-error-status-check
	XCALL;

	if (geterr() == BadAccess) {
		untested("This host does not have permission to change the list");
		report("  so this assertion cannot be tested");
		return;
	} else
		CHECK;

	newhosts = XListHosts(display, &nhosts, &state);
	if (!ishostinacl(host, newhosts, nhosts))
		CHECK;
	else {
		report("Host was not removed from access control list");
		FAIL;
	}

	CHECKPASS(2);

	XAddHost(display, host);
>>EXTERN

static int
ishostinacl(host, acl, nhosts)
XHostAddress	*host;
XHostAddress	*acl;
int 	nhosts;
{
int 	i;

	for (i = 0; i < nhosts; i++) {
		if (samehost(host, &acl[i]))
			return True;
	}
	return False;
}

>>ASSERTION Good B 1
.ER Access acl
>>STRATEGY
Attempt to change access control list.
If an error occurs
  If error is BadAccess
    Report Pass.
  else
	Report Fail.
else
  Report that assertion is untestable for this host.
>>CODE

>>SET no-error-status-check
	XCALL;

	if (geterr() != Success) {
		if (geterr() == BadAccess)
			PASS;
		else {
			report("Received %s error, expecting BadAccess", errorname(geterr()));
			FAIL;
		}
	} else {
		untested("This host has permission to change the list");
		report("  so this assertion cannot be tested");
	}
>>ASSERTION Good B 1
When the host address format is invalid, then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with known bad address data.
If BadAccess error:
  Report that assertion cannot be tested.
Verify that a BadValue error occurs.
>>CODE BadValue

	host = xtbadhosts;

>>SET no-error-status-check
	XCALL;

	if (geterr() == BadAccess) {
		untested("This host does not have permission to change the list");
		report("  so this assertion cannot be tested");
		return;
	} else
		CHECK;

	if (geterr() == BadValue)
		CHECK;
	else {
		report("Got %s, expecting BadValue", errorname(geterr()));
		FAIL;
	}

	CHECKPASS(2);
