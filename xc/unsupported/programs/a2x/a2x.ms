$XConsortium: a2x.doc,v 1.15 92/08/21 09:02:40 rws Exp $

				Using a2x

Copyright 1992 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

DragonDictate is a trademark of Dragon Systems, Inc.
X Window System is a trademark of the Massachusetts Institute of Technology


			     Getting Started

a2x is designed to interface DragonDictate to the X Window System.  A2x
converts the ASCII output of DragonDictate into X device events.  A2x is
capable of manipulating both the keyboard and the pointer; depending on the
applications, it is possible to use X hands-free through a2x.

a2x requires an X protocol extension to function.  Either the XTEST or
XTRAP extension can be used.  (There is code for the old XTESTEXT1
extension in the source, but it has never been tested and probably will not
work.)  The XTEST extension was developed by the X Consortium for use in
the X Test Suite.  It is not currently part of R5 or the Test Suite,
although it is expected that the X Consortium will release it prior to R6.
The XTRAP extension was developed by Digital Equipment Corp., and public
versions are available for both R4 and R5.  a2x was originally written for
the XTEST extension, but XTRAP is perfectly adequate.  (The main difference
as far as a2x is concerned is that XTEST permits time delays inside the X
server, while with XTRAP the delays are handled on the client side and
hence can be somewhat less accurate, but time delays are rarely needed.)

The Imakefile for a2x comes configured for building with XTEST; to build
with XTRAP change the line

	#define BuildWithXTest

to

	#define BuildWithXTrap

instead.

The documentation provided here is for using DragonDictate and a2x solely
for work under X; the assumption is that you do not use DragonDictate with
programs on the PC.  If you use DragonDictate with programs on the PC, you
may need to create dcom files for switching between PC and X use.

There are two ways to run DragonDictate: directly under DOS, and under
DESQview/X.  Running directly under DOS is described here; running under
DESQview/X is described later.

The normal mode of operation is to use a telnet program on the PC, and
telnet across to your workstation and log in.  You may wish to try using
rsh instead of telnet, but the performance may be substantially worse.  If
you do not have a network connection, it should be possible to use a
terminal program and log in to the workstation over a serial line, but
performance over a serial line will likely not be as good as over a
network.  In the telnet session, run the script "DragonDictate" provided
with the a2x distribution.  You should be able to speak this, by saying
"DragonDictate", "[enter key]", "OK" (the "OK" being necessary to pop down
the menu to permit execution to continue).  This script simply clears the
screen and starts up a2x.  Clearing the screen is a convenience so that the
focus of attention on the PC screen will be the DragonDictate menus.  You
can, of course, write your own script or simply run a2x directly.

You will probably find it convenient to force the DragonDictate menus to
appear in a fixed position on the PC screen.  For example, in the
"Parameters" menu from "Voice Console", set

	pop-up-row		0
	pop-up-col		30

You should also set

	pop-up-delay		0
	compatibility-delay	0

for quickest operation.

a2x takes each ASCII character produced by DragonDictate and determines a
key on the X keyboard that can be pressed and released to generate that
character.  If Shift and Control modifiers are necessary to generate the
character, a2x will automatically press and release keys for those
modifiers.  For example, for the character 'A', a2x will typically have to
press a Shift modifier key, press an 'a' key, release the 'a' key, and then
release the Shift modifier key.

It is not necessary to perform all of your "typing" through a2x; you can
mix a2x output with direct keyboard typing and direct pointer manipulation.
But, you should normally only switch between direct input and a2x input
when all keys and buttons are in the released state, to avoid confusing
a2x.  (One simple exception to this rule is that you can use a2x to
press/release buttons and keys while directly moving the pointer with your
hand.)

a2x interprets the character {c-t} specially; it introduces a control
sequence for special a2x operations.  The character {c-t} was chosen
because it occurs infrequently in normal text; escape was not chosen
because you might use it frequently in voice macros to send escape
sequences to applications.  There are two types of a2x control sequences:
{c-t} followed by a single control character, and {c-t} followed by a
string of characters terminated by another {c-t}.  Control sequences are
described in detail below.

a2x also interprets the backspace character ({c-h} or {Backspace})
specially.  When you make corrections through DragonDictate, e.g., you use
"choose <n>", DragonDictate will output backspaces to "undo" the previous
choice before outputting the characters for the new choice (if any).  For
many X users, it is more convenient to receive Delete (Rubout) characters
instead of backspaces.  The default action of a2x is to convert Backspace
to Delete; if you do not want this behavior, specify "-b" on the a2x
command line.

