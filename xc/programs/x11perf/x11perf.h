#ifndef VMS
#include <X11/Xlib.h>
#else
#include <decw$include/Xlib.h>
#endif
#ifndef NULL
#define NULL 0
#endif

#define POLY 1000		/* multiple of 25, please */
#define CHARS 100		/* length of text strings */
#define COLS 25
#define WIDTH 600
#define HEIGHT 600

typedef void (*Proc)(/* Display *dpy; int numObjects */);

#define NullProc ((Proc)0)

typedef struct _Parms {
    int reps;			/* required of all */
    int objects;		/* required of all */
    int special;
    char *font, *bfont;
} ParmRec, *Parms;

typedef struct _Test {
    char *option;
    char *label;
    Proc init;
    Proc proc;
    Proc passCleanup;
    Proc cleanup;
    Bool children;
    Bool doit;
    ParmRec parms;
} Test;

extern Test test[];
extern int subs[];
extern Window root;

#define ForEachTest(x) for (x = 0; test[x].option != NULL; x++)

extern void CreatePerfStuff(/* dpy, width, height, &w, &whitegc, &blackgc */);
