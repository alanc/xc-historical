/* $XConsortium: bboxint.h,v 2.6 89/05/04 15:07:40 converse Exp $ 
 *
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/* Includes for modules implementing buttonbox stuff. */

#ifndef _bboxint_h
#define _bboxint_h

typedef struct _ButtonRec {
    Widget	widget;		/* Widget containing this button. */
    Widget	menu;		/* Menu widget, for folder buttons only */
    ButtonBox	buttonbox;	/* Button box containing this button. */
    char	*name;		/* Name of the button. */
    void	(*func)();	/* Function to be called when this 
				   button is pressed. (command buttons only) */
} ButtonRec;

typedef struct _XmhButtonBoxRec {
    Widget	outer;		/* Widget containing scollbars & inner */
    Widget	inner;		/* Widget containing the buttons. */
    Scrn	scrn;		/* Scrn containing this button box. */
    int		numbuttons;	/* How many buttons in this box. */
    Button	*button;	/* Array of pointers to buttons. */
    int		maxheight;	/* Current maximum height. */
    WidgetClass button_type;	/* Type of buttons in this button box */
} ButtonBoxRec;

#endif /* _bboxint_h */
