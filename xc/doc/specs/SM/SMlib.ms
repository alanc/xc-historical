.\" macro: start marker
.de sM
.ne 4
.sp 1
\\h'-0.3i'\\L'-1v'\\v'3p'\\l'1v'\\v'1v-3p'
.sp -1
..
.\" macro: end marker
.de eM
.sp -1
\\h'-0.3i'\\L'-1v'\\v'1v+4p'\\l'1v'\\v'-4p'
.sp 1
..
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ad b
.sp 10
.TL
\s+2\fBX Session Management Library\fP\s-2
.sp
Public Review Draft
.sp
X Consortium Standard
.sp
X Version 11, Release 6
.AU
Ralph Mor
.AI
X Consortium
.LP
.DS C
Copyright \(co 1993 by the Massachusetts Institute of Technology
.LP
.sp 5
Permission to use, copy, modify, and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.  MIT makes no
representations about the suitability for any purpose of the information in
this document.  This documentation is provided ``as is'' without express or
implied warranty.
.DE
.bp
.bp 1
.EH '\fBX Session Management Library\fP'''
.OH '''\fBX Session Management Library\fP'
.EF ''- % -''
.OF ''- % -''
.NH 1
Acknowledgements
.XS
\*(SN Acknowledgements
.XE
.LP
I would like to thank the following people for their participation in the
X Session Management protocol design: Jordan Brown, Ellis Cohen,
Donna Converse, Vania Joloboff, Stuart Marks, Bob Scheifler, and Mike Wexler.
.LP
.NH 1
Overview of Session Management
.XS
\*(SN Overview of Session Management
.XE
.LP
The purpose of the X Session Management Protocol (XSMP) is to provide a
uniform mechanism for users to save and restore their sessions.  A
\fIsession\fP is a group of clients, each of which has a particular state.
The session is controlled by a network service called the \fIsession
manager\fP\^.  The session manager issues commands to its clients on behalf
of the user.  These commands may cause clients to save their state or to
terminate.  It is expected that the client will save its state in such a
way that the client can be restarted at a later time and resume its
operation as if it had never been terminated.  A client's state might
include information about the file currently being edited, the current
position of the insertion point within the file, or the start of an 
uncommitted transaction.  The means by which clients are restarted is
unspecified by this protocol.
.LP
For purposes of this protocol, a \fIclient\fP\^ of the session manager is
defined as a connection to the session manager.  A client is typically,
though not necessarily, a process running an application program connected
to an X display.  However, a client may be connected to more
than one X display or not be connected to any X displays at all.
.LP
.NH 1
The Session Management Library
.XS
\*(SN The Session Management Library
.XE
.LP
The Session Management Library (SMlib) is a low level interface to XSMP.
It is expected that higher level toolkits, such as Xt, will hide many of
the details of session management from clients.  Higher level toolkits
might also be developed for session managers to use, but no such effort
is currently under way.
.LP
SMlib has two parts to it - one set of functions for clients that want to
be part of a session, and one set of functions for session managers to call.
Some applications will use both sets of functions and act as \fInested
session managers\fP\^ - they will be both a session manager and a client of
another session.  An example is a mail program that could start a text
editor for editing the text of a mail message.  The mail program is part of
a regular session, and at the same time, is also acting as a session manager
to the editor.
.LP
Clients initialize by connecting to the session manager and obtaining
a \fIclient-ID\fP\^ which uniquely identifies them in the session.
The session manager maintains a list of properties for each client in
the session.  These properties describe the client's environment,
and most importantly, describe how the client can be restarted (via an
\fISmRestartCommand\fP\^).  Clients are expected to initialize a set of
required properties when they start up.  They should make sure that they encode
their \fIclient-ID\fP\^ in the \fISmRestartCommand\fP\^, to insure that they
retain the same ID the next time they are started.  The
\fISmRestartCommand\fP\^
will most likely refer to a file which was used to save the local state
of the client.  In order to disambiguate multiple instantiations of the client,
the file should have the \fIclient-ID\fP\^ encoded in it's filename.
.LP
Once the client initializes itself with the session manager, it must be
ready to respond to messages from the session manager.  For example, it
might be asked to save it's state or to terminate.  In the case of a shutdown,
the session manager might give each client a chance to interact with the
user and cancel the shutdown.
.LP
.NH 1
Understanding SMlib's Dependence on ICE
.XS
\*(SN Understanding SMlib's Dependence on ICE
.XE
.LP
The X Session Management Protocol is layered on top of the Inter-Client
Exchange (ICE) Protocol.  The ICE protocol is designed to multiplex several
protocols over a single connection.  As a result, working with SMlib requires
a little knowledge of how the ICE library works.
.LP
The ICE library utilizes callbacks to process messages.  When a client
detects that there is data to read on an ICE connection, it should call
the
.PN IceProcessMessage
function.
.PN IceProcessMessage
will read the message header and look at the major opcode in order
to determine which protocol the message was intended for.  The appropriate
protocol library will then be triggered to unpack the message and hand it
off to the client via a callback.
.LP
The main point to be aware of is that an application using SMlib must
have some code which detects when there is data to read on an ICE connection.
This can be done via a select() call on the file descriptor for the
ICE connection, but more typically,
.PN XtAppAddInput
will be used to register a callback which will invoke
.PN IceProcessMessage
each time there is data to read on the ICE connection.
.LP
To further complicate things, knowing which file descriptors to select()
on requires an understanding of how ICE connections are created.
On the client side, a call must be made to
.PN SmcOpenConnection
in order to open a connection with a session manager.
.PN SmcOpenConnection
will internally make a call into
.PN IceOpenConnection
which will in turn determine if an ICE connection already exists between
the client and session manager.  Most likely, a connection will not already
exist and a new ICE connection will be created.  The main point to be aware
of is that on the client side, it is not obvious when ICE connections get
created, since connections are shared when possible.  To solve this problem,
the ICE library lets the application register \fIwatch procedures\fP\^ which
will be invoked each time an ICE connection is opened or closed.  These watch
procedure could be used to add or remove ICE file descriptors from the list of
descriptors to select() on.
.LP
On the session manager side, things work a bit differently.  The session
manager has complete control over the creation
of ICE connections.  The session manager has to first call
.PN IceListenForConnections
in order to start listening for connections from clients.  Once a connection
attempt is detected,
.PN IceAcceptConnection
must be called, and the session manager can simply add the new ICE
file descriptor to the list of descriptors to select() on.
.LP
Please refer to the \fIInter-Client Exchange Library\fP\^ document for
more details on the library functions related to ICE connections.
.LP
.NH 1
Header Files and Library Name
.XS
\*(SN Header Files and Library Name
.XE
.LP
Applications (both session managers and clients) should include the
header file \fI<X11/SM/SMlib.h>\fP\^.  This header file defines all of the
SMlib data structures and function prototypes.  \fISMlib.h\fP\^ includes the
header file \fI<X11/SM/SM.h>\fP\^ which defines all of the SMlib constants.
.LP
Since SMlib is depended on ICE, applications should link against
SMlib and ICElib by using \fI-lSM -lICE\fP\^.
.LP
.NH 1
Session Management Client (Smc) Functions
.XS
\*(SN Session Management Client (Smc) Functions
.XE
.LP
.NH 2
Connecting to the Session Manager
.XS
\*(SN Connecting to the Session Manager
.XE
.LP
To open a connection with a session manager, call the
.PN SmcOpenConnection
function.
.LP
.sM
.FD 0
SmcConn SmcOpenConnection\^(\^\fInetworkIdsList\fP, \fIcallbacks\fP\^, \fIpreviousId\fP\^, \fIclientIdRet\fP\^, \fIerrorLength\fP\^, \fIerrorStringRet\fP\^)
.br
    char *\fInetworkIdsList\fP\^;
.br
    SmcCallbacks *\fIcallbacks\fP\^;
.br
    char *\fIpreviousId\fP\^;
.br
    char **\fIclientIdRet\fP\^;
.br
    int \fIerrorLength\fP\^;
.br
    char *\fIerrorStringRet\fP\^;
.FN
.IP \fInetworkIdsList\fP 1i
Specifies the network ID(s) of the session manager.
.IP \fIcallbacks\fP 1i
A set of mandatory callbacks used to respond to messages from the
session manager.
.IP \fIpreviousId\fP 1i
The client ID from the previous session.
.IP \fIclientIdRet\fP 1i
The client ID for the current session is returned.
.IP \fIerrorLength\fP 1i
Length of the \fIerrorStringRet\fP\^ argument passed in.
.IP \fIerrorStringRet\fP 1i
Returns a null terminated error message, if any.  \fIerrorStringRet\fP
points to user supplied memory.  No more than \fIerrorLength\fP\^ bytes
are used.
.LP
.eM
\fInetworkIdsList\fP\^ is a null terminated string containing a list of
network IDs for the session manager, separated by commas.
If \fInetworkIdsList\fP\^ is
.PN NULL ,
the value of the
.PN SESSION_MANAGER
environment variable will be used.
Each network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<sm$objname>	or
	local/<hostname>:<path>	
.TE
.LP
An attempt will be made to use the first network ID.  If that fails,
an attempt will be made using the second network ID, and so on.
.LP
Note that if a connection already exists between the client
and the specified session manager, a previously allocated connection
object is returned and this function has no further effect.
.LP
After the connection is established,
.PN SmcOpenConnection
registers the client with the session manager.  If the client is being
restarted from a previous session, \fIpreviousId\fP\^ should contain a null
terminated string representing the client ID from the previous session.
If the client is first joining the session, \fIpreviousId\fP\^ should be
set to
.PN NULL .
If \fIpreviousId\fP\^ is specified, but is determined to be invalid by the
session manager, SMlib will re-register
the client with \fIpreviousId\fP\^ set to
.PN NULL .
.LP
If
.PN SmcOpenConnection
succeeds, the function returns an opaque connection object and the
\fIclientIdRet\fP\^ argument contains the client ID to be used for
this session.  \fIclientIdRet\fP\^ should be freed with a call to free()
when no longer needed.  On failure,
.PN SmcOpenConnection
returns
.PN NULL
and the reason for failure is returned in \fIerrorStringRet\fP\^.
.LP
Note that SMlib used the ICE protocol to establish a connection with
the session manager.  If an ICE connection already exists between the
client and session manager, the same ICE connection is used for session
management.  Otherwise, a new ICE connection is created.
As discussed in the section titled \fIUnderstanding SMlib's Dependence
on ICE\fP\^, the client will have to keep track of when ICE connections
are created or destroyed (using 
.PN IceAddConnectionWatch
and
.PN IceRemoveConnectionWatch ),
and will have to call
.PN IceProcessMessage
each time select() shows that there is data to read on an ICE connection.
Please refer to the \fIInter-Client Exchange Library\fP\^ document for
more details.
.LP
.sp 0.5
A client must be ready to respond to messages sent by a session manager.
\fIcallbacks\fP\^ contains a set of mandatory callbacks; if any of these
function pointers are
.PN NULL ,
.PN SmcOpenConnection
will fail.
.LP
For each callback, the client can register a pointer to client data.
When SMlib invokes the callback, it will pass the client data pointer.
.LP
.sM
	typedef struct {
.sp 0.5
		struct {
.br
			SmcSaveYourselfProc callback;
.br
			SmPointer client_data;
.br
		} save_yourself;
.sp 0.5
		struct {
.br
			SmcDieProc callback;
.br
			SmPointer client_data;
.br
		} die;
.sp 0.5
		struct {
.br
			SmcShutdownCancelledProc callback;
.br
			SmPointer client_data;
.sp 0.5
		} shutdown_cancelled;
.sp 0.5
	} SmcCallbacks;
.LP
.eM
.bp
.NH 3
The Save Yourself Callback
.XS
\*(SN The Save Yourself Callback
.XE
.LP
The \fISave Yourself\fP\^ callback is of type
.PN SmcSaveYourselfProc .
.LP
.sM
typedef void (*SmcSaveYourselfProc)();
.LP
.FD 0
void SaveYourselfProc\^(\^\fIsmcConn\fP, \fIclientData\fP\^, \fIsaveType\fP\^, \fIshutdown\fP\^, \fIinteractStyle\fP\^, \fIfast\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.br
    int \fIsaveType\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteractStyle\fP\^;
.br
    Bool \fIfast\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIclientData\fP 1i
Client data specified when the callback was registered.
.IP \fIsaveType\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteractStyle\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.LP
.eM
The session manager sends a \fISave Yourself\fP\^ message to a client 
either to checkpoint it or just before
termination so that it can save its state.
The client responds with zero or more calls to
.PN SmcSetProperties
to update the properties indicating how to restart the client.
When all the properties have been set, the client should call
.PN SmcSaveYourselfDone .
.LP
If \fIinteractStyle\fP\^ is
.PN SmInteractStyleNone ,
the client must not interact with the
user while saving state.
If \fIinteractStyle\fP\^ is 
.PN SmInteractStyleErrors ,
the client may interact with the user only if an error condition arises.  If
\fIinteractStyle\fP\^ is 
.PN SmInteractStyleAny ,
then the client may interact with the user for any purpose.
Since only one client can interact with the user at a time, the client
must call
.PN SmcInteractRequest
and wait for an \fIInteract\fP\^ message from the session manager.
When the client is done interacting with the user, it should call
.PN SmcInteractDone .
The client can only call
.PN SmcInteractRequest
after it received a \fISave Yourself\fP\^ message, and before it
calls
.PN SmcSaveYourselfDone .
.LP
If \fIsaveType\fP\^ is
.PN SmSaveLocal ,
the client must update the properties to reflect its current state.
Specifically, it should save enough information to restore
the state as seen by the user of this client.  It should not affect the
state as seen by other users.  If \fIsaveType\fP\^ is
.PN SmSaveGlobal
the user wants the client to commit all of its data to permanent,
globally accessible storage.  If \fIsaveType\fP\^ is
.PN SmSaveBoth ,
the client should do both of these (it should first commit the data to
permanent storage before updating its properties).
.LP
.NH 3
The Die Callback
.XS
\*(SN The Die Callback
.XE
.LP
The \fIDie\fP\^ callback is of type
.PN SmcDieProc .
.LP
.sM
typedef void (*SmcDieProc)();
.LP
.FD 0
void DieProc\^(\^\fIsmcConn\fP, \fIclientData\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIclientData\fP 1i
Client data specified when the callback was registered.
.LP
.eM
The session manager sends a \fIDie\fP\^ message to a client 
when it wants it to die.  The client should respond by calling
.PN SmcCloseConnection .
A session manager that behaves properly will send a
\fISave Yourself\fP\^ message before the \fIDie\fP\^ message.
.LP
.NH 3
The Shutdown Cancelled Callback
.XS
\*(SN The Shutdown Cancelled Callback
.XE
.LP
The \fIShutdown Cancelled\fP\^ callback is of type
.PN SmcShutdownCancelledProc .
.LP
.sM
typedef void (*SmcShutdownCancelledProc)();
.LP
.FD 0
void ShutdownCancelledProc\^(\^\fIsmcConn\fP, \fIclientData\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIclientData\fP 1i
Client data specified when the callback was registered.
.LP
.eM
The session manager sends a \fIShutdown Cancelled\fP\^ message
when the user cancelled the shutdown during an interaction
(see the section titled \fIInteracting With the User\fP\^).
The client can now continue as if the shutdown had never happened.
If the client has not called
.PN SmcSaveYourselfDone
yet, it can either abort the save and then call
.PN SmcSaveYourselfDone
with the \fIsuccess\fP\^ argument set to
.PN False ,
or it can continue with the save and then call
.PN SmcSaveYourselfDone
with the \fIsuccess\fP\^ argument set to reflect the outcome of the save.
.LP
.NH 2
Closing the Connection
.XS
\*(SN Closing the Connection
.XE
.LP
To close a connection with a session manager, call the
.PN SmcCloseConnection
function.
.LP
.sM
.FD 0
void SmcCloseConnection\^(\^\fIsmcConn\fP, \fIcount\fP\^, \fIreasonMsgs\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    int \fIcount\fP\^;
.br
    char **\fIreasonMsgs\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIcount\fP 1i
The number of reason messages.
.IP \fIreasonMsgs\fP 1i
The reasons for closing the connection.
.LP
.eM
If
.PN SmcCloseConnection
is being called in response to a \fIDie\fP\^ message,
the connection is simply closed, and the reason messages are not sent
to the session manager.
.LP
Otherwise, \fIreasonMsgs\fP\^ contains a list of null terminated
string encoded in the current locale.  The session manager should
display these reason messages to the user.
.LP
Note that SMlib used the ICE protocol to establish a connection with
the session manager, and various protocols other than session management
may be active on the ICE connection.  When
.PN SmcCloseConnection
is called, the ICE connection will be closed only if all protocols
have been shutdown on the connection.  Check the ICElib
documentation for
.PN IceAddConnectionWatch
and
.PN IceRemoveConnectionWatch
to learn how to set up a callback to be invoked each time an ICE connection is
opened or closed.  Typically this callback adds/removes the ICE file
descriptor from the list of active descriptors (or calls
.PN XtAppAddInput
/
.PN XtRemoveInput ).
.LP
.NH 2
Modifying callbacks
.XS
\*(SN Modifying callbacks
.XE
.LP
To modify callbacks set up in
.PN SmcOpenConnection ,
call the
.PN SmcModifyCallbacks
function.
.LP
.sM
.FD 0
void SmcModifyCallbacks\^(\^\fIsmcConn\fP, \fImask\fP\^, \fIcallbacks\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    unsigned long \fImask\fP\^;
.br
    SmcCallbacks *\fIcallbacks\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fImask\fP 1i
A mask indicating which callbacks to modify.
.IP \fIcallbacks\fP 1i
The new callbacks.
.LP
.eM
When specifying a value for \fImask\fP\^, the following
values may be ORed together:
.LP
	
.PN SmcSaveYourselfProcMask
.br
	
.PN SmcDieProcMask
.br
	
.PN SmcShutdownCancelledProcMask
.LP
.sp 0.5
When changing all of the callbacks, the mask value of
.PN SmcAllProcMask
may be used.
.LP
The
.PN SmcCallbacks
are defined in the description of
.PN SmcOpenConnection .
.LP
.NH 2
Setting and Retrieving Session Management Properties
.XS
\*(SN Setting and Retrieving Session Management Properties
.XE
.LP
To set session management properties for this client, call the
.PN SmcSetProperties
function.
.sM
.FD 0
void SmcSetProperties\^(\^\fIsmcConn\fP, \fIsequenceRef\fP\^, \fInumProps\fP\^, \fIprops\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    unsigned long \fIsequenceRef\fP\^;
.br
    int \fInumProps\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIsequenceRef\fP 1i
The sequence number of the message that caused the \fISet Properties\fP\^
request.  If 0, then not responding to a previous message.
.IP \fInumProps\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to set.
.LP
.eM
The properties are specified as an array of property pointers.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled \fISession Management Properties\fP\^.
.LP
Previously set property values may be over-written using the
.PN SmcSetProperties
function.  Some properties have pre-specified semantics.
The session manager is required to store
non-predefined properties.  Note that the session manager is not
expected to restore the property values when the session is restarted.  As a
result, clients should not try to use the session manager as
a database for storing application specific state.
.LP
In order to get the sequence number of a \fISave Yourself\fP\^ message,
the client would call
.PN SmcGetIceConnection
and
.PN IceLastSequenceNumber
within the
.PN SmcSaveYourselfProc
callback.  Check the documentation for
.PN SmcGetIceConnection
below, and check the ICElib documentation for more details on the
.PN IceLastSequenceNumber
function.
.LP
.sp 0.5
To get session management properties previously stored by the client, call the
.PN SmcGetProperties
function.
.sM
.FD 0
void SmcGetProperties\^(\^\fIsmcConn\fP, \fIpropReplyProc\fP\^, \fIclientData\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmcPropReplyProc \fIpropReplyProc\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIpropReplyProc\fP 1i
The callback to be invoked when the properties reply comes back.
.IP \fIclientData\fP 1i
This pointer to client data will be passed to the
.PN SmcPropReplyProc
callback.
.LP
.eM
Note that the library does not block until the properties reply comes back.
Rather, a callback of type
.PN SmcPropReplyProc
is invoked when the data is ready.
.LP
.sM
typedef void (*SmcPropReplyProc)();
.LP
.FD 0
void PropReplyProc\^(\^\fIsmcConn\fP, \fIclientData\fP\^, \fInumProps\fP\^, \fIprops\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.br
    int \fInumProps\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIclientData\fP 1i
Client data specified when the callback was registered.
.IP \fInumProps\fP 1i
The number of properties returned.
.IP \fIprops\fP 1i
The list of properties returned.
.LP
.eM
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.  In order to free each
property, call the
.PN SmFreeProperty
function (see the section titled \fIFree Functions\fP\^).
Free the actual array of pointers with a call to free().
.LP
.NH 2
Interacting With the User
.XS
\*(SN Interacting With the User
.XE
.LP
After receiving a \fISave Yourself\fP\^ message with an \fIinteractStyle\fP\^
of
.PN SmInteractStyleError
or
.PN SmInteractStyleAny ,
the client may choose to interact with the user.
Since only one client can interact with the user at a time, the client
must call
.PN SmcInteractRequest
and wait for an \fIInteract\fP\^ message from the session manager.
.sM
.FD 0
void SmcInteractRequest\^(\^\fIsmcConn\fP, \fIdialogType\fP\^, \fIinteractProc\fP\^, \fIclientData\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    int \fIdialogType\fP\^;
.br
    SmcInteractProc \fIinteractProc\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIdialogType\fP 1i
The type of dialog the client wishes to present to the user.
.IP \fIinteractProc\fP 1i
The callback to be invoked when the \fIInteract\fP\^ message arrives from
the session manager.
.IP \fIclientData\fP 1i
This pointer to client data will be passed to the
.PN SmcInteractProc
callback when the \fIInteract\fP\^ message arrives.
.LP
.eM
The \fIdialogType\fP argument specifies either
.PN SmDialogError
indicating that the client wants to start an error dialog,
or
.PN SmDialogNormal ,
meaning that the client wishes to start a non-error dialog.
.LP
Note that if a shutdown is in progress, the user may have the option of
cancelling the shutdown.  If the shutdown is cancelled, the clients that
have not interacted yet with the user will receive a
\fIShutdown Cancelled\fP\^ message instead of the \fIInteract\fP\^ message.
.LP
The
.PN SmcInteractProc
callback will be invoked when the \fIInteract message\fP\^ arrives from
the session manager.
.LP
.sM
typedef void (*SmcInteractProc)();
.LP
.FD 0
void InteractProc\^(\^\fIsmcConn\fP, \fIclientData\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    SmPointer \fIclientData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIclientData\fP 1i
Client data specified when the callback was registered.
.LP
.sp 0.5
.eM
After interacting with the user (in response to an \fIInteract\fP\^ message),
call the
.PN SmcInteractDone
function.
.PN 
.sM
.FD 0
void SmcInteractDone\^(\^\fIsmcConn\fP, \fIcancelShutdown\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    Bool \fIcancelShutdown\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIcancelShutdown\fP 1i
If
.PN True ,
indicates that the user requests that the entire shutdown be cancelled.
.LP
.eM
.LP
\fIcancelShutdown\fP may only be
.PN True
if the corresponding \fISave Yourself\fP specified
.PN True
for \fIshutdown\fP\^ and
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny
for the \fIinteractStyle\fP\^.
.LP
.sp 0.5
.NH 2
Completing a Save Yourself
.XS
\*(SN Completing a Save Yourself
.XE
.LP
After saving state in response to a \fISave Yourself\fP\^ message,
call the
.PN SmcSaveYourselfDone
function.
.sM
.FD 0
void SmcSaveYourselfDone\^(\^\fIsmcConn\fP, \fIsuccess\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.br
    Bool \fIsuccess\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fIsuccess\fP 1i
If
.PN True ,
the \fISave Yourself\fP\^ operation was completed successfully.
.LP
.eM
Before calling
.PN SmcSaveYourselfDone ,
the client must have set each required property at least once since
the client registered with the session manager.
.LP
.sp 0.5
.NH 2
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
int SmcProtocolVersion\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns the major version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
int SmcProtocolRevision\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns the minor version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
char *SmcVendor\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns a string that provides some identification of the owner of
the session manager.  The string should be freed with a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmcRelease\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns a string that provides the release number of the session manager.
The string should be freed with a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmcClientID\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns a null terminated string for the client Id associated with
this connection.  This information was also returned in
.PN SmcOpenConnection
(it is provided here for convenience).
.LP
Call free() on this pointer when the client Id is no longer needed.
.LP
.sp 0.5
.sM
.FD 0
IceConn SmcGetIceConnection\^(\^\fIsmcConn\fP\^)
.br
    SmcConn \fIsmcConn\fP\^;
.FN
.eM
Returns the ICE connection object associated with this session management
connection object.  The ICE connection object can be used to get some
additional information about the connection.  Some of the more useful
functions which can be used on the IceConn are IceConnectionNumber,
IceConnectionString, and IceLastSequenceNumber.  Check the ICElib
documentation for more details on these functions.
.LP
.sp 0.5
.NH 2
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
If the client receives an unexpected protocol error from the session manager,
an error handler is invoked by SMlib.  A default error handler exists which
simply prints the error message and exits.  The client can change this error
handler by calling the
.PN SmcSetErrorHandler
function.
.LP
.sM
.FD 0
SmcErrorHandler SmcSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    SmcErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN SmcSetErrorHandler
returns the previous error handler.
.LP
The
.PN SmcErrorHandler
has the following type:
.LP
.sp 0.5
.sM
typedef void (*SmcErrorHandler)();
.br
.FD 0
void ErrorHandler\^(\^\fIsmc_conn\fP, \fIswap\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^, \fIerror_class\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIoffending_minor_opcode\fP\^;
.br
    unsigned long \fIoffending_sequence_num\fP\^;
.br
    int \fIerror_class\fP\^;
.br
    int \fIseverity\fP\^;
.br
    IcePointer \fIvalues\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffending_minor_opcode\fP 1i
The ICE minor opcode of the offending message.
.IP \fIoffending_sequence_num\fP 1i
The sequence number of the offending message.
.IP \fIerror_class\fP 1i
The error class of the offending message.
.IP \fIseverity\fP 1i
.PN IceCanContinue ,
.PN IceFatalToProtocol ,
or
.PN IceFatalToConnection .
.IP \fIvalues\fP 1i
Any additional error values specific to the minor opcode and class.
.LP
.eM
Note that this error handler is invoked for protocol related errors.
To install an error handler to be invoked when an IO error occurs, use
the
.PN IceSetIOErrorHandler
function described in the \fIInter-Client Exchange Library\fP\^ document.
.LP
.sp 0.5
.NH 1
Session Management Server (Sms) Functions
.XS
\*(SN Session Management Server (Sms) Functions
.XE
.LP
.NH 2
Initializing the Library
.XS
\*(SN Initializing the Library
.XE
.LP
.PN SmsInitialize
is the first SMlib function that should be called by a
session manager.  It provides information about the session manager,
and registers a callback which will be invoked each
time a new client connects to the session manager.
.LP
.sM
.FD 0
Status SmsInitialize\^(\^\fIvendor\fP, \fIrelease\fP\^, \fInewClientProc\fP\^, \fImanagerData\fP\^, \fIerrorLength\fP\^, \fIerrorStringRet\fP\^)
.br
    char *\fIvendor\fP\^;
.br
    char *\fIrelease\fP\^;
.br
    SmsNewClientProc \fInewClientProc\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    int \fIerrorLength\fP\^;
.br
    char *\fIerrorStringRet\fP\^;
.FN
.IP \fIvendor\fP 1i
A string specifying the session manager vendor.
.IP \fIrelease\fP 1i
A string specifying the session manager release number.
.IP \fInewClientProc\fP 1i
Callback to be invoked each time a new client connects to the session manager.
.IP \fImanagerData\fP 1i
When the
.PN SmsNewClientProc
callback is invoked, this pointer to manager data will be passed.
.IP \fIerrorLength\fP 1i
Length of the \fIerrorStringRet\fP\^ argument passed in.
.IP \fIerrorStringRet\fP 1i
Returns a null terminated error message, if any.  \fIerrorStringRet\fP
points to user supplied memory.  No more than \fIerrorLength\fP\^ bytes
are used.
.LP
.eM
After the
.PN SmsInitialize
function is called, the session manager should use the
.PN IceListenForConnections
function to listen for new connections, and
.PN IceAcceptConnection
to accept connections.  Refer to the section of this document titled
\fIUnderstanding SMlib's Dependence on ICE\fP\^ to learn more on how
SMlib and ICElib work together.  Also refer to the \fIInter-Client
Exchange Library\fP\^ document for further details on accepting ICE
connections.
.LP
Each time a new client connects to the session manager, the
.PN SmsNewClientProc
callback is invoked.  The session manager obtains a new opaque connection
object which it should use for all future interaction with the client.  The
session manager is also required to register additional callbacks for the
different messages that the client might send.
.bp
.sM
typedef void (*SmsNewClientProc)();
.LP
.FD 0
void NewClientProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIcallbacksRet\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    SmsCallbacks *\fIcallbacksRet\fP\^;
.FN
.IP \fIsmsConn\fP 1i
A new opaque connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIcallbacksRet\fP 1i
Mandatory set of callbacks to be registered by the session manager.
.LP
.eM
For each callback, the session manager can register a pointer to manager
data specific to that callback.  This pointer will be passed to the callback
when it is invoked by SMlib.
.LP
.sM
	typedef struct {
.sp 0.5
		struct {
.br
			SmsRegisterClientProc callback;
.br
			SmPointer manager_data;
.br
		} register_client;
.sp 0.5	
		struct {
.br
			SmsInteractRequestProc callback;
.br
			SmPointer manager_data;
.br
		} interact_request;
.sp 0.5
		struct {
.br
			SmsInteractDoneProc callback;
.br
			SmPointer manager_data;
.br
		} interact_done;
.sp 0.5
		struct {
.br
			SmsSaveYourselfDoneProc callback;
.br
			SmPointer manager_data;
.br
		} save_yourself_done;
.sp 0.5
		struct {
.br
			SmsCloseConnectionProc callback;
.br
			SmPointer manager_data;
.br
		} close_connection;
.sp 0.5
		struct {
.br
			SmsSetPropertiesProc callback;
.br
			SmPointer manager_data;
.br
		} set_properties;
.sp 0.5
		struct {
.br
			SmsGetPropertiesProc callback;
.br
			SmPointer manager_data;
.br
		} get_properties;
.sp 0.5
	} SmsCallbacks;
.LP
.eM
.NH 3
The Register Client Callback
.XS
\*(SN The Register Client Callback
.XE
.LP
The \fIRegister Client\fP\^ callback is the first callback that will be
invoked after the client connects to the session manager.  It's type is
.PN SmsRegisterClientProc .
.LP
.sM
typedef Status (*SmsRegisterClientProc();
.LP
.FD 0
Status RegisterClientProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIpreviousId\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    char *\fIpreviousId\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIpreviousId\fP 1i
The client ID from the previous session.
.LP
.eM
Before any further interaction takes place with the client,
the client must be registered with the session manager.
.LP
If the client is being restarted from a previous session,
\fIpreviousId\fP\^ will contain a null terminated string representing
the client ID from the previous session.  Call free() on the
\fIpreviousId\fP\^ pointer when it is no longer needed.
If the client is first joining the session, \fIpreviousId\fP\^ will be
.PN NULL .
.LP
If \fIpreviousId\fP\^ is invalid, the session manager should not register
the client at this time.  This callback should return a status of 0, which
will cause an error message to be sent to the client.  The client should
re-register with \fIpreviousId\fP\^ set to
.PN NULL .
.LP
Otherwise, the session manager should register the client with a
unique client ID by calling the
.PN SmsRegisterClientReply
function (to be discussed shortly), and the
.PN SmsRegisterClientProc
callback should return a status of 1.
.LP
.sp 0.5
.NH 3
The Interact Request Callback
.XS
\*(SN The Interact Request Callback
.XE
.LP
The \fIInteract Request\fP\^ callback is of type
.PN SmsInteractRequestProc .
.LP
.sM
typedef void (*SmsInteractRequestProc)();
.LP
.FD 0
void InteractRequestProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIdialogType\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    int \fIdialogType\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIdialogType\fP 1i
The type of dialog the client wishes to present to the user.
.LP
.eM	  
When a client receives a \fISave Yourself\fP\^ message with an
\fIinteractStyle\fP\^ of
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny ,
the client may choose to interact with the user.
Since only one client can interact with the user at a time, the client
must request to interact with the user.  The session manager should keep
a queue of all clients wishing to interact.  It should send an \fIInteract\fP\^
message to one client at a time and wait for an \fIInteract Done\fP\^ message
before continuing with the next client.
.LP
The \fIdialogType\fP argument specifies either
.PN SmDialogError
indicating that the client wants to start an error dialog,
or
.PN SmDialogNormal ,
meaning that the client wishes to start a non-error dialog.
.LP
If a shutdown is in progress, the user may have the option of cancelling
the shutdown.  If the shutdown is cancelled (specified in the \fIInteract
Done\fP\^ message), the session manager should send a
\fIShutdown Cancelled\fP\^ message to each client that requested to interact.
.LP
.sp 0.5
.NH 3
The Interact Done Callback
.XS
\*(SN The Interact Done Callback
.XE
.LP
When the client is done interacting with the user, the
.PN SmsInteractDoneProc
callback will be invoked.
.LP
.sM
typedef void (*SmsInteractDoneProc)();
.LP
.FD 0
void InteractDoneProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIcancelShutdown\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    int \fIcancelShutdown\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIcancelShutdown\fP 1i
Specifies if the user requests that the entire shutdown be cancelled.
.LP
.eM
Note that the shutdown can be cancelled only if the corresponding
\fISave Yourself\fP specified
.PN True
for \fIshutdown\fP\^ and
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny
for the \fIinteractStyle\fP\^.
.LP
.sp 0.5
.NH 3
The Save Yourself Done Callback
.XS
\*(SN The Save Yourself Done Callback
.XE
.LP
When the client is done saving its state in response to a
\fISave Yourself\fP\^ message, the
.PN SmsSaveYourselfDoneProc
will be invoked.
.LP
.sM
typedef void (*SmsSaveYourselfDoneProc)();
.LP
.FD 0
void SaveYourselfDoneProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIsuccess\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    Bool \fIsuccess\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIsuccess\fP 1i
If
.PN True ,
the \fISave Yourself\fP\^ operation was completed successfully.
.LP
.eM
Before the \fISave Yourself Done\fP\^ was sent, the client must have
set each required property at least once since it registered with the
session manager.
.LP
.sp 0.5
.NH 3
The Connection Closed Callback
.XS
\*(SN The Connection Closed Callback
.XE
.LP
If the client decides to terminate (not as a result of a \fIDie\fP\^
message), the
.PN SmsCloseConnectionProc
callback is invoked.
.LP
.sM
typedef void (*SmsCloseConnectionProc)();
.LP
.FD 0
void CloseConnectionProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIlocale\fP\^, \fIcount\fP\^, \fIreasonMsgs\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    char *\fIlocale\fP\^;
.br
    int \fIcount\fP\^;
.br
    char **\fIreasonMsgs\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIlocale\fP 1i
The locale in which the \fIreasonMsgs\fP\^ are encoded.
.IP \fIcount\fP 1i
The number of reason messages.
.IP \fIreasonMsgs\fP 1i
The reasons the client is closing the connection.
.LP
.eM
The session manager should display the reason messages to the user.
.LP
The \fIlocale\fP\^ string should be freed with a call to free().
Call
.PN SmFreeReasons
to free the reason messages.  See the section titled \fIFree Functions\fP\^.
.LP
.sp 0.5
.NH 3
The Set Properties Callback
.XS
\*(SN The Set Properties Callback
.XE
.LP
When the client sets session management properties, the
.PN SmsSetPropertiesProc
callback will be invoked.
.LP
.sM
typedef void (*SmsSetPropertiesProc)();
.LP
.FD 0
void SetPropertiesProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^, \fIsequenceRef\fP\^, \fInumProps\fP\^, \fIprops\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.br
    unsigned long \fIsequenceRef\fP\^;
.br
    int \fInumProps\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.IP \fIsequenceRef\fP 1i
The sequence number of the message that caused the \fISet Properties\fP\^
request.  If 0, then not responding to a previous message.
.IP \fInumProps\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to set.
.LP
.eM
The properties are specified as an array of property pointers.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.
.LP
Previously set property values may be over-written.  Some properties
have pre-specified semantics.
The session manager is required to store
non-predefined properties.
.LP
In order to free each property, call the
.PN SmFreeProperty
function.  See the section titled \fIFree Functions\fP\^.
Free the actual array of pointers with a call to free().
.LP
.sp 0.5
.NH 3
The Get Properties Callback
.XS
\*(SN The Get Properties Callback
.XE
.LP
When the client wants to retrieve properties that it previously set, the
.PN SmsGetPropertiesProc
callback is invoked.
.LP
.sM
typedef void (*SmsGetPropertiesProc)();
.LP
.FD 0
void GetPropertiesProc\^(\^\fIsmsConn\fP, \fImanagerData\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    SmPointer \fImanagerData\fP\^;
.FN
.IP \fIsmcConn\fP 1i
The session management connection object.
.IP \fImanagerData\fP 1i
Manager data specified when the callback was registered.
.LP
.eM
The session manager should respond by calling
.PN SmsReturnProperties .
\fIAll\fP\^ of the properties set for this client should be returned.
.LP
.sp 0.5
.NH 2
Registering the Client
.XS
\*(SN Registering the Client
.XE
.LP
In order to register a client (in response to a
.PN SmsRegisterClientProc
callback, call the
.PN SmsRegisterClientReply
function.
.LP
.sM
.FD 0
void SmsRegisterClientReply\^(\^\fIsmsConn\fP, \fIclientId\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    char *\fIclientId\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fIclientId\fP 1i
A null terminated string representing a unique client ID.
.LP
.eM
.LP
.sp 0.5
.NH 2
Sending a Save Yourself
.XS
\*(SN Sending a Save Yourself message
.XE
.LP
In order to send a \fISave Yourself\fP\^ to a client, call the
.PN SmsSaveYourself
function.
.LP
.sM
.FD 0
void SmsSaveYourself\^(\^\fIsmsConn\fP, \fIsaveType\fP\^, \fIshutdown\fP\^, \fIinteractStyle\fP\^, \fIfast\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    int \fIsaveType\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteractStyle\fP\^;
.br
    Bool \fIfast\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.IP \fIsaveType\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteractStyle\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.LP
.eM
The session manager sends a \fISave Yourself\fP\^ message to a client 
either to checkpoint it or just before
termination so that it can save its state.
The client responds with zero or more /fISet Properties\fP\^ messages
to update the properties indicating how to restart the client.
When all the properties have been set, the client sends a
\fISave Yourself Done\fP\^ message.
.LP
If \fIinteractStyle\fP\^ is
.PN SmInteractStyleNone ,
the client must not interact with the
user while saving state.
If \fIinteractStyle\fP\^ is 
.PN SmInteractStyleErrors ,
the client may interact with the user only if an error condition arises.  If
\fIinteractStyle\fP\^ is 
.PN SmInteractStyleAny ,
then the client may interact with the user for any purpose.
The client must send an \fIInteract Request\fP\^ message
and wait for an \fIInteract\fP\^ message from the session manager
before it can interact with the user.  When the client is done
interacting with the user, it should send an \fIInteract Done\fP\^ message.
The \fIInteract Request\fP\^ message can be sent any time after a
\fISave Yourself\fP\^ and before a \fISave Yourself Done\fP\^.
.LP
If \fIsaveType\fP\^ is
.PN SmSaveLocal ,
the client must update the properties to reflect its current state.
Specifically, it should save enough information to restore
the state as seen by the user of this client.  It should not affect the
state as seen by other users.  If \fIsaveType\fP\^ is
.PN SmSaveGlobal
the user wants the client to commit all of its data to permanent,
globally accessible storage.  If \fIsaveType\fP\^ is
.PN SmSaveBoth ,
the client should do both of these (it should first commit the data to
permanent storage before updating its properties).
.bp
.NH 2
Sending an Interact Message
.XS
\*(SN Sending an Interact Message
.XE
.LP
To send an \fIInteract\fP\^ message to a client, call the
.PN SmsInteract
function.
.sM
.FD 0
void SmsInteract\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.LP
.eM
The \fIInteract\fP\^ message grants the client the privilege of interacting
with the user.  When the client is done interacting with the user, it must
send an \fIInteract Done\fP\^ message to the session manager.
.LP
.sp 0.5
.NH 2
Sending a Die Message
.XS
\*(SN Sending a Die Message
.XE
.LP
To send a \fIDie\fP\^ message to a client, call the
.PN SmsDie
function.
.LP
.sM
.FD 0
void SmsDie\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.LP
.eM
It is expected that the session manager would send a \fISave Yourself\fP\^
message to the client before sending the \fIDie\fP\^ message.
<ISSUE - wait for Connection Closed>
.LP
.sp 0.5
.NH 2
Cancelling a Shutdown
.XS
\*(SN Cancelling a Shutdown
.XE
.LP
To cancel a shutdown, call the
.PN SmsShutdownCancelled
function.
.LP
.sM
.FD 0
void SmsShutdownCancelled\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.LP
.eM
The client can now continue as if the shutdown had never happened.
If the client has not sent a \fISave Yourself Done\fP\^ yet, it can
either abort the save and send a \fISave Yourself Done\fP\^ 
with the \fIsuccess\fP\^ field set to
.PN False ,
or it can continue with the save and send a \fISave Yourself Done\fP\^ 
with the \fIsuccess\fP\^ field set to reflect the outcome of the save.
.LP
.sp 0.5
.NH 2
Returning Properties
.XS
\*(SN Returning Properties
.XE
.LP
In response to a \fIGet Properties\fP\^ message, the session manager should
call the
.PN SmsReturnProperties
function.
.LP
.sM
.FD 0
void SmsReturnProperties\^(\^\fIsmsConn\fP\^, \fInumProps\fP\^, \fIprops\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.br
    int \fInumProps\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.LP
.IP \fInumProps\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to return to the client.
.LP
.eM
The properties are returned as an array of property pointers.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.
.LP
.sp 0.5
.NH 2
Cleaning Up After a Client Disconnects
.XS
\*(SN Cleaning Up After a Client Disconnects
.XE
.LP
When the session manager receives a \fIConnection Closed\fP message or
otherwise detects that the client aborted the connection, it should
call the
.PN SmsCleanUp
function in order to free up the connection object.
.LP
.sM
.FD 0
void SmsCleanUp\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.IP \fIsmsConn\fP 1i
The session management connection object.
.LP
.eM
.LP
.sp 0.5
.NH 2
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
int SmsProtocolVersion\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns the major version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
int SmsProtocolRevision\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns the minor version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
char *SmsVendor\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns a string that provides some identification of the owner of
the SMlib being used by the client.  The string should be freed with
a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmsRelease\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns a string that provides some identification of the release
number of the SMlib being used by the client.  The string should be
freed with a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmsClientID\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns a null terminated string for the client Id associated with
this connection.
.LP
Call free() on this pointer when the client Id is no longer needed.
.LP
.sp 0.5
.sM
.FD 0
IceConn SmsGetIceConnection\^(\^\fIsmsConn\fP\^)
.br
    SmsConn \fIsmsConn\fP\^;
.FN
.eM
Returns the ICE connection object associated with this session management
connection object.  The ICE connection object can be used to get some
additional information about the connection.  Some of the more useful
functions which can be used on the IceConn are IceConnectionNumber,
and IceLastSequenceNumber.  Check the ICElib documentation for more
details on these functions.
.LP
.sp 0.5
.NH 2
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
If the session manager receives an unexpected protocol error from a client,
an error handler is invoked by SMlib.  A default error handler exists which
simply prints the error message and exits.  The session manager can change
this error handler by calling the
.PN SmsSetErrorHandler
function.
.LP
.sM
.FD 0
SmsErrorHandler SmsSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    SmsErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN SmsSetErrorHandler
returns the previous error handler.
.LP
The
.PN SmsErrorHandler
has the following type:
.LP
.sp 0.5
.sM
typedef void (*SmsErrorHandler)();
.br
.FD 0
void ErrorHandler\^(\^\fIsms_conn\fP, \fIswap\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^, \fIerror_class\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIoffending_minor_opcode\fP\^;
.br
    unsigned long \fIoffending_sequence_num\fP\^;
.br
    int \fIerror_class\fP\^;
.br
    int \fIseverity\fP\^;
.br
    IcePointer \fIvalues\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffending_minor_opcode\fP 1i
The ICE minor opcode of the offending message.
.IP \fIoffending_sequence_num\fP 1i
The sequence number of the offending message.
.IP \fIerror_class\fP 1i
The error class of the offending message.
.IP \fIseverity\fP 1i
.PN IceCanContinue ,
.PN IceFatalToProtocol ,
or
.PN IceFatalToConnection .
.IP \fIvalues\fP 1i
Any additional error values specific to the minor opcode and class.
.LP
.eM
Note that this error handler is invoked for protocol related errors.
To install an error handler to be invoked when an IO error occurs, use
the
.PN IceSetIOErrorHandler
function described in the \fIInter-Client Exchange Library\fP\^ document.
.LP
.sp 0.5
.NH 1
Session Management Properties
.XS
\*(SN Session Management Properties
.XE
.LP
Each property is defined by the following structure:
.LP
	typedef struct {
.br
		char *name;			/* name of property */
.br
		char *type;			/* type of property */
.br
		int num_vals;		/* number of values */
.br
		SmPropValue *vals;	/* the list of values */
.br
	} SmProp;
.br
.LP	
	typedef struct {
.br
		int length;			/* the length of the value */
.br
		SmPointer value;		/* the value */
.br
	} SmPropValue;
.LP
.sp 0.5
In all of the SMlib functions related to properties, the properties are
passed as an array of
.PN SmProp
pointers.
.LP
	int numProps;
.br
	SmProp **props;
.LP
Specifying the properties in this format is ideal for
.PN SmsSetPropertiesProc
callback, and has been adapted for all other property related functions.
.LP
When the client sets properties in the session manager, some of the properties
may have been previously set, and some of the properties will be set for the
first time.  As a result, the session manager must \fImerge\fP the properties
in it's current database.  Specifying the properties as an array of
.PN SmProp
pointers makes this much simpler for the session manager.
.LP
The X Session Management Protocol defines a list of predefined properties,
several of which are required to be set by the client.
.LP
<paste from XSMP>
.LP
.sp 0.5
.NH 1
Free Functions
.XS
\*(SN Free Functions
.XE
.LP
To free an individual property, call the
.PN SmFreeProperty
function.
.LP
.sM
.FD 0
void SmFreeProperty\^(\^\fIprop\fP\^)
.br
    SmProp *\fIprop\fP\^;
.FN
.IP \fIprop\fP 1i
The property to free.
.LP
.eM
.LP
To free the reason strings from the
.PN SmsCloseConnectionProc
callback, call the
.PN SmFreeReasons
function.
.LP
.sM
.FD 0
void SmFreeReasons\^(\^\fIcount\fP, \fIreasons\fP\^)
.br
    int \fIcount\fP\^;
.br
    char **\fIreasons\fP\^;
.FN
.IP \fIcount\fP 1i
The number of reason strings.
.IP \fIreasons\fP 1i
The list of reason strings to free.

.bp
authentication

client ids


.bp



.LP
.eM
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.bp
.TC
