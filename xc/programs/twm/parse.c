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
 * $XConsortium: parse.c,v 1.10 89/11/13 15:16:26 jim Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$XConsortium: parse.c,v 1.10 89/11/13 15:16:26 jim Exp $";
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
static int ParseUsePPosition();

extern int yylineno;
extern int mods;

int ConstrainedMoveTime = 400;		/* milliseconds, event times */

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
    int subnum;
} TwmKeyword;

#define kw0_NoDefaults			1
#define kw0_AutoRelativeResize		2
#define kw0_ForceIcons			3
#define kw0_NoIconManagers		4
#define kw0_OpaqueMove			5
#define kw0_InterpolateMenuColors	6
#define kw0_WarpCursor			7
#define kw0_NoVersion			8
#define kw0_SortIconManager		9
#define kw0_NoGrabServer		10
#define kw0_NoMenuShadows		11
#define kw0_NoRaiseOnMove		12
#define kw0_NoRaiseOnResize		13
#define kw0_NoRaiseOnDeiconify		14
#define kw0_DontMoveOff			15
#define kw0_NoBackingStore		16
#define kw0_NoSaveUnders		17
#define kw0_RestartPreviousState	18
#define kw0_ClientBorderWidth		19
#define kw0_NoTitleFocus		20
#define kw0_RandomPlacement		21
#define kw0_DecorateTransients		22
#define kw0_SqueezeTitle		23
#define kw0_ShowIconManager		24

#define kws_UsePPosition		1
#define kws_IconFont			2
#define kws_ResizeFont			3
#define kws_MenuFont			4
#define kws_TitleFont			5
#define kws_IconManagerFont		6
#define kws_UnknownIcon			7
#define kws_IconDirectory		8

#define kwn_ConstrainedMoveTime		1
#define kwn_MoveDelta			2
#define kwn_XorValue			3
#define kwn_FramePadding		4
#define kwn_TitlePadding		5
#define kwn_ButtonIndent		6
#define kwn_BorderWidth			7
#define kwn_IconBorderWidth		8
#define kwn_TitleButtonBorderWidth	9

/*
 * The following is sorted alphabetically according to name (which must be
 * in lowercase and only contain the letters a-z).  It is fed to a binary
 * search to parse keywords.
 */
