/************************************************************

  The Client message sent to the application is:

  ATOM = "ResEditor" 		--- RES_EDITOR_NAME

  FORMAT = 32                   --- RES_EDIT_SEND_EVENT_FORMAT

  l[0] = timestamp
  l[1] = command atom name
  l[2] = ident of command.

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
                may be more than one character.

return value  = This is data specific to the command passed.

************************************************************

Commands

************************************************************

Send Widget Tree

--------------------

command value =  <ignored>

return value  	= <widget>\n

widget 		= <grandparent_name>.<parent_name>.<widget_name>:<widget_class>

--------------------

SetValues

--------------------

command value	= <command>\n

command		= <id>#resource_name:resource_value

   id 		= .<widgetid>.<parentid>.<grandparentid>...

if (error = FormattedResError) then 

	return value 	= <value>\n

	value		= [id]:<string>

        id 		= .<widgetid>.<parentid>.<grandparentid>...

else (error == NoResError || error == UnformattedResError)

	return value = <string>

endif

--------------------

GetResources - NIY

--------------------

command value 	= <id>\n

   id 		= .<widgetid>.<parentid>.<grandparentid>...

if (error = FormattedResError) then 

	error is set to 1 and

	return value 	= <value>\n

	value		= <e_val>[id]:<data>

	e_val		= 1 if an error occured, 0 otherwise.
	                  NOTE: This field is exactly one character wide.

	id 		= .<widgetid>.<parentid>.<grandparentid>...

	if (e_val == 0) then

		data 	= <resource>\t...

		resource= <name>:<Class>
		
	else 

		data 	= <string>

	endif

else (error == NoResError || error == UnformattedResError)

	error is set to 1 and

	return value = <string>

endif

************************************************************/

/*
 * Format for atoms.
 */

#define RES_EDITOR_FORMAT           8
#define RES_EDIT_SEND_EVENT_FORMAT 32
/*
 * Atoms
 */

#define RES_EDITOR_NAME         "ResEditor"
#define RES_EDITOR_COMMAND_ATOM "ResEditCommand"
#define RES_EDITOR_COMM_ATOM    "ResEditComm"
#define RES_EDITOR_CLIENT_VALUE "ResEditClientVal"

/*
 * Resource Editor Commands.
 */

#define SEND_WIDGET_TREE "SendWidgetTree"
#define SET_VALUES "SetValues"

/*
 * Seperators for various fields.
 */

#define CLASS_SEPARATOR (':')
#define COMMAND_SEPARATOR ('|')
#define ID_SEPARATOR ('\t')
#define NAME_SEPARATOR ('.')
#define WID_RES_SEPARATOR ('#')
#define NAME_VAL_SEPARATOR CLASS_SEPARATOR
#define EOL_SEPARATOR ('\n')

/*
 * The type of a resource identifier.
 */

typedef unsigned int ResIdent;

typedef enum {NoResError = 0, UnformattedResError,
	      FormattedResError} ResourceError;
