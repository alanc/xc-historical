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
 * $XConsortium: gram.y,v 1.75 89/11/16 19:26:22 jim Exp $
 *
 * .twmrc command grammer
 *
 * 07-Jan-86 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

%{
static char RCSinfo[]=
"$XConsortium: gram.y,v 1.75 89/11/16 19:26:22 jim Exp $";

#include <stdio.h>
#include <ctype.h>
#include "twm.h"
#include "menus.h"
#include "list.h"
#include "util.h"
#include "screen.h"
#include "parse.h"

static char *Action = "";
static char *Name = "";
static MenuRoot	*root, *pull = NULL;

MenuRoot *GetRoot();

static Bool CheckWarpScreenArg(), CheckColormapArg();
static void GotButton(), GotKey(), GotTitleButton();
static char *ptr;
static name_list **list;
static int cont = 0;
static int color;
int mods = 0;

extern int do_single_keyword(), do_string_keyword(), do_number_keyword();
extern name_list **do_colorlist_keyword();
extern int do_color_keyword();
extern int yylineno;
%}

%union
{
    int num;
    char *ptr;
};

%token <num> LB RB LP RP MENUS MENU BUTTON DEFAULT_FUNCTION PLUS MINUS
%token <num> ALL OR CURSORS PIXMAPS ICONS COLOR MONOCHROME FUNCTION 
%token <num> ICONMGR_SHOW ICONMGR WINDOW_FUNCTION ZOOM ICONMGRS
%token <num> ICONMGR_GEOMETRY ICONMGR_NOSHOW MAKE_TITLE
%token <num> ICONIFY_BY_UNMAPPING DONT_ICONIFY_BY_UNMAPPING 
%token <num> NO_TITLE AUTO_RAISE NO_HILITE ICON_REGION 
%token <num> META SHIFT CONTROL WINDOW TITLE ICON ROOT FRAME 
%token <num> COLON EQUALS SQUEEZE_TITLE DONT_SQUEEZE_TITLE
%token <num> START_ICONIFIED NO_TITLE_HILITE TITLE_HILITE
%token <num> MOVE RESIZE WAIT SELECT KILL LEFT_TITLEBUTTON RIGHT_TITLEBUTTON 
%token <num> NUMBER KEYWORD NKEYWORD CKEYWORD CLKEYWORD FKEYWORD FSKEYWORD 
%token <num> DKEYWORD JKEYWORD 
%token <ptr> STRING SKEYWORD

%type <ptr> string
%type <num> action button number signed_number full fullkey

%start twmrc 

%%
twmrc		: stmts
		;

stmts		: /* Empty */
		| stmts stmt
		;

