#include <X11/copyright.h>

/* $XConsortium: SimpleP.h,v 1.7 88/09/06 16:42:24 jim Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987, 1988 */

#ifndef _TemplateP_h
#define _TemplateP_h

#include "Template.h"
#include <X11/CoreP.h>

#define XtRTemplate		"Template"

typedef struct {
    int empty;
} TemplateClassPart;

typedef struct _TemplateClassRec {
    CoreClassPart	core_class;
    TemplateClassPart	template_class;
} TemplateClassRec;

extern TemplateClassRec templateClassRec;

typedef struct {
    /* resources */
    char* template;
    /* private state */
} TemplatePart;

typedef struct _TemplateRec {
    CorePart		core;
    TemplatePart	template;
} TemplateRec;

#endif  _TemplateP_h
