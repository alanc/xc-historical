/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute Of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/


/***********************************************************************
 *
 * $XConsortium: parse.c,v 1.5 89/07/18 17:16:08 jim Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: parse.c,v 1.5 89/07/18 17:16:08 jim Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "menus.h"
#include "util.h"

#define SYSTEM_INIT_FILE "/usr/lib/X11/twm/system.twmrc"
#define BUF_LEN 300

static FILE *twmrc;
static int ptr = 0;
static int len = 0;
static char buff[BUF_LEN+1];
static char *stringSource, *currentString;
extern int yylineno;
extern int mods;

static int twmFileInput(), twmStringInput();
static void twmFileUnput(), twmStringUnput();
int (*twmInputFunc)();
void (*twmUnputFunc)();



/***********************************************************************
 *
 *  Procedure:
 *	ParseTwmrc - parse the .twmrc file
 *
 *  Inputs:
 *	filename  - the filename to parse.  A NULL indicates $HOME/.twmrc
 *
 ***********************************************************************
 */

int ParseTwmrc(filename)
    char *filename;
{
    char *home;
    char init_file[200];

    mods = 0;

/*     InitMenus(); */

    if (filename == NULL)
    {
	/* try the .twmrc.screen file first */
	home = (char *)getenv("HOME");
	sprintf(init_file, "%s/.twmrc.%d", home, Scr->screen);
	if (access(init_file, 4))
	{
	    /* try just the .twmrc file */
	    sprintf(init_file, "%s/.twmrc", home);
	    if (access(init_file, 4))
	    {
		/* try the system file */
		strcpy(init_file, SYSTEM_INIT_FILE);
	    }
	}
    }
    else
	strcpy(init_file, filename);

    if ((twmrc = fopen(init_file, "r")) == NULL)
    {
	fprintf(stderr, "twm: couldn't open \"%s\"\n", init_file);
    	return False;
    }

    ptr = 0;
    len = 0;
    yylineno = 0;
    ParseError = FALSE;
    twmInputFunc = twmFileInput;
    twmUnputFunc = twmFileUnput;

    yyparse();

    fclose(twmrc);

    if (ParseError)
    {
	fprintf(stderr, "twm: errors found in \"%s\", twm aborting\n",
	    init_file);
/*	Done(); */
	return False;
    }
    return True;
}


int ParseString (s)
    char *s;
{
    mods = 0;
    ptr = 0;
    len = 0;
    yylineno = 0;
    ParseError = FALSE;
    twmInputFunc = twmStringInput;
    twmUnputFunc = twmStringUnput;
    stringSource = currentString = s;
    
    yyparse();
    return (ParseError ? True : False);
}


/***********************************************************************
 *
 *  Procedure:
 *	twmFileInput - redefinition of the lex input routine for file input
 *
 *  Returned Value:
 *	the next input character
 *
 ***********************************************************************
 */

static int twmFileInput()
{
    while (ptr == len)
    {
	if (fgets(buff, BUF_LEN, twmrc) == NULL)
	    return NULL;

	yylineno++;

	ptr = 0;
	len = strlen(buff);
    }
    return ((int)buff[ptr++]);
}

static int twmStringInput()
{
    return (*stringSource ? *stringSource++ : *stringSource);
}


/***********************************************************************
 *
 *  Procedure:
 *	twmFileUnput - redefinition of the lex unput routine
 *
 *  Inputs:
 *	c	- the character to push back onto the input stream
 *
 ***********************************************************************
 */

static void twmFileUnput(c)
    int c;
{
    buff[--ptr] = (char) c;
}

static void twmStringUnput(c)
    int c;
{
    if (stringSource > currentString)
      *--stringSource = (char) c;
}

/***********************************************************************
 *
 *  Procedure:
 *	TwmOutput - redefinition of the lex output routine
 *
 *  Inputs:
 *	c	- the character to print
 *
 ***********************************************************************
 */

void
TwmOutput(c)
{
    putchar(c);
}
