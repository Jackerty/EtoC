####################################
## Makefile for EtoC project      ##
####################################
CC?=gcc

S:=src
CFLAGS:= -Wall

RELEASE?=0
ifeq ($(RELEASE),0)
  O:=debug
  CFLAGS+=-g
else ifeq ($(RELEASE),1)
  O:=release
  CFLAGS+=-O3
endif

CXX_OBJECTS:= $(addprefix $(O)/,Cxx.o CxxLex.o OpHand.o PrintTools.o)
CXX_LIBS:=
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

$(O)/%.o:$(S)/%.c | $(O)
	$(CC) -c $(CFLAGS) -o$@ $<

$(O):
	mkdir $(O)

clean:
	rm -r $(O)
	rm $(CXXEXE)
