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
>>TITLE XStoreBuffer CH10

XStoreBuffer(display, bytes, nbytes, buffer)
Display		*display = Dsp;
char		*bytes;
int		nbytes;
int		buffer;
>>ASSERTION Good A
A call to xname stores
.A nbytes
bytes from the
.A bytes
argument into the cut buffer specified by the
.A buffer
argument.
>>STRATEGY
For each cut buffer 0 to 7:
   Store different data into each buffer using xname.
For each cut buffer 0 to 7:
   Obtain the contents of the buffer using XFetchBuffer.
   Verify that the data is correct.
>>CODE
char	*bp;
char	*rbp;
int	nr;
int	len;
int 	i;
int 	j;
struct	bstrct {
	char	*data;
	int	len;
}	bfrs[8], *bptr;

	for(i=0, bptr=bfrs; i<8; i++, bptr++) {

		len = 1+i*123;
		bptr->len = len;

		if((bptr->data = (char*)malloc(len)) == (char *) NULL) {
			delete("malloc() returned NULL.");
			return;
		} else
			CHECK;

		for(j=len, bp=bptr->data; j>0; *bp++ = (j) %  (256 - i), j--);

		bytes = bptr->data;
		nbytes = len;
		buffer = i;
		XCALL;
	}

	for(i=0, bptr=bfrs; i<8; i++, bptr++) {
		
		rbp = XFetchBuffer(display, &nr, i);

		if(rbp == (char *) NULL) {
			report("%s() did not set buffer %d to contain any data.", TestName, i);
			FAIL;
		} else {
			CHECK;

			if(bptr->len != nr) {
				report("%s() set buffer %d to contain %d bytes instead of %d.", TestName, i, nr, bptr->len);
				FAIL;
			} else {
				CHECK;

				if(memcmp(rbp, bptr->data, nr) != 0) {
					report("%s() set buffer %d to contain the wrong data.", TestName, i);
					FAIL;
				} else
					CHECK;
			}

			free(bptr->data);
			XFree(rbp);
		}
	}


	CHECKPASS(8 + 8 * (3));

>>ASSERTION Bad B 1
.ER BadAlloc 
