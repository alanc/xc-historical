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

>>TITLE XContextDependentDrawing LC 
Bool
XContextDependentDrawing(font_set)
XFontSet font_set;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
Given a font set, xname returns True if text drawn with the fontset,
.A font_set,
could include context-dependent drawing. 
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XContextDependentDrawing to see if the 
text can include context-dependent drawing. 
>>CODE
Display *dpy;
char *plocale;
char *font_list;
XFontSet pfs;
char *fontset;
char *defstr;
int missing_cnt;
char **missing_chars;
char *base_font_name;
Bool	res;

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
		while(nextfontset(&font_list))
		{
			pfs = XCreateFontSet(dpy,font_list,&missing_chars,
				&missing_cnt,&defstr);
			if(pfs == NULL)
			{
				report("XCreateFontSet unable to create fontset, %s",
					font_list);
				FAIL;
			}
			else
			{
				trace("Created Font Set %s", font_list);
				font_set = pfs;

				res = XCALL;
				if(res)
				{
					report("Locale %s can include context-dependent drawing for fontset %s",
						plocale,font_list);
					FAIL;
				}
				else
				{
					trace("Locale %s can NOT include context-dependent drawing for fontset %s",
						plocale,font_list);
					CHECK;
				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}
       
        CHECKPASS(nlocales()+nlocales()*nfontset());
