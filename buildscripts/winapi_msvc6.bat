@echo off
set V_PROJECT=fem_draw_winapi
set V_BUILDDIR=build_winapi_msvc6
@echo on
call "%PROGRAMFILES%\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
@echo on

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
cl /c /nologo /Ox /EHsc /W3 /MT /TP /DNDEBUG ..\src_winapi\libs\lodepng.cpp /Folodepng.cpp.obj
@if not exist "lodepng.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /TP /DIDI_ICON1=42 /DNDEBUG ..\src_winapi\main.cpp /Fomain.cpp.obj
@if not exist "main.cpp.obj" goto err
cl /c /nologo /Ox /EHsc /W3 /MT /TP /DIDI_ICON1=42 /DNDEBUG ..\src_winapi\paintwidget.cpp /Fopaintwidget.cpp.obj
@if not exist "paintwidget.cpp.obj" goto err
rc /DIDI_ICON1=42 /fo icon.res ../resources/icon.rc
@if not exist "icon.res" goto err
link /NOLOGO /SUBSYSTEM:WINDOWS main.cpp.obj paintwidget.cpp.obj icon.res lodepng.cpp.obj gdi32.lib comctl32.lib user32.lib comdlg32.lib /OUT:%V_PROJECT%.exe
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
