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

/*
 * Functions to cycle through all the locales that are supported.
 */

#include <locale.h>

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include "Xresource.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"pixval.h"
#include	<string.h>
#include "ximtest.h"

Display	*Dsp;

static	int 	CurLocale = 0;
static	int	NumLocales = 0;

#define MAX_LOCALES	32

static char old_locale_str[128];
static	char locale_strs[MAX_LOCALES][MAXIDLEN];

/* Start again at the beginning of the list of locales */
void
resetlocale()
{
	char *pstr,*npstr;
	int i,nchars;
	char str[MAXLINELEN];

	CurLocale=0;
	if(NumLocales > 0)
		return;			/* already read 'em in */

	for(i=0;i<MAX_LOCALES;i++)
		locale_strs[i][0] = '\0';

	/* build an array of locales from the ximconfig struct */
	pstr = ximconfig.locale;
	if(pstr == NULL)
	{
		delete("No Locales specified");
		return;
	}

	NumLocales=0;
	while(*pstr != '\0')
	{
		npstr = pstr;
		nchars = 0;
		/* skip white space */
		while((*npstr != 0) && ((*npstr == ' ') || (*npstr == '\t'))) 
			npstr++;
		while((*npstr != '\0') && (*npstr != ';') && (*npstr != ',') &&
			  (*npstr != ' ') && (*npstr != '\t'))
		{
			npstr++;
			nchars++;
		}
		if(nchars > 0)
		{
			if(NumLocales >= MAX_LOCALES)
			{
				sprintf(str,"Too many locales (max supported is %d)",
					MAX_LOCALES);
				delete(str);
			}
			strncpy(locale_strs[NumLocales],pstr,nchars);
			locale_strs[NumLocales][nchars] = '\0';
			NumLocales++;
		}
		pstr = npstr;
		if(*npstr != '\0')
			pstr++;
	}

	/* maybe 'C' locale should be assumed? */
	if(NumLocales == 0)
		delete("No Locales found");
}

/*
 * Get the next locale 
 * Returns False if there is one, otherwise True.
 */
int
nextlocale(locale)
	char **locale;
{
	/* cycle through the list of locales from the config file. */
	if(CurLocale >= NumLocales)
		return(False);
	
	*locale = (char *)&locale_strs[CurLocale++][0]; 
	trace("--- Running test with locale %s", *locale);
	return(True);
}

/*
 * Returns the number of times that nextlocale will succeed. Only valid
 * after a call to resetlocale().
 */
int
nlocales()
{
	return(NumLocales);
}

Bool locale_set(plocale)
    char *plocale;
{
    Bool res;
    char *p;
    int fail = 0;
 
    setlocale(LC_CTYPE,plocale);
    res = XSupportsLocale();
    if(!res)
    {
        report("Locale %s not supported",plocale);
	FAIL;
        return(False);
    }
    p = XSetLocaleModifiers("");
    return(True);
}

 
/* save the original locale */
void localesave()
{
char *old_locale;
 
        old_locale_str[0] = '\0';
        old_locale = setlocale(LC_CTYPE,0);
        if(old_locale != NULL)
                strcpy(old_locale_str,old_locale);
}

void
localestartup()
{
        startup();
        if (Dsp == (Display *)0)
                return;

        localesave();
}

void
localecleanup()
{
        if (Dsp) setlocale(LC_CTYPE,old_locale_str);
 
        cleanup();
}
