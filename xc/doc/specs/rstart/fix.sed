#! /bin/sed -f
s/o+/./g
s/|-/+/g
s/.//g
/FORMFEED\[Page/{
s/FORMFEED/        /
a\

}
