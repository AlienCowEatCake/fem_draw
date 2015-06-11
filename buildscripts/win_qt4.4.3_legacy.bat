@echo off
set V_PROJECT=fem_draw
set QTDIR=C:\Qt\Qt4.4.3-static
set MINGWDIR=C:\Qt\MinGW-3.4.2
set V_BUILDDIR=build_win_qt4.4.3_legacy

set PATH=%QTDIR%\bin;%MINGWDIR%\bin;%WINDIR%;%WINDIR%\System32
set INCLUDE=%QTDIR%\include\;%MINGWDIR%\include
set LIB=%QTDIR%\lib\;%MINGWDIR%\lib
set QDIR=%QTDIR%
set QMAKESPEC=win32-g++

set CPLUS_INCLUDE_PATH=%INCLUDE%
set C_INCLUDE_PATH=%INCLUDE%
set LIBRARY_PATH=%LIB%
set LD_LIBRARY_PATH=%LIB%
set MINGW=%MINGWDIR%

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
qmake CONFIG+="release" CONFIG+="use_static_qgif use_static_qjpeg use_static_qtiff" ..\%V_PROJECT%.pro
mingw32-make
strip --strip-all release\%V_PROJECT%.exe
copy release\%V_PROJECT%.exe ..\%V_PROJECT%.exe

pause

