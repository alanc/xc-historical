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

>>TITLE XExtentsOfFontSet LC 
XFontSetExtents *
XExtentsOfFontSet(font_set)
XFontSet font_set;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
xname returns the maximum extents structure of a given fontset.
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XExtentsOfFontSet to get the max ink extents
and the max lobal logical extents.  Check to make sure the extents
are always positive.
>>CODE
Display *dpy;
char *plocale;
char *font_list;
XFontSet pfs;
char *fontset;
char *defstr;
int missing_cnt;
char **missing_chars;
XFontSetExtents *ext;

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
				continue;
			}
			else
			{
				trace("Created Font Set %s", font_list);
				font_set = pfs;

				ext = XCALL;
				if(ext == NULL)
				{
					report("Extents of fontset %s returns null record for locale, %s",
						font_list,plocale);
					FAIL;
				}
				else
				{
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

					if((ext->max_ink_extent.width  <= 0) || 
					   (ext->max_ink_extent.height <= 0)) 
					{
						report("Bad ink extents of fontset, %s in locale, %s",
							font_list,plocale);
							
						FAIL;
					}
					else
						CHECK;
					if((ext->max_logical_extent.width  <= 0) || 
					   (ext->max_logical_extent.height <= 0)) 
					{
						report("Bad logical extents of fontset, %s in locale, %s",
							font_list,plocale);
						FAIL;
					}
					else
						CHECK;

				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}

        CHECKPASS(nlocales()+2*nlocales()*nfontset());
