SHELL = /bin/sh
MAKE = make
NMAKE = nmake
FLAGS = $(MFLAGS) -f Makefile.ini BOOTSTRAPCFLAGS="$(BOOTSTRAPCFLAGS)"

all:
	$(MAKE) $(FLAGS) all

World:
	$(MAKE) $(FLAGS) World

World.Win32:
	$(NMAKE) $(MFLAGS) -f Makefile.ini World.Win32
