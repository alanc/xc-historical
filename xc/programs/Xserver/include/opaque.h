/* $XConsortium: opaque.h,v 1.6 89/01/16 13:45:36 rws Exp $ */

extern char *defaultFontPath;
extern char *defaultTextFont;
extern char *defaultCursorFont;
extern char *rgbPath;
extern long MaxClients;
extern int isItTimeToYield;

extern int CloseFont();
extern unsigned long *Xalloc();
extern unsigned long *Xrealloc();
extern void Xfree();
extern Bool AddResource();
extern void FreeResource();
extern pointer LookupID();
extern long TimeOutValue;
extern long ScreenSaverTime;
extern long ScreenSaverInterval;
extern int  ScreenSaverBlanking;
extern int  ScreenSaverAllowExposures;
extern int argcGlobal;
extern char **argvGlobal;
