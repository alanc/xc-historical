/*
 * Login.c
 */

# include <X11/Xos.h>
# include <stdio.h>
# include <X11/IntrinsicP.h>
# include <X11/StringDefs.h>
# include <X11/Xmu.h>
# include "LoginP.h"

char	defaultLoginTranslations[];

#define offset(field) XtOffset(LoginWidget,login.field)
#define goffset(field) XtOffset(Widget,core.field)

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRString, "400"},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRString, "200"},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(textpixel), XtRString, "Black"},
    {XtNpromptColor, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(promptpixel), XtRString, "Black"},
    {XtNgreetColor, XtCForeground, XtRPixel, sizeof(Pixel),
        offset(greetpixel), XtRString, "Black"},
    {XtNfont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (font), XtRString, XtDefaultFont},
    {XtNpromptFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (promptFont), XtRString, XtDefaultFont},
    {XtNgreetFont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
    	offset (greetFont), XtRString, XtDefaultFont},
    {XtNgreeting, XtCGreeting, XtRString, sizeof (char *),
    	offset(greeting), XtRString, "Welcome to the X Window System"},
    {XtNnamePrompt, XtCNamePrompt, XtRString, sizeof (char *),
	offset(namePrompt), XtRString, "Login: "},
    {XtNpasswdPrompt, XtCNamePrompt, XtRString, sizeof (char *),
	offset(passwdPrompt), XtRString, "Password: "},
    {XtNnotifyDone, XtCCallback, XtRFunction, sizeof (caddr_t),
	offset(notify_done), XtRFunction, (caddr_t) 0},
};

#undef offset
#undef goffset

# define X_INC(w)	((w)->login.font->max_bounds.width)
# define Y_INC(w)	((w)->login.font->max_bounds.ascent +\
			 (w)->login.font->max_bounds.descent)
# define LOGIN_PROMPT_W(w) (XTextWidth (w->login.promptFont,\
				 w->login.namePrompt,\
				 strlen (w->login.namePrompt)))
# define PASS_PROMPT_W(w) (XTextWidth (w->login.promptFont,\
				 w->login.passwdPrompt,\
				 strlen (w->login.passwdPrompt)))
# define PROMPT_W(w)	(max(LOGIN_PROMPT_W(w), PASS_PROMPT_W(w)))
# define GREET_X(w)	(3 * X_INC(w))
# define GREET_Y(w)	(w->login.greeting[0] ? 2 * Y_INC (w) : 0)
# define LOGIN_X(w)	(3 * X_INC(w))
# define LOGIN_Y(w)	(GREET_Y(w) + 2 * Y_INC(w))
# define LOGIN_W(w)	(w->core.width - 6 * X_INC(w))
# define LOGIN_H(w)	(3 * Y_INC(w) / 2)
# define LOGIN_TEXT_X(w)(LOGIN_X(w) + PROMPT_W(w))
# define PASS_X(w)	(LOGIN_X(w))
# define PASS_Y(w)	(LOGIN_Y(w) + 2 * Y_INC(w))
# define PASS_W(w)	(LOGIN_W(w))
# define PASS_H(w)	(LOGIN_H(w))
# define PASS_TEXT_X(w)	(PASS_X(w) + PROMPT_W(w))

static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();
static int repaint_window();
static void draw_it ();

static void ClassInitialize();

static int max (a,b) { return a > b ? a : b; }

static void
EraseName (w, cursor)
    LoginWidget	w;
    int		cursor;
{
    int	x;

    x = LOGIN_TEXT_X (w);
    if (cursor > 0)
	x += XTextWidth (w->login.font, w->login.data.name, cursor);
    XDrawString (XtDisplay(w), XtWindow (w), w->login.bgGC, x, LOGIN_Y(w),
		w->login.data.name + cursor, strlen (w->login.data.name + cursor));
}

