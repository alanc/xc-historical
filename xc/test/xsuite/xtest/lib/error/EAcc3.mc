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
>>ASSERTION Bad A
>>### Access colormap-store
When an attempt to store into a read-only or an unallocated colormap
entry is made, then a
.S BadAccess
error occurs.
>>STRATEGY
Note - no generic code can be written for this included assertion.
There should be a CODE section wherever it is included to override the 
default CODE section below.
>>CODE
report("No code written for this assertion.");
