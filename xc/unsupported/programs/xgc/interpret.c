/*
** interpret.c
**
** interprets and executes lines in the Xgc syntax.
*/

#include "stdio.h"
#include <X11/IntrinsicP.h>
#include "xgc.h"

extern XgcStuff TestStuff;
extern XgcStuff FunctionStuff;
extern XgcStuff LinestyleStuff;
extern XgcStuff CapstyleStuff;
extern XgcStuff JoinstyleStuff;
extern XgcStuff FillstyleStuff;
extern XgcStuff FillruleStuff;
extern XgcStuff ArcmodeStuff;

extern XStuff X;
extern Widget test;
extern Widget GCform;
extern Widget foregroundchoice;
extern Widget dashlistchoice;
extern Widget planemaskchoice;
extern Widget testchoiceform;
extern void change_text();

/* interpret(string)
** -----------------
** Takes string, which is a line written in the xgc syntax, figures
** out what it means, and passes the buck to the right procedure.
** This is a _very_ primitive parser...
*/
void interpret(string)
     String string;
{
  char word1[20], word2[80];
  int i;

  sscanf(string,"%s",word1);
  if (!strcmp(word1,"run")) run_test();

  else {
    sscanf(string,"%s %s",word1,word2);
    print_if_recording(string);

    /* So word1 is the first word on the line and word2 is the second.
       Now the fun begins... */
    
    if (!strcmp(word1,TestStuff.choice.text))  {
      for (i=0;i<NUM_TESTS;++i) {
	if (!strcmp(word2,(TestStuff.data)[i].text)) {
	  change_test((TestStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FunctionStuff.choice.text)) {
      for (i=0;i<NUM_FUNCTIONS;++i) {
	if (!strcmp(word2,(FunctionStuff.data)[i].text)) {
	  GC_change_function((FunctionStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,LinestyleStuff.choice.text)) {
      for (i=0;i<NUM_LINESTYLES;++i) {
	if (!strcmp(word2,(LinestyleStuff.data)[i].text)) {
	  GC_change_linestyle((LinestyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,"linewidth"))
      GC_change_linewidth(atoi(word2),FALSE);
    else if (!strcmp(word1,CapstyleStuff.choice.text)) {
      for (i=0;i<NUM_CAPSTYLES;++i) {
	if (!strcmp(word2,(CapstyleStuff.data)[i].text)) {
	  GC_change_capstyle((CapstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,JoinstyleStuff.choice.text)) {
      for (i=0;i<NUM_JOINSTYLES;++i) {
	if (!strcmp(word2,(JoinstyleStuff.data)[i].text)) {
	  GC_change_joinstyle((JoinstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FillstyleStuff.choice.text)) {
      for (i=0;i<NUM_FILLSTYLES;++i) {
	if (!strcmp(word2,(FillstyleStuff.data)[i].text)) {
	  GC_change_fillstyle((FillstyleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,FillruleStuff.choice.text)) {
      for (i=0;i<NUM_FILLRULES;++i) {
	if (!strcmp(word2,(FillruleStuff.data)[i].text)) {
	  GC_change_fillrule((FillruleStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,ArcmodeStuff.choice.text)) {
      for (i=0;i<NUM_ARCMODES;++i) {
	if (!strcmp(word2,(ArcmodeStuff.data)[i].text)) {
	  GC_change_arcmode((ArcmodeStuff.data)[i].code,FALSE);
	  break;
	}
      }
    }
    else if (!strcmp(word1,"planemask")) 
      GC_change_planemask((unsigned int) atoi(word2),FALSE);
    else if (!strcmp(word1,"dashlist"))
      GC_change_dashlist(atoi(word2),FALSE);
    else if (!strcmp(word1,"font"))
      GC_change_font(word2);
    else if (!strcmp(word1,"foreground"))
      GC_change_foreground((unsigned int) atoi(word2),FALSE);
    else if (!strcmp(word1,"background"))
      GC_change_background((unsigned int) atoi(word2),FALSE);
    else fprintf(stderr,"Ack... %s %s\n",word1,word2);
  }
}

#define select_correct_button(x,n) \
select_button(((CompositeWidget) \
((CompositeWidget)GCform)->composite.children[x])->composite.children[n+1], \
(caddr_t) NULL, (caddr_t) NULL);

void GC_change_function(function,feedback)
     int function;
     Boolean feedback;
{
  XSetFunction(X.dpy,X.gc,function);
  X.gcv.function = function;
  if (feedback) select_correct_button(CFunction,function);
}

void GC_change_foreground(foreground,feedback)
     unsigned long foreground;
     Boolean feedback;
{
  char text[40];

  XSetForeground(X.dpy,X.gc,foreground);
  X.gcv.foreground = foreground;
  sprintf(text,"%d",foreground);
  change_text(foregroundchoice,TForeground,text);
}

void GC_change_background(background,feedback)
     unsigned long background;
     Boolean feedback;
{
  XSetBackground(X.dpy,X.gc,background);
  X.gcv.background = background;
  XSetWindowBackground(X.dpy,XtWindow(test),background);
  XClearWindow(X.dpy,XtWindow(test));
}

void GC_change_linewidth(linewidth,feedback)
     int linewidth;
     Boolean feedback;
{
  X.gcv.line_width = linewidth;
  XChangeGC(X.dpy,X.gc,GCLineWidth,&X.gcv);
}

void GC_change_linestyle(linestyle,feedback)
     int linestyle;
     Boolean feedback;
{
  X.gcv.line_style = linestyle;
  XChangeGC(X.dpy,X.gc,GCLineStyle,&X.gcv);
  if (feedback) select_correct_button(CLinestyle,linestyle);
}

void GC_change_capstyle(capstyle,feedback)
     int capstyle;
     Boolean feedback;
{
  X.gcv.cap_style = capstyle;
  XChangeGC(X.dpy,X.gc,GCCapStyle,&X.gcv);
  if (feedback) select_correct_button(CCapstyle,capstyle);
}

void GC_change_joinstyle(joinstyle,feedback)
     int joinstyle;
     Boolean feedback;
{
  X.gcv.join_style = joinstyle;
  XChangeGC(X.dpy,X.gc,GCJoinStyle,&X.gcv);
  if (feedback) select_correct_button(CJoinstyle,joinstyle);
}

void GC_change_fillstyle(fillstyle,feedback)
     int fillstyle;
     Boolean feedback;
{
  XSetFillStyle(X.dpy,X.gc,fillstyle);
  X.gcv.fill_style = fillstyle;
  if (feedback) select_correct_button(CFillstyle,fillstyle);
}

void GC_change_fillrule(fillrule,feedback)
     int fillrule;
     Boolean feedback;
{
  XSetFillRule(X.dpy,X.gc,fillrule);
  X.gcv.fill_rule = fillrule;
  if (feedback) select_correct_button(CFillrule,fillrule);
}

void GC_change_arcmode(arcmode,feedback)
     int arcmode;
     Boolean feedback;
{
  XSetArcMode(X.dpy,X.gc,arcmode);
  X.gcv.arc_mode = arcmode;
  if (feedback) select_correct_button(CArcmode,arcmode);
}

/* GC_change_dashlist(dashlist)
** ----------------------------
** Now this one's a bit tricky.  dashlist gets passed in as an int, but we
** want to change it to an array of chars, like the GC likes it.
** For example:
**     119 => XXX_XXX_ => [3,1,3,1]
*/

void GC_change_dashlist(dashlist,feedback) 
     int dashlist;
     Boolean feedback;
{
  char dasharray[8];            /* what we're gonna pass to XSetDashes */
  int dashnumber = 0;		/* which element of dasharray we're currently
				   modifying */
  int i;			/* which bit of dashlist we're on */
  int state = 1;		/* whether the last bit we checked was on (1)
				   or off (0) */

  for (i=0;i<8;++i) dasharray[i] = 0; /* initialize dasharray */

  if (dashlist == 0) return;	/* having no dashes at all is bogus */

  /* XSetDashes expects the dashlist to start with an on bit, so if it
     doesn't, we keep on rotating it until it does */
  while (!(dashlist&1)) dashlist /= 2;

  for (i=0;i<8;++i) {		/* i has 8 bits we want to look at */
    /* the following if statements checks to see if the bit we're looking
       at as the same on or offness as the one before it (state) */
    if (((dashlist&1<<i) && state) || (!(dashlist&1<<i) && !state))
      ++dasharray[dashnumber];	/* if so, increment the length of this dash */
    else {			
      state = state^1;		/* reverse the state */
      ++dasharray[++dashnumber]; /* start a new dash */
    }
  } 

  XSetDashes(X.dpy,X.gc,0,dasharray,dashnumber+1);
  X.gcv.dashes = dashlist;
  if (feedback) update_dashlist(dashlistchoice,dashlist);
}

void GC_change_planemask(planemask,feedback)
     unsigned long planemask;
     Boolean feedback;
{
  XSetPlaneMask(X.dpy,X.gc,planemask);
  X.gcv.plane_mask = planemask;
  if (feedback) update_planemask(planemaskchoice,planemask);
}

void change_test(test,feedback) 
     int test;
     Boolean feedback;
{
  X.test = test;
  if (feedback) select_button(((CompositeWidget) testchoiceform)
			      ->composite.children[test+1],
			      (caddr_t) NULL, (caddr_t) NULL);
}

void GC_change_font(str)
     String str;
{
  int num_fonts;		/* number of fonts that match the string
				  ( will be 1 or 0) */

  XListFonts(X.dpy,str,1,&num_fonts); /* see if the font exists */

  if (num_fonts) XSetFont(X.dpy,X.gc,XLoadFont(X.dpy,str));

}

