CC ?= gcc
CFLAGS := -Wall
LIB := ../../libpad.so

FILE := test.c
TIME := 100

OUTPUT = $(shell date +"%Y-%m-%d")


all: compile-lib
	$(CC) -o test test.c $(CFLAGS) $(LIB)
	bash script.sh "$(OUTPUT)"

compile-lib:
	$(MAKE) -C ../../. clean
	$(MAKE) -C ../../. lib DEBUG=0

clean:
	rm -f test
