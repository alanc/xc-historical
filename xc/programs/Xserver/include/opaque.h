/* $Header: opaque.h,v 1.1 87/09/01 17:20:05 toddb Locked $ */

extern char *defaultFontPath;
extern char *defaultTextFont;
extern char *defaultCursorFont;
extern char *rgbPath;
extern long MaxClients;
extern int isItTimeToYield;

extern void CloseFont();
extern unsigned long *Xalloc();
extern unsigned long *Xrealloc();
extern void Xfree();
extern void AddResource();
extern void FreeResource();
extern pointer LookupID();
extern int	TimeOutValue;
extern long ScreenSaverTime;
extern long ScreenSaverInterval;
extern int  ScreenSaverBlanking;
extern int argcGlobal;
extern char **argvGlobal;
