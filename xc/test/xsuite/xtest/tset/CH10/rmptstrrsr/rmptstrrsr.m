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
>>TITLE XrmPutStringResource CH10
void

XrmDatabase *database = &xpsr_database;
char *specifier;
char *value;
>>SET startup rmstartup
>>EXTERN
static XrmDatabase xpsr_database;

>>INCLUDE ../rmptrsrc/common.mc
>>ASSERTION Good A
A call to xname adds a resource
.A specifier
with the specified
.A value
to the specified
.A database 
with a 
.M String
representation type.
>>STRATEGY
Create an empty test database.
Call xname to add database entries.
Call XrmGetResource to verify the database entries were added.
>>CODE
int i;

/* Create an empty test database. */
	xpsr_database = xrm_create_database("");
	if (xpsr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add database entries. */
/* Call XrmGetResource to verify the database entries were added. */
	for(i=0; i<XRM_T1_TESTS; i++) {
		specifier = t1_specifiers[i];
		value = t1_values[i];
		XCALL;
		if(xrm_check_entry(xpsr_database,
			t1_fspecs[i], t1_fclasses[i],
			"String" , t1_values[i])) {
			FAIL;
		} else
			CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(xpsr_database, "xpsr_one");
#endif

	CHECKPASS(1+XRM_T1_TESTS);

	XrmDestroyDatabase(xpsr_database);

>>ASSERTION Good A
When the 
.A database
contains an entry for the resource name specified by
.A specifier,
then a call to xname replaces the resource value in the
.A database
with
.A value ,
and the repesentation type with
.M String .
>>STRATEGY
Create an empty test database.
Call xname to add a database entry.
Call xname to replace a database entry.
Verify the database entry was updated as expected.
>>CODE
	int i;

/* Create an empty test database. */
	xpsr_database = xrm_create_database("");
	if (xpsr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add a database entry. */
/* Call xname to replace a database entry. */
	for(i=0; i<2; i++) {
		specifier = t2_specifiers[i];
		value = t2_values[i];
		trace("Adding %s: (type %s) %s", specifier, "String", t2_values[i]);
		XCALL;
		CHECK;
	}

/* Verify the database entry was updated as expected. */
	if(xrm_check_entry(xpsr_database,
		t2_fullspec, t2_fullclass,
		"String" , t2_values[1])) {
			FAIL;
			report("%s did not update the database contents as expected.",
				TestName);
		} else
			CHECK;

	CHECKPASS(4);

#ifdef TESTING
	XrmPutFileDatabase(xpsr_database, "xpsr_two");
#endif

	XrmDestroyDatabase(xpsr_database);

>>ASSERTION Good A
When
.A database
is NULL, then a call to xname
creates a new database, adds a resource
.A specifier
with the specified
.A value
to the database with a 
.M String
representation type, and returns a pointer to the database in
.A database .
>>STRATEGY
Call xname to add data to a NULL database.
Verify that the database was created, and the data was added as expected.
>>CODE

/* Call xname to add data to a NULL database. */
	xpsr_database = (XrmDatabase)NULL;
	specifier = t2_specifiers[0];
	value = t2_values[0];
	XCALL;

/* Verify that the database was created, and the data was added as expected. */
	if (xpsr_database== (XrmDatabase)NULL) {
		FAIL;
		report("%s did not create a new database when called with",
			TestName);
		report("*database=(XrmDatabase)NULL");
	} else {
		CHECK;
		if(xrm_check_entry(xpsr_database,
			t2_fullspec, t2_fullclass,
			"String" , t2_values[0])) {
				FAIL;
				report("%s did not add to the database as expected.",
					TestName);
			} else
				CHECK;
	}

	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xpsr_database, "xpsr_three");
#endif

	XrmDestroyDatabase(xpsr_database);
