%{
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
 * $XConsortium: lex.l,v 1.33 89/05/11 16:18:09 jim Exp $
 *
 * .twmrc lex file
 *
 * 12-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

static char RCSinfo[]=
"$XConsortium: lex.l,v 1.33 89/05/11 16:18:09 jim Exp $";

#include <stdio.h>
#include "gram.h"

extern int ParseError;

%}
%k 6000
%e 4000
%p 9000
%n 4000
%a 4000

string				\"[^"]*\"
number				[0-9]+
button				[Bb][Uu][Tt][Tt][Oo][Nn]
tbutton				[Tt][Ii][Tt][Ll][Ee][Bb][Uu][Tt][Tt][Oo][Nn]
noraise				[Nn][Oo][Rr][Aa][Ii][Ss][Ee][Oo][Nn]
%%
"{"				{ return (LB); }
"}"				{ return (RB); }
"("				{ return (LP); }
")"				{ return (RP); }
"="				{ return (EQUALS); }
":"				{ return (COLON); }
"+"				{ return PLUS; }
"-"				{ return MINUS; }
"|"				{ return OR; }

{button}{number}		{ (void)sscanf(&yytext[6], "%d", &yylval.num);
				  return (BUTTON);
				}
{tbutton}{number}		{ (void)sscanf(&yytext[11], "%d", &yylval.num);
				  return (TBUTTON);
				}
[Mm][Ee][Nn][Uu]		{ return MENU; }
[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]  { return FUNCTION; }

m				{ return (META); }
s				{ return (SHIFT); }
c				{ return (CONTROL); }

window				{ return (WINDOW); }
icon				{ return (ICON); }
title				{ return (TITLE); }
root				{ return (ROOT); }
frame				{ return (FRAME); }
iconmgr				{ return (ICONMGR); }

w				{ return (WINDOW); }
i				{ return (ICON); }
t				{ return (TITLE); }
r				{ return (ROOT); }
f				{ return (FRAME); }
all				{ return (ALL) ; }

f.nop				{ return F_NOP; }
f.beep				{ return F_BEEP; }
f.quit				{ return F_QUIT; }
f.title				{ return F_TITLE ; }
f.menu				{ return F_MENU; }
f.unfocus			{ return F_UNFOCUS; }
f.refresh			{ return F_REFRESH; }
f.winrefresh			{ return F_WINREFRESH; }
f.file				{ return F_FILE; }
f.twmrc				{ return F_TWMRC; }
f.version			{ return F_VERSION; }
f.circleup			{ return F_CIRCLEUP; }
f.circledown			{ return F_CIRCLEDOWN; }
f.source			{ return F_SOURCE; }
f.cutfile			{ return F_CUTFILE; }
f.function			{ return F_FUNCTION; }
f.showiconmgr			{ return F_SHOWLIST; }
f.hideiconmgr			{ return F_HIDELIST; }
"!"				{ return F_EXEC; }
"^"				{ return F_CUT; }
f.deltastop			{ return F_DELTASTOP; }
f.move				{ return F_MOVE; }
f.forcemove			{ return F_FORCEMOVE; }
f.iconify			{ return F_ICONIFY; }
f.deiconify			{ return F_DEICONIFY; }
f.focus				{ return F_FOCUS; }
f.resize			{ return F_RESIZE; }
f.raise				{ return F_RAISE; }
f.raiselower			{ return F_RAISELOWER; }
f.lower				{ return F_LOWER; }
f.destroy			{ return F_DESTROY; }
f.zoom				{ return F_ZOOM; }
f.fullzoom			{ return F_FULLZOOM; }
f.horizoom			{ return F_HORIZOOM; }
f.upiconmgr			{ return F_UPICONMGR; }
f.downiconmgr			{ return F_DOWNICONMGR; }
f.lefticonmgr			{ return F_LEFTICONMGR; }
f.righticonmgr			{ return F_RIGHTICONMGR; }
f.forwiconmgr			{ return F_FORWICONMGR; }
f.backiconmgr			{ return F_BACKICONMGR; }
f.nexticonmgr			{ return F_NEXTICONMGR; }
f.previconmgr			{ return F_PREVICONMGR; }
f.sorticonmgr			{ return F_SORTICONMGR; }
f.warpto			{ return F_WARPTO; }
f.warptoiconmgr			{ return F_WARPTOICONMGR; }
f.autoraise			{ return F_AUTORAISE; }
f.identify			{ return F_IDENTIFY; }
f.restart			{ return F_RESTART; }

