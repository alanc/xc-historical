
		X Display Manager Control Protocol (XDMCP)

	The X display manager (xdm) is designed to manage a collection of
local or remote displays.  We propose that remote displays communicate with
xdm using the protocol described herein.  This is not yet a formal proposal,
we want to encourage feedback and discussion before a formal proposal is
made.

	Xdmcp is designed to work using an unreliable datagram transport,
in which messages can be lost and reordered.  In particular, xdmcp should
work with the Internet UDP.  Any message sent must be acknowledged in order
to guarantee receipt.

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

The integer values are always sent most significant byte first.

CMD is an ISO Latin-1 string (case ignored) from the set:

	VALUE			MEANING
	-----			-------
	Query			poll for availible managers
	Willing			positive response to poll
	Unwilling		negative response to poll
	Request			service request
	Acknowledge		service acknowledgement
	Negative		service refusal
	Manage			service directive
	Unmanage		request for discontinuation of service
	Unmanged		positive acknowledgement of Unmanage
	Error			negative acknowledgement of any message


The Manage command is followed by an optional list of display-type
parameters seperated from "Manage" and each other by spaces:

	transient		request for single session (default)
	permanent		request for multiple sessions

	secure			Unmanage must have correct auth data
	insecure		Unmanage always suceeds


Here are some scenarios illustrating how the protocol works:

	X terminal			Display Manager

	X terminal turns on, decides
	to find out who on the network
	is available to manage it, and
	broadcasts a message:

		Query

			Message is lost in the
			network by some managers.

	Collects information on all
	willing display managers,
	presents the list to the user
	who selects one.  Periodically
	resends

		Query

					Recognizes Query broadcast and
					responds with

						Willing

	Updates the list of available
	Managers dynamically until the
	user chooses one.  Now sends a
	message to the desired server:

		Request

					Recognizes the Request message.
					Creates a new session ID and
					saves it as state, and
					acknowledges the Request with

						Acknowledge

			Message is delayed in the network.

	X terminal times out waiting
	for Acknowledge message.
	Sends another

		Request

					Recognizes the Request message as
					originating from a client from which
					a Request message has already been
					received, and for which a session ID
					exists as saved state.  As the
					Acknowledge message may not have been
					received because of network error,
					sends another

						Acknowledge

					using the same session ID.
					

	Receives the Acknowledge
	message and responds with

		Manage

			Message is delayed in the network.

	Terminal times out and sends

		Manage

					Receives the Manage, and notices
					that it is from the client that
					sent the Request message and that
					the session ID matches the one
					in saved state.  Terminates any
					existing session it thinks it has
					in progress for this display, and
					then initiates a new session with

						XOpenDisplay

					This will only fail in rare cases,
					such as catastrophic network
					failure, since XOpenDisplay uses
					a reliable stream protocol.

			A delayed Manage message is now
			delivered.
					
					Receives Manage message, and notices
					that the session ID corresponds to the
					session which is now in
					progress.  This message is discarded
					with no response.

	Receives OpenDisplay request
	and services regular X
	protocol.

			A delayed Request message is now
			delivered.
					
					Generates a new session ID and
					saves it as state, and responds
					with a new

						Acknowledge

					However, the existing session is
					not affected.

	Receives Acknowledge, notices
	that a session is already
	in progress.  Discards the
	message.


	Terminal is turned off and back
	on.  This time, the user requests
	a specific host instead asking for
	a menu.  Sends that host a

		Request

					Generates a new session ID
					(unless one already exists in
					saved state), saves it as state,
					and responds with an

						Acknowledge

	Receives Acknowledge and
	responds using the new
	session ID with

		Manage

					Receives the Manage message,
					notices that the session ID
					matches the session ID in saved
					state.  Terminates any session
					in progress for this display, and
					starts a new session with

						XOpenDisplay

	Receives OpenDisplay request
	and services regular X
	protocol.


					Session runs to termination.
					Manager then finishes session with

						XCloseDisplay

	Receives CloseDisplay request,
	Notices that it is for the
	first connection ever established
	in this session, and terminates
	all other existing clients and
	goes back to Query loop.  Thus,
	the first connection has been given
	special semantics.

In more detail, the sematics of each message is as follows.
In all cases, the DISPLAY value sent by the Terminal must be
a "canonical" string that the display manager can use in
XOpenDisplay.  For example, the hostname should be a fully
qualified domain name.  In all cases, the DISPLAY value sent
by a manager must match the DISPLAY value sent by the terminal.

Query:	Terminal -> Manager

	Requests the display manager to respond with a Willing
	message if it is willing to manage the display.  The
	Query might be broadcast on a local net, or might be
	directed specifically at a set of managers.  A
	broadcast might be used so that the terminal doesn't
	have to know where it should connect; this information
	can be stored in xdm resources instead.  A Query message
	is not required; a terminal can send a Request message
	directly to a desired manager.  The session ID and
	authorization fields sent in this message can be chosen
	arbitrarily by the terminal.

	Possible troubles:

		Not all managers receive the Query.

		Indication:  none if Query was broadcast,
			     else failure to receive Willing.

		Solution:  repeatedly send Query while waiting
		for user to choose a host.

Willing: Manager -> Terminal

	Sent in response to the Query message.  This message indicates
	the willingness to manage the terminal.  No state change is
	required in the Display Manager.  The session ID and
	authorization fields in this message must match those in the
	Query.

	Possible troubles:

		Willing not received by the terminal.

		Indication:  none if Query was broadcast,
			     else failure to receive Willing.
		
		Solution:  The terminal should continue to
		send Query messages until a response is received.

