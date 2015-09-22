################################################################################
#
# Copyright 2014 NVIDIA Corporation.  All rights reserved.
#
# Please refer to the NVIDIA end user license agreement (EULA) associated
# with this source code for terms and conditions that govern your use of
# this software. Any use, reproduction, disclosure, or distribution of
# this software and related documentation outside the terms of the EULA
# is strictly prohibited.
#
################################################################################

CC       = pgcc
#CC	    = pathcc
OBJ     = o
EXE	= out
RUN     =
UNAME := $(shell uname -a)
ifeq ($(findstring CYGWIN_NT, $(UNAME)), CYGWIN_NT)
OBJ     = obj
EXE	= exe
endif

CCFLAGS  = -fast
ACCFLAGS = -Minfo -acc
#ACCFLAGS = -acc

all: build run verify

build: hw.c	
	$(CC) $(CCFLAGS) $(ACCFLAGS) -o hw.$(EXE) $<

run: hw.$(EXE)
	$(RUN) ./hw.$(EXE)

verify:


clean:
	@echo 'Cleaning up...'
	@rm -rf *.$(EXE) *.$(OBJ) *.dwf *.pdb prof
