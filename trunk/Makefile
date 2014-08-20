# Makefile for Packit

include FixDeps:Rules/make
CXXFLAGS = -O0 -Wall -mpoke-function-name -mthrowback -ITBX: -IZipArchive:

LD = g++
LDFLAGS = -LTBX: -ltbx -LZipArchive: -lziparch -lstdc++ -static

BINDHELP = bindhelp
CCRES=ccres

TARGET=!PackIt.!RunImage
HELPTARGET=!PackIt.PackItHelp
RESTARGET=!PackIt.Res

CCSRC = $(wildcard src/*.cc)
OBJECTS = $(CCSRC:.cc=.o)
HELPFILES = $(wildcard StrongHelp/[^.]*)
RESASTEXT=src.ResAsText

all:	$(TARGET) $(RESTARGET) $(HELPTARGET)

$(TARGET): packit
	elf2aif packit $(TARGET)

packit:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o packit $(OBJECTS)

$(RESTARGET):	$(RESASTEXT)
	$(CCRES) $(RESASTEXT) $(RESTARGET)
	
$(HELPTARGET):		$(HELPFILES)
	$(BINDHELP) bindhelp £(HELPTARGET) -r -f

clean:
	rm $(OBJECTS) packit

#include $(CCSRC:.cc=.d)
