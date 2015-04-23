@ECHO OFF
SET PATH=C:\MinGW\bin;%ProgramFiles%\CodeBlocks\bin;%PATH%

CD /D "%~dp0"
windres -o resfile.o toolbar.rc
IF ERRORLEVEL 1 EXIT

ld --strip-all --dll -o gnome.dll resfile.o
REM gcc -s -shared -o gnome.dll resfile.o
IF ERRORLEVEL 1 EXIT

DEL resfile.o
