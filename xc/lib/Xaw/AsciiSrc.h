/*
 * $XConsortium: AsciiSrc.h,v 1.2 89/07/06 16:00:54 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */


/*
 * AsciiSrc.h - Public Header file for Ascii Text Source.
 *
 * This is the public header file for the Ascii Text Source.
 * It is intended to be used with the Text widget, the simplest way to use
 * this text source is to use the AsciiText Widget.
 *
 * Date:    June 29, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */


#ifndef _XawAsciiSrc_h
#define _XawAsciiSrc_h

#include <X11/Xaw/TextSrc.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------


*/
 
/* Class record constants */

extern WidgetClass asciiSrcWidgetClass;

typedef struct _AsciiSrcClassRec *AsciiSrcWidgetClass;
typedef struct _AsciiSrcRec      *AsciiSrcWidget;

#define AsciiSourceWidgetClass AsciiSrcWidgetClass
#define AsciiSourceWidget      AsciiSrcWidget

/*
 * Resource Definitions.
 */

#define XtCAsciiString "AsciiString"
#define XtCDataCompression "DataCompression"
#define XtCPieceSize "PieceSize"
#define XtCType "Type"

#define XtNasciiString "asciiString"
#define XtNdataCompression "dataCompression"
#define XtNpieceSize "pieceSize"
#define XtNtype "type"

#define XtRAsciiType "AsciiType"

#define XtEstring "string"
#define XtEfile "file"

typedef enum {XawAsciiFile, XawAsciiString} XawAsciiType;

/************************************************************
 *
 * Public routines 
 *
 ************************************************************/

/*	Function Name: XawAsciiSourceFreeString
 *	Description: Frees the string returned by a get values call
 *                   on the string when the source is of type string.
 *	Arguments: w - the AsciiSrc widget.
 *	Returns: none.
 */

void XawAsciiSourceFreeString(/* w */);
/*
Widget w;
*/

/*	Function Name: XawAsciiSave
 *	Description: Saves all the pieces into a file or string as required.
 *	Arguments: w - the asciiSrc Widget.
 *	Returns: TRUE if the save was successful.
 */

Boolean XawAsciiSave(/* w */);
/*
Widget w;
*/

/*	Function Name: XawAsciiSaveAsFile
 *	Description: Save the current buffer as a file.
 *	Arguments: w - the asciiSrc widget.
 *                 name - name of the file to save this file into.
 *	Returns: True if the save was sucessful.
 */

Boolean XawAsciiSaveAsFile(/* w, name */);
/*
Widget w;
String name;
*/

/*	Function Name: XawAsciiSourceChanged
 *	Description: Returns true if the source has changed since last saved.
 *	Arguments: w - the asciiSource widget.
 *	Returns: a Boolean (see description).
 */

Boolean XawAsciiSourceChanged(/* w */);
/*
Widget w;
*/

#ifdef XAW_BC
/*************************************************************
 *
 * These functions are only preserved for compatability.     
 */

#define ASCII_STRING		/* Turn R3 AsciiDisk and AsciiString */
#define ASCII_DISK		/* Emulation modes. */

#ifdef ASCII_STRING
#define XawStringSourceDestroy XtDestroyWidget
#endif

#ifdef ASCII_DISK
#define XawDiskSourceDestroy XtDestroyWidget
#endif

#ifdef ASCII_STRING
/*	Function Name: AsciiStringSourceCreate
 *	Description: Creates a string source.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource XawStringSourceCreate(/* parent, args, num_args */);
/*
Widget parent;
ArgList args;
Cardinal num_args;
*/
#endif /* ASCII_STRING */

#ifdef ASCII_DISK
/*	Function Name: AsciiDiskSourceCreate
 *	Description: Creates a disk source.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource XawDiskSourceCreate(/* parent, args, num_args */);
/*
Widget parent;
ArgList args;
Cardinal num_args;
*/
#endif /* ASCII_DISK */
#endif /* XAW_BC */
/*
 * End of Compatability stuff.
 *  
 ***************************************************/

#endif /* _XawAsciiSrc_h  - Don't add anything after this line. */

