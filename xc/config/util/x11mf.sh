/**/#!/bin/sh

/**/# 
/**/# generate a Makefile within the build tree
/**/# 
/**/# usage:  x11mf [treedir]
/**/# 

if [ x$1 != x ]; then
	tree=$1
else
	tree=/x11
fi

dir=`pwd`
top=`(cd $tree; /bin/pwd)`
intree=no

case $dir in
	$top*)	intree=yes;;
esac

if [ $intree != yes ]; then
	echo "Must be underneath /x11"
	exit 1
fi

(cd ..; make SUBDIRS=`basename $dir` Makefiles)
