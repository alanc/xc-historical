# $XConsortium$
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
#      SCCS:  @
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
#     tcc -b -s link_exec xtest linkbuild
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

CAT=cat

ALLTESTS=Tests

TESTOFILES=\
	bsfntnmlst.o		\
	cntxtdpndn.o		\
	crtfntst.o		\
	extntsoffn.o		\
	fntsoffnts.o		\
	frfntst.o		\
	lcloffntst.o		\
	mbdrwimgst.o		\
	mbdrwstr.o		\
	mbdrwtxt.o		\
	mbtxtescpm.o		\
	mbtxtextnt.o		\
	mbtxtprchr.o		\
	rmgtdtbs.o		\
	rmlclofdtb.o		\
	rmstdtbs.o		\
	scrnrsrcst.o		\
	spprtslcl.o		\
	stlclmdfrs.o		\
	wcdrwimgst.o		\
	wcdrwstr.o		\
	wcdrwtxt.o		\
	wctxtescpm.o		\
	wctxtextnt.o		\
	wctxtprchr.o

LINKOFILE=linktbl.o
OFILES = $(TESTOFILES) $(LINKOFILE)

DIRLIST=\
	bsfntnmlst		\
	cntxtdpndn		\
	crtfntst		\
	extntsoffn		\
	fntsoffnts		\
	frfntst		\
	lcloffntst		\
	mbdrwimgst		\
	mbdrwstr		\
	mbdrwtxt		\
	mbtxtescpm		\
	mbtxtextnt		\
	mbtxtprchr		\
	rmgtdtbs		\
	rmlclofdtb		\
	rmstdtbs		\
	scrnrsrcst		\
	spprtslcl		\
	stlclmdfrs		\
	wcdrwimgst		\
	wcdrwstr		\
	wcdrwtxt		\
	wctxtescpm		\
	wctxtextnt		\
	wctxtprchr

all: subdirs test

test:$P $(OFILES) $(LIBS) $(TCM)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(TCM) $(LIBS) $(SYSLIBS) $(SYSMATHLIB)
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

clean:	cletests clesubdirs

cletests:
	$(RM) test $(OFILES) $(ALLTESTS) core

clesubdirs:
	for i in $(DIRLIST); do \
		if [ -d $$i ]; then (cd $$i; $(TET_CLEAN_TOOL) ); fi; done

clobber: clean