static TwmKeyword keytable[] = { 
    { "all",			ALL, 0 },
    { "autoraise",		AUTO_RAISE, 0 },
    { "autorelativeresize",	KEYWORD, kw0_AutoRelativeResize },
    { "bordercolor",		BORDER_COLOR, 0 },
    { "bordertilebackground",	BORDER_TILE_BACKGROUND, 0 },
    { "bordertileforeground",	BORDER_TILE_FOREGROUND, 0 },
    { "borderwidth",		NKEYWORD, kwn_BorderWidth },
    { "button",			BUTTON, 0 },
    { "buttonindent",		NKEYWORD, kwn_ButtonIndent },
    { "c",			CONTROL, 0 },
    { "clientborderwidth",	KEYWORD, kw0_ClientBorderWidth },
    { "color",			COLOR, 0 },
    { "constrainedmovetime",	NKEYWORD, kwn_ConstrainedMoveTime },
    { "cursors",		CURSORS, 0 },
    { "decoratetransients",	KEYWORD, kw0_DecorateTransients },
    { "defaultbackground",	DEFAULT_BACKGROUND, 0 },
    { "defaultforeground",	DEFAULT_FOREGROUND, 0 },
    { "defaultfunction",	DEFAULT_FUNCTION, 0 },
    { "destroy",		KILL, 0 },
    { "donticonifybyunmapping",	DONT_ICONIFY_BY_UNMAPPING, 0 },
    { "dontmoveoff",		KEYWORD, kw0_DontMoveOff },
    { "east",			EAST, 0 },
    { "f",			FRAME, 0 },
    { "f.autoraise",		F_AUTORAISE, 0 },
    { "f.backiconmgr",		F_BACKICONMGR, 0 },
    { "f.beep",			F_BEEP, 0 },
    { "f.bottomzoom",		F_BOTTOMZOOM, 0 },
    { "f.circledown",		F_CIRCLEDOWN, 0 },
    { "f.circleup",		F_CIRCLEUP, 0 },
    { "f.colormap",		F_COLORMAP, 0 },
    { "f.cutfile",		F_CUTFILE, 0 },
    { "f.deiconify",		F_DEICONIFY, 0 },
    { "f.delete",		F_DELETE, 0 },
    { "f.deltastop",		F_DELTASTOP, 0 },
    { "f.destroy",		F_DESTROY, 0 },
    { "f.downiconmgr",		F_DOWNICONMGR, 0 },
    { "f.file",			F_FILE, 0 },
    { "f.focus",		F_FOCUS, 0 },
    { "f.forcemove",		F_FORCEMOVE, 0 },
    { "f.forwiconmgr",		F_FORWICONMGR, 0 },
    { "f.fullzoom",		F_FULLZOOM, 0 },
    { "f.function",		F_FUNCTION, 0 },
    { "f.hbzoom",		F_BOTTOMZOOM, 0 },
    { "f.hideiconmgr",		F_HIDELIST, 0 },
    { "f.horizoom",		F_HORIZOOM, 0 },
    { "f.htzoom",		F_TOPZOOM, 0 },
    { "f.hzoom",		F_HORIZOOM, 0 },
    { "f.iconify",		F_ICONIFY, 0 },
    { "f.identify",		F_IDENTIFY, 0 },
    { "f.lefticonmgr",		F_LEFTICONMGR, 0 },
    { "f.leftzoom",		F_LEFTZOOM, 0 },
    { "f.lower",		F_LOWER, 0 },
    { "f.menu",			F_MENU, 0 },
    { "f.move",			F_MOVE, 0 },
    { "f.nexticonmgr",		F_NEXTICONMGR, 0 },
    { "f.nop",			F_NOP, 0 },
    { "f.previconmgr",		F_PREVICONMGR, 0 },
    { "f.quit",			F_QUIT, 0 },
    { "f.raise",		F_RAISE, 0 },
    { "f.raiselower",		F_RAISELOWER, 0 },
    { "f.refresh",		F_REFRESH, 0 },
    { "f.resize",		F_RESIZE, 0 },
    { "f.restart",		F_RESTART, 0 },
    { "f.righticonmgr",		F_RIGHTICONMGR, 0 },
    { "f.rightzoom",		F_RIGHTZOOM, 0 },
    { "f.saveyourself",		F_SAVEYOURSELF, 0 },
    { "f.showiconmgr",		F_SHOWLIST, 0 },
    { "f.sorticonmgr",		F_SORTICONMGR, 0 },
    { "f.source",		F_SOURCE, 0 },
    { "f.title",		F_TITLE, 0 },
    { "f.topzoom",		F_TOPZOOM, 0 },
    { "f.twmrc",		F_TWMRC, 0 },
    { "f.unfocus",		F_UNFOCUS, 0 },
    { "f.upiconmgr",		F_UPICONMGR, 0 },
    { "f.version",		F_VERSION, 0 },
    { "f.vlzoom",		F_LEFTZOOM, 0 },
    { "f.vrzoom",		F_RIGHTZOOM, 0 },
    { "f.warpto",		F_WARPTO, 0 },
    { "f.warptoiconmgr",	F_WARPTOICONMGR, 0 },
    { "f.warptoscreen",		F_WARPTOSCREEN, 0 },
    { "f.winrefresh",		F_WINREFRESH, 0 },
    { "f.zoom",			F_ZOOM, 0 },
    { "forceicons",		KEYWORD, kw0_ForceIcons },
    { "frame",			FRAME, 0 },
    { "framepadding",		NKEYWORD, kwn_FramePadding },
    { "function",		FUNCTION, 0 },
    { "i",			ICON, 0 },
    { "icon",			ICON, 0 },
    { "iconbackground",		ICON_BACKGROUND, 0 },
    { "iconbordercolor",	ICON_BORDER_COLOR, 0 },
    { "iconborderwidth",	NKEYWORD, kwn_IconBorderWidth },
    { "icondirectory",		SKEYWORD, kws_IconDirectory },
    { "iconfont",		SKEYWORD, kws_IconFont },
    { "iconforeground",		ICON_FOREGROUND, 0 },
    { "iconifybyunmapping",	ICONIFY_BY_UNMAPPING, 0 },
    { "iconmanagerbackground",	ICONMGR_BACKGROUND, 0 },
    { "iconmanagerdontshow",	ICONMGR_NOSHOW, 0 },
    { "iconmanagerfont",	SKEYWORD, kws_IconManagerFont },
    { "iconmanagerforeground",	ICONMGR_FOREGROUND, 0 },
    { "iconmanagergeometry",	ICONMGR_GEOMETRY, 0 },
    { "iconmanagerhighlight",	ICONMGR_HIGHLIGHT, 0 },
    { "iconmanagers",		ICONMGRS, 0 },
    { "iconmanagershow",	ICONMGR_SHOW, 0 },
    { "iconmgr",		ICONMGR, 0 },
    { "iconregion",		ICON_REGION, 0 },
    { "icons",			ICONS, 0 },
    { "interpolatemenucolors",	KEYWORD, kw0_InterpolateMenuColors },
    { "lefttitlebutton",	LEFT_TITLEBUTTON, 0 },
    { "m",			META, 0 },
    { "maketitle",		MAKE_TITLE, 0 },
    { "menu",			MENU, 0 },
    { "menubackground",		MENU_BACKGROUND, 0 },
    { "menufont",		SKEYWORD, kws_MenuFont },
    { "menuforeground",		MENU_FOREGROUND, 0 },
    { "menushadowcolor",	MENU_SHADOW_COLOR, 0 },
    { "menutitlebackground",	MENU_TITLE_BACKGROUND, 0 },
    { "menutitleforeground",	MENU_TITLE_FOREGROUND, 0 },
    { "monochrome",		MONOCHROME, 0 },
    { "move",			MOVE, 0 },
    { "movedelta",		NKEYWORD, kwn_MoveDelta },
    { "nobackingstore",		KEYWORD, kw0_NoBackingStore },
    { "nodefaults",		KEYWORD, kw0_NoDefaults },
    { "nograbserver",		KEYWORD, kw0_NoGrabServer },
    { "nohighlight",		NO_HILITE, 0 },
    { "noiconmanagers",		KEYWORD, kw0_NoIconManagers },
    { "nomenushadows",		KEYWORD, kw0_NoMenuShadows },
    { "noraiseondeiconify",	KEYWORD, kw0_NoRaiseOnDeiconify },
    { "noraiseonmove",		KEYWORD, kw0_NoRaiseOnMove },
    { "noraiseonresize",	KEYWORD, kw0_NoRaiseOnResize },
    { "north",			NORTH, 0 },
    { "nosaveunders",		KEYWORD, kw0_NoSaveUnders },
    { "notitle",		NO_TITLE, 0 },
    { "notitlefocus",		KEYWORD, kw0_NoTitleFocus },
    { "notitlehighlight",	NO_TITLE_HILITE, 0 },
    { "noversion",		KEYWORD, kw0_NoVersion },
    { "opaquemove",		KEYWORD, kw0_OpaqueMove },
    { "pixmaps",		PIXMAPS, 0 },
    { "r",			ROOT, 0 },
    { "randomplacement",	KEYWORD, kw0_RandomPlacement },
    { "resize",			RESIZE, 0 },
    { "resizefont",		SKEYWORD, kws_ResizeFont },
    { "restartpreviousstate",	KEYWORD, kw0_RestartPreviousState },
    { "righttitlebutton",	RIGHT_TITLEBUTTON, 0 },
    { "root",			ROOT, 0 },
    { "s",			SHIFT, 0 },
    { "select",			SELECT, 0 },
    { "showiconmanager",	KEYWORD, kw0_ShowIconManager },
    { "sorticonmanager",	KEYWORD, kw0_SortIconManager },
    { "south",			SOUTH, 0 },
    { "squeezetitle",		KEYWORD, kw0_SqueezeTitle },
    { "starticonified",		START_ICONIFIED, 0 },
    { "t",			TITLE, 0 },
    { "t.lower",		F_LOWER, 0 },
    { "t.move",			F_MOVE, 0 },
    { "t.nop",			F_NOP, 0 },
    { "t.raise",		F_RAISE, 0 },
    { "title",			TITLE, 0 },
    { "titlebackground",	TITLE_BACKGROUND, 0 },
    { "titlebuttonborderwidth",	NKEYWORD, kwn_TitleButtonBorderWidth },
    { "titlefont",		SKEYWORD, kws_TitleFont },
    { "titleforeground",	TITLE_FOREGROUND, 0 },
    { "titlehighlight",		TITLE_HILITE, 0 },
    { "titlepadding",		NKEYWORD, kwn_TitlePadding },
    { "unknownicon",		SKEYWORD, kws_UnknownIcon },
    { "usepposition",		SKEYWORD, kws_UsePPosition },
    { "w",			WINDOW, 0 },
    { "wait",			WAIT, 0 },
    { "warpcursor",		KEYWORD, kw0_WarpCursor },
    { "west",			WEST, 0 },
    { "window",			WINDOW, 0 },
    { "windowfunction",		WINDOW_FUNCTION, 0 },
    { "xorvalue",		NKEYWORD, kwn_XorValue },
    { "zoom",			ZOOM, 0 },
};

