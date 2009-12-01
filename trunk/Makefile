# Makefile for Packit

include FixDeps:Rules/make
CXXFLAGS = -O2 -Wall -mpoke-function-name -mthrowback -ITBX: -IZipArchive:

LD = g++
LDFLAGS = -LTBX: -ltbx -LZipArchive: -lziparch -lstdc++ -static

TARGET=!PackIt/!RunImage

CCSRC = $(wildcard src/*.cc)
OBJECTS = $(CCSRC:.cc=.o)

all:	$(TARGET)

$(TARGET): packit
	elf2aif packit $(TARGET)

packit:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o packit $(OBJECTS)

#include $(CCSRC:.cc=.d)
