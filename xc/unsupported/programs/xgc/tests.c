/*
** tests.c
**
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/AsciiText.h>
#include "xgc.h"
#include "math.h"
#include "stdio.h"
#include <sys/types.h>
#ifdef SYSV
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/timeb.h>
#include <sys/resource.h>

#ifndef PI
#define PI 3.14159265
#endif

extern XStuff X;
extern Widget result;

extern void print_if_recording();
void show_result();

/* timer(flag)
** -----------
** When called with StartTimer, starts the stopwatch and returns nothing.
** When called with EndTimer, stops the stopwatch and returns the time
** in microseconds since it started.
**
** Uses rusage() so we can subtract the time used by the system and user
** from our timer, and just concentrate on the time used in the X calls.
*/

static long
timer(flag)
     int flag;
{
  static struct timeval starttime;  /* starting time for gettimeofday() */
  struct timeval endtime;           /* ending time for gettimeofday() */
#ifndef SYSV
  static struct rusage startusage;  /* starting time for getrusage() */
  struct rusage endusage;           /* ending time for getrusage() */
#endif
  struct timezone tz;               /* to make gettimeofday() happy */

  long elapsedtime;                 /* how long since we started the timer */

  switch (flag) {
    case StartTimer:                       /* store initial values */
      gettimeofday(&starttime,&tz);       
#ifndef SYSV
      getrusage(RUSAGE_SELF,&startusage);
#endif
      return((long) NULL);
    case EndTimer:
      gettimeofday(&endtime,&tz);          /* store final values */
#ifndef SYSV
      getrusage(RUSAGE_SELF,&endusage);
#endif
  /* all the following line does is use the formula 
     elapsed time = ending time - starting time, but there are three 
     different timers and two different units of time, ack... */

#ifdef SYSV
      elapsedtime = (long) ((endtime.tv_sec - starttime.tv_sec) * 1000000
			    + endtime.tv_usec - starttime.tv_usec);
#else
      elapsedtime = (long) ((long)
	((endtime.tv_sec - endusage.ru_utime.tv_sec - endusage.ru_stime.tv_sec
	 - starttime.tv_sec + startusage.ru_utime.tv_sec
	 + startusage.ru_stime.tv_sec)) * 1000000) + (long)
      ((endtime.tv_usec - endusage.ru_utime.tv_usec - endusage.ru_stime.tv_usec
	 - starttime.tv_usec + startusage.ru_utime.tv_usec
	 + startusage.ru_stime.tv_usec));
#endif

      return(elapsedtime);                
    default:                              
      fprintf(stderr,"Invalid flag in timer()\n");
      return((long) NULL);
    }
}


void
copyarea_test()
{
  int num_copies = 200;
  int i;
  long totaltime;
  char buf[80];

  num_copies *= X.percent;

  XSetFillStyle(X.dpy,X.miscgc,FillTiled);
  XFillRectangle(X.dpy,X.win,X.miscgc,0,0,400,400);

  XSync(X.dpy,0);
  timer(StartTimer);
  for (i=0;i<num_copies;++i)
    XCopyArea(X.dpy,X.win,X.win,X.gc,i,200-i,
	      200,200,200-i,i);
  XSync(X.dpy,0);
  totaltime = timer(EndTimer);

  sprintf(buf,"%.2f seconds.",(double)totaltime/1000000.);
  show_result(buf);
}

void
copyplane_test()
{
  int num_copies = 200;
  int i;
  long totaltime;
  char buf[80];

  num_copies *= X.percent;

  XSetFillStyle(X.dpy,X.miscgc,FillTiled);
  XFillRectangle(X.dpy,X.win,X.miscgc,0,0,400,400);

  XSync(X.dpy,0);
  timer(StartTimer);
  for (i=0;i<num_copies;++i)
    XCopyPlane(X.dpy,X.win,X.win,X.gc,i,200-i,
	      200,200,200-i,i,X.gcv.plane_mask);
  XSync(X.dpy,0);
  totaltime = timer(EndTimer);

  sprintf(buf,"%.2f seconds.",(double)totaltime/1000000.);
  show_result(buf);
}

void
circle_line_test(num_vertices,radius)
     int num_vertices,radius;
{
  double theta, delta;
  int length, centerx, centery, i;
  int relative_angle;
  long totaltime;
  char buf[80];
  XPoint *coord;

  relative_angle = num_vertices*5/12+1;
  delta = (double) relative_angle / (double) num_vertices * 2 * PI;
  centerx = centery = 200;

  coord = (XPoint *) malloc (sizeof(XPoint)*(num_vertices+1));

  length = (int) (2 * radius * (float) atan(delta/2.));

  for (i=0;i<=num_vertices;++i) {
    theta = (double) i * delta;
    coord[i].x = centerx + (int) (radius * cos(theta));
    coord[i].y = centery + (int) (radius * sin(theta));
  }

  XSync(X.dpy,0);
  timer(StartTimer);
  XDrawLines(X.dpy,X.win,X.gc,coord,num_vertices+1,CoordModeOrigin);
  XSync(X.dpy,0);
  totaltime = timer(EndTimer);

  sprintf(buf,"%d lines of length %d in %.3f seconds.",num_vertices,
	  length,(double)totaltime/1000000.);
  show_result(buf);

  free(coord);
}