a2x also interprets backspace characters specially in that it assumes they
are produced by DragonDictate to make corrections and are not produced as
normal text (i.e., that voice macros do not contain backspace characters).
a2x looks at backspace characters in order to undo edits (described in more
detail below).  As such, it is recommended that you change some predefined
DragonDictate voice macros to replace backspace characters with delete
characters or with control sequences to generate backspace.  For example,
you should modify the keystrokes of the built-in commands "[backspace]",
"[back 1]", "[back 2]", "[back 3]", "[back 4]", and "[back 5]", replacing
each "{Backspace}" with "{Del}" to generate delete instead of backspace.
If you really want to generate backspace, then replace each "{Backspace}"
with the sequence "{c-t}{c-c}h" instead.

If you are using a terminal program over a serial line, make sure your
program is not automatically converting backspace to delete, as this will
cause a2x's undo mechanism to not work.


			     Keyboard Control

To "type" a non-ASCII key on the keyboard, use the following control
sequence:

	{c-t}<keysym>{c-t}

where <keysym> is the string name of a keysym.  a2x will find a key on the
keyboard labeled with this keysym, and press and release that key.  For
example:

	add-word /t "[doit]" "{c-t}Execute{c-t}" invisible

defines a voice macro to press and release a key labeled with the "Execute"
keysym.  If for some reason your Xlib does not know the string name of the
keysym you want, you can specify the hexadecimal value of the keysym
instead.  For example:

	add-word /t "[doit]" "{c-t}FF62{c-t}" invisible

would also specify the Execute keysym.

When the specified key is a modifier key, the key is pressed but is not
released until after the next nonmodifier key (or button) is pressed.  This
is called "sticky" modifiers.  For example:

	add-word /t "[meta key]" "{c-t}Meta_L{c-t}" left right

would set the Meta modifier if a key labeled with Meta_L is attached to a
modifier.  So, you could say "[meta key]" "x" to generate meta-x.  However,
since there are both Meta_L and Meta_R keysyms, and since Meta is commonly
used, there is a special control sequence for this:

	{c-t}{c-m}

So, more commonly you would specify:

	add-word /t "[meta key]" "{c-t}{c-m}" left right
	add-word /t "[meta x]" "{c-t}{c-m}x" left right

Similarly, there is a special control sequence for the Shift modifier:

	{c-t}{c-s}

and a special control sequence for the Control modifier:

	{c-t}{c-c}

The Shift control sequence is not normally needed in conjunction with
character keys, but is useful with function keys.  For example:

	add-word /t "[dismiss window]" "{c-t}{c-c}{c-t}{c-s}{c-t}{c-m}{c-t}F3{c-t}" invisible

generates control-shift-meta-F3, which might, e.g., be bound in a .twmrc
as:

	"F3" = c|s|m : window	: f.delete

The Control sequence would not normally be needed in conjunction with
character keys, since DragonDictate can output control characters, but the
edit mechanism DragonDictate uses has problems with control characters.
Specifically, when DragonDictate generates backspaces as a result of
"choose <n>" or "[scratch that]", it will not generate backspaces for
control characters.  This is a problem, because it prevents a2x from
properly undoing control characters.  For example, if you defined:

	add-word /t "[move right]" "{c-f}" invisible

in order to use emacs-style positioning control, then a2x would be unable
to provide the undo help of generating {c-b} to compensate when
DragonDictate mistakenly matched this voice command, because DragonDictate
would not output any backspaces.  Instead, you should define:

	add-word /t "[move right]" "{c-t}{c-c}f" invisible

This way, DragonDictate will output a backspace for correction, and a2x
will recognize that a control sequence is attached to the 'f' character and
treat the sequence as a single entity for undo purposes.  Of course, if no
undo action is desired for control characters, you can represent them
directly.  For example:

	add-word /t "[move end]" "{c-e}" invisible

is reasonable for emacs-style positioning control because there is normally
no way to return to the original position.  You also don't need to expand
control characters if there are noncontrol characters in the sequence.  For
example:

	add-word /t "[split window]" "{c-x}2" invisible

