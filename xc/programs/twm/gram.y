/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

/***********************************************************************
 *
 * $XConsortium: gram.y,v 1.40 89/06/09 13:36:59 jim Exp $
 *
 * .twmrc command grammer
 *
 * 07-Jan-86 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

%{
static char RCSinfo[]=
"$XConsortium: gram.y,v 1.40 89/06/09 13:36:59 jim Exp $";

#include <stdio.h>
#include <ctype.h>
#include "twm.h"
#include "menus.h"
#include "list.h"
#include "util.h"
#include "screen.h"

static char *Action = "";
static char *Name = "";
static MenuRoot	*root,
		*pull = NULL;


MenuRoot *GetRoot();

static Bool CheckWarpScreenArg();
static char *ptr;
static int Button;
static name_list **list;
static int cont = 0;
static int color;
int num[5], mult, indx = 0;
int mods = 0;

extern int yylineno;
%}

%union
{
    int num;
    char *ptr;
};

%token <num> LB RB MENUS MENU BUTTON TBUTTON DEFAULT_FUNCTION PLUS MINUS
%token <num> F_MENU F_UNFOCUS F_REFRESH F_FILE F_TWMRC F_CIRCLEUP F_QUIT
%token <num> F_NOP F_TITLE F_VERSION F_EXEC F_CUT F_CIRCLEDOWN F_SOURCE
%token <num> F_CUTFILE F_MOVE F_ICONIFY F_FOCUS F_RESIZE F_RAISE F_LOWER
%token <num> F_POPUP F_DEICONIFY F_FORCEMOVE WINDOW_FUNCTION MOVE_DELTA
%token <num> F_DESTROY F_WINREFRESH F_BEEP DONT_MOVE_OFF ZOOM ICONMGRS
%token <num> F_SHOWLIST F_HIDELIST NO_BACKINGSTORE NO_SAVEUNDER
%token <num> F_ZOOM F_FULLZOOM F_UPICONMGR F_DOWNICONMGR F_HORIZOOM
%token <num> F_RIGHTZOOM F_LEFTZOOM F_TOPZOOM F_BOTTOMZOOM F_RESTART 
%token <num> F_LEFTICONMGR F_RIGHTICONMGR F_WARPTO F_DELTASTOP ICONMGR_SHOW
%token <num> F_WARPTOICONMGR OPAQUE_MOVE ICONMGR_HIGHLIGHT SORT_ICONMGR
%token <num> ICONMGR_FOREGROUND ICONMGR_BACKGROUND ICONMGR_FONT ICONMGR
%token <num> ICONMGR_GEOMETRY SHOW_ICONMGR ICONMGR_NOSHOW MAKE_TITLE
%token <num> F_RAISELOWER DECORATE_TRANSIENTS RANDOM_PLACEMENT
%token <num> ICONIFY_BY_UNMAPPING DONT_ICONIFY_BY_UNMAPPING 
%token <num> WARPCURSOR NUMBER BORDERWIDTH CLIENT_BORDERWIDTH TITLE_FONT 
%token <num> RESIZE_FONT NO_TITLE AUTO_RAISE FORCE_ICON NO_HILITE
%token <num> MENU_FONT ICON_FONT UNKNOWN_ICON ICONS ICON_DIRECTORY
%token <num> META SHIFT CONTROL WINDOW TITLE ICON ROOT FRAME 
%token <num> COLON EQUALS BORDER_COLOR TITLE_FOREGROUND TITLE_BACKGROUND
%token <num> DEFAULT_FOREGROUND DEFAULT_BACKGROUND 
%token <num> MENU_FOREGROUND MENU_BACKGROUND MENU_SHADOW_COLOR
%token <num> MENU_TITLE_FOREGROUND MENU_TITLE_BACKGROUND F_AUTORAISE
%token <num> ICON_FOREGROUND ICON_BACKGROUND ICON_BORDER_COLOR NO_GRAB_SERVER
%token <num> NO_RAISE_ON_MOVE NO_RAISE_ON_DEICONIFY NO_RAISE_ON_RESIZE
%token <num> COLOR MONOCHROME NO_TITLE_FOCUS FUNCTION F_FUNCTION
%token <num> BORDER_TILE_FOREGROUND BORDER_TILE_BACKGROUND F_IDENTIFY
%token <num> F_FORWICONMGR F_BACKICONMGR F_NEXTICONMGR F_PREVICONMGR
%token <num> START_ICONIFIED NO_MENU_SHADOWS LP RP NO_VERSION
%token <num> INTERPOLATE_MENUS NO_TITLE_HILITE ICON_BORDERWIDTH TITLE_HILITE
%token <num> F_WARPTOICONMGRE ALL OR CURSORS PIXMAPS CUR_BUTTON CUR_FRAME
%token <num> CUR_TITLE CUR_ICONMGR CUR_ICON NO_ICONMGRS F_SORTICONMGR
%token <num> CUR_MOVE CUR_RESIZE CUR_WAIT CUR_SELECT CUR_KILL
%token <num> ICON_REGION NORTH SOUTH EAST WEST RESTART_PREVIOUS_STATE
%token <num> F_WARPSCREEN
%token <ptr> STRING

%type <ptr> string
%type <num> action button number tbutton full fullkey grav

%start twmrc 

%%
twmrc		: stmts
		;

stmts		: /* Empty */
		| stmts stmt
		;

