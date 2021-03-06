/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: rmvhsts.m,v 1.7 94/04/17 21:06:45 rws Exp $
 */
>>TITLE XRemoveHosts CH07
void

Display	*display = Dsp;
XHostAddress	*hosts = xthosts;
int 	num_hosts = nxthosts;
>>EXTERN

#include	"xthost.h"

>>ASSERTION Good B 1
A call to xname
removes each specified host from the access control list for the
display.
>>STRATEGY
Add some hosts with XAddHosts.
Call xname to remove hosts.
If call fails with a BadAccess:
  Report that assertion can not be tested from this host.
Get new acl.
Verify that host is not in new acl.
>>CODE
XHostAddress	*oldhosts;
XHostAddress	*newhosts;
int 	nhosts;
Bool	state;

	CATCH_ERROR(display);
	XAddHosts(display, xthosts, nxthosts);
	RESTORE_ERROR(display);
	if (GET_ERROR(display) == BadAccess) {
		untested("This assertion cannot be tested from this host");
		report("  the server did not permit the access control list to be changed");
		return;
	}
	if (isdeleted())
		return;

	hosts = xthosts;
	num_hosts = nxthosts;
>>SET no-error-status-check
	XCALL;

	if (geterr() == BadAccess) {
		untested("This host does not have permission to change the list");
		report("  so this assertion cannot be tested");
		return;
	} else
		CHECK;

	newhosts = XListHosts(display, &nhosts, &state);
	if (hostsinacl(hosts, num_hosts, newhosts, nhosts) == 0)
		CHECK;
	else {
		report("Hosts were not removed from access control list");
		FAIL;
	}

	CHECKPASS(2);
>>EXTERN

/*
 * Returns the number of hosts in hosts[] that are in the acl.
 */
static int
hostsinacl(hosts, nxthosts, acl, nhosts)
XHostAddress	*hosts;
int 	nxthosts;
XHostAddress	*acl;
int 	nhosts;
{
int 	i, j;
int 	n;

	n = 0;
	for (i = 0; i < nxthosts; i++) {
		for (j = 0; j < nhosts; j++) {
			if (samehost(&hosts[i], &acl[j])) {
				n++;
				break;
			}
		}
	}
	return(n);
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

	hosts = xtbadhosts;

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
