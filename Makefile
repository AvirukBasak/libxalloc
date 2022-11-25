SRCEXT      := c
OBJEXT      := o
HEADEREXT   := h

# directories

SRC_DIR     := src
BUILD_DIR   := build
TARGET_DIR  := target
LIB_DIR     := lib
TEST_DIR    := tests
INCLUDE_DIR := include

# compiler options

LIB_NAME    := xalloc

CC          := gcc
CFLAGS      := -Wall -Ofast
CDBGFLAGS   := -Wall -g -D DEBUG
DBG         := gdb -q

INCLUDE     := -I $(INCLUDE_DIR) -I $(LIB_DIR)
LIB         := -L$(LIB_DIR) -lm

LIBRARIES   := $(shell find $(LIB_DIR)/ -name "*.a")

# targets

TARGET_NAME := lib$(LIB_NAME)
TARGET      := $(TARGET_DIR)/$(TARGET_NAME).a
DBG_TARGET  := $(TARGET_DIR)/$(TARGET_NAME)-dbg.a
HDR_TARGET  := $(TARGET_DIR)/$(TARGET_NAME).$(HEADEREXT)

SOURCES     := $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT))
HEADERS     := $(shell find $(INCLUDE_DIR)/ -name "*."$(HEADEREXT))
TESTSRC     := $(shell find $(TEST_DIR)/ -name "*."$(SRCEXT))

## release build

rel: mkdirp $(HDR_TARGET) $(TARGET)

OBJECTS     := $(patsubst $(SRC_DIR)/%.$(SRCEXT), $(BUILD_DIR)/%.$(OBJEXT), $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT)))

$(OBJECTS): $(SOURCES) $(HEADERS)
	@cd $(SRC_DIR) && $(MAKE)

$(TARGET): $(LIBRARIES) $(OBJECTS)
	ar rcs $(TARGET) $(BUILD_DIR)/*.$(OBJEXT)

## debug build

dbg: mkdirp $(HDR_TARGET) $(DBG_TARGET)

DBG_OBJECTS := $(patsubst $(SRC_DIR)/%.$(SRCEXT), $(BUILD_DIR)/%-dbg.$(OBJEXT), $(shell find $(SRC_DIR)/ -name "*."$(SRCEXT)))

$(DBG_OBJECTS): $(SOURCES) $(HEADERS)
	@cd $(SRC_DIR) && $(MAKE) dbg

$(DBG_TARGET): $(LIBRARIES) $(DBG_OBJECTS)
	ar rcs $(DBG_TARGET) $(BUILD_DIR)/*-dbg.$(OBJEXT)

## make lib headers

$(HDR_TARGET): $(INCLUDE_DIR)/$(TARGET_NAME).$(HEADEREXT)
	@cp $(INCLUDE_DIR)/$(TARGET_NAME).$(HEADEREXT) $(HDR_TARGET)
	$(info make $(HDR_TARGET))

## build libraries
$(LIBRARIES):
	@cd $(LIB_DIR) && make

## testing / execution

test: rel $(TESTSRC)
	@$(CC) $(CFLAGS) -I $(TARGET_DIR) $(TEST_DIR)/test.$(SRCEXT) -o $(TARGET_DIR)/test -L$(TARGET_DIR) -l$(LIB_NAME) $(LIB)
	./$(TARGET_DIR)/test

testdbg: dbg $(TESTSRC)
	@$(CC) $(CDBGFLAGS) -I $(TARGET_DIR) $(DBG_OBJECTS) $(TEST_DIR)/test.$(SRCEXT) -o $(TARGET_DIR)/test-dbg $(LIB)
	$(DBG) $(TARGET_DIR)/test-dbg

test-fail: rel $(TESTSRC)
	@$(CC) $(CFLAGS) -I $(TARGET_DIR) $(TEST_DIR)/test-fail.$(SRCEXT) -o $(TARGET_DIR)/test-fail -L$(TARGET_DIR) -l$(LIB_NAME) $(LIB)
	./$(TARGET_DIR)/test-fail

test-fail-dbg: dbg $(TESTSRC)
	@$(CC) $(CDBGFLAGS) -I $(TARGET_DIR) $(DBG_OBJECTS) $(TEST_DIR)/test-fail.$(SRCEXT) -o $(TARGET_DIR)/test-fail-dbg $(LIB)
	$(DBG) $(TARGET_DIR)/test-fail-dbg

## mkdirp

mkdirp:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(TARGET_DIR)

## cleanup

clean:
	@cd $(SRC_DIR) && $(MAKE) clean
	@cd $(LIB_DIR) && $(MAKE) clean

cleaner:
	@cd $(SRC_DIR) && $(MAKE) cleaner
	@cd $(LIB_DIR) && $(MAKE) cleaner
	@rm -rf $(BUILD_DIR)
	@rm -rf $(TARGET_DIR)