DragonDictate will output a backspace for the '2', so the {c-x} does not
need to be expanded to "{c-t}{c-c}x" (although it doesn't hurt).


				Autorepeat

It is convenient at times to be able to autorepeat a keystroke, e.g., when
using incremental search in emacs or when paging through a document.  To
autorepeat the last keystroke generated by a2x, use the control sequence:

	{c-t}{c-a}<kdelay> 0 0{c-t}

where <kdelay> is a floating point number specifying the rate in seconds.
For example:

	add-word /t "[autorepeat it]" "{c-t}{c-a}1.5 0 0{c-t}" invisible

will autorepeat every 1.5 seconds.  So, you might have defined:

	add-word /t "[search for]" "{c-s}" left right

for incremental search in emacs.  You can say "[search for]" to start a
search, say what you are searching for, say "[search for]" again, and then
say "[autorepeat it]" to autorepeat the search.

When autorepeat is in progress, it continues until you explicitly stop it
or until the next keystroke is generated through a2x.  You can explicitly
stop autorepeat with the control sequence:

	{c-t}{c-q}

For example:

	add-word /t "[stop moving]" "{c-t}{c-q}" invisible

The autorepeat control sequence also controls pointer motion; the two zeros
given in the keyboard autorepeat sequence can be replaced with other
values, as described further below.


			      Button Control

To toggle the state of a button, use the control sequence:

	{c-t}{c-b}<button>{c-t}

where <button> is the decimal button number.  For example:

	add-word /t "[click it]" "{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}" invisible
	add-word /t "[double click]" "{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}" invisible

will single-click and double-click button 1.

To release all buttons that are pressed, use the control sequence:

	{c-t}{c-b}0{c-t}

For example:

	add-word /t "[press 1]" "{c-t}{c-b}1{c-t}" invisible
	add-word /t "[button up]" "{c-t}{c-b}0{c-t}" invisible

so that you can say "[press 1]" to press button 1, move the pointer to a
final position, and then say "[button up]" to release the button.

Modifiers can be set for buttons, for example:

	add-word /t "[move window]" "{c-t}{c-m}{c-t}{c-b}3{c-t}" invisible

generates meta-button-3.


			      Pointer Motion

To warp the pointer to a specific location on a screen, use the control
sequence:

	{c-t}{c-w}<screen> <x> <y>{c-t}

where <screen> is either a decimal screen number (starting from 0), or -1
to position on whatever screen the pointer is currently on.  For example:

	add-word /t "[go to icons]" "{c-t}{c-w}0 478 235{c-t}" invisible

Moving to an absolute position is not really very useful.  More useful is
to be able to "remember" some position on the screen, usually relative to
some window, and then go back to it.  The current pointer location can be
saved using the control sequence:

	{c-t}{c-l}s<digit>{c-t}

where <digit> is a decimal digit (0-9).  This control sequence finds the
client window (a window with a WM_STATE property) the pointer is in, or if
the pointer is not in a client window, the outermost window the pointer is
in (which might be the root window).  It records the pointer position
relative to the origin of the selected window in one of 10 variables
selected by <digit>.  You can then later warp the pointer back to this
location with the control sequence:

	{c-t}{c-l}w<digit>{c-t}

If the selected window has moved on the screen, the pointer will still warp
to the correct position relative to its new origin.  Example voice macros
using these control sequences:

	add-word /t "[set location]" "{c-t}{c-l}s0{c-t}" invisible
	add-word /t "[go back]" "{c-t}{c-l}w0{c-t}" invisible

Other a2x mechanisms for warping the pointer to specific windows are
described further below.

To warp the pointer relative to its current position, use the control
sequence:

	{c-t}{c-d}<dx> <dy>{c-t}

where <dx> and <dy> are decimal numbers (with an optional minus sign) for
the change in x and y.  For example:

	add-word /t "[nudge down]" "{c-t}{c-d}0 2{c-t}" invisible
	add-word /t "[nudge left]" "{c-t}{c-d}-2 0{c-t}" invisible
	add-word /t "[nudge right]" "{c-t}{c-d}2 0{c-t}" invisible
	add-word /t "[nudge up]" "{c-t}{c-d}0 -2{c-t}" invisible

More useful is to use relative motion in conjunction with autorepeat to
move the pointer at a continuous rate.  The autorepeat control sequence
described earlier can also be used for pointer motion:

	{c-t}{c-a}<kdelay> <delta> <pdelay>{c-t}

where <kdelay> is the keyboard autorepeat rate in seconds (as before),
<delta> is a nonnegative integer specifying a new distance, and <pdelay> is
the pointer autorepeat rate in seconds.  Both <kdelay> and <pdelay> are
floating point numbers.  The <kdelay> value is only used if the event being
autorepeated is a keystroke; the <delta> and <pdelay> values are only used
if the event being autorepeated is pointer motion.  When any value is 0, it
means "don't change the value from what it already is".  If a nonzero
<delta> value is specified, it becomes the new magnitude of motion in both
x and y dimensions.  For example, if the current motion is 5 pixels in the
x dimension, and -5 pixels in the y dimension, and a <delta> of 7 is given,
the new motion will be 7 pixels in the x dimension and -7 pixels in the y
dimension.  Note that, although you can initially specify motion along an
angle that is not a multiple of 45 degrees, if you specify <delta> in the
autorepeat you will end up with an angle that is a multiple of 45 degrees.

Examples of using autorepeat:

	add-word /t "[go down right]" "{c-t}{c-d}5 5{c-t}{c-t}{c-a}0 0 .1{c-t}" invisible
	add-word /t "[go up slow]" "{c-t}{c-d}0 -1{c-t}{c-t}{c-a}0 0 .1{c-t}" invisible
	add-word /t "[go left]" "{c-t}{c-d}-5 0{c-t}{c-t}{c-a}0 0 .1{c-t}" invisible
	add-word /t "[autorepeat]" "{c-t}{c-a}1 5 .1{c-t}" invisible
	add-word /t "[slow down]" "{c-t}{c-a}2 1 0{c-t}" invisible

The autorepeat sequence is designed to cover both keystrokes and motion so
that you can use a single voice command to govern speed.  For example, you
might say "[search for]", "[autorepeat]", then "[slow down]" if things are
going by too fast, and finally "[stop moving]".  Similarly, you might say
"[go left]", then "[slow down]" when you get close, and finally "[stop
moving]".  Notice that "[slow down]" in the example treats keyboard repeat
differently from pointer motion; the repeat rate for the keyboard is slowed
down, but the repeat rate for pointer motion remains the same and the delta
motion is made smaller instead.  This permits finer-grained positioning of
the pointer.


				   Undo

