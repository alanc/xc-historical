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

#define MAX_TEST_STR	4
static char *test_str_list[MAX_TEST_STR] = {
	"T",
	"Te",
	"Tes",
	"Test",
};
>>TITLE XwcTextEscapement LC 
int
XwcTextEscapement(font_set,pwctest,nwchars)
XFontSet font_set;
wchar_t *pwctest;
int nwchars;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
xname returns the escapement of a string for a given fontset.
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XwcTextEscapement to get the number of 
pixels to the origin of the next character in the primary draw direction. 
>>CODE
Display *dpy;
char *plocale;
char *fontset;
char *font_list;
XFontSet pfs;
char *defstr;
int missing_cnt;
char **missing_chars;
XFontSetExtents *ext;
int escapement,nbytes,maxescape;
wchar_t wcharstr[512];

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

				ext = XExtentsOfFontSet(pfs);
				if(ext == NULL)
				{
					report("Extents of fontset %s returns null record for locale, %s",
						font_list,plocale);
					FAIL;
				}
				else
				{
				int j;
					trace("Extents for fontset, %s, in locale %s:",
						font_list,plocale);
					trace("    Max Ink     Extents - (%d,%d) w=%d, h=%d",
						ext->max_ink_extent.x,
						ext->max_ink_extent.y,
						ext->max_ink_extent.width,
						ext->max_ink_extent.height);
					trace("    Max Logical Extents - (%d,%d) w=%d, h=%d",
						ext->max_logical_extent.x,
						ext->max_logical_extent.y,
						ext->max_logical_extent.width,
						ext->max_logical_extent.height);

					maxescape = 0;
					for(j=0;j<MAX_TEST_STR;j++)
					{
						nwchars = mbstowcs(wcharstr,test_str_list[j],128);
						pwctest = wcharstr;
		
						escapement = XCALL;
						if(escapement <= 0)
						{
							report("Illegal escapement, %d, for string %s",
								escapement,test_str_list[j]);
							FAIL;
							continue;
						}
						else
							CHECK;

						if(escapement < maxescape)
						{
						/* as long as the number of characters */
						/* is increasing, the escapment should */
						/* be increasing */
							trace("Escapement size decrease %d max=%d for longer string, %s",
								escapement,maxescape,test_str_list[j]);

							FAIL;
						}
						else
							CHECK;
						if(escapement > maxescape)
						{
							maxescape = escapement;
							trace("Escapement %d for string, %s",
								escapement,test_str_list[j]);
						}

					}
				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}
	
	CHECKPASS(nlocales()+2*MAX_TEST_STR*nlocales()*nfontset());
