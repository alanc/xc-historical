/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
>>TITLE SelectionRequest CH08
>>ASSERTION Good A
There are no assertions in sections 8.1-8.4 for xname events.
Assertions will be submitted from section 4 which will cover
xname event assertions.
>>STRATEGY
Put out a message explaining that there are no specific assertions for
xname events in sections 8.1-8.4, and that delivery of xname events will
be covered in section 4.
>>CODE

	report("There are no specific assertions for %s events in sections 8.1-8.4.", TestName);
	report("Delivery of %s events will be covered in section 4.", TestName);
	tet_result(TET_NOTINUSE);

>>#NOTEm >>ASSERTION
>>#NOTEm When a client requests a selection conversion by calling
>>#NOTEm .F XConvertSelection
>>#NOTEm and the specified selection owner is not
>>#NOTEm .S None ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M type
>>#NOTEs is set to
>>#NOTEs xname.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE The method of expansion is not clear.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M serial
>>#NOTEs is set
>>#NOTEs from the serial number reported in the protocol
>>#NOTEs but expanded from the 16-bit least-significant bits
>>#NOTEs to a full 32-bit value.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the event came from a
>>#NOTEm .S SendEvent
>>#NOTEm protocol request,
>>#NOTEm then
>>#NOTEm .M send_event
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event was not generated by a
>>#NOTEs .S SendEvent
>>#NOTEs protocol request,
>>#NOTEs then
>>#NOTEs .M send_event
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M display
>>#NOTEs is set to
>>#NOTEs a pointer to the display on which the event was read.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M owner
>>#NOTEs is set to
>>#NOTEs the window owning the selection.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M requestor
>>#NOTEs is set to
>>#NOTEs the window requesting the selection.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M selection
>>#NOTEs is set to
>>#NOTEs the atom that names the selection.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE
>>#NOTEs >>#NOTE What is meant by ``the type the selection is desired in''?
>>#NOTEs >>#NOTE
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M target
>>#NOTEs is set to
>>#NOTEs the atom that indicates the type
>>#NOTEs the selection is desired in.
>>#NOTEs >>#NOTE
>>#NOTEs >>#NOTE Testing strategy?
>>#NOTEs >>#NOTE
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M property
>>#NOTEs is set to
>>#NOTEs the atom that names a property name or
>>#NOTEs .S None .
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE
>>#NOTEs >>#NOTE This assertion might be incorrect.  It mimics the spec,
>>#NOTEs >>#NOTE but the result is rather ambiguous.
>>#NOTEs >>#NOTE Testing strategy?
>>#NOTEs >>#NOTE
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M time
>>#NOTEs is set to
>>#NOTEs the time or
>>#NOTEs .S CurrentTime .
