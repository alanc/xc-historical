#ifndef VMS
#include <X11/Xlib.h>
#else
#include <decw$include/Xlib.h>
#endif
#ifndef NULL
#define NULL 0
#endif

#define POLY 1000		/* multiple of 25, please		*/
#define CHARS 100		/* length of text strings		*/
#define COLS 25

#define WIDTH 600		/* Size of large window to work within  */
#define HEIGHT 600

#define CHILDSIZE     8		/* Size of children on windowing tests  */
#define CHILDSPACE    4		/* Space between children		*/

typedef Bool (*InitProc)    (/* Display *dpy; int numObjects */);
typedef void (*Proc)	    (/* Display *dpy; int numObjects */);

#define NullProc ((Proc)0)

typedef struct _Parms {
    int  reps;		/* required of all  */
    int  objects;       /* required of all  */
    int  special;
    char *font, *bfont;
    int  fillStyle;     /* Solid, transparent stipple, or opqaque stipple? */
} ParmRec, *Parms;

typedef struct _Test {
    char    *option;    /* Name to use in prompt line			    */
    char    *label;     /* Fuller description of test			    */
    InitProc init;      /* Initialization procedure			    */
    Proc    proc;       /* Timed benchmark procedure			    */
    Proc    passCleanup;/* Cleanup between repetitions of same test	    */
    Proc    cleanup;    /* Cleanup after test				    */
    Bool    children;   /* Windowing test for differing number of kids?     */
    int     clips;      /* Number of obscuring windows to force clipping    */
    ParmRec parms;      /* Parameters passed to test procedures		    */
} Test;

extern Test test[];
extern int subs[];
extern Window root;

extern int fgPixel, bgPixel;

#define ForEachTest(x) for (x = 0; test[x].option != NULL; x++)

extern void CreatePerfStuff(/* dpy, width, height, &w, &bggc, &fggc */);
