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

>>TITLE XDestroyIC IM
void

XIC ic;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname destroys the input context,
.A ic,
to the input method server.
>>STRATEGY
There is not a good way to test XDestroyIC.  We rely on capturing any 
errors from the server during calls to XDestroyIC.  Loop through all
the input method styles for all the locales, creating and destroying
Input Contexts.
>>CODE
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
Window win;
XFontSet fs = NULL;
int nstyles = 0;
XIMStyle which_style;

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

		cleanup_locale(NULL,fs,im,db);

		db = rm_db_open();
		if(db != NULL)
			CHECK;
		else
		{
			report("Couldn't open database.");
			FAIL;
			continue;
		}

		im = im_open(db);
		if(im != NULL)
			CHECK;
		else
		{
			report("Couldn't open imput method.");
			FAIL;
			continue;
		}

		if(ic_setup(&win,&fs))
			CHECK;
		else
		{
			report("Couldn't setup input styles.");
			FAIL;
			continue;
		}

		/* get the input styles */
		reset_ic_style(im);
		nstyles += n_ic_styles();
		while(next_ic_style(&which_style))
		{
			ic = ic_open(im,win,which_style);
			if(ic != NULL)
				CHECK;
			else
			{
				report("Unable to create input context for locale, %s",
					plocale);
				FAIL;
				continue;
			}

			/* destroy the IC */
			XCALL;
		}
	}   /* nextlocale */
	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+nstyles);
