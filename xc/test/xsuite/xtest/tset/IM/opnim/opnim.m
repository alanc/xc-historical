/* $XConsortium$ */
/*
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the names of Sun or MIT not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun and
 * M.I.T. make no representations about the suitability of this
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

>>TITLE XOpenIM	IM	
XIM

Display *display = Dsp;
XrmDatabase database;
char *res_name = NULL;
char *res_class = NULL;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname opens an input method, matching the current locale and 
modifiers specification.  Current locale and modifiers are bound to the 
input method at opening time.
.A db
is NULL, no database is passed to the input method. 
>>STRATEGY
For all locales, create an resource database and open an input method 
and check the return value for non-null. 
>>CODE
char *plocale;
XIM xim = NULL;

	XrmInitialize();

	resetlocale();
	while(nextlocale(&plocale))
	{

		if (locale_set(plocale))
			CHECK;
		else
		{
			report("Couldn't set locale.");
			FAIL;
			continue;
		}

		cleanup_locale(NULL,NULL,xim,database);

		database = rm_db_open();
		if(database != NULL)
			CHECK;
		else
		{
			report("Couldn't open database.");
			FAIL;
			continue;
		}

		/* open an input method */
		xim = XCALL;
		if(xim == NULL)
		{
			report("Unable to open an input method for locale, %s",plocale);
			XrmDestroyDatabase(database);
			FAIL;
			continue;
		}
		else
			CHECK;
	}    /* nextlocale */
	cleanup_locale(NULL,NULL,xim,database);

	CHECKPASS(3*nlocales());