stmt		: error
		| FORCE_ICON		{ if (Scr->FirstTime) Scr->ForceIcon = TRUE; }
		| ICON_REGION string grav grav number number
					{ AddIconRegion($2, $3, $4, $5, $6); }
		| ICON_FONT string	{   Scr->IconFont.name = $2;
					    GetFont(&Scr->IconFont);
					}
		| RESIZE_FONT string	{   Scr->SizeFont.name = $2;
					    GetFont(&Scr->SizeFont);
					}
		| MENU_FONT string	{   Scr->MenuFont.name = $2;
					    GetFont(&Scr->MenuFont);
					}
		| TITLE_FONT string	{   Scr->TitleBarFont.name = $2;
					    GetFont(&Scr->TitleBarFont);
					}
		| ICONMGR_FONT string	{   Scr->IconManagerFont.name=$2;
					    GetFont(&Scr->IconManagerFont);
					}
		| ICONMGR_GEOMETRY string number{ if (Scr->FirstTime)
						  {
						    Scr->iconmgr.geometry=$2;
						    Scr->iconmgr.columns=$3;
						  }
						}
		| ICONMGR_GEOMETRY string{ if (Scr->FirstTime)
						Scr->iconmgr.geometry=$2;}
		| UNKNOWN_ICON string	{ if (Scr->FirstTime)
						GetUnknownIcon($2); }
		| ICON_DIRECTORY string	{ if (Scr->FirstTime)
					Scr->IconDirectory = ExpandFilename($2);
					}
		| NO_ICONMGRS		{ Scr->NoIconManagers = TRUE; }
		| OPAQUE_MOVE		{ Scr->OpaqueMove = TRUE; }
		| INTERPOLATE_MENUS	{ if (Scr->FirstTime)
					    Scr->InterpolateMenuColors=TRUE; }
		| WARPCURSOR		{ if (Scr->FirstTime)
						Scr->WarpCursor = TRUE; }
		| NO_VERSION		{ if (Scr->FirstTime)
						Scr->ShowVersion = FALSE; }
		| SORT_ICONMGR		{ if (Scr->FirstTime) 
						Scr->SortIconMgr = TRUE; }
		| NO_GRAB_SERVER	{ if (Scr->FirstTime)
						Scr->NoGrabServer = TRUE; }
		| NO_MENU_SHADOWS	{ if (Scr->FirstTime) 
						Scr->Shadow = FALSE; }
		| NO_RAISE_ON_MOVE	{ if (Scr->FirstTime) 
						Scr->NoRaiseMove = TRUE; }
		| NO_RAISE_ON_RESIZE	{ if (Scr->FirstTime) 
						Scr->NoRaiseResize = TRUE; }
		| NO_RAISE_ON_DEICONIFY	{ if (Scr->FirstTime) 
						Scr->NoRaiseDeicon = TRUE; }
		| DONT_MOVE_OFF		{ if (Scr->FirstTime) 
						Scr->DontMoveOff = TRUE; }
		| NO_BACKINGSTORE	{ if (Scr->FirstTime) 
						Scr->BackingStore = FALSE; }
		| NO_SAVEUNDER		{ if (Scr->FirstTime) 
						Scr->SaveUnder = FALSE; }
		| MOVE_DELTA number	{ Scr->MoveDelta = $2; }
		| ZOOM number		{ if (Scr->FirstTime)
					  {
						Scr->DoZoom = TRUE;
						Scr->ZoomCount = $2;
					  }
					}
		| ZOOM			{ if (Scr->FirstTime) 
						Scr->DoZoom = TRUE; }
		| BORDERWIDTH number	{ if (Scr->FirstTime) 
						Scr->BorderWidth = $2; }
		| ICON_BORDERWIDTH number{ if (Scr->FirstTime) 
						Scr->IconBorderWidth = $2; }
		| RESTART_PREVIOUS_STATE { if (Scr->FirstTime)
						RestartPreviousState = True; }
		| CLIENT_BORDERWIDTH	{ if (Scr->FirstTime)
						Scr->ClientBorderWidth=TRUE; }
		| NO_TITLE_FOCUS	{ if (Scr->FirstTime) 
						Scr->TitleFocus = FALSE; }
		| RANDOM_PLACEMENT	{ if (Scr->FirstTime) 
						Scr->RandomPlacement=TRUE; }
		| DECORATE_TRANSIENTS	{ if (Scr->FirstTime) 
						Scr->DecorateTransients = TRUE;}
		| PIXMAPS pixmap_list	{}
		| CURSORS cursor_list	{}
		| ICONIFY_BY_UNMAPPING	{ list = &Scr->IconifyByUn; }
		  win_list
		| ICONIFY_BY_UNMAPPING	{ if (Scr->FirstTime) 
		    Scr->IconifyByUnmapping = TRUE; }
		| SHOW_ICONMGR	{ if (Scr->FirstTime) 
					Scr->ShowIconManager = TRUE; }
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
		| tbutton action	{ Scr->Mouse[$1][C_TITLE][0].func = $2;
					  Scr->Mouse[$1][C_ICON][0].func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Scr->Mouse[$1][C_TITLE][0].menu = pull;
					    Scr->Mouse[$1][C_ICON][0].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT, 0, 0);
					    Scr->Mouse[$1][C_TITLE][0].item = 
						AddToMenu(root,"x",Action,0,$2,
							NULL, NULL);
					    Scr->Mouse[$1][C_ICON][0].item =
						Scr->Mouse[$1][C_TITLE][0].item;
					  }
					  Action = "";
					  pull = NULL;
					}
		| DONT_ICONIFY_BY_UNMAPPING { list = &Scr->DontIconify; }
		  win_list
		| geometry
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

