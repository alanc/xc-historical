# 
#  Copyright 1990, 1991 by the Massachusetts Institute of Technology and
#  UniSoft Group Limited.
#  
#  Permission to use, copy, modify, distribute, and sell this software and
#  its documentation for any purpose is hereby granted without fee,
#  provided that the above copyright notice appear in all copies and that
#  both that copyright notice and this permission notice appear in
#  supporting documentation, and that the names of MIT and UniSoft not be
#  used in advertising or publicity pertaining to distribution of the
#  software without specific, written prior permission.  MIT and UniSoft
#  make no representations about the suitability of this software for any
#  purpose.  It is provided "as is" without express or implied warranty.
# 
# $XConsortium$
#

##########################################################################
#
#	SCCS:		
#	NAME:		font
#	PRODUCT:	xtest library
#	AUTHOR:		Steve Ratcliffe, UniSoft Ltd.
#	DATE CREATED:	30 Aug 1990
#	TARGETS:	libfont.a
#	MODIFICATIONS:
#
##########################################################################


CFILES = xtfont0.c xtfont1.c xtfont2.c xtfont3.c xtfont4.c xtfont5.c \
	xtfont6.c
OFILES = xtfont0.o xtfont1.o xtfont2.o xtfont3.o xtfont4.o xtfont5.o \
	xtfont6.o
RM_FILES = *.ln *.o core *.a

##############################################################################

all: libfont.a

libfont.a: $P	$(OFILES)
		$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
		: $(RANLIB) $@

FORCE:		CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB:	$(LINTFONTS)

$(LINTFONTS):	llib-lfonts.ln
		$(CP) $? $@
		@echo "	$? made and installed"

llib-lfonts.ln:
		$(LINT) -Cfonts $(LINTFLAGS) $(CFILES)

clean:
		$(RM) $(RM_FILES)

clobber:	clean

install: all
	$(CP) libfont.a $(XTESTLIBDIR)
	$(RANLIB) $(XTESTLIBDIR)/libfont.a

# -----------------
#
# NOTE: The way that the fonts are installed is system dependent.
# This part of the Makefile is provided solely as an example.
#
# See the User Guide for more details.
#

FONTCOMP=bdftosnf
EXTEN=.snf
FONTDIR=/usr/lib/X11/ncd/fonts/xtest
FONTS=xtfont0 xtfont1 xtfont2 xtfont3 xtfont4 xtfont5 xtfont6

bdfcomp:
	@ echo "These commands are system dependent - See User Guide"
	- $(SHELL) ./fcomp $(FONTS)
	- mkfontdir;
bdfinstall:
	@ echo "These commands are system dependent - See User Guide"
	cp *.bdf *.snf fonts.* $(FONTDIR)

