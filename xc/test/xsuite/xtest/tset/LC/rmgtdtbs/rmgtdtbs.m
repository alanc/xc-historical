/* $XConsortium: rmgtdtbs.m,v 1.2 94/04/17 21:14:28 rws Exp $ */
/*

Copyright (c) 1993  X Consortium

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

 *
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the name of Sun not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun 
 * makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without any
 * express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
>>EXTERN
#include <locale.h>
#include <ximtest.h>

>>TITLE XrmGetDatabase LC 
XrmDatabase

Display	*display = Dsp;
>>SET startup rmstartup
>>ASSERTION Good A
A call to xname on 
.A display
which has a previously bound database should return the same database
which was been bound.
>>STRATEGY
Create a resource manager database, set the display database and call
XrmGetDatabase.  The database pointer should be the same one which was
just created.
>>CODE
XrmDatabase db;
XrmDatabase pdb;

	db = XrmGetStringDatabase("");
	if(db == NULL)
	{
		report("Unable to create resource database");
		FAIL;
	}
	else
	{
		XrmSetDatabase(display,db);
		pdb = XCALL;
		if(pdb == NULL)
		{
			report("%s returns NULL for name of resource database",
				TestName);
			FAIL;
		}
		else
		{
			if(db != pdb)
			{
				report("%s returns different database than was set by XrmSetDatabase",TestName);
				FAIL;
			}
			else
				CHECK;
		}
	}

	CHECKPASS(1);
	
>>ASSERTION Bad A
A call to xname on 
.A display 
which does not yet have a resource manager database associated with it 
should return NULL.
>>STRATEGY
Call XrmGetDatabase on the current display without setting the 
database of the display.  The result should be NULL. 
>>CODE
XrmDatabase db;

	db = NULL;
	XrmSetDatabase(display,db);
	db = XCALL;
	if(db != NULL)
	{
		report("%s returns a non-null resource manager database before one is set",TestName);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(1);
