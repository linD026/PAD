#!/usr/bin/env bash

gcc -o test ./target.c /home/slda/github/pad/libpad.so -DCONFIG_DEBUG -rdynamic -pthread -g -Wall -no-pie
#-fcf-protection=none
LD_LIBRARY_PATH=./../../. ./test
#LD_PRELOAD=./../../libpad.so ./test

