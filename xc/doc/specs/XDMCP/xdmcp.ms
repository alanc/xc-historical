
		X Display Manager Control Protocol

	The X display manager (xdm) is designed to manage a collection of
local or remote displays.  The remote displays communicate with xdm
using the protocol described herein.  This protocol does not represent
any commitment on the part of MIT, nor is it expected to ever be
standardized in this form.  If you have comments and suggestions,
please send them to keith@expo.lcs.mit.edu.  The server semantics
for the magic authorized connection are also subject to change, if
you see any holes in them, please let me know.

	Xdmcp is designed to accept multiple copies of any message,
in any order, and should work using UDP.

	The message format is fixed, although some messages may
use portions of the format for different things:

	2	CARD16		Version number
	2	i		length of CMD
	i	STRING8		CMD
	4	INT32		Session ID
	2	j		length of DISPLAY
	j	STRING8		DISPLAY
	2	k		length of AUTH_NAME
	k	STRING8		AUTH_NAME
	2	l		length of AUTH_DATA
	l	STRING8		AUTH_DATA


CMD is an ISO Latin-1 string (case ignored) from the set:

	query			poll for availible managers
	willing			response to poll
	request			service request
	acknowledge		service acknowledgement
	negative		service refusal
	manage			service directive
	unmanage		request for discontinuation of service
	unmanged		positive acknowledgement of unmanage
	protocol error		negative acknowledgement of any request

Here is how it works:

	X terminal			Display Manager

	X terminal turns on, decides
	to find out who on the network
	is availible to manage, and
	broadcasts a packet:

		query
			Packet is lost in Network

	Collects information on all
	willing display managers,
	presents the list to the user
	who selects one.  Periodically
	sending

		query

					Recognises query broadcast and
					responds with

						willing

	Updating the list of availible
	Managers dynamically until the
	user chooses one.  Now sends a
	packet to the desired server:

		request

					Recognises the request packet
					and changes state.  Any manage
					packets sent at this point will
					kill any existing session.
					Creates a new session ID and
					acknowledges the request with

						acknowledge

			Packet is lost in the network

	X terminal times out waiting
	for acknowledge packet.
	Sends another

		request

					Recognises the request packet as
					originating from a client from which
					a request packet has already been
					received.  As the acknowledge packet
					may not have been received because
					of network error, sends a duplicate

						acknowledge
					

	Receives the acknowledge
	packet and responds with

		manage

			Packet is lost in the network

	Terminal times out and sends

		manage

					Receives manage request, and notices
					that it is from a client which has
					recently sent a request packet.
					Initiates a session by starting up
					the greeting process:

						XOpenDisplay

			This will only fail in cases
			of catastrophic network
			failure, as XOpenDisplay uses
			a reliable stream protocol.

			First Manage packet is now
			delivered
					
					Receives manage request, and notices
					that it cooresponds to the
					XOpenDisplay which is now in
					progress.  This packet is ignored.

	Receives OpenDisplay request
	and services regular X
	protocol.

			First Request packet is now
			delivered
					
					Generates new Session ID and
					responds with another

						Acknowledge

	Receives Acknowledge, notices
	that a session is already
	in progress.  Ignores it.


	Terminal is turned off and back
	on.  This time, the user requests
	a specific host instead of the
	menu.  Sends another

		Request

					Generates new Session ID
					and responds with another

						Acknowledge

	Receives acknowledge and
	responds with the new Session
	ID with

		Manage

					Receives the manage request,
					notices that the Session ID
					matches the distributed
					session ID in the Acknowledge
					packet.  Terminates session
					in progress and starts
					session with

						XOpenDisplay

	Receives OpenDisplay request.
	

					Session runs to termination,
					Manager then finishes session with

						XCloseDisplay

	Receives CloseDisplay request
	on "magic" connection.  Terminates
	all existing clients and
	goes back to Query loop.

So, the sematics of each request are:

Query:	Terminal -> Manager

	Broadcast to the network requesting each display manager
	which receives the packet to simply respond with a Willing
	packet directed at the originating address.

	Possible troubles:

		Not all managers receive Query.  

		Indication:  none

		Solution:  repeatedly send Query while waiting
		for user to choose session.

Willing: Manager -> Terminal

	Sent in response to the Query request.  Simply indicates
	the network address of an availible Display Manager.  No
	state change implied in the Display Manager

	Possible troubles:

		Message not received.

		Indication:  none
		
		Solution:  Not My Problem.  If the Terminal
		is repeatedly sending Query packets, the
		Manager will eventually receive and return
		a packet succesfully.

Request: Terminal -> Manager

	Sent indicating desire for management.  
  	
	Possible troubles:

		Message not received.

		Indication:  Acknowledge receipt timesout

		Solution:  resend request

		Message received out of order.

		Indication: none

		Solution:  Each time a Request is sent, the Manager
		sends the Session ID associated with the next session.
		When a session is started, the Session ID sent will not
		match the current session, and the Manage request will
		be denied.

