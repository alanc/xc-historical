.\" Use tbl and -ms
.\" Disable hyphenation.  I hate it.
.hy 0
.de hy
..
.\" A couple of macros to standardize state diagrams and make them
.\" easy to type.
.de s	\" Begin state - .s <state name>
\f6\\$1\fP:
.br
..
.de t	\" Transition - .t "condition" <new state>
.RS
\\$1 \(-> \f6\\$2\fP
.RE
..
.\" Turn off page header & footer?
.de PT
..
.de BT
..
.\" For some reason, my groff doesn't like I.  I've not tried to
.\" figure out why yet.
.ie t \{
.fp 6 I
.fp 7 C
.fp 8 CB \}
.el \{
.fp 6 B
.fp 7 R
.fp 8 B
\}
.ps 10
.nr PS 10
\&
.TL
\s+2\fBInter-Client Exchange (ICE) Protocol\fP\s-2
.sp
Preliminary Draft 0.9b
.sp
X Consortium Standard
.sp
X Version 11, Release 6
.AU
Robert Scheifler
.AI
X Consortium
.AU
Jordan Brown
.AI
Quarterdeck Office Systems
.AB
.LP
There are numerous possible "inter-client" protocols, with many similarities
and common needs - authentication, extension, version negotiation, byte
order negotiation, etc.  The ICE protocol is intended to provide a framework
for building such protocols, allowing them to make use of common negotiation
mechanisms and to be multiplexed over a single transport connection.
.AE
.LP
.DS C
.if n Copyright (c) 1993 by the Massachusetts Institute of Technology
.if t Copyright \(co 1993 by the Massachusetts Institute of Technology
.DE
.sp 3
.LP
Permission to use, copy, modify, and distribute this documentation for
any purpose and without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.
MIT makes no representations about the suitability for any purpose
of the information in this document.  This documentation is provided
"as is" without express or implied warranty.
.de PT
.ie o .tl 'ICE''Inter-Client Exchange Protocol '
.el .tl 'Inter-Client Exchange Protocol ''ICE'
..
.bp 1
.de BT
.tl ''\fB % \fP''
..
.NH 1
Purpose and Goals
.LP
In discussing a variety of protocols - existing, under development, and
hypothetical - it was noted that there are many common elements between
the protocols and that there were cases where the same two parties
needed to talk multiple protocols.  For instance, most protocols have
needs for authentication and for version and extension negotiation.
An inset relationship is likely to require session management, data
transfer, focus negotiation, and command notification.
.LP
ICE provides a generic framework for building protocols, providing
a common mechanism for authentication, version negotiation, multiplexing,
etc, and a common library interface supplying buffering and similar
services.
.NH 1
Overview of the protocol
.LP
ICE is designed to allow a variety of "subprotocols" to use common
mechanisms and common protocol design for the negotiations, buffering,
and so on that most protocols need.  It is designed to allow several
subprotocols to coexist on the same underlying transport connection, to
help reduce the number of transport connections needed in a system.
.LP
Through some mechanism outside ICE, two parties agree that they
desire to use ICE to communicate.  (This negotiation might take
place over some other protocol, such as the X selection protocol, or
might be a more static mechanism, such as an environment variable.)
The parties agree, again using a mechanism outside ICE, on which
party will be the "originating" party and which party will be the
"answering" party.  (Note that this is independent of the intended
direction of data flow.) The originating party first determines
whether or not there is already an existing ICE connection between
the parties.  If there is, it uses the existing connection and simply
negotiates the new subprotocol.  If there is not, it makes a stream
connection to the other party, and exchanges the initial ICE connection
setup messages.
.LP
The initial ICE connection setup dialog consists of three major
parts:  byte order exchange, authentication, and connection information
exchange.  The first message in each direction is a \fBByteOrder\fP
message telling which byte order will be used by the sending party in
messages that it sends.  After that, the originating party sends
a \fBConnectionSetup\fP message giving information about itself
(vendor name and release number) and giving a list of ICE version
numbers it is capable of supporting and a list of authentication
schemes it is willing to accept.  The answering party (optionally, if
\fBmust-authenticate\fP is false) picks
an authentication scheme and returns an \fBAuthenticationRequired\fP
message specifying the authentication scheme to be used and giving
whatever data is appropriate to the authentication scheme chosen.  The
parties exchange \fBAuthenticationReply\fP and
\fBAuthenticationNextPhase\fP messages until the authentication dialog
is complete, at which point the answering party sends a
\fBConnectionReply\fP message giving information about itself and
choosing which version of the ICE protocol will be used.
.LP
Once an ICE connection is established, the originating party sends a
\fBProtocolSetup\fP message to start a subprotocol negotiation.  (Note
that the originating party for a given subprotocol may or may not have
been the party that originally initiates the ICE connection.)  The
\fBProtocolSetup\fP message gives the name of the subprotocol that the
parties are to use, and the ICE major opcode that the originating party
will use for messages that it sends using that subprotocol.  The
\fBProtocolSetup\fP negotiation is quite similar to the
\fBConnectionSetup\fP negotiation, with a similar authentication
negotiation.  After the authentication dialog, the answering party
sends a \fBProtocolReply\fP giving the ICE major opcode that it will
use for messages that it sends using that subprotocol.
.LP
The parties then exchange messages as appropriate to the subprotocol.
ICE supplies a small amount of "framework" for the subprotocol - in
particular, it defines a field for a minor opcode and a standard format
for error messages.
.LP
At some point the parties agree (in a manner specific to the
subprotocol) that they are finished with the subprotocol.
When no subprotocols are in use, either party sends a \fBWantToClose\fP
message and, if both parties concur, the connection is closed.
(Actually, due to the parallel and independent nature of the
negotiation, it's quite likely that both parties will send \fBWantToClose\fP
messages simultaneously.  This is taken to imply that they are both
willing to close the connection.)
.NH 1
Data Types
.LP
ICE packets contain several types of data.  Byte order is negotiated in
the initial connection messages; in general data is sent in the sender's
byte order and the receiver is required to swap it appropriately.
In order to support 64-bit machines, ICE messages and many ICE data types
are padded to multiples of 8 bytes.  All messages are designed so that
fields are "naturally" aligned on 16-, 32-, and 64-bit boundaries.
.sp
.RS
.nf
pad(E, b) = (b - (E mod b)) mod b
(Pad value E to a multiple of b bytes)
.fi
.RE
.NH 2
Primitive Types
.LP
.TS
l c l
l c l

l c l.
Type Name	Length	Description
	(in bytes)

CARD8	1	A single byte unsigned integer
CARD16	2	Two byte unsigned integer
CARD32	4	Four byte unsigned integer
.TE
.NH 2
Complex Types
.LP
.TS
l c l l
l c l l

l c l l.
Type Name	Length	Type	Description
	(in bytes)

VERSION
	2	CARD16	Major version number
	2	CARD16	Minor version number

XPCS
	2	CARD16	length of string in bytes
	n	LISTofCHAR	string
	p		unused, p = pad(n+2, 4)
.TE
.NH 1
Packet Format
.LP
All ICE packets have the following information:
.TS
c c l
c c l

c l l.
Length	Field	Description
(in bytes)	Type	

1	CARD8	protocol major opcode
1	CARD8	protocol minor opcode
2	<vary>	<use is specific to protocol>
4	CARD32	length of remaining data in 8-byte units

n	???	packet-specific data
.TE
.LP
The fields are as follows:
.LP
Protocol major opcode
.RS
This specifies what subprotocol the message is intended for.  Subprotocol
0 is reserved for ICE control messages.  The major opcodes of other
subprotocols are dynamically assigned and exchanged at protocol
negotiation time.
.RE
.LP
Protocol minor opcode
.RS
This specifies what protocol-specific operation is to be performed.
Minor opcode 0 is reserved for Errors; other values are protocol-specific.
.RE
.LP
Length of data in 8-byte units
.RS
This specifies the length of the information following the first 8 bytes.
Each message-type has a different format, and will need to be separately
length-checked against this value.  As every data item has either an
explicit length, or an implicit length, this can be easily accomplished.
Packets that have too little or too much data indicate a serious
protocol failure.
.RE
.NH 1
Global Protocol Description
.LP
Every message sent (in a given direction) has an implicit sequence number,
starting with 1.  Sequence numbers are global to the connection; independent
sequence numbers are NOT maintained for each protocol.
.LP
Messages of a given major-opcode (i.e., of a given protocol) must be
responded to (if a response is called for) in order by the receiving party.
Messages from different protocols can be responded to in arbitrary order.
.LP
Minor opcode 0 in every protocol is for reporting errors.  At most one error
is generated per request.  If more than one error condition is encountered
in processing a request, the choice of which error is returned is
implementation-dependent.  Protocol designers are urged to design their
protocol to avoid this situation.
.NH 2
Error
.RS
.TS
lf7 l.
offending-minor-opcode	CARD8
severity	{CanContinue, FatalToProtocol, FatalToConnection}
sequence-number	CARD32
class	CARD16
value(s)	<dependent on major/minor opcode and class>
.TE
.LP
An error message sent in response to a message from any protocol.
This error message exists in all protocol major-opcode spaces; it
is minor-opcode zero in every protocol.  The minor opcode of the
message that caused the error is reported, as well as the sequence
number of that message.  The class defines the generic class of
error.  Classes are specified separately for each protocol (numeric
values can mean different things in different protocols).  The error
value(s), if any, and their types vary with the specific error class
for the protocol.

(Asynchronous errors - errors not associated with a previous
message???  If so, offending-minor and sequence = 0.)
.RE
.NH 1
ICE Control Subprotocol - Major Opcode 0
.LP
Each of the ICE control opcodes is described below.
Most of the packets have additional information included beyond the
description above.  The additional information is appended to the packet
header in the order described, and the length field is computed accordingly.
.LP
Note:  In the following message descriptions, "Expected Errors" indicates
errors that might occur in the normal course of events.  Other errors
(in particular BadMajor, BadMinor, BadState, BadLength, BadValue,
ProtocolDuplicate, and MajorOpcodeDuplicate) might occur, but generally
indicate serious failures (read as "bugs") on the part of the erring peer.
.NH 2
ByteOrder
.RS
.TS
lf7 l.
byte-order	{BigEndian, LittleEndian}
.TE
.LP
Both parties must send this message before sending any other,
including errors.  This message specifies the byte order that
will be used on subsequent messages sent by this party.
.LP
Note:  If the receiver detects an error in this message,
it must be sure to send its own ByteOrder message before sending
the Error.
.RE
.NH 2
ConnectionSetup
.RS
.TS
lf7 l.
must-authenticate	BOOL
versions	LISTofVERSION
authentication-protocol-names	LISTofXPCS
vendor	XPCS
release	XPCS
.TE
Expected errors: \f7NoVersion\fP, \f7NoAuthentication\fP
.LP
The party that initiates the connection (that one that does the "connect()")
must send this as the second message (after ByteOrder) on startup.
.LP
Versions gives a list, in decreasing order of preference, of the
protocol versions this party is capable of speaking.  This document
specifies major version 1, minor version 0.
.LP
Authentication-protocol-names gives the names of authentication
protocols the sending party is willing to perform.
.LP
If must-authenticate is true, the initiating party demands
authentication; the accepting party *must* pick an authentication
scheme and use it.  If must-authenticate is false, the accepting
party is allowed to decline authentication completely.
.LP
Vendor gives the name of the vendor of this ICE implementation.
.LP
Release gives the release identifier of this ICE implementation.
.LP
.RE
.NH 2
AuthenticationRequired
.RS
.TS
lf7 l.
authentication-protocol-index	CARD8
data	<specific to authentication protocol>
.TE
Expected errors: \f7AuthenticationRejected\fP, \f7AuthenticationFailed\fP
.LP
This message is sent in response to a ConnectionSetup or ProtocolSetup
message to specify that authentication is to be done and what authentication
mechanism is to be used.
.LP
The authentication protocol is specified by an index into the list
of names given in the ConnectionSetup or ProtocolSetup.  Any
protocol-specific data that might be required is also sent.
.RE
.NH 2
AuthenticationReply
.RS
.TS
lf7 l.
data	<specific to authentication protocol>
.TE
Expected errors:  \f7AuthenticationRejected\fP, \f7AuthenticationFailed\fP
.LP
This message is sent in response to an AuthenticationRequired message, to
supply authentication data as defined by the authentication protocol
being used.
.LP
Note that this message is sent by the party that initiated the current
negotiation - the party that sent the ConnectionSetup or ProtocolSetup
message.
.RE
.NH 2
AuthenticationNextPhase
.RS
.TS
lf7 l.
data	<specific to authentication protocol>
.TE
Expected errors:  \f7AuthenticationRejected\fP, \f7AuthenticationFailed\fP
.LP
This message is sent in response to an AuthenticationReply message, to
supply authentication data as defined by the authentication protocol
being used.
.RE
.NH 2
ConnectionReply
.RS
.TS
lf7 l.
version-index	CARD8
vendor	XPCS
release	XPCS
.TE
Version-index gives an index into the list of versions offered in
the ConnectionSetup message; it specifies the version of the ICE
protocol that both party should speak for the duration of the
connection.
.LP
Vendor gives the name of the vendor of this ICE implementation.
.LP
Release gives the release identifier of this ICE implementation.
.RE
.NH 2
ProtocolSetup
.RS
.TS
lf7 l.
protocol-name	XPCS
major-opcode	CARD8
versions	LISTofVERSION
vendor	XPCS
release	XPCS
authentication-protocol-names	LISTofXPCS
.TE
Expected Errors:  \f7UnknownProtocol\fP, \f7NoVersion\fP, \f7NoAuthentication\fP
.LP
Used to negotiate a protocol and establish any authentication
specific to it.
.LP
Protocol-name gives the name of the protocol the party wishes
to speak.
.LP
Major-opcode gives the opcode that the party will use in messages
it sends.
.LP
Versions gives a list of version numbers, in decreasing order of
preference, that the party is willing to speak.
.LP
Vendor identifies the vendor of this implementation of the specific
protocol.
.LP
Release identifies the release of this implementation of the
specific protocol.
.LP
Authentication-protocol-names  specifies a (possibly null) list
of authentication protocols the party is willing to perform.
.RE
.NH 2
ProtocolReply
.RS
.TS
lf7 l.
major-opcode	CARD8
version-index	CARD8
vendor	XPCS
release	XPCS
.TE
Major-opcode gives the opcode that this party will use in
messages that it sends.  Version-index gives an index into the list
of versions offered in the ProtocolSetup message; it specifies
the version of the protocol that both party should speak for
the duration of the connection.  Vendor identifies the vendor of
this implementation of the specific protocol.  Release identifies
the release of this implementation of the specific protocol.
.RE
.NH 2
Ping
.RS
.TS
l.
<no parameters>
.TE
A way to test if the other side is still paying attention.
.RE
.NH 2
PingReply
.RS
.TS
l.
<no parameters>
.TE
The way to say that you are still paying attention.
.RE
.NH 2
WantToClose
.RS
.TS
l.
<no parameters>
.TE
The sending party has noticed that there are no active
protocols left and the connection can be closed.  There are
four possible scenarios arising from this request:
.br
(1) The other side noticed too, and has already sent a
WantToClose.  On receiving a WantToClose while already attempting
to shut down, each party should simply close the connection.
.br
(2) The other side hasn't noticed, but agrees.  It closes
the connection.
.br
(3) The other side has a ProtocolSetup "in flight", in
which case it is to ignore WantToClose and the party
sending WantToClose is to abandon the shutdown attempt
when it receives the ProtocolSetup.
.br
(4) The other side wants the connection kept open for some
reason not specified by the protocol, in which case it
sends NoClose.
.LP
See the state transition diagram for additional information.
.RE
.NH 2
NoClose
.RS
.TS
l.
<no parameters>
.TE
The sending party does not want the connection closed at
this time.  The receiving party should not close the
connection, and should not request WantToClose; the
sending party will initiate the next "Shall we close?"
dialog.
.RE
.NH 1
State Diagrams
.LP
Here are the state diagrams for the party that initiates the connection:
.LP
.s start
.t "connect to other end, send ConnectionSetup" conn_wait

.s conn_wait
.t "receive ConnectionReply" stasis
.t "receive AuthenticationRequired" conn_auth1
.t "receive Error" quit
.t "receive <other>, send Error" quit

.s conn_auth1
.t "if good auth data, send AuthenticationReply" conn_auth2
.t "if bad auth data, send Error" quit

.s conn_auth2
.t "receive ConnectionReply" stasis
.t "receive AuthenticationNextPhase" conn_auth1
.t "receive Error" quit
.t "receive <other>, send Error" quit

.LP
Here are top-level state transitions for the party that accepts connections.

.s listener
.t "accept connection" init_wait

.s init_wait
.t "receive ConnectionSetup" auth_ask
.t "receive <other>, send Error" quit

.s auth_ask
.t "send ConnectionReply" stasis
.t "send AuthenticationRequired" auth_wait
.t "send Error" quit

.s auth_wait
.t "receive AuthenticationReply" auth_check
.t "receive <other>, send Error" quit

.s auth_check
.t "if no more auth needed, send ConnectionReply" stasis
.t "if good auth data, send AuthenticationNextPhase" auth_wait
.t "if bad auth data, send Error" quit

.LP
Here are the top-level state transitions for all parties after the initial
connection establishment subprotocol.
.LP
Note:  this is not quite the truth for branches out from stasis, in
that multiple conversations can be interleaved on the connection.

.s stasis
.t "send ProtocolSetup" proto_wait
.t "receive ProtocolSetup" proto_reply
.t "send Ping" ping_wait
.t "receive Ping, send PingReply" stasis
.t "receive WantToClose" shutdown_attempt
.t "receive <other>, send Error" stasis
.t "All protocols shut down, send WantToClose" close_wait

.s proto_wait
.t "receive ProtocolReply" stasis
.t "receive AuthenticationRequired" give_auth1
.t "receive Error, give up on this protocol" stasis

.s give_auth1
.t "if good auth data, send AuthenticationReply" give_auth2
.t "if bad auth data, send Error, give up on this protocol" stasis

.s give_auth2
.t "receive ProtocolReply" stasis
.t "receive AuthenticationNextPhase" give_auth1
.t "receive Error, give up on this protocol" stasis

.s proto_reply
.t "send ProtocolReply" stasis
.t "send AuthenticationRequired" take_auth1
.t "send Error, give up on this protocol" stasis

.s take_auth1
.t "receive AuthenticationReply" take_auth2
.t "receive Error, give up on this protocol" stasis

.s take_auth2
.t "if good auth data" take_auth3
.t "if bad auth data, send Error, give up on this protocol" stasis

.s take_auth3
.t "if no more auth needed, send ProtocolReply" stasis
.t "if good auth data, send AuthenticationNextPhase" take_auth1
.t "if bad auth data, send Error, give up on this protocol" stasis

.s ping_wait
.t "receive PingReply" stasis

.s quit
.t "" close connection


Here are the state transitions for shutting down the connection:

.s shutdown_attempt
.t "if protocols active," stasis
.t "else if want to stay alive anyway, send NoClose" stasis
.t "else" quit

.s close_wait
.t "receive ProtocolSetup" proto_reply
.t "receive NoClose" stasis
.t "receive WantToClose" quit
.t "connection close" quit

.NH 1
Protocol Encoding
.LP
In the encodings below, the first column is the number of bytes occupied.
The second column is either the type (if the value is variable) or the
actual value.  The third column is the description of the value (e.g.,
the parameter name).  Receivers are not to examine unused and pad bytes.
.LP
This document describes major version 1, minor version 0 of the ICE protocol.
.LP
LISTof<type> indicates some number of repetitions of <type>, with no
additional padding.  The number of repetitions must be specified elsewhere
in the message.
.RS
.TS
c c
l l.
Message Name	Encoding
_
Error	0
ByteOrder	1
ConnectionSetup	2
AuthenticationRequired	3
AuthenticationReply	4
AuthenticationNextPhase	5
ConnectionReply	6
ProtocolSetup	7
ProtocolReply	8
Ping	9
PingReply	10
WantToClose	11
NoClose	12
.TE
.RE
.NH 2
Error
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|CARD8|major-opcode
1|0|Error
2|CARD16|class
4|(n+p)/8+1|length
1|CARD8|offending-minor-opcode
1||severity:
|0|   CanContinue
|1|   FatalToProtocol (???)
|2|   FatalToConnection
2||unused
4|CARD32|sequence number of erroneous message
n|<varies>|value(s)
p||pad, p = pad(n,8)
.TE
.RE
.NH 2
ByteOrder
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|1|ByteOrder
1||byte-order:
|0|   LittleEndian
|1|   BigEndian
1||unused
4|0|length

.TE
.RE
.NH 2
ConnectionSetup
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|2|ConnectionSetup
1|CARD8|Number of versions offered
1|CARD8|Number of authentication protocol names offered
4|(i+j+k+m+p)/8+1|length
1|BOOL|must-authenticate
7||unused
i|XPCS|vendor
j|XPCS|release
k|LISTofXPCS|authentication-protocol-names
m|LISTofVERSION|version-list
p||unused, p = pad(i+j+k+m,8)

.TE
.RE
.NH 2
AuthenticationRequired
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|3|AuthenticationRequired
1|CARD8|authentication-protocol-index
1||unused
4|(n+p)/8+1|length    
2|n|length of authentication data
6||unused
n|<varies>|data
p||unused, p = pad(n,8)

.TE
.RE
.NH 2
AuthenticationReply
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|4|AuthenticationReply
2||unused
4|(n+p)/8+1|length
2|n|length of authentication data
6||unused
n|<varies>|data
p||unused, p = pad(n,8)

.TE
.RE
.NH 2
AuthenticationNextPhase
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|5|AuthenticationNextPhase
2||unused
4|(n+p)/8+1|length
2|n|length of authentication data
6||unused
n|<varies>|data
p||unused, p = pad(n,8)
.TE
.RE
.NH 2
ConnectionReply
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|6|ConnectionReply
1|CARD8|version-index
1||unused
4|(i+j+p)/8|length
i|XPCS|vendor
j|XPCS|release
p||unused, p = pad(i+j,8)

.TE
.RE
.NH 2
ProtocolSetup
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|7|ProtocolSetup
1|CARD8|major-opcode
1||unused
4|(i+j+k+m+n+p)/8+1|length
1|CARD8|Number of versions offered
1|CARD8|Number of authentication protocol names offered
6||unused
i|XPCS|protocol-name
j|XPCS|vendor
k|XPCS|release
m|LISTofXPCS|authentication-protocol-names
n|LISTofVERSION|version-list
p||unused, p = pad(i+j+k+m+n,8)

.TE
.RE
.NH 2
ProtocolReply
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|8|ProtocolReply
1|CARD8|version-index
1|CARD8|major-opcode
4|(i+j+p)/8|length
i|XPCS|vendor
j|XPCS|release
p||unused, p = pad(i+j, 8)

.TE
.RE
.NH 2
Ping
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|9|Ping
2|0|unused
4|0|length

.TE
.RE
.NH 2
PingReply
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|10|PingReply
2|0|unused
4|0|length

.TE
.RE
.NH 2
WantToClose
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|11|WantToClose
2|0|unused
4|0|length

.TE
.RE
.NH 2
NoClose
.RS
.TS
tab(|);
l l l
l l l

l l l.
Length|Type /|Description
|Value
1|0|ICE
1|12|NoClose
2|0|unused
4|0|length
.TE
.RE
.NH 1
Errors
.NH 2
Generic Errors
.LP
These errors should be used by all protocols, as applicable.
For ICE (major opcode 0), FatalToProtocol really means FatalToConnection.
.TS
lf7 l l.
Error-class		BadMinor
Offending-minor		Any
Severity		FatalToProtocol or CanContinue (protocol's discretion)
Values		None
.TE
.RS
Received a message with an unknown minor opcode.
.RE
.TS
lf7 l l.
Error-class		BadState
Offending-minor		Any
Severity		FatalToProtocol or CanContinue (protocol's discretion)
Values		None
.TE
.RS
Received a message with a valid minor opcode which is not appropriate
for the current state of the protocol.
.RE
.TS
lf7 l l.
Error-class		BadLength
Offending-minor		Any
Severity		FatalToProtocol or CanContinue (protocol's discretion)
Values		None
.TE
.RS
Received a message with a bad length.  The length of the message is
longer or shorter than required to contain the data.
.RE
.TS
lf7 l l.
Error-class		BadValue
Offending-minor		Any
Severity		CanContinue
Values	CARD32	Offset to offending value in offending message
	CARD32	Length of offending value (???)
	<varies>	Offending value (???)
.TE
.RS
Received a message with a bad value specified.
.RE
.NH 2
ICE Errors
.LP
These errors are all major opcode 0 errors.
.TS
lf7 l l.
Error-class		NoAuthentication
Offending-minor		ConnectionSetup, ProtocolSetup
Severity		ConnectionSetup \(-> FatalToConnection
		ProtocolSetup \(-> FatalToProtocol
Values	None
.TE
.RS
None of the authentication protocols offered are available.
.RE
.TS
lf7 l l.
Error-class		NoVersion
Offending-minor		ConnectionSetup, ProtocolSetup
Severity		ConnectionSetup \(-> FatalToConnection
		ProtocolSetup \(-> FatalToProtocol
Values	None
.TE
.RS
None of the protocol versions offered are available.
.RE
.TS
lf7 l l.
Error-class		AuthenticationRejected
Offending-minor		AuthenticationReply, AuthenticationRequired,
		AuthenticationNextPhase
Severity		FatalToProtocol
Values	XPCS	Reason
.TE
.RS
Authentication rejected.  The peer has failed to properly
authenticate itself.
.RE
.TS
lf7 l l.
Error-class		AuthenticationFailed
Offending-minor		AuthenticationReply, AuthenticationRequired,
		AuthenticationNextPhase
Severity		FatalToProtocol
Values	XPCS	Reason
.TE
.RS
Authentication failed.  AuthenticationFailed does not imply
that the authentication was rejected, as AuthenticationRejected
does.  Instead it means that the sender was unable to complete
the authentication for some other reason.  (For instance, it
may have been unable to contact an authentication server.)
.RE
.TS
lf7 l l.
Error-class		ProtocolDuplicate
Offending-minor		ProtocolSetup
Severity		FatalToProtocol (but see note)
Values	XPCS	Protocol Name
.TE
.RS
The protocol name was already registered.  This is fatal to
the "new" ProtocolSetup, but does not affect the existing
registration.
.RE
.TS
lf7 l l.
Error-class		MajorOpcodeDuplicate
Offending-minor		ProtocolSetup
Severity		FatalToProtocol (but see note)
Values	CARD8	Opcode
.TE
.RS
The major opcode specified was already registered.  This is
fatal to the "new" ProtocolSetup, but does not affect the
existing registration.
.RE
.TS
lf7 l l.
Error-class		UnknownProtocol
Offending-minor		ProtocolSetup
Severity		FatalToProtocol
Values	XPCS	protocol name
.TE
.RS
The protocol specified is not supported.
.RE
.TS
lf7 l l.
Error-class		BadMajor
Offending-minor		<any>
Severity		CanContinue (but see note)
Values	CARD8	Opcode
.TE
.RS
The opcode given is not one that has been registered.  There
is a good argument for making this be FatalToConnection.
.RE
.NH 1
Error Encoding
.LP
Generic Errors have classes in the range 0x8000..0xFFFF.
.TS
l l.
BadMinor	0x8000
BadState	0x8001
BadLength	0x8002
BadValue	0x8003
.TE
.LP
Per-Protocol Errors have classes in the range 0x0000-0x7fff.
.LP
ICE Errors:
.TS
l l.
BadMajor	0
NoAuthentication	1
NoVersion	2
AuthenticationRejected	3
AuthenticationFailed	4
ProtocolDuplicate	5
MajorOpcodeDuplicate	6
UnknownProtocol	7
.TE
.ce
[end]
