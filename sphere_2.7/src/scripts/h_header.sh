#! /bin/sh
#
#  Quick and dirty way of printing headers from
#  NIST format speech files. Since it uses 'sed'
#  it may not work on files with very long lines
#  or non-ascii characters in the header before
#  "end_head"

if test $# -eq 0 ; then
	exec sed -n -e '/^end_head$/ q' -e p
	exit 1
fi

E=0
for FILE
	do
	test $# -gt 1 && echo "::::: ${FILE} :::::"
	sed -n -e '/^end_head$/ q' -e p < "${FILE}"
	S=$?
	test $S -ne 0 && E=$S
	done

exit $E
