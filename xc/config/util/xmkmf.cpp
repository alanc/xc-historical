/**/#!/bin/sh

/**/#
/**/# generate a Makefile from an Imakefile from outside the X sources.
/**/# 

case $# in 
    0) ;;
    1) topdir=-DTOPDIR=$1 ;;
    *) echo "usage:  $0 [topdirpathname]" 1>&2; exit 1 ;;
esac

if [ -f Makefile ]; then 
    echo mv Makefile Makefile.bak
    mv Makefile Makefile.bak
fi
echo imake -DUseInstalled CONFIGDIRSPEC $topdir
imake -DUseInstalled CONFIGDIRSPEC $topdir
