@echo off
set V_PROJECT=fem_draw_qt
set V_ARCH=x64
set V_VCVARS="C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall.bat"
set V_QTDIR=C:\Qt\4.4.3\msvc2005_64_static
set V_JOM=C:\Qt\Tools\QtCreator\bin\jom.exe
set V_BUILDDIR=build_win_qt4.4.3_msvc2005_x64

call %V_VCVARS% %V_ARCH%
set PATH=%V_QTDIR%\bin;%PATH%
set QMAKESPEC=win32-msvc2005

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
qmake CONFIG+="release" CONFIG+="use_static_qjpeg use_static_qtiff" ..\%V_PROJECT%.pro
%V_JOM% -j1
copy release\%V_PROJECT%.exe ..\%V_PROJECT%_x64.exe

pause

