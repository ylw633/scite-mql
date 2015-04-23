@echo off
REM for make debug version use: 
REM >make_with_MinGW.bat DEBUG

setlocal
set PATH=C:\MinGW\bin;%PATH%;

if "%1"=="DEBUG" set DEBUG=1

cd scintilla\win32
mingw32-make
if errorlevel 1 goto :end

cd ..\..\scite\win32
mingw32-make
if errorlevel 1 goto :end

copy /Y ..\bin\SciTE.exe ..\..\..\pack\
copy /Y ..\bin\SciLexer.dll ..\..\..\pack\

:end
cd ..\..