cursor_entry	: CUR_FRAME string string {
			NewBitmapCursor(&Scr->FrameCursor, $2, $3); }
		| CUR_FRAME string	{
			NewFontCursor(&Scr->FrameCursor, $2); }
		| CUR_TITLE string string {
			NewBitmapCursor(&Scr->TitleCursor, $2, $3); }
		| CUR_TITLE string {
			NewFontCursor(&Scr->TitleCursor, $2); }
		| CUR_ICON string string {
			NewBitmapCursor(&Scr->IconCursor, $2, $3); }
		| CUR_ICON string {
			NewFontCursor(&Scr->IconCursor, $2); }
		| CUR_ICONMGR string string {
			NewBitmapCursor(&Scr->IconMgrCursor, $2, $3); }
		| CUR_ICONMGR string {
			NewFontCursor(&Scr->IconMgrCursor, $2); }
		| CUR_BUTTON string string {
			NewBitmapCursor(&Scr->ButtonCursor, $2, $3); }
		| CUR_BUTTON string {
			NewFontCursor(&Scr->ButtonCursor, $2); }
		| CUR_MOVE string string {
			NewBitmapCursor(&Scr->MoveCursor, $2, $3); }
		| CUR_MOVE string {
			NewFontCursor(&Scr->MoveCursor, $2); }
		| CUR_RESIZE string string {
			NewBitmapCursor(&Scr->ResizeCursor, $2, $3); }
		| CUR_RESIZE string {
			NewFontCursor(&Scr->ResizeCursor, $2); }
		| CUR_WAIT string string {
			NewBitmapCursor(&Scr->WaitCursor, $2, $3); }
		| CUR_WAIT string {
			NewFontCursor(&Scr->WaitCursor, $2); }
		| MENU string string {
			NewBitmapCursor(&Scr->MenuCursor, $2, $3); }
		| MENU string {
			NewFontCursor(&Scr->MenuCursor, $2); }
		| CUR_SELECT string string {
			NewBitmapCursor(&Scr->SelectCursor, $2, $3); }
		| CUR_SELECT string {
			NewFontCursor(&Scr->SelectCursor, $2); }
		| CUR_KILL string string {
			NewBitmapCursor(&Scr->DestroyCursor, $2, $3); }
		| CUR_KILL string {
			NewFontCursor(&Scr->DestroyCursor, $2); }
		;

