/*
** lex file for xgc syntax
*/

/* Lots of stuff stolen from gwm's wool.lex */

%{

#ifdef notdef

#undef output
#undef input
#undef unput

typedef struct {
  char *buffer;
  char *ptr;
  char *last;
  int (*overflow_handler) ();
} *STRING_STREAM;

static int yyin_is_string;      /* 0 ==> reading from STREAM yyin */
                                /* 1 ==> reading from STRING yystrin */
static int yyout_is_string;     /* 0 ==> printing on STREAM yyout */
                /* 1 ==> printing on WOOL_STRING_STREAM yystrout */
static char *yystrin;           /* current pointer on input string */
static STRING_STREAM yystrout; /* current wool output stream */

#define output(c) {\
    if(yyout_is_string){\
        *(yystrout->ptr)++ = c;\
        *(yystrout->ptr) = '\0';\
        if((yystrout->ptr) >= (yystrout->last)){\
            yyoutflush();\
        }\
    }else{\
        putc(c,yyout);\
    }\
}

#define input() (((\
        yytchar =\
        (yysptr>yysbuf ?\
            U(*--yysptr)\
        :   (yyin_is_string ?\
                ( *yystrin ? *yystrin++ : 0)\
            :   getc(yyin))))\
        ==10 ?\
            (yylineno++,yytchar)\
        :   yytchar)\
    ==EOF ?\
        0\
    :   yytchar)

#define unput(c) {\
    yytchar= (c);\
    if(yytchar=='\n')\
        yylineno--;\
    *yysptr++=yytchar;\
}
#endif

#include <X11/X.h>
#include "gram.h"
#include "constants.h"

extern int yylval;
%}

%p				3000
%a				3000

number				[0-9]+
word				[^\n\t ]+

%%

[Rr]un				{ return (RUN); }

[Ff]unction			{ return (FUNCTION); }
[Cc]lear			{ yylval = GXclear;
				  return (FUNCTIONTYPE); }
[Aa]nd				{ yylval = GXand;
				  return (FUNCTIONTYPE); }
[Aa]ndReverse			{ yylval = GXandReverse;
				  return (FUNCTIONTYPE); }
[Cc]opy				{ yylval = GXcopy;
				  return (FUNCTIONTYPE); }
[Aa]ndInverted			{ yylval = GXandInverted;
				  return (FUNCTIONTYPE); }
[Nn]oop				{ yylval = GXnoop;
				  return (FUNCTIONTYPE); }
[Xx]or				{ yylval = GXxor;
				  return (FUNCTIONTYPE); }
[Oo]r				{ yylval = GXor;
				  return (FUNCTIONTYPE); }
[Nn]or				{ yylval = GXnor;
				  return (FUNCTIONTYPE); }
[Ee]quiv			{ yylval = GXequiv;
				  return (FUNCTIONTYPE); }
[Ii]nvert			{ yylval = GXinvert;
				  return (FUNCTIONTYPE); }
[Oo]rReverse			{ yylval = GXorReverse;
				  return (FUNCTIONTYPE); }
[Cc]opyInverted			{ yylval = GXcopyInverted;
				  return (FUNCTIONTYPE); }
[Oo]rInverted			{ yylval = GXorInverted;
				  return (FUNCTIONTYPE); }
[Nn]and				{ yylval = GXnand;
				  return (FUNCTIONTYPE); }
[Ss]et				{ yylval = GXset;
				  return (FUNCTIONTYPE); }

[Tt]est				{ return (TEST); }
CopyArea			{ yylval = CopyArea;
				  return (TESTTYPE); }
CopyPlane			{ yylval = CopyPlane;
				  return (TESTTYPE); }
PolyPoint			{ yylval = PolyPoint;
				  return (TESTTYPE); }
PolyLine			{ yylval = PolyLine;
				  return (TESTTYPE); }
PolySegment			{ yylval = PolySegment;
				  return (TESTTYPE); }
PolyRectangle			{ yylval = PolyRectangle;
				  return (TESTTYPE); }
PolyArc				{ yylval = PolyArc;
				  return (TESTTYPE); }
FillPolygon			{ yylval = FillPolygon;
				  return (TESTTYPE); }
PolyFillRect			{ yylval = PolyFillRect;
				  return (TESTTYPE); }
PolyFillArc			{ yylval = PolyFillArc;
				  return (TESTTYPE); }
PutImage			{ yylval = PutImage;
				  return (TESTTYPE); }
GetImage			{ yylval = GetImage;
				  return (TESTTYPE); }
PolyText8			{ yylval = PolyText8;
				  return (TESTTYPE); }