f.hzoom                       { return F_HORIZOOM; }
f.leftzoom                    { return F_LEFTZOOM; }
f.vlzoom                      { return F_LEFTZOOM; }
f.rightzoom                   { return F_RIGHTZOOM; }
f.vrzoom                      { return F_RIGHTZOOM; }
f.topzoom                     { return F_TOPZOOM; }
f.htzoom                      { return F_TOPZOOM; }
f.bottomzoom                  { return F_BOTTOMZOOM; }
f.hbzoom                      { return F_BOTTOMZOOM; }



t.nop				{ return F_NOP; }
t.raise				{ return F_RAISE; }
t.lower				{ return F_LOWER; }
t.move				{ return F_MOVE; }

{noraise}[Mm][Oo][Vv][Ee]	{ return NO_RAISE_ON_MOVE; }
{noraise}[Dd][Ee][Ii][Cc][Oo][Nn][Ii][Ff][Yy] { return NO_RAISE_ON_DEICONIFY; }
{noraise}[Rr][Ee][Ss][Ii][Zz][Ee] { return NO_RAISE_ON_RESIZE; }


[Cc][Uu][Rr][Ss][Oo][Rr][Ss]	{ return CURSORS; }
[Pp][Ii][Xx][Mm][Aa][Pp][Ss]	{ return PIXMAPS; }
[Ff][Rr][Aa][Mm][Ee]		{ return CUR_FRAME; }
[Tt][Ii][Tt][Ll][Ee]		{ return CUR_TITLE; }
[Ii][Cc][Oo][Nn]		{ return CUR_ICON; }
{button}			{ return CUR_BUTTON; }
[Mm][Oo][Vv][Ee]		{ return CUR_MOVE; }
[Rr][Ee][Ss][Ii][Zz][Ee]	{ return CUR_RESIZE; }
[Ww][Aa][Ii][Tt]		{ return CUR_WAIT; }
[Ss][Ee][Ll][Ee][Cc][Tt]	{ return CUR_SELECT; }
[Dd][Ee][Ss][Tt][Rr][Oo][Yy]	{ return CUR_KILL; }
[Ii][Cc][Oo][Nn][Mm][Gg][Rr]	{ return CUR_ICONMGR; }

