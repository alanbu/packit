# Makefile for Packitcmd

# Detect host
HOST := $(shell uname -s)
ifeq ($(HOST),)
  # Assume RISC OS
  HOST := riscos
else
  ifeq ($(HOST),RISC OS)
    HOST := riscos
  endif
endif

ifeq ($(HOST),riscos)
  INCLUDE_DIR = -ITBX: -IZipArchive:
  LIB_DIR = -LTBX: - LZipArchive:

  TARGET=^.!PackIt.packitcmd
else
  INCLUDE_DIR = -I$(GCCSDK_INSTALL_ENV)/include
  LIB_DIR = -L$(GCCSDK_INSTALL_ENV)/lib

  TARGET=../\!PackIt/packitcmd,ff8
endif

CXXFLAGS = -O0 -Wall -mpoke-function-name -std=c++11 -mthrowback $(INCLUDE_DIR)
CPPFLAGS = -MD -MP

LD = g++
LDFLAGS = $(LIB_DIR) -ltbx -lziparch -lstdc++ -static


CCSRC = $(wildcard *.cc)
OBJECTS = $(CCSRC:.cc=.o)

all:	$(TARGET) $(RESTARGET)

$(TARGET): packitcmdelf
	elf2aif packitcmdelf $(TARGET)

packitcmdelf:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o packitcmdelf $(OBJECTS)

clean:
	rm $(OBJECTS) packitcmdelf

-include $(CCSRC:.cc=.d)

