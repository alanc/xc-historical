.de PT
..
.de BT
..
.de Fe	\" protocol field name
\\fC\\$1\\fP\\$2
..
.de Pa  \" protocol packet type
\\f(CB\\$1\\fP\\$2
..
.de St \" state name
\\fI\\$1\\fP\\$2
..
.ps 12
.nr PS 12
\&
.sp 8
.ce 1
\s+2\fBX Display Manager Control Protocol\fP\s-2
.sp 6
.ce 3
Keith Packard
.sp 6p
Massachusetts Institute of Technology
Laboratory for Computer Science
.sp 8
.LP
Copyright \(co 1989 by the Massachusetts Institute of Technology
.LP
This document is for internal use by Member and Affiliate
organizations of the MIT X Consortium.  It may be redistributed
internally within such organizations, provided the above copyright
notice and this permission notice appear in all copies, but it is not
to be published and it is not to be redistributed externally.  MIT
makes no representations about the suitability for any purpose of the
information in this document.  It is provided "as is" without express
or implied warranty.  The information in this document is a
preliminary draft and therefore subject to change, and does not
represent an approved specification of the MIT X Consortium.
.bp
.AB
.LP
Since the X Display Manager (xdm) may be used to manage remote displays (such
as X terminals), a protocol for requesting service over the network is needed.
Since it should have as little overhead as possible, the XDM Control Protocol
(also called XDMCP) is designed to use unreliable datagrams, a common
format among all packets, and a synchronous request-reply model to handle
sequencing and retransmission.
.AE
.de PT
.ie o .tl 'XDMCP''X Display Manager Control Protocol '
.el .tl 'X Display Manager Control Protocol ''XDMCP'
..
.bp 1
.de BT
.tl ''\fB % \fP''
..
.NH 1
Packet Format
.LP
All XDMCP packets have the following information:
.TS
c c c
c c c
c l l.
Length in	Field	Description of field
Bytes	Type	

2	CARD16	version number
2	CARD16	length of packet in bytes
2	CARD16	opcode
4	CARD32	session id
2	CARD16	length of display number (j)
j	STRING8	display number
2	CARD16	length of auth name (k)
k	STRING8	authorization protocol name
2	CARD16	length of auth data (l)
l	STRING8	authorization protocol data
.TE
.LP
The numeric fields are stored most-significant byte first and string fields
are not padded.  The fields are as follows:
.LP
.Fe "Version Number"
.RS
This specifies the version of XDMCP that generated this packet in
case changes in this protocol are required.  Terminals and display
Managers may choose to support older versions for compatibility.
This field will initially be 1.
.RE
.LP
.Fe "Length of Packet in Bytes"
.RS
This specifies the total length of the packet in octets and should
be equal to the size of the fixed-length header information (16 bytes
in version 1) plus the lengths of the three string fields.  Packets
that have too little or too much data should be ignored.
.RE
.LP
.Fe "Opcode"
.RS
This specifies what step of the protocol this packet represents and
should contain one of the following values (encoding provided at the
end):
.Pa "BroadcastQuery" ,
.Pa "Query" , 
.Pa "Willing" , 
.Pa "Unwilling" ,
.Pa "Request" ,
.Pa "Accept" ,
.Pa "Decline" ,
.Pa "Manage" ,
.Pa "Refuse" .
.RE
.LP
.Fe "Session ID"
.RS
This is used in the Accept, Manage, and Refuse packets as a check
code for distinguishing this session from others in case of
retransmissions.
.RE
.LP
.Fe "Length of Display Name in Bytes"
.RS
This specifies the length of the display name field.
.RE
.LP
.Fe "Display Name"
.RS
This specifies the name of display as would be used with -display
command line arguments or the DISPLAY environment variable (under
UNIX).  Terminals should always set this field; Display Managers
should simply copy the field from the packet to which they are
responding.
.RE
.LP
.Fe "Length of Authorization Protocol Name"
.RS
This specifies the length of the authorization protocol name field.
.RE
.LP
.Fe "Authorization Protocol Name"
.RS
This field identifies the protocol to be used in decoding the
information in the Authorization Protocol Data field.  It is usually
a string and its interpretation is up to the recipient of the packet.
A registry of Protocol Names will be needed.
.RE
.LP
.Fe "Length of Authorization Protocol Data"
.RS
This field contains arbitrary data for use in authorizing a given
connection.  The interpretation of this information is determined
by the name of the protocol being used.  Secure protocols (such as
Kerberos) may use encryption to provide mutual verification between
the user and the Display Manager.  Unsecure protocols (such as
MIT-MAGIC-COOKIE-1) may simply rely on a piece of shared information.
.RE
.LP	
Packets should be checked to make sure that they satisfy the following
conditions:
.IP 1
They must contain valid opcodes.
.IP 2
The total length of the packet should equal the size of the fixed
information (16 bytes) plus the lengths of the
.Fe "Display name" , 
.Fe "auth name"
and
.Fe "auth data" .
.IP 3
The authorization protocol name should be recognized.  A zero-length
.Fe "auth name"
means that no authorization information is provided.  Recipients
may choose whether or not they will accept such packets.
.IP 4
The
.Fe "opcode"
should be expected (a finite state diagram is given
in a later section).
.IP 5
If the packet is of type
.Pa "Manage"
or
.Pa "Refuse" ,
the
.Fe "Session id"
should
match the value sent in the preceeding
.Pa "Accept"
packet.
.NH 1
Protocol
.LP
Each of the opcodes is described below.  Since a given packet type is only ever
generated by the Terminal or by the Display Manager, each of the opcodes
indicates the direction in which it is sent.
.LP
.Pa "BroadcastQuery"
.br
.Pa "Query"
.RS
Terminal \(-> Manager
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field is not used and should be set to zero (0).
.RE
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields may be used by the Display Manager to verify the
identity of the Terminal.  This would be used in environments
that wish to control access to a given terminal or system.
This could be done by sending a message that was encrypted with
the Display Manager's private key.
.RE
.RE
Semantics:
.RS
A
.Pa "Query"
packet is sent from the Terminal to a specific host
to ask if that host is willing to provide xdm services to this
terminal.  The host should respond with
.Pa "Willing"
if it is willing
to service the Terminal or 
.Pa "Unwilling"
if it is not.
.br
A
.Pa "BroadcastQuery"
packet is similar to the
.Pa "Query"
packet except
that it is sent to all hosts on the network.  However, unlike
.Pa "Query"
requests, hosts that are not willing to service the Terminal
should simply ignore
.Pa "BroadcastQuery"
requests.
.RE
Valid Responses:
.RS
.Pa Willing ,
.Pa Unwilling
.RE
Problems/Solutions:
.RS
Problem:
.RS
Not all managers receive the
.Pa "BroadcastQuery" .
.RE
.RS
Indication:
.RS
none if
.Pa "BroadcastQuery"
was broadcast,
else failure to receive
.Pa "Willing" .
.RE
Solution:
.RS
repeatedly send
.Pa "BroadcastQuery"
while waiting
for user to choose a host.
.RE
.RE
.RE
.RE
.LP
.Pa "Willing"
.RS
Manager \(-> Terminal
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field is not used and should be set to zero (0).
.RE
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields may be used by the Terminal to verify that the
Display Manager was able to decode the authorization
information sent in the
.Pa Query
or
.Pa BroadcastQuery
packet.  This might be done by returning the previous auth data permuted in
some known way (such as by adding 1 and reencrypting with the Terminal's key
which it could have been sent encrypted in the
.Pa Query ).
.RE
.RE
Semantics:
.RS
A
.Pa Willing
packet is sent by Display Managers that may service 
connections from this Terminal.  It is sent in response to either
a
.Pa Query
or a
.Pa BroadcastQuery
but does not imply a commitment to 
provide service (it may later decide that it as accepted enough
connections already).
.RE
Problems/Solutions:
.RS
Problem:
.RS
.Pa Willing
not received by the terminal.
.br
Indication:
.RS
none if
.Pa Query
was broadcast, else failure to
receive
.Pa Willing .
.RE
Solution:
.RS
The terminal should continue to send
.Pa Query
messages until a response is received.
.RE
.RE
.RE
.RE
.LP
.Pa Unwilling
.RS
Manager \(-> Terminal
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field is not used and should be set to zero (0).
.RE
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields should be set in the same manner as those in
a
.Pa Willing
packet.
.RE
.RE
Semantics:
.RS
An
.Pa Unwilling
packet is sent by Display Managers in response to
direct
.Pa Query
requests (as opposed to general
.Pa BroadcastQuery
requests) if the Display Manager will not accept requests for management.
This is typically sent by Display Managers that wish to only service
particular Terminals or which handle a limited number of Terminals at once.
.RE
Problems/Solutions:
.RS
Problem:
.RS
.Pa Unwilling
not received by the terminal.
.br
Indication:
.RS
Terminal fails to receive
.Pa Unwilling .
.RE
Solution:
.RS
The terminal should continue to
send
.Pa Query
messages until a response is received.
.RE
.RE
.RE
.RE
.LP
.Pa Request
.RS
Terminal \(-> Manager
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field is not used and should be set to zero (0).
.RE
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields should be set in the same manner as those in a
.Pa Query
packet.
.RE
.RE
Semantics:
.RS
A
.Pa Request
packet is sent by a Terminal to a specific host to
request a session id in preparation for a establishing a 
connection.  If the Display Manager is willing to service a 
connection to this Terminal, it should return an
.Pa Accept
packet 
with a valid session id and should be ready for subsequent
Manage requests.  Otherwise, it should return a
.Pa Decline
packet.
.RE
Valid Responses:
.RS
.Pa Accept ,
.Pa Decline
.RE
Problems/Solutions:
.RS
Problem:
.RS
Request not received by manager.
.br
Indication:
.RS
Terminal timeout waiting for response.
.RE
Solution:
.RS
Terminal resends
.Pa Request
message.
.RE
.RE
Problem:
.RS
Message received out of order by manager.
.br
Indication:
.RS
none
.RE
Solution:
.RS
Each time a
.Pa Request
is sent, the manager sends the
.Fe "Session ID"
associated
with the next session in the
.Pa Acknowledge .
If that next session is not yet started, the manager will simply resend with
the same
.Fe "Session ID" .
If the session is in progress, the manager will reply with a new
.Fe "Session ID" ;
in case, the
.Pa Acknowledge
will be discarded by the terminal.
.RE
.RE
.RE
.RE
.LP
.Pa Accept
.RS
Manager \(-> Terminal
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field should contain a non-zero number that is to be used
as an identifier for this session.  The Display Manager should
make an attempt to not reuse identifiers in a repeatable way.
.RE
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields should be set in response to the
.Pa Request
packet
in the same manner the fields are set in a
.Pa Willing
packet.
.RE
.RE
Semantics:
.RS
An
.Pa Accept
packet is sent by a Display Manager in response to a 
.Pa Request
packet if the Display Manager is willing to establish a
connection for the Terminal.  The
.Fe "Session ID"
is used to identify
this connection from any preceeding ones and will be used by the
Terminal in its subsequent
.Pa Manage
packet.
.RE
Problems/Solutions:
.RS
Problem:
.RS
.Pa Accept 
or
.Pa Decline
not received by terminal.
.br
Indication:
.RS
Terminal timeout waiting for response to
.Pa Request .
.RE
Solution:
.RS
Terminal resends
.Pa Request
message.
.RE
.RE
Problem:
.RS
Message received out of order by terminal.
.br
Indication:
.RS
Terminal receives
.Pa Accept
after 
.Pa Manage
has been sent.
.RE
Solution:
.RS
Terminal discards
.Pa Accept
messages after
it has sent a
.Pa Manage
message.
.RE
.RE
.RE
.RE
.RE
.LP
Decline
.RS
Manager \(-> Terminal
.br
Fields:
.RS
Session ID
.RS
This field is not used and should be set to zero (0).
.RE
Auth Name
.br
Auth Data
.RS
These fields should be set in the same manner as those in
an Accept packet.
.RE
.RE
Semantics:
.RS
A Decline packet is sent by a Display Manager in response to a
Request packet if the Display Manager is unwilling to establish
a connection for the Terminal.  This is allowed even if the
Display Manager had responded Willing to a previous Query (as 
might be the case if the Display Managers had a limit to the 
number of Terminals it would serve).
.RE
Problems/Solutions:
.RS
same as for Accept.
.RE
.RE
.LP
Manage
.RS
Terminal \(-> Manager
.br
Fields:
.RS
Session ID
.RS
This field should contain the non-zero session id returned
in the Accept packet.
.RE
Auth Name
.br
Auth Data
.RS
These fields should contain information that the Display
Manager will need to open the initial X connection.  Secure
protocols may use encryption as described for the
.Pa Query
packet.
.RE
.RE
Semantics:
.RS
A
.Pa Manage
packet is sent by a Terminal to ask the Display Manager 
to begin a session on the Terminal.  If the
.Fe "Session ID"
is correct
the Display Manager should open a X connection, otherwise it 
should respond with a
.Pa Refuse
packet.
.RE
Valid Responses:
.RS
X connection with correct auth info,
.Pa Refuse
.RE
Problems/Solutions:
.RS
Problem:
.RS
.Pa Manage
not received by manager.
.br
Indication:
.RS
Terminal timeout waiting for response.
.RE
Solution:
.RS
Terminal resends
.Pa Manage
message.
.RE
.RE
Problem:
.RS
.Pa Manage
received out of order by manager.
.br
Indication:
.RS
session already in progress.
.RE
Solution:
.RS
.Pa Refuse
message is sent.
.RE
Indication:
.RS
.Fe "Session ID"
doesn't match next
.Fe "Session ID"
.RE
Solution:
.RS
.Pa Refuse
message is sent.
.RE
.RE
.RE
.RE
.LP
.Pa Refuse
.RS
Manager \(-> Terminal
.br
Fields:
.RS
.Fe "Session ID"
.RS
This field should be set the invalid
.Fe "Session ID"
received in the
.Pa Manage
packet.
.RE
.RS
.Fe "Auth Name"
.br
.Fe "Auth Data"
.RS
These fields should be set to the authorization information
provided in the
.Pa Manage
packet.
.RE
.RE
Semantics:
.RS
A
.Pa Refuse
packet is sent by a Display Manager in response to 
problems establishing a connection.  The Terminal should assume
that it received an old
.Pa Accept
packet and should resend its
.Pa Request
packet.
.RE
Problems/Solutions:
.RS
Problem:
.RS
Error message is lost.
.br
Indication:
.RS
terminal times out waiting for OpenDisplay/Error.
.RE
Solution:
.RS
terminal resends
.Pa Manage
message.
.RE
.RE
.RE
.RE
.RE
.NH 1
Session Termination
.LP
When the session is over, the initial connection with the display (the
one which ack's the manage packet) will be closed.  At this point,
all other display connections should be closed and the terminal
can request another session.
.NH 1
State Diagrams
.LP
These state diagrams are designed to cover all actions of both
the terminal and the manager.
.LP
Terminal:
.RS
.LP

.St start :
.RS
user-requested connect to some host \(-> 
.St broadcast

.br
user-requested connect to one host \(-> 
.St query

.RE

.St broadcast :
.RS
Send
.Pa BroadcastQuery
packet
.br
\(-> 
.St collect-broadcast-query

.RE
.LP

.St collect-broadcast :
.RS
receive
.Pa Willing
\(-> 
.St update-broadcast-willing

.br
user-requested connect to one host \(-> 
.St start-connection

.br
receive other \(-> 
.St collect-broadcast

.br
timeout \(-> 
.St broadcast

.RE
.LP

.St update-broadcast-willing :
.RS
Add new host to host-list.
.br
Display host-list.
.br
\(-> 
.St collect-broadcast

.RE
.LP

.St query :
.RS
Send
.Pa Query
packet
.br
\(-> 
.St collect-query

.RE
.LP

.St collect-query :
.RS
receive
.Pa Willing
\(-> 
.St start-connection

.br
receive Unwilling \(-> 
.St stop-connection

.br
receive other \(-> 
.St collect-query

.br
timeout \(-> 
.St query

.RE	
.LP

.St start-connection :
.RS
Send
.Pa Request
packet
.br
\(-> 
.St await-request-response

.RE
.LP

.St await-request-response :
.RS
receive
.Pa Accept
\(-> 
.St manage

.br
receive
.Pa Decline
\(-> 
.St stop-connection

.br
timeout \(-> 
.St start-connection

.RE
.LP

.St manage :
.RS
Save
.Fe "Session ID"
.br
Close all display connections
.br
Send
.Pa Manage
packet with
.Fe "Session ID"
.br
\(-> 
.St await-manage-response

.RE	
.LP

.St await-manage-response :
.RS
receive XOpenDisplay: \(-> 
.St run-session

.br
receive
.Pa Refuse
with matching
.Fe "Session ID" :
\(-> 
.St start-connection

.br
timeout \(-> 
.St manage

.RE
.LP

.St stop-connection :
.RS
Display cause of termination
.br
\(-> 
.St start

.RE	

.St run-session :
.RS
await close of first display connection
.br
\(-> 
.St reset-terminal

.RE	
.LP

.St reset-terminal :
.RS
close all display connections
.br
\(-> 
.St start

.RE
.RE
.LP
Manager:
.RS
.LP

.St idle :
.RS
receive
.Pa BroadcaseQuery
\(-> 
.St broadcast-respond

.br
receive
.Pa Query
\(-> 
.St query-respond

.br
receive
.Pa Request
\(-> 
.St request-respond

.br
receive
.Pa Manage
\(-> 
.St manage

.br
\(-> 
.St idle

.RE	
.LP

.St broadcast-respond :
.RS
if willing to manage \(-> 
.St send-willing

.br
\(-> 
.St idle

.RE
.LP

.St query-respond :
.RS
if willing to manage \(-> 
.St send-willing

.br
\(-> 
.St send-unwilling

.RE
.LP

.St send-willing :
.RS
Send
.Pa Willing
packet
.br
\(-> 
.St idle

.RE
.LP

.St send-unwilling :
.RS
Send
.Pa Unwilling
packet
.br
\(-> 
.St idle

.RE
.Lp

.St request-respond :
.RS
if acceptable \(->
.St manage
\(-> 
.St accept-session

.br
\(-> 
.St decline-session

.RE
.LP

.St accept-session :
.RS
Generate
.Fe "Session ID" .
Save 
.Fe "Session ID" "/display name"
somewhere
.br
Send
.Pa Accept
packet
.br
\(-> 
.St idle

.RE		
.LP

.St decline-session :
.RS
Send
.Pa Decline
packet
.br
\(-> 
.St idle

.RE	
.LP

.St manage :
.RS
If
.Fe "Session ID"
matches saved
.Fe "Session ID
\(-> 
.St run-session

.br
\(-> 
.St refuse

.RE	
.LP

.St refuse :
.RS
Send
.Pa Refuse
packet
.br
\(-> 
.St idle

.RE	
.LP

.St run-session :
.RS
Terminate any session in progress
.br
open display succeeds \(-> 
.St session

.br
\(-> 
.St refuse

.RE	
.LP

.St session :
.RS
Start a new session
.br
XCloseDisplay
.br
\(-> 
.St idle

.RE
.RE
.NH 1
Constraits on the protocol
.LP
Any message sent must be acknowledged in order to guarantee receipt.  This
is satisfied in the Manage case by receipt of the XOpenDisplay command.
Sequence problems are solved by insisting that the
.Fe "Session ID"
sent in the
.Pa Acknowledge
message is returned with the 
.Pa Manage
message.  While waiting for a user to choose the desired host, making the
Terminal repeatedly send out
.Pa Query
messages will cause any machines which suddenly start/stop responding to be
reflected in the menu.  In this way, the user can wait for a machine to come
online before choosing a session.
.LP
Terminals may be turned off at random.  Any existing session
running on a terminal which has been turned off must be
identifiable.  This is made possible by the sequence
.Pa Request/Acknowledge/Manage .
If this sequence is sent, any
existing session is terminated immediately and a new session
started.  There is the problem (at least with TCP) that
connections may not be closed when the terminal is turned
off.  The manager could ameliorate this for local connections
by periodically XSync'ing on its own connection, and terminating
the session if its own connection ever closes.
.LP
Terminals should not be required to retain permanent state for
purposes of the control protocol.  Once solution to sequencing
errors would be to use monotonically-increasing message identifiers
to allow both sides to ignore messages which arrive out-of-sequence.
For this to work, terminals would at a minimum have to increment
a stable "crash count" each time they are powered on, and use that
number as part of a larger sequence number.  But if terminals
cannot retain permanent state this cannot work.  Instead, the
manager assumes the responsibility for permanent state, when
generating session IDs.
.LP
The Manager must not be responsible for packet reception.  To
prevent the Manager from becoming stuck because of a hostile
Terminal, no portion of the protocol requires the Manager to retry
transmission.  Part of this means that any valid packet which the
Manager does receive *must* be acknowledged in some way, to prevent
the Terminal from continuously resending packets.  The Terminal
can keep the protocol running as it will always know when the
Manager has received (at least one copy of) a packet.  On the
Manager side, this means that any packet may be received
more than once (if the response was lost) and duplicates must
be ignored.