[Nn][Oo][Gg][Rr][Aa][Bb][Ss][Ee][Rr][Vv][Ee][Rr]	{return NO_GRAB_SERVER;}
[Ii][Cc][Oo][Nn][Rr][Ee][Gg][Ii][Oo][Nn]	{ return ICON_REGION; }
[Nn][Oo][Rr][Tt][Hh]				{ return NORTH; }
[Ss][Oo][Uu][Tt][Hh]				{ return SOUTH; }
[Ee][Aa][Ss][Tt]				{ return EAST; }
[Ww][Ee][Ss][Tt]				{ return WEST; }
[Nn][Oo][Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ss] { return NO_ICONMGRS;}
[Ii][Nn][Tt][Ee][Rr][Pp][Oo][Ll][Aa][Tt][Ee][Mm][Ee][Nn][Uu][Cc][Oo][Ll][Oo][Rr][Ss] { return (INTERPOLATE_MENUS); }
[Nn][Oo][Tt][Ii][Tt][Ll][Ee][Hh][Ii][Gg][Hh][Ll][Ii][Gg][Hh][Tt] { return (NO_TITLE_HILITE); }
[Tt][Ii][Tt][Ll][Ee][Hh][Ii][Gg][Hh][Ll][Ii][Gg][Hh][Tt]	{ return (TITLE_HILITE); }
[Nn][Oo][Mm][Ee][Nn][Uu][Ss][Hh][Aa][Dd][Oo][Ww][Ss] { return (NO_MENU_SHADOWS);}
[Nn][Oo][Ss][Hh][Aa][Dd][Oo][Ww][Mm][Ee][Nn][Uu][Ss] { return (NO_MENU_SHADOWS);}
[Cc][Ll][Ii][Ee][Nn][Tt][Bb][Oo][Rr][Dd][Ee][Rr][Ww][Ii][Dd][Tt][Hh] { return (CLIENT_BORDERWIDTH); }
[Nn][Oo][Vv][Ee][Rr][Ss][Ii][Oo][Nn] { return (NO_VERSION); }
[Ss][Tt][Aa][Rr][Tt][Ii][Cc][Oo][Nn][Ii][Ff][Ii][Ee][Dd] { return (START_ICONIFIED); }
[Oo][Pp][Aa][Qq][Uu][Ee][Mm][Oo][Vv][Ee] { return OPAQUE_MOVE; }
[Mm][Oo][Vv][Ee][Dd][Ee][Ll][Tt][Aa] { return MOVE_DELTA; }
[Cc][Oo][Ll][Oo][Rr]		{ return (COLOR); }
[Mm][Oo][Nn][Oo][Cc][Hh][Rr][Oo][Mm][Ee] { return (MONOCHROME); }
[Nn][Oo][Tt][Ii][Tt][Ll][Ee][Ff][Oo][Cc][Uu][Ss] { return (NO_TITLE_FOCUS); }
[Nn][Oo][Hh][Ii][Gg][Hh][Ll][Ii][Gg][Hh][Tt] { return (NO_HILITE); }
[Zz][Oo][Oo][Mm]		{ return (ZOOM); }
[Uu][Nn][Kk][Nn][Oo][Ww][Nn][Ii][Cc][Oo][Nn] { return UNKNOWN_ICON; }
[Ww][Aa][Rr][Pp][Cc][Uu][Rr][Ss][Oo][Rr] { return WARPCURSOR; }
[Bb][Oo][Rr][Dd][Ee][Rr][Ww][Ii][Dd][Tt][Hh] { return BORDERWIDTH; }
[Ii][Cc][Oo][Nn][Bb][Oo][Rr][Dd][Ee][Rr][Ww][Ii][Dd][Tt][Hh] { return ICON_BORDERWIDTH; }
[Tt][Ii][Tt][Ll][Ee][Ff][Oo][Nn][Tt] { return (TITLE_FONT); }
[Mm][Ee][Nn][Uu][Ff][Oo][Nn][Tt] { return (MENU_FONT); }
[Ii][Cc][Oo][Nn][Ff][Oo][Nn][Tt] { return (ICON_FONT); }
[Rr][Ee][Ss][Ii][Zz][Ee][Ff][Oo][Nn][Tt] { return (RESIZE_FONT); }
[Nn][Oo][Tt][Ii][Tt][Ll][Ee]	{ return (NO_TITLE); }
[Mm][Aa][Kk][Ee][Tt][Ii][Tt][Ll][Ee]	{ return (MAKE_TITLE); }
[Aa][Uu][Tt][Oo][Rr][Aa][Ii][Ss][Ee]	{ return (AUTO_RAISE); }
[Ff][Oo][Rr][Cc][Ee][Ii][Cc][Oo][Nn][Ss]	{ return (FORCE_ICON); }
[Ii][Cc][Oo][Nn][Rr][Ee][Gg][Ii][Oo][Nn]	{ return (ICON_REGION); }
[Dd][Ee][Ff][Aa][Uu][Ll][Tt][Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn] {
			       return (DEFAULT_FUNCTION); }
[Ww][Ii][Nn][Dd][Oo][Ww][Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn] {
			       return (WINDOW_FUNCTION); }
[Ii][Cc][Oo][Nn][Ss]			{ return (ICONS); }
[Ii][Cc][Oo][Nn][Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy] {return (ICON_DIRECTORY);}
[Bb][Oo][Rr][Dd][Ee][Rr][Cc][Oo][Ll][Oo][Rr] { return (BORDER_COLOR); }
[Tt][Ii][Tt][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (TITLE_FOREGROUND); }
[Tt][Ii][Tt][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (TITLE_BACKGROUND); }
[dD][eE][fF][aA][uU][lL][tT][fF][oO][rR][eE][gG][rR][oO][uU][nN][dD]	{
				return (DEFAULT_FOREGROUND); }
[dD][eE][fF][aA][uU][lL][tT][bB][aA][cC][kK][gG][rR][oO][uU][nN][dD]	{
				return (DEFAULT_BACKGROUND); }
