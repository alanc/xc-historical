# $XConsortium$
#
# Miscellaneous housekeeping functions.
#

# clean up object and junk files.
#
clean:
	$(RM) Test $(OFILES) $(LOFILES) $(LINKOBJ) $(LINKEXEC) core\
		MTest m$(LINKEXEC) $(MOFILES) CONFIG Makefile.bak $(AUXCLEAN)

# clobber - clean up and remove remakable sources.
#
clobber: clean
	$(RM) MTest.c Test.c mlink.c link.c Makefile

# Lint makerules
#
lint: $(CFILES)
	$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINT:lint

