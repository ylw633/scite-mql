@echo off
REM for make debug version use: 
REM >make_with_VC.bat DEBUG

setlocal
set VC7=%ProgramFiles%\Microsoft Visual Studio .NET
set VC71=%ProgramFiles%\Microsoft Visual Studio .NET 2003
set VC8=%ProgramFiles%\Microsoft Visual Studio 8
set VC9=%ProgramFiles%\Microsoft Visual Studio 9.0
set VC10=%ProgramFiles%\Microsoft Visual Studio 10.0
set VC12=%ProgramFiles%\Microsoft Visual Studio 12.0
set Tools=Common7\Tools
set path=%VC12%\%Tools%;%VC10%\%Tools%;%VC9%\%Tools%;%VC8%\%Tools%;%VC71%\%Tools%;%VC7%\%Tools%;%path%
call vsvars32.bat

if "%1"=="DEBUG" set parameter1=DEBUG=1

cd scintilla\win32
nmake %parameter1% -f scintilla.mak
if errorlevel 1 goto :end

cd ..\..\scite\win32
nmake %parameter1% -f scite.mak
if errorlevel 1 goto :end

copy /Y ..\bin\SciTE.exe ..\..\..\pack\
copy /Y ..\bin\SciLexer.dll ..\..\..\pack\

:end
cd ..\..
