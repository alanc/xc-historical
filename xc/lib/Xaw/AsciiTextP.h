#include <X11/copyright.h>

/* $Header: AsciiTextP.h,v 1.2 88/02/05 23:09:28 swick Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#ifndef _AsciiTextP_h
#define _AsciiTextP_h

#include <X11/AsciiText.h>
#include <X11/TextP.h>	/* AsciiText is a subclass of Text */

extern XtActionsRec textActionsTable[];
extern Cardinal textActionsTableCount;

typedef struct {int empty;} AsciiStringClassPart;

typedef struct _AsciiStringClassRec {
    CoreClassPart	 core_class;
    TextClassPart	 text_class;
    AsciiStringClassPart ascii_string_class;
} AsciiStringClassRec;

extern AsciiStringClassRec asciiStringClassRec;

typedef struct {
    String	string;		/* string for which to create a source */
} AsciiStringPart;

typedef struct _AsciiStringRec {
    CorePart		core;
    TextPart		text;
    AsciiStringPart	ascii_string;
} AsciiStringRec;

typedef struct {int empty;} AsciiDiskClassPart;

typedef struct _AsciiDiskClassRec {
    CoreClassPart	core_class;
    TextClassPart	text_class;
    AsciiDiskClassPart	ascii_disk_class;
} AsciiDiskClassRec;

extern AsciiDiskClassRec asciiDiskClassRec;

typedef struct {
    String	file_name;	/* file for which to create a source */
} AsciiDiskPart;

typedef struct _AsciiDiskRec {
    CorePart		core;
    TextPart		text;
    AsciiDiskPart	ascii_disk;
} AsciiDiskRec;

#endif  _AsciiTextP_h
