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

>># Common definitions 
>># Used by:
>>#	XrmGetResource 
>>#	XrmQGetResource.
>>EXTERN

#define	XGR_T1_DATA	13
static char *t1_data[XGR_T1_DATA][3] = {
/* Specifier, type, value */
	{	"a.b.c",	"String",	"one"	},
	{	"a.z",	"String",	"two"	},
	{	"a*z",	"String",	"three"	},
	{	"b.d.z",	"String",	"four"	},
	{	"b.d.Z",	"String",	"five"	},
	{	"c*z",	"String",	"six"	},
	{	"c.a.z",	"String",	"seven"	},
	{	"c.A.z",	"String",	"eight"	},
	{	".d.z",	"String",	"nine"	},
	{	"*d.z",	"String",	"ten"	},
	{	"e.d.z",	"String",	"eleven"	},
	{	"*f*z",	"String",	"twelve"	},
	{	"*g*z",	"String",	"thriteen"	} };

#define	XGR_T1_TEST	8
static char *t1_test[XGR_T1_TEST][5] = {
/* Full Name, Full Class, Expected Type, Expected Value, Failure message */
	{	"a.b.c",	"I.J.K",	"String",	"one", "Simple match failed" },
	{	"a.z",	"I.K",	"String",	"two", "period not more specific than asterisk" },
	{	"b.d.z",	"B.D.Z",	"String",	"four", "name was not more specific than class" },
	{	"c.a.z",	"I.J.K", "String",	"seven",	"specifying a name was not more specific than ommiting one" },
	{	"c.z.z",	"C.A.Z",	"String",	"eight",	"specifying a class was not more specific than ommiting one" },
	{	"e.d.z",	"I.J.K",	"String",	"eleven",	"left components were not more specific than right components" },
	{	"f.g.z",	"I.J.K",	"String",	"twelve",	"left components were not more specific than right components" },
	{	"d.z",	"I.J",	"String",	"nine",	"absent prefix not identical to a period"} };

#define XGR_T2_TEST 3
static char *t2_test[XGR_T2_TEST][3] = {
	{ "a.b", "A.B", "Name and class didn't match an entry" },
	{ "A.B", "a,b", "Name and class didn't match an entry" },
	{ "q",	"Q",	"Non-existant database entry matched." } };

