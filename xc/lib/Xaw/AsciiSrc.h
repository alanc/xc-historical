/*
 * $XConsortium: AsciiSrc.c,v 1.1 89/06/21 17:24:30 kit Exp $
 */

#include <X11/Xaw/Text.h>

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

#define ASCII_STRING		/* Turn R3 AsciiDisk and AsciiString */
#define ASCII_DISK		/* Emulation modes. */

#ifdef ASCII_STRING
#  define XawStringSourceDestroy XawAsciiSourceDestroy
#endif

#ifdef ASCII_DISK
#  define XawDiskSourceDestroy XawAsciiSourceDestroy
#endif

/************************************************************
 *
 * Public routines 
 *
 ************************************************************/

/*	Function Name: XawAsciiSourceFreeString
 *	Description: Frees the string returned by a get values call
 *                   on the string when the source is of type string.
 *	Arguments: w - the widget.
 *	Returns: none.
 */

void XawAsciiSourceFreeString(/* w */);
/*
Widget w;
*/
  
/*	Function Name: XawAsciiSourceCreate
 *	Description: Creates the AsciiSource.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */
  
XawTextSource XawAsciiSourceCreate(/* parent, args, num_args */);
/*
Widget	parent;
ArgList	args;
Cardinal num_args;
*/

/*	Function Name: XawAsciiSourceDestroy
 *	Description: Destroys an ascii source (frees all data)
 *	Arguments: src - ths source to free.
 *	Returns: none.
 */

void XawAsciiSourceDestroy(/* src */);
/*
XawTextSource src;
*/

/*	Function Name: XawAsciiSave
 *	Description: Saves all the pieces into a file or string as required.
 *	Arguments: w - the asciiText Widget.
 *	Returns: TRUE if the save was successful.
 */

Boolean XawAsciiSave(/* w */);
/*
Widget w;
*/

/*	Function Name: XawAsciiSaveAsFile
 *	Description: Save the current buffer as a file.
 *	Arguments: w - the ascii text widget.
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
 *	Arguments: w - the ascii source widget.
 *	Returns: a Boolean (see description).
 */

Boolean XawAsciiSourceChanged(/* w */);
/*
Widget w;
*/

#ifdef ASCII_STRING
/************************************************************
 *
 * Compatability functions.
 *
 ************************************************************/
 
/*	Function Name: AsciiStringSourceCreate
 *	Description: Creates a string source.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource XawAsciiStringSourceCreate(/* parent, args, num_args */);
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

XawTextSource XawAsciiDiskSourceCreate(/* parent, args, num_args */);
/*
Widget parent;
ArgList args;
Cardinal num_args;
*/
#endif /* ASCII_DISK */
