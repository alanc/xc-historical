# $XConsortium: mmmisc.mc,v 1.2 92/06/11 19:34:18 rws Exp $
#
# Miscellaneous housekeeping functions.
#

# clean up and remove remakable sources, objects and junk files.
#
clean:
	$(RM) Test $(OFILES) $(LOFILES) $(LINKOBJ) $(LINKEXEC) core\
		MTest m$(LINKEXEC) $(MOFILES) CONFIG Makefile.bak $(AUXCLEAN)\
		MTest.c Test.c mlink.c link.c Makefile *.err

# clobber - aka clean.
#
clobber:	clean

# Lint makerules
#
lint: $(CFILES)
	$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINT:lint

