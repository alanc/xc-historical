/************************************************************

  The Client message sent to the application is:

  ATOM = "ResEditor" 		--- RES_EDITOR_NAME

  FORMAT = 32                   --- RES_EDIT_SEND_EVENT_FORMAT

  l[0] = timestamp
  l[1] = command atom name
  l[2] = ident of command.
  l[3] = protocol version number to use.

--------------------

  The syntax for Resource editor commands is:

  Selection = 	<ident>\t<command>|<command value>

  ident = 	A uniquie identification number.

  command =	The command that the application should execute.

  command value = 	The some additional data required by this command.

--------------------

  The syntax returned to the resource editor is:

  Selection = 	<ident>\t<error>|<return value>

  ident = 	the ident number passed with the command.

  error = 	either 0 or non-zero.  If non-zero then an error has occured.
  
  Current Error codes:

  	0 	- no error.
	1	- unable to talk this version of the protocol.
	2       - unformatted error.
	3	- formatted error.

if (error == 1)
	return value	= a version of the protocol that the client will 
			  understand.
else if (error == 2)
	return value	= <unformatted ascii string>
else
	return value	= This is data specific to the command passed.
endif

************************************************************

Commands

The command values and return values for the various commands are
documented here.  If the <error> value is 1 or 2 then the return
values will be as stated above.

Currently supported commands are:

	SendWidgetTree
	SetValues
	GetResources
	GetGeometry

************************************************************

--------------------
SendWidgetTree
--------------------

command value 	= <ignored>

return value  	= <widget><EOL_SEPARATOR><return_value>

widget 		= <grandparent_name>.<parent_name>.<_name>:<class>|<window>

*_name		= <widget_name>\t<widget_id>

widget_name	= name of the widget returned by XtName()

class		= class of the widget stored in the class record stucture

widget_id	= id of the widget represented as a string in decimal form.

window   	= id of the window associated with this widget, or 0 if
		  this widget has no window.

--------------------
SetValues
--------------------

command value	= <command><EOL_SEPARATOR><command value>

command		= <id>#resource_name:resource_value

   id 		= .<widget_id>.<parent_id>.<grandparent_id>...

if (error == 3) then 

	return value 	= <value><EOL_SEPARATOR>

	value		= {[id], "Unknown Widget"}:<string>

else if (error == 0)

	return value = <string>

endif

--------------------
GetResources
--------------------

command value 	= <id><EOL_SEPARATOR><command value>

   id 		= .<widgetid>.<parentid>.<grandparentid>...

return value 	= <value><EOL_SEPARATOR>

   value	= <e_val><info>

   e_val	= 1 if an error occured, 0 otherwise.
                  NOTE: This field is exactly one character wide.

   if (e_val == 0) then

        info	= [id]:<data>

   	data 	= <resource>\t...

	resource= <type><name>:<class>#<type>

	type = 'n' or 'c'
	       NOTE: type is exactly one character long
	             and distinguishes between (n)ormal 
		     and (c)onstraint resources.

   else if (e_val == 3)

	info	= {[id], "Unknown Widget"}:<string>

   else

   	data 	= <string>

   endif

-----------
GetGeometry
-----------

command value 	= <id><EOL_SEPARATOR>...

   id 		= .<widgetid>.<parentid>.<grandparentid>...

return value 	= <value><EOL_SEPARATOR>

   value	= <e_val>

   e_val	= 1 if an error occured, 0 otherwise.
                  NOTE: This field is exactly one character wide.

   if (e_val == 0) then

        info	= [id]:<data>
	
   	data 	= <geometry>#<border_width> || <"NOT_VISABLE">

	geometry = <geometry string in format used by XParseGeomtery>
		
   else if (e_val == 3) then

	info 	=  {[id], "Unknown Widget"}:<string>

   else

   	data 	= <string>

   endif

-----------
FindChild
-----------

command value 	= <id>:<position_geom>

position_geom	= {+-}<x_coord>{+-}<y_coord>

EXAMPLE:	+2+5  OR  -300+30

NOTE:		These are root coordinates, not coordinates relative
		to the widget passed.

   id 		= .<widgetid>.<parentid>.<grandparentid>...

   if (e_val == 0)

	return value 	= <id>

   else 

        return value    = <string>

NOTE:

The returned widget is undefined if the point is contained in
two or more mapped widgets, or in two overlapping Rect objs.

************************************************************/

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

#define EDITRES_NAME         "EditRes"
#define EDITRES_COMMAND_ATOM "EditResCommand"
#define EDITRES_COMM_ATOM    "EditResComm"
#define EDITRES_CLIENT_VALUE "EditResClientVal"

/*
 * Resource Editor Commands.
 */

#define EDITRES_SEND_WIDGET_TREE	"SendWidgetTree"
#define EDITRES_SET_VALUES		"SetValues"
#define EDITRES_GET_GEOMETRY		"GetGeometry"
#define EDITRES_GET_RESOURCES		"GetResources"
#define EDITRES_FIND_CHILD		"FindChild"

/*
 * Seperators for various fields.
 */

#define CLASS_SEPARATOR (':')
#define COMMAND_SEPARATOR ('|')
#define ID_SEPARATOR ('\t')
#define NAME_SEPARATOR ('.')
#define WID_RES_SEPARATOR ('#')
#define EDITRES_BORDER_WIDTH_SEPARATOR WID_RES_SEPARATOR
#define NAME_VAL_SEPARATOR CLASS_SEPARATOR
#define CLASS_TYPE_SEPARATOR WID_RES_SEPARATOR
#define RESOURCE_SEPARATOR ID_SEPARATOR
#define WINDOW_SEPARATOR COMMAND_SEPARATOR
#define EOL_SEPARATOR ('\n')

/*
 * The type of a resource identifier.
 */

typedef unsigned int ResIdent;

typedef enum {NoResError = 0, ProtocolResError = 1, UnformattedResError = 2,
	      FormattedResError = 3} ResourceError;