The normal mode for using DragonDictate with X is to have DragonDictate's
"keys-immediate" parameter set to 1, so that actions happen immediately
without confirmation.  But, when DragonDictate matches on the wrong
utterance, the wrong action is executed.  Although DragonDictate will
generate backspaces when you use "choose <n>" to make corrections, these
backspaces seldom correctly undo the action that was executed.  There is no
perfect solution to this problem, but a2x has an undo facility that can
help in many cases.

When a2x starts up, it reads an undo file.  By default, it reads the file
".a2x" from your home directory, but you can supply an alternative file on
the command line using the "-u" option.  The syntax of this file is simple;
each line is of the form:

<dd-sequence>:<undo-sequence>

The <dd-sequence> is the sequence that is being undone; the <undo-sequence>
is the sequence that a2x should generate to "undo" that sequence.  Each
sequence is specified as a string of characters.  Control characters are
specified as '^' followed by a single character, e.g., "^T" for {c-t}.
Comments can be inserted in the file as lines beginning with '!'.

When a2x receives backspaces, it searches the undo file for the longest
<dd-sequence> that generates that many backspaces, and then processes the
corresponding <undo-sequence> just as if it had come from DragonDictate.

As a simple example, the undo entries for emacs-style forward/backward
character motion ({c-f} and {c-b}) are:

	^T^Cb:^T^Cf
	^T^Cf:^T^Cb

Note that these are specified for undoing "{c-t}{c-c}b" and "{c-t}{c-c}f",
not for {c-b} and {c-f}.  This is because {c-b} and {c-f} will not have
backspaces generated for them by DragonDictate (refer to the backspace
discussion earlier).

While you are building up your voice macros for a2x, you will probably be
editing your undo file from time to time.  To tell a2x to reload the file,
use the control sequence:

	{c-t}{c-u}

For example:

	add-word /t "[reload undo]" "{c-t}{c-u}" invisible


			  Moving Between Windows

To warp the pointer to a new window, use the control sequence:

	{c-t}{c-j}<options>{c-t}

or
	{c-t}{c-j}<options> <mult>{c-t}

where <mult> is a positive floating point number, and <options> consists of
some subset of the following options:

	Z  (this is optional)
	one of {C, D, L, R, U} or {c, d, l, r, u}  (one is required)
	O  (this is optional, only useful with {C, D, L, R, U})
	one or both of {k, b}  (these are optional)
	one of {n, p, N, P}  (this is optional, and must be last)

The options are defined as follows:

Z		no-op

This letter is a "no-op"; it is provided so that you can place it at the
beginning of the control sequence to "soak up" an uppercase conversion
performed by DragonDictate as a result of a previous utterance.  For
example, you might just have finished dictating a sentence, and then issue
a command to change windows; the first option letter will be converted to
uppercase.  This option is not normally needed if you set the punctuation
of your macro to "invisible".

C		closest top-level window
D		top-level window going down
U		top-level window going up
L		top-level window going left
R		top-level window going right
O		skip overlapping top-level windows

These options permit you to jump up ('U'), down ('D'), right ('R'), left
('L'), or in any direction ('C') to a new top-level window closest (in the
direction of travel) to the current pointer position.  'O' can be used in
conjunction with one of the other options to ignore all windows that
overlap the top-level window the pointer is in.

For example:

	add-word /t "[skip down]" "{c-t}{c-j}D{c-t}" invisible
	add-word /t "[skip left]" "{c-t}{c-j}L{c-t}" invisible
	add-word /t "[way right]" "{c-t}{c-j}RO{c-t}" invisible
	add-word /t "[way up]" "{c-t}{c-j}UO{c-t}" invisible

c		closest widget
d		widget going down
l		widget going left
r		widget going right
u		widget going up

