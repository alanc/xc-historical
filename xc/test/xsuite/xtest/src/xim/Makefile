# $XConsortium$
##########################################################################
#
#
##########################################################################
# Default directory locations
#
# The location of TET_ROOT.  This must not contain variable expansions.
#
#
# Location of Test Suite
#
SUITE_ROOT	= ${TET_ROOT}/xtest
SUITE_LIB	= ${SUITE_ROOT}/lib
SUITE_BIN	= ${SUITE_ROOT}/bin
SUITE_INC	= ${SUITE_ROOT}/include

INSTLIB		= $(SUITE_LIB)
INSTBIN		= $(SUITE_BIN)

################################
##### C compiler Directives Section
################################

CFILES		= linklocale.c cleanlocale.c ic.c nextlocale.c nextlclmod.c \
			nextfontset.c parse.c xim_cb.c xim_comp.c xim_response.c \
			xim_save.c xim_stimulus.c
OFILES		= linklocale.o cleanlocale.o ic.o nextlocale.o nextlclmod.o \
			nextfontset.o parse.o xim_cb.o xim_comp.o xim_response.o \
			xim_save.o xim_stimulus.o

LIBRARY		= libximtest.a

##############################################################################

install:	all
	$(CP) $(LIBRARY) $(INSTLIB)/$(LIBRARY)
	$(RANLIB) $(INSTLIB)/$(LIBRARY)

all:	$(LIBRARY)
	
$(LIBRARY): $P 	$(OFILES)
	$(AR) $@ `$(LORDER) $(OFILES) | $(TSORT)`
	$(RANLIB) $@

FORCE:		CLOBBER all

LINT:
		$(LINT) $(LINTFLAGS) $(CFILES) $(LINTTCM) $(LINTLIBS)

LINTLIB:	$(LINTSUNTEST)

$(LINTTEST):	llib-lsuntest.ln
		cp $? $@
		@echo "	$? made and installed"

llib-lsuntest.ln: $(CFILES)
		$(LINT) -Csuite $(LINTFLAGS) -u $?

CLEAN:
		$(RM) $(OFILES) $(LIBRARY) makefile.bak 

CLOBBER:	CLEAN
		$(RM) $(INSTLIB)/$(LIBRARY)
		

clean:		CLEAN

shar:
		shar Makefile $(SUITE_INC)/*.h $(CFILES) > libsuntest.shar

