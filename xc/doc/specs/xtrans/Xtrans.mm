'\".nr Ej 1
.PH "'''"
.ce
\fBX Transport Interface\fR
.sp
.ce
\fIDraft Version 0.6\fR
.sp 3
.H 1 "Purposes and Goals"
.P
The X Transport Interface is intended to combine all system and transport
specific code into a single place in the source tree. This API should be used
by all libraries, clients and servers of the X Window System. Use of this API
should allow the addition of new types of transports and support for new
platforms without making any changes to the source except in the X Transport
Interface code.
.P
This interface should solve the problem of multiple #ifdef TRANSPORT and
#ifdef PLATFORM statements scattered throughout the source tree.
.P
This interface should provide enough functionality to support all types of
protocols, including connection oriented protocols such as X11 and FS, and
connection-less oriented protocols such as XDMCP.
.H 1 "Overview of the interface"
.P
The interface provides an API for use by applications. The functions in this
API perform work that is common to all transports and systems, such as
parsing an address into a host and port number. The functions in this API
call transport specific functions that are contained in a table whose
contents are defined at compile time. This table contains an entry for each
type of transport. Each entry is a record containing mostly pointers to
function that implements the interface for the given transport.
.P
This API does not provide an abstraction for select() or poll().
These function are themselves transport independent, so an additional interface
is less for these functions.  It is also unclear how such an interface would
affect performance.
.H 1 "Definition of Address Specification Format"
.P
Addresses are specified in the following syntax,
.sp
\fIprotocol/host:port\fR
.sp
where \fIprotocol\fR specifies a protocol family or an alias for a protocol
family. A definition of common protocol families is given in a later section.
.P
The \fIhost\fR part specifies the name of a host or other transport dependent
entity that could be interpreted as a Network Service Access Point (NSAP).
.P
The \fIport\fR part specifies the name of a Transport Service Access Point
(TSAP). The format of the TSAP is defined by the underlying transport
implementation, but it is represented using a string format when it is part
of an address.
.H 1 "Internal Data Structures"
.P
There are two major data structures associated with the transport independent
portion of this interface.  Addition data structures may be used internally by
each transport.
.H 2 "Xtransport"
.P
Each transport supported has an entry in the transport table.
The transport table is an array of \fIXtransport\fR records. Each record
contains all the entry points for a single transport. This record is defined as:
.DS
.nf
typedef struct _Xtransport {
        char    *TransName;
        int    flags;
        XtransConnInfo  *(*OpenCOTSClient)(struct _Xtransport *, char *,
							char *, char *);
        XtransConnInfo *(*OpenCOTSServer)(struct _Xtransport *, char *,
							char *, char *);
        XtransConnInfo  *(*OpenCLTSClient)(struct _Xtransport *, char *,
							char *, char *);
        XtransConnInfo  *(*OpenCLTSServer)(struct _Xtransport *, char *,
							char *, char *);
        int     (*SetOption)(struct _XtransConnInfo *, int, int, int);
        int     (*CreateListener)(struct _XtransConnInfo *, int, char *);
        XtransConnInfo     *(*Accept)(struct _XtransConnInfo *, int);
        int     (*Connect)(struct _XtransConnInfo *, int, char *);
        int     (*BytesReadable)(struct _XtransConnInfo *, int, BytesReadable_t *);
        int     (*Read)(struct _XtransConnInfo *, int, char *, int);
        int     (*Write)(struct _XtransConnInfo *, int, char *, int);
        int     (*Readv)(struct _XtransConnInfo *, int, struct iovec *, int);
        int     (*Writev)(struct _XtransConnInfo *, int, struct iovec *, int);
        int     (*Disconnect)(struct _XtransConnInfo *, int);
        int     (*Close)(struct _XtransConnInfo *, int);
        int     (*NameToAddr)(struct _XtransConnInfo *, int);
        int     (*AddrToName)(struct _XtransConnInfo *, int);
        } Xtransport;

.fi
.DE
.P
The \fIflags\fR field currently contains only one flag \fITRANS_ALIAS\fR that
indicates that this record is providing an alias, and should not be used
to create a listner.

.H 2 "XtransConnInfo"
.P
Each connection will have an \fIXtransConnInfo\fR record allocated for it. This
record contains information specific to the connection. The record is defined
as:
.DS
.nf
typedef struct  _XtransConnInfo {
        struct _Xtransport     *transptr;
        char    *priv;
        int     flags;
        int     fd;
        int     family;
        char    *addr;
        int     addrlen;
        char    *peeraddr;
        int     peeraddrlen;
        } XtransConnInfo;
