%{
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
 * $XConsortium: lex.l,v 1.55 89/11/13 10:52:54 jim Exp $
 *
 * .twmrc lex file
 *
 * 12-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

static char RCSinfo[]=
"$XConsortium: lex.l,v 1.55 89/11/13 10:52:54 jim Exp $";

/* #include <stdio.h> */		/* lex already includes stdio.h */
#include "gram.h"
#include "parse.h"

extern int ParseError;

%}

string				\"([^"]|\\.)*\"
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
[a-zA-Z\.]+			{ return parse_keyword (yytext, &yylval.num); }

"!"				{ return F_EXEC; }
"^"				{ return F_CUT; }

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

#undef unput
#undef input
#undef output
#undef feof
#define unput(c)	(*twmUnputFunc)(c)
#define input()		(*twmInputFunc)()
#define output(c)	TwmOutput(c)
#define feof()		(1)
