#!/usr/bin/env bash

PREFIX=$1
PDF="$PREFIX/pad"
PROGRAM="$PREFIX/tests/simple/program.c"
#TARGET="$PREFIX/tests/simple/target.c"
PID=$2
SYMBOL="target"
ACTION="LOAD"
CC="/usr/bin/gcc"

echo "$PDF --CC $CC --PROGRAM $PROGRAM --TARGET_PID $PID --SYMBOL $SYMBOL --ACTION $ACTION"
$PDF --CC $CC --PROGRAM $PROGRAM --TARGET_PID $PID --SYMBOL $SYMBOL --ACTION $ACTION