static void
DrawName (w, cursor)
    LoginWidget	w;
    int		cursor;
{
    int	x;

    x = LOGIN_TEXT_X (w);
    if (cursor > 0)
	x += XTextWidth (w->login.font, w->login.data.name, cursor);
    XDrawString (XtDisplay(w), XtWindow (w), w->login.textGC, x, LOGIN_Y(w),
		w->login.data.name + cursor, strlen (w->login.data.name + cursor));
}

static void
realizeCursor (w, gc)
    LoginWidget	w;
    GC		gc;
{
    int	x, y;
    int height, width;

    switch (w->login.state) {
    case GET_NAME:
	x = LOGIN_TEXT_X (w);
	y = LOGIN_Y (w);
	height = w->login.font->max_bounds.ascent + w->login.font->max_bounds.descent;
	width = 1;
	if (w->login.cursor > 0)
	    x += XTextWidth (w->login.font, w->login.data.name, w->login.cursor);
	break;
    case GET_PASSWD:
	x = PASS_TEXT_X (w);
	y = PASS_Y (w);
	height = w->login.font->max_bounds.ascent + w->login.font->max_bounds.descent;
	width = 1;
	break;
    }
    XFillRectangle (XtDisplay (w), XtWindow (w), gc,
		    x, y - w->login.font->max_bounds.ascent, width, height);
}

static void
XorCursor (w)
    LoginWidget	w;
{
    realizeCursor (w, w->login.xorGC);
}

static void
EraseCursor (w)
    LoginWidget (w);
{
    realizeCursor (w, w->login.bgGC);
}

static void
draw_it (w)
    LoginWidget	w;
{
    EraseCursor (w);
    if (w->login.greeting[0])
	    XDrawString (XtDisplay (w), XtWindow (w), w->login.greetGC,
			GREET_X(w), GREET_Y(w),
			w->login.greeting, strlen (w->login.greeting));
    XDrawString (XtDisplay (w), XtWindow (w), w->login.promptGC,
		LOGIN_X(w), LOGIN_Y(w),
		w->login.namePrompt, strlen (w->login.namePrompt));
    XDrawString (XtDisplay (w), XtWindow (w), w->login.promptGC,
		PASS_X(w), PASS_Y(w),
		w->login.passwdPrompt, strlen (w->login.passwdPrompt));
    DrawName (w, 0);
    XorCursor (w);
    XSetInputFocus (XtDisplay (w), XtWindow (w), RevertToPointerRoot, CurrentTime);
}

static void
DeleteBackwardChar (ctx, event)
    LoginWidget ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    if (ctx->login.cursor > 0) {
	ctx->login.cursor--;
	switch (ctx->login.state) {
	case GET_NAME:
	    EraseName (ctx, ctx->login.cursor);
	    strcpy (ctx->login.data.name + ctx->login.cursor,
		    ctx->login.data.name + ctx->login.cursor + 1);
	    DrawName (ctx, ctx->login.cursor);
	    break;
	case GET_PASSWD:
	    strcpy (ctx->login.data.passwd + ctx->login.cursor,
		    ctx->login.data.passwd + ctx->login.cursor + 1);
	    break;
	}
    }
    XorCursor (ctx);	
}

static void
DeleteForwardChar (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	if (ctx->login.cursor < strlen (ctx->login.data.name)) {
	    EraseName (ctx, ctx->login.cursor);
	    strcpy (ctx->login.data.name + ctx->login.cursor,
		    ctx->login.data.name + ctx->login.cursor + 1);
	    DrawName (ctx, ctx->login.cursor);
	}
	break;
    case GET_PASSWD:
    	if (ctx->login.cursor < strlen (ctx->login.data.passwd)) {
	    strcpy (ctx->login.data.passwd + ctx->login.cursor,
		    ctx->login.data.passwd + ctx->login.cursor + 1);
	}
	break;
    }
    XorCursor (ctx);	
}