.fi
.DE
.H 1 "Exposed Transport Independent API"
.P
This API is included in each library and server that uses it. The API
may be used by the library, but it is not added to the public API for that
library.  This interface is simply an implementation
facilitator. This API contains a low level set of core primitives, and a few
utility functions that are built on top of the primitives. The utility
functions exist to provide a more familiar interface that can be used to
port existing code.
.P
A macro is defined in Xtrans.h for TRANS(func) that creates a unique function
name depending on where the code is compiled. For example, when built for Xlib,
TRANS(OpenCOTSClient) becomes _X11TransOpenCOTSClient.
.P
All failures are considered fatal, and the connection should be closed and
re-established if desired. In most cases, however, the value of errno will
be available for debugging purposes.
.H 2 "Core Interface API"
.BL
.LI
int TRANS(OpenCOTSClient)(char *address)
.P
This function creates a Connection-Oriented Transport that is suitable for
use by a client.
The parameter \fIaddress\fR contains the full address of the 
server to which this endpoint will be connected.
This functions returns a valid fd on success, or -1 on failure.
.LI
int TRANS(OpenCOTSServer)(char *address)
.P
This function creates a Connection-Oriented Transport that is suitable for
use by a server.
The parameter \fIaddress\fR contains the full address to which this
server will be bound.
This functions returns a valid fd on success, or -1 on failure.
.LI
int TRANS(OpenCLTSClient)(char *address)
.P
This function creates a Connection-Less Transport that is suitable for
use by a client.
The parameter \fIaddress\fR contains the full address of the 
server to which this endpoint will be connected.
This functions returns a valid fd on success, or -1 on failure.
.LI
int TRANS(OpenCLTSServer)(char *address)
.P
This function creates a Connection-Less Transport that is suitable for
use by a server.
The parameter \fIaddress\fR contains the full address to which this
server will be bound.
This functions returns a valid fd on success, or -1 on failure.
.LI
int TRANS(SetOption)(int fd, int option, int arg)
.P
This function sets transport options, similar to the way setsockopt() and
ioctl() work.
The parameter \fIfd\fR is an endpoint that was obtained from
_XTransOpen*() functions.
The parameter \fIoption\fR contains the option that will be set. The actual
values for \fIoption\fR are defined in a later section.
The parameter \fIarg\fR can be used  to pass in an additional value that may
be required by some options.
This function return 0 on success and -1 on failure.
.P
Note: Based on current usage, the complimentary function TRANS(GetOption)()
is not necessary.
.LI
int TRANS(CreateListener)(int fd, char *port)
.P
This function sets up the server endpoint for a non-blocking listen.
The parameter \fIfd\fR is an endpoint that was obtained from
TRANS(OpenCOTSServer)() or TRANS(OpenCLTSServer)().
The parameter \fIport\fR specifies the port to which this endpoint
should be bound for listening.
If \fIport\fR is NULL, then the transport may attempt to allocate any
available TSAP for this connection. If the transport cannot support this,
then this function will return a failure.
This function return 0 on success and -1 on failure.
.LI
int TRANS(Accept)(int fd)
.P
Once a connection indication is received, this function can be called to 
accept the connection.
The parameter \fIfd\fR is an opened and bound endpoint that was
obtained from TRANS(OpenCOTSServer)() and passed to TRANS(CreateListner)().
This function will return the fd for the new connection or -1 on failure.
.LI
TRANS(Connect)(int fd, char *address)
.P
This function creates a connection to a server.
The parameter \fIfd\fR is an endpoint that was obtained from
TRANS(OpenCOTSClient)().
The parameters \fIaddress\fR specify the TSAP to which this
endpoint should connect. If the protocol is included in the address, it will
be ignored.
.LI
int TRANS(BytesReadable)(int fd);
.P
This function provides the same functionality as the BytesReadable macro.
.LI
int TRANS(Read)(int fd, char *buf, int size)
.P
This function will return the number of bytes requested on a COTS connection,
and will return the minimum of the number bytes requested or the size of
the incoming packet on a CLTS connection.
.LI
int TRANS(Write)(int fd, char *buf, int size)
.P
This function will write the requested number of bytes on a COTS connection, and
will send a packet of the requested size on a CLTS connection.
.LI
int TRANS(Readv)(int fd, struct iovec *buf, int size)
.P
Similar to TRANS(Read)().
.LI
int TRANS(Writev)(int fd, struct iovec *buf, int size)
.P
Similar to TRANS(Write)().
.LI
int TRANS(Disconnect)(int fd)
.P
This function is used when an orderly disconnect is desired. This function
breaks the connection on the transport. It is similar to the
socket function shutdown().
.LI
int TRANS(Close)(int fd)
.P
This function closes the transport, unbinds it, and frees all resources that
was associated with the transport. If a _XTransDisconnect() call was not made
on the connection, a disorderly disconnect may occur.
.LI
int TRANS(NameToAddr)(int fd /*???what else???*/ )
.P
This function performs a name resolution in a transport dependent way.
The parameters of this function have not been finalized yet.
.LI
int TRANS(AddrToName)(int fd /*???what else???*/ )
.P
This function performs a reverse name lookup in a transport dependent way.
The parameters of this function have not been finalized yet.
.LI
int TRANS(GetMyAddr)(int fd,int *familyp, int *addrlenp, char **addrp )
.P
This function is similar to getsockname(). This function will allocate space
for the address, so it must be freed by the caller.  Not all transports will
have a valid address until a connection is established. This function should
not be used until the connection is established with Connect() or Accept().
.LI
int TRANS(GetPeerAddr)(int fd,int *familyp, int *addrlenp, char **addrp )
.P
This function is similar to getpeername().  This function will allocate space
for the address, so it must be freed by the caller.  Not all transports will
have a valid address until a connection is established. This function should
not be used until the connection is established with Connect() or Accept().
.LI
int TRANS(MakeAllCOTSServerListeners)(char *port, FdMask *fds)
.P
This function should be used by most servers. It will try to establish a COTS
server endpoint for each transport listed in the transport table.  The list
of fds pointed to by \fIfds\fR will be updated to reflect the actual fds that
were opened for listening by this function.
.LI
int TRANS(MakeAllCLTSServerListeners)(char *port, FdMask *fds)
.P
This function should be used by most servers. It will try to establish a CLTS
server endpoint for each transport listed int he transport table.  The list
of fds pointed to by \fIfds\fR will be updated to reflect the actual fds that
were opened for listening by this function.
.LE
.H 2 "Utility API"
.P
This section describes a few useful functions that have been implemented on top
of the Core Interface API. These functions are being provided as a convenience.
.BL
.LI
int TRANS(MakeConnection)(char *host, char *port, int retries, int *familyp, int *serveraddrlenp, char **serveraddrp)
.P
This function performs an Open and and Connect in a single function call. The
parameters are also similar to existing functions. The \fIfamilyp\fR parameter
will contain a family in the X protocol format (ie FamilyInternet).
.LI
int TRANS(ConvertFamily)(int *familyp, int *addrlen, char *addr)
.P
This function converts the value of an address family from the socket
definition (ie AF_INET, AF_UNIX), to the X protocol definition
(ie FamilyInternet, FamilyLocal).
.LE
.H 1 "Transport Option Definition"
.P
The following options are defined for the TRANS(SetOption)() function. If an
OS or transport does not support any of these options, then it will silently
ignore the option.
.BL
.LI
TRANS_NONBLOCKING
.P
This option controls the blocking mode of the connection. If the argument
is set to 1, then the connection will be set to blocking. If the argument
is set to 0, then the connection will be set to non-blocking.
.LI
TRANS_CLOSEONEXEC
.P
This option determines what will happen to the connection when an exec
is encountered. If the argument is set to 1, then the connection will be
closed when an exec occurs. If the argument is set to 0, then the connection
will not be closed when an exec occurs.
.LI
TRANS_COALESCENCE
.P
This option determines the coalescence behavior of a connection. If the
argument is set to 1, then the connection will be set to try and coalesce
data before sending it over the transport. If the argument is set to 0, then
the connection will send the data immediately without attempting any
coalescence.
.LE
.H 1 "Hidden Transport Dependent API"
.P
The hidden transport dependent functions are placed in the Xtransport record.
These function are similar to the Exposed Transport Independent API, but some
of the parameters and return values are slightly different.
Stuff like the #ifdef SUNSYSV should be handled inside these functions.
.BL
.LI
XtransConnInfo *OpenCOTSClient(struct _Xtransport *thistrans, char *protocol, char *host, char *port)
.P
This function creates a Connection-Oriented Transport. The parameter
\fIthistrans\fR points to an Xtransport entry in the transport table. The
parameters \fIprotocol\fR, \fIhost\fR, and \fIport\fR point to strings
containing the corresponding parts of the address that was passed into
TRANS(OpenCOTSClient)(). 
.P
This function must allocate and initialize the contents of the XtransConnInfo
structure that is returned by this function. This function will open the
transport, and bind it into the transport namespace if applicable. The
local address portion of the XtransConnInfo structure will also be filled
in by this function.
.LI
XtransConnInfo *OpenCOTSServer(struct _Xtransport *thistrans, char *protocol, char *host, char *port)
.P
This function creates a Connection-Oriented Transport. The parameter
\fIthistrans\fR points to an Xtransport entry in the transport table. The
parameters \fIprotocol\fR, \fIhost\fR, and \fIport\fR point to strings
containing the corresponding parts of the address that was passed into
TRANS(OpenCOTSClient)(). 
.P
This function must allocate and initialize the contents of the XtransConnInfo
structure that is returned by this function. This function will open the
transport. 
.LI
XtransConnInfo *OpenCLTSClient(struct _Xtransport *thistrans, char *protocol, char *host, char *port)
.P
This function creates a Connection-Less Transport. The parameter
\fIthistrans\fR points to an Xtransport entry in the transport table. The
parameters \fIprotocol\fR, \fIhost\fR, and \fIport\fR point to strings
containing the corresponding parts of the address that was passed into
TRANS(OpenCOTSClient)(). 
.P
This function must allocate and initialize the contents of the XtransConnInfo
structure that is returned by this function. This function will open the
transport, and bind it into the transport namespace if applicable. The
local address portion of the XtransConnInfo structure will also be filled
in by this function.
.LI
XtransConnInfo *OpenCLTSServer(struct _Xtransport *thistrans, char *protocol, char *host, char *port)
.P
This function creates a Connection-Less Transport. The parameter
\fIthistrans\fR points to an Xtransport entry in the transport table. The
parameters \fIprotocol\fR, \fIhost\fR, and \fIport\fR point to strings
containing the corresponding parts of the address that was passed into
TRANS(OpenCOTSClient)(). 
.P
This function must allocate and initialize the contents of the XtransConnInfo
structure that is returned by this function. This function will open the
transport.
.LI
int SetOption(struct _Xtransport *thistrans, int fd, int option, int arg )
.P
This function provides a transport dependent way of implementing the options
defined by the X Transport Interface. In the current prototype, this function
is not being used, because all of the option defined so far, are transport
independent. This function will have to be used if a radically different
transport type is added, or a transport dependent option is defined.
.LI
int CreateListener(struct _Xtransport *thistrans, int fd, char *port )
.P
This function takes a transport endpoint opened for a server, and sets it
up to listen for incoming connection requests. The parameter \fIport\fR
should contain the port portion of the address that was passed to the Open
function.
.P
This function will bind the transport into the transport name space if
applicable, and fill in the local address portion of the XtransConnInfo
structure. The transport endpoint will then be set to listen for
incoming connection requests.
.LI
XtransConnInfo Accept(struct _Xtransport *thistrans, int fd )
.P
This function creates a new transport endpoint as a result of an incoming
connection request. The parameter \fIfd\fR is the endpoint that was opened
for listening by the server. The new endpoint is opened and bound into the
transport's namespace. A XtransConnInfo structure describing the new endpoint
is returned from this function
.LI
int Connect(struct _Xtransport *thistrans, int fd, char *host, char *port )
.P
This function establishes a connection to a server. The parameters \fIhost\fR
and \fIport\fR describe the server to which the connection should be
established. The connection will be established so that Read() and Write()
call can be made.
.LI
int BytesReadable(struct _Xtransport *thistrans, int fd, BytesReadable_t *pend )
.P
This function replaces the BytesReadable() macro. This allows each transport
to have it's own mechanism for determining how much data is ready to be read.
.LI
int Read(struct _Xtransport *thistrans, int fd, char *buf, int size )
.P
This function reads \fIsize\fR bytes into \fIbuf\fR from the connection.
.LI
int Write(struct _Xtransport *thistrans, int fd, char *buf, int size )
.P
This function writes \fIsize\fR bytes from \fIbuf\fR to the connection.
.LI
int Readv(struct _Xtransport *thistrans, int fd, struct iovec *buf, int size )
.P
This function performs a readv() on the connection.
.LI
int Writev(struct _Xtransport *thistrans, int fd, struct iovec *buf, int size )
.P
This function performs a writev() on the connection.
.LI
int Disconnect(struct _Xtransport *thistrans, int fd )
.P
This function initiates an orderly shutdown of a connection. If a transport
does not distinguish between orderly and disorderly disconnects, then a
call to this function will have no affect.
.LI
int Close(struct _Xtransport *thistrans, int fd )
.P
This function will break the connection, and close the endpoint.
.LI
int NameToAddr(struct _Xtransport *thistrans, int fd )
.P
This function performs name resolution in a transport dependent way. Actual
use of this function is still being determined.
.LI
int AddrToName(struct _Xtransport *thistrans, int fd )
.P
This function performs reverse name resolution in a transport dependent way.
Actual use of this function is still being determined.
.LE
.H 1 "Configuration"
.P
The implementation of each transport can be platform specific. It is expected
that existing connection types such as TCPCONN, UNIXCONN, LOCALCONN and
STREAMSCONN will be replaced with flags for each possible transport type.
.P
Below are the flags that can be set in \fIConnectionFlags\fR in the vendor.cf
or site.def config files.
.TS
center;
l l .
TCPCONN	Enables the INET Domain Socket based transport
UNIXCONN	Enables the UNIX Domain Sokcet based transport
TLICONN	Enables the TLI based transports
LOCALCONN	Enables the SYSV Local connection transports
DNETCONN	Enables the DECnet transports
.TE
.H 1 "Transport Specific Definitions"
.TS
center box;
lb | cb sb sb
lb | cb | cb | cb
lb | cb | cb | cb
l | l | l | l.
Protocol	Address Component
	_	_	_