stmt		: error
		| noarg
		| sarg
		| narg
		| squeeze
		| ICON_REGION string DKEYWORD DKEYWORD number number
					{ AddIconRegion($2, $3, $4, $5, $6); }
		| ICONMGR_GEOMETRY string number	{ if (Scr->FirstTime)
						  {
						    Scr->iconmgr.geometry=$2;
						    Scr->iconmgr.columns=$3;
						  }
						}
		| ICONMGR_GEOMETRY string	{ if (Scr->FirstTime)
						    Scr->iconmgr.geometry = $2;
						}
		| ZOOM number		{ if (Scr->FirstTime)
					  {
						Scr->DoZoom = TRUE;
						Scr->ZoomCount = $2;
					  }
					}
		| ZOOM			{ if (Scr->FirstTime) 
						Scr->DoZoom = TRUE; }
		| PIXMAPS pixmap_list	{}
		| CURSORS cursor_list	{}
		| ICONIFY_BY_UNMAPPING	{ list = &Scr->IconifyByUn; }
		  win_list
		| ICONIFY_BY_UNMAPPING	{ if (Scr->FirstTime) 
		    Scr->IconifyByUnmapping = TRUE; }
		| LEFT_TITLEBUTTON string EQUALS action { 
					  GotTitleButton ($2, $4, False);
					}
		| RIGHT_TITLEBUTTON string EQUALS action { 
					  GotTitleButton ($2, $4, True);
					}
		| button string		{ root = GetRoot($2, 0, 0);
					  Scr->Mouse[$1][C_ROOT][0].func = F_MENU;
					  Scr->Mouse[$1][C_ROOT][0].menu = root;
					}
		| button action		{ Scr->Mouse[$1][C_ROOT][0].func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Scr->Mouse[$1][C_ROOT][0].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT, 0, 0);
					    Scr->Mouse[$1][C_ROOT][0].item = 
						AddToMenu(root,"x",Action,0,$2,
							NULL, NULL);
					  }
					  Action = "";
					  pull = NULL;
					}
		| string fullkey	{ GotKey($1, $2); }
		| button full		{ GotButton($1, $2); }
		| DONT_ICONIFY_BY_UNMAPPING { list = &Scr->DontIconify; }
		  win_list
		| ICONMGR_NOSHOW	{ list = &Scr->IconMgrNoShow; }
		  win_list
		| ICONMGR_NOSHOW	{ Scr->IconManagerDontShow = TRUE; }
		| ICONMGRS		{ list = &Scr->IconMgrs; }
		  iconm_list
		| ICONMGR_SHOW		{ list = &Scr->IconMgrShow; }
		  win_list
		| NO_TITLE_HILITE	{ list = &Scr->NoTitleHighlight; }
		  win_list
		| NO_TITLE_HILITE	{ if (Scr->FirstTime)
						Scr->TitleHighlight = FALSE; }
		| NO_HILITE		{ list = &Scr->NoHighlight; }
		  win_list
		| NO_HILITE		{ if (Scr->FirstTime)
						Scr->Highlight = FALSE; }
		| NO_TITLE		{ list = &Scr->NoTitle; }
		  win_list
		| NO_TITLE		{ if (Scr->FirstTime)
						Scr->NoTitlebar = TRUE; }
		| MAKE_TITLE		{ list = &Scr->MakeTitle; }
		  win_list
		| START_ICONIFIED	{ list = &Scr->StartIconified; }
		  win_list
		| AUTO_RAISE		{ list = &Scr->AutoRaise; }
		  win_list
		| MENU string LP string COLON string RP	{
					root = GetRoot($2, $4, $6); }
		  menu			{ root->real_menu = TRUE;}
		| MENU string 		{ root = GetRoot($2, 0, 0); }
		  menu			{ root->real_menu = TRUE; }
		| FUNCTION string	{ root = GetRoot($2, 0, 0); }
		  function
		| ICONS 		{ list = &Scr->IconNames; }
		  icon_list
		| COLOR 		{ color = COLOR; }
		  color_list
		| MONOCHROME 		{ color = MONOCHROME; }
		  color_list
		| DEFAULT_FUNCTION action { Scr->DefaultFunction.func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Scr->DefaultFunction.menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT, 0, 0);
					    Scr->DefaultFunction.item = 
						AddToMenu(root,"x",Action,0,$2,
							NULL, NULL);
					  }
					  Action = "";
					  pull = NULL;
					}
		| WINDOW_FUNCTION action { Scr->WindowFunction.func = $2;
					   root = GetRoot(TWM_ROOT, 0, 0);
					   Scr->WindowFunction.item = 
						AddToMenu(root,"x",Action,0,$2,
							NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
		;


noarg		: KEYWORD		{ if (!do_single_keyword ($1)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
					"unknown singleton keyword %d\n",
						     $1);
					    ParseError = 1;
					  }
					}
		;

sarg		: SKEYWORD string	{ if (!do_string_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		;

narg		: NKEYWORD number	{ if (!do_number_keyword ($1, $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown numeric keyword %d (value %d)\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		;



full		: EQUALS keys COLON contexts COLON action  { $$ = $6; }
		;

fullkey		: EQUALS keys COLON contextkeys COLON action  { $$ = $6; }
		;

keys		: /* Empty */
		| keys key
		;

key		: META			{ mods |= Mod1Mask; }
		| SHIFT			{ mods |= ShiftMask; }
		| CONTROL		{ mods |= ControlMask; }
		| OR			{ }
		;

contexts	: /* Empty */
		| contexts context
		;

context		: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{  }
		;

contextkeys	: /* Empty */
		| contextkeys contextkey
		;

contextkey	: WINDOW		{ cont |= C_WINDOW_BIT; }
		| TITLE			{ cont |= C_TITLE_BIT; }
		| ICON			{ cont |= C_ICON_BIT; }
		| ROOT			{ cont |= C_ROOT_BIT; }
		| FRAME			{ cont |= C_FRAME_BIT; }
		| ICONMGR		{ cont |= C_ICONMGR_BIT; }
		| META			{ cont |= C_ICONMGR_BIT; }
		| ALL			{ cont |= C_ALL_BITS; }
		| OR			{ }
		| string		{ Name = $1; cont |= C_NAME_BIT; }
		;


pixmap_list	: LB pixmap_entries RB
		;

pixmap_entries	: /* Empty */
		| pixmap_entries pixmap_entry
		;

pixmap_entry	: TITLE_HILITE string { SetHighlightPixmap ($2); }
		;


cursor_list	: LB cursor_entries RB
		;

cursor_entries	: /* Empty */
		| cursor_entries cursor_entry
		;

cursor_entry	: FRAME string string {
			NewBitmapCursor(&Scr->FrameCursor, $2, $3); }
		| FRAME string	{
			NewFontCursor(&Scr->FrameCursor, $2); }
		| TITLE string string {
			NewBitmapCursor(&Scr->TitleCursor, $2, $3); }
		| TITLE string {
			NewFontCursor(&Scr->TitleCursor, $2); }
		| ICON string string {
			NewBitmapCursor(&Scr->IconCursor, $2, $3); }
		| ICON string {
			NewFontCursor(&Scr->IconCursor, $2); }
		| ICONMGR string string {
			NewBitmapCursor(&Scr->IconMgrCursor, $2, $3); }
		| ICONMGR string {
			NewFontCursor(&Scr->IconMgrCursor, $2); }
		| BUTTON string string {
			NewBitmapCursor(&Scr->ButtonCursor, $2, $3); }
		| BUTTON string {
			NewFontCursor(&Scr->ButtonCursor, $2); }
		| MOVE string string {
			NewBitmapCursor(&Scr->MoveCursor, $2, $3); }
		| MOVE string {
			NewFontCursor(&Scr->MoveCursor, $2); }
		| RESIZE string string {
			NewBitmapCursor(&Scr->ResizeCursor, $2, $3); }
		| RESIZE string {
			NewFontCursor(&Scr->ResizeCursor, $2); }
		| WAIT string string {
			NewBitmapCursor(&Scr->WaitCursor, $2, $3); }
		| WAIT string {
			NewFontCursor(&Scr->WaitCursor, $2); }
		| MENU string string {
			NewBitmapCursor(&Scr->MenuCursor, $2, $3); }
		| MENU string {
			NewFontCursor(&Scr->MenuCursor, $2); }
		| SELECT string string {
			NewBitmapCursor(&Scr->SelectCursor, $2, $3); }
		| SELECT string {
			NewFontCursor(&Scr->SelectCursor, $2); }
		| KILL string string {
			NewBitmapCursor(&Scr->DestroyCursor, $2, $3); }
		| KILL string {
			NewFontCursor(&Scr->DestroyCursor, $2); }
		;

color_list	: LB color_entries RB
		;

color_entries	: /* Empty */
		| color_entries color_entry
		;

color_entry	: CLKEYWORD string	{ if (!do_colorlist_keyword ($1, color,
								     $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled list color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		| CLKEYWORD string	{ list = do_colorlist_keyword($1,color,
								      $2);
					  if (!list) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color list keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		  win_color_list
		| CKEYWORD string	{ if (!do_color_keyword ($1, color,
								 $2)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color keyword %d (string \"%s\")\n",
						     $1, $2);
					    ParseError = 1;
					  }
					}
		;


win_color_list	: LB win_color_entries RB
		;

win_color_entries	: /* Empty */
		| win_color_entries win_color_entry
		;

win_color_entry	: string string		{ if (Scr->FirstTime &&
					      color == Scr->Monochrome)
					    AddToList(list, $1, $2); }
		;

squeeze		: SQUEEZE_TITLE { 
#ifdef SHAPE
				    if (HasShape) Scr->SqueezeTitle = TRUE;
#endif
				}
		| SQUEEZE_TITLE { list = &Scr->SqueezeTitleL; 
#ifdef SHAPE
				  if (HasShape && Scr->SqueezeTitle == -1)
				    Scr->SqueezeTitle = TRUE;
#endif
				}
		  LB win_sqz_entries RB
		| DONT_SQUEEZE_TITLE { Scr->SqueezeTitle = FALSE; }
		| DONT_SQUEEZE_TITLE { list = &Scr->DontSqueezeTitleL; }
		  win_list
		;

win_sqz_entries	: /* Empty */
		| win_sqz_entries string JKEYWORD signed_number number	{
				if (Scr->FirstTime) {
				   do_squeeze_entry (list, $2, $3, $4, $5);
				}
			}
		;


iconm_list	: LB iconm_entries RB
		;

iconm_entries	: /* Empty */
		| iconm_entries iconm_entry
		;

iconm_entry	: string string number	{ if (Scr->FirstTime)
					    AddToList(list, $1,
						AllocateIconManager($1, NULL,
							$2,$3));
					}
		| string string string number
					{ if (Scr->FirstTime)
					    AddToList(list, $1,
						AllocateIconManager($1,$2,
						$3, $4));
					}
		;

win_list	: LB win_entries RB
		;

win_entries	: /* Empty */
		| win_entries win_entry
		;

win_entry	: string		{ if (Scr->FirstTime)
					    AddToList(list, $1, 0);
					}
		;

icon_list	: LB icon_entries RB
		;

icon_entries	: /* Empty */
		| icon_entries icon_entry
		;

icon_entry	: string string		{ if (Scr->FirstTime) AddToList(list, $1, $2); }
		;

function	: LB function_entries RB
		;

function_entries: /* Empty */
		| function_entries function_entry
		;

function_entry	: action		{ AddToMenu(root, "", Action, NULL, $1,
						NULL, NULL);
					  Action = "";
					}
		;

menu		: LB menu_entries RB
		;

menu_entries	: /* Empty */
		| menu_entries menu_entry
		;

menu_entry	: string action		{ AddToMenu(root, $1, Action, pull, $2,
						NULL, NULL);
					  Action = "";
					  pull = NULL;
					}
		| string LP string COLON string RP action {
					  AddToMenu(root, $1, Action, pull, $7,
						$3, $5);
					  Action = "";
					  pull = NULL;
					}
		;

action		: FKEYWORD	{ $$ = $1; }
		| FSKEYWORD string {
				$$ = $1;
				Action = $2;
				switch ($1) {
				  case F_SOURCE:
				    $$ = F_TWMRC;
				    break;
				  case F_MENU:
				    pull = GetRoot ($2, 0, 0);
				    pull->prev = root;
				    break;
				  case F_WARPTOSCREEN:
				    if (!CheckWarpScreenArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr, 
			"ignoring invalid f.warpscreen argument \"%s\"\n", 
					         Action);
					$$ = F_NOP;
				    }
				    break;
				  case F_COLORMAP:
				    if (CheckColormapArg (Action)) {
					$$ = F_COLORMAP;
				    } else {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.colormap argument \"%s\"\n", 
						 Action);
					$$ = F_NOP;
				    }
				    break;
				} /* end switch */
				   }
		;


signed_number	: number		{ $$ = $1; }
		| PLUS number		{ $$ = $2; }
		| MINUS number		{ $$ = -($2); }
		;

button		: BUTTON number		{ $$ = $2;
					  if ($2 == 0)
						yyerror("bad button 0");

					  if ($2 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror("button number too large");
					  }
					}
		;

string		: STRING		{ ptr = (char *)malloc(strlen($1)+1);
					  strcpy(ptr, $1);
					  RemoveDQuote(ptr);
					  $$ = ptr;
					}
number		: NUMBER		{ $$ = $1; }
		;

%%
yyerror(s) char *s;
{
    twmrc_error_prefix();
    fprintf (stderr, "error in input file:  %s\n", s ? s : "");
    ParseError = 1;
}
RemoveDQuote(str)
char *str;
{
    register char *i, *o;
    register n;
    register count;

    for (i=str+1, o=str; *i && *i != '\"'; o++)
    {
	if (*i == '\\')
	{
	    switch (*++i)
	    {
	    case 'n':
		*o = '\n';
		i++;
		break;
	    case 'b':
		*o = '\b';
		i++;
		break;
	    case 'r':
		*o = '\r';
		i++;
		break;
	    case 't':
		*o = '\t';
		i++;
		break;
	    case 'f':
		*o = '\f';
		i++;
		break;
	    case '0':
		if (*++i == 'x')
		    goto hex;
		else
		    --i;
	    case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
		n = 0;
		count = 0;
		while (*i >= '0' && *i <= '7' && count < 3)
		{
		    n = (n<<3) + (*i++ - '0');
		    count++;
		}
		*o = n;
		break;
	    hex:
	    case 'x':
		n = 0;
		count = 0;
		while (i++, count++ < 2)
		{
		    if (*i >= '0' && *i <= '9')
			n = (n<<4) + (*i - '0');
		    else if (*i >= 'a' && *i <= 'f')
			n = (n<<4) + (*i - 'a') + 10;
		    else if (*i >= 'A' && *i <= 'F')
			n = (n<<4) + (*i - 'A') + 10;
		    else
			break;
		}
		*o = n;
		break;
	    case '\"':
	    case '\'':
	    case '\\':
	    default:
		*o = *i++;
		break;
	    }
	}
	else
	    *o = *i++;
    }
    *o = '\0';
}

MenuRoot *
GetRoot(name, fore, back)
char *name;
char *fore, *back;
{
    MenuRoot *tmp;

    tmp = FindMenuRoot(name);
    if (tmp == NULL)
	tmp = NewMenuRoot(name, fore, back);

    if (fore)
    {
	int save;

	save = Scr->FirstTime;
	Scr->FirstTime = TRUE;
	GetColor(COLOR, &tmp->hi_fore, fore);
	GetColor(COLOR, &tmp->hi_back, back);
	Scr->FirstTime = save;
    }

    return tmp;
}

static void GotButton(butt, func)
int butt, func;
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++)
    {
	if ((cont & (1 << i)) == 0)
	    continue;

	Scr->Mouse[butt][i][mods].func = func;
	if (func == F_MENU)
	{
	    pull->prev = NULL;
	    Scr->Mouse[butt][i][mods].menu = pull;
	}
	else
	{
	    root = GetRoot(TWM_ROOT, 0, 0);
	    Scr->Mouse[butt][i][mods].item = AddToMenu(root,"x",Action,0,func,
		    NULL, NULL);
	}
    }
    Action = "";
    pull = NULL;
    cont = 0;
    mods = 0;
}

static void GotKey(key, func)
char *key;
int func;
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++)
    {
	if ((cont & (1 << i)) == 0) 
	  continue;
	if (!AddFuncKey(key, i, mods, func, Name, Action)) 
	  break;
    }

    Action = "";
    pull = NULL;
    cont = 0;
    mods = 0;
}


static void GotTitleButton (bitmapname, func, rightside)
    char *bitmapname;
    int func;
    Bool rightside;
{
    if (!AddTitleButton (bitmapname, func, Action, pull, rightside)) {
	twmrc_error_prefix();
	fprintf (stderr, 
		 "unable to add %s titlebutton \"%s\"\n",
		 rightside ? "right" : "left", bitmapname);
    }
    Action = "";
    pull = NULL;
}

static Bool CheckWarpScreenArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s,  WARPSCREEN_NEXT) == 0 ||
	strcmp (s,  WARPSCREEN_PREV) == 0 ||
	strcmp (s,  WARPSCREEN_BACK) == 0)
      return True;

    for (; *s && isascii(*s) && isdigit(*s); s++) ;
    return (*s ? False : True);
}


static Bool CheckColormapArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);

    if (strcmp (s, COLORMAP_NEXT) == 0 ||
	strcmp (s, COLORMAP_PREV) == 0 ||
	strcmp (s, COLORMAP_DEFAULT) == 0)
      return True;

    return False;
}


twmrc_error_prefix ()
{
    fprintf (stderr, "twm:  line %d:  ", yylineno);
}
