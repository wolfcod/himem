@echo off
set TCLIB=C:\TC\LIB\
echo Building HIMEM exe file

if %1 == "DEBUG" goto debug
rem settings for debug build

:debug
set TASMDEBUG=/zi
set LINKDEBUG=/v
set TCDEBUG=-M
goto build

rem settings for release build
:release
set TASMDEBUG=
set LINKDEBUG=
set TCDEBUG=

:build
echo deleting old file..
del *.map
del *.obj
del *.bak
del *.exe

tcc -c -mt %TCDEBUG% main.cpp
tcc -c -mt %TCDEBUG% cpuid.cpp
tcc -c -mt %TCDEBUG% xmsapi.cpp
tcc -c -mt %TCDEBUG% handler.cpp
tasm xmm386.asm
tasm driver.asm

if "%1" == "DRIVER" goto linkdrv

:link
echo Linking standalone version...
tlink /3 %LINKDEBUG% handler.obj main.obj cpuid.obj xmsapi.obj xmm386.obj c:\tc\lib\c0t.obj, himem.exe, himem.map, c:\tc\lib\cs.lib
if exist himem.exe goto clean
goto end

:linkdrv
echo Linking driver version...
rem Strip data from handler...
tlink /t /3 driver.obj xmm386.obj handler.obj, himem.sys, himem.map, c:\tc\lib\cs.lib
if exist himem.sys goto clean
goto end


:clean
del *.obj

:end
echo done.
