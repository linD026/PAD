PWD := $(CURDIR)

INC=$(PWD)/include
INC_PARAMS=$(INC:%=-I%)

CC ?= gcc
CFLAGS := -Wall
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -std=c11

DEBUG_FLAGS =

DEBUG := 1

ifeq ($(strip $(DEBUG)), 1)
DEBUG_FLAGS += -D'CONFIG_DEBUG'
CFLAGS+=$(DEBUG_FLAGS)
endif

SRC := src/pad.c
SRC += src/logs.c
SRC += src/verifier.c
SRC += src/probe.c
SRC += src/shmem.c

OBJ := $(SRC:.c=.o)

BIN := pad

###
# library

ARCH := x86-64

ifeq ($(strip $(ARCH)), x86-64)
ARCH_LIB_SRC := src/libpad/x86_64.c
CFLAGS += -D'CONFIG_ARCH_X86_64'
endif

LIB_SRC := $(ARCH_LIB_SRC)
LIB_SRC += src/logs.c
LIB_SRC += src/shmem.c
LIB_SRC += src/libpad/pad.c

CFLAGS += -fPIC
CFLAGS += -no-pie

LIB_OBJ := $(LIB_SRC:.c=.o)
STATIC_LIB := libpad.a
DYNAMIC_LIB := libpad.so

ifneq ($(strip $(static)),)
LD=ar
LDFLAGS=crsv
LD_BIN=$(STATIC_BIN)
LD_TO=
LD_GEN=ranlib
LD_GEN_TARGET=$(STATIC_LIB)
LIB=$(STATIC_LIB)
else
LD=$(CC)
LDFLAGS=-shared
LDFLAGS+=$(DEBUG_FLAGS)
LD_BIN=$(DYNAMIC_LIB)
LD_TO=-o
LD_GEN=
LD_GEN_TARGET=
LIB=$(DYNAMIC_LIB)
endif

###

RM := rm

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PARAMS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(INC_PARAMS) $(OBJ) -o $@

lib: $(LIB_OBJ)
	$(LD) $(LDFLAGS) $(LD_TO) $(LD_BIN) $(LIB_OBJ)
	$(LD_GEN) $(LD_GEN_TARGET)

clean:
	$(RM) -f src/*/*.o
	$(RM) -f src/*.o
	$(RM) -f $(BIN) $(LD_BIN)

cscope:
	find $(PWD) -name "*.c" -o -name "*.h" > $(PWD)/cscope.files
	cscope -b -q

indent:
	clang-format -i include/*/*.[ch]
	clang-format -i src/*/*.[ch]
	clang-format -i src/*.[ch]
	clang-format -i tests/*/*.[ch]

ifeq ($(quiet), 1)
.SILENT:
endif
