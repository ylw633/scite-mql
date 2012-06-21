@echo off
%2\other\metalang.exe %1
if errorlevel 1 goto end
%2\other\refreshcrapt4.exe

:end