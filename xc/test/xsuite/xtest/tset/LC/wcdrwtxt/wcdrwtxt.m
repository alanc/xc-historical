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

>>TITLE XwcDrawText LC 
void
XwcDrawText(display,d,gc,x,y,items,nitems) 
Display	*display = Dsp;
Drawable d;
GC		gc;
int 	x = 4;
int 	y = 20;
XwcTextItem	*items;
int 	nitems = 0;
>>EXTERN
static void
fillbuf(bp)
	char	*bp;
{
	int 	i;

	for (i = 0; i < 256; i++)
		*bp++ = i;
}
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
On a call to xname each of the text
.A items ,
specifying a string
.M chars
of 8-bit characters
from a
.M font
with interstring spacing given by
.M delta ,
is drawn in turn.
>>STRATEGY
For all locales, for all fontsets, draw all the 
characters between 0&255 in all the xtest fonts, by setting
up XTestItem structs to point to groups of characters at a time.
Pixmap verify.  Only one visual will be tested, since XDrawText is
being tested elsewhere and XwcDrawText eventually calls the same
routines as XDrawText.    
>>EXTERN
#define	T1_NITEMS 3
#define	T1_GROUPSIZE 3
>>CODE
Display *dpy;
char *plocale;
XVisualInfo     *vp;
unsigned int    width, height;
char *font_list;
XFontSet pfs;
char *fontset;
char *defstr;
int missing_cnt;
char **missing_chars;
char  buf[256];
int   delta;
char *charstr;
wchar_t wcharstr[T1_NITEMS][T1_GROUPSIZE];
XwcTextItem ti[T1_NITEMS];

	fillbuf(buf);

	dpy = Dsp;
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

		if (!linklocale(plocale))
		{
			untested("Couldn't create data link.");
			FAIL;
			continue;
		}

		resetvinf(VI_WIN_PIX); 
		if(nextvinf(&vp))
		{
		int i, c;

			d = makewin(display, vp);
			gc = makegc(display, d);
			getsize(display, d, &width, &height);

			/* cycle through the fontsets */
			resetfontset();
			while(nextfontset(&font_list))
			{
				trace("Font Set %s", font_list);
				pfs = XCreateFontSet(dpy,font_list,&missing_chars,
					&missing_cnt,&defstr);
				if(pfs == NULL)
				{
					report("XCreateFontSet unable to create fontset, %s",
						font_list);
					FAIL;
					continue;
				}

				for (i = 0; i < T1_NITEMS; i++)
					ti[i].font_set = None;

				items = ti;
				nitems = T1_NITEMS;
				delta = 0;

				ti[0].font_set = pfs;
				for (c = 0; c < 256; )
				{
					debug(1, "Chars from %d...", c);
					for(y=20; y < height; y+=20)
					{
						for (i = 0; i < T1_GROUPSIZE; i++)
						{
							if (c < 256)
							{
								charstr = buf+c;
								ti[i].nchars = (256-c<=T1_GROUPSIZE)? 256-c: T1_GROUPSIZE;
								c += T1_GROUPSIZE;
								ti[i].delta = delta;
								if (delta++ >= 7)
									delta = -2;
							}
							mbstowcs(wcharstr[i],charstr,T1_GROUPSIZE);
							ti[i].chars = wcharstr[i];
						}
						XCALL;
					}
					debug(1, "..to char %d", c);
					PIXCHECK(display, d);
					dclear(display, d);
				}	/* for c */
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}	/* nextvinf */
		}	/* nextfontset */
	}	/* nextlocale */
	unlinklocales();

	CHECKPASS(nlocales()+(256/(((height/20)-1)*T1_GROUPSIZE*T1_GROUPSIZE)-1)*nlocales()*nfontset());