void
polyline_test()
{
  circle_line_test((int)(601*X.percent),190);
}

void
polysegment_test()
{
  XSegment *segments;
  int num_segments = 600;
  long totaltime;
  char buf[80];
  int i;

  num_segments *= X.percent;

  segments = (XSegment *) malloc(sizeof(XSegment) * num_segments);

  segments[0].x1 = random()%400; segments[0].y1 = random()%400;
  segments[0].x2 = random()%400; segments[0].y2 = random()%400;

  for(i=1;i<num_segments;++i) {
    segments[i].x1 = (segments[i-1].x1-segments[i-1].y2+400+i)%400;
    segments[i].y1 = (segments[i-1].y1+segments[i-1].x2+i)%400;
    segments[i].x2 = (segments[i-1].x1-segments[i-1].y1+400+i)%400;
    segments[i].y2 = (segments[i-1].x2+segments[i-1].y2+i)%400;
  }

  XSync(X.dpy,0);
  start_timer();
  XDrawSegments(X.dpy,X.win,X.gc,segments,num_segments);
  XSync(X.dpy,0);
  totaltime = end_timer();
  
  sprintf(buf,"%d segments in %.3f seconds.",num_segments,
	  (double)totaltime/1000000.);
  show_result(buf);

  free(segments);
}

void
polypoint_test()
{
  XPoint *points;
  int num_points = 100000;
  long totaltime;
  char buf[80];
  int i;
  
  num_points *= X.percent;

  points = (XPoint *) malloc(sizeof(XPoint) * num_points);

  points[0].x = random()%400; points[0].y = random()%400;
  points[1].x = random()%400; points[1].y = random()%400;

  for (i=2;i<num_points;++i) {
    points[i].x = (points[i-1].x+points[i-2].y+i*3/200)%400;
    points[i].y = (points[i-1].y+points[i-2].x+i*5/200)%400;
  }

  XSync(X.dpy,0);
  start_timer();
  XDrawPoints(X.dpy,X.win,X.gc,points,20000,CoordModeOrigin);
  XDrawPoints(X.dpy,X.win,X.gc,&points[20000],20000,CoordModeOrigin);
  XDrawPoints(X.dpy,X.win,X.gc,&points[40000],20000,CoordModeOrigin);
  XDrawPoints(X.dpy,X.win,X.gc,&points[60000],20000,CoordModeOrigin);
  XDrawPoints(X.dpy,X.win,X.gc,&points[80000],20000,CoordModeOrigin);
  XSync(X.dpy,0);
  totaltime = end_timer();

  sprintf(buf,"%d points in %.3f seconds.",num_points,
	  (double)totaltime/1000000.);
  show_result(buf);

  free(points);
}

void
genericrectangle_test(fill)
     Boolean fill;
{
  XRectangle *rects;
  int num_rects = 200;
  int perimeter = 0, area = 0;
  int i;
  long totaltime;
  char buf[80];

  num_rects *= X.percent;

  rects = (XRectangle *) malloc(sizeof(XRectangle) * num_rects);

  for (i=0;i<num_rects;++i) {
    rects[i].x = rects[i].y = 200 - i;
    rects[i].width = rects[i].height = 2 * i;
    perimeter += rects[i].width * 2 + rects[i].height * 2;
    area += rects[i].width * rects[i].height;
  }

  XSync(X.dpy,0);
  start_timer();
  if (fill) XFillRectangles(X.dpy,X.win,X.gc,rects,num_rects);
  else XDrawRectangles(X.dpy,X.win,X.gc,rects,num_rects);
  XSync(X.dpy,0);
  totaltime = end_timer();

  if (fill)
    sprintf(buf,"%d pixels in %.2f seconds.",area,(double)totaltime/1000000.);
  else
    sprintf(buf,"Total line length %d in %.3f seconds.",perimeter,
	    (double)totaltime/1000000.);
  show_result(buf);

  free(rects);
}

void
polyrectangle_test()
{
  genericrectangle_test(FALSE);
}

void
polyfillrectangle_test()
{
  genericrectangle_test(TRUE);
}

/*****************************/

