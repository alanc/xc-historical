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
\s+2\fBInter-Client Exchange (ICE) Library\fP\s-2
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
.EH '\fBInter-Client Exchange Library\fP'''
.OH '''\fBInter-Client Exchange Library\fP'
.EF ''- % -''
.OF ''- % -''
.NH 1
Overview of ICE
.XS
\*(SN Overview of ICE
.XE
.LP
There are numerous possible "inter-client" protocols, with many similarities
and common needs - authentication, extension, version negotiation, byte
order negotiation, etc.  The ICE protocol is intended to provide a framework
for building such protocols, allowing them to make use of common negotiation
mechanisms and to be multiplexed over a single transport connection.
.LP
.NH 1
The ICE Library
.XS
\*(SN The ICE Library
.XE
.LP
A client that wishes to utilize ICE must first register the protocols it
understands with the ICE library.  It then either opens a connection with
another client, or it waits for connections made by other clients.  A client
can both initiate connections with other clients and be
waiting for clients to connect to itself (a nested session manager is an
example).  Once an ICE connection is established between the two clients, one
of the clients needs to initiate a \fIProtocol Setup\fP\^ in order to
"activate" a given protocol.  Once the other client accepts the
\fIProtocol Setup\fP\^, the
two clients are ready to start passing messages specific to that protocol to
each other.  Multiple protocols may be active on a single ICE connection.
Clients are responsible for notifying the ICE library when a protocol is no
longer active on an ICE connection (although ICE does not define how each
sub-protocol triggers a protocol shutdown).
.LP
The ICE library utilizes callbacks to process incoming messages.  There are
two main reasons for this:
.LP
.IP 1)
If we generate a message and want to wait for a reply, since there may
be several protocols enabled, messages from other protocols might arrive
before the reply we're really waiting for.  The library could buffer up
such messages, but utilizing callbacks is a much cleaner solution.  By
doing so, we can invoke the callback for the message received, then
continue waiting for our reply.
.LP
.IP 2)
By using callbacks, protocol setups and authentication happen "behind
the scenes."  For example, if the ICE library receives a \fIProtocol Setup\fP\^
and it requires authentication, it invokes a callback to get the
authentication data, and automatically sends an \fIAuthenticationRequired\fP\^
message to the other client.  Likewise, upon receiving the
\fIAuthenticationRequired\fP\^ message, a callback is invoked which provides
the reply data, and an \fIAuthenticationReply\fP\^ message is automatically
generated.  Furthermore, if a message totally unrelated to this
authentication arrives in the middle of the authentication, it can
be quietly processed by it's callback as if nothing happened.
.LP
.NH 1
Intended Audience
.XS
\*(SN Intended Audience
.XE
.LP
This document is intended primarily for implementors of protocol libraries
layered on top of ICE.  In addition, application writers that wish to have
their code accept ICE connections should look at the section titled
\fIListening For and Accepting ICE Connections\fP\^.
Typically, clients that wish to utilize ICE will make calls into individual
protocol libraries rather than directly make calls into the ICE library.
.LP
.NH 1
Header Files and Library Name
.XS
\*(SN Header Files and Library Name
.XE
.LP
ICElib based libraries or applications should include the header
file \fI<X11/ICE/ICElib.h>\fP\^.  This header file defines all of the
ICElib data structures and function prototypes.  \fIICElib.h\fP\^ includes the
header file \fI<X11/ICE/ICE.h>\fP\^ which defines all of the ICElib constants.
Protocol libraries that need to read and write messages should include
the header file \fI<X11/ICE/ICElibint.h>\fP\^.
.LP
Applications should link against ICElib using \fI-lICE\fP\^.
.LP
.NH 1
Note on prefixes
.XS
\*(SN Note on prefixes
.XE
.LP
Throughout the library, a distinction is made between a client that
initiates a \fIProtocol Setup\fP\^ and a client which responds with a
\fIProtocol Reply\fP\^.  The following prefixes are used by the library
functions and data structures to distinguish between the two:
.LP
.PN IcePO
- \fIIce Protocol Originator\fP\^
.LP
.PN IcePA
- \fIIce Protocol Acceptor\fP\^
.LP
.NH 1
Protocol Registration
.XS
\*(SN Protocol Registration
.XE
.LP
Before a \fIProtocol Setup\fP\^ can be sent from one client to another,
both sides must register the protocol with the ICE library.  The side
that initiates the \fIProtocol Setup\fP\^ must call the
.PN IceRegisterForProtocolSetup
function, and the side that responds with a \fIProtocol Reply\fP\^
must call the
.PN IceRegisterForProtocolReply
function.  Before describing these functions, \fIversion records\fP\^ and
\fIauthentication records\fP\^ are discussed.
.LP
.NH 2
Version Records
.XS
\*(SN Version Records
.XE
.LP
A protocol library may support multiple versions of the same protocol.
At protocol registration time, the library must specify a list of version
records.  A version record consists of a major and minor version of the
protocol, as well as a callback to be used for processing incoming messages.
.LP
.sp 0.5
For processing messages on behalf of the client that initiated a
\fIProtocol Setup\fP\^:
.LP
.sM
typedef struct {
.br
	int major_version;
.br
	int minor_version;
.br
	IcePOprocessMsgProc process_msg_proc;
.br
} IcePOversionRec;
.LP
.eM
.sp 0.5
For processing messages on behalf of the client that accepted a
\fIProtocol Setup\fP\^:
.LP
.sM
typedef struct {
.br
	int major_version;
.br
	int minor_version;
.br
	IcePAprocessMsgProc process_msg_proc;
.br
} IcePAversionRec;
.LP
.eM
.sp 0.5
When the ICE library reads a message with a major opcode other than
zero (reserved for ICE protocol), it needs to call a function which will
unpack the message and take the appropriate action.
.LP
If the message arrives at the client which initiated the \fIProtocol Setup\fP\^,
the
.PN IcePOprocessMsgProc
callback is invoked.  If the message arrives at the client which accepted
a \fIProtocol Setup\fP\^, the
.PN IcePAprocessMsgProc
callback is invoked.
.LP
.sp 0.5
.sM
typedef Bool (*IcePOprocessMsgProc)();
.FD 0
Bool POprocessMsgProc\^(\^\fIiceConn\fP, \fIopcode\fP\^, \fIlength\fP\^, \fIswap\fP\^, \fIreplyWait\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    int \fIopcode\fP\^;
.br
    unsigned long \fIlength\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    IceReplyWaitInfo \fIreplyWait\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.IP \fIopCode\fP 1i
The minor opcode of the message.
.IP \fIlength\fP 1i
The length (in multiple of 8 bytes) of the message beyond the ICE header.
.IP \fIswap\fP 1i
A flag which indicates if byte swapping is necessary.
.IP \fIreplyWait\fP 1i
Indicates if the client is waiting for a reply.
.LP
.eM
.sM
typedef void (*IcePAprocessMsgProc)();
.FD 0
void PAprocessMsgProc\^(\^\fIiceConn\fP, \fIopcode\fP\^, \fIlength\fP\^, \fIswap\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    int \fIopcode\fP\^;
.br
    unsigned long \fIlength\fP\^;
.br
    Bool \fIswap\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.IP \fIopCode\fP 1i
The minor opcode of the message.
.IP \fIlength\fP 1i
The length (in multiple of 8 bytes) of the message beyond the ICE header.
.IP \fIswap\fP 1i
A flag which indicates if byte swapping is necessary.
.LP
.eM
The
.PN IcePOprocessMsgProc
callback needs to check the \fIreplyWait\fP\^ argument.
If \fIreplyWait\fP\^ is
.PN NULL ,
the ICE library expects the function to
pass the message to the client via a callback.  For example, if
this is a Session Management \fISave Yourself\fP\^ message, this function
should notify the client of the \fISave Yourself\fP\^ via a callback.
.LP
However, if \fIreplyWait\fP\^ is not
.PN NULL ,
then the client is waiting for
a reply or an error for a message it previously sent.  \fIreplyWait\fP\^
is of type
.PN IceReplyWaitInfo .
.LP
.sM
typedef struct {
.br
	unsigned long sequence_of_request;
.br
	int major_opcode_of_request;
.br
	int minor_opcode_of_request;
.br
	IcePointer reply;
.br
} IceReplyWaitInfo;
.LP
.eM
.PN IceReplyWaitInfo
contains the major/minor opcodes and sequence # of
the message for which a reply is being waited for.  It also contains
a pointer to the reply message to be filled in (the protocol library
should cast this
.PN IcePointer
to the appropriate reply type).  In most
cases, the reply will have some fixed-size part, and the sender
function will have provided a pointer to a structure (e.g.) to hold
this fixed-size data.  If there is variable-length data, it would be
expected that the reply function will have to allocate additional
memory and store pointer(s) to that memory in the fixed-size
structure.  If the entire data is variable length (e.g., a single
variable-length string), then the sender function would probably
just pass a pointer to fixed-size space to hold a pointer, and the
reply function would allocate the storage and store the pointer.
It is the responsibility of the client receiving the reply to
free up any memory allocated on it's behalf.
.LP
The return value of
.PN IcePOprocessMsgProc
is a boolean set to
.PN True
if a reply or error has been returned (as opposed to a callback
having been invoked).  Note that an error should only be returned
if it corresponds to the reply being waited for.  Otherwise, the
.PN IcePOprocessMsgProc
should either handle the error internally, or invoke an error handler
for it's library.
.LP
.sp 0.5       
The
.PN IcePAprocessMsgProc
callback, on the other hand, should always pass
the message to the client via a callback.  For example, if this is a Session
Management \fIInteract Request\fP\^ message, this function should notify the
client of the \fIInteract Request\fP\^ via a callback.
.LP
The reason the
.PN IcePAprocessMsgProc
callback does not have a \fIreplyWait\fP\^ like
.PN IcePOprocessMsgProc
does, is because a process that is acting as
a "server" should never block for a reply (infinite blocking can
occur if the connecting client does not act properly).
.LP
Note that when reading messages, byte swapping may be necessary.  As a
convenience, the length field in the ICE header will be swapped by ICElib
if necessary.
.bp
The following macros should be used to read messages.  They will be discussed
in greater detail later in this document.
.LP
    IceReadSimpleMessage ()
.br
    IceReadCompleteMessage ()
.sp 0.5
    IceReadMessageHeader ()
.br
    IceReadData ()
.br
    IceReadData16 ()
.br
    IceReadData32 ()
.LP
.NH 2
Authentication Records
.XS
\*(SN Authentication Records
.XE
.LP
Protocol libraries may support multiple authentication methods for their
protocol.  For each authentication method, the library must specify the
name of the authentication method, as well as an authentication callback.
.LP
For authentication on behalf of the client that initiated a
\fIProtocol Setup\fP\^:
.LP
.sM
typedef struct {
.br
	char *auth_name;
.br
	IcePOauthProc auth_proc;
.brn
} IcePOauthRec;
.LP
.eM
.sp 0.5
For authentication on behalf of the client that accepted a
\fIProtocol Setup\fP\^:
.LP
.sM
typedef struct {
.br
	char *auth_name;
.br
	IcePAauthProc auth_proc;
.br
} IcePAauthRec;
.LP
.eM
.sp 0.5
.PN IcePOauthProc
is the callback invoked for the client that initiated
the \fIProtocol Setup\fP\^ and needs to respond to an
\fIAuthentication Required\fP\^ or
\fIAuthentication Next Phase\fP\^ message from the other client.
.LP
.sM
typedef IcePOauthStatus (*IcePOauthProc)();
.FD 0
IcePOauthStatus POauthProc\^(\^\fIauthStatePtr\fP, \fIcleanUp\fP\^, \fIswap\fP\^,
.br
                    \fIauthDataLen\fP\^, \fIauthData\fP\^, \fIreplyDataLenRet\fP\^, \fIreplyDataRet\fP\^, \fIerrorStringRet\fP\^)
.br
     IcePointer *\fIauthStatePtr\fP\^;
.br
     Bool \fIcleanUp\fP\^;
.br
     Bool \fIswap\fP\^;
.br
     int \fIauthDataLen\fP\^;
.br
     IcePointer \fIauthData\fP\^;
.br
     int *\fIreplyDataLenRet\fP\^;
.br
     IcePointer *\fIreplyDataRet\fP\^;
.br
     char **\fIerrorStringRet\fP\^;
.FN
.IP \fIauthStatePtr\fP 1i
A pointer to state maintained by the
authentication procedure.  The first time this
function is invoked, *\fIauthStatePtr\fP\^ is
.PN NULL ,
and the function should initialize it's state and set
this pointer.
.IP \fIcleanUp\fP 1i
If
.PN True ,
authentication is over, and the function
should clean up any state it was maintaining.  The
last 6 arguments should be ignored.
.IP \fIswap\fP 1i
If
.PN True ,
the \fIauthData\fP\^ may have to be byte swapped
(depending on it's contents).
.IP \fIauthDataLen\fP 1i
The length (in bytes) of the authenticator data.
.IP \fIauthData\fP 1i
The data from the authenticator.  This pointer may
point to a volatile block of memory.  So, if you need
to keep the data beyond this function call, be sure
to make a copy of it.
.IP \fIreplyDataLenRet\fP 1i
The length (in bytes) of the reply data returned.
.IP \fIreplyDataRet\fP 1i
The reply data returned.  ICElib will be
responsible for freeing this data.
.IP \fIerrorStringRet\fP 1i
If the authentication procedure encounters an error during
authentication, it should allocate and return
an error string.  ICElib will be
responsible for freeing this string.
.LP
.eM
The
.PN IcePOauthProc
should return one of four values:
.TS
lw(2i) lw(5i).
T{
.PN IcePOauthHaveReply
T}	T{
A reply is available
T}
.sp 4p
T{
.PN IcePOauthRejected
T}	T{
Authentication rejected
T}
.sp 4p
T{
.PN IcePOauthFailed
T}	T{
Authentication failed
T}
.sp 4p
T{
.PN IcePOauthDoneCleanup
T}	T{
Done cleaning up
T}
.TE
.LP
.sp 0.5
.PN IcePAauthProc
is the callback invoked for the client that received a
\fIProtocol Setup\fP\^ and needs to initiate an
\fIAuthentication Required\fP\^ message or respond to an
\fIAuthentication Reply\fP\^.
.LP
.sM
typedef IcePAauthStatus (*IcePAauthProc) ();
.FD 0
IcePAauthStatus PAauthProc\^(\^\fIauthStatePtr\fP, \fIswap\fP\^,
.br
                    \fIreplyDataLen\fP\^, \fIreplyData\fP\^, \fIauthDataLenRet\fP\^, \fIauthDataRet\fP\^, \fIerrorStringRet\fP\^)
.br
    IcePointer *\fIauthStatePtr\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIreplyDataLen\fP\^;
.br
    IcePointer \fIreplyData\fP\^;
.br
    int *\fIauthDataLenRet\fP\^;
.br
    IcePointer *\fIauthDataRet\fP\^;
.br
    char **\fIerrorStringRet\fP\^;
.FN
.IP \fIauthStatePtr\fP 1i
A pointer to state maintained by the
authentication procedure.  The first time this
function is invoked, *\fIauthStatePtr\fP\^ is
.PN NULL ,
and the function should initialize it's state and set
this pointer.  When authentication
is accepted or rejected, the function should clean up
the state it was maintaining.
.IP \fIswap\fP 1i
If
.PN True ,
the \fIreplyData\fP\^ may have to be byte swapped
(depending on it's contents).
.IP \fIreplyDataLen\fP 1i
The length (in bytes) of the authenticatee data.
.IP \fIreplyData\fP 1i
The data from the authenticatee.  This pointer may
point to a volatile block of memory.  So, if you need
to keep the data beyond this function call, be sure
to make a copy of it.
.IP \fIauthDataLenRet\fP 1i
The length of the authentication data returned.
.IP \fIauthDataRet\fP 1i
The authentication data returned.  ICElib will be
responsible for freeing this data.
.IP \fIerrorStringRet\fP 1i
If authentication is rejected or fails, an error
string is returned.  ICElib will be
responsible for freeing this string.
.LP
.eM
The
.PN IcePAauthProc
should return one of four values:
.TS
lw(2i) lw(5i).
T{
.PN IcePAauthContinue
T}	T{
Continue authentication
T}
.sp 4p
T{
.PN IcePAauthAccepted
T}	T{
Authentication accepted
T}
.sp 4p
T{
.PN IcePAauthRejected
T}	T{
Authentication rejected
T}
.sp 4p
T{
.PN IcePAauthFailed
T}	T{
Authentication failed
T}
.TE
.LP
.NH 3
Protocol Registration Functions
.XS
\*(SN Protocol Registration Functions
.XE
.LP
Before a client is ready to do a \fIProtocol Setup\fP\^ on a given protocol,
it must call
.PN IceRegisterForProtocolSetup
to reserve a major opcode and to provide information about the protocol.
.LP
.sM
.FD 0
int IceRegisterForProtocolSetup\^(\^\fIprotocolName\fP, \fIvendor\fP\^, \fIrelease\fP\^,
.br
                    \fIversionCount\fP\^, \fIversionRecs\fP\^, \fIauthCount\fP\^, \fIauthRecs\fP\^, \fIIOErrorProc\fP\^)
.br
     char *\fIprotocolName\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.br
     int \fIversionCount\fP\^;
.br
     IcePOversionRec *\fIversionRecs\fP\^;
.br
     int \fIauthCount\fP\^;
.br
     IcePOauthRec *\fIauthRecs\fP\^;
.br
     IceIOErrorProc \fIIOErrorProc\fP\^;
.FN
.IP \fIprotocolName\fP 1i
A string specifying the name of the protocol to register.
.IP \fIvendor\fP 1i
A string specifying the vendor implementing the protocol library.
.IP \fIrelease\fP 1i
A string specifying the release of the protocol library.
.IP \fIversionCount\fP 1i
The number of different versions of the protocol supported.
.IP \fIversionRecs\fP 1i
Information about each version of the protocol supported (prioritized in
decreasing order of preference).  See the previous section titled
\fIVersion Records\fP\^.
.IP \fIauthCount\fP 1i
The number of authentication methods supported.
.IP \fIauthRecs\fP 1i
Information about each authentication method (prioritized in decreasing
order of preference).  See the previous section titled
\fIAuthentication Records\fP\^.
.IP \fIIOErrorProc\fP 1i
Error handler to invoke if the ICE connection unexpectedly breaks.  Pass
.PN NULL
if not interested in being notified.
See the section titled \fIError Handling\fP\^ for more details.
.LP
.eM
.PN IceRegisterForProtocolSetup
returns the major opcode reserved, or -1 if an error occurred.
.LP
.sp
Before a client is ready to receive a \fIProtocol Setup\fP\^ for a given
protocol, it must call
.PN IceRegisterForProtocolReply
to reserve a major opcode and to provide information about the protocol.
.LP
.sM
.FD 0
int IceRegisterForProtocolReply\^(\^\fIprotocolName\fP, \fIvendor\fP\^, \fIrelease\fP\^,
.br
                    \fIversionCount\fP\^, \fIversionRecs\fP\^, \fIprotocolSetupNotifyProc\fP\^, \fIauthCount\fP\^, \fIauthRecs\fP\^, \fIIOErrorProc\fP\^)
.br
     char *\fIprotocolName\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.br
     int \fIversionCount\fP\^;
.br
     IcePAversionRec *\fIversionRecs\fP\^;
.br
     IceProtocolSetupNotifyProc \fIprotocolSetupNotifyProc\fP\^;
.br
     int \fIauthCount\fP\^;
.br
     IcePAauthRec *\fIauthRecs\fP\^;
.br
     IceIOErrorProc \fIIOErrorProc\fP\^;
.FN
.IP \fIprotocolName\fP 1i
A string specifying the name of the protocol to register.
.IP \fIvendor\fP 1i
A string specifying the vendor of the application using the protocol library.
For example, the vendor of a session manager.
.IP \fIrelease\fP 1i
A string specifying the release of the application using
the protocol library.  For example, the release of a
session manager.
.IP \fIversionCount\fP 1i
The number of different versions of the protocol supported.
.IP \fIversionRecs\fP 1i
Information about each version of the protocol supported (prioritized in
decreasing order of preference).  See the previous section titled
\fIVersion Records\fP\^.
.IP \fIprotocolSetupNotifyProc\fP 1i
A callback to be invoked when the client on the other side of
the ICE connection has successfully completed a \fIProtocol Setup\fP\^.
.IP \fIauthCount\fP 1i
The number of authentication methods supported.
.IP \fIauthRecs\fP 1i
Information about each authentication method (prioritized in decreasing
order of preference).  See the previous section titled
\fIAuthentication Records\fP\^.
.IP \fIIOErrorProc\fP 1i
Error handler to invoke if the ICE connection unexpectedly breaks.  Pass
.PN NULL
if not interested in being notified.
See the section titled \fIError Handling\fP\^ for more details.
.LP
.eM
.PN IceRegisterForProtocolReply
returns the major opcode reserved, or -1 if an error occurred.
.LP
Since \fIProtocol Setups\fP\^ happen "behind the scenes," the protocol library
needs some way of being notified when the \fIProtocol Setup\fP\^ has
successfully completed.  The
.PN IceProtocolSetupNotifyProc
callback serves this purpose.
.LP
.sM
typedef void (*IceProtocolSetupNotifyProc) ();
.FD 0
void ProtocolSetupNotifyProc\^(\^\fIiceConn\fP, \fImajorVersion\fP\^, \fIminorVersion\fP\^, \fIvendor\fP\^, \fIrelease\fP\^)
.br
     IceConn \fIiceConn\fP\^;
.br
     int \fImajorVersion\fP\^;
.br
     int \fIminorVersion\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.IP \fImajorVersion\fP 1i
The major version of the protocol.
.IP \fIminorVersion\fP 1i
The minor version of the protocol.
.IP \fIvendor\fP 1i
The vendor of the library that sent the \fIProtocol Setup\fP\^.
.IP \fIrelease\fP 1i
The release of the library that sent the \fIProtocol Setup\fP\^.
.LP
.eM
The \fIvendor\fP\^ and \fIrelease\fP\^ strings should be freed with free()
when they are no longer needed.
.LP
.NH 1
ICE Connections
.XS
\*(SN ICE Connections
.XE
.LP
In order for two clients to establish an ICE connection, one client has
to be "waiting" for connections, and the other client has to initiate the
connection.  Most clients will initiate connections, so we discuss that first.
.LP
.NH 2
Opening an ICE Connection
.XS
\*(SN Opening an ICE Connection
.XE
.LP
In order to open an ICE connection with another client (that is waiting
for connections), call the
.PN IceOpenConnection
function.
.LP
.sM
.FD 0
IceConn IceOpenConnection\^(\^\fInetworkIdsList\fP, \fIerrorLength\fP\^, \fIerrorStringRet\fP\^)
.br
     char *\fInetworkIdsList\fP\^;
.br
     int  \fIerrorLength\fP\^;
.br
     char *\fIerrorStringRet\fP\^;
.FN
.IP \fInetworkIdsList\fP 1i
Specifies the network ID(s) of the other client.
.IP \fIerrorLength\fP 1i
Length of the \fIerrorStringRet\fP\^ argument passed in.
.IP \fIerrorStringRet\fP 1i
Returns a null terminated error message, if any.  \fIerrorStringRet\fP
points to user supplied memory.  No more than \fIerrorLength\fP\^ bytes
are used.
.LP
.eM
.PN IceOpenConnection
returns an opaque ICE connection object if it succeeds,
.PN NULL
otherwise.
.LP
\fInetworkIdsList\fP\^ contains a list of network IDs separated by commas.
An attempt will be made to use the first network ID.  If that fails,
an attempt will be made using the second network ID, and so on.
Each network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
Most protocol libraries will have some sort of "open" function which should
internally make a call into
.PN IceOpenConnection .
If the ICE library detects that an ICE connection has already been opened
with the other client, a second connection is not made - the previously
allocated connection object is returned.
.LP
Any authentication requirements are handled internally by the ICE library.
.LP
After
.PN IceOpenConnection
is called, the client is ready to send a
\fIProtocol Setup\fP\^ (provided that
.PN IceRegisterForProtocolSetup
was called), or receive a \fIProtocol Setup\fP\^ (provided that
.PN IceRegisterForProtocolReply
was called).
.LP
.NH 2
Listening For and Accepting ICE Connections
.XS
\*(SN Listening For and Accepting ICE Connections
.XE
.LP
Clients wishing to accept ICE connections must first call
.PN IceListenForConnections
so they can listen for connections. A list of file descriptors and associated 
network IDs are returned.  The client does select() on these file descriptors
to detect new connections.
.LP
.sM
.FD 0
Status IceListenForConnections\^(\^\fIcountRet\fP, \fIdescripsRet\fP\^, \fInetworkIdsRet\fP\^, \fIerrorLength\fP\^, \fIerrorStringRet\fP\^)
.br
     int  *\fIcountRet\fP\^;
.br
     int  **\fIdescripsRet\fP\^;
.br
     char **\fInetworkIdsRet\fP\^;
.br
     int  \fIerrorLength\fP\^;
.br
     char *\fIerrorStringRet\fP\^;
.FN
.IP \fIcountRet\fP 1i
The number of file descriptors returned.
.IP \fIdescripsRet\fP 1i
Returns a list of file descriptors.
.IP \fInetworkIdsRet\fP 1i
Returns a list of network IDs corresponding to the list of file descriptors.
.IP \fIerrorLength\fP 1i
The length of the \fIerrorStringRet\fP\^ argument passed in.
.IP \fIerrorStringRet\fP 1i
Returns a null terminated error message, if any.  \fIerrorStringRet\fP
points to user supplied memory.  No more than \fIerrorLength\fP\^ bytes
are used.
.LP
.eM
\fInetworkIdsRet\fP\^ returns a list of network IDs separated by commas.
Each network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
\fIdescripsRet\fP\^ and \fInetworkIdsRet\fP\^ should be freed with free() when no longer needed.
.LP
.sp 0.5
After a connection attempt is detected via select() on a file
descriptor returned by
.PN IceListenForConnections ,
.PN IceAcceptConnection
should be called.  This returns a new opaque ICE connection object.
.LP
.sM
.FD 0
IceConn IceAcceptConnection\^(\^\fIfd\fP\^)
.br
    int \fIfd\fP\^;
.FN
.IP \fIfd\fP 1i
The file descriptor on which a new connection was detected.
.LP
.eM
Since the library can not block and wait for the connection to
become valid (infinite blocking can occur if the connecting client
does not act properly), the following steps must be taken before
the ICE connection is used to send and receive messages...
.LP
.sp 0.5
.vs 10
.nf
	newIceConn = IceAcceptConnection (listen_fd);
	status = IceConnectPending;
	timeStart = timeNow;

	do
	{
		select() on {newIceConn, all open connections}

		for each iceConn in the list of open connections
		{
			if (data ready on iceConn)
				IceProcessMessage (iceConn, NULL);
		}

		if (data ready on newIceConn)
		{
			/*
			 * IceProcessMessage is called until the connection
			 * is non-pending.  Doing so handles the connection
			 * setup request and any authentication requirements.
			 */

			IceProcessMessage (newIceConn, NULL);
			status = IceConnectionStatus (newIceConn);
		}
		else
		{
			if (timeNow - timeStart > MAX_WAIT_TIME)
				status = IceConnectRejected;
		}
	}
	while (status == IceConnectPending);

	if (status == IceConnectAccepted)
	{
		Add newIceConn to the list of open connections
	}
	else if (status == IceConnectRejected)
	{
		blah;
	}
.fi
.vs
.LP
.sp 0.5
After
.PN IceAcceptConnection
is called and the connection has been
validated, the client is ready to receive a \fIProtocol Setup\fP\^ (provided
that
.PN IceRegisterForProtocolReply
was called), or send a \fIProtocol Setup\fP\^
(provided that
.PN IceRegisterForProtocolSetup
was called).
.bp
.NH 2
Closing ICE Connections
.XS
\*(SN Closing ICE Connections
.XE
.LP
To close an ICE connection created with
.PN IceOpenConnection
or
.PN IceAcceptConnection ,
call the
.PN IceCloseConnection
function.
.LP
.sM
.FD 0
Status IceCloseConnection\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection to close.
.LP
.eM
In order to physically close an ICE connection, the following conditions
must be met:
.LP
.IP -
The \fIopen reference count\fP\^ must have reached zero on this ICE connection.
When
.PN IceOpenConnection
is called, it tries to use a previously opened
ICE connection.  If it is able to use an existing connection, it increments
the \fIopen reference count\fP\^ on the connection by one.  So in order to
close an ICE connection, each call to
.PN IceOpenConnection
must be matched with a call to
.PN IceCloseConnection .
The connection can be physically closed only
on the last call to
.PN IceCloseConnection .
Note that connections created with
.PN IceAcceptConnection
are never shared, and the \fIopen reference count\fP\^ for
such connections is never greater than one.
.LP
.IP -
The \fIactive protocol count\fP\^ must have reached zero.  Each time a
\fIProtocol Setup\fP\^ succeeds on the connection
the \fIactive protocol count\fP\^
is incremented by one.  When the client no longer expects to use the
protocol on the connection, the
.PN IceProtocolShutdown
function should be called, which decrements the \fIactive protocol count\fP\^
by one (see the \fIProtocol Shutdown\fP section of this document).
.LP
.PN IceCloseConnection
can return a bad status only under one condition.
If the \fIopen reference count\fP\^ has reached zero, but the
\fIactive protocol count\fP\^ has not reached zero, the client
neglected to shut down all active protocols before closing the connection.
The connection will not be closed.
.LP
.sp 0.5
When it is known that the client on the other side of the ICE connection
has terminated the connection without initiating shutdown negotiation, the
.PN IceSetShutdownNegotiation
function should be called to turn off shutdown negotiation.  This will prevent
.PN IceCloseConnection
from writing to a broken connection.
.LP
.sM
.FD 0
void IceSetShutdownNegotiation\^(\^\fIiceConn\fP, \fInegotiate\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    Bool \fInegotiate\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fInegotiate\fP 1i
If
.PN False ,
shutdown negotiating will be turned off.
.LP
.eM
In order to check the shutdown negotiation status of an ICE connection,
call the
.PN IceCheckShutdownNegotiation
function.
.LP
.sM
.FD 0
Bool IceCheckShutdownNegotiation\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.LP
.eM
.PN IceCheckShutdownNegotiation
returns
.PN True
if shutdown negotiation will take place on the connection,
.PN False
otherwise.  Negotiation is on by default for a connection.  It
can only be changed with the
.PN IceSetShutdownNegotiation
function.
.LP
.NH 2
Connection Watch Procedures
.XS
\*(SN Connection Watch Procedures
.XE
.LP
In order to add a watch procedure which will be called
each time ICElib opens a new connection via
.PN IceOpenConnection
or
.PN IceAcceptConnection ,
or closes a connection via
.PN IceCloseConnection ,
call the
.PN IceAddConnectionWatch
function.
.LP
.sM
.FD 0
Status IceAddConnectionWatch\^(\^\fIwatchProc\fP, \fIclientData\fP\^)
.br
    IceWatchProc \fIwatchProc\fP\^;
.br
    IcePointer \fIclientData\fP\^;
.FN
.IP \fIwatchProc\fP 1i
The watch procedure to invoke when ICElib physically opens or
closes a connection.
.IP \fIclientData\fP 1i
This pointer will be passed to the watch procedure.
.LP
.eM
Note that several calls to
.PN IceOpenConnection
might share the same ICE connection.  In such a case, the watch procedure
is only invoked when the connection is first created.  Similarly, since
connections might be shared, the watch procedure is called only if
.PN IceCloseConnection
physically closes the connection.
.LP
The watch procedures are very useful for Xt based applications, which
need to call
.PN XtAppAddInput
when new connections are created, and
.PN XtRemoveInput
when connections are destroyed.  Since connections are
shared, knowing when to call these Xt functions would be difficult
without the watch procedures.

The watch procedure is of type
.PN IceWatchProc .
.LP
.sM
typedef void (*IceWatchProc)();
.LP
.FD 0
void WatchProc\^(\^\fIiceConn\fP, \fIclientData\fP\^, \fIopening\fP\^, \fIwatchData\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    IcePointer \fIclientData\fP\^;
.br
    Bool \fIopening\fP\^;
.br
    IcePointer *\fIwatchData\fP\^;
.FN
.IP \fIiceConn\fP\^ 1i
The opened or closed ICE connection.  Call
.PN IceConnectionNumber
to get the file descriptor associated with this connection.
.IP \fIclientData\fP\^ 1i
Client data specified in the call to
.PN IceAddConnectionWatch .
.IP \fIopening\fP\^ 1i
If
.PN True ,
the connection is being opened.  If
.PN False ,
the connection is being closed.
.IP \fIwatchData\fP\^ 1i
Can be used to save a pointer to client data.
.LP
.eM
If opening is
.PN True ,
the client should set the *\fIwatchData\fP\^
pointer to any data it may need to save.  For example,
.PN XtAppAddInput
might be called, and you need to save the
.PN XtInputId .
.LP
If opening is
.PN False ,
you can use the *\fIwatchData\fP\^ pointer.
For example, *\fIwatchData\fP\^ might point to the
.PN XtInputId
which you need to call
.PN XtRemoveInput .
.LP
.sp 0.5
To remove a watch procedure, call the
.PN IceRemoveConnectionWatch
function.
.LP
.sM
.FD 0
void IceRemoveConnectionWatch\^(\^\fIwatchProc\fP, \fIclientData\fP\^)
.br
    IceWatchProc \fIwatchProc\fP\^;
.br
    IcePointer \fIclientData\fP\^;
.LP
.FN
.IP \fIwatchProc\fP 1i
The watch procedure that was passed to
.PN IceAddConnectionWatch .
.IP \fIclientData\fP 1i
The \fIclientData\fP\^ pointer that was passed to
.PN IceAddConnectionWatch .
.LP
.eM
.bp
.NH 1
Protocol Setup and Shutdown
.XS
\*(SN Protocol Setup and Shutdown
.XE
.LP
In order to activate a protocol on a given ICE connection, call the
.PN IceProtocolSetup
function.
.LP
.sM
.FD 0
IceProtocolSetupStatus IceProtocolSetup\^(\^\fIiceConn\fP, \fImyOpcode\fP\^, a\fIauthCount\fP\^, \fIauthIndices\fP\^,
.br
                    \fImajorVersionRet\fP\^, \fIminorVersionRet\fP\^, \fIvendorRet\fP\^, \fIreleaseRet\fP\^, \fIerrorLength\fP\^, \fIerrorStringRet\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    int \fImyOpcode\fP\^;
.br
    int \fIauthCount\fP\^;
.br
    int *\fIauthIndices\fP\^;
.br
    int *\fImajorVersionRet\fP\^;
.br
    int *\fIminorVersionRet\fP\^;
.br
    char **\fIvendorRet\fP\^;
.br
    char **\fIreleaseRet\fP\^;
.br
    int \fIerrorLength\fP\^;
.br
    char *\fIerrorStringRet\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImyOpcode\fP 1i
The major opcode of the protocol to be set up.  This major opcode
was returned by
.PN IceRegisterForProtocolSetup .
.IP \fIauthCount\fP 1i
The number of elements in the \fIauthIndices\fP\^ array.
.IP \fIauthIndices\fP 1i
A list of indices into the authentication protocols specified
in
.PN IceRegisterForProtocolSetup .
The indices are zero based.
.IP \fImajorVersionRet\fP 1i
The major version of the protocol to be used is returned.
.IP \fIminorVersionRet\fP 1i
The minor version of the protocol to be used is returned.
.IP \fIvendorRet\fP 1i
The vendor name of the protocol receiver is returned.  For example,
this might be the vendor name of a session manager.
.IP \fIreleaseRet\fP 1i  
The release of the protocol receiver is returned.  For example,
this might be the release of a session manager.
.IP \fIerrorLength\fP 1i
Specifies the length of the \fIerrorStringRet\fP\^ argument passed in.
.IP \fIerrorStringRet\fP 1i
Returns a null terminated error message, if any.  \fIerrorStringRet\fP
points to user supplied memory.  No more than \fIerrorLength\fP\^ bytes
are used.
.LP
.eM
When
.PN IceRegisterForProtocolSetup
was called, a list of all
authentication protocols supported were passed in.  At the time
.PN IceProtocolSetup
is called, it is possible that only a subset of
those protocols can be used for authentication.  So,
.PN IceProtocolSetup
takes a list of indices into the authentication protocols specified in
.PN IceRegisterForProtocolSetup .
.LP
As a special case, if \fIauthCount\fP\^ > 0 and \fIauthIndices\fP\^ is
.PN NULL ,
the first \fIauthCount\fP\^ authentication protocols specified in
.PN IceRegisterForProtocolSetup
will be used.
.LP
The \fIvendorRet\fP\^ and \fIreleaseRet\fP\^ strings should be freed
with free() when no longer needed.
.LP
.sp 0.5
.PN IceProtocolSetup
returns one of three values:
.LP
.TS
lw(2i) lw(4i).
T{
.PN IceProtocolSetupSuccess :
T}	T{
\fImajorVersionRet\fP\^, \fIminorVersionRet\fP\^, \fIvendorRet\fP\^, \fIreleaseRet\fP\^ are set.
T}
.sp 4p
T{
.PN IceProtocolSetupFailure :
T}	T{
Check errorStringRet for failure reason.  \fImajorVersionRet\fP\^, \fIminorVersionRet\fP\^, \fIvendorRet\fP\^, \fIreleaseRet\fP\^ are NOT set.
T}
.sp 4p
T{
.PN IceProtocolAlreadyActive :
T}	T{
This protocol is already active on this connection.  \fImajorVersionRet\fP\^, \fIminorVersionRet\fP\^, \fIvendorRet\fP\^, \fIreleaseRet\fP\^ are NOT set.
T}
.TE
.LP
.sp 0.5
In order to notify the ICE library when a given protocol
will no longer be used on an ICE connection, call the
.PN IceProtocolShutdown
function.
.LP
.sM
.FD 0
Status IceProtocolShutdown\^(\^\fIiceConn\fP, \fImajorOpcode\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    int \fImajorOpcode\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImajorOpcode\fP 1i
The major opcode of the protocol to shut down.
.LP
.eM
.PN IceProtocolShutdown
returns a bad status if
the major opcode was never registered OR the protocol of the
major opcode was never "activated" on the connection.  By "activated"
we mean that a \fIProtocol Setup\fP\^ succeeded on the connection.
Note that ICE does not define how each sub-protocol triggers a
protocol shutdown.
.LP
.NH 1
Processing Messages
.XS
\*(SN Processing Messages
.XE
.LP
When select() shows that there is data to read on an ICE connection
object,
.PN IceProcessMessage
should be called to process the message.
.LP
.sM
.FD 0
Bool IceProcessMessage\^(\^\fIiceConn\fP, \fIreplyWait\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    IceReplyWaitInfo *\fIreplyWait\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIreplyWait\fP 1i
Indicates if the client is waiting for a reply.
.LP
.eM
If the client is not waiting for a reply or error to a message it previously
sent, then \fIreplyWait\fP\^ should be set to
.PN NULL
and the message will be processed by invoking a callback.
If \fIreplyWait\fP\^ is not
.PN NULL ,
\fIreplyWait\fP\^ contains the major/minor opcodes and sequence #
of the message for which a reply is being waited for.  It also contains
a pointer to the reply message to be filled in (the protocol library
should cast this
.PN IcePointer
to the appropriate reply type).  In most
cases, the reply will have some fixed-size part, and the sender
function will have provided a pointer to a structure (e.g.) to hold
this fixed-size data.  If there is variable-length data, it would be
expected that the reply function will have to allocate additional
memory and store pointer(s) to that memory in the fixed-size
structure.  If the entire data is variable length (e.g., a single
variable-length string), then the sender function would probably
just pass a pointer to fixed-size space to hold a pointer, and the
reply function would allocate the storage and store the pointer.
It is the responsibility of the client receiving the reply to
free up any memory allocated on it's behalf.
.LP
.sp 0.5
.sM
typedef struct {
.br
	unsigned long sequence_of_request;
.br
	int major_opcode_of_request;
.br
	int minor_opcode_of_request;
.br
	IcePointer reply;
.br
} IceReplyWaitInfo;
.LP
.eM
.sp 0.5
The return value of
.PN IceProcessMessage
is a boolean set to
.PN True
if a reply or error has been returned (as opposed to a callback
having been invoked).
.bp
.NH 1
Ping
.XS
\*(SN Ping
.XE
.LP
To send a \fIPing\fP\^ message to the client on the other side of the
ICE connection, call the
.PN IcePing
function.
.LP
.sM
.FD 0
void IcePing\^(\^\fIiceConn\fP, \fIpingReplyProc\fP\^, \fIclientData\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    IcePingReplyProc \fIpingReplyProc\fP\^;
.br
    IcePointer \fIclientData\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIpingReplyProc\fP 1i
The callback to invoke when the \fIPing\fP\^ reply arrives.
.IP \fIclientData\fP 1i
This pointer will be passed to the
.PN IcePingReplyProc
callback.
.LP
.eM
When
.PN IceProcessMessage
processes the Ping reply, it will invoke the
.PN IcePingReplyProc
callback.
.LP
.sM
typedef void (*IcePingReplyProc)();
.LP
.FD 0
void PingReplyProc\^(\^\fIiceConn\fP, \fIclientData\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    IcePointer \fIclientData\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.IP \fIclientData\fP 1i
The client data specified in the call to
.PN IcePing .
.LP
.eM
.NH 1
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
IceConnectStatus IceConnectionStatus\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the status of an ICE connection created by
.PN IceAcceptConnection .
.LP
The possible return values are
.PN IceConnectPending ,
.PN IceConnectAccepted ,
or
.PN IceConnectRejected .
.LP
.sp 0.5
.sM
.FD 0
char *IceVendor\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the ICE library vendor on the other side of the connection.
The string should be freed with a call to free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
char *IceRelease\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the release of the ICE library on the other side of the connection.
The string should be freed with a call to free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
int IceProtocolVersion\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the major version of the ICE protocol on this connection.
.LP
.sp 0.5
.sM
.FD 0
int IceProtocolRevision\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the minor version of the ICE protocol on this connection.
.LP
.sp 0.5
.sM
.FD 0
int IceConnectionNumber\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the file descriptor of this ICE connection.
.LP
.sp 0.5
.sM
.FD 0
char *IceConnectionString\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
For ICE connections created with
.PN IceOpenConnection ,
this function returns the network ID of the client which
accepted this connection.
.br
For ICE connections created with
.PN IceAcceptConnection ,
this function will return
.PN NULL .
.br
The string should be freed with a call to free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
unsigned long IceLastSequenceNumber\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.eM
Returns the sequence number of the last message sent or received
on this ICE connection.
.LP
.NH 1
Macros For Sending ICE Messages
.XS
\*(SN Macros For Sending ICE Messages
.XE
.LP
.sM
.FD 0
IceGetHeader\^(\^\fIiceConn\fP, \fImajorOpcode\fP\^, \fIminorOpcode\fP\^, \fIheaderSize\fP\^, \fImsgType\fP\^, \fIpMsg\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImajorOpcode\fP 1i
The major opcode of the message.
.IP \fIminorOpcode\fP 1i
The minor opcode of the message.
.IP \fIheaderSize\fP 1i
The size of the message header (in bytes).
.IP \fImsgType\fP 1i
The actual C data type of the message header.
.IP \fIpMsg\fP 1i
The message header pointer.  After this macro is called, the
library can store data in the message header.
.LP
.eM
.PN IceGetHeader
is used to set up a message header on an ICE connection.
It sets the major and minor opcodes of the message, and initializes
the message's length to the length of the header.  If additional
variable length data follows, the message's length field should be
updated.
.LP
All messages must have the following ICE header:
.LP
	CARD8	majorOpcode;
.br
	CARD8	minorOpcode;
.br
	CARD8	data[2];
.br
	CARD32	length B32;
.LP
The 3rd and 4th bytes of the message can be used as needed.
The field names \fImajorOpcode\fP\^, \fIminorOpcode\fP\^, and
\fIlength\fP\^ must be used in the message header (
.PN IceGetHeader
depends on this naming convention).
.LP
.sp 0.5
.sM
.FD 0
IceGetHeaderExtra\^(\^\fIiceConn\fP, \fImajorOpcode\fP\^, \fIminorOpcode\fP\^, \fIheaderSize\fP\^, \fIextra\fP\^, \fImsgType\fP\^, \fIpMsg\fP\^, \fIpData\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImajorOpcode\fP 1i
The major opcode of the message.
.IP \fIminorOpcode\fP 1i
The minor opcode of the message.
.IP \fIheaderSize\fP 1i
The size of the message header (in bytes).
.IP \fIextra\fP 1i
The size of the extra data beyond the header (in multiple of 8 bytes).
.IP \fImsgType\fP 1i
The actual C data type of the message header.
.IP \fIpMsg\fP 1i
The message header pointer.  After this macro is called, the
library can store data in the message header.
.IP \fIpData\fP 1i
Returns a pointer to the ICE output buffer which points
immediately after the message header.  The variable length
data should be stored here.  If there was not enough room
in the ICE output buffer, \fIpData\fP\^ is set to
.PN NULL .
.LP
.eM
.PN IceGetHeaderExtra
is used to generate a message with a fixed (and relatively small) amount
of variable length data.  The complete message must fit in the ICE output
buffer.
.LP
The
.PN IceGetOutBufSize (
\fIiceConn\fP\^) macro can be used to get the size of the ICE output buffer.
.LP
.sp 0.5
.sM
.FD 0
IceSimpleMessage\^(\^\fIiceConn\fP, \fImajorOpcode\fP\^, \fIminorOpcode\fP\^)
.FN
.br
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImajorOpcode\fP 1i
The major opcode of the message.
.IP \fIminorOpcode\fP 1i
The minor opcode of the message.
.LP
.eM
.PN IceSimpleMessage
is used to generate a message which is identical
in size to the ICE header message, and has no additional data.
.LP
.sp 0.5
.sM
.FD 0
IceErrorHeader\^(\^\fIiceConn\fP, \fIoffendingMajorOpcode\fP\^, \fIoffendingMinorOpcode\fP\^, \fIoffendingSequenceNum\fP\^,
.br
                    \fIseverity\fP\^, \fIerrorClass\fP\^, \fIdataLength\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIoffendingMajorOpcode\fP 1i
The major opcode of the protocol in which an error was detected.
.IP \fIoffendingMinorOpcode\fP 1i
The minor opcode of the protocol in which an error was detected.
.IP \fIoffendingSequenceNum\fP 1i
The sequence number of the message that caused the error.
.IP \fIseverity\fP 1i
.PN IceCanContinue ,
.PN IceFatalToProtocol ,
or
.PN IceFatalToConnection .
.IP \fIerrorClass\fP 1i
The error class.  See below.
.IP \fIdataLength\fP 1i
Length of data (in multiple of 8 bytes) to be written after the header.
.LP
.eM
.PN IceErrorHeader
sets up an error message header.
.LP
Note that the two clients connected by ICE may be using different
major opcodes for a given protocol.  The \fImajorOpcode\fP\^ passed
to this macro is the major opcode of the protocol for the client
sending the error.
.LP
Generic errors which are common to all protocols have classes
in the range 0x8000..0xFFFF.  See the ICE protocol specification
document for more details.
.TS
lw(1i) lw(1i).
T{
.PN IceBadMinor
T}	T{
0x8000
T}
.sp 4p
T{
.PN IceBadState
T}	T{
0x8001
T}
.sp 4p
T{
.PN IceBadLength
T}	T{
0x8002
T}
.sp 4p
T{
.PN IceBadValue
T}	T{
0x8003
T}
.TE
.LP
Per-protocol errors have classes in the range 0x0000-0x7fff.
.LP
.sp 0.5
.sM
.FD 0
IceWriteData\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(char *) data\fP\^)
IceWriteData16\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(short *) data\fP\^)
IceWriteData32\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(long *) data\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to write.
.IP \fIdata\fP 1i
The data to write.
.LP
.eM
These macros write data to the ICE connection.  If the data fits
into the ICE output buffer, it is copied there.  Otherwise, the
ICE output buffer is flushed and the data is directly sent.
.LP
The macros are used in conjunction with
.PN IceGetHeader
and
.PN IceErrorHeader .
.LP
If
.PN IceWriteData16
is used, the data is a list of 16 bit quantities.
.br
If
.PN IceWriteData32
is used, the data is a list of 32 bit quantities.
.LP
Note: There is no implicit padding.  It is assumed that the caller
will insure proper alignment by using
.PN IceWritePad
if necessary (see below).
.LP
.sp 0.5
.sM
.FD 0
IceSendData\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(char *) data\fP\^)
IceSendData16\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(short *) data\fP\^)
IceSendData32\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(long *) data\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to send.
.IP \fIdata\fP 1i
The data to send.
.LP
.eM
.PN IceSendData
bypasses copying the data to the ICE output buffer
and sends the data directly.  If necessary, the ICE output buffer
is first flushed.
.LP
If
.PN IceSendData16
is used, the data is a list of 16 bit quantities.
.br
If
.PN IceSendData32
is used, the data is a list of 32 bit quantities.
.LP
.sp 0.5
.sM
.FD 0
IceWritePad\^(\^\fIiceConn\fP, \fIbytes\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of pad bytes.
.LP
.eM
.PN IceWritePad
can be used to force 32 or 64 bit alignment.
A maximum of 7 pad bytes can be specified.
.bp
.sM
.FD 0
IceFlush\^(\^\fIiceConn\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.LP
.eM
.PN IceFlush
flushes all of the data stored in the ICE output buffer.
.LP
.NH 1
Macros For Reading ICE Messages
.XS
\*(SN Macros For Reading ICE Messages
.XE
.LP
.sM
.FD 0
IceReadSimpleMessage\^(\^\fIiceConn\fP, \fImsgType\fP\^, \fIpMsg\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fImsgType\fP 1i
The actual C data type of the message header.
.IP \fIpMsg\fP 1i
This pointer is set to the message header.
.LP
.eM
.PN IceReadSimpleMessage
reads into the ICE input buffer a message which is identical
in size to the ICE header message (there is no additional data
after the 8 byte ICE header).
.LP
.sp 0.5
For a message with variable length data, there are two ways of reading
the message.  One method involves reading the complete message in one
pass using
.PN IceReadCompleteMessage .
The second method involves reading the message header, then reading
the variable length data in chunks (see
.PN IceReadMessageHeader
and
.PN IceReadData ).
.LP
.sM
.FD 0
IceReadCompleteMessage\^(\^\fIiceConn\fP, \fIheaderSize\fP\^, \fImsgType\fP\^, \fIpMsg\fP\^, \fIpData\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIheaderSize\fP 1i
The size of the message header (in bytes).
.IP \fImsgType\fP 1i
The actual C data type of the message header.
.IP \fIpMsg\fP 1i
This pointer is set to the message header.
.IP \fIpData\fP 1i
This pointer is set to the variable length data of the message.
.LP
.eM
The ICE library maintains an input buffer used to read messages.  If space
permits,
.PN IceReadCompleteMessage
will read the complete message into the
ICE input buffer.  Otherwise, a buffer will be allocated to hold the
variable length data.  After the call, the \fIpData\fP\^ argument should
be checked against
.PN NULL
to make sure that there was sufficient memory to allocate the buffer.
.LP
.sp 0.5
After calling
.PN IceReadCompleteMessage
and processing the message,
.PN IceDisposeCompleteMessage
should be called.
.LP
.sM
.FD 0
IceDisposeCompleteMessage\^(\^\fIiceConn\fP, \fIpData\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIpData\fP 1i
The pointer to the variable length data returned in
.PN IceReadCompleteMessage .
.LP
.eM
If a buffer had to be allocated to hold the variable length data (because
it didn't fit in the ICE input buffer), it is freed here by ICElib.
.LP
.sp 0.5
.sM
.FD 0
IceReadMessageHeader\^(\^\fIiceConn\fP, \fIheaderSize\fP\^, \fImsgType\fP\^, \fIpMsg\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIheaderSize\fP 1i
The size of the message header (in bytes).
.IP \fImsgType\fP 1i
The actual C data type of the message header.
.IP \fIpMsg\fP 1i
This pointer is set to the message header.
.LP
.eM
.PN IceReadMessageHeader
reads just the message header into the ICE input buffer.  The rest
of the data should be read with the
.PN IceReadData
family of macros.  This method of reading a message should be used when the
variable length data must be read in chunks.
.LP
.sp 0.5
.sM
.FD 0
IceReadData\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(char *) pData\fP\^)
IceReadData16\^(\^\fIiceConn\fP, \fIbytes\fP\^ \fI(short *) pData\fP\^)
IceReadData32\^(\^\fIiceConn\fP, \fIbytes\fP\^, \fI(long *) pData\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to read.
.IP \fIpData\fP 1i
The data is read into this user supplied buffer.
.LP
.eM
These macros read data directly into a user supplied buffer.
.LP
.PN IceReadData16
should be used to read the data as 16 bit quantities.
.br
.PN IceReadData32
should be used to read the data as 32 bit quantities.
.LP
.sp 0.5
.sM
.FD 0
IceReadPad\^(\^\fIiceConn\fP, \fIbytes\fP\^)
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of pad bytes.
.LP
.eM
.PN IceReadPad
can be used to force 32 or 64 bit alignment.
A maximum of 7 pad bytes can be specified.
.LP
.NH 1
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
There are two default error handlers in ICElib: 
one to handle typically fatal conditions (for example, 
a connection dying because a machine crashed) 
and one to handle ICE-specific protocol errors.
These error handlers can be changed to user-supplied routines if you
prefer your own error handling and can be changed as often as you like.
The action of the default handlers is to print an explanatory
message and exit.
.LP
.sp 0.5
To set the ICE error handler, use
.PN IceSetErrorHandler .
.LP
.sM
.FD 0
IceErrorHandler IceSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    IceErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The ICE error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN IceSetErrorHandler
returns the previous error handler.
.LP
The ICE error handler is invoked when an unexpected ICE protocol
error is encountered.  Note that errors in other protocol domains
should be handled by their respective libraries (these libraries
should have their own error handlers).
.LP
An ICE error handler has the type of
.PN IceErrorHandler :
.LP
.sp 0.5
.sM
typedef void (*IceErrorHandler)();
.FD 0
void ErrorHandler\^(\^\fIiceConn\fP, \fIswap\fP\^, \fIoffendingMinorOpcode\fP\^, \fIoffendingSequenceNum\fP\^, \fIerrorClass\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIoffendingMinorOpcode\fP\^;
.br
    unsigned long \fIoffendingSequenceNum\fP\^;
.br
    int \fIerrorClass\fP\^;
.br
    int \fIseverity\fP\^;
.br
    IcePointer \fIvalues\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffendingMinorOpcode\fP 1i
The ICE minor opcode of the offending message.
.IP \fIoffendingSequenceNum\fP 1i
The sequence number of the offending message.
.IP \fIerrorClass\fP 1i
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
The following error classes are defined at the ICE level.  Refer to the
\fIICE Protocol Specification\fP\^ for more details.
.LP
.PN IceBadMinor ,
.PN IceBadState ,
.PN IceBadLength ,
.PN IceBadValue ,
.PN IceBadMajor ,
.PN IceNoAuth ,
.PN IceNoVersion ,
.PN IceAuthRejected ,
.PN IceAuthFailed ,
.PN IceProtocolDuplicate ,
.PN IceMajorOpcodeDuplicate ,
or
.PN IceUnknownProtocol .
.LP
.sp 0.5
To handle fatal I/O errors, use
.PN IceSetIOErrorHandler .
.LP
.sM
.FD 0
IceIOErrorHandler IceSetIOErrorHandler\^(\^\fIhandler\fP\^)
.br
    IceIOErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The I/O error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN IceSetIOErrorHandler
returns the previous IO error handler.
.LP
An ICE I/O error handler has the type of
.PN IceIOErrorHandler :
.LP
.sM
typedef void (*IceIOErrorHandler)();
.LP
.FD 0
void IOErrorHandler\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.LP
.eM
The IO error handler should never return.  It should either do an
exit() or a long jump (using setjmp() and longjmp() calls).
Results are unpredicatable otherwise.  
.LP
.sp 0.5
Before the application I/O error handler is invoked, protocol libraries
that were interested in being notified of I/O errors will have their
.PN IceIOErrorProc
handlers invoked.  This handler is set up in the protocol registration
functions (see \fIIceRegisterForProtocolSetup\fP\^ and 
\fIIceRegisterForProtocolReply\fP\^), and could be used to clean up
state specific to the protocol.
.LP
.sM
typedef void (*IceIOErrorProc)();
.LP
.FD 0
void IOErrorProc\^(\^\fIiceConn\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.FN
.IP \fIiceConn\fP 1i
The ICE connection object.
.LP
.eM
Note that every
.PN IceIOErrorProc
callback must return.  This is required
because each active protocol must be notified of the broken connection,
and the application IO error handler must be invoked afterwards.
.LP
.NH 1
Utility Functions
.XS
\*(SN Utility Functions
.XE
.LP
To allocate scratch space for the client, use
.PN IceAllocScratch .
The client should \fInot\fP\^ free this memory!
.LP
.sM
.FD 0
char *IceAllocScratch\^(\^\fIiceConn\fP, \fIsize\fP\^)
.br
    IceConn \fIiceConn\fP\^;
.br
    unsigned long \fIsize\fP\^;
.FN
.IP \fIiceConn\fP 1i
A valid ICE connection object.
.IP \fIsize\fP 1i
The number of bytes required.
.LP
.eM
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.TC
