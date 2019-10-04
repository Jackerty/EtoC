####################################
## Makefile for EtoC project      ##
####################################
CC?=gcc

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

CXX_OBJECTS:= $(addprefix $(O)/,CxxMain.o CxxLex.o OpHand.o PrintTools.o ThreadTown.o Hash.o BufferManager.o)
CXX_LIBS:=-pthread -lrt
CXXEXE:=cxxtoc

.PHONY: all clean moduletest test_OpHand
all: $(CXXEXE)

# This is the compiler for c++ to c
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

$(O)/%.o:$(S)/%.c | $(O)
	$(CC) -c $(CFLAGS) -o$@ $<

$(O):
	mkdir $(O)

clean:
	rm -r $(O)
	rm $(CXXEXE)