static int numkeywords = (sizeof(keytable)/sizeof(keytable[0]));

int parse_keyword (s, nump)
    char *s;
    int *nump;
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
	    switch (p->value) {
	      case KEYWORD:
	      case SKEYWORD:
	      case NKEYWORD:
		*nump = p->subnum;
		break;
	    }
            return p->value;
        } else {
            upper = middle - 1;
        }
    }
    return -1;
}



/*
 * action routines called by grammar
 */

int do_single_keyword (keyword)
    int keyword;
{
    switch (keyword) {
      case kw0_NoDefaults:
	Scr->NoDefaults = TRUE;
	return 1;

      case kw0_AutoRelativeResize:
	Scr->AutoRelativeResize = TRUE;
	return 1;

      case kw0_ForceIcons:
	if (Scr->FirstTime) Scr->ForceIcon = TRUE;
	return 1;

      case kw0_NoIconManagers:
	Scr->NoIconManagers = TRUE;
	return 1;

      case kw0_OpaqueMove:
	Scr->OpaqueMove = TRUE;
	return 1;

      case kw0_InterpolateMenuColors:
	if (Scr->FirstTime) Scr->InterpolateMenuColors = TRUE;
	return 1;

      case kw0_WarpCursor:
	if (Scr->FirstTime) Scr->WarpCursor = TRUE;
	return 1;

      case kw0_NoVersion:
	/* obsolete */
	return 1;

      case kw0_SortIconManager:
	if (Scr->FirstTime) Scr->SortIconMgr = TRUE;
	return 1;

      case kw0_NoGrabServer:
	Scr->NoGrabServer = TRUE;
	return 1;

      case kw0_NoMenuShadows:
	if (Scr->FirstTime) Scr->Shadow = FALSE;
	return 1;

      case kw0_NoRaiseOnMove:
	if (Scr->FirstTime) Scr->NoRaiseMove = TRUE;
	return 1;

      case kw0_NoRaiseOnResize:
	if (Scr->FirstTime) Scr->NoRaiseResize = TRUE;
	return 1;

      case kw0_NoRaiseOnDeiconify:
	if (Scr->FirstTime) Scr->NoRaiseDeicon = TRUE;
	return 1;

      case kw0_DontMoveOff:
	Scr->DontMoveOff = TRUE;
	return 1;

      case kw0_NoBackingStore:
	Scr->BackingStore = FALSE;
	return 1;

      case kw0_NoSaveUnders:
	Scr->SaveUnder = FALSE;
	return 1;

      case kw0_RestartPreviousState:
	RestartPreviousState = True;
	return 1;

      case kw0_ClientBorderWidth:
	if (Scr->FirstTime) Scr->ClientBorderWidth = TRUE;
	return 1;

      case kw0_NoTitleFocus:
	Scr->TitleFocus = FALSE;
	return 1;

      case kw0_RandomPlacement:
	Scr->RandomPlacement = TRUE;
	return 1;

      case kw0_DecorateTransients:
	Scr->DecorateTransients = TRUE;
	return 1;

      case kw0_SqueezeTitle:
	Scr->SqueezeTitle = TRUE;
	return 1;

      case kw0_ShowIconManager:
	Scr->ShowIconManager = TRUE;
	return 1;
    }

    return 0;
}


