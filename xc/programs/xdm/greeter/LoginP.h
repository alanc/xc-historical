/*
* $XConsortium:  $
*/

#ifndef _LoginP_h
#define _LoginP_h

#include "Login.h"
#include <X11/CoreP.h>

#define GET_NAME	0
#define GET_PASSWD	1
#define DONE		2

/* New fields for the login widget instance record */
typedef struct {
	Pixel		textpixel;	/* foreground pixel */
	Pixel		promptpixel;	/* prompt pixel */
	Pixel		greetpixel;	/* greeting pixel */
	Pixel		failpixel;	/* failure pixel */
	GC		textGC;		/* pointer to GraphicsContext */
	GC		bgGC;		/* pointer to GraphicsContext */
	GC		xorGC;		/* pointer to GraphicsContext */
	GC		promptGC;
	GC		greetGC;
	GC		failGC;
	char		*greeting;	/* greeting */
	char		*namePrompt;	/* name prompt */
	char		*passwdPrompt;	/* password prompt */
	char		*fail;		/* failure message */
	XFontStruct	*font;		/* font for text */
	XFontStruct	*promptFont;	/* font for prompts */
	XFontStruct	*greetFont;	/* font for greeting */
	XFontStruct	*failFont;	/* font for failure message */
	int		state;		/* state */
	int		cursor;		/* current cursor position */
	int		failUp;		/* failure message displayed */
	LoginData	data;		/* name/passwd */
	void		(*notify_done)();/* proc to call when done */
	int		failTimeout;	/* seconds til drop fail msg */
	XtIntervalId	interval_id;	/* drop fail message note */
   } LoginPart;

/* Full instance record declaration */
typedef struct _LoginRec {
   CorePart core;
   LoginPart login;
   } LoginRec;

/* New fields for the Login widget class record */
typedef struct {int dummy;} LoginClassPart;

/* Full class record declaration. */
typedef struct _LoginClassRec {
   CoreClassPart core_class;
   LoginClassPart login_class;
   } LoginClassRec;

/* Class pointer. */
extern LoginClassRec loginClassRec;

#endif _LoginP_h