ImageText8			{ yylval = ImageText8;
				  return (TESTTYPE); }
PolyText16			{ yylval = PolyText16;
				  return (TESTTYPE); }
ImageText16			{ yylval = ImageText16;
				  return (TESTTYPE); }

[Ll]inestyle			{ return (LINESTYLE); }
OnOffDash			{ yylval = LineOnOffDash;
				  return (LINESTYLETYPE); }
DoubleDash			{ yylval = LineDoubleDash;
				  return (LINESTYLETYPE); }

[Cc]apstyle			{ return (CAPSTYLE); }
NotLast				{ yylval = CapNotLast;
				  return (CAPSTYLETYPE); }
Butt				{ yylval = CapButt;
				  return (CAPSTYLETYPE); }
Projecting			{ yylval = CapProjecting;
				  return (CAPSTYLETYPE); }

[Jj]oinstyle			{ return (JOINSTYLE); }
Miter				{ yylval = JoinMiter;
				  return (JOINSTYLETYPE); }
Bevel				{ yylval = JoinBevel;
				  return (JOINSTYLETYPE); }

Round				{ return (ROUND); }

[Ff]illstyle			{ return (FILLSTYLE); }
Tiled				{ yylval = FillTiled;
				  return (FILLSTYLETYPE); }
Stippled			{ yylval = FillStippled;
				  return (FILLSTYLETYPE); }
OpaqueStippled			{ yylval = FillOpaqueStippled;
				  return (FILLSTYLETYPE); }

Solid				{ return (SOLID); }

[Ff]illrule			{ return (FILLRULE); }
EvenOdd				{ yylval = EvenOddRule;
				  return (FILLRULETYPE); }
Winding				{ yylval = WindingRule;
				  return (FILLRULETYPE); }

[Aa]rcmode			{ return (ARCMODE); }
Chord				{ yylval = ArcChord;
				  return (ARCMODETYPE); }
PieSlice			{ yylval = ArcPieSlice;
				  return (ARCMODETYPE); }

[Ff]oreground			{ return (FOREGROUND); }
[Bb]ackground			{ return (BACKGROUND); }
[Ll]inewidth			{ return (LINEWIDTH); }
[Pp]lanemask			{ return (PLANEMASK); }
[Dd]ashlist			{ return (DASHLIST); }

{number}			{ (void) sscanf (yytext, "%d", &yylval);
				  return (NUMBER); }
\#[^\n]*\n			{ ; }
[\t ]				{ ; }
\n				{ return ('\n'); }

{word}				{ fprintf(stderr,
					"xgc: bad word `%s', line %d\n",
					yytext, yylineno); }

.				{ fprintf(stderr,
					"xgc: bad character `%s', line %d\n",
					yytext, yylineno); }

%%

yywrap() { return (1); }

#ifdef notdef
/*
 * yyinflush
 * to flush wool input buffers (i.e. the unput buffer)
 * stores old input buffer in arg (should be ~ 16) if not NULL
 */

yyinflush(wool_old_unput_buffer)
char *wool_old_unput_buffer;
{
    if (wool_old_unput_buffer) {
        if (yysptr != yysbuf)
            strncpy(wool_old_unput_buffer, yysbuf, yysptr - yysbuf);
        wool_old_unput_buffer[yysptr - yysbuf] = '\0';
    }
    yysptr = yysbuf;
}

/*
 * yyoutflush
 * to flush wool output buffer.
 */

yyoutflush(){
    if(yyout_is_string){
        (*(yystrout->overflow_handler))(yystrout->buffer);
        yystrout->ptr = yystrout-> buffer;
    }else{
        fflush(yyout);
    }
}

/*
 * wool_input_redirect
 * to set wool's parsing to the stream or string argument.
 * arg1 = type (0 = string, 1 = stream);
 * arg2 = stream or string
 * arg3 = POINTER to (FILE *) or (char *) where will go the old stream
 *        (if NULL not set)(returns the old type)
 * arg4 = where to save contents of unput buffer (if not NULL)
 */

int wool_input_redirect(type, stream, oldstream_p, old_buffer_contents)
int type;
char *stream;
char **oldstream_p;
char *old_buffer_contents;
{
    int oldtype = yyin_is_string;
    if(oldstream_p) *oldstream_p =
        (oldtype ? (char *) yystrin : (char *) yyin);
    yyinflush(old_buffer_contents);
    if(yyin_is_string = type){
        yystrin = stream;
    }else{
        yyin = (FILE *) stream;
    }
    return oldtype;
}
#endif
