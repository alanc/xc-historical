/* $XConsortium: opaque.h,v 1.10 93/07/12 09:44:59 dpw Exp $ */

#ifndef OPAQUE_H
#define OPAQUE_H

extern char *defaultFontPath;
extern char *defaultTextFont;
extern char *defaultCursorFont;
extern char *rgbPath;
extern long MaxClients;
extern char isItTimeToYield;
extern char dispatchException;

/* bit values for dispatchException */
#define DE_RESET     1
#define DE_TERMINATE 2
#ifdef SYNC
#define DE_PRIORITYCHANGE 4  /* set when a client's priority changes */
#endif

extern long TimeOutValue;
extern long ScreenSaverTime;
extern long ScreenSaverInterval;
extern int  ScreenSaverBlanking;
extern int  ScreenSaverAllowExposures;
extern int argcGlobal;
extern char **argvGlobal;

#endif /* OPAQUE_H */
