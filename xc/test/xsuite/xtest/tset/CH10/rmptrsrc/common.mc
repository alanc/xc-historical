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

>># shared resource manager test declarations.
>># Used by:
>>#	XrmPutResource
>>#	XrmPutStringResource
>>#	XrmQPutResource
>>#	XrmQPutStringResource

>># include common functions
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN

#define XRM_T1_TESTS	4

static char *t1_specifiers[XRM_T1_TESTS] = {
	"opus.hodgepodge.portnoy",
	"calvin*and.hobbes",
	"*The.Far*Side",
	"Cutter.John.was*here" };

static char *qt1_specifiers[XRM_T1_TESTS][5] = {
	{ "opus" , "hodgepodge", "portnoy" , (char *)NULL, (char *)NULL },
	{ "calvin", "and", "hobbes", (char *)NULL, (char *)NULL },
	{ "The", "Far", "Side" , (char *)NULL, (char *)NULL },
	{ "Cutter", "John", "was", "here", (char *)NULL } };

static XrmBinding qt1_bindings[XRM_T1_TESTS][4] = {
	{ XrmBindTightly, XrmBindTightly, XrmBindTightly },
	{ XrmBindTightly, XrmBindLoosely, XrmBindTightly },
	{ XrmBindLoosely, XrmBindTightly, XrmBindLoosely },
	{ XrmBindTightly, XrmBindTightly, XrmBindTightly, XrmBindLoosely } };

static char *t1_fspecs[XRM_T1_TESTS] = {
	"opus.hodgepodge.portnoy",
	"calvin.and.hobbes",
	"the.far.side",
	"cutter.john.was.here" };

static char *t1_fclasses[XRM_T1_TESTS] = {
	"opus.hodgepodge.portnoy",
	"Boy.Thingy0.Tiger",
	"The.Far.Side",
	"Cutter.John.was.here" };

static char *t1_types[XRM_T1_TESTS] = {
	"String",
	"Thing",
	"Ping",
	"Bing" };

static char *t1_values[XRM_T1_TESTS] = {
	"Value One",
	"Value Two",
	"What we say to cats.",
	"NCC-1701a" };

static char *t2_specifiers[2] = {
	"A.b*C",
	"A.b*C" };

static char *qt2_specifier[] = {
	"A", "b", "C", (char *)NULL }; 

static XrmBinding qt2_bindings[] = {
	XrmBindTightly, XrmBindTightly, XrmBindLoosely };

static char *t2_fullspec  = "a.b.c";

static char *t2_fullclass = "A.B.C";

static char *t2_types[2] = {
	"TypeOne",
	"TypeTwo" };

static char *t2_values[2] = {
	"One",
	"Two" };
