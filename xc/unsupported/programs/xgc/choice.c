/*
** xbench
**
** choice.c
**
** All the generic stuff for dealing with choice widgets.
*/

#include <stdio.h>
#include <X11/IntrinsicP.h>	/* we need to use this instead of Intrinsic.h
				   because we need to delve directly into
				   the ComopositeWidget structure to find
				   out what children a widget has */
#include <X11/Form.h>
#include <X11/Label.h>
#include <X11/Command.h>
#include <X11/StringDefs.h>

#include "xbench.h"

extern XStuff X;

/* create_choice(w,info)
** ---------------------
** What a choice widget is:  A collection of command buttons placed inside
**   a form widget.  Exactly one of these command buttons can be "on" at
**   any given time;  the rest are "off".  "On" command buttons have
**   the foreground and background colors reversed.
**   Also, specifically because it comes in handy in xbench, choosing one
**   of the buttons causes a string associated with it to be printed out
**   (and interpreted).  Half of the string is global to the whole form
**   and the other half is local to each button.
**   There's also a label widget to the left of that mess, with an
**   incredibly descriptive title.
**
** create_choice() makes one.
**
** w is the form widget (already created) into which we will place the
** command buttons.  info contains lots of useful information, such
** as the names of the buttons and their strings (see xbench.h).
*/

void create_choice(w,info)
     Widget      w;
     XbenchStuff *info;
{
  static Widget *commands;
  static Widget label;
  char          tmp[30];
  char          *text;
  int           i;
  Boolean       samewidth = FALSE;

  static XtCallbackRec callbacklist[] = {
    {(XtCallbackProc) select_button,         NULL},
    {(XtCallbackProc) print_text_to_buffer,  NULL},
    {NULL,                                   NULL}
  };

  static Arg labelargs[] = {
    {XtNborderWidth,  (XtArgVal) 0},
    {XtNjustify,      (XtArgVal) XtJustifyRight}, /* Unfortunately, this
						   seems to have no effect */
    {XtNvertDistance, (XtArgVal) 4}
  };

  static Arg commandargs[] = {
    {XtNfromHoriz,     (XtArgVal) NULL},
    {XtNfromVert,      (XtArgVal) NULL},
    {XtNhorizDistance, (XtArgVal) 4},
    {XtNvertDistance,  (XtArgVal) 4},
    {XtNcallback,      (XtArgVal) NULL},
    {XtNjustify,       (XtArgVal) XtJustifyCenter}
  };

  commands = (Widget *) malloc((unsigned)info->choice.num_commands 
			       * sizeof(Widget));

  label = XtCreateManagedWidget(info->choice.name,labelWidgetClass,w,
				labelargs,XtNumber(labelargs));

  commandargs[4].value = (XtArgVal) callbacklist;

  for (i=0;i<info->choice.num_commands;++i) { /* make the command widgets */
    if (i==0) {			/* setting its fromHoriz */
      commandargs[0].value = (XtArgVal) label; /* put the first one next to
						  the label */
      commandargs[1].value = (XtArgVal) NULL;
      commandargs[2].value = (XtArgVal) 10;
      commandargs[3].value = (XtArgVal) 4;
    }
    else {
                                /* Are we starting a new row? */
      if (info->choice.columns && i > 1 && !(i%info->choice.columns)) {
	commandargs[0].value = (XtArgVal) label;
	commandargs[1].value = (XtArgVal) commands[i-info->choice.columns];
	commandargs[2].value = (XtArgVal) 10;
	commandargs[3].value = (XtArgVal) 4;
      }
      else {			/* I guess not, just put it to the right of
				   the last one */
	commandargs[0].value = (XtArgVal) commands[i-1];
	commandargs[1].value = (XtArgVal) NULL;
	commandargs[2].value = (XtArgVal) -1; /* overlapping slightly */
	commandargs[3].value = (XtArgVal) 4;
      }
				/* Make sure its vertical spacing is correct */
      if (info->choice.columns && i >= info->choice.columns) {
	commandargs[1].value = (XtArgVal) commands[i-info->choice.columns];
	commandargs[3].value = (XtArgVal) -1;
      }
    }

    if (info->choice.columns) samewidth = TRUE; /* so we get a nice table */

    /* in the closure we put a string containing what should be put into
       the buffer when this command is executed.  There's a part that's
       common to all the command buttons in a choice widget and a part
       that's individual to each.  For example
          fillrule EvenOdd
       "fillrule" will be printed by every command button inside the FillRule
       choice, then each command button will print something different
       after it. */

    strcpy(tmp,info->choice.text);
    strcat(tmp," ");
    strcat(tmp,(*(info->data))[i].text);
    strcat(tmp,"\n");
    text = (char *) malloc(strlen(tmp) * sizeof(char) + 2);
    strcpy(text,tmp);
    
    callbacklist[1].closure = (caddr_t) text;

    /* so make the thing already */
    commands[i] = XtCreateManagedWidget((*info->data)[i].name,
	    				commandWidgetClass,w,
					commandargs,XtNumber(commandargs));
  }

    /* now make all the command buttons the same width if that's what we
     want to do */
  if (samewidth) {
    Dimension maxwidth = 0;
    Dimension width;
    static Arg args[] = {
      {XtNwidth,        (XtArgVal) NULL}
    };
    
    args[0].value = (XtArgVal) &width;

    for (i=0;i<info->choice.num_commands;++i) { /* for each command */
      XtGetValues(commands[i],args,1);   /* find its width */
      maxwidth = max(maxwidth,width);
    }
    
    args[0].value = (XtArgVal) maxwidth;
				/* Now set all of their widths to the
				   max width */
    for (i=0;i<info->choice.num_commands;++i)
      XtSetValues(commands[i],args,1);
  }
}

