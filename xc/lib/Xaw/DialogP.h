/* $Header: */

/* Private definitions for Dialog widget */

#ifndef _DialogP_h
#define _DialogP_h

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
/* DON'T ADD STUFF AFTER THIS #endif */
