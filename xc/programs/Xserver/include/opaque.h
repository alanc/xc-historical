/* $XConsortium: opaque.h,v 1.16 94/03/18 17:20:06 dpw Exp $ */

#ifndef OPAQUE_H
#define OPAQUE_H

#include <X11/Xmd.h>

extern char *defaultFontPath;
extern char *defaultTextFont;
extern char *defaultCursorFont;
extern char *rgbPath;
extern int MaxClients;
extern char isItTimeToYield;
extern char dispatchException;

/* bit values for dispatchException */
#define DE_RESET     1
#define DE_TERMINATE 2
#define DE_PRIORITYCHANGE 4  /* set when a client's priority changes */

extern CARD32 TimeOutValue;
extern CARD32 ScreenSaverTime;
extern CARD32 ScreenSaverInterval;
extern int  ScreenSaverBlanking;
extern int  ScreenSaverAllowExposures;
extern int argcGlobal;
extern char **argvGlobal;

#endif /* OPAQUE_H */
