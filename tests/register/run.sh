#!/usr/bin/env bash

gcc -o test ./test_register.c ../../libpad.so -DCONFIG_DEBUG -rdynamic -pthread -g -Wall
#-fcf-protection=none
LD_LIBRARY_PATH=./../../. ./test
#LD_PRELOAD=./../../libpad.so ./test
