#!/bin/sh

PREFIX=test
TESTS='0 1 2 3 4'
PROG=runme.sh
count=0

if [ ! -x $PROG ]; then
  echo Error: $PROG is not in the current directory or is not executable
  exit 1
fi

for T in $TESTS; do
  if [ -f $PREFIX.$T.xo ]; then  
    echo ===========================================
    echo Test file: $PREFIX.$T.xo 
    ./$PROG 0 $PREFIX.$T.xo 32 | grep -v "^CPU" | grep -v '^$' | sort -n -s > $PREFIX.$T.out
    if diff $PREFIX.$T.out $PREFIX.$T.gold > /dev/null; then
      echo " " PASSED
      let "count = count + 1"
    else
      echo " " FAILED
    fi
    # rm $PREFIX.$T.out
  else
    echo Oops: file $PREFIX.$T.xo does not exist!
  fi
done

echo =============================================
echo $count tests passed
