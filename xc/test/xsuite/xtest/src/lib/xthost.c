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

/*
 * This file contains definitions of three items related to the 
 * mechanisms provided by the X server under test to add, get or remove
 * hosts from the access control list. These are only used in the tests 
 * for those Xlib functions which use or modify the access control list. 
 * 
 * The host access control functions use the XHostAddress structure. 
 * You should refer to the Xlib documentation for your system, to 
 * determine the allowed formats for host addresses in an 
 * XHostAddress structure.
 * 
 * This file should be edited if the default declarations are not correct
 * for your system. Some defaults are provided for the FamilyInternet and
 * FamilyDECnet types. Only the first has been tested by UniSoft.
 * There is no guarantee that they will work correctly even if your 
 * system supports the indicated protocol family.
 * 
 * This file should declare:
 * 1. an array xthosts[] of at least 5 XHostAddress structures containing
 *    valid family,length,address triples.
 * 2. an array xtbadhosts[] of at least 5 XHostAddress structures containing
 *    invalid family,length,address triples.
 * 3. a function samehost() that compares two XHostAddress structures
 *    and returns True if they are equivalent.
 * 
 * In general it may be necessary to declare the address parts separately
 * if it is not possible or convenient to use string notation.
 */

#include	"xtest.h"
#include	"Xlib.h"

#define	Internet

#ifdef Internet
XHostAddress xthosts[] = {
	{FamilyInternet,4,"0000"},
	{FamilyInternet,4,"ABCD"},
	{FamilyInternet,4,"1000"},
	{FamilyInternet,4,"0100"},
	{FamilyInternet,4,"0010"},
};

XHostAddress xtbadhosts[] = {
	{FamilyInternet,1,"0000"},
	{FamilyInternet,1,"ABCD"},
	{FamilyInternet,1,"1000"},
	{FamilyInternet,1,"0100"},
	{FamilyInternet,1,"0010"},
};
#endif

#ifdef VMS
/* Taken from t7, not tested by UniSoft */
XHostAddress xthosts[] = {
	{FamilyDECnet,13,"* WEEKS WEEKS"},
	{FamilyDECnet,15,"* GABBY GLADWIN"},
	{FamilyDECnet,11,"* ERIK ERIK"},
	{FamilyDECnet,15,"* GLOSTA EMORSE"},
	{FamilyDECnet,14,"* NIFTY VINSEL"}
};
XHostAddress xtbadhosts[] = {
	{FamilyInternet,13,"* WEEKS WEEKS"},
	{FamilyInternet,15,"* GABBY GLADWIN"},
	{FamilyInternet,11,"* ERIK ERIK"},
	{FamilyInternet,15,"* GLOSTA EMORSE"},
	{FamilyInternet,14,"* NIFTY VINSEL"}
};
#endif

int 	nxthosts = sizeof(xthosts)/sizeof(xthosts[0]);
int 	nxtbadhosts = sizeof(xtbadhosts)/sizeof(xtbadhosts[0]);

/*
 * Compare two XHostAddress structures for equality.  Returns True if
 * the given addresses are equivalent.
 * The supplied code will need modifying if a simple byte-for-byte
 * comparison of the address field is not the correct way to compare
 * them.
 */
samehost(h1, h2)
XHostAddress	*h1;
XHostAddress	*h2;
{
int 	i;

	if (h1->family != h2->family || h1->length != h2->length)
		return(False);

	for (i = 0; i < h1->length; i++) {
		if (h1->address[i] != h2->address[i])
			return(False);
	}
	return(True);
}
