@ECHO OFF
SET root=%~dp0

rem -----------------------------------------------------
CALL :header Make SHELL.DLL
CD %root%\shell
CALL make.cmd
MOVE /Y shell.dll ..\..\pack\tools\LuaLib\

rem -----------------------------------------------------
CALL :header Make GUI.DLL
CD %root%\gui
CALL make.cmd
MOVE /Y gui.dll ..\..\pack\tools\LuaLib\

rem -----------------------------------------------------
CALL :header Make LPEG.DLL
CD %root%\lpeg
CALL make.cmd
MOVE /Y lpeg.dll ..\..\pack\tools\LuaLib\

GOTO end

:header
ECHO.
ECHO ^> ~~~~~~ [ %* ] ~~~~~~
TITLE Create SciTE-Ru: %*
GOTO :EOF

:end
CD %root%
