.\" Use tbl, -ms, macros.t included
.\" macros.t ----------------------------------
.na
.de Ds
.nf
.\\$1D \\$2 \\$1
.ft 1
.ps \\n(PS
.if \\n(VS>=40 .vs \\n(VSu
.if \\n(VS<=39 .vs \\n(VSp
..
.de De
.ce 0
.if \\n(BD .DF
.nr BD 0
.in \\n(OIu
.if \\n(TM .ls 2
.sp \\n(DDu
.fi
..
.de FD
.LP
.KS
.TA .5i 3i
.ta .5i 3i
.nf
..
.de FN
.fi
.KE
.LP
..
.de IN		\" send an index entry to the stderr
.tm \\n%:\\$1:\\$2:\\$3
..
.de C{
.KS
.nf
.D
.\"
.\"	choose appropriate monospace font
.\"	the imagen conditional, 480,
.\"	may be changed to L if LB is too
.\"	heavy for your eyes...
.\"
.ie "\\*(.T"480" .ft L
.el .ie "\\*(.T"300" .ft L
.el .ie "\\*(.T"202" .ft PO
.el .ie "\\*(.T"aps" .ft CW
.el .ft R
.ps \\n(PS
.ie \\n(VS>40 .vs \\n(VSu
.el .vs \\n(VSp
..
.de C}
.DE
.R
..
.de Pn
.IN \\$2
.ie t \\$1\fB\^\\$2\^\fR\\$3
.el \\$1\fI\^\\$2\^\fP\\$3
..
.de PN
.IN \\$1
.ie t \fB\^\\$1\^\fR\\$2
.el \fI\^\\$1\^\fP\\$2
..
.de NT
.ne 7
.ds NO Note
.if \\n(.$ .ds NO \\$1
.ie n .sp
.el .sp 10p
.TB
.ce
\\*(NO
.ie n .sp
.el .sp 5p
.if '\\$1'C' .ce 99
.if '\\$2'C' .ce 99
.in +5n
.ll -5n
.R
..
.		\" Note End -- doug kraft 3/85
.de NE
.ce 0
.in -5n
.ll +5n
.ie n .sp
.el .sp 10p
..
.ny0
.\" end of macros.t --------------------------------------
.de Ip
.IP \(bu 3
..
.de Is
.IP \(sq 3
..
.de Cs
.IP
.nf
.ft CW
..
.de Ce
.ft P
.fi
..
.de Bu
.br
.ti +.5i
.ie \\n(.$ \\$1
.el \\(bu
..
.\"     These macros should select a typewriter font if you have one.
.de LS
.KS
.DS
.ps -2
.vs -2
.ft CW
.ta .25i .5i .75i 1.0i 1.25i 1.5i 1.75i 2.0i 2.25i 2.5i 2.75i 3.0i 
..
.de LE
.ft R
.ps +2
.ps +2
.DE
.KE 
..
.\"
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
\&
.sp 8
.ce 50
\s+3\fBExtending X for Recording\fP\s-3
.sp
\fBVersion 1.2\fP
.sp
\fBX Version 11, Release 6\fR
.sp 6
Martha Zimet
Network Computing Devices, Inc.
.ce 0
.bp
.br
\&
.sp 15
.ps 9
.nr PS 9
.LP
Copyright \(co 1993 Network Computing Devices, Inc.
.LP
Permission to use, copy, modify, distribute, and sell this
documentation for any purpose is hereby granted without fee,
provided that the above copyright notice and this permission
notice appear in all copies.  Network Computing Devices, Inc.
makes no representations about the suitability for any purpose
of the information in this document.  This documentation is
provided ``as is'' without express or implied warranty.
.ps 11
.nr PS 11
.bp 1
.EH '\fBExtending X for Recording - Version 1.2\fP''\fBX11, Release 6\fP'
.OH '\fBExtending X for Recording\- Version 1.2\fP''\fBX11, Release 6\fP'
.EF ''\fB % \fP''
.OF ''\fB % \fP''

.NH 1
Introduction
.XS
\*(SN Introduction
.XE
.NH 2
Overview 
.XS
\*(SN Overview
.XE
.LP
Several proposals have been written over the past few years that address some 
of the issues surrounding the recording and playback of user actions 
in the X Window System\s-4\u1\d\s0:
.Ip
\fISome Proposals for a Minimal X11 testing Extension\fP, 
Kieron Drake, UniSoft Ltd., April 1991
.Ip
\fIX11 Input Synthesis Extension Proposal\fP, Larry Woestman,
Hewlett Packard, November 1991
.Ip
\fIXTrap Architecture\fP, Dick Annicchiario, et al, Digital Equipment Corporation, 
July  1991
.Ip
\fIXTest Extension Recording Specification\fP, Yochanan Slonim, 
Mercury Interactive, December 1992
.FS
1. X Window System is a trademark of the Massachusetts Institute of
Technology.
.FE
.LP
This document both unifies and extends the previous diverse approaches to 
generate a proposal for an X extension that provides support for the recording 
of all core X protocol and arbitrary extension protocol. 
.LP
Input synthesis, or playback, has already been implemented in the XTest 
extension, an X Consortium standard, therefore, this extension 
is limited to recording.  This proposal addresses the issue of synchronization, 
since a recording solution would be incomplete without it.  The requirement to 
provide  synchronization of contextual information comes from the playback 
mechanism.  That is, there needs to be a way to interleave device or user 
events with their consequences.  Hence, contextual information must be 
captured during recording.  
.LP
Since it is impossible to predict in advance what synchronization information 
will be required by a particular application, the proposed extension takes 
a ``\fIbroad brush\fP'' approach, whereby facilities will exist to both record 
and filter core X protocol and arbitrary extension protocol.  As such, the 
proposal does not enforce a specific synchronization methodology (eg. window 
mapping/unmappping, cursor changes, text strings, etc.). 
.LP
Since DEC's contributed extension ``DEC-XTRAP'', Version 3.3, has provided  
core functionality for the proposed extension, the name of the extension, 
as seen by ListExtensions and QueryExtension, is ``XTRAP.''  
.LP
Several people have provided input to this document:
.LP
.TS
box center tab(|);
l  l.

Robert Chesler | Synergytics & Absol-puter
Kieron Drake   | UniSoft Ltd. 
Marc Evans     | Synergytics
Jim Fulton     | NCD
Ken Miller     | DEC
Bob Scheifler  | X Consortium 

.TE
.bp 

.NH 2
Goals
.XS
\*(SN Goals
.XE 
.LP
.RS
.Ip
To implement an X Consortium standard for recording and synchronization,  
whereby both device events and contextual synchronization information in the 
form of device event consequences are recorded. 
.Ip 
To record contextual information used in synchronized playback 
without beforehand knowledge of the application which is being recorded.
.Ip
To provide the ability to work with an arbitrary set of X extension protocol.
.RE
.NH 2
Requirements
.XS
\*(SN Requirements
.XE 
.LP
The extension should:
.RS
.IP 1. 
not be dependent on other clients or extensions for its operation
.IP 2. 
not significantly impact performance
.IP 3. 
should support the recording of all device input (core 
devices and XInput devices)
.IP 4. 
should be extensible
.IP 5. 
should support the recording of contextual information for user events
.IP 6. 
follow an event-driven synchronization model
.IP 7. 
handle byte-order and provide word-size independence 
.IP 8. 
support multiple client connections
.RE 

.NH 1
Design
.XS
\*(SN Design 
.XE
.NH 2
Overview
.XS
\*(SN Overview
.XE
.LP
The mechanism used by this extension for recording is to intercept, 
or trap, core X protocol and arbitrary X extension protocol entirely 
within the X server itself.  When the extension has been requested by one 
or more clients to intercept specific protocol, the protocol data is 
formatted and returned to the trapping clients.  The extension provides 
a mechanism for capturing all events, including input device events 
that go to no clients, that is analogous to a client expressing  ``interest'' 
in all events, in all windows, including the root window.  Event filtering 
in the extension provides a mechanism for feeding device events to 
trapping clients; it does not provide a mechanism for in-place, 
synchronous event substitution, modification, or withholding.  It is 
assumed that this functionality will be provided in another extension 
or in a later version of this extension.  
.LP
A 
.PN "Configuration"  
resource is added by the extension to the set of resources managed by 
the server.  Although any program with multiple paths open to the server 
is viewed as multiple clients by the X Window System protocol\s-4\u1\d\s0, 
the recommended communication model for a trapping client is to open 
two connections to the server and use one for configuration control and 
the other for reading protocol data.   The ``control'' connection can 
execute requests to obtain information about the supported protocol version, create and destroy configurations, specify protocol types to be intercepted 
and options enabled for a configuration, query the current state of a 
configuration, and to stop interception and reporting of protocol data.   
The ``data'' connection can execute a request to start interception 
and reporting of specified protocol for a particular configuration.  
Intercepted protocol is sent back on this connection and may be split 
across several reply packets.  
.LP
The extension manages the intercepted protocol data through a record format 
that describes and extends the intercepted protocol data.  The data can be 
unpackaged by any client that follows the data formatting protocol of 
the extension.  This data format wraps the core X protocol.  
Aligned extension-specific information is at the front of the data. 
.FS
1. Scheifler, Robert W. `X Window System Protocol Version 11' 
.FE
.bp 

.NH 2 
Types
.XS
\*(SN Types
.XE
.sp 2 
.LP
The following new types are used in the request definitions in the 
following section.
.LP
.TS
tab(@);
l l.
CONFIGURATION:@XID     
.TE
.LP
The CONFIGURATION type defines a client-side handle for a server 
\fBConfiguration\fP.  


.LP
.TS
tab(@);
l l.
RANGE:@\s+2[\s0\fIfirst\fP, \fIlast\fP: CARD8\s+2]\s0
RANGE16:@\s+2[\s0\fIfirst\fP, \fIlast\fP: CARD16\s+2]\s0
EXTRANGE:@\s+2[\s0\fImajor\fP\^: CARD8, \fIminor\fP: LISTofRANGE16\s+2]\s0   
.TE
.LP
.TS
tab(@);
l l l.
TRAP_FLAGS:@\s+2[\s0\fIflags-state\fP:@CARD32
@\fIcore-requests\fP:@LISTofRANGE
@\fIcore-replies\fP:@LISTofRANGE
@\fIext-requests\fP:@LISTofEXTRANGE
@\fIext-replies\fP:@LISTofEXTRANGE
@\fIevents\fP:@LISTofRANGE
@\fIerrors\fP:@LISTofRANGE\s+2]\s0         
.TE
.LP
The  
.PN TRAP_FLAGS 
structure contains the protocol values to be intercepted and the 
interception state for each protocol type.  Typically, this structure 
is sent by trapping clients when creating or modifying a configuration. 
.LP
The flags-state field is a mask of bits, one bit per field, that specifies 
the interception state for each protocol type.  Alternative values 
for each flags-state element are: {Disabled, Enabled}; the default is 
Disabled.  The following flags-state elements are supported:
.IP
.PN "TrapCoreRequest"  
.br 
Intercept core X protocol requests.  
.IP 
.PN "TrapCoreReply"
.br 
Intercept core X protocol replies. 
.IP
.PN "TrapExtRequest"  
.br 
Intercept arbitrary X extension requests.  
.IP 
.PN "TrapExtReply"
.br 
Intercept arbitrary X extension replies. 
.IP 
.PN "TrapEvent"
.br 
Intercept core X protocol events and arbitrary extension events. 
.IP 
.PN "TrapError" 
.br
Intercept core X protocol errors and arbitrary extension errors. 
.LP
The core-requests, core-replies, ext-requests, ext-replies, events and 
errors fields specify the specific core X protocol or X extension protocol 
to be intercepted by the extension: core protocol requests, core replies,  
extension requests, extension replies, events and errors, respectively.  
.bp
  
.LP
.TS
tab (@) ;
l l l.
TRAP_STATE:@\s+2[\s0\fIenabled\fP:@BOOL
@\fIflags-state\fP:@CARD32
@\fIintercepted-core-requests\fP:@LISTofRANGE
@\fIintercepted-core-replies\fP:@LISTofRANGE
@\fIintercepted-ext-requests\fP:@LISTofEXTRANGE
@\fIintercepted-ext-replies\fP:@LISTofEXTRANGE
@\fIintercepted-events\fP:@LISTofRANGE
@\fIintercepted-errors\fP:@LISTofRANGE\s+2]\s0
.TE 
.LP
This structure specifies the current state of a configuration 
environment.   
.PN "TRAP_STATE" 
is returned when the configuration state has been queried by a 
trapping client over the control connection.  
.LP
The enabled field specifies the state of data transfer between the extension 
and the trapping client, and is either enabled (1) or disabled (0).  The 
default state is disabled.  When enabled, all core X protocol and 
extension protocol received from (xRequest) or sent to (xReply, xError, 
xEvent) a particular client, and requested by the trapping client to 
be intercepted, will be reported to the trapping client over the 
data connection.  The flags-state field specifies the interception state 
for each protocol type.   The intercepted-core-requests 
field specifies core protocol requests the client has requested be 
intercepted; intercepted-core-replies specifies core replies;  intercepted-ext-requests specifies extension requests;  
intercepted-ext-replies specifies extension replies; 
intercepted-events specifies events; intercepted-errors specifies errors. 

.NH 2
Errors
.LP
.IP
\fIConfiguration\fP\^:
.br
This error is returned if the value for a Configuration argument 
in a request does not name a valid CONFIGURATION.  
.bp 

.NH 1
Protocol
.XS
\*(SN Protocol
.XE
.NH 2
Requests 
.XS
\*(SN Requests
.XE
.sp 
.LP
.PN "QueryVersion"
.TA .75i
.ta .75i
.IP 
\fImajor-version\fP\, \fIminor-version\fP\^: CARD16  
.LP 
\(-> 
.IP
\fImajor-version\fP\, \fIminor-version\fP\^: CARD16
.br
.IP
Errors:  
.LP
This request specifies the XTRAP extension protocol version the client 
would like to use.  When the specified protocol version is supported 
by the extension, the protocol version the server expects from the 
client is returned.  Clients should use this request before other XTRAP 
extension requests.  If this request is not the first extension request 
executed, the protocol version used by the extension is implementation 
dependent.  
.LP
This request determines whether or not the XTRAP extension protocol 
version specified by the client is supported by the extension.  If the 
extension supports the version specified by the client, this version number 
should be returned.  If the client has requested a higher version than is 
supported by the server, the server's highest version should be returned.  
Otherwise, if the client has requested a lower version than is supported
by the server, the server's lowest version should be returned.  This document 
defines major version one (1), minor version zero (0).  

.LP
.PN "CreateConfiguration"
.TA .75i
.ta .75i
.IP 
\fIid\fP\^: CONFIGURATION
.IP
\fIflags\fP\^: TRAP_FLAGS  
.br
.IP
Errors:   
.PN IDChoice , 
.PN Value , 
.PN Alloc  
.LP
This request creates a new 
.PN Configuration  
object within the server and assigns identifier id to it.  The flags 
argument specifies the values of the configuration environment.  When 
flags is ``NULL'', the interception state for each protocol type is Disabled  
and the ranges of protocol values are initialized to zero.  
.LP
The error 
.PN "IDChoice" 
is returned when the id argument is not in the range assigned to the 
client or is already associated with a configuration.  
.PN "Value" 
is returned when a value specified falls outside of the accepted range.   
.PN "Alloc" 
is returned when the extension is unable to allocate the necessary 
resources. 

.LP
.PN "FreeConfiguration"
.TA .75i
.ta .75i
.IP 
\fIconfiguration\fP\^: CONFIGURATION
.br
.IP
Errors:  
.PN Configuration 
.LP 
This request deletes the association between the resource ID and the 
configuration and destroys the configuration.  If enabled for the 
configuration, data transfer between the extension and any trapping 
clients will be discontinued.  All client connections that have enabled 
data transfer for the configuration will be released.  
.LP
A configuration is 
destroyed automatically when the connection to the creating client is 
closed down and the close-down mode is \fBDestroyAll\fP.  When the 
configuration argument is a not associated with a  
.PN Configuration  
previously created by 
.PN CreateConfiguration  
that is still associated with a resource ID,  
.PN Configuration 
error is returned.
.bp


.LP
.PN "ChangeConfiguration"
.TA .75i
.ta  .75i
.IP
\fIconfiguration\fP\^: CONFIGURATION
.IP
\fIid-base\fP\^: CARD32
.IP
\fIflags\fP\^: TRAP_FLAGS  
.br 
.IP
Errors:  
.PN Value , 
.PN Alloc , 
.PN Configuration  
.LP
This request specifies the configuration parameters to be modified for 
one client, or a set of clients, and associates both the parameters and 
the client(s) to the configuration.  Typically, this 
request is used by a trapping client over the control 
connection to express interest in specific core X protocol and X 
extension protocol to be intercepted by the extension.  
.LP   
The id-base argument is the resource identifier base sent to the client 
from the server in the connection setup reply, and hence, identifies the 
client to be trapped.  An id-base of zero specifies the configuration 
parameters will apply to the set of clients that includes all current 
existing clients and all future clients.  The flags argument specifies 
the values of the configuration environment, respective to the client or 
set of clients. 
.LP
The error 
.PN "Value" 
is returned when a value specified falls outside of the accepted range.  
.PN "Alloc" 
is returned when the extension is unable to allocate the necessary 
resources.  When the configuration argument is not valid, a 
.PN Configuration 
error is returned. 

  
.LP 
.PN "GetConfiguration"
.TA .75i
.ta  .75i
.IP
\fIconfiguration\fP\^: CONFIGURATION 
.IP
\fIid-base\fP\^: CARD32
.LP
\(-> 
.IP
\fIstate\fP\^: TRAP_STATE
.br
.IP
Errors:
.PN Match ,  
.PN Configuration 
.LP
This request is used by trapping client over the control connection to 
query the state of a configuration and returns the current configuration 
state and characteristics, specific to a trapped client or a set of 
clients.  The id-base argument identifies the client that has previously 
been associated with the configuration with 
.PN GetConfiguration . 
An id-base of zero signifies that the configuration has been associated 
with the set of clients that includes current existing clients and future clients. 
.LP
The error 
.PN "Match" 
is returned when the id-base and the configuration have not been 
previously associated with 
.PN GetConfiguration .  
When the configuration argument is not valid, a 
.PN Configuration 
error is returned. 
.bp

.LP 
.PN "EnableConfiguration"
.TA .75i
.ta  .75i
.IP
\fIconfiguration\fP\^: CONFIGURATION
.IP
\fIenable\fP\^: BOOL
.LP 
\(->\(pl  
.IP
\fIreplies-following-hint\fP\^:  CARD32
.IP 
\fIdirection\fP\^:  {FromClient, FromServer} 
.IP
\fIclient-swapped\fP\^:  BOOL 
.IP
\fIid-base\fP\^:  CARD32 
.IP
\fIclient-seq\fP\^:  CARD32
.IP 
\fIdata\fP\^: LISTofCARD8
.br 
.IP
Errors:  
.PN Value , 
.PN Alloc , 
.PN Configuration
.LP
This request specifies whether or not data transfer  is enabled between 
the trapping client and the extension, and returns the protocol data 
the trapping client has previously expressed interest in 
with the 
.PN "ChangeConfiguration"   
request.  Typically, the request to enable data transfer will come from the 
data connection, while the request to disable data transfer will come from the 
control connection.  
.LP
When enable is ``True'', once the extension completes processing this 
request, it begins intercepting and reporting to the trapping client all 
core and extension protocol received from or sent to the client associated 
with the configuration and that the trapping client has expressed interest 
in.  The protocol data reported to the trapping client over the data 
connection may be split across several reply packets.  
.LP
All intercepted protocol data is returned in the byte-order of the 
trapped client.  Therefore, trapping clients are responsible for all byte swapping, if required.
.LP
When enable is 
`False', the extension is directed to discontinue data transfer between the 
extension and the trapping client and disable protocol reporting.  Once the 
extension completes processing this request, no protocol received from or 
sent to the client will be reported. 
.LP
When the enable request is made over the data connection, the replies-following-hint contains a positive value that specifies the 
number of replies that are likely, but not required, to follow.  When the 
disable request is made over the control connection, the reply is sent over 
the data connection with replies-following-hint set to zero, 
indicating the end of the reply sequence.  Direction is set to the direction 
of the protocol data, which is either from the client (xRequest), or from 
the server (xReply, xError, xEvent).  Client-swapped is ``True'' if the 
byte order of the client being trapped is swapped relative to the server; otherwise, client-swapped is ``False.'' Id-base is the resource identifier 
base sent to the client from the server in the connection setup reply, 
and hence, identifies the client being trapped.  Client-seq is set to 
the sequence number of the request just processed by the server on behalf 
of the trapped client.  
.LP
The error 
.PN "Value" 
is returned when a value specified falls outside of the accepted range.  
.PN "Alloc" 
is returned when the extension is unable to allocate the necessary 
resources.   When the configuration argument is not valid, a 
.PN Configuration 
error is returned.    