/* select_button(w,closure,call_data)
** ----------------------------------
** This is called every time a button inside a choice widget is selected.
** It does the job of making the button's "on"ness visible to the user.
** It reverses the foreground and background of the selected button
** and returns the foreground and background of all its siblings to
** their normal values.
**
** The funny arguments are because it's a callback procedure.
*/

/*ARGSUSED*/
void select_button(w,closure,call_data)
     Widget w;			/* the widget that was selected */
     caddr_t closure;
     caddr_t call_data;
{
  CompositeWidget parent;	/* the parent (form widget) of the button
				   that was selected */
  Widget cur_widget;		/* the widget we're talking about while
				   we're in the loop */
  static Arg args[] = {
    {XtNforeground,          (XtArgVal) NULL },
    {XtNbackground,          (XtArgVal) NULL },
    {XtNhighlightThickness,  (XtArgVal) NULL } 
  };
  int num_children;		/* number of children the parent widget has */
  int i;			/* counter */

  parent = (CompositeWidget) XtParent(w);
  num_children = parent->composite.num_children;

  for (i=0;i<num_children;++i) { /* go through the siblings one by one */
    cur_widget = parent->composite.children[i];
    if (cur_widget == w) {	/* is the one the user selected? */
      args[0].value = X.background;
      args[1].value = X.foreground;
      args[2].value = 0;	/* because otherwise it looks funny */
    }
    else {			/* make the other ones look normal */
      args[0].value = X.foreground;
      args[1].value = X.background;
      args[2].value = 2;
    }
    XtSetValues(cur_widget,args,XtNumber(args));
  }
}

/* get_label_width(w)
** ------------------
** Returns the width of the label contained in the form widget w.  The
** label must be the first child of the form, since that's how it
** gets the info.
*/

int get_label_width(w)
     Widget w;
{
  CompositeWidget cw;		/* so we can deal with its children */
  Dimension width;
  static Arg args[] = {		/* looking at its width */
    {XtNwidth,               (XtArgVal) NULL}
  };

  args[0].value = (XtArgVal) &width;

  cw = (CompositeWidget) w;	/* w should be a form so the cast is OK */
  XtGetValues(cw->composite.children[0],args,1); /* children[0] because
						    the label is the first
						    widget inside it */
  return (width);
}

/* set_label_width(w,width)
** ------------------------
** Sets the width of w's first child (hopefully a label) to 'width'.
*/

void set_label_width(w,width)
     Widget w;
{
  CompositeWidget cw;		/* so we can deal with its children */
  static Arg args[] = {
    {XtNwidth,               (XtArgVal) NULL }
  };

  cw = (CompositeWidget) w;	/* w is a form so the cast is OK */
  args[0].value = (XtArgVal) width;
  XtSetValues(cw->composite.children[0],args,XtNumber(args));
				/* children[0] because the label is the
				   first widget inside it */
}

/* line_up_labels(w,num_choicegroups)
** ----------------------------------
** For the first num_choicegroups forms in the widget w, looks at their
** labels and then makes all of their widths equal to the widest
** label's width.  The result is an aesthetically pleasing column.
*/

void line_up_labels(w,num_choicegroups)
     Widget w;
     int num_choicegroups;
{
  int i;
  int maxwidth = 0, width;

  /* find out what the largest label width is */
  for (i=0;i<num_choicegroups;++i) {
    width = get_label_width(((CompositeWidget)w)->composite.children[i]);
				/* this argument is the i'th child of w */
    maxwidth = max(width,maxwidth);
  }

  /* now set all of the labels to that width */
  for (i=0;i<num_choicegroups;++i) 
    set_label_width(((CompositeWidget)w)->composite.children[i],maxwidth);
}

/* choose_defaults(w,num_choicegroups)
** -----------------------------------
** Goes through the first num_choicegroups choice widgets in the widget w,
** and calls the callbacks of their first children, so that something
** will be selected before the user does anything.
*/

void choose_defaults(w,num_choicegroups)
     Widget w;
     int    num_choicegroups;
{
  int i,j;
 
  for (i=0;i<num_choicegroups;++i) {
    if (i==0) j = 4; else j = 1; /* start with #4 for function */
    XtCallCallbacks(
	  ((CompositeWidget)((CompositeWidget)w)->composite.children[i])
		     ->composite.children[j],
	  XtNcallback,NULL);
  /* That first argument translates to the second child of the i'th child
     of w (the first child is the label).  Trust me. */
  }
}

/* print_text_to_buffer(w,closure,call_data)
** -----------------------------------------
** This is also in the list of callbacks for the command buttons in a
** choice widget.  It sends the string contained in closure (which
** was set way back in create_choice()) over to interpret(), which
** decides what to do with it.
*/

/*ARGSUSED*/
void print_text_to_buffer(w,closure,call_data)
     Widget  w;
     caddr_t closure;		/* contains the string */
     caddr_t call_data;
{
  interpret((char *) closure,FALSE);		/* Gee, that was easy */
}
