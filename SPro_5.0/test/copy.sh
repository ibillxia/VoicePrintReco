#!/bin/sh -x
#
# Test scopy program
#
# $Author: ggravier $
# $Date: 2007-01-19 17:35:24 +0100 (Fri, 19 Jan 2007) $
# $Revision: 132 $
#


prog=scopy

echo "checking ${prog}:"

if test ! -x ../${prog}; then
  echo "  you should probably build ${prog} before running test..."
  exit 1
fi

mkdir -p log

#
# check usage 
#
../${prog} --version > log/${prog}.stdout 2> log/${prog}.stderr
if test $? -eq 0 -a x`awk '{print NR}' log/${prog}.stderr | tail -1` = x; then
  echo "  build test passed"
else
  echo "  build test failed (see test/log/${prog}.stderr for details)"
  exit 1
fi
\rm -f log/*

#
# check usage 
#
../${prog} -o ascii dat/know1.lfcc - > log/${prog}.stdout 2> log/${prog}.stderr
diff log/${prog}.stdout ref/copy.1 > /dev/null 2> /dev/null
if test $? -eq 0 -a x`awk '{print NR}' log/${prog}.stderr | tail -1` = x; then
  echo "  copy test passed"
else
  echo "  copy test failed (see test/log/${prog}.stderr for details)"
  exit 1
fi
\rm -f log/*

rmdir log
