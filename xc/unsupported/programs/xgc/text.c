/*
** xbench
**
** text.c
**
** How to make a text widget that returns a string when the cursor
** leaves its window.
*/

#include <X11/Intrinsic.h>
#include <X11/Form.h>
#include <X11/Label.h>
#include <X11/AsciiText.h>
#include <X11/StringDefs.h>
#include "xbench.h"

extern XStuff X;

/* the strings which are displayed on the screen, edited, and sent
   to interpret() */
static char textstrings[NUMTEXTWIDGETS][40] = {"0","6x10","0","1"};

static char oldtextstrings[NUMTEXTWIDGETS][40] = {"0","6x10","0","1"};

/* The labels displayed next to them */
static char *labels[NUMTEXTWIDGETS] = {"Line Width","Font","Foreground",
				       "Background"};

/* the first half of what gets sent to interpret() */
static char *names[NUMTEXTWIDGETS] = {"linewidth ","font ","foreground ",
				      "background "};

/* create_text_choice(w,type,length,width)
** ---------------------------------------
** Inside w (a form), creates an editable text widget of width width.  The
** user can enter a string of up to length characters.  type is one of
** the constants defined in xbench.h; it decides things like the label,
** what string will be displayed and edited, etc.  When the pointer leaves
** the widget, the widget does the appropriate thing with the text
** inside it; the user doesn't have to press an "enter" button or anything.
*/

void create_text_choice(w,type,length,width)
     Widget w;
     int type;
     int length, width;
{
  char translationtable[40];	/* for adding the new action (calling
				   WriteText() when the pointer leaves) */

  static XtActionsRec actionTable[] = {	/* likewise */
    {"WriteText",  WriteText}
  };

  static Arg labelargs[] = {
    {XtNborderWidth,   (XtArgVal) 0},
    {XtNjustify,       (XtArgVal) XtJustifyRight}
  };

  static Arg textargs[] = {
    {XtNeditType,   (XtArgVal) XttextEdit},
    {XtNstring,     (XtArgVal) NULL},
    {XtNlength,     (XtArgVal) NULL},
    {XtNwidth,      (XtArgVal) NULL},
    {XtNhorizDistance, (XtArgVal) 10},
    {XtNfromHoriz,  (XtArgVal) NULL},
    {XtNinsertPosition, (XtArgVal) NULL}
  };

  static Widget text;		/* the text widget */
  static Widget label;		/* the label widget */

  /* see the Xt Manual if you want to understand how this works... I
     don't feel like going through the whole thing.
     type is sent as an argument to WriteText() so it knows what string
     to do stuff with. */
  sprintf(translationtable,"<Leave>:WriteText(%d)",type);

  /* label uses type to find out what its title is */
  label = XtCreateManagedWidget(labels[type],labelWidgetClass,w,
				labelargs,XtNumber(labelargs));
  
  /* text uses type to find out what its string is */
  switch (type) {
  case TForeground:
    sprintf(textstrings[type],"%d",(int) X.gcv.foreground);
    sprintf(oldtextstrings[type],"%d",(int) X.gcv.foreground);
    break;
  case TBackground:
    sprintf(textstrings[type],"%d",(int) X.gcv.background);
    sprintf(oldtextstrings[type],"%d",(int) X.gcv.background);
    break;
  }
  textargs[1].value = (XtArgVal) textstrings[type];
  textargs[2].value = (XtArgVal) length;
  textargs[3].value = (XtArgVal) width;
  textargs[5].value = (XtArgVal) label;
  textargs[6].value = (XtArgVal) strlen(textstrings[type]);

  text = XtCreateManagedWidget("text",asciiStringWidgetClass,w,
			       textargs,XtNumber(textargs));

  /* like before, look in the Xt Manual for an explanation */
  XtAddActions(actionTable,XtNumber(actionTable));
  XtAugmentTranslations(text,XtParseTranslationTable(translationtable));
}

/* WriteText(w,event,params,num_params)
** ------------------------------------
** Makes an appropriate string and sends it off to interpret().
** It's an ActionProc, thus the funny arguments.
*/

/*ARGSUSED*/
void WriteText(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;
     int *num_params;
{
  char mbuf[80];
  int type;			/* which string # to send */

  type = atoi(params[0]);
  if (strcmp(textstrings[type],oldtextstrings[type])) {
    strcpy(oldtextstrings[type],textstrings[type]);
    sprintf(mbuf,names[type]);	/* the right first half */
    strcat(mbuf,textstrings[type]); /* the right second half */
    strcat(mbuf,"\n");		/* the right new line */
    interpret(mbuf);
  }
}
