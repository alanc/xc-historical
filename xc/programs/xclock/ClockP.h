#ifndef _XtClockPrivate_h
#define _XtClockPrivate_h

#define SEG_BUFF_SIZE		128

typedef struct  {
	 Pixel	fgpixel;	/* color index for text */
	 Pixel	Hipixel;	/* color index for Highlighting */
	 Pixel	Hdpixel;	/* color index for hands */
	 XFontStruct	*font;	/* font for text */
	 GC	myGC;		/* pointer to GraphicsContext */
	 GC	EraseGC;	/* eraser GC */
	 GC	HandGC;		/* Hand GC */
	 GC	HighGC;		/* Highlighting GC */
/* start of graph stuff */
	 int	update;		/* update frequence */
	 Dimension radius;		/* radius factor */
	 Boolean chime;
	 Boolean beeped;
	 Boolean analog;
	 Boolean show_second_hand;
	 Dimension second_hand_length;
	 Dimension minute_hand_length;
	 Dimension hour_hand_length;
	 Dimension hand_width;
	 Dimension second_hand_width;
	 Position centerX;
	 Position centerY;
	 int	numseg;
	 int	padding;
	 XPoint	segbuff[SEG_BUFF_SIZE];
	 XPoint	*segbuffptr;
	 XPoint	*hour, *sec;
	 struct tm  otm ;
	 XtIntervalId interval_id;
   } Clock;

#endif _XtClockPrivate_h