void
genericarc_test(fill)
     Boolean fill;
{
  XArc *arcs;
  int num_arcs = 100;
  int i;
  long totaltime;
  char buf[80];

  num_arcs *= X.percent;

  arcs = (XArc *) malloc(sizeof(XArc) * num_arcs);

  for (i=0;i<num_arcs;++i) {
    arcs[i].x = i;
    arcs[i].y = i;
    arcs[i].width = i;
    arcs[i].height = i;
    arcs[i].angle1 = i * 50;
    arcs[i].angle2 = i * 50;
  }

  XSync(X.dpy,0);
  start_timer();
  if (fill) XFillArcs(X.dpy,X.win,X.gc,arcs,num_arcs);
  else XDrawArcs(X.dpy,X.win,X.gc,arcs,num_arcs);
  XSync(X.dpy,0);
  totaltime = end_timer();

  sprintf(buf,"God only knows how many pixels in %.3f seconds.",
	  (double)totaltime/1000000.);
  show_result(buf);

  free(arcs);
}

void
polyarc_test()
{
  genericarc_test(FALSE);
}

void
polyfillarc_test()
{
  genericarc_test(TRUE);
}

void
polytext8_test()
{
  int num_strings = 200;
  static char string[] = "pack my box with five dozen liquor jugs";
  int i;
  long totaltime;
  char buf[80];

  num_strings *= X.percent;

  XSync(X.dpy,0);
  start_timer();
  for (i=0;i<num_strings;++i) {
    XDrawString(X.dpy,X.win,X.gc,(i%2 ? i : num_strings - i),i,
		string,sizeof(string));
  }
  XSync(X.dpy,0);
  totaltime = end_timer();

  sprintf(buf,"%d strings in %.2f seconds.",num_strings,
	  (double) totaltime/1000000.);
  show_result(buf);
}

void
imagetext8_test()
{
  int num_strings = 200;
  static char string[] = "pack my box with five dozen liquor jugs";
  int i;
  long totaltime;
  char buf[80];

  num_strings *= X.percent;

  XSync(X.dpy,0);
  start_timer();
  for (i=0;i<num_strings;++i) {
    XDrawImageString(X.dpy,X.win,X.gc,(i%2 ? i : num_strings - i),i,
		string,sizeof(string));
  }
  XSync(X.dpy,0);
  totaltime = end_timer();

  sprintf(buf,"%d strings in %.2f seconds.",num_strings,
	  (double) totaltime/1000000.);
  show_result(buf);
}

void
putimage_test()
{
  int num_copies = 200;
  int i;
  long totaltime;
  char buf[80];

  num_copies *= X.percent;

  XSetFillStyle(X.dpy,X.miscgc,FillTiled);
  XFillRectangle(X.dpy,X.win,X.miscgc,0,0,400,400);

  X.image = XGetImage(X.dpy,X.win,0,0,200,200,~0,XYPixmap);

  XSync(X.dpy,0);
  timer(StartTimer);
  for (i=0;i<num_copies;++i)
    XPutImage(X.dpy,X.win,X.gc,X.image,0,0,i,i,200,200);
  XSync(X.dpy,0);
  totaltime = timer(EndTimer);

  sprintf(buf,"%.2f seconds.",(double)totaltime/1000000.);
  show_result(buf);
}


/*****************************/
/*****************************/

void
run_test()
{
  XClearWindow(X.dpy,X.win);

  print_if_recording("run\n");
  
  switch (X.test) {
    case CopyArea:      copyarea_test();           break;
    case CopyPlane:     copyplane_test();          break;
    case PolyPoint:     polypoint_test();          break;
    case PolyLine:      polyline_test();           break;
    case PolySegment:   polysegment_test();        break;
    case PolyRectangle: polyrectangle_test();      break;
    case PolyArc:       polyarc_test();            break;
    case PolyFillRect:  polyfillrectangle_test();  break;
    case PolyFillArc:   polyfillarc_test();        break;
    case PolyText8:     polytext8_test();          break;
    case ImageText8:    imagetext8_test();         break;
    case PutImage:      putimage_test();           break;
    default: fprintf(stderr,"That test don't exist yet.\n");
    }
}

/*****************************/

/* set_text(w,string)
** ------------------
** Sets the text in a read-only text widget to the specified string.
*/

void
set_text(w,string)
     Widget w;
     char *string;
{
  static Arg args[] = {
    {XtNstring,  (XtArgVal) NULL}
  };
  XawTextSource source;
  char *current_string;

  args[0].value = (XtArgVal) &current_string;
  XtGetValues(w,args,XtNumber(args));

  strcpy(current_string,string);

  args[0].value = (XtArgVal) current_string;
  source = XawStringSourceCreate(w,args,XtNumber(args));
  XawTextSetSource(w,source,(XawTextPosition) 0);
}

void
show_result(string)
     char *string;
{
  char buf[80];

  set_text(result,string);

  strcpy(buf,"# ");
  strcat(buf,string);
  strcat(buf,"\n");
  print_if_recording(buf);
}
