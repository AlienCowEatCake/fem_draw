@echo off
set V_PROJECT=fem_draw_winapi
set MINGWDIR=C:\Qt\Tools\mingw342_32
set V_BUILDDIR=build_winapi_mingw342
set MINGW-CXX=g++
set MINGW-WINDRES=windres
set MINGW-STRIP=strip

set PATH=%MINGWDIR%\bin;%MINGWDIR%\libexec\gcc\mingw32\3.4.2;%WINDIR%;%WINDIR%\System32
set CPLUS_INCLUDE_PATH=%MINGWDIR%\include\c++
set C_INCLUDE_PATH=%MINGWDIR%\include;%MINGWDIR%\lib\gcc\mingw32\3.4.2\include
set INCLUDE=%CPLUS_INCLUDE_PATH%;%C_INCLUDE_PATH%
set LIBRARY_PATH=%MINGWDIR%\lib;%MINGWDIR%\lib\gcc\mingw32\3.4.2
set LIB=%LIBRARY_PATH%
@echo on

cd "%~dp0"
cd ..
rmdir /S /Q %V_BUILDDIR% 2>nul >nul
mkdir %V_BUILDDIR%
cd %V_BUILDDIR%
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/libs/lodepng.cpp -o lodepng.cpp.o
@if not exist "lodepng.cpp.o" goto err
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/libs/jo_jpeg.cpp -o jo_jpeg.cpp.o
@if not exist "jo_jpeg.cpp.o" goto err
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/libs/jo_gif.cpp -o jo_gif.cpp.o
@if not exist "jo_gif.cpp.o" goto err
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/libs/jo_tga.cpp -o jo_tga.cpp.o
@if not exist "jo_tga.cpp.o" goto err
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/main.cpp -o main.cpp.o
@if not exist "main.cpp.o" goto err
%MINGW-CXX% -c -O3 -DNDEBUG -DUNICODE -D_UNICODE -Wall -Wextra ../src_winapi/paintwidget.cpp -o paintwidget.cpp.o
@if not exist "paintwidget.cpp.o" goto err
%MINGW-WINDRES% -DNDEBUG -DUNICODE -D_UNICODE -I../src_winapi -I../resources ../src_winapi/resources.rc -o resources.rc.o
@if not exist "resources.rc.o" goto err
%MINGW-CXX% -O3 -Wall -Wextra main.cpp.o paintwidget.cpp.o resources.rc.o lodepng.cpp.o jo_jpeg.cpp.o jo_gif.cpp.o jo_tga.cpp.o -lgdi32 -lcomctl32 -luser32 -lcomdlg32 -lshell32 -mwindows -o %V_PROJECT%.exe
@if not exist "%V_PROJECT%.exe" goto err
strip --strip-all %V_PROJECT%.exe
copy %V_PROJECT%.exe ..\%V_PROJECT%_unicode.exe
@goto end

:err
@echo off
echo.
echo Error!

:end
@echo off
pause
