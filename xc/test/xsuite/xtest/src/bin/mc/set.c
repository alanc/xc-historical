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
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#include	<stdio.h>
#include	<string.h>
#include	"mc.h"

struct	settings Settings;

setcmd(bp)
char	*bp;
{
char	*tok;
char	*arg1;

	(void) strtok(bp, SEPS);

	tok = strtok((char *)0, SEPS);
	arg1 = strtok((char *)0, SEPS);
	if (arg1)
		arg1 = mcstrdup(arg1);

	if (strcmp(tok, "startup") == 0) {
		Settings.startup = arg1;
	} else if (strcmp(tok, "cleanup") == 0) {
		Settings.cleanup = arg1;
	} else if (strcmp(tok, "tpstartup") == 0) {
		Settings.tpstartup = arg1;
	} else if (strcmp(tok, "tpcleanup") == 0) {
		Settings.tpcleanup = arg1;
	} else if (strcmp(tok, "need-gc-flush") == 0) {
		Settings.needgcflush = 1;
	} else if (strcmp(tok, "fail-return") == 0) {
		Settings.failreturn = 1;
	} else if (strcmp(tok, "fail-no-return") == 0) {
		Settings.failreturn = 0;
	} else if (strcmp(tok, "return-value") == 0) {
		Settings.valreturn = arg1;
	} else if (strcmp(tok, "macro") == 0) {
		Settings.macro = 1;
		if (arg1)
			Settings.macroname = arg1;
	} else if (strcmp(tok, "begin-function") == 0) {
		Settings.beginfunc = arg1;
	} else if (strcmp(tok, "end-function") == 0) {
		Settings.endfunc = arg1;
	} else if (strcmp(tok, "no-error-status-check") == 0) {
		Settings.noerrcheck = 1;
	} else if (strcmp(tok, "error-status-check") == 0) {
		Settings.noerrcheck = 0;
	} else {
		err("Unrecognised set option\n");
		errexit();
	}

}