color_list	: LB color_entries RB
		;

color_entries	: /* Empty */
		| color_entries color_entry
		;

color_entry	: BORDER_COLOR string	{ GetColor(color,
						   &Scr->BorderColor, $2); }
		| BORDER_COLOR string	{ GetColor(color,
						   &Scr->BorderColor, $2);
					    list = &Scr->BorderColorL; }
		  win_color_list
		| ICONMGR_HIGHLIGHT string { GetColor(color,
						&Scr->IconManagerHighlight,$2);}
		| ICONMGR_HIGHLIGHT string { GetColor(color,
						&Scr->IconManagerHighlight,$2);
					    list = &Scr->IconManagerHighlightL;}
		  win_color_list
		| BORDER_TILE_FOREGROUND string { GetColor(color,
						&Scr->BorderTileC.fore, $2); }
		| BORDER_TILE_FOREGROUND string { GetColor(color,
						&Scr->BorderTileC.fore, $2);
					    list = &Scr->BorderTileForegroundL;}
		  win_color_list
		| BORDER_TILE_BACKGROUND string { GetColor(color,
						&Scr->BorderTileC.back, $2); }
		| BORDER_TILE_BACKGROUND string { GetColor(color,
						&Scr->BorderTileC.back, $2);
					    list = &Scr->BorderTileBackgroundL;}
		  win_color_list
		| TITLE_FOREGROUND string { GetColor(color,
						&Scr->TitleC.fore, $2); }
		| TITLE_FOREGROUND string { GetColor(color,
						&Scr->TitleC.fore, $2);
					    list = &Scr->TitleForegroundL; }
		  win_color_list
		| TITLE_BACKGROUND string { GetColor(color,
						&Scr->TitleC.back, $2); }
		| TITLE_BACKGROUND string { GetColor(color,
						&Scr->TitleC.back, $2);
					    list = &Scr->TitleBackgroundL; }
		  win_color_list
		| DEFAULT_FOREGROUND string { GetColor(color,
						&Scr->DefaultC.fore, $2); }
		| DEFAULT_BACKGROUND string { GetColor(color,
						&Scr->DefaultC.back, $2); }
		| ICON_FOREGROUND string { GetColor(color,
						&Scr->IconC.fore, $2); }
		| ICON_FOREGROUND string { GetColor(color,
						&Scr->IconC.fore, $2);
					    list = &Scr->IconForegroundL; }
		  win_color_list
		| ICON_BACKGROUND string { GetColor(color,
						&Scr->IconC.back, $2); }
		| ICON_BACKGROUND string { GetColor(color,
						&Scr->IconC.back, $2);
					    list = &Scr->IconBackgroundL; }
		  win_color_list
		| ICON_BORDER_COLOR string { GetColor(color,
						&Scr->IconBorderColor, $2); }
		| ICON_BORDER_COLOR string { GetColor(color,
						&Scr->IconBorderColor, $2);
					    list = &Scr->IconBorderColorL; }
		  win_color_list
		| MENU_FOREGROUND string { GetColor(color,
						&Scr->MenuC.fore, $2); }
		| MENU_BACKGROUND string { GetColor(color,
						&Scr->MenuC.back, $2); }
		| MENU_TITLE_FOREGROUND string { GetColor(color,
						&Scr->MenuTitleC.fore, $2); }
		| MENU_TITLE_BACKGROUND string { GetColor(color,
						&Scr->MenuTitleC.back, $2); }
		| MENU_SHADOW_COLOR string { GetColor(color,
						&Scr->MenuShadowColor, $2); }
		| ICONMGR_FOREGROUND string { GetColor(color,
						&Scr->IconManagerC.fore, $2);
						list = &Scr->IconManagerFL; }
		  win_color_list
		| ICONMGR_FOREGROUND string { GetColor(color,
						&Scr->IconManagerC.fore, $2);}
		| ICONMGR_BACKGROUND string { GetColor(color,
						&Scr->IconManagerC.back, $2);
						list = &Scr->IconManagerBL; }
		  win_color_list
		| ICONMGR_BACKGROUND string { GetColor(color,
						&Scr->IconManagerC.back, $2);}
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
iconm_list	: LB iconm_entries RB
		;