int do_string_keyword (keyword, s)
    int keyword;
    char *s;
{
    switch (keyword) {
      case kws_UsePPosition:
	{ 
	    int ppos = ParseUsePPosition (s);
	    if (ppos < 0) {
		twmrc_error_prefix();
		fprintf (stderr,
			 "ignoring invalid UsePPosition argument \"%s\"\n", s);
	    } else {
		Scr->UsePPosition = ppos;
	    }
	    return 1;
	}

      case kws_IconFont:
	if (!Scr->HaveFonts) Scr->IconFont.name = s;
	return 1;

      case kws_ResizeFont:
	if (!Scr->HaveFonts) Scr->SizeFont.name = s;
	return 1;

      case kws_MenuFont:
	if (!Scr->HaveFonts) Scr->MenuFont.name = s;
	return 1;

      case kws_TitleFont:
	if (!Scr->HaveFonts) Scr->TitleBarFont.name = s;
	return 1;

      case kws_IconManagerFont:
	if (!Scr->HaveFonts) Scr->IconManagerFont.name = s;
	return 1;

      case kws_UnknownIcon:
	if (Scr->FirstTime) GetUnknownIcon (s);
	return 1;

      case kws_IconDirectory:
	if (Scr->FirstTime) Scr->IconDirectory = ExpandFilename (s);
	return 1;
    }

    return 0;
}