static void
MoveBackwardChar (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    if (ctx->login.cursor > 0)
    	ctx->login.cursor--;
    XorCursor (ctx);
}

static void
MoveForwardChar (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
    	if (ctx->login.cursor < strlen (ctx->login.data.name))
	    ++ctx->login.cursor;
	break;
    case GET_PASSWD:
    	if (ctx->login.cursor < strlen (ctx->login.data.passwd))
	    ++ctx->login.cursor;
	break;
    }
    XorCursor (ctx);
}

static void
MoveToBegining (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    ctx->login.cursor = 0;
    XorCursor (ctx);
}

static void
MoveToEnd (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
    	ctx->login.cursor = strlen (ctx->login.data.name);
	break;
    case GET_PASSWD:
    	ctx->login.cursor = strlen (ctx->login.data.passwd);
	break;
    }
    XorCursor (ctx);
}

static void
EraseToEndOfLine (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	EraseName (ctx, ctx->login.cursor);
	ctx->login.data.name[ctx->login.cursor] = '\0';
	break;
    case GET_PASSWD:
	ctx->login.data.passwd[ctx->login.cursor] = '\0';
	break;
    }
    XorCursor (ctx);
}

static void
EraseLine (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    MoveToBegining (ctx, event);
    EraseToEndOfLine (ctx, event);
}

static void
FinishField (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	ctx->login.state = GET_PASSWD;
	ctx->login.cursor = 0;
	break;
    case GET_PASSWD:
	ctx->login.state = DONE;
	ctx->login.cursor = 0;
	(*ctx->login.notify_done) (ctx, &ctx->login.data);
	break;
    }
    XorCursor (ctx);
}

ResetLogin (w)
    LoginWidget	w;
{
    EraseName (w, 0);
    w->login.cursor = 0;
    w->login.data.name[0] = '\0';
    w->login.data.passwd[0] = '\0';
    w->login.state = GET_NAME;
}

static void
InsertChar (ctx, event)
    LoginWidget	ctx;
    XEvent	*event;
{
    char strbuf[128];
    int  len;

    len = XLookupString (event, strbuf, sizeof (strbuf), 0, 0);
    if (len + ctx->login.cursor >= NAME_LEN - 1)
    	len = NAME_LEN - ctx->login.cursor - 2;
    if (len == 0)
	return;
    XorCursor (ctx);
    switch (ctx->login.state) {
    case GET_NAME:
	EraseName (ctx, ctx->login.cursor);
	bcopy (ctx->login.data.name + ctx->login.cursor,
	       ctx->login.data.name + ctx->login.cursor + len,
	       strlen (ctx->login.data.name + ctx->login.cursor) + 1);
	bcopy (strbuf, ctx->login.data.name + ctx->login.cursor, len);
	DrawName (ctx, ctx->login.cursor);
	ctx->login.cursor += len;
	break;
    case GET_PASSWD:
	bcopy (ctx->login.data.passwd + ctx->login.cursor,
	       ctx->login.data.passwd + ctx->login.cursor + len,
	       strlen (ctx->login.data.passwd + ctx->login.cursor) + 1);
	bcopy (strbuf, ctx->login.data.passwd + ctx->login.cursor, len);
	ctx->login.cursor += len;
	break;
    }
    XorCursor (ctx);
}

static void
ClassInitialize ()
{
}