Unwilling: Manager -> Terminal

	Sent in response to the Query message.  This message indicates
	the unwillingness to manage the terminal.  No state change is
	required in the Display Manager.  The session ID and
	authorization fields in this message must match those in the
	Query.  This message should not be sent in response to a
	broadcast Query, only in response to a Query sent specifically
	to the manager.

	Possible troubles:

		Unwilling not received by the terminal.

		Indication:  failure to receive Unwilling.
		
		Solution:  The terminal should continue to
		send Query messages until a response is received.

Request: Terminal -> Manager

	Sent indicating desire for management.  The session ID
	and authorization fields in this message can be chosen as needed
	by the terminal.
  	
	Possible troubles:

		Request not received by manager.

		Indication:  Terminal timeout waiting for response.

		Solution:  Terminal resends Request message.

		Message received out of order by manager.

		Indication: none

		Solution:  Each time a Request is sent, the manager
		sends the session ID associated with the next session
		in the Acknowledge.  If that next session is not yet
		started, the manager will simply resend with the same
		session ID.  If the session is in progress, the manager
		will reply with a new session ID; in this case, the
		Acknowledge will be discarded by the terminal.

Acknowledge: Manager -> Terminal

	Sent to acknowledge a Request.  A session ID chosen by the
	manager will be included in this message and must be sent
	back by the terminal in an associated Manage message.  The
	session ID is associated with the "next" session.  The
	session ID should be unique over a reasonably long time
	period.  The manager must remember this session ID, and
	respond to any subsequent Request with the same session ID.
	If no session ID is currently remembered, the manager should
	create a new one.

	The authorization fields in this message will vary according to
	the authorization protocol being used.  In the simplest case,
	the manager should simply send fields defining the initial
	authorization to be used on all client connections.  In this
	case, the authorization fields sent in the Request message are
	ignored.  However, if the fields are actually being use to
	authenticate the manager, then the Acknowledge fields may contain
	some encrypted information based on the fields in the Request message.
	For example, the Request message might contain a clear text
	user name (supplied by the user at the terminal) plus a "magic
	cookie" encrypted with the user's private key.  The manager
	would respond with this same cookie plus some additional
	data (known by the terminal) plus the initial authorization
	data to be used on client connections, all encrypted under the
	same key.

	Possible troubles:

		Acknowledge not received by terminal.

		Indication: Terminal timeout waiting for response to Request.

		Solution:  Terminal resends Request message.

		Message received out of order by terminal.

		Indication:  Terminal receives Acknowledge after
		Manage has been sent.
		
		Solution:  Terminal discards Acknowledge messages after
		it has sent a Manage message.

Negative: Manager -> Terminal

	Sent when the manager refuses to manage the terminal.  The
	session ID should match the session ID sent in the Request.
	The authorization fields should be sent just as for the
	Acknowledge message.

	Possible troubles:

		Negative not received by terminal.

		Indication:  Terminal timeout waiting for response to Request.

		Solution:  Terminal resends Request message.

Manage: Terminal -> Manager

	Directs the Display Manager to initiate a session on the terminal.
	The session ID sent must match the session ID that was in the
	Acknowledge message.

	The authorization fields in this message will vary according to
	the authorization protocol being used.  In the simplest case,
	the fields in this message are arbitrary and will be ignored.
	If the fields are actually being use to authenticate the terminal,
	then the fields may contain some encrypted information based on
	the fields in the Acknowledge message.

	Possible troubles:

		Manage not received by manager.

		Indication:  Terminal timeout waiting for XOpenDisplay.

		Solution:  Terminal resends Manage message.

		Manage received out of order by manager.

		Indication:  session already in progress.

		Solution:  Manage message is discarded.

		Indication: Session ID doesn't match next session ID

		Solution:  Error message is sent to terminal.

XOpenDisplay: Manager -> Terminal

	Initiated to start a new session.  The connection should
	be initiated using whatever authorization was agreed to
	in the Acknowledge message.  This display connection
	should be considered "magic".  When this first connection
	is established, the terminal should execute an implicit
	GrabServer on behalf of the manager.  This will allow the
	manager (in case there is no real connection authorization
	protocol being used) a secure environment for initial user
	login.  When this connection is closed, the current session
	is over; the terminal should close all X connections and restart.

	Possible troubles:

		XOpenDisplay fails.

		Indication: XOpenDisplay returns NULL.

		Solution:  Discontinue session and wait for another
		Request message.

Unmanage: Terminal -> Manager

	TBD

Unmanaged: Manager -> Terminal

	TBD

This protocol is designed to satisfy the following constraints:

	Any message sent must be acknowledged in order to guarantee
	receipt.  This is satisfied in the Manage case by receipt
	of the XOpenDisplay command.  Sequence problems are solved
	by insisting that the session ID sent in the Acknowledge
	message is returned with the Manage message.  While waiting
	for a user to choose the desired host, making the Terminal
	repeatedly send out Query messages will cause any
	machines which suddenly start/stop responding to be
	reflected in the menu.  In this way, the user can
	wait for a machine to come online before choosing a
	session.

	Terminals may be turned off at random.  Any existing session
	running on a terminal which has been turned off must be
	identifiable.  This is made possible by the sequence
	Request/Acknowledge/Manage.  If this sequence is sent, any
	existing session is terminated immediately and a new session
	started.  There is the problem (at least with TCP) that
	connections may not be closed when the terminal is turned
	off.  The manager could ameliorate this for local connections
	by periodically XSync'ing on its own connection, and terminating
	the session if its own connection ever closes.

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
