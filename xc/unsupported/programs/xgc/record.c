/*
** record.c
**
*/

#include <X11/Intrinsic.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/StringDefs.h>
#include <stdio.h>

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
extern Boolean recording;
extern Widget recordbutton;

extern char filename[40];

void cancel_record();
void done_choosing_filename();

FILE *recordfile;
FILE *playbackfile;
extern FILE *yyin;

/*ARGSUSED*/
void toggle_recordbutton(w,closure,call_data)
     Widget w;
     caddr_t closure;
     caddr_t call_data;
{
  static Arg recordargs[] = {
    {XtNlabel,        (XtArgVal) NULL}
  };
  
  char tmp[20];
  
  if (!recording) {
    start_recording();
  }
  else {
    recording = FALSE;
    stop_recording();
    sprintf(tmp,"Record");
    recordargs[0].value = (XtArgVal) tmp;
  }

  XtSetValues(recordbutton,recordargs,XtNumber(recordargs));
}

void start_recording() 
{
  get_filename(done_choosing_filename,cancel_record);
}


void stop_recording() 
{
  fclose(recordfile);
}

void cancel_record() 
{
}

void done_choosing_filename() 
{
  static Arg recordargs[] = {
    {XtNlabel,        (XtArgVal) NULL}
  };
  
  char tmp[20];
  
  if (recordfile = fopen(filename,"w")) {
    recording = TRUE;
    sprintf(tmp,"End Record");
    recordargs[0].value = (XtArgVal) tmp;
    XtSetValues(recordbutton,recordargs,XtNumber(recordargs));

    print_out_gc_values();
  }
}

void print_if_recording(str)
     char *str;
{
  if (recording)
    fprintf(recordfile,"%s",str);
}

void close_file_if_recording()
{
  if (recording)
    fclose(recordfile);
}

void print_out_gc_values()
{
  int i;
  for (i=0;i<NUM_TESTS;++i) {
    if ((TestStuff.data)[i].code == X.test) {
      fprintf(recordfile,"%s %s\n",
	      TestStuff.choice.text,(TestStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_FUNCTIONS;++i) {
    if ((FunctionStuff.data)[i].code == X.gcv.function) {
      fprintf(recordfile,"%s %s\n",
	      FunctionStuff.choice.text,(FunctionStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_LINESTYLES;++i) {
    if ((LinestyleStuff.data)[i].code == X.gcv.line_style) {
      fprintf(recordfile,"%s %s\n",
	      LinestyleStuff.choice.text,(LinestyleStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_CAPSTYLES;++i) {
    if ((CapstyleStuff.data)[i].code == X.gcv.cap_style) {
      fprintf(recordfile,"%s %s\n",
	      CapstyleStuff.choice.text,(CapstyleStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_JOINSTYLES;++i) {
    if ((JoinstyleStuff.data)[i].code == X.gcv.join_style) {
      fprintf(recordfile,"%s %s\n",
	      JoinstyleStuff.choice.text,(JoinstyleStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_FILLSTYLES;++i) {
    if ((FillstyleStuff.data)[i].code == X.gcv.fill_style) {
      fprintf(recordfile,"%s %s\n",
	      FillstyleStuff.choice.text,(FillstyleStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_FILLRULES;++i) {
    if ((FillruleStuff.data)[i].code == X.gcv.fill_rule) {
      fprintf(recordfile,"%s %s\n",
	      FillruleStuff.choice.text,(FillruleStuff.data)[i].text);
      break;
    }
  }
  for (i=0;i<NUM_ARCMODES;++i) {
    if ((ArcmodeStuff.data)[i].code == X.gcv.arc_mode) {
      fprintf(recordfile,"%s %s\n",
	      ArcmodeStuff.choice.text,(ArcmodeStuff.data)[i].text);
      break;
    }
  }
  fprintf(recordfile,"linewidth %d\n",X.gcv.line_width);
  fprintf(recordfile,"foreground %d\n",X.gcv.foreground);
  fprintf(recordfile,"background %d\n",X.gcv.background);
  fprintf(recordfile,"planemask %d\n",X.gcv.plane_mask);
  fprintf(recordfile,"dashlist %d\n",X.gcv.dashes);
  /* FONT */
}  

/********************************************/

void start_playback()
{
  get_filename(chose_playback_filename,cancel_playback);
}

void cancel_playback()
{
}

void chose_playback_filename()
{
  if (playbackfile = fopen(filename,"r")) {
    yyin = playbackfile;
    yyparse();
  }
}

void read_from_keyboard()
{
  yyin = stdin;
  yyparse();
}