/* ARGSUSED */
static void Initialize (greq, gnew)
    Widget greq, gnew;
{
    LoginWidget w = (LoginWidget)gnew;
    XtGCMask	valuemask;
    XGCValues	myXGCV;

    myXGCV.foreground = w->login.textpixel;
    myXGCV.background = w->core.background_pixel;
    valuemask = GCForeground | GCBackground;
    if (w->login.font) {
	myXGCV.font = w->login.font->fid;
	valuemask |= GCFont;
    }
    w->login.textGC = XtGetGC(gnew, valuemask, &myXGCV);
    myXGCV.foreground = w->core.background_pixel;
    w->login.bgGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->login.textpixel ^ w->core.background_pixel;
    myXGCV.function = GXxor;
    valuemask |= GCFunction;
    w->login.xorGC = XtGetGC (gnew, valuemask, &myXGCV);

    /*
     * Note that the second argument is a GCid -- QueryFont accepts a GCid and
     * returns the curently contained font.
     */

    if (w->login.font == NULL)
	w->login.font = XQueryFont (XtDisplay (w),
		XGContextFromGC (DefaultGCOfScreen (XtScreen (w))));

    if (w->login.promptFont == NULL)
        w->login.promptFont = w->login.font;

    if (w->login.greetFont == NULL)
    	w->login.greetFont = w->login.font;

    valuemask = GCForeground | GCBackground | GCFont;
    myXGCV.foreground = w->login.promptpixel;
    myXGCV.font = w->login.promptFont->fid;
    w->login.promptGC = XtGetGC (gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->login.greetpixel;
    myXGCV.font = w->login.greetFont->fid;
    w->login.greetGC = XtGetGC (gnew, valuemask, &myXGCV);

    w->login.data.name[0] = '\0';
    w->login.data.passwd[0] = '\0';
    w->login.state = GET_NAME;

}

 
static void Realize (gw, valueMask, attrs)
     Widget gw;
     XtValueMask *valueMask;
     XSetWindowAttributes *attrs;
{
    XtCreateWindow( gw, (unsigned)InputOutput, (Visual *)CopyFromParent,
		     *valueMask, attrs );
}

static void Destroy (gw)
     Widget gw;
{
    LoginWidget w = (LoginWidget)gw;
    bzero (w->login.data.name, NAME_LEN);
    bzero (w->login.data.passwd, NAME_LEN);
    XtDestroyGC (w->login.textGC);
    XtDestroyGC (w->login.bgGC);
    XtDestroyGC (w->login.xorGC);
    XtDestroyGC (w->login.promptGC);
    XtDestroyGC (w->login.greetGC);
}

/* ARGSUSED */
static void Redisplay(gw, event, region)
     Widget gw;
     XEvent *event;
     Region region;
{
    draw_it ((LoginWidget) gw);
}

static Boolean SetValues ()
{
}

char defaultLoginTranslations [] =
"\
Ctrl<Key>H:	delete-previous-character() \n\
Ctrl<Key>D:	delete-character() \n\
Ctrl<Key>B:	move-backward-character() \n\
Ctrl<Key>F:	move-forward-character() \n\
Ctrl<Key>A:	move-to-begining() \n\
Ctrl<Key>E:	move-to-end() \n\
Ctrl<Key>K:	erase-to-end-of-line() \n\
Ctrl<Key>U:	erase-line() \n\
Ctrl<Key>X:	erase-line() \n\
<Key>BackSpace:	delete-previous-character() \n\
<Key>Delete:	delete-previous-character() \n\
<Key>Return:	finish-field() \n\
<Key>:		insert-char() \
";

XtActionsRec loginActionsTable [] = {
  {"delete-previous-character",	DeleteBackwardChar},
  {"delete-character",		DeleteForwardChar},
  {"move-backward-character",	MoveBackwardChar},
  {"move-forward-character",	MoveForwardChar},
  {"move-to-begining",		MoveToBegining},
  {"move-to-end",		MoveToEnd},
  {"erase-to-end-of-line",	EraseToEndOfLine},
  {"erase-line",		EraseLine},
  {"finish-field", FinishField},
  {"insert-char", InsertChar},
};

LoginClassRec loginClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Login",
    /* size			*/	sizeof(LoginRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	loginActionsTable,
    /* num_actions		*/	XtNumber (loginActionsTable),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultLoginTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
    }
};

WidgetClass loginWidgetClass = (WidgetClass) &loginClassRec;

