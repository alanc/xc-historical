			     Getting Started

A2x is designed to interface DragonDictate to the X Window System.  A2x
converts the ASCII output of DragonDictate into X device events.  A2x is
capable of manipulating both the keyboard and the pointer; depending on
the applications, it is possible to use X hands free through a2x.

A2x requires an X protocol extension to function.  Either the XTEST or
XTRAP extension can be used.  (There is code for the old XTESTEXT1
extension in the source, but it has never been tested and probably will
not work.)  The XTEST extension was developed by the X Consortium for use
in the X Test Suite.  It is not currently part of R5 or the Test Suite,
although it is expected that the X Consortium will release it prior to R6.
The XTRAP extension was developed by Digital Equipment Corp., and public
versions are available for both R4 and R5.  The XTEST extension is best to
use, but XTRAP is perfectly adequate.  (The main difference as far as a2x
is concerned is that XTEST permits time delays inside the X server, while
with XTRAP the delays are handled on the client side and hence can be
somewhat less accurate.)

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

The normal mode of operation is to use a telnet program on the PC, and
telnet across to your X workstation and log in.  (If you do not have a
network connection, it should be possible to use a terminal program and
log in to the workstation using a serial line.  However, performance over
a serial line will not be as good as over a network.)  In that telnet
session, run the script "DragonDictate" provided with the a2x
distribution.  This script simply clears the screen and starts up a2x.
Clearing the screen is a convenience so that the focus of attention on the
PC screen will be the DragonDictate menus.  (You can, of course, write
your own script or simply run a2x directly.)

You will probably find it convenient to force the DragonDictate menus to
appear in a fixed position on the PC screen.  For example, in the
"Parameters" menu from "Voice Console", set

	pop-up-row		0
	pop-up-col		30

You should also set

	pop-up-delay		0
	compatibility-delay	0

for quickest operation.

A2x takes each ASCII character produced by DragonDictate and determines a
key on the X keyboard that can be pressed and released to generate that
character.  If Shift and Control modifiers are necessary to generate the
character, a2x will automatically press and release keys for those
modifiers.  For example, for the character 'A', a2x will typically have to
press a Shift modifier key, press an 'a' key, release the 'a' key, and
then release the Shift modifier key.

It is not necessary to perform all of your "typing" through a2x; you can
mix a2x output with direct keyboard typing and direct pointer
manipulation.  But, you should normally only switch between direct input
and a2x input when all keys and buttons are in the released state, to
avoid confusing a2x.  (One simple exception to this rule is that you can
use a2x to press/release buttons and keys while directly moving the
pointer with your hand.)

A2x interprets the character {c-t} specially; it introduces a control
sequence for special a2x operations.  The character {c-t} was chosen
because it occurs infrequently in normal text; escape was not chosen
because you might use it frequently in voice macros to send escape
sequences to applications.  There are two types of a2x control sequences:
{c-t} by a single control character, and {c-t} followed by a string of
characters terminated by another {c-t}.  Control sequences are described
in detail below.

A2x also interprets the backspace character ({c-h} or {Backspace})
specially.  When you make corrections through DragonDictate, e.g., you use
"choose <n>", DragonDictate will output backspaces to "undo" the previous
choice before outputting the characters for the new choice (if any).  For
many X users, it is more convenient to receive delete (rubout) characters
instead of backspaces.  The default action of a2x is to convert backspace
to delete; if you do not want this behavior, specify "-b" on the a2x
command line.

A2x also interprets backspace characters specially in that it assumes
they are produced by DragonDictate to make corrections and are not
produced as normal text (i.e., that voice macros do not contain backspace
characters).  A2x looks at backspace characters in order to undo edits
(described in more detail below).  As such, it is recommended that you
change some predefined DragonDictate voice macros to replace backspace
characters with delete characters or with control sequences to generate
backspace.  For example, you should modify the keystrokes of the built-in
commands "[backspace]", "[back 1]", "[back 2]", "[back 3]", "[back 4]",
and "[back 5]", replacing each "{Backspace}" with "{Del}" to generate
delete instead of backspace.  If you really want to generate backspace,
then replace each "{Backspace}" with the sequence "{c-t}{c-c}h" instead.

			     Keyboard Control

To "type" a non-ASCII key on the keyboard, use the following control
sequence:

	{c-t}<keysym>{c-t}

where <keysym> is the string name of a keysym.  A2x will find a key on the
keyboard labeled with this keysym, and press and release that key.  For
example,

	add-word /t "[doit]" "{c-t}Execute{c-t}" invisible

defines a voice macro to press and release a key labeled with the
"Execute" keysym.  If for some reason your Xlib does not know the string
name of the keysym you want, you can specify the hexadecimal value of the
keysym instead.  For example,

	add-word /t "[doit]" "{c-t}FF62{c-t}" invisible

would also specify the Execute keysym.

When the specified key is a modifier key, the key is pressed but is not
released until after the next nonmodifier key (or button) is pressed.
This is called "sticky" modifiers.  For example,

	add-word /t "[meta key]" "{c-t}Meta_L{c-t}" left right

would set the Meta modifier if a key labeled with Meta_L is attached to a
modifier.  So, you could say "[meta key]" "x" to generate meta-x.
However, since there are both Meta_L and Meta_R keysyms, and since Meta is
commonly used, there is a special control sequence for this:

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

generates control-shift-meta-F3, which might, e.g., be bound in a .twmrc as:

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

This way, DragonDictate will output a backspace, and a2x will recognize
that a control sequence is attached to the 'f' character and treat the
sequence as a single entity for undo purposes.  Of course, if no undo
action is desired for control characters, you can represent them directly.
For example,

	add-word /t "[move end]" "{c-e}" invisible

is reasonable for emacs-style positioning control because there is
normally no way to return to the original position.

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

The autorepeat control sequence also controls pointer motion; the two
zeros given in the keyboard autorepeat sequence can be replaced with other
values, as described further below.


			     Pointer Control

To toggle the state of a button, use the control sequence:

	{c-t}{c-b}<button>{c-t}

where <button> is the decimal button number.  For example:

	add-word /t "[click it]" "{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}" invisible
	add-word /t "[double click]" "{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}{c-t}{c-b}1{c-t}" invisible

will single-click and double-click button 1.

To release all buttons that are pressed, use the control sequence:

	{c-t}{c-b}0{c-t}

For example,

	add-word /t "[press 1]" "{c-t}{c-b}1{c-t}" invisible
	add-word /t "[button up]" "{c-t}{c-b}0{c-t}" invisible

so that you can say "[press 1]" to press button 1, move the pointer to a
final position, and then say "[button up]" to release the button.

Modifiers can be set for buttons, for example:

	add-word /t "[move window]" "{c-t}{c-m}{c-t}{c-b}3{c-t}" invisible

generates meta-button-3.

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

If the selected window has moved on the screen, the pointer will still
warp to the correct position relative to its new origin.  Example voice
macros using these control sequences:

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
<delta> is a pointer motion substitution value, and <pdelay> is the
pointer autorepeat rate in seconds.



DragonDictate is a trademark of Dragon Systems, Inc.
X Window System is a trademark of the Massachusetts Institute of Technology
