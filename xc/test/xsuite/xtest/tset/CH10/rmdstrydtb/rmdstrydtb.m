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
>>TITLE XrmDestroyDatabase CH10
void

XrmDatabase database;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>ASSERTION Good B 1
A call to xname destroys the specified
.A database .
>>STRATEGY
Create a database.
Call xname to destroy the database.
>>CODE

/* Create a database. */
	database=xrm_create_database("test:one");
	if(database == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to destroy the database. */
	XCALL;

	CHECKUNTESTED(1);

>>ASSERTION Good B 1
When
.A database
is
.S NULL ,
then a call to xname returns immediately.
>>STRATEGY
Call xname with a NULL database.
Verify that xname returns.
(It is not possible to test that return is immediate).
>>CODE

/* Call xname with a NULL database. */
	database = (XrmDatabase)NULL;
	XCALL;

/* Verify that xname returns. */
/* (It is not possible to test that return is immediate). */
	CHECK;
	CHECKUNTESTED(1);

>>ASSERTION Good B 1
A call to xname frees the memory associated with the
.A database .
