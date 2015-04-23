@ECHO OFF
SET root=%~dp0
MODE CON COLS=120 LINES=2000
SET PATH=C:\MinGW\bin\;C:\Program Files\CodeBlocks\bin;%PATH%

rem -----------------------------------------------------
CALL :if_exist "gcc.exe"
IF ERRORLEVEL 1 (
	ECHO Error : Please install MinGW!
	ECHO - For more information visit: http://scite-ru.org
	GOTO error
)

IF NOT "%1"=="/build" CALL :clear

rem -----------------------------------------------------
CALL :header Make Scintilla
CD %root%src\scintilla\win32
mingw32-make
IF ERRORLEVEL 1 GOTO error

rem -----------------------------------------------------
CALL :header Make SciTE
CD %root%src\scite\win32
mingw32-make
IF ERRORLEVEL 1 GOTO error

CD ..\bin
IF NOT EXIST SciTE.exe PAUSE

rem -----------------------------------------------------
MOVE /Y SciTE.exe ..\..\..\pack\
MOVE /Y SciLexer.dll ..\..\..\pack\
IF ERRORLEVEL 1 GOTO error

rem -----------------------------------------------------
CALL :header Make SHELL.DLL
CD %root%lualib\shell
CALL make.cmd
MOVE /Y shell.dll ..\..\pack\tools\LuaLib\

rem -----------------------------------------------------
CALL :header Make GUI.DLL
CD %root%lualib\gui
CALL make.cmd
MOVE /Y gui.dll ..\..\pack\tools\LuaLib\

rem -----------------------------------------------------
CALL :header Make LPEG.DLL
CD %root%lualib\lpeg
CALL make.cmd
MOVE /Y lpeg.dll ..\..\pack\tools\LuaLib\

rem -----------------------------------------------------
CALL :header Make COOL.DLL
CD %root%iconlib\cool
CALL make.cmd
MOVE /Y cool.dll ..\..\pack\toolbar\

rem -----------------------------------------------------
CALL :header Make GNOME.DLL
CD %root%iconlib\gnome
CALL make.cmd
MOVE /Y gnome.dll ..\..\pack\toolbar\

rem -----------------------------------------------------
IF "%1"=="/build" GOTO completed

CALL :clear

:completed
ECHO.
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO Building SciTE-Ru successfully completed!
TITLE SciTE-Ru completed
GOTO end

:error
ECHO.
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO Errors were found!
GOTO end

:error_install
ECHO.
ECHO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ECHO Please install MinGW!
ECHO For more information visit http://scite-ru.org
GOTO end

:if_exist
FOR /f %%i IN (%1) DO IF "%%~$PATH:i"=="" EXIT /b 1
EXIT /b 0

:header
ECHO.
ECHO ^> ~~~~~~ [ %* ] ~~~~~~
TITLE Create SciTE-Ru: %*
GOTO :EOF

:clear
CD %root%src
DEL /S /Q *.a *.aps *.bsc *.dll *.dsw *.exe *.idb *.ilc *.ild *.ilf *.ilk *.ils *.lib *.map *.ncb *.obj *.o *.opt *.pdb *.plg *.res *.sbr *.tds *.exp > NUL 2<&1
GOTO :EOF

:end
CD %root%
