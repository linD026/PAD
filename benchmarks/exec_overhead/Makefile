CC ?= gcc
CFLAGS := -Wall
CFLAGS += -D'CONFIG_TEST'
LIB := ../../libpad.so

FILE := test.c
TIME := 10

OUTPUT = $(shell date +"%Y-%m-%d")


all: compile-lib
	$(CC) -o test test.c $(CFLAGS) $(LIB)
	bash script.sh "$(OUTPUT)"

compile-lib:
	$(MAKE) -C ../../. clean
	$(MAKE) -C ../../. lib DEBUG=0 TEST=1

clean:
	rm -f test
