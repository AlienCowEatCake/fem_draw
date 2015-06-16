@echo off
set V_PROJECT=fem_draw_winapi
set V_BUILDDIR=build_winapi_msvc2010
@echo on
call "%VS100COMNTOOLS%\vsvars32.bat"
@echo on

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
cl /c /nologo /Ox /EHsc /W3 /MT /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /DIDI_ICON1=42 /DNDEBUG ..\src_winapi\main.cpp /Fomain.cpp.obj
@if not exist "main.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /DIDI_ICON1=42 /DNDEBUG ..\src_winapi\paintwidget.cpp /Fopaintwidget.cpp.obj
@if not exist "paintwidget.cpp.obj" goto err
rc /nologo /DUNICODE /D_UNICODE /DIDI_ICON1=42 /fo icon.res ../resources/icon.rc
@if not exist "icon.res" goto err
link /NOLOGO /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup main.cpp.obj paintwidget.cpp.obj icon.res gdi32.lib comctl32.lib user32.lib comdlg32.lib /OUT:%V_PROJECT%.exe
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
