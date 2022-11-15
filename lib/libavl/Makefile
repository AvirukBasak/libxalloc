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

all: $(LIB_DIR)/libavl.a $(INCLUDE_DIR)/avl.$(HEADEREXT)

$(LIB_DIR)/libavl.a: avl.$(SRCEXT)
	$(CC) $(CFLAGS) -c -o avl.$(OBJEXT) avl.$(SRCEXT)
	ar rcs $(LIB_DIR)/libavl.a avl.$(OBJEXT)

$(INCLUDE_DIR)/avl.$(HEADEREXT): avl.$(HEADEREXT)
	@cp avl.$(HEADEREXT) $(INCLUDE_DIR)/avl.$(HEADEREXT)

clean:
	@rm -f $(INCLUDE_DIR)/avl.$(HEADEREXT)
	@rm -f $(LIB_DIR)/libavl.a

cleaner: clean