These options permit you to jump to a new widget within the current
top-level window that the pointer is in.  The direction of motion is the
same as for the top-level jump options.  For example:

	add-word /t "[jump close]" "{c-t}{c-j}c{c-t}" invisible
	add-word /t "[jump down]" "{c-t}{c-j}d{c-t}" invisible

The distance between a window and the pointer position is computed by using
the closest visible point of the window.  When computing the distance
metric, a2x employs a scale factor in the "off axis" to skew the
computation, which favors windows farther away but in the desired direction
over windows closer but in the wrong direction.  For example, if you are
moving in the "up" direction, the distance between two points is computed
as:

square_root((x2 - x1) * (x2 - x1) * multiplier + (y2 - y1) * (y2 - y1))

The default multiplier is 10.  You can override this default by explicitly
providing a <mult> floating point value in the jump control sequence.

k		require keyboard interest
b		require button interest

It is often useful to restrict the set of windows to those that select for
keyboard and/or button events.  Specifying 'k' requires that the window
select for KeyPress or KeyRelease events.  Specifying 'b' requires that the
window select for ButtonPress or ButtonRelease events.  Specifying both 'k'
and 'b' requires that the window select for KeyPress or KeyRelease or
ButtonPress or ButtonRelease (i.e., "kb" means 'k' or 'b', not 'k' and
'b').  For example, when jumping between widgets it is usually more useful
to require some form of input:

	add-word /t "[jump left]" "{c-t}{c-j}lkb{c-t}" invisible
	add-word /t "[jump right]" "{c-t}{c-j}rkb{c-t}" invisible

n<name>			require WM_NAME name
p<name>			require WM_NAME name prefix
N<name>			require name
N.<class>		require class
N<name>.<class>		require name and class
P<name>			require name prefix
P.<class>		require class prefix
P<name>.<class>		require name prefix and class prefix

These options further restrict the set of windows considered to those with
a particular name and/or class.  The 'n' and 'p' options are only useful
when jumping to top-level windows; the 'N' and 'P' options are useful for
all jumps.  If one of these options is used, it must be the last option.

The 'n' and 'p' options look at the WM_NAME property on the top-level
window.  The <name> must match exactly (for 'n') or be a prefix of (for
'p') the WM_NAME string value for the window to be considered.  The 'N' and
'P' options look at both the WM_CLASS property of the window and at the
_MIT_OBJ_CLASS property of the window.  Both of these properties contain a
name/class pair.  The <name>, if specified, must match exactly (for 'N') or
be a prefix of (for 'P') the name of the window, and the <class>, if
specified, must match exactly (for 'N') or be a prefix of (for 'P') the
class of the window, for a window to be considered.

In case the <name> you want to specify has a '.' in it, a2x uses the last
'.' as the separator between <name> and <class>, so if you have no <class>
to provide, simply terminate the <name> with another '.'.  For example:

	nxterm-expo.lcs.mit.edu.

The _MIT_OBJ_CLASS property is set by a nonstandard addition to the Xt
Intrinsics.  If the application resource "xtIdentifyWindows" is set to
true, then every widget created will have this property placed on it
automatically by the Intrinsics.  (You need R5 with public patches at least
up through 16 for Xt to generate this property; earlier versions of Xt
either do not have this support or generate a WM_CLASS instead, which
clobbers the real WM_CLASS on shell windows.)

For example:

	add-word /t "[start jump]" "{c-t}{c-j}ZcP" left right
	add-word /t "[start skip]" "{c-t}{c-j}ZCp" left right
	add-word /t "[go there]" "{c-t}" invisible

With these macros, you can say sequences like "[start skip]", "emacs", "[go
there]", to jump to the closest emacs application window, or "[start
jump]", "inbox", "[go there]" within an xmh application to jump to the
folder button named "inbox".  Note that it is OK to split a control
sequence across utterances, a2x cannot tell the difference.  If you make a
mistake saying the name, you can use "choose <n>" to make corrections.


				 Triggers

Consider the following:

	add-word /t "[bad reply to]" "{c-t}F17{c-t}{c-t}{c-j}Cpdrafts:{c-t}" capitalize left right

Assume that F17 is sent to an xmh application to start a reply to a
message.  It will take xmh some time to create the window, and it will take
additional time for the window manager to decorate the window and make it
visible.  If the jump control sequence is executed too soon, it will fail
to jump to the desired window.  It would be inconvenient to have to guess
how long this will take.  Instead, the a2x trigger mechanism can be used to
delay until the desired window is mapped.

To establish a trigger for a window mapping, use the control sequence:

	{c-t}{c-y}M<option>{c-t}

To establish a trigger for a window unmapping, use the control sequence:

	{c-t}{c-y}U<option>{c-t}

In both cases, the available options are:

n<name>			require WM_NAME name
p<name>			require WM_NAME name prefix
N<name>			require name
N.<class>		require class
N<name>.<class>		require name and class
P<name>			require name prefix
P.<class>		require class prefix
P<name>.<class>		require name prefix and class prefix

