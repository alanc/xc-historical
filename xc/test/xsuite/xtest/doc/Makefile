#
# Copyright 1990, 1991 by the Massachusetts Institute of Technology and
# UniSoft Group Limited.
# 
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation, and that the names of MIT and UniSoft not be
# used in advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.  MIT and UniSoft
# make no representations about the suitability of this software for any
# purpose.  It is provided "as is" without express or implied warranty.
#
# $XConsortium: Makefile,v 1.4 92/06/12 14:23:08 rws Exp $
#

# This Makefile can be used to print the documentation for the X test suite.
# - "make print00" formats and prints the Release Notes
# - "make print01" formats and prints the User Guide
# - "make print02" formats and prints the Programmers Guide 
# - "make printall" formats and prints all three documents
# You may need to alter the following variables first:

# This command expands .so directives to include the specified files.
SOELIM=soelim

# This command converts tbl requests to nroff format.
TBL=tbl 

MACROS=-mm

# This command formats text using nroff/troff with the mm macros
# and converts to PostScript
#TROFF=ditroff $(MACROS) -Tpsc | psdit
#TROFF=troff $(MACROS) -Tpsc | psdit
TROFF=psroff $(MACROS)

# This command sends output to a printer.
#LPR=cpr -Bps
LPR=lpr

all: relnote.ps userguide.ps progguide.ps paper.ps

printall:	print00 print01 print02

print00:	relnote.ps
	cat relnote.ps | $(LPR)

relnote.ps:	relnote.mm
	$(SOELIM) relnote.mm | $(TBL) | $(TROFF) >relnote.ps

print01:	userguide.ps
	cat userguide.ps | $(LPR)

userguide.ps:	userguide.mm
	$(SOELIM) userguide.mm | $(TBL) | $(TROFF) >userguide.ps

print02:	progguide.ps
	cat progguide.ps | $(LPR)

progguide.ps:	progguide.mm
	$(SOELIM) progguide.mm | $(TBL) | $(TROFF) >progguide.ps

print03:	paper.ps
	cat paper.ps | $(LPR)

paper.ps:	paper.mm
	$(SOELIM) paper.mm | $(TBL) | $(TROFF) >paper.ps
