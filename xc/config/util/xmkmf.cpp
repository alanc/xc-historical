/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from outside the X sources.
/**/# 

if [ -f Makefile ]; then 
    echo mv Makefile Makefile.bak
    mv Makefile Makefile.bak
fi
echo imake -DUseInstalled CONFIGDIRSPEC $*
imake -DUseInstalled CONFIGDIRSPEC $*
