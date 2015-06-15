@echo off
set V_PROJECT=fem_draw_winapi
set MINGWDIR=C:\Qt\MinGW-3.4.2
set V_BUILDDIR=build_winapi_mingw3.4.2
set MINGW-CXX=g++
set MINGW-WINDRES=windres
set MINGW-STRIP=strip

set PATH=%MINGWDIR%\bin;%WINDIR%;%WINDIR%\System32
set INCLUDE=%MINGWDIR%\include
set LIB=%MINGWDIR%\lib

set CPLUS_INCLUDE_PATH=%INCLUDE%
set C_INCLUDE_PATH=%INCLUDE%
set LIBRARY_PATH=%LIB%
set LD_LIBRARY_PATH=%LIB%
set MINGW=%MINGWDIR%

cd "%~dp0"
cd ..
@echo on
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
%MINGW-CXX% -c -O3 -DNDEBUG -DIDI_ICON1=42 -Wall -Wextra ../src_winapi/main.cpp -o main.cpp.o
%MINGW-CXX% -c -O3 -DNDEBUG -DIDI_ICON1=42 -Wall -Wextra ../src_winapi/paintwidget.cpp -o paintwidget.cpp.o
%MINGW-WINDRES%  -DIDI_ICON1=42 -I../resources ../resources/icon.rc -o icon.rc.o
%MINGW-CXX% -O3 -Wall -Wextra main.cpp.o paintwidget.cpp.o icon.rc.o -lgdi32 -lcomctl32 -luser32 -mwindows -o %V_PROJECT%.exe
strip --strip-all %V_PROJECT%.exe
copy %V_PROJECT%.exe ..\%V_PROJECT%.exe

@echo off
pause

