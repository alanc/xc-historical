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
# $XConsortium: Makefile,v 1.7 92/06/29 18:18:25 rws Exp $
#

#
# This Makefile can be used to compile all the tests in this 
# section of the X test suite in such a way that they are all links to 
# a single executable file. This normally allows a considerable 
# reduction in the disc space requirements for the X test suite 
# when fully built.
#
# There are two ways this can be done:
# 1) Using the TET. Execute the command:
#     tcc -b -s link_scen xtest linkbuild
# in the directory $TET_ROOT/xtest.
# This will execute the TET build tool (which is normally pmake) 
# in the top level directory of each section of the test suite 
# (including this directory).
#
# 2) Directly without using the TET. Execute the command:
#     pmake 
# in this directory.
#
# For more details, refer to the User Guide
#

# CFLAGS - Compilation flags specific to the X Protocol tests.
#
CFLAGS=$(XP_CFLAGS)
SYSLIBS=$(XP_SYSLIBS)
LIBS=$(XP_LIBS)

# LINTFLAGS - Flags for lint specific to the X Protocol tests.
#
LINTFLAGS=$(XP_LINTFLAGS)
LINTLIBS=$(XP_LINTLIBS)

CAT=cat

ALLTESTS=Tests

# XXX misc.o is missing
TESTOFILES=\
allwdvcevn.o \
chngdvccnt.o \
chngdvcdnt.o \
chngfdbckc.o \
chngkybrdd.o \
chngpntrdv.o \
chngdvckym.o \
clsdvc.o \
dvcbll.o \
gtdvcmdfrm.o \
gtfdbckcnt.o \
gtextnsnvr.o \
grbdvc.o \
grbdvcbttn.o \
grbdvcky.o \
gtdvcbttnm.o \
gtdvccntrl.o \
gtdvcfcs.o \
gtdvcmtnev.o \
gtdvcdntpr.o \
gtdvckympp.o \
gtslctdext.o \
lstinptdvc.o \
mscllns.o \
opndvc.o \
qrydvcstt.o \
stdvcvltrs.o \
stdvcbttnm.o \
stdvcfcs.o \
stdvcmdfrm.o \
stdvcmd.o \
slctextnsn.o \
sndextnsne.o \
ungrbdvc.o \
ungrbdvcbt.o \
ungrbdvcky.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

# XXX misc is missing
DIRLIST=\
allwdvcevn \
chngdvccnt \
chngdvcdnt \
chngfdbckc \
chngkybrdd \
chngpntrdv \
chngdvckym \
clsdvc \
dvcbll \
gtdvcmdfrm \
gtfdbckcnt \
gtextnsnvr \
grbdvc \
grbdvcbttn \
grbdvcky \
gtdvcbttnm \
gtdvccntrl \
gtdvcfcs \
gtdvcmtnev \
gtdvcdntpr \
gtdvckympp \
gtslctdext \
lstinptdvc \
mscllns \
opndvc \
qrydvcstt \
stdvcvltrs \
stdvcbttnm \
stdvcfcs \
stdvcmdfrm \
stdvcmd \
slctextnsn \
sndextnsne \
ungrbdvc \
ungrbdvcbt \
ungrbdvcky

all: subdirs test

test:$P $(OFILES) $(LIBS) $(TCM)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(TCM) $(LIBS) $(SYSLIBS)
	$(CAT) test > $(ALLTESTS)
	$(RM) test

subdirs:
	if [ ! -f $(ALLTESTS) ]; then $(CAT) /dev/null > $(ALLTESTS); \
	chmod a+x $(ALLTESTS); else : ; fi
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then \
			(cd $$i; echo Compiling in $$i; $(TET_BUILD_TOOL) linkexec); \
		fi; \
	done

# The xtestlib is made if it doesn't exist
$(XTESTLIB):
	cd $(XTESTROOT)/src/lib; $(TET_BUILD_TOOL) install

# The fontlib is made if it doesn't exist
$(XTESTFONTLIB):
	cd $(XTESTROOT)/fonts; $(TET_BUILD_TOOL) install

# The X Protocol test library is made if it doesn't exist
$(XSTLIB):
	cd $(XTESTROOT)/src/libproto; $(TET_BUILD_TOOL) install


clean:	cletests clesubdirs

cletests:
	$(RM) test $(OFILES) $(ALLTESTS) core

clesubdirs:
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then (cd $$i; $(TET_CLEAN_TOOL) ); fi; done

clobber: clean
