# Makefile for Packit

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
  RESASTEXT=src.ResAsText

  BINDHELP = bindhelp
  CCRES=ccres

  TARGET=!PackIt.!RunImage
  HELPTARGET=!PackIt.PackItHelp
  RESTARGET=!PackIt.Res
  CMDTARGET=packitcmd
else
  INCLUDE_DIR = -I$(GCCSDK_INSTALL_ENV)/include
  LIB_DIR = -L$(GCCSDK_INSTALL_ENV)/lib
  RESASTEXT=src/ResAsText

  BINDHELP = $(GCCSDK_INSTALL_ENV)/bin/bindhelp
  CCRES=$(GCCSDK_INSTALL_ENV)/bin/ccres

  TARGET=!PackIt/!RunImage,ff8
  HELPTARGET=!PackIt/PackItHelp,3d6
  RESTARGET=!PackIt/Res,fae
  CMDTARGET=packitcmd,ff8
endif

CXXFLAGS = -O0 -Wall -mpoke-function-name -mthrowback $(INCLUDE_DIR)
CPPFLAGS = -MD -MP

LD = g++
LDFLAGS = $(LIB_DIR) -ltbx -lziparch -lstdc++ -static


CCSRC = $(wildcard src/*.cc)
OBJECTS = $(CCSRC:.cc=.o)
HELPFILES = $(wildcard StrongHelp/[^.]*)

all:	$(TARGET) $(RESTARGET) $(HELPTARGET) $(CMDTARGET)

$(TARGET): packit
	elf2aif packit $(TARGET)

packit:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o packit $(OBJECTS)

$(CMDTARGET):
	$(MAKE) -C packitcmdsrc 

$(RESTARGET):	$(RESASTEXT)
	$(CCRES) $(RESASTEXT) $(RESTARGET)
	
$(HELPTARGET):		$(HELPFILES)
	$(BINDHELP) StrongHelp $(HELPTARGET) -r -f

clean:
	rm $(OBJECTS) packit

-include $(CCSRC:.cc=.d)

