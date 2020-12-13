####################################
## Makefile for EtoC project      ##
####################################
# C Compiler 
CC?=gcc
# Source location
S:=src

CFLAGS:= -Wall -D_POSIX_C_SOURCE=200809L -std=c18 $(EXTRA_CFLAGS)

RELEASE?=0
ifeq ($(RELEASE),0)
  O:=debug
  CFLAGS+=-g
else ifeq ($(RELEASE),1)
  O:=release
  CFLAGS+=-O3
endif

# C++ to C compiler objects and libraries.
CXX_OBJECTS:= $(addprefix $(O)/,CxxMain.o CxxLex.o OpHand.o PrintTools.o ThreadTown.o Hash.o BufferManager.o)
CXX_LIBS:=-pthread -lrt
CXXEXE:=cxxtoc

# All depencies files.
DEP = $(CXX_OBJECTS:%.o=%.d)

.PHONY: all clean moduletest test_OpHand
all: $(CXXEXE)

# This is the compiler for C++ to C
$(CXXEXE): $(CXX_OBJECTS)
	$(CC) $(CFLAGS) -o$@ $^ $(CXX_LIBS)

# Compile and run module tests
moduletest: test_OpHand

test_OpHand: moduletest/test_OpHand.c src/OpHand.c
	$(CC) $(CFLAGS) $^ -otest
test_ThreadTown: moduletest/test_ThreadTown.c src/ThreadTown.c src/PrintTools.c
	$(CC) $(CFLAGS) $^ -otest -pthread

resHash: research/HashResearch.c
	$(CC) $(CFLAGS) $^ -otest

# If exists include header depencies.
-include $(DEP)
# Generic object creation with user (local) header depencies. 
$(O)/%.o:$(S)/%.c | $(O)
	$(CC) -c -MMD $(CFLAGS) -o$@ $<

# Make support directory.
$(O):
	mkdir -p $(O)

# Cleaning function.
clean:
	rm -r $(O)
	rm $(CXXEXE)
