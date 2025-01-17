#!/usr/bin/env bash

NR_TIME=5


PREFIX="$1"
TMP_FILE="tmp-$PREFIX.log"

echo -n > $TMP_FILE
echo -n > $SUM_FILE

for (( i=1; i <= $NR_TIME; i=i+1 )); do
	LD_LIBRARY_PATH=./../../. ./test >>  $TMP_FILE
done

python3 graph.py $TMP_FILE $SUM_FILE
