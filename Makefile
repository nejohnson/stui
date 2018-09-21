###############################################################################
#
# Makefile for STUI.  GCC variant.
#
# Copyright (C) 2011-2018 Neil Johnson  <www.njohnson.co.uk>
#
###############################################################################

.SUFFIXES:
.SUFFIXES: .a .c .o

CC = gcc

CFLAGS       += -std=c99 -pedantic -funsigned-bitfields -Wundef
CFLAGS       += -O2 -g
CFLAGS       += -I./include -I./driver -I. -I./osal

LDFLAGS      += -Losal -lpthread -lrt

OBJS         += testapp.o

BUILD_DIR     = build

SRC = testapp.c server.c xterm.c

VPATH = test server driver

OBJS    := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

#===============================# Patterns #==================================#

$(BUILD_DIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
	
#=============================# Build Rules #=================================#

# Build targets for users


test: testapp


what:
	@echo Possible targets:
	@echo "  test  : test application"
	@echo "  what  : show this info"

# Internal targets

osal/libosal.a: 
	OSAL_HOSTOS=linux make -C osal lib

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
	
testapp: $(BUILD_DIR) osal/libosal.a $(OBJS) build/format.o
	$(CC) -o $@  $(OBJS) build/format.o $(LDFLAGS) -losal $(LDLIBS)
	
build/testapp.o: test/testapp.c
	$(CC) -c $(CFLAGS) $< -o $@

build/format.o: modules/format/src/format.c
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -rf $(BUILD_DIR)
	rm -rf testapp
	make -C osal clean
