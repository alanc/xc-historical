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
>>TITLE XStoreBytes CH10

XStoreBytes(display, bytes, nbytes)
Display	*display = Dsp;
char	*bytes = buff;
int	nbytes = NELEM(buff);
>>EXTERN
static char	buff[1217];
>>ASSERTION Good A
A call to xname stores
.A nbytes
bytes from the
.A bytes
argument into cut buffer zero.
>>STRATEGY
Store bytes in cut buffer 0 using xname.
Obtain the contents of cut buffer 0 using XFetchBuffer.
Verify that the buffer contents is correct.
>>CODE
char	*bp;
char	*bpr = (char *) NULL;
int	i;
int	br;

	for(i=NELEM(buff), bp = buff; i>0; *bp++ = (char) (i & 255), i--);

	XCALL;

	bp = XFetchBuffer(display, &br, 0);

	if(bp == (char *) NULL) {
		report("%s() did not set buffer 0 to contain any data.", TestName);
		FAIL;
	} else {
		CHECK;

		if(br != NELEM(buff)) {
			report("%s() set buffer 0 to contain %d bytes instead of %d.", TestName, br, NELEM(buff));
			FAIL;
		} else {
			CHECK;

			if(memcmp(bp, buff, NELEM(buff)) != 0) {
				report("%s() set buffer 0 to contain the wrong data.", TestName);
				FAIL;
			} else
				CHECK;
		}

		XFree(bp);
	}

	CHECKPASS(3);

>>ASSERTION Bad B 1
.ER BadAlloc 
