#!/usr/bin/env bash

#-fno-function-cse

#gcc -o test ./test_inject.c -g -Wall

#gcc -o test ./test_inject_call.c -g -Wall

#gcc -o test ./test_inject_ind.c -g -Wall

gcc -o test ./test_inject_so.c ../../libpad.so -DCONFIG_DEBUG -rdynamic -pthread -g -Wall
#-fcf-protection=none
LD_LIBRARY_PATH=./../../. ./test
#LD_PRELOAD=./../../libpad.so ./test