These options have the same interpretation as in the jump control sequence.
If no option is supplied, any window that is mapped/unmapped will satisfy
the trigger.

To establish a trigger for a selection ownership, use the control sequence:

	{c-t}{c-y}S<selection>{c-t}

where <selection> is the name of the selection (e.g., PRIMARY).  a2x will
take ownership of the specified selection; the trigger will be satisfied
when a2x loses ownership (usually when some other client takes ownership).

Once a trigger has been set, you can delay until the trigger is satisfied
(or 10 seconds elapse) with the control sequence:

	{c-t}{c-y}W{c-t}

You can override the default timeout value by instead using the control
sequence:

	{c-t}{c-y}W <delay>{c-t}

where <delay> is a floating point number in seconds.

Only one trigger can be outstanding at one time.  You should set the
trigger before you execute an action that will cause the trigger to be
satisfied.  For example:

	add-word /t "[reply to]" "{c-t}{c-y}Mpdrafts:{c-t}{c-t}F17{c-t}{c-t}{c-y}W{c-t}{c-t}{c-j}Cpdrafts:{c-t}" capitalize left right

As a more complex example, consider searching for the letter '@' in an
Athena Text widget and then selecting text surrounding it.  The search
command will pop up a search dialogue box, and you must wait for the box to
appear before entering the '@'.  After entering the Return key at the box,
you must wait for the box to pop down before generating further commands to
the text widget (or else they would be executed by the search box).  For
example:

	add-word /t "[select address]" "{c-t}{c-y}Mnsearch{c-t}{c-s}{c-t}{c-y}W{c-t}@{c-t}{c-y}Unsearch{c-t}{Enter}{c-t}{c-y}W{c-t}{c-t}{c-m}@" invisible

This sets a map trigger before typing {c-s}, waits for the map before
entering the '@', sets an unmap trigger before typing Return, then waits
for the unmap before selecting text (assume that {c-t}{c-m}@ does this).

Another example:

	add-word /t "[bad paste single]" "{c-@}{c-e}{c-t}{c-m}{c-w}{c-t}{c-j}Cpdrafts:{c-t}{c-t}{c-m}{c-y}" invisible

This macro selects text in the current window (assume that {c-@} sets a
mark at the current text position, {c-e} moves to the end of the text line,
and {c-t}{c-m}{c-w} selects the text between the mark and the current text
position), then jumps to a top-level window with "drafts:" as a name
prefix, and then pastes the selection into that window (assume that
{c-t}{c-m}{c-y} causes a paste).  Unfortunately, if you try this, you
probably discover that it doesn't work correctly.  The problem is that it
takes an application a while to actually assert a selection in X, and a2x
can usually jump to the new window and cause the next application to ask
for the selection faster than the first application can assert the
selection.  The result is that you will often get the previous selection
contents, rather than the expected contents.  So you need to wait for the
application to take ownership of the selection.  For example:

	add-word /t "[paste single]" "{c-@}{c-e}{c-t}{c-y}SPRIMARY{c-t}{c-t}{c-m}{c-w}{c-t}{c-j}Cpdrafts:{c-t}{c-t}{c-y}W{c-t}{c-t}{c-m}{c-y}" invisible


				  Delays

Sometimes the trigger mechanism may not be sufficient for your needs.  You
can also simply delay for a fixed period of time, using the control
sequence:

	{c-t}{c-z}<delay>{c-t}

where <delay> is a floating point number specifying the amount of time in
seconds a2x should wait before generating more events.  For example:

	add-word /t "[timed click]" "{c-t}{c-b}1{c-t}{c-t}{c-z}.3{c-t}{c-t}{c-b}1{c-t}" invisible


				  Macros

There are occasions when you want to repeat some sequence of actions
multiple times, but it isn't worth creating a permanent DragonDictate voice
macro because you are unlikely to need the sequence again.  If all of the
actions occur within one window, and that application supports a macro
record/playback mechanism, you can use that.  But if the actions span
multiple windows or applications, you can use a2x to record and playback
sequences.

To start recording a macro, use the control sequence:

	{c-t}{c-f}r{c-t}

For example:

	add-word /t "[record macro]" "{c-t}{c-f}r{c-t}" invisible

Then, speak all of the actions you wish to record.  When you have finished,
stop recording and save the macro with the control sequence:

	{c-t}{c-f}s<digit>{c-t}

where <digit> is a decimal digit (0-9), permitting you to save up to 10
different recorded macros.  For example:

	add-word /t "[save macro]" "{c-t}{c-f}s0{c-t}" invisible

To execute a recorded macro, use the control sequence:

	{c-t}{c-f}e<digit>{c-t}

