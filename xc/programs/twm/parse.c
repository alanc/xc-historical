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
 * $XConsortium: parse.c,v 1.6 89/07/27 13:59:39 jim Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: parse.c,v 1.6 89/07/27 13:59:39 jim Exp $";
#endif

#include <stdio.h>
#include <X11/Xos.h>
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

extern char *getenv();
extern char *defaultTwmrc;		/* default bindings */


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
    char *cp = filename;
    char init_file[257];

    if (!cp) {
	char *home = getenv ("HOME");
	if (home) {
	    int len = strlen (home);

	    cp = init_file;
	    sprintf (init_file, "%s/.twmrc.%d", home, Scr->screen);
	    
	    if (access (init_file, R_OK) != 0) {
		init_file[len + 7] = '\0';
		if (access (init_file, R_OK) != 0) {
		    cp = NULL;
		}
	    }
	}
	if (!cp) cp = SYSTEM_INIT_FILE;
    }

    InitMenus();
    mods = 0;

    if (!(twmrc = fopen (cp, "r"))) {
	if (filename) {
	    fprintf (stderr, 
		     "%s:  unable to open twmrc file \"%s\"; using defaults\n",
		     "twm", filename);
	}
	ParseString (defaultTwmrc);	/* use default bindings */
	return 0;
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
	fprintf (stderr, "twm:  errors found in \"%s\"\n", cp);
	return 0;
    }
    return 1;
}


int ParseString (s)
    char *s;
{
    mods = 0;
    ptr = 0;
    len = 0;
    yylineno = 1;
    ParseError = FALSE;
    twmInputFunc = twmStringInput;
    twmUnputFunc = twmStringUnput;
    stringSource = currentString = s;
    
    yyparse();
    return (ParseError ? 0 : 1);
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
    unsigned int c = (unsigned int) *stringSource;

    if (c != 0) {
	if (c == '\n') yylineno++;
	stringSource++;
    }
    return (int) c;
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
