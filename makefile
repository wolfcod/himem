#  Makefile for cpuid example.
#
#  Copyright (c) 1996 by Borland International, Inc.
#
#  Usage:   make -B         # cpuid example
#           make -B -DDEBUG # cpuid example with debug info
#

!if $d(DEBUG)
TASMDEBUG=/zi
LINKDEBUG=/v
TCDEBUG=/zi
TLIB=C:\TC\LIB
!else
TASMDEBUG=
LINKDEBUG=
TCDEBUG=
TLIB=C:\TC\LIB
!endif

RUNTIME=$(TLIB)\c0t.obj
CRTLIB=$(TLIB)\cs.lib

cpuid.exe:
      tcc -c -mt $(TCDEBUG) main.cpp
      tcc -c -mt $(TCDEBUG) cpuid.cpp
      tcc -c -mt $(TCDEBUG) xmsapi.cpp
      tcc -c -mt $(TCDEBUG) handler.cpp
      tasm xmm386.asm
      # tlink /3 $(LINKDEBUG) handler.obj main.obj cpuid.obj xmsapi.obj $(RUNTIME), himem.exe, himem.map, $(CRTLIB)

clean:
        del *.obj