where <digit> is the number of the macro you want to execute.  For example:

	add-word /t "[execute macro]" "{c-t}{c-f}e0{c-t}" invisible

Once you are through with a macro, you can delete it (to avoid accidental
execution if DragonDictate matches wrong) with the control sequence:

	{c-t}{c-f}d<digit>{c-t}

For example:

	add-word /t "[delete macro]" "{c-t}{c-f}d0{c-t}" invisible


			       Server Grabs

If an application grabs exclusive control of the server (using the Xlib
function XGrabServer), a2x will not be able to function; protocol requests
from a2x will be suspended just as other X clients will have their requests
suspended.  This may be a problem, depending on whether the applications
you use grab the server during the time you need to generate input.  For
example, many window managers will grab the server when they resize or move
windows, or perhaps even when displaying menus.  In that case, you will be
unable to use voice control for those operations.  Some applications may
provide options to disable server grabs; for example, twm provides the
NoGrabServer option to not use server grabs for menus or opaque window
moves, and olwm provides a "ServerGrabs" boolean resource for menus and
notices.


			    Changing Displays

If you use more than one display in your work, you can dynamically point
a2x at whatever display you want to interact with, using the control
sequence:

	{c-t}{c-r}D<display>{c-t}

where <display> is standard display name string or just a host name.  If
the string does not contain a colon (:), ":0" is appended to the string.
The string can be empty, in which case it is equivalent to ":0".  For
example:

	add-word /t "[start display]" "{c-t}{c-r}D" left right
	add-word /t "[go there]" "{c-t}" invisible

permits you to say "[start display]", "expo", "[go there]" to go to the
display "expo:0".  Note that in this example, the terminating {c-t} of the
control sequence is in a separate voice macro, and is part of a separate
utterance.  This is fine; a2x does not know where utterance boundaries are,
it simply interprets a stream of characters.  If you make a mistake saying
the display name, you can use "choose <n>" to make corrections.


			      Miscellaneous

You can terminate a2x using the control sequence:

	{c-t}{c-e}

For example:

	add-word /t "[dragon quit]" "{c-t}{c-e}{Dcom}dkey {Enter}{Dcom}" invisible

(The "dkey" dcom simply forces the DragonDictate menu to pop down.)  If a2x
seems to be doing strange things, you can dump recent history of characters
received using the control sequence:

	{c-t}{c-p}

This will print out on your PC screen.

You can generate a {c-t} with the control sequence:

	{c-t}{c-t}

or with:

	{c-t}{c-c}t


				  Emacs

You may find the following GNU emacs 18 functions useful for manipulating
selections through the keyboard:

(defun dragon-cut-text (beg end)
  "Copy text between mark and position into window system cut buffer.
Save in Emacs kill ring also."
  (interactive "r")
  (x-store-cut-buffer (buffer-substring beg end))
  (copy-region-as-kill beg end))

(defun dragon-paste-text ()
  "Insert window system cut buffer contents at cursor."
  (interactive)
  (insert (x-get-cut-buffer)))

For example:

