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
>>TITLE XrmQGetSearchList CH10
Bool

XrmDatabase database = (XrmDatabase)NULL;
XrmNameList names = nl;
XrmClassList classes = cl;
XrmHashTable *list_return = sl; 	/* This is a XrmSearchList */
int list_length = XQGSL_LENGTH;
>>SET startup rmstartup
>>EXTERN

#define XQGSL_LENGTH 3
static	XrmHashTable	sl[XQGSL_LENGTH];

#define XQGSL_N_QUARK 10

static	XrmName	nl[XQGSL_N_QUARK];
static	XrmClass	cl[XQGSL_N_QUARK];

#define XQGSL_T3_ENTRIES 2
static char *t3_data[XQGSL_T3_ENTRIES] = {
	"a:one",
	"b:two" };

#define XQGSL_T4_ENTRIES 10
static char *t4_data[XQGSL_T4_ENTRIES] = {
	"a*z:one",
	"a.b*z:two",
	"a.b.c*z:three",
	"a.b.c.d*z:four",
	"a.b.c.d.e*z:five",
	"A*Z:six",
	"A.B*Z:seven",
	"A.B.C*Z:eight",
	"A.B.C.D*Z:nine",
	"A.B.C.D.E*Z:ten" };

>>ASSERTION Good B 2
A call to xname returns a list of database levels in
.A list_return
where a match for the specified
.A names
and
.A classes 
in the
.A database
could occur.
>>#
>># The problem here is that the XrmSearchList is a pointer to a
>># Xlib defined structure (it is defined in Xrm.c). Therefore, we
>># cannot perform any processing on the pointer in a portable
>># manner.
>># 
>># The essence of this assertion is tested in XrmQGetSearchResource,
>># which must call XrmQGetSearchList to build it's XrmSearchList
>># argument.
>>#
>>ASSERTION Good B 2
On a call to xname, the list returned in
.A list_return
is in best-to-worst match order.
>>#
>># The problem here is that the XrmSearchList is a pointer to a
>># Xlib defined structure (it is defined in Xrm.c). Therefore, we
>># cannot perform any processing on the pointer in a portable
>># manner.
>># 
>>ASSERTION Good A
When
.A list_length
is greater than or equal to the number of database levels 
where a match could occur,
then a call to xname returns
.S True .
>>STRATEGY
Create a test database with insufficient database levels to fill the list.
Call xname to obtain search list.
Verify that True was returned.
>>CODE
int a;
Bool ret;

/* Create a test database with insufficient database levels to fill the list. */
	for(a=0; a<XQGSL_T3_ENTRIES; a++) {
		XrmPutLineResource(&database, t3_data[a]);
	}

/* Call xname to obtain search list. */
	for(a=0; a<XQGSL_N_QUARK; a++) {
		nl[a]=(XrmName)0;
		cl[a]=(XrmClass)0;
	}
	XrmStringToNameList( "a", nl );
	XrmStringToClassList("A", cl );
	ret = XCALL;

/* Verify that True was returned. */
	if (ret != True) {
		FAIL;
		report("%s did not return True when the list_length was",
			TestName);
		report("greater than or equal to the number of");
		report("possible match database levels.");
		report("Returned value: %s", boolname(ret));
	} else
		CHECK;

	CHECKPASS(1);

	XrmDestroyDatabase(database);

>>ASSERTION Good A
When
.A list_length
is less than the number of database levels 
where a match could occur,
then a call to xname returns
.S False .
>>STRATEGY
Create a test database with sufficient database levels to fill the list.
Call xname to obtain search list.
Verify that False was returned.
>>CODE
int a;
Bool ret;

/* Create a test database with sufficient database levels to fill the list. */
	for(a=0; a<XQGSL_T4_ENTRIES; a++) {
		XrmPutLineResource(&database, t4_data[a]);
	}

/* Call xname to obtain search list. */
	for(a=0; a<XQGSL_N_QUARK; a++) {
		nl[a]=(XrmName)0;
		cl[a]=(XrmClass)0;
	}
	XrmStringToNameList( "a.b.c.d.e.f.g.h.z", nl );
	XrmStringToClassList( "A.B.C.D.E.F.G.H.Z", cl );
	ret = XCALL;

/* Verify that False was returned. */
	if (ret != False) {
		FAIL;
		report("%s did not return False when the list_length was",
			TestName);
		report("less than the number of possible match database levels.");
		report("Returned value: %s", boolname(ret));
	} else
		CHECK;

	CHECKPASS(1);

	XrmDestroyDatabase(database);
