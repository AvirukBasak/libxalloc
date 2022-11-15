SRCEXT      := c
OBJEXT      := o
HEADEREXT   := h

# directories

SRC_DIR     := ../../src
BUILD_DIR   := ../../build
BIN_DIR     := ../../bin
LIB_DIR     := ../../lib
TEST_DIR    := ../../tests
INCLUDE_DIR := ../../include

CC          := gcc
CFLAGS      := -Wall -Ofast
CDBGFLAGS   := -Wall -g -ggdb -D DEBUG
DBG         := gdb -q

all: $(LIB_DIR)/libavl.a

$(LIB_DIR)/libavl.a: avl.$(SRCEXT)
	$(CC) $(CFLAGS) -c -o avl.$(OBJEXT) avl.$(SRCEXT)
	ar rcs $(LIB_DIR)/libavl.a avl.$(OBJEXT)

clean:
	@rm -f $(LIB_DIR)/libavl.a
	@rm -f avl.o

cleaner: clean