(global-set-key "\e\C-w" 'dragon-cut-text)
(global-set-key "\e\C-y" 'dragon-paste-text)


			    Athena Text Widget

If you frequently use programs that use the Athena Text widget, you will
probably find it useful to manipulate selections through the keyboard.
Unfortunately, the Text widget up through R5 does not provide this support.
A patch for the R5 Text widget is available with the a2x distribution.
With this patch, you can use translations such as:

*Text.translations: #override\n\
	!:Ctrl<Key>@: select-start()\n\
	!:Ctrl<Key>space: select-start()\n\
	!:Meta<Key>@: select-word(PRIMARY, CUT_BUFFER0)\n\
	!:Ctrl<Key>_: insert-selection(CUT_BUFFER1)\n\
	!:Ctrl<Key>w: extend-end(PRIMARY, CUT_BUFFER0)kill-selection() \n\
	!:Meta Ctrl<Key>w: extend-end(PRIMARY, CUT_BUFFER0)\n\
	!:Meta Ctrl<Key>y: insert-selection(PRIMARY, CUT_BUFFER0)


				DESQview/X

If you have DESQview/X from Quarterdeck Office Systems, it is possible to
export the DragonDictate menus onto your X display.  This means you don't
have to look at another screen to make corrections, but at the cost of
dedicating some screen real estate to a window for menus.  You may also
find the menu display slower going across the network.

[If you are trying to use DESQview/X with FTP Software's version 2.1, you
may have trouble getting it configured.  You need to have IPCUST.SYS in
your CONFIG.SYS for compatibility:
	DEVICE=C:\PCTCP\IPCUST.SYS
and you need to do a one-time "install" step:
	CD \PCTCP
	IPCONFIG IPCUST.SYS HOSTNAME <pc-hostname>
	IPCONFIG IPCUST.SYS USERID <your-userid>
Reboot, and run DESQview/X's Setup again to configure the network.]

To accomplish this, you start a DOS window on your X display, then start up
DragonDictate in that DOS window.  If you have a version of a2x compiled to
run on your PC, you can then run that a2x in the DOS window; otherwise, you
telnet to your workstation (from within the DOS window) and then run the
"DragonDictate" script.

To be able to run DragonDictate in a DOS window, you must permit lots of
Expanded Memory to be used.  Use DVPMAN on \DVX\DVPS\DOS.DVP, and under the
Advanced Options, make the field for "Maximum Expanded Memory Size" blank.

To open a remote DOS window, you can type:

	run dos -display <display-spec>

in a local DOS window, or else type:

	rsh <PC-hostname> dos

from a shell on your X display.  Alternatively, you can create a fancier
.DVP to run.  For example, in the \DVP\DVPS\ directory, copy DOS.DVP to
A2X.DVP.  Then use DVPMAN to edit this file.  Change the "Program Name" to
"a2x", and change the "DOS Command" to:

	a2xini -display <display-spec>

In the Advanced Options, make the field for "Maximum Expanded Memory Size"
blank, and clear the toggle for "Wait for Window to Open".  Now create a
file A2XINI.BAT, containing something like:

	CD \DICTATE\USERS		(or wherever your voice files are)
	CALL DT /s <your-name>
	TELNET <workstation-hostname>

if you use telnet, or:

	CD \DICTATE\USERS		(or wherever your voice files are)
	CALL DT /s <your-name>
	A2X -display <display-spec>

if you are running a2x directly on the PC.  Place this program in the
DESQview/X "Open Window" menu, calling it "a2x", and then you can start up
DragonDictate and connect to your workstation simply by clicking on the
menu entry when DESQview/X starts up.

Running DragonDictate under DESQview/X is a somewhat fragile setup.  Most
important, once you have started DragonDictate in one DOS window, do not
try to start up DragonDictate again, or your PC will hang.  You will have
to reboot before starting up DragonDictate again.

DESQview/X looks for the font "pc8x16" to use in the DOS window.  The
BDF form of this font comes with DESQview/X, in \DVX\BDF\.  You may
want to use pc8x14 or pc8x8 instead, to take up less space on your
screen.  In that case, you will want to create an X font alias to map
the name pc8x16 to the font you prefer.

There is no good way through DESQview/X to control the initial size and
position of the DOS window on your X display.  (Actually, you can control
the initial size by changing the Maximum Width and Maximum Height in the
.DVP file, but if you do this DragonDictate's menus will come out
scrambled.)  For this reason, a2x has command line options to permit you to
reconfigure the DOS window:

	-w <DOS-window-name> -g <geometry-spec>

The <DOS-window-name> must be the WM_NAME of the DOS window (usually
displayed by the window manager in the title bar).  For example, if you
opened a standard DOS window, the name would be "DOS Window".  If you
created an A2X.DVP and assigned it the Program Name "a2x", then the name
will be "a2x".  You want this name to be unique across all windows on your
display.  The <geometry-spec> should be a complete geometry specification,
with x, y, width and height all specified; the width and height are numbers
of characters.  To contain full DragonDictate dialogue boxes, you will need
a width of at least 56 and a height of at least 19.

You will need a key on your X keyboard to act as the DragonDictate HOTKEY.
By default, DragonDictate uses the keypad '+' key, which is the X keysym
KP_Add.  Make sure you have a key with the appropriate keysym.  You may
find it inconvenient that the HOTKEY only works when the input focus is in
the DOS window; specifically, it is difficult to quickly turn off the
microphone.  For convenience, you can tell a2x to grab the HOTKEY away from
all other applications, so that it always functions as the HOTKEY
regardless where the input focus is.  To do this, specify the command line
options:

	-w <DOS-window-name> -h <HOTKEY-keysym>

If you are using the "DragonDictate" script, you can pass command line
options to a2x by providing them as options to the script, or you can
preset the A2XOPTS environment variable to the options you want.  For
example:

	setenv A2XOPTS "-w a2x -h KP_Add -g 58x18+1+1"

With a2x running in a DOS window on your X screen, you need to be careful
about speaking and typing when the input focus is in the DOS window and no
DragonDictate menu or dialogue box is up.  Any character you type will be
sent by DragonDictate to a2x, a2x will generate an event that is received
by the DOS window, DragonDictate will get the character again, and send it
on to a2x again, and the character will continue to infinite loop until you
move the input focus out of the DOS window.  Fortunately, nothing really
bad happens, you just drive the load average up on your machine.  If you
speak when menus are down and the input focus is in the DOS window, the
characters generated by a2x will be directed back to the DOS window, and
DragonDictate will enter them as corrections to the word choice.

