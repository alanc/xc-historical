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
 * Functions to cycle through all the locale modifiers that are supported.
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include "Xresource.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"pixval.h"
#include	<string.h>
#include 	"ximtest.h"

Display	*Dsp;

static	int 	CurLclMod = 0;
static	int		NumLclMod = 0;

#define MAX_LOCALE_MODS		32

static	char *lclmod_strs[MAX_LOCALE_MODS];

/* Start again at the beginning of the list of locale modifiers */
void
resetlclmod()
{
	char *pstr,*npstr;
	int i,nchars;
	char str[MAXLINELEN];

	CurLclMod = 0;
	if(NumLclMod > 0)
		return;			/* already read 'em in */

	for(i=0;i<MAX_LOCALE_MODS;i++)
		lclmod_strs[i] = NULL;

	/* build an array of locales from the ximconfig struct */
	pstr = ximconfig.locale_modifiers;
	if(pstr == NULL)
	{
		delete("No Locale modifiers specified");
		return;
	}

	/* the locale modifiers are of the form: */
	/* @category=value@category=value... */
	/* several sets of modifiers can be specified by seperating */
	/* the modifiers by commas  ie: */
	/* @c1=v1@c2=v2,@c11=v11,@c21=v21@c22=v22@c23=v23 */
	NumLclMod =0;
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
			if(NumLclMod >= MAX_LOCALE_MODS)
			{
				sprintf(str,"Too many locale modifiers (max supported is %d)",
					MAX_LOCALE_MODS);
				delete(str);
			}
			lclmod_strs[NumLclMod] = (char *)malloc(nchars+1);
			strncpy(lclmod_strs[NumLclMod],pstr,nchars);
			lclmod_strs[NumLclMod][nchars] = '\0';
			NumLclMod++;
		}
		pstr = npstr;
		if(*npstr != '\0')
			pstr++;
	}

	/* maybe 'C' locale should be assumed? */
	if(NumLclMod == 0)
		delete("No Locales found");
}

/*
 * Get the next locale modifier 
 * Returns False if there is one, otherwise True.
 */
int
nextlclmod(lclmod)
	char **lclmod;
{
	/* cycle through the list of locale modifiers from the config file. */
	if(CurLclMod >= NumLclMod)
		return(False);
	
	*lclmod = lclmod_strs[CurLclMod++]; 
	trace("--- Running test with locale modifiers %s", *lclmod);
	return(True);
}

/*
 * Returns the number of times that nextlclmod will succeed. Only valid
 * after a call to resetlclmod().
 */
int
nlclmod()
{
	return(NumLclMod);
}
