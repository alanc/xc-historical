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

>>TITLE XCreateFontSet LC 
XFontSet
XCreateFontSet(dpy,base_font_name_list,missing_charset_list_return,missing_charset_count_return,def_string_return)
Display *dpy;
char *base_font_name_list;
char ***missing_charset_list_return;
int *missing_charset_count_return;
char **def_string_return;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname creates a font set for the specified display.  The
font set is bound to the current locale when xname is called.
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet.  
>>CODE
Display *dpy;
char *plocale;
char *defstr;
int missing_cnt;
char **missing_chars;
XFontSet pfs;

	resetlocale();
	dpy = Dsp;
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

		/* cycle through the fontsets */
		resetfontset();
		while(nextfontset(&base_font_name_list))
		{
			def_string_return = (char **)&defstr;
			missing_charset_count_return = (int *)&missing_cnt;
			missing_charset_list_return = (char ***)&missing_chars;
			
			pfs = XCALL;
			if(pfs == NULL)
			{
				report("XCreateFontSet unable to create fontset, %s",
					base_font_name_list);
				FAIL;
			}
			else
			{
				trace("Created Font Set %s", base_font_name_list);
				trace("    default string %s",defstr);
				trace("    %d missing chars",missing_cnt);
				CHECK;
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}
       
        CHECKPASS(nlocales()+nlocales()*nfontset());
