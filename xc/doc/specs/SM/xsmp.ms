.\" Use tbl, -ms, and macros.t
.\" $XConsortium: xsmp.ms,v $
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.\" macro: start marker
.de sM
.KS
.sp 1
\\h'-0.5i'\\L'-1v'\\v'2p'\\l'1v'\\v'1v-2p'
.sp -1
..
.\" macro: end marker
.de eM
.sp -1
\\h'-0.5i'\\L'-1'\\v'1v+2p'\\l'1v'\\v'-2p'
.sp 1
.KE
..
.fp 6 CI
.fp 7 C
.fp 8 CB
.ps 10
.nr PS 10
\&
.TL
\s+2\fBX Session Management Protocol\fP\s-2
.sp
Version 0.21
.sp
X Consortium Work In Progress
.sp
X Version 11, Release 6
.AU
Mike Wexler
.AI
Kubota Pacific Computer, Inc.
.AB
.LP
This document specifies a protocol that facilitates the management of groups
of client applications by a session manager.  The session manager can cause
clients to save their state, to shut down, and to be restarted into a
previously saved state.  This protocol is layered on top of the X
Consortium's ICE protocol.
.AE
.LP
.bp
.sp 8
.LP
.DS C
X Window System is a trademark of M.I.T.
.sp
Copyright \(co 1992, 1993 by the Massachusetts Institute of Technology
.DE
.sp 3
.LP
Permission to use, copy, modify, and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.  MIT makes no
representations about the suitability for any purpose of the information in
this document.  This documentation is provided ``as is'' without express or
implied warranty.
.bp 1
.EH '\fBX Session Management Protocol\fP''\fBX11, Release 6 post-alpha\fP'
.OH '\fBX Session Management Protocol\fP''\fBX11, Release 6 post-alpha\fP'
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Acknowledgements
.XS
\*(SN Acknowledgements
.XE
.LP
First I would like to thank the entire ICCCM and Intrinsics working groups for
the comments and suggestions. I would like to make special thanks to the 
following people (in alphabetical order), Jordan Brown, Ellis Cohen, Donna 
Converse, Vania Joloboff, Stuart Marks, Ralph Mor and Bob Scheifler.
.NH 1
Definitions and Goals
.XS
\*(SN Definitions and Goals
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
uncommitted transaction.
The means by which clients are
restarted is unspecified by this protocol.
.LP
For purposes of this protocol, a \fIclient\fP of the session manager is
defined as a connection to the session manager.  A client is typically,
though not necessarily, a process running an application program connected
to an X window system display.  However, a client may be connected to more
than one X display or not be connected to any X displays at all.
.LP
This protocol is layered on top of the X Consortium's ICE protocol, and relies on
the ICE protocol to handle connection management and authentication.
.LP
.NH 1
Overview of the Protocol
.XS
\*(SN Overview of the Protocol
.XE
.LP
Clients use XSMP to register themselves with the session manager (SM).  When
a client starts up, it should connect to the SM.  The client should remain
connected for as long as it runs.  A client may resign from the session by
issuing the proper protocol messages before disconnecting.  Termination of
the connection without notice will be taken as an indication that the client
died unexpectedly.
.LP
Clients are expected to save their state in such a way as to allow multiple
instantiations of themselves to be managed independently.  A unique value
called a \fIclient-ID\fP is provided by the protocol for the purpose of
disambiguating multiple instantiations of clients.  Clients may use this ID,
for example, as part of a filename in which to store the state for a
particular instantiation.  The client-ID should be saved as part of the
command used to restart this client (the \fIRestartCommand\fP\^) so that the
client will retain the same ID after it is restarted.  Certain small pieces
of state may also be stored in the RestartCommand.  For example, an X11 client
may place the `\-iconic' option in its RestartCommand to indicate that it
should appear as an icon when it is restarted.
.LP
The client finds the network address of the SM in a system-dependent way.
On POSIX systems an environment variable called SESSION_MANAGER will contain
a list of network IDs. Each id will contain the transport name followed by a 
slash and the (transport-specific)
address.  A TCP/IP address would look like this:
.ID
	\f7tcp/\fP\fIhostname\fP\^\f7:\fP\^\fIportnumber\fP
.DE
where the hostname is a fully qualified domain name.
A Unix Domain address looks like this:
.ID
	\f7local/\fP\fIhostname\fP\^\f7:\fP\^\fIpath\fP
.DE
A DECnet address would look like this:
.ID
	\f7decnet/\fP\fInodename\fP\^\f7::sm$\fP\^\fIobjname\fP
.DE
If multiple network IDs are specified, they should be separated by a comma.
.NT Rationale
There was much discussion over whether the XSMP protocol should use X as
the transport protocol or whether it should use its own independent
transport.  It was decided that it would use an independent protocol for
several reasons.  First, the SM manager should be able to manage programs that
don't maintain an X connection.  Second, the X protocol is not appropriate to
use as a general-purpose transport protocol.  Third, a session might
span multiple displays.
.LP
The protocol is connection based, because there is no other way for the SM
to determine reliably when clients terminate.
.LP
It should be noted that this protocol introduces another single point of 
failure into the system.  Although it is possible for clients to continue 
running after the SM has exited, this will probably not be the case in 
normal practice. Normally the program that starts the SM will consider the
session to be terminated when the SM exits (either normally or abnormally).
.LP
To get around this would require some sort of 
rendezvous server that would also introduce a single point of failure.  In the
absence of a generally available rendezvous server, XSMP is kept simple in
the hopes of making simple reliable SMs.
.NE
.LP
Some clients may wish to manage the programs they start.  For example, a
mail program could start a text editor for editing the text of a mail
message.  A client that does this is a session manager itself;
it should supply the clients it starts with the appropriate connection
information (i.e. the SESSION_MANAGER environment variable) that specifies
a connection to itself instead of to the ``real'' session manager.
.LP
Each client has associated with it a list of properties. 
A property set by one client is not visible to any other client.
These properties are used for the client to inform the SM of the clients
current state.
When a client initially connects to the SM, there are no properties set.
.NH 1
Data Types
.XS
\*(SN Data Types
.XE
.LP
XSMP messages contain several types of data.  Both the SM and the client
always send messages in their native byte order.  Thus, both sides may need
to byte-swap the messages received.  The need to do byte-swapping is
determined at run-time by the ICE protocol. 
.NT Rationale
As XSMP will not be used to
transport large quantities of data, this restriction will not substantially
hamper the efficiency of any implementation.
.NE
.LP
If an invalid value is specified for a field of any of the enumerated types, a
.PN BadValue
error message must be sent by the receiver of the message to the sender of the
message.
.TS H
expand;
lB lB lB
l l lw(3.5i).
_
.sp 6p
Type Name	Length (bytes)	Description
.sp 6p
_
.sp 6p
.TH
.R
BOOL	1	T{
.PN False
or
.PN True
T}
INTERACT_STYLE	1	T{
.PN None ,
.PN Errors ,
or
.PN Any
T}
DIALOG_TYPE	1	T{
.PN Error
or
.PN Normal
T}
SAVE_TYPE	1	T{
.PN Global ,
.PN Local ,
or
.PN Both
T}
CARD8	1	a one-byte unsigned integer
CARD16	2	a two-byte unsigned integer
CARD32	4	a four-byte unsigned integer
ARRAY8	4 + \fIn\fP	T{
A CARD32, \fIn\fP\^,
specifying the number of CARD8 values, followed by \fIn\fP CARD8
values.  After the CARD8 values will be enough
pad bytes to reach the next 8-byte boundary.
T}
LISTofARRAY8	?	T{
A CARD32 specifying the number of ARRAY8 values, followed by that many
ARRAY8 values.
T}
PROPERTY	?	T{
An ARRAY8 specifying the name of the
property, followed by an ARRAY8 specifying the
type of the property, followed by a \%LISTofARRAY8 containing the value.
The type of the value
is specified by the type field.
The type field is one of the types described in this
table.
T}
LISTofPROPERTY	?	T{
A CARD32 specifying the number of
properties, followed by that many \%PROPERTYs.  Each property will begin
on an eight byte boundary.
T}		
.sp 6p
_
.TE
.NH 1
Protocol Setup and Message Format
.XS
\*(SN Protocol Setup and Message Format
.XE
.LP
To start the XSMP protocol, the client sends the server an ICE
.PN ProtocolSetup
message.  The protocol-name field should be specified as ``XSMP'', the major
version of the protocol should be one, and the minor version should be zero.
These values may change if the protocol is revised.  The minor version
number will be incremented if the change is compatible, otherwise the major
version number will be incremented.
.LP
All XSMP messages are in the standard ICE packet format.  The packet's major
opcode is assigned to XSMP by ICE at run-time.  The different parties
(client and SM) may be assigned different major opcodes for XSMP.  Once
assigned, all XSMP packets issued by this party will use the same major
opcode.  The packet's minor opcode specifies which protocol message this
packet contains. 
.NH 1
Protocol
.XS
\*(SN Protocol
.XE
.LP
The protocol consists of a sequence of messages as described below.  Each
message type is specified by an ICE minor opcode.  A given message type is
sent either from a client to the session manager or from the session manager
to a client; the appropriate direction is listed with each message's
description.  For each message type, the set valid responses and possible error
messages are listed.  The ICE severity is given in parentheses following
each error class.
.LP
.sM
.PN RegisterClient
[Client \(-> SM]
.RS
.LP
\fIprevious-ID\fP\^: ARRAY8
.LP
Valid Responses: 
.PN RegisterClientReply
.LP
Possible Errors:
.PN BadValue
.Pn ( CanContinue )
.RE
.eM
.LP
The client must send this message to the SM to register the client's existence.
If a client is being restarted from a previous
session, the previous-ID field must contain the client ID from the
previous session.  The client ID may not contain bytes that contain the value 0.
For new clients, previous-ID should be null (i.e. it
should be of zero length).
.LP
If previous-ID is not valid, the SM will send a
.PN BadValue
error message to the client.
At this point the SM reverts to the register state and waits for another
.PN RegisterClient .
The client should then send a
.PN RegisterClient
with a null previous-ID field.
.LP
.sM
.PN RegisterClientReply
[Client \(<- SM]
.RS
.LP
\fIclient-ID\fP\^: ARRAY8
.LP
Valid Responses:
.PN SetProperties
.RE
.eM
.LP
The client-ID specifies a unique identification for this client.
If the client had specified an ID in the previous-ID field of the
.PN RegisterClient
message, client-ID will be identical to the previously specified ID.  If
previous-ID was null, client-ID will be a unique ID freshly generated by the
SM.
.LP
The client ID should be globally unique. This means that any client
ID generated on any system anywhere should be unique throughout all
machines in the universe.
.LP
This globally unique ID consists of the following pieces: a version,
an address type, an address, a time stamp, a process ID format,
a process ID, and a sequence number. The version identifies which
version of the SM ID scheme is being used. Currently only
version 1 is defined and this field should contain the character 1.
The address and address type
together identify a specific machine that the ID was generated on.
The address ID format is a one character field. The following types
are defined:
.nf
.ta .2i .5i 2.0i
	1	a 4 byte IP address encoded as 8 hexadecimal digits
	2	a 6 byte DECNET address encoded as 12 hex digits
.fi
Note that the numbers above are actually the ISO-8859 characters '1' 
and '2'. Also, the hexadecmail encodings user digits and uppercase
letters only.
.NT "Example"
The IP address 198.112.45.11 would be represented as 1C6702D0B.
.NE
The address is one the host addresses of the machine where the SM is 
running.
The timestamp is a number consisting of 13 decimal
digits specifying the number of milliseconds since Midnight UTC 
Jan 1 1970. 
The process ID type is a one character field with the following types
defined:
.nf
.ta .2i .5i 2.0i
	1	a Unix process ID encoded as a 10 left padded 
                deciaml digits
.fi
The 1 above is actually the ISO 8859-1 character '1'.
The Process ID is the Process ID of the SM.
The sequence number is 4 decimal digits encoded in ISO8859-1.
It is incremented
every time the SM hands out an ID. When it reach 9999 it wraps to
0000.
.NT "Rationale"
This is to facilitate the passing around of session or portions
of session between users and machines.
.NE
.LP
When a client receives a 
.PN RegisterClientReply
message, it should
set its
.PN RestartCommand
property using the
.PN SetProperties
message.
The client must also set any other properties that are needed to restart
the client if it crashes,
specifically, properties such as CurrentDirectory and Environment.
.LP
.sM
.PN SaveYourself
[Client \(<- SM]
.RS
.LP
\fItype\fP\^: SAVE_TYPE
.br
\fIshutdown\fP\^: BOOL
.br
\fIinteract-style\fP\^: INTERACT_STYLE
.br
\fIfast\fP\^: BOOL
.LP
Valid Responses:
.PN SetProperties ,
.PN SaveYourselfDone ,
.PN InteractRequest
.RE
.eM
.LP
The SM sends this message to a client 
either to checkpoint it or just before
termination so that it can save its state.
The client responds with zero or more 
.PN SetProperties
messages to update
the properties indicating how to restart the client.
When all the 
.PN SetProperties
messages have been sent, the client should send a
.PN SaveYourselfDone
message.
.LP
If interact-style is
.PN None ,
the client must not interact with the
user while saving state.  If the interact-style is 
.PN Error ,
the client
may interact with the user only if an error condition arises.  If
interact-style is 
.PN Any ,
then the client may interact with the user for
any purpose.
This is done by sending an
.PN InteractRequest
message.  The SM will send an
.PN Interact
message to
each client that sent an
.PN InteractRequest.  
The client must postpone all
interaction until it gets the
.PN Interact
message.  When the client is done
interacting it should send the SM an
.PN InteractDone
message. The 
.PN InteractRequest
message can be sent any time after a
.PN SaveYourself
and before a 
.PN SaveYourselfDone .
.LP
When a client receives
.PN SaveYourself
and has not yet responded
.PN SaveYourselfDone
to a previous
.PN SaveYourself ,
it must send a
.PN SaveYourselfDone
and may then begin responding as appropriate
to the newly received 
.PN SaveYourself .
.LP
The type field specifies the type of information that should be saved:
.PN Global ,
.PN Local ,
or
.PN Both .
The Local type indicates that the application must update the
properties to reflect its current state, send a
.PN SaveYourselfDone
and continue.  Specifically it should save enough information to restore
the state as seen by the user of this client.  It should not affect the
state as seen by other users.
The
.PN Global
type indicates that the user wants the client to 
commit all of its data to permanent, globally accessible
storage.
.PN Both
indicates that the client should do both of these.  If
.PN Both
is specified, the client should first commit the data to permanent storage
before updating its SM properties.
.NT Examples
If a word processor was sent a 
.PN SaveYourself
with a type of 
.PN Local ,
it could create a temporary file that included the
current contents of the file, the location of the cursor, and
other aspects of the current editing session.  It would then update
its 
.PN RestartCommand 
property with enough information to find this 
temporary file.
.LP
If a word processor was sent a 
.PN SaveYourself
with a type of
.PN Global ,
it would simply save the currently edited file.
.LP
If a word processor was sent a 
.PN SaveYourself
with a type of
.PN Both ,
it would first save the currently edited file.  It would then create a
temporary file with information such as the current position of the cursor
and what file is being edited.  Finally, it would update its 
.PN RestartCommand
property with enough information to find the temporary file.
.NE
.LP
The shutdown field specifies whether the the system is being shut down.
.NT Rationale
The interaction
is different depending on whether or not shutdown is set.
If not shutting down,
then the client can save and resume normal operation. 
If shutting down,
the client must save and then must prevent interaction 
until it receives either a "Die" or a "ShutdownCancelled",
because anything the user does after the save will be lost.
.NE
The fast field specifies that the client should save its state as quickly as
possible.  For example, if the SM knows that power is about to fail, it
should set the fast field to
.PN True .
.NT "Advice to Implementors"
Both the client and the SM should save the sequence number of the
.PN SaveYourself
message as an ID of this particular save event.
It is used in the 
.PN SetProperties,
.PN SaveYourselfDone,
.PN Interact,
.PN InteractRequest,
and
.PN InteractDone
messages to resolve race conditions.
.NE
.LP
.sM
.PN InteractRequest
[Client \(-> SM]
.RS
.LP
\fIdialog-type\fP\^: DIALOG_TYPE
.br
\fIreference-sequence-number\fP\^: CARD32
.LP
Valid Responses:
.PN Interact ,
.PN ShutdownCancelled
.LP
.RE
.eM
.LP
During a checkpoint or session-save operation,
only one client at a time is granted the privilege of interacting with
the user.  The
.PN InteractRequest
message causes the SM to emit an
.PN Interact
message at some later time if the shutdown is not cancelled
by another client first.
.LP
The dialog-type field specifies either
.PN Error
indicating that the 
client wants to start an error dialog or
.PN Normal ,
meaning the client 
wishes to start a non-error dialog.
.LP
If the SM hasn't initiated a 
.PN SaveYourself
or the reason is not valid in
the current 
.PN SaveYourself ,
then a
.PN BadState
error message will be 
sent to the client.
.LP
The client must set the reference-sequence-number field to the 
sequence number of the
.PN SaveYourself
message that caused this 
.PN InteractRequest.
.RE
.LP
.sM
.PN Interact
[Client \(<- SM]
.RS
.LP
\fIreference-sequence-number\fP\^: CARD32
.LP
Valid Responses:
.PN InteractDone
.LP
.RE
.eM
.LP
This message grants the client the privilege of interacting with the
user.  When the client is done interacting with the user it must
send an 
.PN InteractDone
message to the SM.
The reference-sequence-number field must contain the sequence number of
the 
.PN SaveYourself
message that caused this
.PN Interact
message.
.LP
.sM
.PN InteractDone
[Client \(-> SM]
.RS
.LP
\fIcancel-shutdown\fP\^: BOOL
.br
\fIreference-sequence-number\fP\^: CARD32
.LP
Valid Responses:
.PN ShutdownCancelled
.LP
.RE
.eM
.LP
This message is used by a client to notify the SM that it is done interacting
and that the SM can move onto the next client.
.LP
The cancel-shutdown field specifies that the user requests that the entire
shutdown be cancelled.  Cancel-shutdown may only be
.PN True
if the corresponding
.PN SaveYourself
message specified
.PN True
for the shutdown field and
.PN Any
or
.PN Errors
for the interact-style field.  Otherwise, cancel-shutdown must be
.PN False .
If cancel-shutdown is
.PN True
when the corresponding
.PN SaveYourself
message did not allow it, the SM will generate a
.PN BadState
error message.
.LP
The reference-sequence-number field contains the sequence number of the
.PN SaveYourself
message that caused this interaction.
.LP
.sM
.PN SaveYourselfDone
[Client \(-> SM]
.RS
.LP
\fIsuccess\fP\^: BOOL
.br
\fIreference-sequence-number\fP\^: CARD32
.LP
Valid Responses: 
.PN Die ,
.PN ShutdownCancelled
.LP
.RE
.eM
.LP
This message is sent by a client to indicate that all of the properties
representing its state have been updated.
If the 
.PN SaveYourself
message had the shutdown flag set to 
.PN True ,
the client must wait for a
.PN ShutdownCancelled
or
.PN Die
message before changing its state.
Before issuing a
.PN SaveYourselfDone ,
a client must have set each of required
properties at least once since the client registered with the SM.
The reference-sequence-number field should contain the sequence number
of the
.PN SaveYourself 
message that is being completed.
If the 
.PN SaveYourself
operation was successful, then the client
should set the Success field to
.PN True ;
otherwise the client should set
it to
.PN False .
.NT Example
If a client tries to save its state and runs out of disk space,
it might return 
.PN False
in the success
field of the 
.Pn SaveYourselfDone
message.
.NE
.LP
.sM
.PN Die
[Client \(<- SM]
.RS
.LP
Valid Responses:
.PN ConnectionClosed
.RE
.eM
.LP
When the SM wants a client to die it sends a
.PN Die
message.  Before the client dies it responds
by sending a 
.PN ConnectionClosed
message and may then close
its connection to the SM at any time.
.LP
.sM
.PN ShutdownCancelled
[Client \(<- SM]
.RS
.RE
.eM
.LP
The shutdown currently in process has been aborted.  The client can now
continue as if the shutdown had never happened.
If the client has not sent
.PN SaveYourselfDone
yet, the client can either
abort the save and send 
.PN SaveYourselfDone
with the success field
set to
.PN False ,
or it can continue with the save and send a
.PN SaveYourselfDone
with the success field set to reflect the outcome
of the save.
.LP
.sM
.PN ConnectionClosed
[Client \(-> SM]
.RS
.LP
\fIreason\fP\^: LISTofARRAY8
.RE
.eM
.LP
Specifies that the client has decided to terminate.
It should be immediately followed by closing the connection.
.LP
The reason field specifies why the client is resigning from the session. It is encoded as an array of compound text strings.
If the resignation is
expected by the user, there will typically be zero ARRAY8s here.  But
if the client encountered an unexpected fatal error, the error message
(which might otherwise be printed on stderr on a POSIX system) should
be forwarded to the SM here, one ARRAY8 per line of the
message.  It is the responsibility of the SM to display
this reason to the user.
.LP
After sending this message, the client must not send any additional XSMP
messages to the SM.
.NT "Advice to Implementors"
If additional messages are received, they should be discarded.
.NE
.NT Rationale
The reason for sending the
.PN ConnectionClosed
message before
actually closing the connections is that some transport protocols will
not provide immediate notification of connection closure.
.NE
.LP
.sM
.PN SetProperties
[Client \(-> SM]
.RS
.LP
\fIreference-sequence-number\fP\^: CARD32
.br
\fIproperties\fP: LISTofPROPERTY
.RE
.eM
.LP
Sets the specified properties to the specified value.
Existing properties not specified in the 
.PN SetProperties
message are unaffected.
Some properties have pre-specified semantics. If a client sets a property that
is not defined by the XSMP, the property should be stored.  See
the section on pre-specified properties.  If the
.PN SetProperties
is in response to 
either a 
.PN RegisterClientReply
or a
.PN SaveYourself
message, the client should specify the sequence number
of this message in the reference-sequence-number field.  Otherwise,
the client must set the reference-sequence-number field to zero.
.LP
.sM
.PN DeleteProperties
[Client \(-> SM]
.RS
.LP
.br
\fIproperty-names\fP: LISTofARRAY8
.RE
.eM
.LP
Removes the named properties.
.LP
.sM
.PN GetProperties
[Client \(-> SM]
.RS
.LP
Valid Responses:
.PN GetPropertiesReply
.RE
.eM
.LP
Requests that the SM respond with the
values of all the properties for this client.
.LP
.sM
.PN GetPropertiesReply
[Client \(<- SM]
.RS
.LP
\fIvalues\fP\^: LISTofPROPERTY
.RE
.eM
.LP
This message is sent in reply to a
.PN GetProperties
message and includes
the values of all the properties.
.NH 1
Errors
.XS
\*(SN Errors
.XE
.LP
When the receiver of a message detects an error condition, the receiver should send
an ICE error message to the receiver. 
There are only two types of errors that are used by the XSMP:
.PN BadValue 
and
.PN BadState.
These are both defined in the ICE protocol.
.LP
Any message received out-of-sequence
will generate a
.PN BadState
error message.
.NH 1
State Diagrams
.XS
\*(SN State Diagrams
.XE
.LP
These state diagrams are designed to cover all actions of both
the client and the SM. 
.NH 2
Client State Diagram
.XS
\*(SN Client State Diagram
.XE
.LP
.nf
.DS L 0
\f6start:\fP
	Initiate ICE protocol setup \(-> \f7wait-for-setup\fP
.DE
.sp
.DS L 0
\f6wait-for-setup:\fP
	ICE protocol setup complete \(-> \f7register\fP
.DE
.sp
.DS L 0
\f6register:\fP
	send \fBRegisterClient\fP \(-> \f7collect-id\fP
.DE
.sp
.DS L 0
\f6collect-id:\fP
	receive \fBRegisterClientReply\fP \(-> \f7set-restart-command\fP
.DE
.sp
.DS L 0
\f6set-restart-command:\fP
	send \fBSetProperties\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6shutdown-cancelled:\fP
	send \fBSaveYourselfDone\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6idle:\fP [Undoes any freeze of interaction with user.] 
	receive \fBDie\fP \(-> \f7die\fP
	receive \fBSaveYourself\fP \(-> \f7freeze-interaction\fP
	send \fBSetProperties\fP \(-> \f7idle\fP
	send \fBDeleteProperties\fP \(-> \f7idle\fP
	send \fBConnectionClosed\fP \(-> \f7connection-closed\fP
.DE
.sp
.DS L 0
\f6die:\fP
	send \fBConnectionClosed\fP \(-> \f7connection-closed\fP
.DE
.sp
.DS L 0
\f6freeze-interaction:\fP
	freeze interaction with user \(-> \f7save-yourself\fP
.DE
.sp
.DS L 0
\f6save-yourself:\fP
	receive \fBShutdownCancelled\fP \(-> \f7shutdown-cancelled\fP
	send \fBSetProperties\fP \(-> \f7save-yourself\fP
	send \fBInteractRequest\fP \(-> \f7interact-request\fP
	if shutdown mode:
		send \fBSaveYourselfDone\fP \(-> \f7save-yourself-done\fP
	otherwise:
		send \fBSaveYourselfDone\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6interact-request:\fP
	receive \fBInteract\fP \(-> \f7interact\fP
	receive \fBShutdownCancelled\fP \(-> \f7shutdown-cancelled\fP
.DE
.sp
.DS L 0
\f6interact:\fP
	send \fBInteractDone\fP \(-> \f7save-yourself\fP
	receive \fBShutdownCancelled\fP \(-> \f7shutdown-cancelled\fP
.DE
.sp
.DS L 0
\f6save-yourself-done:\fP
	receive \fBDie\fP \(-> \f7die\fP
	receive \fBShutdownCancelled\fP \(-> \f7idle\fP
.DE
.NH 2
Session Manager State Diagram
.XS
\*(SN Session Manager State Diagram
.XE
.LP
.nf
.DS L 0
\f6start:\fP
	receive \fBProtocolSetup\fP \(-> \f7protocol-setup\fP
.DE
.sp
.DS L 0
\f6protocol-setup:\fP
	send \fBProtocolSetupReply\fP \(-> \f7register\fP
.DE
.sp
.DS L 0
\f6register:\fP
	receive \fBRegisterClient\fP \(-> \f7acknowledge-register\fP
.DE
.sp
.DS L 0
\f6acknowledge-register:\fP
	send \fBRegisterClientReply\fP \(-> \f7wait-for-properties\fP
.DE
.sp
.DS L 0
\f6wait-for-properties:\fP
	receive \fBSetProperties\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6idle:\fP
	receive \fBSetProperties\fP \(-> \f7idle\fP
	receive \fBDeleteProperties\fP \(-> \f7idle\fP
	receive \fBConnectionClosed\fP \(-> \f7start\fP
	receive \fBGetProperties\fP \(-> \f7get-properties\fP
	send \fBSaveYourself\fP \(-> \f7saving-yourself\fP
.DE
.sp
.DS L 0
\f6get-properties:\fP
	send \fBGetPropertiesReply\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6saving-yourself:\fP
	receive \fBInteractRequest\fP \(-> \f7saving-yourself\fP
	send \fBInteract\fP \(-> \f7saving-yourself\fP
	receive \fBInteractDone\fP \(-> \f7saving-yourself\fP
	receive \fBSetProperties\fP \(-> \f7saving-yourself\fP
	if shutting down:
		receive \fBSaveYourselfDone\fP \(-> \f7save-yourself-done\fP
	otherwise
		receive \fBSaveYourselfDone\fP \(-> \f7idle\fP
.DE
.sp
.DS L 0
\f6save-yourself-done:\fP
	If all clients are saved 
	send \fBDie\fP \(-> \f7die\fP
.sp
	If some clients are not saved:
	\(-> \f7saving-yourself\fP
.DE
.NH 1
Protocol Encoding
.XS
\*(SN Protocol Encoding
.XE
.NH 2
Types
.XS
\*(SN Types
.XE
.LP
.nf
.ta .2i .5i 2.0i
BOOL
	0	False
	1	True
.sp
INTERACT_STYLE
	0	None
	1	Errors
	2	Any
.sp
DIALOG_TYPE
	0	Error
	1	Normal
.sp
SAVE_TYPE
	0	Global
	1 	Local
	2 	Both
.sp
ARRAY8
	4	CARD32	length
	n	LISTofCARD8	the array
	p		p = pad (4 + n, 8)
.sp
LISTofARRAY8
	4	CARD32	count
	4		unused
	a	ARRAY8	first array
	b	ARRAY8	second array
	\&.
	\&.
	\&.
	q	ARRAY8	last array
.sp
PROPERTY
	a	ARRAY8	name
	b	ARRAY8	type
	c	LISTofARRAY8	values
.sp
LISTofPROPERTY
	4       CARD32	count
	4       	unused
	a       PROPERTY	first property
	b       PROPERTY	second property
	\&.
	\&.
	\&.
	q	PROPERTY	last property
.NH 2
Messages
.XS
\*(SN Messages
.XE
.LP
XSMP is a sub-protocol of ICE.  The major opcode is assigned at run-time
by ICE and is represented here by `?'.
.LP
.nf
.ta .2i .5i 2.0i 
.PN RegisterClient
	1	?	XSMP
	1	1	opcode
	2		unused
	4	a/8	length of remaining data in 8-byte units
	a	ARRAY8	previous-ID
.sp
.PN RegisterClientReply
	1	?	XSMP
	1	2	opcode
	2		unused
	4	a/8	length of remaining data in 8-byte units
	a	ARRAY8	client-ID
.sp
.PN SaveYourself
	1	?	XSMP
	1	3	opcode
	2		unused
	4	1	length of remaining data in 8-byte units
	1	SAVE_TYPE	type
	1	BOOL	shutdown
	1	INTERACT_STYLE	interact-style
	1	BOOL	fast
	4		unused
.sp
.PN InteractRequest
	1	?	XSMP
	1	4	opcode
	1	DIALOG_TYPE	dialog type
	1		unused
	4	1	length of remaining data in 8-byte units
	4	CARD32	reference-sequence-number
	4	unused	pad
.sp
.PN Interact
	1	?	XSMP
	1	5	opcode
	2		unused
	4	1	length of remaining data in 8-byte units
	4	CARD32	reference-sequence-number
	4	unused	pad
.sp
.PN InteractDone
	1	?	XSMP
	1	6	opcode
	1	BOOL	cancel-shutdown
	1		unused
	4	1	length of remaining data in 8-byte units
	4	CARD32	reference-sequence-number
	4	unused	pad
.sp
.PN SaveYourselfDone
	1	?	XSMP
	1	7	opcode
	1	BOOL	success
	1		unused
	4	1	length of remaining data in 8-byte units
	4	CARD32	reference-sequence-number
	4	unused	pad
.sp
.PN Die
	1	?	XSMP
	1	8	opcode
	2		unused
	4	0	length of remaining data in 8-byte units
.sp
.PN ShutdownCancelled
	1	?	XSMP
	1	9	opcode
	2		unused
	4	0	length of remaining data in 8-byte units
.sp
.PN ConnectionClosed
	1	?	XSMP
	1	10	opcode
	2		unused
	4	(a)/8	length of remaining data in 8-byte units
	a	LISTofARRAY8	reason
.sp
.PN SetProperties
	1	?	XSMP
	1	11	opcode
	2		unused
	4	1+a/8	length of remaining data in 8-byte units
	4	CARD32	reference-sequence-number
	4		unused
	a	LISTofPROPERTY	properties
.sp
.PN DeleteProperties
	1	?	XSMP
	1	12	opcode
	2		unused
	4	a/8	length of remaining data in 8-byte units
	a	LISTofARRAY8	properties
.sp
.PN GetProperties
	1	?	XSMP
	1	13	opcode
	2		unused
	4	0	length of remaining data in 8-byte units
.sp
.PN GetPropertiesReply
	1	?	XSMP
	1	14	opcode
	2		unused
	4	a/8	length of remaining data in 8-byte units
	a	LISTofPROPERTY	properties
.NH 1
Predefined Properties
.XS
\*(SN Predefined Properties
.XE
.LP
All property values are stored in a LISTofARRAY8. If the type of the property is CARD8,
the value is stored as a LISTofARRAY8 with one ARRAY8 that is one byte long. That single
byte contains the CARD8. If the type of the property is ARRAY8, the value is stored
in the first element of a single element LISTofARRAY8.
.TS H
l l c .
_
.sp 6p
.B
Name	Type	Required?
.sp 6p
_
.sp 6p
.TH
.R
CloneCommand	OS-specific	Yes
CurrentDirectory	OS-specific	No
DiscardCommand	OS-specific	No
Environment	OS-specific	No
OperatingSystemName	ARRAY8	Yes
ProcessID	OS-specific	No
Program	OS-specific	Yes
RestartCommand	OS-specific	Yes
ResignCommand	OS-specific	No
RestartStyleHint	CARD8	No
ShutdownCommand	OS-specific	No
UserID	ARRAY8	Yes
.sp 6p
_
.TE
.IP CloneCommand 3
This is like the 
.PN RestartCommand 
except it restarts a copy of the
application.  The only difference is that the application doesn't
supply its client id at register time.  On POSIX systems this should
be an LISTofARRAY8.
.IP CurrentDirectory 3
On POSIX-based systems specifies the value of the current directory that
needs to be set up prior to starting the Program and should of type
ARRAY8.
.IP DiscardCommand 3
The discard command contains a string that when delivered to the host that 
the client is running on (determined from the connection), will
cause it to discard any information about the current state.  If this command
is not specified, the SM will assume that all of the clients state is encoded
in the 
.PN RestartCommand.
On POSIX systems the type should be LISTofARRAY8.
.IP Environment 3
On POSIX based systems, this will contain an LISTofARRAY8 where
the ARRAY8s alternate between environment variable name and environment
variable value.  
.IP OperatingSystemName 3
This property describes which operating system the client is on in order
to aid the SM in restarting the client.
.NT "Open Issue"
We need a registry of OS names.
The list should include at least POSIX.
.NE
.IP ProcessID 3
This specifies an OS specific identifier for the process.  On POSIX
systems this should contain the return value of getpid() turned into
an ISO 8859-1 (decimal) string.
.IP Program 3
The name of the program that is running.  On POSIX systems this should be 
first parameter passed to execve and should be of type ARRAY8.
.IP RestartCommand 3
The restart command contains a string that when delivered to the
host that the client is running on (determined from the connection),
will cause the client to restart in
its current state.  On POSIX-based systems this is of type LISTofARRAY8
and each of the elements in the array represents an element in
the argv array.
This restart command should ensure that the client restarts with the specified
client-ID.
.IP ResignCommand 3
A client that sets the
.PN RestartStyleHint
to
.PN RestartAnway
uses this property to specify command 
that undoes the effect of the client and removes
any saved state.
.NT Example
A user runs xmodmap. xmodmap registers with the SM, sets 
.PN RestartStyleHint
to 
.PN RestartAnyway,
and then terminates. In order to allow the SM (at the
user's request) to undo this, xmodmap would register a
.PN ResignCommand
that undoes the effects of the xmodmap.
.NE
.IP RestartStyleHint 3
.RS
.LP
If the RestartStyleHint property is present, it will contain the 
style of restarting the client prefers.  If this flag isn't specified,
.PN RestartIfRunning
is assumed.
The possible values are as follows:
.TS H
l n.
_
.sp 6p
.B
Name	Value
.sp 6p
_
.sp 6p
.TH
.R
RestartIfRunning	0
RestartAnyway	1
RestartImmediately	2
.sp 6p
_
.TE
.LP
The
.PN RestartIfRunning
style is used in the usual case.  The client should
be restarted in the next session if it was running at the end of the
current session.
.LP
The
.PN RestartAnyway
style is used to tell the SM that the application
should be restarted in the next session even if it exits before the 
current session is terminated.  
It should be noted that this is only a hint and the SM
will follow the policies specified by its users in determining what applications
to restart.
.LP
.NT Rationale
This can be specified by a client which supports (as Windows clients
do) a means for the user to indicate while exiting that
restarting is desired.  It can also be used for clients that
spawn other clients and then go away, but which want to be
restarted.
.NE
.LP
A client that uses
.PN RestartAnyway
should also set the
.PN ResignCommand
and
.PN ShutdownCommand
properties to commands that undo the state of the client
after it exits.
.LP
The
.PN RestartImmediately
style is like
.PN RestartAnyway ,
but in addition, the
client is meant to run continuously.  If the client exits, the
SM should try to restart it in the current session.
.RE
.IP ShutdownCommand
This command is executed at shutdown time to clean up after a client that
is no longer running but retained its state by setting
.PN RestartStyleHint
to 
.PN RestartAnyway.
The client must not remove any saved state as the client is still part of
the session.
.NT Example
A client is run at start up time that turns on a camera. This client then
exits. At session shutdown, the user wants the camera turned off. This client
would set the 
.PN RestartStyleHint
to 
.PN RestartAnyway
and would register a 
.PN ShutdownCommand
that would turn off the camera.
.NE
.IP UserID 3
Specifies the user's ID.  On POSIX-based systems this
will contain the the user's name (the pw_name field of struct passwd).
.bp
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.tm .pn \n%
.TC