[Bb][Oo][Rr][Dd][Ee][Rr][Tt][Ii][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (BORDER_TILE_FOREGROUND); }
[Bb][Oo][Rr][Dd][Ee][Rr][Tt][Ii][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (BORDER_TILE_BACKGROUND); }
[Mm][Ee][Nn][Uu][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_FOREGROUND); }
[Mm][Ee][Nn][Uu][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_BACKGROUND); }
[Mm][Ee][Nn][Uu][Tt][Ii][Tt][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_TITLE_FOREGROUND); }
[Mm][Ee][Nn][Uu][Tt][Ii][Tt][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_TITLE_BACKGROUND); }
[Mm][Ee][Nn][Uu][Ss][Hh][Aa][Dd][Oo][Ww][Cc][Oo][Ll][Oo][Rr] {
				return (MENU_SHADOW_COLOR); }
[Ii][Cc][Oo][Nn][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (ICON_FOREGROUND); }
[Ii][Cc][Oo][Nn][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (ICON_BACKGROUND); }
[Ii][Cc][Oo][Nn][Bb][Oo][Rr][Dd][Ee][Rr][Cc][Oo][Ll][Oo][Rr] {
				return (ICON_BORDER_COLOR); }
[Dd][Oo][Nn][Tt][Mm][Oo][Vv][Ee][Oo][Ff][Ff] return (DONT_MOVE_OFF); }
[Dd][Ee][Cc][Oo][Rr][Aa][Tt][Ee][Tt][Rr][Aa][Nn][Ss][Ii][Ee][Nn][Tt][Ss] {
				return (DECORATE_TRANSIENTS); }
[Ii][Cc][Oo][Nn][Ii][Ff][Yy][Bb][Yy][Uu][Nn][Mm][Aa][Pp][Pp][Ii][Nn][Gg] { return (ICONIFY_BY_UNMAPPING); }
[Dd][Oo][Nn][Tt][Ii][Cc][Oo][Nn][Ii][Ff][Yy][Bb][Yy][Uu][Nn][Mm][Aa][Pp][Pp][Ii][Nn][Gg] { return (DONT_ICONIFY_BY_UNMAPPING); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Hh][Ii][Gg][Hh][Ll][Ii][Gg][Hh][Tt] { return (ICONMGR_HIGHLIGHT); }
[Ss][Oo][Rr][Tt][Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr] { return (SORT_ICONMGR); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ss] { return (ICONMGRS); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] { return (ICONMGR_FOREGROUND); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] { return (ICONMGR_BACKGROUND); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ff][Oo][Nn][Tt] { return (ICONMGR_FONT); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Gg][Ee][Oo][Mm][Ee][Tt][Rr][Yy] { return (ICONMGR_GEOMETRY); }
[Ss][Hh][Oo][Ww][Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr] { return (SHOW_ICONMGR); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Dd][Oo][Nn][Tt][Ss][Hh][Oo][Ww] { return ICONMGR_NOSHOW; }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ss][Hh][Oo][Ww] { return ICONMGR_SHOW; }
[Nn][Oo][Bb][Aa][Cc][Kk][Ii][Nn][Gg][Ss][Tt][Oo][Rr][Ee] { return NO_BACKINGSTORE; }
[Nn][Oo][Ss][Aa][Vv][Ee][Uu][Nn][Dd][Ee][Rr][Ss]? { return NO_SAVEUNDER; }
[Rr][Aa][Nn][Dd][Oo][Mm][Pp][Ll][Aa][Cc][Ee][Mm][Ee][Nn][Tt] { return RANDOM_PLACEMENT; }
[Rr][Ee][Ss][Tt][Aa][Rr][Tt][Pp][Rr][Ee][Vv][Ii][Oo][Uu][Ss][Ss][Tt][Aa][Tt][Ee] {
					return RESTART_PREVIOUS_STATE; }
{string}			{ yylval.ptr = (char *)yytext; return STRING; }
{number}			{ (void)sscanf(yytext, "%d", &yylval.num);
				  return (NUMBER);
				}
\#[^\n]*\n			{;}
[\n\t ]				{;}
.				{
				  fprintf(stderr,
					"twm: bad character `%s', line %d\n",
					yytext, yylineno);
				  ParseError = 1;
				}
%%
yywrap() { return(1);}

#undef unput(c)
#undef input()
#undef output(c)
#define unput(c)	TwmUnput(c)
#define input()		TwmInput()
#define output(c)	TwmOutput(c)
