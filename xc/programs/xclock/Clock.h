#ifndef _XtClock_h
#define _XtClock_h

/***********************************************************************
 *
 * Clock Widget
 *
 ***********************************************************************/

#define XtNclock		"clock"

/* Resource names used to the clock widget */

#define XtNupdate		"update"	/* Int: how often to update hands? */
#define XtNforeground		"foreground"    /* color of clock face or text */
#define XtNhand			"hands"         /* color of hands */
#define XtNhigh			"highlight"     /* color of hand outline */
#define XtNanalog		"analog"        /* Boolean: digital if FALSE */
#define XtNchime		"chime"         /* Boolean:  */
#define XtNpadding		"padding"       /* Int: amount of space around outside of clock */
#define XtNfont			"font"          /* Font for digital clock */

typedef struct _ClockWidgetClassData ClockWidgetClassData;

extern ClockWidgetClassData clockWidgetClassData;
#define clockWidgetClass (&clockWidgetClassData)

#endif _XtClock_h
/* DON'T ADD STUFF AFTER THIS #endif */
