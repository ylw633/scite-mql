@ECHO OFF
:: -------------------------------------------
SET sfx_filename=SciTE_Setup
SET RAR="%ProgramFiles%\WinRAR\WinRAR.exe"
SET cur_path=%~dp0
SET distrib_path=%cur_path%..\pack
:: -------------------------------------------

CD /D "%distrib_path%"
%RAR% a -r -rr -s -m5 "%cur_path%%sfx_filename%"
IF ERRORLEVEL 1 GOTO error

CD /D "%cur_path%"
%RAR% SSciTE.sfx %sfx_filename%.rar -Ztext.html
IF ERRORLEVEL 1 GOTO error

DEL %sfx_filename%.rar
EXIT

:error
ECHO Error create SFX file!
