@echo off
%2\other\mql.exe /i:%3 %1
if errorlevel 1 goto end
%2\other\refreshcrapt4.exe

:end
exit %ERRORLEVEL%