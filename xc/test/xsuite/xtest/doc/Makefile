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
# $XConsortium$
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

# This command formats text using nroff/troff with the mm macros.
TROFF=ditroff -mm -Tpsc

# This command converts to PostScript format if needed (if not, it can
# be altered to "cat").
PSDIT=psdit

# This command sends output to a printer.
LPR=cpr -Bps

all:	printall

printall:	print00 print01 print02

print00:	rel_note.ps
	cat rel_note.ps | $(LPR)

rel_note.ps:	rel_note.mm
	$(SOELIM) rel_note.mm | $(TBL) | $(TROFF) | $(PSDIT) >rel_note.ps

print01:	user_guide.ps
	cat user_guide.ps | $(LPR)

user_guide.ps:	user_guide.mm
	$(SOELIM) user_guide.mm | $(TBL) | $(TROFF) | $(PSDIT) >user_guide.ps

print02:	prog_guide.ps
	cat prog_guide.ps | $(LPR)

prog_guide.ps:	prog_guide.mm
	$(SOELIM) prog_guide.mm | $(TBL) | $(TROFF) | $(PSDIT) >prog_guide.ps

print03:	paper.ps
	cat paper.ps | $(LPR)

paper.ps:	paper.mm
	$(SOELIM) paper.mm | $(TBL) | $(TROFF) | $(PSDIT) >paper.ps
