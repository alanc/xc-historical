# $XConsortium: mmsa.mc,v 1.2 92/06/11 19:34:26 rws Exp $
#
# Build a standalone version of the test case.
#
Test: $(OFILES) $(LIBS) $(TCM) $(AUXFILES)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(TCM) $(LIBLOCAL) $(LIBS) $(SYSLIBS)

Test.c: $(SOURCES)
	$(CODEMAKER) -o Test.c $(SOURCES)

Test.o: $(DEPHEADERS)
