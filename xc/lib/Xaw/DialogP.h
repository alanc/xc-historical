/* $Header: DialogP.h,v 1.3 88/01/29 10:50:44 swick Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

/* Private definitions for Dialog widget */

#ifndef _DialogP_h
#define _DialogP_h

#include <X11/Dialog.h>
#include <X11/FormP.h>

typedef struct {int empty;} DialogClassPart;

typedef struct _DialogClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    FormClassPart	form_class;
    DialogClassPart	dialog_class;
} DialogClassRec;

extern DialogClassRec dialogClassRec;

typedef struct _DialogPart {
    /* resources */
    String	label;		/* description of the dialog	*/
    String	value;		/* for the user response	*/
    Cardinal	max_length;	/* of user response		*/
    /* private data */
    Widget	labelW;		/* widget to display description*/
    Widget	valueW;		/* user response TextWidget	*/
} DialogPart;

typedef struct _DialogRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    FormPart		form;
    DialogPart		dialog;
} DialogRec;

typedef struct {int empty;} DialogConstraintsPart;

typedef struct _DialogConstraintsRec {
    FormConstraintsPart	  form;
    DialogConstraintsPart dialog;
} DialogConstraintsRec, *DialogConstraints;

#endif _DialogP_h