Acknowledge: Manager -> Terminal

	Sent acknowledging management request.  If the terminal does not
	receive this message, it should resend the Request packet.  Suitable
	timeouts are network dependent, and should be settable by the
	user.  A Session ID will be included in this message and must
	be sent back in an associated Manage request.  The session
	ID is associated with the "next" session.  Any session in
	progress will not match the session key sent back, if
	a session is not in progress the key that will be used
	for that session is sent.

	Possible troubles:

		Message not received.

		Indication: Terminal timeout waiting for Acknowledge

		Solution:  Terminal assumes that Request message was
		not received and handles it appropriately.

		Message received out of order.

		Indication:  Terminal receives message not just after
		sending Request.
		
		Solution:  Terminal ignores Acknowledge messages after
		it has sent the Manage message.

Manage: Terminal -> Manager

	Directs the Display Manager to initiate a session on the display.
	Includes enough information to authenticate connections and
	open the display with XOpenDisplay.  The Session ID which was
	sent in the Acknowledge packet is returned here.

	Possible troubles:

		Message not received.

		Indication:  Terminal timeout waiting for XOpenDisplay
		or Negative.

		Solution:  Resend message

		Message received out of order.

		Indication:  XOpenDisplay in progress

		Solution:  message ignored

		Indication: Session ID doesn't match next session ID

		Solution:  Negative message returned.

Negative: Manager -> Terminal

	Sent when the session ID sent in the Manage packet does not
	match the current session ID.


	Possible troubles:

		Message not received.

		Indication:  Terminal timeout waiting for
		XOpenDisplay or Negative

		Solution:  Terminal resends Manage request.

		Message received out of order

		Indication:  Terminal receives Negative message
		while not waiting for XOpenDisplay/Negative

		Solution:  Terminal ignores message

		Indication:  Session ID in Negative message doesn't
		match session ID in Manage.

		Solution:  Terminal ignores message

XOpenDisplay: Manager -> Terminal

	Initiated to start a new session.  This display connection
	should be considered "magic".  When this connection is
	closed, the current session is over.  Any appropriate
	actions can be taken by the terminal.  The most obvious results
	could be:  Close all X connections, Query to find list
	of availible Managers and start over from the top.


	Possible troubles:

		XOpenDisplay fails.

		Solution:  Discontinue session and wait for another
		request.

This protocol is designed to satisfy the following constraints:

	UDP is unreliable.  Any packet sent must be acknowledged
	else the receipt is not guaranteed.  This is satisfied
	in the Manage case by receipt of the XOpenDisplay command.
  	Sequence problems are solved by insisting that the
	session ID sent in the Acknowledge command are returned
	with the Manage command.  While waiting for a
	user to choose the desired host, making the Terminal
	repeatedly send out Query packets will cause any
	machines which suddenly start/stop responding to be
	reflected in the menu.  In this way, the user can
	wait for a machine to come online before choosing a
	session.

	Terminals may be turned off at random.  Any existing session
	running on a terminal which has been turned off must be
	identifyable.  This is made possible by the sequence
	Request, Manage.  If this sequence is sent, any existing
	session is terminated immediately and a new session started.

	Terminals cannot retain permanent state.  An ideal solution
	to sequence errors would be to use packet identifiers
	which increase monotonically to allow both sides to
	ignore packets which arrive out-of-sequence.  If terminals
	cannot retain permanent state this cannot work.  The
	session ID is generated by the Display Manager which is
	expected to ignore illegal sessions by retaining the
	appropriate state.

	A Manager must be able to identify terminals which have been
	turned off.  This is satisified by the terminal starting
	another Request/Acknowledge/Manage/XOpenDisplay sequence.

The manage command is followed by an optional list of display-type
parameters seperated from "manage" and each other by spaces:

	transient		request for single session (default)
	permanent		request for multiple sessions

	secure			unmanage must have correct auth data
	insecure		unmanage always suceeds

DISPLAY is a list of uninterpreted octects which can be passed to the
XOpenDisplay function to connect to the requesting display.

AUTH_NAME and AUTH_DATA are used by xdm to authenticate unmange requests and
are passed in the server connection setup dialog for the display to
authenticate the display manager.

The X server must recognise the display manager connection by checking
AUTH_NAME and AUTH_DATA; this connection must be handled specially:  when
the connection is made, an implicit grab-server is executed.  When the
connection is broken, the server must reset itself, disconnecting all
clients.  At this time, it can also request another session.

Any requesting display must have no other connected clients when xdm
requests its connection.  Either the other connections must be closed down
when this connection is made, or the display should disable connections
before sending the "manage" command and only accept authorized connections.

