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
>>TITLE XrmPutFileDatabase CH10
void

XrmDatabase database = (XrmDatabase)NULL;
char *stored_db = "xpfd_file";
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN

#define XPFD_T1_COUNT	4
static char *t1_data[XPFD_T1_COUNT][3] = {
	{ "a.b.c",	"ONE",	"a.b.c" },
	{ "D.E.F",	"TWO",	"D.E.F"},
	{ "*Z",	"THREE",	"A.Z"	},
	{ ".e.f.g.h.i.j",	"*!++ X&",	"e.f.g.h.i.j"	} };

>>ASSERTION Good A
A call to xname stores the resource name and value pairs 
in the specified file
.A stored_db 
from the specified
.A database
in ResourceLine format.
>>STRATEGY
Create a new database containing the test information.
Call xname to write the database.
Call XrmGetFileDatabase to check the database was written out.
Check the retrieved database contents were as expected.
Remove created file.
>>CODE
int a;
XrmDatabase rdb;

/* Create a new database containing the test information. */
	for(a=0; a<XPFD_T1_COUNT; a++) {
		XrmPutStringResource(&database, t1_data[a][0], t1_data[a][1]);
		CHECK;
	}

/* Call xname to write the database. */
	unlink( stored_db );
	XCALL;

/* Call XrmGetFileDatabase to check the database was written out. */
	rdb = XrmGetFileDatabase( stored_db );
	if (rdb == (XrmDatabase)NULL) {
		FAIL;
		delete("XrmGetFileDatabase could not open the written database.");
		return;
	} else {
/* Check the retrieved database contents were as expected. */
		for(a=0; a<XPFD_T1_COUNT; a++) {
			if (xrm_check_entry(rdb, t1_data[a][2], t1_data[a][2],
				"String", t1_data[a][1])) {
				delete("Unexpected data item returned from read in database");
				report("%s may have failed.", TestName);
			} else
				CHECK;
		}
	}

	CHECKPASS(XPFD_T1_COUNT + XPFD_T1_COUNT);

/* Remove created file. */
#ifndef TESTING
	unlink( stored_db ); 	/* To examine test file, */
				/* use pmake CFLOCAL=-DTESTING */
#endif
