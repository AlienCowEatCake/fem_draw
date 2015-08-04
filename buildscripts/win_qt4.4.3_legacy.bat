@echo off
set V_PROJECT=fem_draw_qt
set QTDIR=C:\Qt\qt4.4.3-mingw342-static
set MINGWDIR=C:\Qt\MinGW-3.4.2
set V_BUILDDIR=build_win_qt4.4.3_legacy

set PATH=%QTDIR%\bin;%MINGWDIR%\bin;%MINGWDIR%\libexec\gcc\mingw32\3.4.2;%WINDIR%;%WINDIR%\System32
set CPLUS_INCLUDE_PATH=%MINGWDIR%\include\c++
set C_INCLUDE_PATH=%MINGWDIR%\include;%MINGWDIR%\lib\gcc\mingw32\3.4.2\include
set INCLUDE=%QTDIR%\include;%CPLUS_INCLUDE_PATH%;%C_INCLUDE_PATH%
set LIBRARY_PATH=%MINGWDIR%\lib;%MINGWDIR%\lib\gcc\mingw32\3.4.2
set LIB=%QTDIR%\lib\;%LIBRARY_PATH%
set QDIR=%QTDIR%
set QMAKESPEC=win32-g++

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
qmake CONFIG+="release" CONFIG+="use_static_qjpeg use_static_qtiff" ..\%V_PROJECT%.pro
mingw32-make
strip --strip-all release\%V_PROJECT%.exe
copy release\%V_PROJECT%.exe ..\%V_PROJECT%.exe

pause

