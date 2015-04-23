@ECHO OFF
SET python=c:\Python34\python.exe

IF NOT EXIST "%python%" GOTO error_install
SET cur_path=%~dp0

:: LexGen.py
CD %cur_path%scintilla\scripts
"%python%" LexGen.py
IF ERRORLEVEL 1 GOTO error

:: IFaceTableGen.py
CD %cur_path%scite\scripts
"%python%" IFaceTableGen.py
IF ERRORLEVEL 1 GOTO error

:: RegenerateSource.py
CD %cur_path%scite\scripts
"%python%" RegenerateSource.py
IF ERRORLEVEL 1 GOTO error

:completed
ECHO __________________
ECHO Generation successfully completed!
GOTO end

:error
ECHO __________________
ECHO Errors were found!
GOTO end

:error_install
ECHO Please install Python!
ECHO For more information visit http://scite-ru.org
GOTO end

:end
CD %cur_path%
PAUSE
