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
>>TITLE XrmGetResource CH10
Bool

XrmDatabase database = (XrmDatabase)NULL;
char *str_name;
char *str_class;
char **str_type_return = &str_type;
XrmValue *value_return = &value;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>INCLUDE common.mc
>>EXTERN
char *str_type;
XrmValue value;

>>ASSERTION Good A
When
.A str_name
is a fully qualified resource name and
.A str_class
is a fully qualified resource class
and the
.A database
contains one or more corresponding resources,
then a call to xname places
the representation type as a string in
.A str_type_return 
and the value in
.A value_return
of the closest matching resource as defined by the matching rules,
and returns 
.S True .
>>STRATEGY
Create a database containing test information.
Interrogate database using xname.
Verify that the returned type and value were as expected.
>>CODE
int a;
XrmValue b;
Bool ret;

/* Create a database containing test information. */
	database = xrm_create_database("");
	if (database == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

	for(a=0; a<XGR_T1_DATA; a++) {
		xrm_fill_value(&b, t1_data[a][2]);
		XrmPutResource(&database, t1_data[a][0], t1_data[a][1], &b );
		CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(database, "xgr_one");
#endif

/* Interrogate database using xname. */
/* Verify that the returned type and value were as expected. */
	for(a=0; a<XGR_T1_TEST; a++) {
		str_name = t1_test[a][0];
		str_class = t1_test[a][1];
		trace("Testing: %s (class %s)", str_name, str_class);
		str_type = (char *)NULL;
		value.addr = (caddr_t)NULL;
		ret = XCALL;
		if (ret==False) {
			FAIL;
			report("%s failed to match a database entry.",
				TestName);
			report("name :%s", t1_test[a][0]);
			report("class:%s", t1_test[a][1]);
			report("Possible diagnosis: %s", t1_test[a][4]);
		} else {
			CHECK;

			if (str_type == NULL || strcmp(t1_test[a][2], str_type)) {
				FAIL;
				report("%s failed to return expected type.",
					TestName);
				report("Expected type: '%s'", t1_test[a][2]);
				report("Returned type: '%s'",
					(str_type==NULL?"<NULL POINTER>":str_type));
			} else
				CHECK;


			if (value.addr == NULL
				|| strncmp(t1_test[a][3], (char *)value.addr, strlen(t1_test[a][3]))){
				char tb[100];
				unsigned int l;
				if (value.addr == NULL) {
					strcpy(tb, "<NULL POINTER>");
					l = strlen(tb);
				} else {
					(void) strncpy(tb, (char*)value.addr, value.size);
					tb[value.size]='\0';
					l = value.size;
				}
				FAIL;
				report("%s failed to return expected value.",
					TestName);
				report("Expected value: '%s'", t1_test[a][3]);
				report("Returned value: '%.*s'", l, tb);
				report("Possible diagnosis: %s", t1_test[a][4]);
			} else
				CHECK;
		}
	}

	CHECKPASS(1 + XGR_T1_DATA + 3*XGR_T1_TEST);
>>ASSERTION Good A
When
.A str_name
is a fully qualified resource name and
.A str_class
is a fully qualified resource class
and the
.A database 
does not contain a corresponding resource as defined by the matching rules,
then a call to xname returns
.A False .
>>STRATEGY
Create a database containing test information.
Interrogate database using xname.
Verify that the test examples were not found.
>>CODE
int a;
XrmValue b;
Bool ret;

/* Create a database containing test information. */
	database = xrm_create_database("");
	if (database == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

	for(a=0; a<XGR_T1_DATA; a++) {
		xrm_fill_value(&b, t1_data[a][2]);
		XrmPutResource(&database, t1_data[a][0], t1_data[a][1], &b );
		CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(database, "xgr_two");
#endif

/* Interrogate database using xname. */
/* Verify that the test examples were not found. */
	for(a=0; a<XGR_T2_TEST; a++) {
		str_name = t2_test[a][0];
		str_class = t2_test[a][1];
		trace("Testing: %s (class %s)", str_name, str_class);
		str_type = (char *)NULL;
		value.addr = (caddr_t)NULL;
		ret = XCALL;
		if (ret==False) {
			CHECK;
		} else {
			char tb[100];
			unsigned int l;

			FAIL;
			report("%s returned a database match when a failure was expected.",
				TestName);
			report("Returned type: '%s'",
					(str_type==NULL?"<NULL POINTER>":str_type));
			if (value.addr == NULL) {
				strcpy(tb, "<NULL POINTER>");
				l = strlen(tb);
			} else {
				(void) strncpy(tb, (char*)value.addr, value.size);
				tb[value.size]='\0';
				l = value.size;
			}
			report("Returned value: '%.*s'", l, tb);
			report("Possible diagnosis: %s", t2_test[a][3]);
		}
	}

	CHECKPASS(1 + XGR_T1_DATA + XGR_T2_TEST);
