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

>>TITLE XmbDrawString LC 
void
XmbDrawString(display,d,font_set,gc,x,y,string,length) 
Display	*display = Dsp;
Drawable d;
XFontSet font_set;
GC		gc;
int 	x = 4;
int 	y = 20;
char	*string="AbCdEfGhIjKlMnOpQrStUv";
int		length = strlen(string);
>>SET startup localestartup
>>SET cleanup localecleanup
>>EXTERN
static void
fillbuf(bp)
	char	*bp;
{
	int 	i;

	for (i = 0; i < 256; i++)
		*bp++ = i;
}
>>ASSERTION Good A
On a call to xname 
the image of each 8-bit character in the 
.A string,
as defined by the 
.M font set
is treated as an additional mask for a fill operation on the 
.A drawable .
>>STRATEGY
For each locales, for all fontsets, draw all the characters between 
0&255, by setting up strings to point to groups of characters at a time.  
Pixmap verify.  Only one Visual will be tested since XDrawString is being 
tested elsewhere.
>>EXTERN
>>CODE
char    buf[256];
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
			d = makewin(display, vp);
			gc = makegc(display, d);
			getsize(display, d, &width, &height);

			/* cycle through the fontsets */
			resetfontset();
			while(nextfontset(&font_list))
			{
			int c;
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
				else
					CHECK;

				font_set = pfs;
				for (c = 0; c < 256; )
				{
					debug(1, "Chars from %d...", c);
					for(y=20; y < height; y+=20)
					{
						if (c < 256)
						{
							string = buf+c;
							length = (256-c < 8) ? 256-c : 8;
							c += 8;
							XCALL;
						}
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

	CHECKPASS(nlocales()+(256/(((height/20)-1)*8)-1)*nlocales()*nfontset());