int do_number_keyword (keyword, num)
    int keyword;
    int num;
{
    switch (keyword) {
      case kwn_ConstrainedMoveTime:
	ConstrainedMoveTime = num;
	return 1;

      case kwn_MoveDelta:
	Scr->MoveDelta = num;
	return 1;

      case kwn_XorValue:
	if (Scr->FirstTime) Scr->XORvalue = num;
	return 1;

      case kwn_FramePadding:
	if (Scr->FirstTime) Scr->FramePadding = num;
	return 1;

      case kwn_TitlePadding:
	if (Scr->FirstTime) Scr->TitlePadding = num;
	return 1;

      case kwn_ButtonIndent:
	if (Scr->FirstTime) Scr->ButtonIndent = num;
	return 1;

      case kwn_BorderWidth:
	if (Scr->FirstTime) Scr->BorderWidth = num;
	return 1;

      case kwn_IconBorderWidth:
	if (Scr->FirstTime) Scr->IconBorderWidth = num;
	return 1;

      case kwn_TitleButtonBorderWidth:
	if (Scr->FirstTime) Scr->TBInfo.border = num;
	return 1;

    }

    return 0;
}

static int ParseUsePPosition (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, "off") == 0) {
	return PPOS_OFF;
    } else if (strcmp (s, "on") == 0) {
	return PPOS_ON;
    } else if (strcmp (s, "non-zero") == 0 ||
	       strcmp (s, "nonzero") == 0) {
	return PPOS_NON_ZERO;
    }

    return -1;
}


