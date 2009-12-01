# Makefile for Packit

include FixDeps:Rules/make
CXXFLAGS = -O2 -Wall -mpoke-function-name -mthrowback -ITBX:

LD = g++
LDFLAGS = -LTBX: -ltbx -lstdc++ -static


OBJECTS = main.o

^.!RunImage:	!RunImage
	elf2aif !RunImage ^.!RunImage
!RunImage:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o !RunImage,e1f $(OBJECTS)

#include $(OBJECTS:.o=.d)
