SRCEXT      := c
OBJEXT      := o
HEADEREXT   := h

# directories

SRC_DIR     := src
BUILD_DIR   := build
BIN_DIR     := bin
LIB_DIR     := lib
TEST_DIR    := tests
INCLUDE_DIR := include

# compiler options

LIB_NAME    := xalloc

CC          := gcc
CFLAGS      := -Wall -Ofast
CDBGFLAGS   := -Wall -g -ggdb -D DEBUG
DBG         := gdb -q

INCLUDE     := -I $(INCLUDE_DIR) -I $(LIB_DIR)
LIB         := -L$(LIB_DIR) -lm

# targets

TARGET_NAME := lib$(LIB_NAME)
TARGET      := $(BIN_DIR)/$(TARGET_NAME).a
DBG_TARGET  := $(BIN_DIR)/$(TARGET_NAME)-dbg.a

SOURCES     := $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT))
TESTSRC     := $(shell find $(TEST_DIR)/ -name "*."$(SRCEXT))
HEADERS     := $(shell find $(INCLUDE_DIR)/ -name "$(TARGET_NAME).$(HEADEREXT)")

LIBRARIES   := $(LIB_DIR)/libdummy/dummy.o

## release build

all: mkdirp $(BIN_DIR)/$(TARGET_NAME).$(HEADEREXT) $(TARGET)

OBJECTS     := $(patsubst $(SRC_DIR)/%.$(SRCEXT), $(BUILD_DIR)/%.$(OBJEXT), $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT)))

$(OBJECTS): $(SOURCES)
	@cd $(SRC_DIR) && $(MAKE)

$(TARGET): $(OBJECTS) $(LIBRARIES)
	ar rcs $(TARGET) $(BUILD_DIR)/*.$(OBJEXT) $(LIBRARIES)

## debug build

dbg: mkdirp $(BIN_DIR)/$(TARGET_NAME).$(HEADEREXT) $(DBG_TARGET)

DBG_OBJECTS := $(patsubst $(SRC_DIR)/%.$(SRCEXT), $(BUILD_DIR)/%-dbg.$(OBJEXT), $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT)))

$(DBG_OBJECTS): $(SOURCES)
	@cd $(SRC_DIR) && $(MAKE) dbg

$(DBG_TARGET): $(DBG_OBJECTS) $(LIBRARIES)
	ar rcs $(DBG_TARGET) $(BUILD_DIR)/*-dbg.$(OBJEXT) $(LIBRARIES)

## lib

$(LIBRARIES):
	@cd $(LIB_DIR) && $(MAKE)

## headers

$(BIN_DIR)/$(TARGET_NAME).$(HEADEREXT): $(HEADERS)
	@cp $(HEADERS) $(BIN_DIR)/$(TARGET_NAME).$(HEADEREXT)
	$(info make $(BIN_DIR)/$(TARGET_NAME).$(HEADEREXT))

## execution

test: all $(TESTSRC)
	@$(CC) $(CFLAGS) -I $(BIN_DIR) $(TEST_DIR)/test.$(SRCEXT) -o $(BIN_DIR)/test $(LIB) -L$(BIN_DIR) -lxalloc
	./$(BIN_DIR)/test
	@#rm ./$(BIN_DIR)/test

testdbg: dbg $(TESTSRC)
	@$(CC) $(CDBGFLAGS) $(INCLUDE) -I $(BIN_DIR) $(DBG_OBJECTS) $(TEST_DIR)/test.$(SRCEXT) -o $(BIN_DIR)/test-dbg $(LIB)
	$(DBG) $(BIN_DIR)/test-dbg
	@#rm ./$(BIN_DIR)/test-dbg



test-fail: all $(TESTSRC)
	@$(CC) $(CFLAGS) -I $(BIN_DIR) $(TEST_DIR)/test-fail.$(SRCEXT) -o $(BIN_DIR)/test-fail $(LIB) -L$(BIN_DIR) -lxalloc
	./$(BIN_DIR)/test-fail
	@#rm ./$(BIN_DIR)/test

test-fail-dbg: dbg $(TESTSRC)
	@$(CC) $(CDBGFLAGS) $(INCLUDE) -I $(BIN_DIR) $(DBG_OBJECTS) $(TEST_DIR)/test.$(SRCEXT) -o $(BIN_DIR)/test-fail-dbg $(LIB)
	$(DBG) $(BIN_DIR)/test-fail-dbg
	@#rm ./$(BIN_DIR)/test-dbg

## mkdirp

mkdirp:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

## Clean

clean:
	@cd $(SRC_DIR) && $(MAKE) clean
	@cd $(LIB_DIR) && $(MAKE) clean

cleaner:
	@cd $(SRC_DIR) && $(MAKE) cleaner
	@cd $(LIB_DIR) && $(MAKE) cleaner
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
