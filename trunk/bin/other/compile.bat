@echo off
@cd %2
.\other\metalang.exe %1
if errorlevel 1 goto end
.\other\refreshcrapt4.exe

:end