Family	protocol	host	port
=
Internet	T{
inet
.br
tcp
.br
udp
T}	name of an internet addressable host	T{
string containing the name of a service or a valid port number.
.br
Example: "xserver0", "7100"
T}
_
DECnet	decnet	name of a DECnet addressable host	T{
string containing the complete name of the object.
.br
Example: "X$X0"
T}
_
NETware	ipx	name of a NETware addressable host	T{
Not sure of the specifics yet.
T}
_
OSI	osi	name of an OSI addressable host	T{
Not sure of the specifics yet.
T}
_
Local	T{
local
.br
pts
.br
named
.br
sco
.br
isc
T}	(ignored)	T{
String containing the port name, ie "xserver0", "fontserver0".
T}
.TE
.H 1 "Implementation Notes"
.P
This section refers to the prototype implementation that is being developed
concurrently with this document. This prototype has been able to flush out
many details and problems as the specification was being developed.
.P
All of the source code for this interface is located in xc/lib/xtrans.
.P
All functions names in the source are of the format TRANS(func)(). The TRANS()
macro is defined as
.DS
.sp
#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _PROTOCOLTrans##func
#else
#define TRANS(func) _PROTOCOLTrans/**/func
#endif
.sp
.DE
PROTOCOL will be uniquely defined in each directory where this code
is compiled. PROTOCOL will be defined to be the name of the protocol that is
implemented by the library or server, such as X11, FS, and ICE.
.P
All libraries and servers that use the X Transport Interface should have a new
file called transport.c. This file will include the transports based
on the configuration flags \fIConnectionFlags\fR. Below is an example
transport.c.
.DS
.nf
#include "Xtransint.h"

