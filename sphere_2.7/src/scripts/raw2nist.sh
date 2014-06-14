#! /bin/sh
#
# Converts raw data to Sphere format speech file.
# Uses h_add and h_edit.

if [ $# -lt 2 ]; then
	echo "raw2nist { infile | - } outfile [h_edit options]" >&2
	exit 1
fi

IN="$1"
OUT="$2"
shift 2

if [ "X${OUT}" = X- ]; then
	echo "raw2nist { infile | - } outfile [h_edit options]" >&2
	exit 1
fi

h_add "${IN}" "${OUT}"
S=$?
if [ "${S}" -ne 0 ]; then
	rm -f "${OUT}"
	exit "${S}"
fi

if [ $# -ne 0 ]; then
	h_edit "$@" "${OUT}"
	S=$?
	if [ "${S}" -ne 0 ]; then
		rm -f "${OUT}"
		exit "${S}"
	fi
fi

exit 0
