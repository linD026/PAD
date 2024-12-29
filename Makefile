PWD := $(CURDIR)

INC=$(PWD)/include
INC_PARAMS=$(INC:%=-I%)

CC ?= gcc
CFLAGS := -Wall
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -std=c11

DEBUG_FLAGS =

ifneq ($(strip $(debug)),)
DEBUG_FLAGS += -D'CONFIG_DEBUG'
CFLAGS+=$(DEBUG_FLAGS)
endif

SRC := src/pad.c
SRC += src/logs.c
SRC += src/verifier.c
SRC += src/probe.c

OBJ := $(SRC:.c=.o)

BIN := pad

RM := rm

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PARAMS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(INC_PARAMS) $(OBJ) -o $@

clean:
	$(RM) -f src/*/*.o
	$(RM) -f src/*.o
	$(RM) -f $(BIN)

cscope:
	find $(PWD) -name "*.c" -o -name "*.h" > $(PWD)/cscope.files
	cscope -b -q

indent:
	clang-format -i include/*/*.[ch]
	clang-format -i src/*/*.[ch]
	clang-format -i src/*.[ch]

ifeq ($(quiet), 1)
.SILENT:
endif
