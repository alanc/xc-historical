/**/#! /bin/sh
/**/# Collects multiple outputs of x11perf.  Runs getavg, fillblanks, getrate, on
/**/# each file, and collects the output neatly.
/**/#
/**/# Normally uses the results from $1 to extract the test labels, which allows
/**/# comparisons of a subset of x11perf tests.  Using -l <filename> as $1 and $2
/**/# forces x11perfcomp to use the labels stored in $2.
/**/#
/**/# Mark Moraes, University of Toronto
/**/# Joel McCormack, DEC Western Research Lab
PATH=LIBPATH:.:$PATH
export PATH

set -e
tmp=/tmp/rates.$$
trap "rm -rf $tmp" 0 1 2 15
mkdir $tmp $tmp/rates
average=
relaverage=0
allfiles=
case $1 in
-a)
	average=1
	shift;
	;;
-r)
	average=1
	relaverage=1
	shift;
	;;
esac
case $1 in
-l)	cp $2 $tmp/labels
	shift; shift
	;;
*)	getlabel $* > $tmp/labels
	;;
esac
n=1
for i
do
	base=`basename $i`
	(echo "     $n  "; \
	 echo '--------'; \
	 getavg $i > $tmp/$n.avg; \
	 fillblnk $tmp/$n.avg $tmp/labels > $tmp/$n.favg; \
	 getrate $tmp/$n.favg) \
	    > $tmp/rates/$n
	echo "$n: $base"
	allfiles="$allfiles$n "
	n=`expr $n + 1`
done
case x$average in
x)
	average=/bin/cat
	;;
*)
	average="getrel $n $relaverage"
	;;
esac
echo ''
(echo Operation; echo '---------'; cat $tmp/labels) | \
(cd $tmp/rates; paste $allfiles -) | \
sed 's/	/   /g' | $average

