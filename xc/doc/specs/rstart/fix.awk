#! /bin/awk -f

BEGIN {
	ignore = 1;
}

# following line starts /^L/
//	{
	print;
	ignore = 1;
	next;
}

/^$/ {
	if(ignore) next;
}

{
	ignore = 0;
	print;
}
