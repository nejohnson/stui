###############################################################################
#
#  OSAL - Operating System Abstraction Layer for Embedded Systems
#  Copyright (C) 2011, Neil Johnson
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms,
#  with or without modification,
#  are permitted provided that the following conditions are met:
#  
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
#  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
###############################################################################

.SUFFIXES:
.SUFFIXES: .a .c .o

CC = gcc
AR = ar
RANLIB = ranlib

# One of these needs defining.  The default is Linux

OSAL_HOSTOS ?= linux
# OSAL_HOSTOS ?= mingw32
# OSAL_HOSTOS ?= tnkernel
# OSAL_HOSTOS ?= freertos

#=============================================================================#
# Set up the various build option flags
#=============================================================================#

OSAL_VPATH = test

#==============================# Trace Layer #================================#

ifdef OSAL_TRACE
   OSAL_DEFINES += OSAL_TRACE
   OSAL_VPATH   += trace
   OSAL_SRC     += trc_task.c trc_semaphore.c trc_timer.c trc_event.c \
                   trc_queue.c trc_defs.c trc_mutex.c
endif

#========================# OSAL Host OS Adaptation #==========================#

# -- TNKernel adaptation layer ------------------------------------------------

ifeq (tnkernel,$(findstring tnkernel, $(OSAL_HOSTOS)))
   OSAL_VPATH   += tnkernel
   OSAL_SRC     += osal.c os_task.c os_semaphore.c os_timer.c os_event.c \
                   os_queue.c os_log.c
endif

# -- FreeRTOS adaptation layer ------------------------------------------------

ifeq (freertos,$(findstring freertos, $(OSAL_HOSTOS)))
   LIB_VPATH   += freertos
   LIB_SRC     += osal.c os_task.c os_semaphore.c os_timer.c os_event.c \
                  os_queue.c os_log.c
endif

# -- Linux adaptation layer --------------------------------------------------

ifeq (linux,$(findstring linux, $(OSAL_HOSTOS)))
   OSAL_DEFINES += _XOPEN_SOURCE=600
# _POSIX_C_SOURCE shouldn't really be necessary with _XOPEN_SOURCE=600, but
# seems to be at the moment
   OSAL_DEFINES += _POSIX_C_SOURCE=200112L

#   OSAL_DEFINES += OSAL_LOG_SIZE_LIMIT=1000000

   OSAL_LPATH   += .
   OSAL_LIBS    += pthread rt
   OSAL_VPATH   += linux
   OSAL_IPATH   += . linux
   OSAL_SRC     += osal.c os_task.c os_semaphore.c os_timer.c os_event.c \
                   os_queue.c os_log.c os_mutex.c

   CFLAGS       += -std=c99 -pedantic -funsigned-bitfields -Wundef
   CFLAGS       += -O2
endif

# -- MinGW32 adaptation layer --------------------------------------------------

ifeq (mingw32,$(findstring mingw32, $(OSAL_HOSTOS)))
   OSAL_DEFINES += _XOPEN_SOURCE=600
# _POSIX_C_SOURCE shouldn't really be necessary with _XOPEN_SOURCE=600, but
# seems to be at the moment
   OSAL_DEFINES += _POSIX_C_SOURCE=200112L

#   OSAL_DEFINES += OSAL_LOG_SIZE_LIMIT=1000000
   
   OSAL_LIBS    += winmm
   OSAL_LPATH   += .
   OSAL_LDFLAGS += -mwindows
   
   OSAL_VPATH   += mingw32
   OSAL_IPATH   += . mingw32
   OSAL_SRC     += osal.c os_task.c os_semaphore.c os_timer.c os_event.c \
                   os_queue.c os_log.c os_mutex.c

   CFLAGS       += -std=c99 -pedantic -mwindows
   CFLAGS       += -funsigned-bitfields -fno-builtin
   CFLAGS       += -W -Wstrict-prototypes -Wmissing-declarations -Wundef -Wall
   CFLAGS       += -DGNU
   CFLAGS       += -O2 -mwindows
endif

#========================# Configuration Variables #==========================#

# Specify where to put the objects (not in source)
BUILD_DIR := build

DIR_SEP   = /
OS_HEADER = os_localdefs.h

# Build the path to the OS-specific header
OSAL_DEFINES += OSAL_HOSTOS_LOCALDEFS=\"$(OSAL_HOSTOS)$(DIR_SEP)$(OS_HEADER)\"

# Set up source search path
VPATH += $(OSAL_VPATH)

# Do any conversions
OBJS    := $(patsubst %.c,$(BUILD_DIR)/%.o,$(OSAL_SRC))
CFLAGS  += $(patsubst %,-I%,$(OSAL_IPATH))
CFLAGS  += $(patsubst %,-D%,$(OSAL_DEFINES))
LDFLAGS += $(OSAL_LDFLAGS)
LDFLAGS += $(patsubst %,-L%,$(OSAL_LPATH))
LDLIBS  += $(patsubst %,-l%,$(OSAL_LIBS))

#===============================# Patterns #==================================#

$(BUILD_DIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
	
#=============================# Build Rules #=================================#

# Build targets for users

lib: libosal.a

test: testharness

clean: libosal_clean testharness_clean	

what:
	@echo Possible targets:
	@echo "  lib   : OSAL library only"
	@echo "  test  : library and test harness"
	@echo "  clean : remove all built files and directories"
	@echo "  what  : show this info"

# Internal targets

libosal.a: $(BUILD_DIR) $(OBJS)
	$(AR) -rcvs $@ $(OBJS)
	cp -f $(OSAL_HOSTOS)$(DIR_SEP)$(OS_HEADER) .
	
libosal_clean:
	rm -fr $(BUILD_DIR)
	rm -f libosal.a	
	rm -f $(OS_HEADER)
	
$(BUILD_DIR):
	mkdir $(BUILD_DIR)
	
testharness: build/testharness.o libosal.a
	$(CC) -o $@  $< $(LDFLAGS) -losal $(LDLIBS)
	
testharness_clean:
	rm -f testharness
	rm -f osal_test.txt
		
###################################[ END ]#####################################
