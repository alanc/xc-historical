/************************************************************

		The Editres Protocol


  The Client message sent to the application is:

  ATOM = "ResEditor" 		--- RES_EDITOR_NAME

  FORMAT = 32                   --- RES_EDIT_SEND_EVENT_FORMAT

  l[0] = timestamp
  l[1] = command atom name
  l[2] = ident of command.
  l[3] = protocol version number to use.



  The binary protocol has the following format:

	Card8:		8-bit unsingned integer
	Card16:		16-bit unsingned integer
	Card32:		32-bit unsingned integer
	Int16:		16-bit signed integer
	Window:		32-bit value
	Widget:		32-bit value
	
	[a][b][c] represent an exclusive list of choices.

	All widgets are passed as a list of widgets, containing the 
	full instance heirarch of this widget.  The heirarchy is ordered
	from parent to child.  Thus first element of each list is
	the root of the widget tree.

	ListOfFoo comprises a lsit of things in the following format:
	
	number:			Card16
	<number> things:	????
	
  This is a synchronous protocol, every request MUST be followed by a
  reply.  

  Request:

	Serial Number:	Card8
	Op Code:	Card8 -	{ SendWidgetTree = 0,
				  SetValues = 1,
				  GetResources = 2,
				  GetGeometry = 3,
				  FindChild = 4 }
	Length:		Card32
	Data:		

   Reply:

	Serial Number:	Card8
	Type:		Card8 - { PartialSuccess, Failure, ProtocolMismatch }
	Length:		Card32


   Byte Order:

	All Fields are MSB -> LSB

   ERRORS:

	Failure:

		Message:	String8

	ProtocolMismatch:

		RequestedVersion:   	Card8

------------------------------------------------------------

   SendWidgetTree:

	--->

	Number of Entries:	Card16
	Entry:
		widget:		ListOfWidgets
		name:		String8
		class:		String8
		window:		Card32

	Send widget Tree returns the fully specified list of widgets
	for each widget in the tree.  This is enough information to completely
	reconstruct the entire widget heirarchy.

   SetValues:

	name:	String8
	type:	String8
	value:  String8
	Number of Entries:	Card16
	Entry:
		widget:		ListOfWidgets

	--->

	Number of Entries:	Card16
	Entry:
		widget:		ListOfWidgets:
		message:	String8

	SetValues will allow the same resource to be set for a number of 
	widgets.  This function will return a message for any widget for which
	the SetValues request failed.
	
   GetResources:

	Number of Entries:	Card16
	Entry
		widget:		ListOfWidgets:

	---->

	Number of Entries:	Card16
	Entry
		Widget:			ListOfWidgets:
		Error:			Bool

		[ Message:		String 8 ]
		[ Number of Resources:	Card16
		Resource:
			Kind:	{normal, constraint}
			Name:	String8
			Class:	String8	
			Type:	String8 ]

	GetResource retrieves the kind, name, type and class for every 
	widget passed to it.  If an error occured with the resource fetch
	Error will be set to True for the given widget and a message
	is returned rather than the resource info.

  GetGeometry:

	Number of Entries:	Card16
	Entry
		Widget:		ListOfWidgets:

	---->

	Number of Entries:	Card16
	Entry
		Widget:			ListOfWidgets:
		Error:			Bool

		[ message:		String 8 ]
		[ Visable:      Boolean
		  X: 		Int16
		  Y:  		Int16
		  Width: 	Card16
	      	  Height:	Card16
		  BorderWidth:	Card16 ]

	GetGeometry retreives the x, y, width, height and border width 
	for each widget specified.  If an error occured with the geometry
	fetch Error will be set to True for the given widget and a message
	is returned rather than the geometry info.  X an Y corrospond to
	the root coordinates of the outside of window border of the widget
	specified.
	
  FindChild:

	Widget:		ListOfWidgets
	X:		Int16
	Y:		Int16
	
	--->

	Widget:		ListOfWidgets

	Find Child returns a descendent of the widget specified that 
	is at the root coordinates specified.

	NOTE:

	The returned widget is undefined if the point is contained in
	two or more mapped widgets, or in two overlapping Rect objs.

************************************************************/

#include <X11/Intrinsic.h>

#define BYTE 8
#define BYTE_MASK 255

#define HEADER_SIZE 6

#define EDITRES_IS_OBJECT 2
#define EDITRES_IS_UNREALIZED 0

/*
 * Format for atoms.
 */

#define EDITRES_FORMAT             8
#define EDITRES_SEND_EVENT_FORMAT 32

/*
 * Atoms
 */

#define EDITRES_NAME         "Editres"
#define EDITRES_COMMAND_ATOM "EditresCommand"
#define EDITRES_COMM_ATOM    "EditresComm"
#define EDITRES_CLIENT_VALUE "EditresClientVal"
#define EDITRES_PROTOCOL_ATOM "EditresProtocol"

typedef enum { SendWidgetTree = 0, SetValues = 1, GetResources = 2,
	       GetGeometry = 3, FindChild = 4 } EditresCommand;

typedef enum {NormalResource = 0, ConstraintResource = 1} ResourceType;

/*
 * The type of a resource identifier.
 */

typedef unsigned char ResIdent;

typedef enum {PartialSuccess, Failure, ProtocolMismatch} EditResError;

typedef struct _WidgetInfo {
    unsigned short num_widgets;
    unsigned long * ids;
    Widget real_widget;
} WidgetInfo;

typedef struct _ProtocolStream {
    unsigned long size, alloc;
    XtPointer real_top, top, current;
} ProtocolStream;

/************************************************************
 *
 * Function definitions for reading and writing protocol requests.
 *
 ************************************************************/

void _EresInsertString8(), _EresInsert8(), _EresInsert16(), _EresInsert32();
void _EresInsertWidgetInfo(), _EresInsertWidget(), _EresResetStream();

Boolean _EresRetrieve8(), _EresRetrieve16(), _EresRetrieveSigned16();
Boolean _EresRetrieve32(), _EresRetrieveString8(), _EresRetrieveWidgetInfo();

