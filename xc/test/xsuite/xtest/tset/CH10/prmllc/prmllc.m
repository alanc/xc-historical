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
>>TITLE Xpermalloc CH10
char *

unsigned int size;
>>ASSERTION Good A
A call to xname returns a pointer to a block of memory
.A size
bytes large.
>>STRATEGY
Call xname to allocate a block of memory.
Verify that all addresses within the block may be accessed.
>>CODE
int i, badaccess;
char *ret;

/* Call xname to allocate a block of memory. */
	size = 2048;
	ret = XCALL;

	if( ret == (char *)NULL ) {
		delete("%s returned a NULL pointer.", TestName);
		report("Expecting a pointer to a block of %u bytes", size);
	} else
		CHECK;

/* Verify that all addresses within the block may be accessed. */
	for(i=0; i<size; i++) {
		*(ret+i)='s';	/* A SIGSEGV indicates t r o u b l e */
		CHECK;
	}

	badaccess=0;
	for(i=0; i<size; i++) {
		if( *(ret+i) != 's' ) {
			badaccess++;
		} else
			CHECK;
	}

	if( badaccess!=0 ) {
		FAIL;
		report("%s did not return a writable block of memory.",
			TestName);
		report("%d addresses within the allocated block could not be written to.", badaccess);
	} else
		CHECK;

	CHECKPASS(2+size*2);

>>ASSERTION Bad B 1
When sufficient temporary storage cannot be allocated, then a call to
xname returns
.S NULL .
