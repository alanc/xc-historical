/**/#! /bin/sh
/**/# Collects multiple outputs of x11perf.  Just feed it a list of files, each
/**/# containing the output from an x11perf run, and this shell will extract the
/**/# object/second information and show it in tabular form.  An 80-column line
/**/# is big enough to compare 4 different servers.
/**/#
/**/# This script normally uses the results from $1 to extract the test label
/**/# descriptions, so you can run x11perf on a subset of the test and then
/**/# compare the results.  But note that x11perffill requires the labels file
/**/# to be a superset of the x11perf results file.  If you run into an ugly
/**/# situation in which none of the servers completes the desired tests 
/**/# (quite possible on non-DEC servers :), you can use -l <filename> as $1 and
/**/# $2 to force x11perfcomp to use the labels stored in file $2.  (You can run
/**/# x11perf with the -labels option to generate such a file.)
/**/#
/**/# Mark Moraes, University of Toronto <moraes@csri.toronto.edu>
/**/# Joel McCormack, DEC Western Research Lab <joel@decwrl.dec.com>

PATH=LIBPATH:.:$PATH
export PATH

set -e
tmp=/tmp/rates.$$
trap "rm -rf $tmp" 0 1 2 15
mkdir $tmp $tmp/rates
ratio=
allfiles=
/**/# Include relative rates in output?  Report only relative rates?
case $1 in
-r|-a)
	ratio=1
	shift;
	;;
-ro)
	ratio=2
	shift;
	;;
esac
/**/# Get either the provided label file, or construct one from the first
/**/# x11perf output file given.
case $1 in
-l)	cp $2 $tmp/labels
	shift; shift
	;;
*)	awk '$2 == "reps" || $2 == "trep" { \
	    print $0; \
	    next; \
        } \
	' $1 | sed 's/^.*: //' | uniq > $tmp/labels
	;;
esac
/**/# Go through all files, and create a corresponding rate file for each
n=1
for i
do
/**/# Get lines with average numbers, fill in any tests that may be missing
/**/# then extract the rate field
	base=`basename $i`
	(echo "     $n  "; \
	 echo '--------'; \
	 awk '$2 == "reps" || $2 == "trep" {
		line = $0;
		next;
	    }
	    NF == 0 && line != "" {
		print line;
		line="";
		next;
	    }
	 ' $i > $tmp/$base.avg; \
	 fillblnk $tmp/$base.avg $tmp/labels |
	 sed 's/( *\([0-9]*\)/(\1/'   |
	 awk '$2 == "reps" || $2 == "trep" {
		n = substr($6,2,length($6)-7);
		printf "%8s\n", n;
    	}
	') > $tmp/rates/$base
	echo "$n: $base"
	n=`expr $n + 1`
	allfiles="$allfiles$base "
done
case x$ratio in
x)
	ratio=/bin/cat
	;;
x1)
	ratio="perfboth $n"
	;;
*)
	ratio="perfratio $n"
	;;
esac
echo ''
(echo Operation; echo '---------'; cat $tmp/labels) | \
(cd $tmp/rates; paste $allfiles -) | \
sed 's/	/  /g' | $ratio
rm -rf $tmp