#ifdef DNETCONN
#include "Xtransdnet.c"
#endif
#ifdef LOCALCONN
#include "Xtranslocal.c"
#endif
#ifdef TCPCONN
#include "Xtranssock.c"
#endif
#ifdef TLICONN
#include "Xtranstli.c"
#endif
#include "Xtrans.c"
#include "Xtransutil.c"
.fi
.DE
.P
The source files for this interface are listed below.
.DS
.TS
center;
l l.
Xtrans.h	T{
Function prototypes and defines for
the Transport Independent API.
T}
Xtransint.h	T{
Used by the interface implementation only.
Contains the internal data structures.
T}
Xtranssock.c	T{
Socket implementation of the Transport Dependent API.
T}
Xtranstli.c	T{
TLI implementation of the Transport Dependent API.
T}
Xtransdnet.c	T{
DECnet implementation of the Transport Dependent API.
T}
Xtranslocal.c	T{
Implementation of the Transport Dependent API for
SYSV Local connections.
T}
Xtrans.c	T{
Exposed Transport Independent API Functions.
T}
Xtransutil.c	T{
Collection of Utility functions that use the
X Transport Interface.
T}
.TE
.DE
.P
The file \fIXtrans.h\fR contains much of the transport related code that
previously in Xlibint.h and Xlibnet.h. This will make the definitions
available for all transport users. This should also obsolete the equivilent
code in other libraries.