iconm_entries	: /* Empty */
		| iconm_entries iconm_entry
		;

iconm_entry	: string string number	{ if (Scr->FirstTime)
					    AddToList(list, $1,
						AllocateIconManager($1, "",
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
action		: F_NOP			{ $$ = F_NOP; }
		| F_BEEP		{ $$ = F_BEEP; }
		| F_RESTART		{ $$ = F_RESTART; }
		| F_QUIT		{ $$ = F_QUIT; }
		| F_FOCUS		{ $$ = F_FOCUS; }
		| F_REFRESH		{ $$ = F_REFRESH; }
		| F_WINREFRESH		{ $$ = F_WINREFRESH; }
		| F_SOURCE string	{ Action = $2; $$ = F_TWMRC; }
		| F_DELTASTOP		{ $$ = F_DELTASTOP; }
		| F_MOVE		{ $$ = F_MOVE; }
		| F_FORCEMOVE		{ $$ = F_FORCEMOVE; }
		| F_AUTORAISE		{ $$ = F_AUTORAISE; }
		| F_IDENTIFY		{ $$ = F_IDENTIFY; }
		| F_ICONIFY		{ $$ = F_ICONIFY; }
		| F_DEICONIFY		{ $$ = F_DEICONIFY; }
		| F_UNFOCUS		{ $$ = F_UNFOCUS; }
		| F_RESIZE		{ $$ = F_RESIZE; }
		| F_ZOOM		{ $$ = F_ZOOM; }
                | F_LEFTZOOM            { $$ = F_LEFTZOOM; }
                | F_RIGHTZOOM           { $$ = F_RIGHTZOOM; }
                | F_TOPZOOM             { $$ = F_TOPZOOM; }
                | F_BOTTOMZOOM          { $$ = F_BOTTOMZOOM; }
		| F_HORIZOOM		{ $$ = F_ZOOM; }
		| F_FULLZOOM		{ $$ = F_FULLZOOM; }
		| F_RAISE		{ $$ = F_RAISE; }
		| F_RAISELOWER		{ $$ = F_RAISELOWER; }
		| F_LOWER		{ $$ = F_LOWER; }
		| F_DESTROY		{ $$ = F_DESTROY; }
		| F_TWMRC		{ $$ = F_TWMRC; }
		| F_VERSION		{ $$ = F_VERSION; }
		| F_TITLE		{ $$ = F_TITLE; }
		| F_RIGHTICONMGR	{ $$ = F_RIGHTICONMGR; }
		| F_LEFTICONMGR		{ $$ = F_LEFTICONMGR; }
		| F_UPICONMGR		{ $$ = F_UPICONMGR; }
		| F_DOWNICONMGR		{ $$ = F_DOWNICONMGR; }
		| F_FORWICONMGR		{ $$ = F_FORWICONMGR; }
		| F_BACKICONMGR		{ $$ = F_BACKICONMGR; }
		| F_NEXTICONMGR		{ $$ = F_NEXTICONMGR; }
		| F_PREVICONMGR		{ $$ = F_PREVICONMGR; }
		| F_SORTICONMGR		{ $$ = F_SORTICONMGR; }
		| F_CIRCLEUP		{ $$ = F_CIRCLEUP; }
		| F_CIRCLEDOWN		{ $$ = F_CIRCLEDOWN; }
		| F_CUTFILE		{ $$ = F_CUTFILE; }
		| F_SHOWLIST		{ $$ = F_SHOWLIST; }
		| F_HIDELIST		{ $$ = F_HIDELIST; }
		| F_WARPTOICONMGR 	{ $$ = F_WARPTOICONMGR; }
		| F_MENU string		{ pull = GetRoot($2, 0, 0);
					  pull->prev = root;
					  $$ = F_MENU;
					}
		| F_WARPTO string	{ Action = $2; $$ = F_WARPTO; }
		| F_WARPTOICONMGR string { Action = $2; $$ = F_WARPTOICONMGRE; }
		| F_FILE string		{ Action = $2; $$ = F_FILE; }
		| F_EXEC string		{ Action = $2; $$ = F_EXEC; }
		| F_CUT string		{ Action = $2; $$ = F_CUT; }
		| F_FUNCTION string	{ Action = $2; $$ = F_FUNCTION; }
		| F_WARPSCREEN string	{ Action = $2; 
					  if (CheckWarpScreenArg (Action)) {
					      $$ = F_WARPSCREEN;
					  } else {
					      fprintf (stderr, 
	"twm: line %d:  ignoring invalid f.warpscreen argument \"%s\"\n", 
						       yylineno, Action);
					      $$ = F_NOP;
					  }
					}
		;

grav		: NORTH			{ $$ = NORTH; }
		| SOUTH			{ $$ = SOUTH; }
		| EAST			{ $$ = EAST; }
		| WEST			{ $$ = WEST; }
		;

geometry	: plus_minus_number plus_minus_number {
					printf("(%d, %d)\n", num[0], num[1]);
					indx = 0;
					}
		;
plus_minus_number: plus_minus number	{ num[indx++] = mult * $2; }
		;

plus_minus	: plus
		| minus
		;

plus		: PLUS			{ mult = 1; }
		;
minus		: MINUS			{ mult = -1; }
		;

button		: BUTTON		{ $$ = $1;
					  if ($1 == 0)
						yyerror();

					  if ($1 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror();
					  }
					}
		;
tbutton		: TBUTTON		{ $$ = $1;
					  if ($1 == 0)
						yyerror();

					  if ($1 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror();
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
    fprintf (stderr, "twm: line %d:  syntax error:  %s\n", yylineno,
	     s ? s : "");
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

GotButton(butt, func)
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

GotKey(key, func)
char *key;
int func;
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++)
    {
	if ((cont & (1 << i)) == 0)
	    continue;

	AddFuncKey(key, i, mods, func, Name, Action);
    }

    Action = "";
    pull = NULL;
    cont = 0;
    mods = 0;
}


static Bool CheckWarpScreenArg (s)
    register char *s;
{
    XmuCopyISOLatin1Lowered (s, s);
    if (strcmp (s, "forw") == 0 || strcmp (s, "back") == 0 ||
	strcmp (s, "prev") == 0) return True;

    for (; *s && isascii(*s) && isdigit(*s); s++) ;
    return (*s ? False : True);
}
