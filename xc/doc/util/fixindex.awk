BEGIN {
	FS = ":";
	BD = "\\s+1\\fB";
	ED = "\\fP\\s-1";
}

NR == 1 {
	if ($3 != "")
		printf(".Ib \"%s\"\n", $2);
	major = $2;
	minor = $3;
	pageno = $1;
	oldpageno = $1;
	oldpagelist = "";
	pagelist = $1;
}

NR != 1 {
	if ($2 == major && $3 == minor)		# neither has changed
	{
		if ($1 != pageno) {		# new page number, append
			if ($4 == "@DEF@") {
				pagelist = pagelist ", " BD $1 ED;
			}
			else {
				pagelist = pagelist ", " $1;
			}
			oldpageno = $1;
			oldpagelist = pagelist;
		}
		else {				# old page, but check for def
               	        if ($4 == "@DEF@") {
                            if (pageno == oldpageno) {
                                if (oldpagelist != "")
                                        oldpagelist = oldpagelist ", "
			    }
                            pagelist = oldpagelist BD $1 ED;
                        }
                }
	}
	else					# one has changed
	{
		if (minor != "")	# dump full record
			printf(".I< \"%s\" \"%s\" \"%s\"\n", major, minor, pagelist);
		else
			printf(".I> \"%s\" \"%s\"\n", major, pagelist);
		pagelist = $1;			# restart pagelist
		oldpagelist = "";
		oldpageno = $1;
		if ($2 != major && $3 != "")	# major has changed, minor not null
		printf(".Ib \"%s\"\n", $2);
	}
	major = $2;
	minor = $3;
	pageno = $1;
}

END {
	if (minor != "")			# dump full record
		printf(".I< \"%s\" \"%s\" \"%s\"\n", major, minor, pagelist);
	else
		printf(".I> \"%s\" \"%s\"\n", major, pagelist);
}
