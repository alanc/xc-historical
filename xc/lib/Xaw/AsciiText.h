#include <X/copyright.h>

/* $Header: $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#ifndef _AsciiText_h
#define _AsciiText_h

#include <X/Text.h>		/* AsciiText is a subclass of Text */

typedef struct _AsciiStringClassRec	*AsciiStringWidgetClass;
typedef struct _AsciiStringRec		*AsciiStringWidget;

extern WidgetClass asciiStringWidgetClass;

typedef struct _AsciiDiskClassRec	*AsciiDiskWidgetClass;
typedef struct _AsciiDiskRec		*AsciiDiskWidget;

extern WidgetClass asciiDiskWidgetClass;

#endif  _AsciiText_h
