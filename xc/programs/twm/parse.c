/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
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
 * $XConsortium: parse.c,v 1.2 89/11/13 08:56:48 jim Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: parse.c,v 1.2 89/11/13 08:56:48 jim Exp $";
#endif

#include <stdio.h>
#include <X11/Xos.h>
#include "twm.h"
#include "screen.h"
#include "menus.h"
#include "util.h"
#include "gram.h"

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


/**********************************************************************
 *
 *  Parsing table and routines
 * 
 ***********************************************************************/

typedef struct _TwmKeyword {
    char *name;
    int value;
} TwmKeyword;

/*
 * The other tokens are already defined:
 * %token <num> BUTTON FRAME TITLE ICONMGR ICON 
 */
#ifndef KILL
#define KILL CUR_KILL
#define MOVE CUR_MOVE
#define RESIZE CUR_RESIZE
#define SELECT CUR_SELECT
#define WAIT CUR_WAIT
#endif

/*
 * The following is sorted alphabetically according to name (which must be
 * in lowercase and only contain the letters a-z).  It is fed to a binary
 * search to parse keywords.
 */
static TwmKeyword keytable[] = { 
    { "all",			ALL },
    { "autoraise",		AUTO_RAISE },
    { "autorelativeresize",	AUTO_RELATIVE_RESIZE },
    { "bordercolor",		BORDER_COLOR },
    { "bordertilebackground",	BORDER_TILE_BACKGROUND },
    { "bordertileforeground",	BORDER_TILE_FOREGROUND },
    { "borderwidth",		BORDERWIDTH },
    { "button",			BUTTON },
    { "buttonindent",		BUTTON_INDENT },
    { "c",			CONTROL },
    { "clientborderwidth",	CLIENT_BORDERWIDTH },
    { "color",			COLOR },
    { "constrainedmovetime",	CONSTRAINED_MOVE_TIME },
    { "cursors",		CURSORS },
    { "decoratetransients",	DECORATE_TRANSIENTS },
    { "defaultbackground",	DEFAULT_BACKGROUND },
    { "defaultforeground",	DEFAULT_FOREGROUND },
    { "defaultfunction",	DEFAULT_FUNCTION },
    { "destroy",		KILL },
    { "donticonifybyunmapping",	DONT_ICONIFY_BY_UNMAPPING },
    { "dontmoveoff",		DONT_MOVE_OFF },
    { "east",			EAST },
    { "f",			FRAME },
    { "f.autoraise",		F_AUTORAISE },
    { "f.backiconmgr",		F_BACKICONMGR },
    { "f.beep",			F_BEEP },
    { "f.bottomzoom",		F_BOTTOMZOOM },
    { "f.circledown",		F_CIRCLEDOWN },
    { "f.circleup",		F_CIRCLEUP },
    { "f.colormap",		F_COLORMAP },
    { "f.cutfile",		F_CUTFILE },
    { "f.deiconify",		F_DEICONIFY },
    { "f.delete",		F_DELETE },
    { "f.deltastop",		F_DELTASTOP },
    { "f.destroy",		F_DESTROY },
    { "f.downiconmgr",		F_DOWNICONMGR },
    { "f.file",			F_FILE },
    { "f.focus",		F_FOCUS },
    { "f.forcemove",		F_FORCEMOVE },
    { "f.forwiconmgr",		F_FORWICONMGR },
    { "f.fullzoom",		F_FULLZOOM },
    { "f.function",		F_FUNCTION },
    { "f.hbzoom",		F_BOTTOMZOOM },
    { "f.hideiconmgr",		F_HIDELIST },
    { "f.horizoom",		F_HORIZOOM },
    { "f.htzoom",		F_TOPZOOM },
    { "f.hzoom",		F_HORIZOOM },
    { "f.iconify",		F_ICONIFY },
    { "f.identify",		F_IDENTIFY },
    { "f.lefticonmgr",		F_LEFTICONMGR },
    { "f.leftzoom",		F_LEFTZOOM },
    { "f.lower",		F_LOWER },
    { "f.menu",			F_MENU },
    { "f.move",			F_MOVE },
    { "f.nexticonmgr",		F_NEXTICONMGR },
    { "f.nop",			F_NOP },
    { "f.previconmgr",		F_PREVICONMGR },
    { "f.quit",			F_QUIT },
    { "f.raise",		F_RAISE },
    { "f.raiselower",		F_RAISELOWER },
    { "f.refresh",		F_REFRESH },
    { "f.resize",		F_RESIZE },
    { "f.restart",		F_RESTART },
    { "f.righticonmgr",		F_RIGHTICONMGR },
    { "f.rightzoom",		F_RIGHTZOOM },
    { "f.saveyourself",		F_SAVEYOURSELF },
    { "f.showiconmgr",		F_SHOWLIST },
    { "f.sorticonmgr",		F_SORTICONMGR },
    { "f.source",		F_SOURCE },
    { "f.title",		F_TITLE },
    { "f.topzoom",		F_TOPZOOM },
    { "f.twmrc",		F_TWMRC },
    { "f.unfocus",		F_UNFOCUS },
    { "f.upiconmgr",		F_UPICONMGR },
    { "f.version",		F_VERSION },
    { "f.vlzoom",		F_LEFTZOOM },
    { "f.vrzoom",		F_RIGHTZOOM },
    { "f.warpto",		F_WARPTO },
    { "f.warptoiconmgr",	F_WARPTOICONMGR },
    { "f.warptoscreen",		F_WARPTOSCREEN },
    { "f.winrefresh",		F_WINREFRESH },
    { "f.zoom",			F_ZOOM },
    { "forceicons",		FORCE_ICON },
    { "frame",			FRAME },
    { "framepadding",		FRAME_PADDING },
    { "function",		FUNCTION },
    { "i",			ICON },
    { "icon",			ICON },
    { "iconbackground",		ICON_BACKGROUND },
    { "iconbordercolor",	ICON_BORDER_COLOR },
    { "iconborderwidth",	ICON_BORDERWIDTH },
    { "icondirectory",		ICON_DIRECTORY },
    { "iconfont",		ICON_FONT },
    { "iconforeground",		ICON_FOREGROUND },
    { "iconifybyunmapping",	ICONIFY_BY_UNMAPPING },
    { "iconmanagerbackground",	ICONMGR_BACKGROUND },
    { "iconmanagerdontshow",	ICONMGR_NOSHOW },
    { "iconmanagerfont",	ICONMGR_FONT },
    { "iconmanagerforeground",	ICONMGR_FOREGROUND },
    { "iconmanagergeometry",	ICONMGR_GEOMETRY },
    { "iconmanagerhighlight",	ICONMGR_HIGHLIGHT },
    { "iconmanagers",		ICONMGRS },
    { "iconmanagershow",	ICONMGR_SHOW },
    { "iconmgr",		ICONMGR },
    { "iconregion",		ICON_REGION },
    { "icons",			ICONS },
    { "interpolatemenucolors",	INTERPOLATE_MENUS },
    { "lefttitlebutton",	LEFT_TITLEBUTTON },
    { "m",			META },
    { "maketitle",		MAKE_TITLE },
    { "menu",			MENU },
    { "menubackground",		MENU_BACKGROUND },
    { "menufont",		MENU_FONT },
    { "menuforeground",		MENU_FOREGROUND },
    { "menushadowcolor",	MENU_SHADOW_COLOR },
    { "menutitlebackground",	MENU_TITLE_BACKGROUND },
    { "menutitleforeground",	MENU_TITLE_FOREGROUND },
    { "monochrome",		MONOCHROME },
    { "move",			MOVE },
    { "movedelta",		MOVE_DELTA },
    { "nobackingstore",		NO_BACKINGSTORE },
    { "nodefaults",		NODEFAULTS },
    { "nograbserver",		NO_GRAB_SERVER },
    { "nohighlight",		NO_HILITE },
    { "noiconmanagers",		NO_ICONMGRS },
    { "nomenushadows",		NO_MENU_SHADOWS },
    { "noraiseondeiconify",	NO_RAISE_ON_DEICONIFY },
    { "noraiseonmove",		NO_RAISE_ON_MOVE },
    { "noraiseonresize",	NO_RAISE_ON_RESIZE },
    { "north",			NORTH },
    { "nosaveunders",		NO_SAVEUNDER },
    { "noshadowmenus",		NO_MENU_SHADOWS },
    { "notitle",		NO_TITLE },
    { "notitlefocus",		NO_TITLE_FOCUS },
    { "notitlehighlight",	NO_TITLE_HILITE },
    { "noversion",		NO_VERSION },
    { "opaquemove",		OPAQUE_MOVE },
    { "pixmaps",		PIXMAPS },
    { "r",			ROOT },
    { "randomplacement",	RANDOM_PLACEMENT },
    { "resize",			RESIZE },
    { "resizefont",		RESIZE_FONT },
    { "restartpreviousstate",	RESTART_PREVIOUS_STATE },
    { "righttitlebutton",	RIGHT_TITLEBUTTON },
    { "root",			ROOT },
    { "s",			SHIFT },
    { "select",			SELECT },
    { "showiconmanager",	SHOW_ICONMGR },
    { "sorticonmanager",	SORT_ICONMGR },
    { "south",			SOUTH },
    { "squeezetitle",		SQUEEZETITLE },
    { "starticonified",		START_ICONIFIED },
    { "t",			TITLE },
    { "t.lower",		F_LOWER },
    { "t.move",			F_MOVE },
    { "t.nop",			F_NOP },
    { "t.raise",		F_RAISE },
    { "title",			TITLE },
    { "titlebackground",	TITLE_BACKGROUND },
    { "titlebuttonborderwidth",	TITLEBUTTON_BORDERWIDTH },
    { "titlefont",		TITLE_FONT },
    { "titleforeground",	TITLE_FOREGROUND },
    { "titlehighlight",		TITLE_HILITE },
    { "titlepadding",		TITLE_PADDING },
    { "unknownicon",		UNKNOWN_ICON },
    { "usepposition",		USE_PPOSITION },
    { "w",			WINDOW },
    { "wait",			WAIT },
    { "warpcursor",		WARPCURSOR },
    { "west",			WEST },
    { "window",			WINDOW },
    { "windowfunction",		WINDOW_FUNCTION },
    { "xorvalue",		XOR_VALUE },
    { "zoom",			ZOOM },
};

static int numkeywords = (sizeof(keytable)/sizeof(keytable[0]));

int parse_keyword (s)
    char *s;
{
    register int lower = 0, upper = numkeywords - 1;

    XmuCopyISOLatin1Lowered (s, s);
    while (lower <= upper) {
        int middle = (lower + upper) / 2;
	TwmKeyword *p = &keytable[middle];
        int res = strcmp (p->name, s);

        if (res < 0) {
            lower = middle + 1;
        } else if (res == 0) {
            return p->value;
        } else {
            upper = middle - 1;
        }
    }
    return -1;
}
