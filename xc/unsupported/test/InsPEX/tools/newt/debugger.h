
/* $XConsortium: debugger.h,v 5.2 94/04/17 20:46:19 rws Exp $ */

/*****************************************************************

Copyright (c) 1989,1990, 1991  X Consortium

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

Copyright (c) 1989,1990, 1991 by Sun Microsystems, Inc.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
and the X Consortium, not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/



/*
 * debugger.h - header file defining externally visible
 * functions of the debugger
 */

#define MAXLINE 1000			/* maximum lines per file image */
#define MAXBIT 125			/* maximum bits to be allocated for */
				        /*     a files userbreakpts array */
#define EXECUTE_BYPASS 0		/* stepping past a function structure */
#define EXECUTE_NORMAL 1		/* stepping into a function structure */

/*
 * Use a static bit array to indicate which lines are breakable.
 * Suppose we allow a max of 1000 lines source code, we only need
 * 125 chars.
 * Use the following formula to access the correct bit:
 * (x-1)/8 --> index in char array
 * (x-1)%8 --> offset within the byte
 * where x is the line number from 1 to 1000.
 * If a line is breakable, the corresponding entry is set to 1.
 */

typedef struct {
	char userbreakpt[MAXBIT];	/* user break points array */
	char breakablept[MAXBIT];	/* breakable points array */
	char *fileimage[MAXLINE];	/* file image */
	char *name;			/* file name */
	int maxline;			/* total number of lines */
} FILESTR;

extern int N_debugger();
extern int N_debuguse();
extern int N_setflag();
extern int N_getflag();

extern int N_resumeat;                 /* the line to continue at */
extern int N_isskip;                   /* user has issued 'cont at ...' command */
extern int N_isstep;                   /* user has issued 'step' command */
extern int N_isnext;                   /* user has issued 'next' command */
extern int N_stepcount;                /* number of steps to go */
extern int N_nextcount;      	       /* number of 'next' steps to go */ 
extern FILESTR *N_fstr;		       /* current debug structure */
