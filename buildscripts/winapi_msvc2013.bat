@echo off
set V_PROJECT=fem_draw_winapi
set V_BUILDDIR=build_winapi_msvc2013
@echo on
call "%VS120COMNTOOLS%\vsvars32.bat"
@echo on

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\libs\lodepng.cpp /Fo:lodepng.cpp.obj
@if not exist "lodepng.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\libs\jo_jpeg.cpp /Fo:jo_jpeg.cpp.obj
@if not exist "jo_jpeg.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\libs\jo_gif.cpp /Fo:jo_gif.cpp.obj
@if not exist "jo_gif.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\libs\jo_tga.cpp /Fo:jo_tga.cpp.obj
@if not exist "jo_tga.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\main.cpp /Fo:main.cpp.obj
@if not exist "main.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /DNDEBUG /arch:IA32 ..\src_winapi\paintwidget.cpp /Fo:paintwidget.cpp.obj
@if not exist "paintwidget.cpp.obj" goto err
rc /nologo /DUNICODE /D_UNICODE /DPSAPI_VERSION=1 /fo resources.res ../src_winapi/resources.rc
@if not exist "resources.res" goto err
link /NOLOGO /SUBSYSTEM:WINDOWS,5.01 main.cpp.obj paintwidget.cpp.obj resources.res lodepng.cpp.obj jo_jpeg.cpp.obj jo_gif.cpp.obj jo_tga.cpp.obj gdi32.lib comctl32.lib user32.lib comdlg32.lib /OUT:%V_PROJECT%.exe
@if not exist "%V_PROJECT%.exe" goto err
copy %V_PROJECT%.exe ..\%V_PROJECT%.exe
@goto end

:err
@echo off
echo.
echo Error!

:end
@echo off
pause
