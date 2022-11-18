SRCEXT      := c
OBJEXT      := o
HEADEREXT   := h

# directories

SRC_DIR     := ../src
BUILD_DIR   := ../build
BIN_DIR     := ../bin
LIB_DIR     := ../lib
TEST_DIR    := ../tests
INCLUDE_DIR := ../include

CC          := gcc
CFLAGS      := -Wall -Ofast
CDBGFLAGS   := -Wall -g -ggdb -D DEBUG
DBG         := gdb -q

all:
	@cd libdummy && $(MAKE)

clean:
	@cd libdummy && $(MAKE) clean

cleaner:
	@cd libdummy && $(MAKE) cleaner
